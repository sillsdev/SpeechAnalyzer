/////////////////////////////////////////////////////////////////////////////
// fft.cpp
// Implementation of the FFT algorithm
// Author: Steve MacLean (SDM)
// copyright 1999 JAARS Inc. SIL
//
// Revision History
//   1.5Test11.2
//        SDM Original
/////////////////////////////////////////////////////////////////////////////

#define FFT

#include <math.h>
#include <limits.h>
#include "fft.h"

typedef uint32 uint32;

#define inline __forceinline

// w = exp(j*2*pi()/N) Nth root of UNITY

// w = cos(2*pi()/N) + j*sin(2*pi()/N)

// A.m = Sum.(n=0).To.(N-1)
//       {
//         a.n*pow(w,m*n)
//       }


// a.n <-> A.m  DFS pair



class CComplexFloat {
public:
    float real;
    float imaginary;

    inline CComplexFloat(const float & in_real = 0, const float & in_imaginary = 0) {
        real = in_real;
        imaginary = in_imaginary;
    }

    inline CComplexFloat & operator=(const CComplexFloat & other) {
        real = other.real;
        imaginary = other.imaginary;
        return *this;
    }

    inline const CComplexFloat & operator*=(const CComplexFloat & other) {
        float realnew = real*other.real-imaginary*other.imaginary;
        imaginary = imaginary*other.real+real*other.imaginary;
        real = realnew;
        return *this;
    }

    inline const CComplexFloat & operator+=(const CComplexFloat & other) {
        real = real+other.real;
        imaginary = imaginary+other.imaginary;
        return *this;
    }

    inline const CComplexFloat & operator-=(const CComplexFloat & other) {
        real = real-other.real;
        imaginary = imaginary-other.imaginary;
        return *this;
    }
    inline const CComplexFloat  operator*(const CComplexFloat & other) const {
        return CComplexFloat(*this) *= other;
    }
    inline const CComplexFloat  operator+(const CComplexFloat & other) const {
        return CComplexFloat(*this) += other;
    }
    inline const CComplexFloat  operator-(const CComplexFloat & other) const {
        return CComplexFloat(*this) -= other;
    }
    inline const CComplexFloat  conjugate() const {
        return CComplexFloat(real,-imaginary);
    }
    inline const CComplexFloat  undo_j() const {
        return CComplexFloat(imaginary,-real);
    }
    inline const CComplexFloat  redo_j() const {
        return CComplexFloat(-imaginary,real);
    }
};

inline static const float pi(void) {
    return (float) 3.1415926535897932384626433832795;
}

inline static const CComplexFloat w(int32 m, int32 N) {
    return CComplexFloat((float)cos(2*pi()*m/N),(float)sin(2*pi()*m/N));
}

inline static void butterfly(CComplexFloat & top, CComplexFloat & bottom, const CComplexFloat & scale) {
    CComplexFloat scaleBottom = bottom*scale;
    bottom = top - scaleBottom;
    top += scaleBottom;
}

#define binary6(a,b,c,d,e,f) (((((a*2 + b)*2 + c)*2 + d)*2 + e)*2 + f)

