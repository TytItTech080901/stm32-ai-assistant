#include "fft_visualer.h"
#include "audio_frontend.h"
#include "fbank.h"
#include <math.h>
#include <string.h>

static AudioFrameFrontendState s_fft_frontend;
static AudioSpectrumState      s_fft_spectrum;
static float32_t               s_fft_frame[FFT_LEN];
static float32_t               s_fft_power[FFT_LEN / 2 + 1];
static float32_t               s_smoothed_bands[FFT_VISUAL_MAX_BANDS];

void FFTvisualer_Init(void)
{
    AudioFrameFrontend_Init(&s_fft_frontend, FRAME_LEN, HOP_LEN);
    if (AudioSpectrum_Init(&s_fft_spectrum, SAMPLE_RATE, FRAME_LEN, FFT_LEN, SPECTRUM_TYPE_POWER) !=
        0) {
        while (1) {
        }
    }
    FFTvisualer_Reset();
}

void FFTvisualer_Reset(void)
{
    AudioFrameFrontend_Init(&s_fft_frontend, FRAME_LEN, HOP_LEN);
    memset(s_smoothed_bands, 0, sizeof(s_smoothed_bands));
}

uint8_t FFTvisualer_ProcessPcmHop(const int16_t* pcm, uint32_t sample_count, uint8_t* out_bands,
                                  uint32_t band_count)
{
    if (band_count == 0 || band_count > FFT_VISUAL_MAX_BANDS) {
        return 0;
    }

    if (AudioFrameFrontend_PushPcmHop(&s_fft_frontend, pcm, sample_count) == 0) {
        return 0;
    }

    AudioFrameFrontend_GetFrame(&s_fft_frontend, s_fft_frame, FFT_LEN);
    AudioSpectrum_Compute(&s_fft_spectrum, s_fft_frame, s_fft_power, FFT_LEN / 2 + 1);

    for (uint32_t band = 0; band < band_count; ++band) {
        uint32_t  start_bin = 1U + ((FFT_LEN / 2 - 1U) * band) / band_count;
        uint32_t  stop_bin  = 1U + ((FFT_LEN / 2 - 1U) * (band + 1U)) / band_count;
        float32_t sum       = 0.0f;
        uint32_t  count     = 0;

        if (stop_bin <= start_bin) {
            stop_bin = start_bin + 1U;
        }

        for (uint32_t bin = start_bin; bin < stop_bin; ++bin) {
            sum += s_fft_power[bin];
            ++count;
        }

        float32_t avg_power = (count > 0U) ? (sum / (float32_t)count) : 0.0f;
        float32_t db_level  = 10.0f * log10f(avg_power + 1.0e-12f) + 90.0f;
        if (db_level < 0.0f) {
            db_level = 0.0f;
        } else if (db_level > 90.0f) {
            db_level = 90.0f;
        }

        s_smoothed_bands[band] = (s_smoothed_bands[band] * 0.65f) + (db_level * 0.35f);
        out_bands[band] =
            (uint8_t)((s_smoothed_bands[band] / 90.0f) * (float32_t)FFT_VISUAL_GRAPH_HEIGHT);
    }

    return 1;
}
