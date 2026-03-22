#include "user_OledDisplayTask.h"
#include "user_KwsInferTask.h"
#include "OLED.h"
#include "uart_cli.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

QueueHandle_t xOledEventQueue = NULL;

static void Screen_Boot(void)
{
  OLED_Clear();
  OLED_ShowString(1, 2, "AI Assistant");
  OLED_ShowString(2, 4, "STM32F407");
  OLED_ShowString(3, 5, "FreeRTOS");
  OLED_ShowString(4, 3, "Booting...");
}

static void Screen_MainFull(void)
{
  char     line[17];
  uint32_t heap_kb = xPortGetFreeHeapSize() / 1024;

  OLED_Clear();
  OLED_ShowString(1, 1, "AI Assistant");
  OLED_ShowString(2, 1, "Stat: Listening");

  snprintf(line, sizeof(line), "Heap:  %3lu KB", (unsigned long)heap_kb);
  OLED_ShowString(3, 1, line);

  if (kws_model_ready)
    OLED_ShowString(4, 1, "Model: Ready  ");
  else
    OLED_ShowString(4, 1, "Model: Init...");
}

static void Screen_MainRefresh(void)
{
  char     line[17];
  uint32_t heap_kb = xPortGetFreeHeapSize() / 1024;

  snprintf(line, sizeof(line), "Heap:  %3lu KB", (unsigned long)heap_kb);
  OLED_ShowString(3, 1, line);

  if (kws_model_ready)
    OLED_ShowString(4, 1, "Model: Ready  ");
}

static void Screen_Kws(uint8_t conf_pct)
{
  char line[17];

  OLED_Clear();
  OLED_ShowString(1, 1, "** Detected! **");
  OLED_ShowString(2, 3, "Wake Word!");

  snprintf(line, sizeof(line), "Conf: %u%%", conf_pct);
  OLED_ShowString(3, 1, line);
}

static void Screen_SpectrumHeader(void)
{
  OLED_Clear();
  OLED_ShowString(1, 1, "Playback FFT");
}

void OledDisplayTask(void* argument)
{
  OLED_Event_t evt;
  uint8_t      spectrum_active                  = 0;
  uint8_t      last_bands[FFT_VISUAL_MAX_BANDS] = {0};
  uint8_t      last_band_count                  = FFT_VISUAL_MAX_BANDS;
  TickType_t   last_spectrum_tick               = 0;

  (void)argument;

  OLED_Init();
  Screen_Boot();
  vTaskDelay(pdMS_TO_TICKS(2000));

  Screen_MainFull();

  for (;;) {
    if (xQueueReceive(xOledEventQueue, &evt, pdMS_TO_TICKS(500)) == pdTRUE) {
      if (evt.type == OLED_EVENT_KWS) {
        uint8_t pct = (uint8_t)(evt.confidence * 100.0f);
        Screen_Kws(pct);
        vTaskDelay(pdMS_TO_TICKS(2000));

        while (xQueueReceive(xOledEventQueue, &evt, 0) == pdTRUE) {
          if (evt.type == OLED_EVENT_SPECTRUM_FRAME) {
            spectrum_active    = 1;
            last_band_count    = evt.band_count;
            last_spectrum_tick = xTaskGetTickCount();
            memcpy(last_bands, evt.bands, evt.band_count);
          } else if (evt.type == OLED_EVENT_SPECTRUM_STOP && !spectrum_active) {
            spectrum_active = 0;
          }
        }

        if (spectrum_active) {
          Screen_SpectrumHeader();
          OLED_DrawSpectrumBars(last_bands, last_band_count);
        } else {
          Screen_MainFull();
        }
      } else if (evt.type == OLED_EVENT_SPECTRUM_FRAME) {
        if (!spectrum_active) {
          Screen_SpectrumHeader();
        }
        spectrum_active    = 1;
        last_band_count    = evt.band_count;
        last_spectrum_tick = xTaskGetTickCount();
        memcpy(last_bands, evt.bands, evt.band_count);
        OLED_DrawSpectrumBars(last_bands, last_band_count);
      } else if (evt.type == OLED_EVENT_SPECTRUM_STOP) {
        if (spectrum_active) {
          last_spectrum_tick = xTaskGetTickCount();
        } else {
          Screen_MainFull();
        }
      }
    } else {
      if (spectrum_active) {
        if ((xTaskGetTickCount() - last_spectrum_tick) > pdMS_TO_TICKS(1000)) {
          spectrum_active = 0;
          Screen_MainFull();
        }
      } else {
        Screen_MainRefresh();
      }
    }
  }
}
