#ifndef __USER_OLEDDISPLAYTASK_H__
#define __USER_OLEDDISPLAYTASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "queue.h"
#include "fft_visualer.h"

typedef enum
{
  OLED_EVENT_KWS = 0,
  OLED_EVENT_SPECTRUM_FRAME,
  OLED_EVENT_SPECTRUM_STOP,
} OLED_EventType_t;

typedef struct
{
  OLED_EventType_t type;
  float            confidence;
  uint8_t          band_count;
  uint8_t          bands[FFT_VISUAL_MAX_BANDS];
} OLED_Event_t;

extern QueueHandle_t xOledEventQueue;

void OledDisplayTask(void* argument);

#ifdef __cplusplus
}
#endif

#endif
