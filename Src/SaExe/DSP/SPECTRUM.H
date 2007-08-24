#ifndef SPECTRUM_H

  #define SPECTRUM_H
          
  #include "dspTypes.h"
  #include "Error.h"
  #include "Signal.h"
  #include "Scale.h"
  #include "Formants.h"
  #include "fft.h"
  

  
  #include "DSPWins.h"
    
  #define MAX_FRAME_DUR    2.0F       //maximum frame duration (in sec) allowed
  
                                                                
  typedef struct{
                 short    nScaleSelect;         //scale, either LINEAR or DB
                 uint16   wLength;              //number of spectral values to calculate
                 float    fSmoothFreq;          //max frequency allowed for smoothing
                 float    fFFTRadius;           //radius for evaluating FFT in z-plane
                 int32      nWindowType;            //DSP window switch:  ON = apply window
                }SPECT_SETTINGS;
              
  typedef struct{
                 uint16          wSpectLength;
                 float           fFreqScale;
                 float           fdBRef;                 
                 float          *pfRawSpectrum;
                 float          *pfSmoothSpectrum;
                 uint16          wNumFormants;
                 FORMANT_VALUES *pstFormant;
                }SPECT_PARMS;
                
  class CSpectrum{
             public:
               static char *Copyright(void);
               static float Version(void);
               static dspError_t CreateObject(CSpectrum** ppoSpectrum, 
                                       SPECT_SETTINGS &stSpectSetting, SIG_PARMS &stSignalParm,  
                                       uint16 wFFTLength = MAX_FFT_LENGTH);  //must be a power of 2
               SPECT_PARMS GetSpectParms(void);                        
                                       
                                        
               //dspError_t GetMetrics(uint16 *pwFreqCnt, float *pfFreqScale, uint16 *pwWinLen);
               //dspError_t GetRange (float *pMin, float *pMax);
               ~CSpectrum();
 
             private:
               static dspError_t ValidateSignalParms(SIG_PARMS &stSignalParms);
               static dspError_t ValidateSettings(SPECT_SETTINGS &stSpectSetting);
               CSpectrum(SPECT_SETTINGS &stSpectSetting, 
                         SIG_PARMS &stSignalParm, DspWin &poDSPWindow, 
                         uint16 wFFTLength, float *pfFFTBuffer, 
                         float *pfRawSpectrum, float *pfSmoothSpectrum);
               dspError_t Preprocess(uint8 *pubFrame);
               dspError_t Preprocess(int8 *psbFrame);
               dspError_t Preprocess(short *psFrame);
               dspError_t CalcPwrFFT(void);
               dspError_t GetPwrSpectrum(void);               
               void           *m_pFrameStart;
               uint32           m_dwFrameLength;
               uint16          m_wSmpRate;
               int8           m_sbSmpFormat;
               uint16          m_wSpectLength;
               short           m_nScaleSelect;
               float           m_fSmoothFreq;
               float           m_fSpectCalcRadius;
               DspWin          m_oDSPWindow;
               uint16          m_wFFTLength;
               float          *m_pfFFTBuffer;
               float          *m_pfRawSpectrum;
               float          *m_pfSmoothSpectrum;             
               uint16          m_wNumFormants;
               FORMANT_VALUES  m_pstFormant[MAX_NUM_FORMANTS+1];   //pitch and formants 1, 2, etc.
               uint16          m_wMinPitchPeriod;               
               uint16          m_wMaxPitchPeriod;
               double             m_dMaxRawPower;
			   float           m_fPwrDbRef[3];
			   bool m_bReplicate;
            };
          
#endif
          