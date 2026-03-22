#ifndef __FFT_VISUALER_H
#define __FFT_VISUALER_H

#include <stdint.h>

#define FFT_VISUAL_MAX_BANDS 16U
#define FFT_VISUAL_GRAPH_HEIGHT 48U

void    FFTvisualer_Init(void);
void    FFTvisualer_Reset(void);
uint8_t FFTvisualer_ProcessPcmHop(const int16_t* pcm, uint32_t sample_count, uint8_t* out_bands,
                                  uint32_t band_count);

#endif
