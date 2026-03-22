#include "feature_extraction.h"
#include "fbank.h"
#include "audio_frame.h"
#include <stdio.h>
#include "inmp441.h"

static AudioFrameFrontendState  S_AudioFrontend;
static AudioSpectrumState       S_AudioSpectrum;
static MelFilterTypeDef         S_MelFilter;
static MelSpectrogramTypeDef    S_MelSpectr;
static LogMelSpectrogramTypeDef S_LogMelSpectr;

float32_t        pInFrame[FFT_LEN];
float32_t        pOutColBuffer[NUM_MELS];
static float32_t pMelFilterCoefs[NUM_MEL_COEFS];
static uint32_t  pMelFilterStartIndices[NUM_MELS];
static uint32_t  pMelFilterStopIndices[NUM_MELS];

void FbankFeature_Init(void)
{
    AudioFrameFrontend_Init(&S_AudioFrontend, FRAME_LEN, HOP_LEN);
    if (AudioSpectrum_Init(&S_AudioSpectrum, SAMPLE_RATE, FRAME_LEN, FFT_LEN,
                           SPECTRUM_TYPE_POWER) != 0)
        while (1) {
        }

    S_MelFilter.pStartIndices = pMelFilterStartIndices;
    S_MelFilter.pStopIndices  = pMelFilterStopIndices;
    S_MelFilter.pCoefficients = pMelFilterCoefs;
    S_MelFilter.NumMels       = NUM_MELS;
    S_MelFilter.FFTLen        = FFT_LEN;
    S_MelFilter.SampRate      = SAMPLE_RATE;
    S_MelFilter.FMin          = 0.0;
    S_MelFilter.FMax          = S_MelFilter.SampRate / 2.0;
    S_MelFilter.Formula       = MEL_SLANEY;
    S_MelFilter.Normalize     = 1;
    S_MelFilter.Mel2F         = 1;
    MelFilterbank_Init(&S_MelFilter);

    S_MelSpectr.SpectrogramConf = (SpectrogramTypeDef*)AudioSpectrum_GetConfig(&S_AudioSpectrum);
    S_MelSpectr.MelFilter       = &S_MelFilter;

    S_LogMelSpectr.MelSpectrogramConf = &S_MelSpectr;
    S_LogMelSpectr.LogFormula         = LOGMELSPECTROGRAM_SCALE_LOG;
    S_LogMelSpectr.Ref                = 1.0f;
    S_LogMelSpectr.TopdB              = 80.0f;
}

void AudioPreprocessing_Run(void)
{
    AudioFrameFrontend_PushPcmHop(&S_AudioFrontend, pcm_buf, PCM_BUF_LEN);
    AudioFrameFrontend_GetFrame(&S_AudioFrontend, pInFrame, FFT_LEN);
    LogMelSpectrogramColumn(&S_LogMelSpectr, pInFrame, pOutColBuffer);
}
