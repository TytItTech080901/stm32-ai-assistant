/**
 ******************************************************************************
 * @file    speaker.c
 * @brief   MAX98357 I2S speaker driver with W25Q streaming playback
 ******************************************************************************
 */

#include "speaker.h"
#include "w25qxx.h"
#include "i2s.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "user_ShowAudioTask.h"

#define SPK_BUF_SAMPLES 512

static int16_t buf_A[SPK_BUF_SAMPLES];
static int16_t buf_B[SPK_BUF_SAMPLES];

static SemaphoreHandle_t xDmaDoneSem = NULL;

QueueHandle_t xSpeakerQueue = NULL;

static void Speaker_StreamVisualizeSamples(const int16_t* samples, uint32_t sample_count)
{
  static int16_t  hop_buffer[PCM_BUF_LEN];
  static uint32_t hop_fill = 0;

  for (uint32_t i = 0; i < sample_count; ++i) {
    hop_buffer[hop_fill++] = samples[i];
    if (hop_fill == PCM_BUF_LEN) {
      ShowAudio_SubmitPcmSamples(hop_buffer, PCM_BUF_LEN);
      hop_fill = 0;
    }
  }
}

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

  ShowAudio_StreamBegin();

  uint32_t chunk = (remaining > SPK_BUF_SAMPLES) ? SPK_BUF_SAMPLES : remaining;
  W25Q_Read(read_addr, (uint8_t*)play_buf, chunk * 2U);
  read_addr += chunk * 2U;
  remaining -= chunk;
  Speaker_StreamVisualizeSamples(play_buf, chunk);

  HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)play_buf, chunk);

  while (remaining > 0U) {
    chunk = (remaining > SPK_BUF_SAMPLES) ? SPK_BUF_SAMPLES : remaining;
    W25Q_Read(read_addr, (uint8_t*)load_buf, chunk * 2U);
    read_addr += chunk * 2U;
    remaining -= chunk;
    Speaker_StreamVisualizeSamples(load_buf, chunk);

    xSemaphoreTake(xDmaDoneSem, portMAX_DELAY);
    HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)load_buf, chunk);

    int16_t* tmp = play_buf;
    play_buf     = load_buf;
    load_buf     = tmp;
  }

  xSemaphoreTake(xDmaDoneSem, portMAX_DELAY);
  ShowAudio_StreamEnd();
}
