/**
 ******************************************************************************
 * @file    user_KwsInferTask.c
 * @brief   KWS inference FreeRTOS task — X-CUBE-AI model execution
 *
 *  数据流:
 *   AudioFbankTask  →  xFbankQueue  →  本任务每帧接收 80 维 Fbank
 *     → ai_network_run()  → 输出 2 个概率: [keyword, non-keyword]
 *     → 滑窗平均 + 冷却时间
 *     → 关键词命中 → 发 IdleBreak 消息唤醒 LCD
 ******************************************************************************
 */

#include "user_KwsInferTask.h"
#include "user_AudioFbankTask.h"
#include "user_OledDisplayTask.h"
#include "speaker.h"
#include "network.h"
#include "network_data_params.h"
#include "ai_platform.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>
#include <stdio.h>
#include "main.h"
#include <uart_cli.h>

#define FBANK_BINS 80
#define SMOOTH_WIN 15      // 滑窗平均窗口 (帧数), 15 × 10ms = 150ms */
#define KWS_THRESHOLD 0.5f // 判断阈值
#define COOLDOWN_FRAMES 80 // 80帧冷却

/* 每隔多少帧打印一次概率 (100 帧 = 1 秒); 设 0 关闭打印 */
#define KWS_DEBUG_PRINT_INTERVAL 100U

// W25Q16
#define PROMPT_FLASH_ADDR 0x000000
#define PROMPT_SAMPLES 24000

static ai_handle network_handle = AI_HANDLE_NULL;

AI_ALIGNED(4) static ai_float ai_in_data[AI_NETWORK_IN_1_SIZE];
AI_ALIGNED(4) static ai_float ai_out_data[AI_NETWORK_OUT_1_SIZE];
AI_ALIGNED(4) static ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];

TaskHandle_t     xKwsInferTaskHandle = NULL;
volatile uint8_t kws_model_ready     = 0;

static float    prob_ring[SMOOTH_WIN];
static uint8_t  ring_idx        = 0;
static uint8_t  ring_filled     = 0;
static uint16_t cooldown_cnt    = 0;
static uint32_t debug_frame_cnt = 0;

static int AI_Init(void);
static int AI_Run(const float* pIn, float* pOut);

void KwsInferTask(void* argument)
{
    // if (g_dump_mode) {
    //   vTaskSuspend(NULL);
    // }
    float feat[FBANK_BINS];

    if (AI_Init() != 0) {
        vTaskSuspend(NULL);
    }
    kws_model_ready = 1;
    memset(prob_ring, 0, sizeof(prob_ring));

    for (;;) {
        if (xQueueReceive(xFbankQueue, feat, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        memcpy(ai_in_data, feat, sizeof(float) * FBANK_BINS);

        if (AI_Run(ai_in_data, ai_out_data) != 0) {
            continue;
        }

        float kw_prob = ai_out_data[0];

#if KWS_DEBUG_PRINT_INTERVAL > 0
        debug_frame_cnt++;
        if (debug_frame_cnt >= KWS_DEBUG_PRINT_INTERVAL) {
            debug_frame_cnt = 0;
            printf("[KWS] raw: out[0]=%.3f out[1]=%.3f avg=%.3f\r\n", (double)ai_out_data[0],
                   (double)ai_out_data[1], (double)kw_prob);
        }
#endif

        prob_ring[ring_idx] = kw_prob;
        ring_idx++;
        if (ring_idx >= SMOOTH_WIN) {
            ring_idx    = 0;
            ring_filled = 1;
        }

        uint8_t count = ring_filled ? SMOOTH_WIN : ring_idx;
        float   sum   = 0.0f;
        for (uint8_t i = 0; i < count; i++) {
            sum += prob_ring[i];
        }
        float avg_prob = (count > 0) ? (sum / count) : 0.0f;

        if (cooldown_cnt > 0) {
            cooldown_cnt--;
            continue;
        }

        // 判定
        if (avg_prob > KWS_THRESHOLD) {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_5);
            printf("[KWS] Keyword detected! avg_prob=%.2f\r\n", avg_prob);

            OLED_KwsEvent_t oled_evt = {.confidence = avg_prob};
            xQueueSend(xOledEventQueue, &oled_evt, 0);

            Speaker_PlayFromFlash(PROMPT_FLASH_ADDR, PROMPT_SAMPLES);

            cooldown_cnt = COOLDOWN_FRAMES;
            memset(prob_ring, 0, sizeof(prob_ring));
            ring_idx    = 0;
            ring_filled = 0;
        }
    }
}

static int AI_Init(void)
{
    ai_error err;

    const ai_handle acts[] = {activations};
    err                    = ai_network_create_and_init(&network_handle, acts, NULL);
    if (err.type != AI_ERROR_NONE) {
        return -1;
    }
    return 0;
}

static int AI_Run(const float* pIn, float* pOut)
{
    ai_i32 n_batch;

    ai_buffer* ai_input  = ai_network_inputs_get(network_handle, NULL);
    ai_buffer* ai_output = ai_network_outputs_get(network_handle, NULL);

    ai_input[0].data  = AI_HANDLE_PTR(pIn);
    ai_output[0].data = AI_HANDLE_PTR(pOut);

    n_batch = ai_network_run(network_handle, ai_input, ai_output);
    if (n_batch != 1) {
        return -1;
    }
    return 0;
}
