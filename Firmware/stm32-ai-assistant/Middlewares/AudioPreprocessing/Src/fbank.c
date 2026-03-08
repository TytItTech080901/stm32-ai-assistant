#include "feature_extraction.h"
#include "fbank.h"
#include <stdio.h>
#include "inmp441.h"

/* 计算一帧25ms对应的80维Fbank特征，
   通过滑动窗口实现每10ms计算一次 */

arm_rfft_fast_instance_f32 S_Rfft;
MelFilterTypeDef           S_MelFilter;
SpectrogramTypeDef         S_Spectr;
MelSpectrogramTypeDef      S_MelSpectr;
LogMelSpectrogramTypeDef   S_LogMelSpectr;

float32_t pRollingBuffer[FRAME_LEN];
float32_t pInFrame[FFT_LEN];
float32_t pOutColBuffer[NUM_MELS];
float32_t pWindowFuncBuffer[FRAME_LEN];
float32_t pSpectrScratchBuffer[FFT_LEN];
float32_t pMelFilterCoefs[NUM_MEL_COEFS];
uint32_t pMelFilterStartIndices[NUM_MELS];
uint32_t pMelFilterStopIndices[NUM_MELS];


void FbankFeature_Init(void)
{
	/* Init window function */
	if (Window_Init(pWindowFuncBuffer, FRAME_LEN, WINDOW_HANN) != 0)
		while(1);
	
	/* Init RFFT */
	arm_rfft_fast_init_f32(&S_Rfft, FFT_LEN);

	/* Init Spectrogram */
	S_Spectr.pRfft    = &S_Rfft;
	S_Spectr.Type     = SPECTRUM_TYPE_POWER;
	S_Spectr.pWindow  = pWindowFuncBuffer;
	S_Spectr.SampRate = SAMPLE_RATE;
	S_Spectr.FrameLen = FRAME_LEN;
	S_Spectr.FFTLen   = FFT_LEN;
	S_Spectr.pScratch = pSpectrScratchBuffer;

	/* Init Mel filter */
	S_MelFilter.pStartIndices = pMelFilterStartIndices;
	S_MelFilter.pStopIndices  = pMelFilterStopIndices;
	S_MelFilter.pCoefficients = pMelFilterCoefs;
	S_MelFilter.NumMels   = NUM_MELS;
	S_MelFilter.FFTLen    = FFT_LEN;
	S_MelFilter.SampRate  = SAMPLE_RATE;
	S_MelFilter.FMin      = 0.0;
	S_MelFilter.FMax      = S_MelFilter.SampRate / 2.0;
	S_MelFilter.Formula   = MEL_SLANEY;
	S_MelFilter.Normalize = 1;
	S_MelFilter.Mel2F     = 1;
	MelFilterbank_Init(&S_MelFilter);

	/* Init MelSpectrogram */
	S_MelSpectr.SpectrogramConf = &S_Spectr;
	S_MelSpectr.MelFilter       = &S_MelFilter;

	/* Init LogMelSpectrogram */
	S_LogMelSpectr.MelSpectrogramConf = &S_MelSpectr;
	/* wekws/TF models use natural log (ln); librosa/dB would be SCALE_DB */
	S_LogMelSpectr.LogFormula = LOGMELSPECTROGRAM_SCALE_LOG;
	S_LogMelSpectr.Ref = 1.0f;
	S_LogMelSpectr.TopdB = 80.0f;  /* not used by SCALE_LOG, kept for reference */
}

void AudioPreprocessing_Run(void)
{
	memmove(&pRollingBuffer[0], &pRollingBuffer[160], (FRAME_LEN - 160) * sizeof(float32_t));
	for (int i = 0; i < 160; i++) {
		pRollingBuffer[240 + i] = (float32_t)pcm_buf[i] / 32768.0f;
	}
	memcpy(pInFrame, pRollingBuffer, FRAME_LEN * sizeof(float32_t));
	memset(&pInFrame[FRAME_LEN], 0, (FFT_LEN - FRAME_LEN) * sizeof(float32_t));
//	memset(pInFrame, 0, sizeof(pInFrame));
  LogMelSpectrogramColumn(&S_LogMelSpectr, pInFrame, pOutColBuffer);
}
