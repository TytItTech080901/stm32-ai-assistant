#include "uart_cli.h"
#include "usart.h"
#include "task.h"
#include <stdio.h>

void UartCliTask(void* argument)
{
  // for dump mode test, not use in final project
  // if (g_dump_mode) {
  //   vTaskSuspend(NULL);
  // }

  char    buf[CLI_BUF_LEN];
  uint8_t idx = 0;
  uint8_t ch;
  uint8_t last_was_cr = 0; /* 用于过滤 \r\n 中的 \n */

  printf("  A simple CLI\r\n");
  printf("  Type 'help' for commands\r\n");
  printf("> ");

  for (;;) {
    size_t len = xStreamBufferReceive(xUartRxStream, &ch, 1, portMAX_DELAY);
    if (len == 0) {
      continue;
    }

    /* 过滤 \r\n 序列中重复的换行 */
    if (ch == '\n' && last_was_cr) {
      last_was_cr = 0;
      continue;
    }
    last_was_cr = (ch == '\r');

    HAL_UART_Transmit(&huart1, &ch, 1, HAL_MAX_DELAY);

    if (ch == '\r' || ch == '\n') {
      printf("\r\n");
      buf[idx] = '\0';
      cli_parse(buf);
      idx = 0;
      if (!g_dump_mode)
        printf("> ");
    } else if (ch == '\b' || ch == 127) {
      if (idx > 0) {
        idx--;
        printf(" \b");
      }
    } else {
      if (idx < CLI_BUF_LEN - 1) {
        buf[idx++] = (char)ch;
      }
    }
  }
}
