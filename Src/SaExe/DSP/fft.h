#ifndef FFT_H

#include "dspTypes.h"

#define FFT_H

#define MAX_FFT_LENGTH  8192


enum {FORWARD=1, INVERSE=-1};

/* fft.h - C function declaration header file for FFT library */

extern "C" int32 rfft2f(float * pfarray, int32 n, int32 idir);
extern "C" int32 rfft2d(double * pfarray, int32 n, int32 idir);

#endif
