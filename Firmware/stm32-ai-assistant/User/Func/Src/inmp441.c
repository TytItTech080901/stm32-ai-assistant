#include "inmp441.h"
#include "i2s.h"
#include "FreeRTOS.h"
#include "task.h"

uint32_t i2s_rx_dma_buf[I2S_DMA_BUF_LEN]; // 1280
int16_t  pcm_buf[I2S_DMA_BUF_LEN / 8];    // 160

volatile uint8_t audio_buf_idx = 0;

TaskHandle_t xAudioFbankTaskHandle = NULL;

void AudioIO_Init(void)
{
  HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)i2s_rx_dma_buf, I2S_DMA_BUF_LEN / 2);
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef* hi2s)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  audio_buf_idx                       = 0;
  if (xAudioFbankTaskHandle != NULL) {
    vTaskNotifyGiveFromISR(xAudioFbankTaskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef* hi2s)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  audio_buf_idx                       = 1;
  if (xAudioFbankTaskHandle != NULL) {
    vTaskNotifyGiveFromISR(xAudioFbankTaskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void Audio_GetPCM(uint8_t buf_idx)
{
  /* buf_idx: 0=ping缓冲区 */
  int      start = (buf_idx == 1) ? (I2S_DMA_BUF_LEN / 2) : 0;
  int16_t* pOut  = pcm_buf;
  for (int i = start; i < I2S_DMA_BUF_LEN / 2 + start; i += 4) {
    *pOut++ = (int16_t)i2s_rx_dma_buf[i];
  }
}
