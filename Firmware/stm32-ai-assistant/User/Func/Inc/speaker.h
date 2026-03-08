#ifndef __SPEAKER_H
#define __SPEAKER_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

/**
 * 播放请求: 指定 W25Q16 中的起始地址和采样点数
 *
 * 音频格式: 16kHz, 16-bit, mono
 * flash_addr 是 W25Q16 芯片内部地址 (0x000000 ~ 0x1FFFFF)
 * samples    采样点数 (不是字节数, 字节数 = samples × 2)
 */
typedef struct
{
  uint32_t flash_addr;
  uint32_t samples;
} Speaker_Request_t;

extern QueueHandle_t xSpeakerQueue;

/**
 * @brief  初始化 Speaker 驱动（创建内部信号量）
 */
void Speaker_Init(void);

/**
 * @brief  向播放任务发送播放请求（非阻塞）
 * @param  flash_addr  W25Q16 内部地址
 * @param  samples     采样点数
 */
void Speaker_PlayFromFlash(uint32_t flash_addr, uint32_t samples);

/**
 * @brief  从 Flash 流式播放一段音频（乒乓双缓冲，阻塞直到播完）
 * @param  flash_addr  W25Q16 内部地址
 * @param  total_samples 总采样点数
 */
void Speaker_StreamPlay(uint32_t flash_addr, uint32_t total_samples);

#endif
