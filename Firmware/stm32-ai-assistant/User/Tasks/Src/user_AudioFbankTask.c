/**
  ******************************************************************************
  * @file    user_AudioFbankTask.c
  * @brief   Audio capture + Fbank feature extraction FreeRTOS task
  ******************************************************************************
  */

#include "user_AudioFbankTask.h"
#include "inmp441.h"
#include "fbank.h"
#include "uart_cli.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>

#define FBANK_BINS NUM_MELS
#define FBANK_QUEUE_LEN 4

QueueHandle_t xFbankQueue = NULL;

void AudioFbankTask(void* argument)
{
    AudioIO_Init();
    FbankFeature_Init();
    configASSERT(xFbankQueue != NULL);

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        Audio_GetPCM(audio_buf_idx);

        if (g_dump_mode) {
            HAL_UART_Transmit(&huart1, (uint8_t*)pcm_buf, PCM_BUF_LEN * 2, HAL_MAX_DELAY);
        } else {
            AudioPreprocessing_Run();
            xQueueSend(xFbankQueue, pOutColBuffer, 0);
        }
    }
}
