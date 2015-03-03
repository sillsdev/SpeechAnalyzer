#ifndef COMPLEX_H
#define COMPLEX_H

struct SComplexRectFloat {
    float real;
    float imag;
};

struct SComplexPolarFloat {
    float mag;
    float phase;
};

union UComplexFloat {
    SComplexRectFloat rect;
    SComplexPolarFloat polar;
};

SComplexRectFloat Cadd(SComplexRectFloat a, SComplexRectFloat b);
SComplexRectFloat Csub(SComplexRectFloat a, SComplexRectFloat b);
SComplexRectFloat Cmul(SComplexRectFloat a, SComplexRectFloat b);
SComplexRectFloat Complex(float re, float im);
SComplexRectFloat Conjg(SComplexRectFloat z);
SComplexRectFloat Cdiv(SComplexRectFloat a, SComplexRectFloat b);
float Cabs(SComplexRectFloat z);
SComplexRectFloat Csqrt(SComplexRectFloat z);
SComplexRectFloat RCmul(float x, SComplexRectFloat a);

#endif