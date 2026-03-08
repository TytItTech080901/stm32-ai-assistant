/**
 ******************************************************************************
 * @file    speaker.c
 * @brief   MAX98357 I2S 喇叭驱动 — 乒乓双缓冲 DMA 播放
 *
 *  I2S3: 16kHz, 16-bit, Philips, Master TX, DMA1_Stream5
 ******************************************************************************
 */

#include "speaker.h"
#include "w25qxx.h"
#include "i2s.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define SPK_BUF_SAMPLES 512

static int16_t buf_A[SPK_BUF_SAMPLES];
static int16_t buf_B[SPK_BUF_SAMPLES];

static SemaphoreHandle_t xDmaDoneSem = NULL;

QueueHandle_t xSpeakerQueue = NULL;

void Speaker_Init(void)
{
  xDmaDoneSem = xSemaphoreCreateBinary();
  configASSERT(xDmaDoneSem != NULL);
}

void Speaker_PlayFromFlash(uint32_t flash_addr, uint32_t samples)
{
  Speaker_Request_t req = {.flash_addr = flash_addr, .samples = samples};
  xQueueSend(xSpeakerQueue, &req, 0);
}

/* DMA 发送完成回调 */
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef* hi2s)
{
  if (hi2s->Instance == SPI3 && xDmaDoneSem != NULL) {
    BaseType_t xWoken = pdFALSE;
    xSemaphoreGiveFromISR(xDmaDoneSem, &xWoken);
    portYIELD_FROM_ISR(xWoken);
  }
}

void Speaker_StreamPlay(uint32_t flash_addr, uint32_t total_samples)
{
  int16_t* play_buf  = buf_A;
  int16_t* load_buf  = buf_B;
  uint32_t remaining = total_samples;
  uint32_t read_addr = flash_addr;

  /* 读第一块 */
  uint32_t chunk = (remaining > SPK_BUF_SAMPLES) ? SPK_BUF_SAMPLES : remaining;
  W25Q_Read(read_addr, (uint8_t*)play_buf, chunk * 2);
  read_addr += chunk * 2;
  remaining -= chunk;

  /* 启动 DMA 播放第一块 */
  HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)play_buf, chunk);

  while (remaining > 0) {
    /* DMA 在播放 play_buf 的同时，从 Flash 预读下一块到 load_buf */
    chunk = (remaining > SPK_BUF_SAMPLES) ? SPK_BUF_SAMPLES : remaining;
    W25Q_Read(read_addr, (uint8_t*)load_buf, chunk * 2);
    read_addr += chunk * 2;
    remaining -= chunk;

    /* 等待当前 DMA 播放完成 */
    xSemaphoreTake(xDmaDoneSem, portMAX_DELAY);

    /* 启动播放刚读好的 load_buf */
    HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)load_buf, chunk);

    /* 交换 */
    int16_t* tmp = play_buf;
    play_buf     = load_buf;
    load_buf     = tmp;
  }

  /* 等最后一块播完 */
  xSemaphoreTake(xDmaDoneSem, portMAX_DELAY);
}
