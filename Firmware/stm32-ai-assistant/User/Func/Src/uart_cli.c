#include "uart_cli.h"
#include "usart.h"
#include "main.h"
#include "w25qxx.h"
#include "inmp441.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "network.h"
#include "network_data_params.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ---- Stream Buffer for UART RX ---------------------------------------- */
StreamBufferHandle_t xUartRxStream = NULL;
static uint8_t       uart_rx_byte;

int fputc(int ch, FILE* f)
{
  HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
  if (huart->Instance == USART1 && xUartRxStream != NULL) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xStreamBufferSendFromISR(xUartRxStream, &uart_rx_byte, 1, &xHigherPriorityTaskWoken);
    HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)
{
  if (huart->Instance == USART1) {
    __HAL_UART_CLEAR_OREFLAG(huart);
    HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
  }
}

void UART_StartReceiveIT(void)
{
  xUartRxStream = xStreamBufferCreate(128, 1);
  configASSERT(xUartRxStream != NULL);
  HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
}

typedef struct
{
  const char* name;
  const char* help;
  void (*handler)(void);
} CLI_Command_t;

static void cmd_help(void);
static void cmd_status(void);
static void cmd_heap(void);
static void cmd_model(void);
static void cmd_burn(const char* args);

static const CLI_Command_t cli_commands[] = {
    {"help", "Show available commands", cmd_help},
    {"status", "System status", cmd_status},
    {"heap", "FreeRTOS heap info", cmd_heap},
    {"model", "AI model info", cmd_model},
};
#define CLI_CMD_COUNT (sizeof(cli_commands) / sizeof(cli_commands[0]))

// this para only for test, not use in final project
volatile uint8_t g_dump_mode = 0;

static void cmd_help(void)
{
  printf("Available commands:\r\n");
  for (uint32_t i = 0; i < CLI_CMD_COUNT; i++) {
    printf("  %-10s %s\r\n", cli_commands[i].name, cli_commands[i].help);
  }
  printf("  %-10s %s\r\n", "burn", "Burn audio to W25Q flash");
}

static void cmd_status(void)
{
  uint32_t tick = (uint32_t)xTaskGetTickCount();
  uint32_t sec  = tick / 1000;
  uint32_t h    = sec / 3600;
  uint32_t m    = (sec % 3600) / 60;
  uint32_t s    = sec % 60;

  printf("  MCU:        STM32F407VGT6\r\n");
  printf("  RTOS:       FreeRTOS %s\r\n", tskKERNEL_VERSION_NUMBER);
  printf("  Runtime:     %02lu:%02lu:%02lu\r\n", (unsigned long)h, (unsigned long)m,
         (unsigned long)s);
  printf("  Tasks:      %lu\r\n", (unsigned long)uxTaskGetNumberOfTasks());
}

static void cmd_heap(void)
{
  printf("  Free:     %u bytes\r\n", (unsigned)xPortGetFreeHeapSize());
  printf("  Min ever: %u bytes\r\n", (unsigned)xPortGetMinimumEverFreeHeapSize());
}

static void cmd_model(void)
{
  printf("  Name:         %s\r\n", AI_NETWORK_ORIGIN_MODEL_NAME);
  printf("  Nodes:        %d\r\n", AI_NETWORK_N_NODES);
  printf("  Input:        %d floats (%d bytes)\r\n", AI_NETWORK_IN_1_SIZE,
         AI_NETWORK_IN_1_SIZE_BYTES);
  printf("  Output:       %d floats (%d bytes)\r\n", AI_NETWORK_OUT_1_SIZE,
         AI_NETWORK_OUT_1_SIZE_BYTES);
  printf("  Activations:  %d bytes\r\n", AI_NETWORK_DATA_ACTIVATIONS_SIZE);
  printf("  Weights:      %d bytes\r\n", AI_NETWORK_DATA_WEIGHTS_SIZE);
}

static uint8_t burn_page_buf[256];
static void    cmd_burn(const char* args)
{
  uint32_t size = (args && *args) ? strtoul(args, NULL, 0) : 0;
  if (size == 0) {
    printf("  Usage: burn <size_in_bytes>\r\n");
    printf("  Use Usart to send data\r\n");
    return;
  }

  /* 1) 擦除所需扇区 (4KB/扇区) */
  uint32_t sectors = (size + 4095) / 4096;
  printf("  Erasing %lu sectors...", (unsigned long)sectors);
  for (uint32_t i = 0; i < sectors; i++) {
    W25Q_SectorErase(i * 4096);
  }
  printf(" OK\r\n");

  /* 2) 通知主机可以开始发送 */
  printf("READY\r\n");

  /* 3) 接收并编程 */
  uint32_t addr      = 0;
  uint32_t remaining = size;

  while (remaining > 0) {
    uint16_t chunk = (remaining > 256) ? 256 : (uint16_t)remaining;

    /* 从 Stream Buffer 读取 chunk 字节 */
    uint16_t received = 0;
    while (received < chunk) {
      size_t n = xStreamBufferReceive(xUartRxStream, &burn_page_buf[received], chunk - received,
                                      pdMS_TO_TICKS(10000));
      if (n == 0) {
        printf("\r\nERROR: timeout at offset %lu\r\n", (unsigned long)addr);
        return;
      }
      received += (uint16_t)n;
    }

    /* 写入 Flash */
    W25Q_PageProgram(addr, burn_page_buf, chunk);
    addr += chunk;
    remaining -= chunk;

    /* ACK: 主机等待收到 'K' 后发下一包 */
    uint8_t ack = 'K';
    HAL_UART_Transmit(&huart1, &ack, 1, HAL_MAX_DELAY);
  }

  printf("\r\nDONE: %lu bytes -> flash 0x000000\r\n", (unsigned long)size);
}

void cli_parse(char* line)
{
  size_t len = strlen(line);
  while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\r' || line[len - 1] == '\n')) {
    line[--len] = '\0';
  }
  if (len == 0)
    return;

  if (strncmp(line, "burn", 4) == 0 && (line[4] == ' ' || line[4] == '\0')) {
    cmd_burn(line[4] == ' ' ? line + 5 : NULL);
    return;
  }

  for (uint32_t i = 0; i < CLI_CMD_COUNT; i++) {
    if (strcmp(line, cli_commands[i].name) == 0) {
      cli_commands[i].handler();
      return;
    }
  }
  printf("  Unknown command: '%s'. Type 'help' for list.\r\n", line);
}
