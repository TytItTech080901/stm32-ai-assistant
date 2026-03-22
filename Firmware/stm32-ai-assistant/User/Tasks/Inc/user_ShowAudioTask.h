#ifndef __USER_SHOWAUDIOTASK_H__
#define __USER_SHOWAUDIOTASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "inmp441.h"
#include "fft_visualer.h"

typedef enum {
    SHOW_AUDIO_CMD_STREAM_BEGIN = 0,
    SHOW_AUDIO_CMD_PCM_HOP,
    SHOW_AUDIO_CMD_STREAM_END,
} ShowAudioCmdType_t;

typedef struct {
    ShowAudioCmdType_t type;
    uint16_t           sample_count;
    int16_t            pcm[PCM_BUF_LEN];
} ShowAudioMessage_t;

extern QueueHandle_t xShowAudioQueue;

void ShowAudioTask(void* argument);
void ShowAudio_StreamBegin(void);
void ShowAudio_StreamEnd(void);
void ShowAudio_SubmitPcmSamples(const int16_t* samples, uint16_t sample_count);

#ifdef __cplusplus
}
#endif

#endif
