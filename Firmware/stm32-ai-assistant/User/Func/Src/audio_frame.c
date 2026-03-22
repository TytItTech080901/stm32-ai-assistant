/**
 ******************************************************************************
 * @file    audioframe.c
 * @author  tytxxc
 * @brief   generate audio frame and init spectrum
 ******************************************************************************
 */
#include "audio_frame.h"
#include <string.h>

void AudioFrame_Init(AudioFrameFrontendState* state, uint32_t frame_len, uint32_t hop_len)
{
    memset(state, 0, sizeof(*state));
    state->frame_len = frame_len;
    state->hop_len   = hop_len;
}

uint8_t AudioFrame_PushPcmHop(AudioFrameFrontendState* state, const int16_t* pcm,
                                      uint32_t sample_count)
{
    if (sample_count == 0 || sample_count > state->frame_len) {
        return 0;
    }

    memmove(&state->rolling_buffer[0], &state->rolling_buffer[sample_count],
            (state->frame_len - sample_count) * sizeof(float32_t));
    for (uint32_t i = 0; i < sample_count; ++i) {
        state->rolling_buffer[state->frame_len - sample_count + i] = (float32_t)pcm[i] / 32768.0f;
    }
    memcpy(state->frame_buffer, state->rolling_buffer, state->frame_len * sizeof(float32_t));
    memset(&state->frame_buffer[state->frame_len], 0,
           (FFT_LEN - state->frame_len) * sizeof(float32_t));

    return 1;
}

void AudioFrame_GetFrame(const AudioFrameFrontendState* state, float32_t* out_frame,
                                 uint32_t out_len)
{
    uint32_t copy_len = out_len > FFT_LEN ? FFT_LEN : out_len;
    memcpy(out_frame, state->frame_buffer, copy_len * sizeof(float32_t));
}

int AudioSpectrum_Init(AudioSpectrumState* state, uint32_t sample_rate, uint32_t frame_len,
                       uint32_t fft_len, Spectrogram_TypeTypedef type)
{
    memset(state, 0, sizeof(*state));

    if (Window_Init(state->window_buffer, frame_len, WINDOW_HANN) != 0) {
        return -1;
    }

    if (arm_rfft_fast_init_f32(&state->rfft, fft_len) != ARM_MATH_SUCCESS) {
        return -1;
    }

    state->spectrogram.pRfft    = &state->rfft;
    state->spectrogram.Type     = type;
    state->spectrogram.pWindow  = state->window_buffer;
    state->spectrogram.SampRate = sample_rate;
    state->spectrogram.FrameLen = frame_len;
    state->spectrogram.FFTLen   = fft_len;
    state->spectrogram.pScratch = state->scratch_buffer;

    return 0;
}

const SpectrogramTypeDef* AudioSpectrum_GetConfig(const AudioSpectrumState* state)
{
    return &state->spectrogram;
}

void AudioSpectrum_Compute(AudioSpectrumState* state, float32_t* frame, float32_t* out_spectrum,
                           uint32_t out_len)
{
    (void)out_len;
    SpectrogramColumn(&state->spectrogram, frame, out_spectrum);
}
