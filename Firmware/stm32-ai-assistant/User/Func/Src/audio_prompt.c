/**
 ******************************************************************************
 * @file    audio_prompt.c
 * @brief   生成 beep 提示音 PCM 数据
 ******************************************************************************
 */

#include "audio_prompt.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

int16_t beep_pcm[BEEP_PCM_LEN];

void AudioPrompt_Init(void)
{
    for (uint32_t i = 0; i < BEEP_PCM_LEN; i++) {
        float t      = (float)i / BEEP_SAMPLE_RATE;
        float sample = sinf(2.0f * M_PI * BEEP_FREQ_HZ * t);

        /* 简单淡入淡出: 前后各 800 样本 (50ms) */
        float env = 1.0f;
        if (i < 800)
            env = (float)i / 800.0f;
        else if (i > BEEP_PCM_LEN - 800)
            env = (float)(BEEP_PCM_LEN - i) / 800.0f;

        beep_pcm[i] = (int16_t)(sample * env * BEEP_AMPLITUDE);
    }
}
