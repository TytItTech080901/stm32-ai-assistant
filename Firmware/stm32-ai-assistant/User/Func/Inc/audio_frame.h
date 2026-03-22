#ifndef __AUDIO_FRAME_H
#define __AUDIO_FRAME_H

#include <stdint.h>
#include "arm_math.h"
#include "feature_extraction.h"
#include "fbank.h"

typedef struct {
    uint32_t  frame_len;
    uint32_t  hop_len;
    float32_t rolling_buffer[FRAME_LEN];
    float32_t frame_buffer[FFT_LEN];
} AudioFrameFrontendState;

typedef struct {
    arm_rfft_fast_instance_f32 rfft;
    SpectrogramTypeDef         spectrogram;
    float32_t                  window_buffer[FRAME_LEN];
    float32_t                  scratch_buffer[FFT_LEN];
} AudioSpectrumState;

void    AudioFrame_Init(AudioFrameFrontendState* state, uint32_t frame_len, uint32_t hop_len);
uint8_t AudioFrame_PushPcmHop(AudioFrameFrontendState* state, const int16_t* pcm,
                              uint32_t sample_count);
void    AudioFrame_GetFrame(const AudioFrameFrontendState* state, float32_t* out_frame,
                            uint32_t out_len);

int AudioSpectrum_Init(AudioSpectrumState* state, uint32_t sample_rate, uint32_t frame_len,
                       uint32_t fft_len, Spectrogram_TypeTypedef type);
const SpectrogramTypeDef* AudioSpectrum_GetConfig(const AudioSpectrumState* state);
void AudioSpectrum_Compute(AudioSpectrumState* state, float32_t* frame, float32_t* out_spectrum,
                           uint32_t out_len);

#endif
