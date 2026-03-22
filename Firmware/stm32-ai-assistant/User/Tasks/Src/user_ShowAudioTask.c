#include "user_ShowAudioTask.h"
#include "user_OledDisplayTask.h"
#include <string.h>

QueueHandle_t xShowAudioQueue = NULL;

static void ShowAudio_SendSpectrumFrame(const uint8_t* bands, uint8_t band_count)
{
    OLED_Event_t evt = {.type = OLED_EVENT_SPECTRUM_FRAME, .band_count = band_count};
    memcpy(evt.bands, bands, band_count);
    xQueueSend(xOledEventQueue, &evt, 0);
}

void ShowAudio_StreamBegin(void)
{
    if (xShowAudioQueue == NULL) {
        return;
    }

    ShowAudioMessage_t msg = {.type = SHOW_AUDIO_CMD_STREAM_BEGIN, .sample_count = 0};
    xQueueSend(xShowAudioQueue, &msg, 0);
}

void ShowAudio_StreamEnd(void)
{
    if (xShowAudioQueue == NULL) {
        return;
    }

    ShowAudioMessage_t msg = {.type = SHOW_AUDIO_CMD_STREAM_END, .sample_count = 0};
    xQueueSend(xShowAudioQueue, &msg, 0);
}

void ShowAudio_SubmitPcmSamples(const int16_t* samples, uint16_t sample_count)
{
    if (xShowAudioQueue == NULL || sample_count != PCM_BUF_LEN) {
        return;
    }

    ShowAudioMessage_t msg = {.type = SHOW_AUDIO_CMD_PCM_HOP, .sample_count = sample_count};
    memcpy(msg.pcm, samples, sizeof(int16_t) * sample_count);
    xQueueSend(xShowAudioQueue, &msg, 0);
}

void ShowAudioTask(void* argument)
{
    ShowAudioMessage_t msg;
    uint8_t            bands[FFT_VISUAL_MAX_BANDS];

    (void)argument;
    configASSERT(xShowAudioQueue != NULL);
    FFTvisualer_Init();

    for (;;) {
        if (xQueueReceive(xShowAudioQueue, &msg, portMAX_DELAY) != pdPASS) {
            continue;
        }

        switch (msg.type) {
            case SHOW_AUDIO_CMD_STREAM_BEGIN:
                FFTvisualer_Reset();
                break;
            case SHOW_AUDIO_CMD_PCM_HOP:
                if (FFTvisualer_ProcessPcmHop(msg.pcm, msg.sample_count, bands,
                                              FFT_VISUAL_MAX_BANDS)) {
                    ShowAudio_SendSpectrumFrame(bands, FFT_VISUAL_MAX_BANDS);
                }
                break;
            case SHOW_AUDIO_CMD_STREAM_END: {
                OLED_Event_t evt = {.type = OLED_EVENT_SPECTRUM_STOP, .band_count = 0};
                xQueueSend(xOledEventQueue, &evt, 0);
                break;
            }
            default:
                break;
        }
    }
}