static const uint32 reverse6bits[] = {
    binary6(0,0,0,0,0,0),
    binary6(1,0,0,0,0,0),
    binary6(0,1,0,0,0,0),
    binary6(1,1,0,0,0,0),
    binary6(0,0,1,0,0,0),
    binary6(1,0,1,0,0,0),
    binary6(0,1,1,0,0,0),
    binary6(1,1,1,0,0,0),
    binary6(0,0,0,1,0,0),
    binary6(1,0,0,1,0,0),
    binary6(0,1,0,1,0,0),
    binary6(1,1,0,1,0,0),
    binary6(0,0,1,1,0,0),
    binary6(1,0,1,1,0,0),
    binary6(0,1,1,1,0,0),
    binary6(1,1,1,1,0,0),
    binary6(0,0,0,0,1,0),
    binary6(1,0,0,0,1,0),
    binary6(0,1,0,0,1,0),
    binary6(1,1,0,0,1,0),
    binary6(0,0,1,0,1,0),
    binary6(1,0,1,0,1,0),
    binary6(0,1,1,0,1,0),
    binary6(1,1,1,0,1,0),
    binary6(0,0,0,1,1,0),
    binary6(1,0,0,1,1,0),
    binary6(0,1,0,1,1,0),
    binary6(1,1,0,1,1,0),
    binary6(0,0,1,1,1,0),
    binary6(1,0,1,1,1,0),
    binary6(0,1,1,1,1,0),
    binary6(1,1,1,1,1,0),
    binary6(0,0,0,0,0,1),
    binary6(1,0,0,0,0,1),
    binary6(0,1,0,0,0,1),
    binary6(1,1,0,0,0,1),
    binary6(0,0,1,0,0,1),
    binary6(1,0,1,0,0,1),
    binary6(0,1,1,0,0,1),
    binary6(1,1,1,0,0,1),
    binary6(0,0,0,1,0,1),
    binary6(1,0,0,1,0,1),
    binary6(0,1,0,1,0,1),
    binary6(1,1,0,1,0,1),
    binary6(0,0,1,1,0,1),
    binary6(1,0,1,1,0,1),
    binary6(0,1,1,1,0,1),
    binary6(1,1,1,1,0,1),
    binary6(0,0,0,0,1,1),
    binary6(1,0,0,0,1,1),
    binary6(0,1,0,0,1,1),
    binary6(1,1,0,0,1,1),
    binary6(0,0,1,0,1,1),
    binary6(1,0,1,0,1,1),
    binary6(0,1,1,0,1,1),
    binary6(1,1,1,0,1,1),
    binary6(0,0,0,1,1,1),
    binary6(1,0,0,1,1,1),
    binary6(0,1,0,1,1,1),
    binary6(1,1,0,1,1,1),
    binary6(0,0,1,1,1,1),
    binary6(1,0,1,1,1,1),
    binary6(0,1,1,1,1,1),
    binary6(1,1,1,1,1,1),
};

inline static uint32 reverseBits(uint32 value, uint32 maxValue) {
    uint32 returnValue = 0;
    const int mask = 0x3f;  // 6bits

    uint32 test;
    for (test = 1; test < maxValue; test = test << 6) {
        returnValue = (returnValue << 6) + reverse6bits[value&mask];
        value = value >> 6;
    }
    return returnValue/(test/maxValue);
}


#pragma auto_inline(off)

void swapOrder(CComplexFloat * const complexData, const int32 n) {
    // swap order
    for (uint32 swap = 0; swap < (uint32) n; swap++) {
        uint32 reverse = reverseBits(swap, n);
        if (swap > reverse) { // avoid swapping twice
            CComplexFloat temp = complexData[swap];
            complexData[swap] = complexData[reverse];
            complexData[reverse] = temp;
        }
    }
}
#pragma auto_inline(on)


static void cfft2f(CComplexFloat * const complexData, const int32 n, const int32 idir) {
    // swap order
    swapOrder(complexData, n);

    if (idir == FORWARD) {
        // forward DFT
        for (int32 pass = 1; (1 << pass) <= n; pass++) {
            int32 mPasses = 1 << (pass-1); // 1,2,4,8,16....

            CComplexFloat scaleFactor = w(-1,2*mPasses);
            CComplexFloat scale(1, 0);
            for (int32 m=0; m<mPasses; m++) {
                for (int32 top=m; top < n; top += 2*mPasses) {
                    int32 bottom = top + mPasses;

                    butterfly(complexData[top],complexData[bottom],scale);
                }
                scale *= scaleFactor;
            }
        }
    } else {
        // inverse DFT transform

        // a.n = (1/N)*Sum.(m=0).To.(N-1)
        //             {
        //               A.m*pow(w,-m*n)
        //             }
        for (int32 pass = 1; (1 << pass) <= n; pass++) {
            int32 mPasses = 1 << (pass-1); // 1,2,4,8,16....

            CComplexFloat scaleFactor = w(1,2*mPasses);
            CComplexFloat scale(1, 0);
            for (int32 m=0; m<mPasses; m++) {
                for (int32 top=m; top < n; top += 2*mPasses) {
                    int32 bottom = top + mPasses;

                    butterfly(complexData[top],complexData[bottom],scale);
                }
                scale *= scaleFactor;
            }
        }
        for (int32 i=0; i<n; i++) {
            complexData[i] = complexData[i] * CComplexFloat(1.0f/float(n));
        }
    }
}


