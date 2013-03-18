/**************************************************************************\
* MODULE:       CURVEFIT.CPP                                               *
* COPYRIGHT:    SUMMER INSTITUTE OF LINGUISTICS, 2000                      *
* AUTHOR:       ALEC K. EPTING                                             *
*                                                                          *
*                                                                          *
* DESCRIPTION:                                                             *
*                                                                          *
* Class to fit best possible curve of a specified order through a given    *
* set of data points.                                                      *
*                                                                          *
*                                                                          *
* TYPICAL APPLICATION:                                                     *
*                                                                          *
* For speech, to find the equation for a polynomial curve that passes      *
* through a set of sample data.                                            *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER VARIABLES:                                                 *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER FUNCTIONS:                                                 *
*                                                                          *
* Note: all arguments are passed using the C calling convention.           *
*                                                                          *
* Copyright                                                                *
*   Function to retrieve pointer to copyright notice                       *
*   Arguments:    None                                                     *
*   Return value: pointer to null-terminated string containing             *
*                 copyright notice                                         *
*                                                                          *
* Version                                                                  *
*   Function to return version number of class                             *
*   Arguments:     None                                                    *
*   Return value:  Version number in 4-byte floating point format          *
*                                                                          *
* CreateObject                                                             *
*   Function to validate parameters, construct an object, and initialize   *
*   it.                                                                    *
*                                                                          *
*   Arguments:                                                             *
*     ppCurveFitter       address to contain pointer to curve fitting      *
*                         object                                           *
*     CurveType           curve type from enumerated list                  *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                      no fatal errors                            *
*     INVALID_CURVE_TYPE        fatal error: invalid curve type specified  *
*     INVALID_PARM_PTR          fatal error: invalid address specified for *
*                                            pointer to curve fitter       *
*     OUT_OF_MEMORY             fatal error: insufficient memory for       *
*                                            buffers                       *
*                                                                          *
* FitCurve                                                                 *
*   Function to fit data points to curve                                   *
*                                                                          *
*   Arguments:                                                             *
*     pData               pointer to data points, each of which is assumed *
*                         to be of data type float                         *
*     dwDataLength        number of samples in data buffer                 *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     INVALID_PARM_PTR    fatal error:  invalid address for curve          *
*                                       coefficient pointer                *
*     INVALID_SIG_PTR     fatal error:  null pointer to data               *
*     INVALID_SIG_LEN     fatal error:  zero length                        *
*                                                                          *
*                                                                          *
* GetValue                                                                 *
*   Function to retrive value of point on curve at a specified position    *
*                                                                          *
*   Arguments:                                                             *
*     pValue              location to contain y-value, of type float       *
*     x                   x-coordinate, of type float, assuming 0.         *
*                         corresponds to first data point passed via       *
*                         FitCurve, 1. to second, etc.                     *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     INVALID_PARM_PTR    fatal error:  invalid address for curve          *
*                                       coefficient pointer                *
*     INVALID_SIG_PTR     fatal error:  null pointer to data               *
*     INVALID_SIG_LEN     fatal error:  zero length                        *
*                                                                          *
*                                                                          *
* CalcFirstDerivative                                                      *
*   Function to retrieve first derivative of the curve at a specified      *
*   position.                                                              *
*                                                                          *
*   Arguments:                                                             *
*     pFirstDeriv         location to contain first derivative, type float *
*     x                   pointer to x-coordinate, of type float,          *
*                         assumes x-coordinate is the offset from          *
*                         the location of the first data point passed to   *
*                         FitCurve                                         *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     INVALID_PARM_PTR    fatal error:  invalid address for first          *
*                                       derivative or x-coordinate         *
*     INFINITY            warning: derivative is infinite                  *
*                                                                          *
* CalcFirstDerivative                                                      *
*   Function to retrieve first derivative of the curve at a specified      *
*   position.                                                              *
*                                                                          *
*   Arguments:                                                             *
*     pFirstDeriv         location to contain first derivative, type float *
*     x                   x-coordinate, of type float, assuming 0.         *
*                         corresponds to first data point passed via       *
*                         FitCurve, 1. to second, etc.                     *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     INVALID_PARM_PTR    fatal error:  invalid address for first          *
*                                       derivative                         *
*     INFINITY            warning: derivative is infinite                  *
*                                                                          *
* FindFirstDerivativeRoot                                                  *
*   Function to find x-coordinate where first derivative of curve is 0,    *
*   such as maximum, minimum, or inflection point.                         *
*                                                                          *
*   Arguments:                                                             *
*     pFirstDerivRoot     location to contain first derivative root,       *
*                         offset of type float, assuming first data        *
*                         point passed to FitCurve is at offset 0.         *
*     nRootIndex          index of root to return, starting with the       *
*                         lowest value.                                    *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     INVALID_PARM_PTR    fatal error:  invalid address for first          *
*                                       derivative root                    *
*     NO_ROOT_FOUND       warning:  curve has no first derivative root     *
*                                                                          *
*                                                                          *
* CalcSecondDerivative                                                     *
*   Function to retrieve second derivative of the curve at a specified     *
*   position.                                                              *
*                                                                          *
*   Arguments:                                                             *
*     pSecondDeriv        location to contain second derivative, float     *
*     x                   pointer to x-coordinate, of type float,          *
*                         assumes x-coordinate is the offset from          *
*                         the location of the first data point passed to   *
*                         FitCurve                                         *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     INVALID_PARM_PTR    fatal error:  invalid address for second         *
*                                       derivative or x-coordinate         *
*     INFINITY            warning: derivative is infinite                  *
*                                                                          *
*                                                                          *
* CalcSecondDerivative                                                     *
*   Function to retrieve seoond derivative of the curve at a specified     *
*   position.                                                              *
*                                                                          *
*   Arguments:                                                             *
*     pSecondDeriv        location to contain second derivative, float     *
*     x                   x-coordinate, of type float, assuming 0.         *
*                         corresponds to first data point passed via       *
*                         FitCurve, 1. to second, etc.                     *
*                                                                          *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     INVALID_PARM_PTR    fatal error:  invalid address for second         *
*                                       derivative or x-coordinate         *
*     INFINITY            warning: derivative is infinite                  *
*                                                                          *
*                                                                          *
*                                                                          *
* TYPICAL CALLING SEQUENCE:                                                *
*          :                                                               *
*   #include "ASAPDSP.h"                                                   *
*          :                                                               *
*   CCurveFitting *pParabola;                                              *
*   dspError_t dspError_t = CCurveFitting::CreateObject(&pParabola, PARABOLIC);          *
*   if (dspError_t) return(dspError_t);                                                  *
*                                                                          *
*   dspError_t dspError_t = pParabola->FitCurve(y, 3);                                   *
*   if (dspError_t) return(dspError_t);                                                  *
*   float FirstDerivRoot;                                                  *
*   dspError_t = pParabola->FindFirstDerivRoot(&FirstDerivRoot, 1);               *
*   if (dspError_t) return(dspError_t);                                                  *
*   dspError_t = pParabola->GetValue(&Peak, FirstDerivRoot);                      *
*   if (dspError_t) return(dspError_t);                                                  *
*   float SecondDeriv;                                                     *
*   dspError_t = pParabola->CalcSecondDerivative(&SecondDeriv, FirstDerivRoot);   *
*   if (dspError_t) return(dspError_t);                                                  *
*   if (SecondDeriv >= 0) Peak = (float)UNDEFINED_DATA;                    *
*   return;                                                                *
*                                                                          *
*                                                                          *
* TEST DRIVER:                                                             *
*                                                                          *
*                                                                          *
* COMPILER:         Microsoft Visual C++ version 1.52                      *
* COMPILE OPTIONS:  /nologo /f- /G3 /W3 /Zi /AL /YX /Od /D "_DEBUG" /I     *
*                   "c:\msvc\asapdsp" /FR /GA /Fd"SA.PDB" /Fp"SA.PCH"      *
* LINK OPTIONS:     /NOLOGO /LIB:"lafxcwd" /LIB:"oldnames" /LIB:"libw"     *
*                   /LIB:"llibcew" /NOD /PACKC:61440 /STACK:10240          *
*                   /ALIGN:16 /ONERROR:NOEXE /CO /LIB:"commdlg.lib"        *
*                   /LIB:"mmsystem.lib" /LIB:"olecli.lib"                  *
*                   /LIB:"olesvr.lib" /LIB:"shell.lib"                     *
*                                                                          *
* TO DO:            1. Other curves.                                       *
*                                                                          *
* CHANGE HISTORY:                                                          *
*   Date     By             Description                                    *
* 09/16/00  AKE    Initial coding and debug.                               *
\**************************************************************************/
#include "stddsp.h"

