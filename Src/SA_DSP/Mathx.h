#ifndef MATHX_H
#define MATHX_H
/*-----------------------------------------------------------------------*\
|                                                                         |
|                                MATHX.H                                  |
|                            Math Extensions                              |
|                                                                         |
\*-----------------------------------------------------------------------*/
#include <float.h>
#include "error.h"
#include "complex.h"

// Constant definitions
#ifndef PI
#define PI           3.14159265358979323846264338327950288419716939937511
#endif
#define LOG_2        0.30102999566398119521373889472449302678818988146211  // log 2 base 10
#define FLT_MAX_NEG  (-FLT_MAX)
#define FLT_MIN_NEG  (-FLT_MIN)

// Type definitions
struct SStatistic {
    double Mean;
    double StdDev;
};

// Miscellaneous definitions
enum ECOMPARISON {LESS = -1, EQUAL = 0, GREATER = 1};

// Function prototypes
float BessI0(float x);
double Round(double x);
bool IsSingleBitOn(unsigned short data);


#endif
