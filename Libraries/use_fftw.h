#ifndef LAB_2_USE_FFTW_H
#define LAB_2_USE_FFTW_H
#include "use_ffmpeg.h"

uint8_t crossCorrelation(const Audio *a1, const Audio *a2, int *delta_samples);

#endif
