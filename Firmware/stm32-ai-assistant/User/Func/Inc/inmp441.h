#ifndef __INMP441_H
#define __INMP441_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#define I2S_DMA_BUF_LEN 1280U
#define PCM_BUF_LEN (I2S_DMA_BUF_LEN / 8) /* 160 samples = 10ms @16kHz */

extern uint32_t         i2s_rx_dma_buf[I2S_DMA_BUF_LEN];
extern int16_t          pcm_buf[PCM_BUF_LEN];
extern volatile uint8_t audio_buf_idx;
extern TaskHandle_t     xAudioFbankTaskHandle;

void AudioIO_Init(void);
void Audio_GetPCM(uint8_t buf_idx);

#endif
