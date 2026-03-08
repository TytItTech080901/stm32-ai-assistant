#ifndef __USER_KWSINFERTASK_H__
#define __USER_KWSINFERTASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t     xKwsInferTaskHandle;
extern volatile uint8_t kws_model_ready;

void KwsInferTask(void* argument);

#ifdef __cplusplus
}
#endif

#endif