#define CURVEFIT_CPP
#define COPYRIGHT_DATE  "2000"
#define VERSION_NUMBER  "0.1"

#include "CurveFit.h"


////////////////////////////////////////////////////////////////////////////////////////
// Class function to return copyright notice.                                         //
////////////////////////////////////////////////////////////////////////////////////////
char * CCurveFitting::Copyright(void) {
    static char Notice[] = {"Curve Fitting, Version " VERSION_NUMBER "\n"
                            "Copyright (c) " COPYRIGHT_DATE " by Summer Institute of Linguistics. "
                            "All rights reserved.\n"
                           };
    return(Notice);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return version of class.                                         //
////////////////////////////////////////////////////////////////////////////////////////
float CCurveFitting::Version(void) {
    return((float)atof(VERSION_NUMBER));
}


////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate curve fitter parameters and construct object.           //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CCurveFitting::CreateObject(CCurveFitting ** ppCurveFitter, CURVE_TYPE CurveType) {
    if (!ppCurveFitter) {
        return(Code(INVALID_PARM_PTR));
    }
    float * pCoeff = (float *)new float[CurveType];
    if (!pCoeff) {
        return(Code(OUT_OF_MEMORY));
    }
    *ppCurveFitter = new CCurveFitting(CurveType, pCoeff);
    if (!*ppCurveFitter) {
        delete pCoeff;
        return(Code(OUT_OF_MEMORY));
    }
    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Peak picker object constructor.                                                    //
////////////////////////////////////////////////////////////////////////////////////////
CCurveFitting::CCurveFitting(CURVE_TYPE CurveType, float * pCoeff) {
    m_nCurveType = (short)CurveType;
    m_coeff = pCoeff;
    m_pData = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// Peak picker object destructor.                                                     //
////////////////////////////////////////////////////////////////////////////////////////
CCurveFitting::~CCurveFitting() {
    delete m_coeff;
}



////////////////////////////////////////////////////////////////////////////////////////
// Object function to fit data points to a specified curve.                           //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CCurveFitting::FitCurve(const float * Data, uint16 nDataPoints) {
    if (!Data) {
        return(Code(INVALID_PARM_PTR));
    }
    if (!nDataPoints) {
        return(Code(INVALID_DATA_LEN));
    }
    switch (m_nCurveType) {
    case PARABOLIC:
        if (nDataPoints < 3) {
            return(Code(INVALID_DATA_LEN));
        }
        if (nDataPoints == 3) {
            // exact fit
            ////////////////////////////////////////////////////////////////////////
            // From Garrett Mitchener                                             //
            //                                                                    //
            // Coefficient derivation for fitting a parabola                      //
            //                                                                    //
            //        y[x] = c[0] + c[1]*x + c[2]*x^2                             //
            //                                                                    //
            // to the points (-1,y[0]), (0,y[1]), (1,y[2]).  Starting at x = -1   //
            // greatly simplifies the coefficient formulae, resulting in faster   //
            // execution.                                                         //
            //                                                                    //
            // y[0] = c[0] + c[1]*(-1) + c[2]*(-1)^2 = c[0] - c[1] + c[2]         //
            //                                                                    //
            // y[1] = c[0] + c[1]*0 + c[2]*0^2 = c[0]                             //
            //                                                                    //
            // y[2] = c[0] + c[1]*1 + c[2]*1^2 = c[0] + c[1] + c[2]               //
            //                                                                    //
            // y[2] - y[0] = c[0] + c[1] + c[2] - (c[0] - c[1] + c[2])            //
            // (y[2] - y[0])/2 = c[1]                                             //
            //                                                                    //
            // y[2] = y[1] + (y[2] - y[0])/2 + c[2]                               //
            // y[2] - (y[2] - y[0])/2 - y[1] = c[2]                               //
            // (y[2] + y[0])/2 - y[1] = c[2]                                      //
            //                                                                    //
            // Thus,                                                              //
            //   c[0] = y[1],                   value at middle point             //
            //   c[1] = (y[2] - y[0])/2,        trendline                         //
            //   c[2] = (y[2] + y[0])/2 - y[1], deviation from the trendline      //
            ////////////////////////////////////////////////////////////////////////
            m_coeff[0] = Data[1];
            m_coeff[1] = (Data[2] - Data[0])/2.F;
            m_coeff[2] = (Data[2] + Data[0])/2.F - Data[1];
        } else {
            // least squared fit
            //!!code goes here
        }
        m_pData = (float *)Data;
        break;
    }

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to calculate first derivative of specified curve at given location.//
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CCurveFitting::CalcFirstDerivative(float * pFirstDeriv, float x) {
    if (!m_pData) {
        return(Code(INVALID_PARM_PTR));
    }
    switch (m_nCurveType) {
    case PARABOLIC:
        x -= 1.F;  // subtract 1 since parabolic formulae assume x starts at -1 instead of 0
        *pFirstDeriv = m_coeff[1] + 2.F * m_coeff[2] * x;
        break;
    }
    return(DONE);
}

dspError_t CCurveFitting::CalcFirstDerivative(float * pFirstDeriv, const float * x) {
    if (!m_pData) {
        return(Code(INVALID_PARM_PTR));
    }
    if (!x) {
        return(Code(INVALID_PARM_PTR));
    }
    switch (m_nCurveType) {
    case PARABOLIC:
        // Calculate first derivative, subtracting 1 from the x index, since parabolic formulae
        // assume x starts at -1 instead of 0.
        *pFirstDeriv = m_coeff[1] + 2.F * m_coeff[2] * ((float)(x - m_pData) - 1.F);
        break;
    }
    return(DONE);
}

dspError_t CCurveFitting::FindFirstDerivRoot(float * pFirstDerivRoot, uint16 /* nRootIndex */) {
    if (!m_pData) {
        return(Code(INVALID_PARM_PTR));
    }
    if (!pFirstDerivRoot) {
        return(Code(INVALID_PARM_PTR));
    }
    switch (m_nCurveType) {
    case PARABOLIC:
        if (m_coeff[2] == 0.F) {
            return(NO_ROOT_FOUND);
        }
        *pFirstDerivRoot = -m_coeff[1] / (2.F * m_coeff[2]);
        *pFirstDerivRoot += 1.F;  // add 1 since parabolic formulae assume x starts at -1 instead of 0
        break;
    }
    return(DONE);
}

dspError_t CCurveFitting::CalcSecondDerivative(float * pSecondDeriv, float /* x */) {
    if (!m_pData) {
        return(Code(INVALID_PARM_PTR));
    }
    switch (m_nCurveType) {
    case PARABOLIC:
        *pSecondDeriv = 2.F * m_coeff[2];
        break;
    }
    return(DONE);
}

dspError_t CCurveFitting::CalcSecondDerivative(float * pSecondDeriv, const float * x) {
    if (!m_pData) {
        return(Code(INVALID_PARM_PTR));
    }
    if (!x) {
        return(Code(INVALID_PARM_PTR));
    }
    switch (m_nCurveType) {
    case PARABOLIC:
        *pSecondDeriv = 2.F * m_coeff[2];
        break;
    }
    return(DONE);
}

dspError_t CCurveFitting::GetValue(float * pValue, float x) {
    if (!m_pData) {
        return(Code(INVALID_PARM_PTR));
    }
    switch (m_nCurveType) {
    case PARABOLIC:
        x = x - 1.F;   // adjust by 1 since formulae based on x starting at -1
        *pValue = m_coeff[0] + m_coeff[1]*x + m_coeff[2]*x*x;
        break;
    }
    return(DONE);
}


