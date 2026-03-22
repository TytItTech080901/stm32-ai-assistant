#ifndef __UART_CLI_H
#define __UART_CLI_H

#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "stream_buffer.h"

#define CLI_BUF_LEN 64

extern StreamBufferHandle_t xUartRxStream;

// extern volatile uint8_t  g_dump_mode;

int  fputc(int ch, FILE* f);
void cli_parse(char* line);
void UartCliTask(void* argument);
void UART_StartReceiveIT(void);

#endif
