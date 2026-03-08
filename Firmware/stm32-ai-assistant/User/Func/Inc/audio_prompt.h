/**
 ******************************************************************************
 * @file    audio_prompt.h
 * @brief   预置 PCM 提示音数据 (16kHz, 16-bit, mono)
 *
 *  当前为 0.5 秒 1kHz 正弦波提示音 (beep)
 *  如需替换为真实语音 "你好，有什么事":
 *    1. 录制/合成 WAV 文件 (16kHz, 16-bit, mono)
 *    2. 用 Python 转成 C 数组:
 *       import numpy as np
 *       import scipy.io.wavfile as wav
 *       sr, data = wav.read("prompt.wav")
 *       data = data.astype(np.int16)
 *       with open("audio_prompt.h", "w") as f:
 *           f.write("static const int16_t prompt_pcm[] = {\n")
 *           for i in range(0, len(data), 12):
 *               vals = ", ".join(str(x) for x in data[i:i+12])
 *               f.write("    " + vals + ",\n")
 *           f.write("};\n")
 *           f.write(f"#define PROMPT_PCM_LEN {len(data)}\n")
 *    3. 将生成的数组粘贴替换下方内容
 *
 *  注意: 16kHz × 2 秒 = 32000 samples = 64KB
 *        STM32F407 有 1MB Flash，足够存放多段语音
 ******************************************************************************
 */

#ifndef __AUDIO_PROMPT_H
#define __AUDIO_PROMPT_H

#include <stdint.h>

/*
 * 0.5 秒 1kHz 正弦波 beep (16kHz 采样率, 8000 samples)
 * 仅用于测试喇叭是否正常工作
 * 后续替换为真实 TTS 语音数据
 */

#define BEEP_FREQ_HZ 1000
#define BEEP_SAMPLE_RATE 16000
#define BEEP_DURATION_MS 500
#define BEEP_PCM_LEN (BEEP_SAMPLE_RATE * BEEP_DURATION_MS / 1000) /* 8000 */
#define BEEP_AMPLITUDE 16000

/* 编译期生成正弦表不方便，用运行时初始化 */
extern int16_t beep_pcm[BEEP_PCM_LEN];

/**
 * @brief  初始化 beep 正弦波 PCM 数据（调用一次即可）
 */
void AudioPrompt_Init(void);

#endif
