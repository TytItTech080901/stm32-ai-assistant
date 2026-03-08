#ifndef __FBANK_H
#define __FBANK_H

#include <stdio.h>
#include "main.h"
#include "feature_extraction.h"
#include "inmp441.h"

#define SAMPLE_RATE  16000U /* Input signal sampling rate */
#define FFT_LEN       512U /* Number of FFT points. Must be greater or equal to FRAME_LEN */
#define FRAME_LEN   400U /* Window length and then padded with zeros to match FFT_LEN */
// #define HOP_LEN        512U /* Number of overlapping samples between successive frames */
#define HOP_LEN (I2S_DMA_BUF_LEN / 8) /* 1280 / 8 = 160, 10ms */

#define NUM_MELS        80U /* Number of mel bands */
#define NUM_MEL_COEFS  2048U /* Number of mel filter weights. Returned by MelFilterbank_Init */

#define SPEC_T 30
#define SPEC_F NUM_MELS

extern float32_t pOutColBuffer[NUM_MELS];

void FbankFeature_Init(void);
void AudioPreprocessing_Run(void);

#endif
