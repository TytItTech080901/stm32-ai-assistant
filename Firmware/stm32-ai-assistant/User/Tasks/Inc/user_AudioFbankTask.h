#ifndef __USER_AUDIOFBANKTASK_H
#define __USER_AUDIOFBANKTASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "queue.h"

extern QueueHandle_t xFbankQueue;

void AudioFbankTask(void* argument);

#ifdef __cplusplus
}
#endif

#endif