// perform a real fft
// a complex fft is perfomed.
// the result is broken into cos & sin coefficients up
// to the nyquist frequency.
// the cos nyquist term is stored in the sin DC term since this is
// always 0 for real functions.
// the fft takes 2N trig lookups and N/2*log.base2 N multiplies
// the cos transform takes N^2 multiplies and N^2 lookups
// for large N this is significant
// the sin transform takes an equivalent number of operations
extern "C" int32 slowrfft2f(float * pfarray, int32 n, int32 idir) {
    CComplexFloat * data = new CComplexFloat[n];
    float * floatData = (float *) data;

    if (idir == FORWARD) {
        // copy real data to complex array
        for (int32 i=0; i<n; i++) {
            data[i] = pfarray[i];
        }
        cfft2f(data, n, idir);
        // copy complex coeffiecients to real cos, sin coeeficients
        for (int32 i=0; i<n; i++) {
            pfarray[i] = floatData[i] * 2.0f;
        }
        // special case pfarray[1] represents the cosine term at the nyquist frequency
        pfarray[1]=floatData[n]*2.0f;
    } else {
        // copy real data to complex array
        for (int32 i=0; i<n; i++) {
            floatData[i] = pfarray[i]/2.0f;
        }
        floatData[1] = 0; // the imaginary DC term is 0
        floatData[n] = pfarray[1]/2.0f;  // this is the nyquist term
        floatData[n+1] = 0; // the imaginary nyquist term is 0
        // the remaining terms are the complex conjugate of the previous terms
        for (int32 i=1; i<n/2; i++) {
            data[n-i] = data[i].conjugate();
        }
        cfft2f(data, n, idir);
        // copy complex coeffiecients to real cos, sin coeeficients
        for (int32 i=0; i<n; i++) {
            pfarray[i] = data[i].real;
        }
    }
    delete data;
    return 0;
}

extern "C" int32 rfft2f(float * pfarray, int32 n, int32 idir) {
    int32 halfN = n/2;
    if (idir == FORWARD) {
        CComplexFloat Pm,Qm,Wm;
        CComplexFloat * data = (CComplexFloat *) pfarray;
        CComplexFloat WmScale = w(-1,n);
        float a,b;
        // treat real data as two half length arrays
        // interleaved.  Functionally equivalent to
        // multiplying odd data by complex j
        // process using complex fft
        cfft2f((CComplexFloat *)pfarray, halfN, idir);
        // extract terms Pm Qm corresponding to each half real array
        // calculate final term using Am = Pm + Qm*W2n^m
        // remember Pm = P-m*, Qm = Q-m*
        a = pfarray[0] + pfarray[1];
        b = pfarray[0] - pfarray[1];
        pfarray[0] = 2*a;
        pfarray[1] = 2*b;

        Wm = WmScale;
        for (int32 i=1; i<=halfN/2; i++) {
            Pm = data[i] + data[halfN-i].conjugate();// should be divided by 2 then multiplied in following operation
            Qm = (data[i] - data[halfN-i].conjugate()).undo_j();//  ""
            data[i] = Pm + Qm*Wm;
            data[halfN-i] = (Pm - Qm*Wm).conjugate();
            Wm *= WmScale;
        }
    } else {
        CComplexFloat Pm,Qm,Wm;
        CComplexFloat * data = (CComplexFloat *) pfarray;
        CComplexFloat WmScale = w(1,n);
        float a,b;
        // see ablove inverse operation
        a = pfarray[0]/2;
        b = pfarray[1]/2;
        pfarray[0] = (a+b)/2; // solve simultaneous equations
        pfarray[1] = (a-b)/2;
        Wm = WmScale;
        for (int32 i=1; i<=halfN/2; i++) {
            Pm = (data[i] + data[halfN-i].conjugate())*0.5;
            Qm = (data[i] - data[halfN-i].conjugate())*Wm*0.5; // Wm is inverse of Wm in forward operation
            data[i] = (Pm + Qm.redo_j())*0.5;
            data[halfN - i] = (Pm - Qm.redo_j()).conjugate()*0.5;
            Wm *= WmScale;
        }
        // treat real data as two half length arrays
        cfft2f((CComplexFloat *)pfarray, n/2, idir);
    }
    return 0;
}
