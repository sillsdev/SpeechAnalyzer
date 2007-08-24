#ifndef CURVEFIT_H
#define CURVEFIT_H

 #include "dspTypes.h"
 #include "Error.h"

 enum CURVE_TYPE {PARABOLIC = 3};  // type must be set to number of polynomial coefficients

 class CCurveFitting
       { public:
           static char *Copyright(void);
           static float Version(void);
           static dspError_t CreateObject(CCurveFitting **ppCurveFitter, CURVE_TYPE CurveType);
           dspError_t FitCurve(const float *pData, uint16 nDataPoints);
           dspError_t GetValue(float *pValue, float x);
           dspError_t CalcFirstDerivative(float *pFirstDeriv, float x);
           dspError_t CalcFirstDerivative(float *pFirstDeriv, const float *x);
           dspError_t FindFirstDerivRoot(float *pFirstDerivRoot, uint16 nRootIndex = 0);
           dspError_t CalcSecondDerivative(float *pSecondDeriv, float x);
           dspError_t CalcSecondDerivative(float *pSecondDeriv, const float *x);
           ~CCurveFitting();                
         private:
           CCurveFitting(CURVE_TYPE CurveType, float *pCoeff);
           short       m_nCurveType;
           float      *m_coeff;
           float *m_pData;
       };
         
#endif   // CURVEFIT_H         