/**
 ******************************************************************************
 * @file    user_SpeakerTask.c
 * @brief   Speaker 播放 FreeRTOS 任务
 *
 *  从 xSpeakerQueue 接收播放请求，调用 Speaker 驱动层的
 *  Speaker_StreamPlay() 从 W25Q16 读取并通过 I2S3 DMA 播放。
 ******************************************************************************
 */

#include "user_SpeakerTask.h"
#include "speaker.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "uart_cli.h"

void SpeakerTask(void* argument)
{
  // if (g_dump_mode) {
  //   vTaskSuspend(NULL);
  // }
  Speaker_Request_t req;

  Speaker_Init();

  for (;;) {
    if (xQueueReceive(xSpeakerQueue, &req, portMAX_DELAY) == pdTRUE) {
      Speaker_StreamPlay(req.flash_addr, req.samples);
    }
  }
}