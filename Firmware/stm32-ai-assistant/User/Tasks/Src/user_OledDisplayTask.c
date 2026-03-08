/**
 ******************************************************************************
 * @file    user_OledDisplayTask.c
 * @brief   OLED display FreeRTOS task — 3-screen state machine
 *
 *  Screen 1 (Boot)   : AI Voice Asst. / STM32F407 / FreeRTOS / Booting...
 *  Screen 2 (Main)   : Status / Heap / Model — refreshes every 500ms
 *  Screen 3 (KWS)    : Detection result + confidence — holds 2s then back
 ******************************************************************************
 */

#include "user_OledDisplayTask.h"
#include "user_KwsInferTask.h"
#include "OLED.h"
#include "uart_cli.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

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

void OledDisplayTask(void* argument)
{
  OLED_KwsEvent_t evt;

  // if (g_dump_mode) {
  //   vTaskSuspend(NULL);
  // }

  OLED_Init();
  Screen_Boot();
  vTaskDelay(pdMS_TO_TICKS(2000));

  Screen_MainFull();

  for (;;) {
    if (xQueueReceive(xOledEventQueue, &evt, pdMS_TO_TICKS(500)) == pdTRUE) {
      uint8_t pct = (uint8_t)(evt.confidence * 100.0f);
      Screen_Kws(pct);
      vTaskDelay(pdMS_TO_TICKS(2000));

      while (xQueueReceive(xOledEventQueue, &evt, 0) == pdTRUE) {
      }

      Screen_MainFull();
    } else {
      Screen_MainRefresh();
    }
  }
}
