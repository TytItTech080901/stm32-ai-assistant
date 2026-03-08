#ifndef __USER_OLEDDISPLAYTASK_H__
#define __USER_OLEDDISPLAYTASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "queue.h"

typedef struct
{
  float confidence;
} OLED_KwsEvent_t;

extern QueueHandle_t xOledEventQueue;

void OledDisplayTask(void* argument);

#ifdef __cplusplus
}
#endif

#endif
