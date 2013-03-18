#define COMPLEX_C
/* CAUTION: This is the ANSI C (only) version of the Numerical Recipes
utility file complex.c.  Do not confuse this file with the same-named
file complex.c that is supplied in the same subdirectory or archive
as the header file complex.h.  *That* file contains both ANSI and
traditional K&R versions, along with #ifdef macros to select the
correct version.  *This* file contains only ANSI C.               */

#include <math.h>
#include "complex.h"

COMPLEX_RECT_FLOAT Cadd(COMPLEX_RECT_FLOAT a, COMPLEX_RECT_FLOAT b) {
    COMPLEX_RECT_FLOAT c;
    c.real=a.real+b.real;
    c.imag=a.imag+b.imag;
    return c;
}

COMPLEX_RECT_FLOAT Csub(COMPLEX_RECT_FLOAT a, COMPLEX_RECT_FLOAT b) {
    COMPLEX_RECT_FLOAT c;
    c.real=a.real-b.real;
    c.imag=a.imag-b.imag;
    return c;
}


COMPLEX_RECT_FLOAT Cmul(COMPLEX_RECT_FLOAT a, COMPLEX_RECT_FLOAT b) {
    COMPLEX_RECT_FLOAT c;
    c.real=a.real*b.real-a.imag*b.imag;
    c.imag=a.imag*b.real+a.real*b.imag;
    return c;
}

COMPLEX_RECT_FLOAT Complex(float re, float im) {
    COMPLEX_RECT_FLOAT c;
    c.real=re;
    c.imag=im;
    return c;
}

COMPLEX_RECT_FLOAT Conjg(COMPLEX_RECT_FLOAT z) {
    COMPLEX_RECT_FLOAT c;
    c.real=z.real;
    c.imag = -z.imag;
    return c;
}

COMPLEX_RECT_FLOAT Cdiv(COMPLEX_RECT_FLOAT a, COMPLEX_RECT_FLOAT b) {
    COMPLEX_RECT_FLOAT c;
    float r,den;
    if (fabs(b.real) >= fabs(b.imag)) {
        r=b.imag/b.real;
        den=b.real+r*b.imag;
        c.real=(a.real+r*a.imag)/den;
        c.imag=(a.imag-r*a.real)/den;
    } else {
        r=b.real/b.imag;
        den=b.imag+r*b.real;
        c.real=(a.real*r+a.imag)/den;
        c.imag=(a.imag*r-a.real)/den;
    }
    return c;
}

float Cabs(COMPLEX_RECT_FLOAT z) {
    double x,y,ans,temp;
    x=fabs((double)z.real);
    y=fabs((double)z.imag);
    if (x == 0.0) {
        ans=y;
    } else if (y == 0.0) {
        ans=x;
    } else if (x > y) {
        temp=y/x;
        ans=x*sqrt(1.0+(double)temp*(double)temp);
    } else {
        temp=x/y;
        ans=y*sqrt(1.0+(double)temp*(double)temp);
    }
    return (float)ans;
}

COMPLEX_RECT_FLOAT Csqrt(COMPLEX_RECT_FLOAT z) {
    COMPLEX_RECT_FLOAT c;
    double x,y,w,r;
    if ((z.real == 0.0) && (z.imag == 0.0)) {
        c.real=0.0;
        c.imag=0.0;
        return c;
    } else {
        x=fabs((double)z.real);
        y=fabs((double)z.imag);
        if (x >= y) {
            r=y/x;
            w=sqrt(x)*sqrt(0.5*(1.0+sqrt(1.0+(double)r*(double)r)));
        } else {
            r=x/y;
            w=sqrt(y)*sqrt(0.5*(r+sqrt(1.0+(double)r*(double)r)));
        }
        if (z.real >= 0.0) {
            c.real=(float)w;
            c.imag=(float)((double)z.imag/(2.0*w));
        } else {
            c.imag=(z.imag >= 0) ? (float)w : (float)-w;
            c.real=(float)(z.imag/(2.0*c.imag));
        }
        return c;
    }
}

COMPLEX_RECT_FLOAT RCmul(float x, COMPLEX_RECT_FLOAT a) {
    COMPLEX_RECT_FLOAT c;
    c.real=x*a.real;
    c.imag=x*a.imag;
    return c;
}
/* (C) Copr. 1986-92 Numerical Recipes Software *!>p.#. */
