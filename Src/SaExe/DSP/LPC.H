#ifndef LPC_H
 #define LPC_H
 
 #include "dspTypes.h"
 #include "MathX.h" 
 #include "Error.h"
 #include "Signal.h"
 #include "FFT.h"
 #include "Formants.h"

 #define LPC_8BIT_DB_PWR_REF    63.112F
 #define LPC_16BIT_DB_PWR_REF  126.190F
 

 enum {REFL_COEFF = 0x0001, PRED_COEFF = 0x0002, CORR_VALUES = 0x0004, 
       LOG_AREA_RATIOS = 0x0008, NORM_CROSS_SECT = 0x0010, ENERGY = 0x0020, GAIN = 0x0040, 
       FORMANTS = 0x0080, WINDOW_SIGNAL = 0x100, PRE_EMPHASIS = 0x0200, NO_DC_BIAS = 0x0400, PRED_SIGNAL = 0x0800, 
       MEAN_SQ_ERR = 0x1000, RESIDUAL = 0x2000, POWER_SPECTRUM = 0x4000};   //list of LPC analysis
                                                                            //parameter sets
 enum FORMANT_METHOD{
	  FROM_POLES,
	  FROM_SPECTRUM
      };
                                                                            
 typedef struct{
                uint16 bReflCoeff          : 1;
                uint16 bPredCoeff          : 1;
                uint16 bCorrValues         : 1;
                uint16 bLogAreaRatios      : 1;
                uint16 bNormCrossSectAreas : 1;
                uint16 bEnergy             : 1;
                uint16 bGain               : 1;
                uint16 bFormants           : 1;
                uint16 bWindowSignal       : 1;
                uint16 bPreEmphasis        : 1;
                uint16 bNoDcBias           : 1;             
                uint16 bPredSignal         : 1;
                uint16 bMeanSqPredError    : 1;
                uint16 bResidual           : 1;
                uint16 bPwrSpectrum        : 1;                
                uint16 bSpare32k           : 1;
               }LPC_PARM_SETS;
        
 typedef union{       
               LPC_PARM_SETS  ParmSet;  
               uint16 Flags;                                                     
              }LPC_PROC_REQ;
      
      
 enum{LPC_COVAR_LATTICE = 0, LPC_AUTOCOR = 1, LPC_CEPSTRAL = 2};                      //LPC analysis methods
 typedef struct LPC_SETTINGS {
   LPC_SETTINGS() {nMethod = 0; nOrder = 0; nFrameLen = 0; fFFTRadius = 1.; dPitch = 0; dClosurePhase = 0;}

   uint16  nMethod;                            //LPC analysis method selected from above
   uint16  nOrder;                             //order of LPC analysis (number of stages or sections)
   uint32   nFrameLen;                          //frame length in samples
   float   fFFTRadius;                          //for POWER_SPECTRUM, radius for spectral evaluation, >0 and <= 1.F
   double     dPitch;                            // pitch for closed phase analysis
   double     dClosurePhase;                     // phase of glottal closure relative to signal start in radians 
   LPC_PROC_REQ Process;                      //LPC parameter sets to process
 }LPC_SETTINGS; 
 
 typedef struct{
                uint32  nFrameLen;                           //frame length in samples
                short   *pFrameBfr;                          //pointer to frame buffer
                short   *pWinFrameBfr;                       //pointer to windowed frame buffer
                uint16  nOrder;                              //order of LPC predictor
                uint8   nMethod;                             //LPC analysis method                
                double     dMethodError;                        //specific method error
                double     dMeanSqPredError;                    //mean squared prediction error
                double     dMeanQuadPredError;                  //mean prediction error^4
                double     dPeakError;                          //mean squared peak error
                uint16  nReflCoeffs;                         //number of reflection coefficients
                double    *pReflCoeff;                          //pointer to reflection coefficients
                uint16  nPredCoeffs;                         //number of prediction coefficients
                double    *pPredCoeff;                          //pointer to prediction coefficients:
                                                             //  s[n] = pPredCoeff[1]*s[n-1] + 
                                                             //         pPredCoeff[2]*s[n-2] + ...
                                                             //         pPredCoeff[nPredCoeff-1]*s[n-nPredCoeff+1]
                                                             //  pPredCoeff[0] is coefficient for s[n], set to 1
                uint16   nLpcCoeffs;                         //number of Lpc coefficients
                double     *pLpcCoeff;                          //pointer to polynomial coefficients for all-pole model
                double      dGain;                              //prediction filter gain
                double      dMeanEnergy;                        //mean energy in frame     
                double      nPoles;                             //number of poles (= nOrder)
                COMPLEX_POLAR_FLOAT *pPole;                  //pointer to array of poles
                uint16   nCorrValues;                        //number of correlation values
                double     *pCorrValue;                         //pointer to correlation values
                uint16   nLogAreaRatios;                     //number of log area ratios
                double     *pLogAreaRatio;                      //pointer to log area ratios
                uint16   nNormCrossSectAreas;                //number of normalized cross sectional areas
                double     *pNormCrossSectArea;                 //pointer to normalized cross sectional areas
                uint16   nPredValues;                        //number of predicted values
                double      *pPredValue;                         //pointer to predicted signal      
                uint16   nResiduals;                         //number of residual values
                double      *pResidual;                          //pointer to buffer containing residual 
                                                             //  (signal minus prediction)
                uint16   nSpectValues;                       //number of spectral values
                float   *pPwrSpectrum;                       //pointer to power spectrum                              
                uint16   nFormants;                          //number of formant frequencies
                FORMANT_VALUES Formant[MAX_NUM_FORMANTS+1];  //array of formant frequencies and their log magnitudes
                                                             //  Formant[0].fFrequency is frequency based on frame length and is
                                                             //    equal to pitch if the frame corresponds to 1 pitch period
                                                             //  Formant[1] corresponds to F1, Formant[2] to F2, and 
                                                             //  Formant[3] to F3, etc.
               }LPC_MODEL;
               
// #define LPC_MODEL_INIT  {0, NULL, 0, 0, 0.F, 0.F, 0, NULL, 0, NULL, 0.F, 0.F, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL}

#pragma pack(1)
 typedef struct{
                double     dErrorRatio;                       // ratio of prediction error to signal frame energy
                uint16  nNormCrossSectAreas;               //number of normalized cross sectional areas
                double     dNormCrossSectArea[1];             //normalized cross sectional area array
               }VOCAL_TRACT_MODEL;               
#pragma pack() 
              
 typedef struct{
                LPC_PROC_REQ   Process;                    //LPC parameter sets to process
                double      **ppCovarMatrix;              //pointer to array of row pointers into 
                                                           //covariance matrix
                double           *pScratchArray;              //pointer to scratch array used in covariance 
                                                           //lattice harmonic mean method
                float          fFFTRadius;                 //radius used to calculate power spectrum                             
                double     dPitch;                              // pitch for closed phase analysis
                double     dClosurePhase;                       // phase of glottal closure relative to signal start in radians 
                LPC_MODEL      Model;                      //structure containing LPC model
               }LPC_PARMS;              
               
// #define LPC_PARMS_INIT  {0, NULL, NULL, LPC_MODEL_INIT}

class CLinPredCoding{
           public:
             static char *Copyright(void);
             static float Version(void);
             static dspError_t CreateObject(CLinPredCoding **ppLpcObject, LPC_SETTINGS &LpcSetting, 
                                     SIG_PARMS &Signal, uint16 wFFTLength = MAX_FFT_LENGTH);     
             dspError_t GetLpcModel(LPC_MODEL **ppLpcModel, uint8 *pFrame);
             dspError_t GetLpcModel(LPC_MODEL **ppLpcModel, short *pFrame);
             dspError_t GetLpcModel(LPC_MODEL **ppLpcModel, void *pFrame);
             dspError_t GetPowerSpectrum(uint16 wSpectLength, int32 nScaleSelect);
             float GetDecibelPowerRef();
             ~CLinPredCoding();                
           private:  
             CLinPredCoding(LPC_PARMS &LpcParm, SIG_PARMS &Signal, uint16 wFFTLength);
             static void FreeLpcMem(LPC_PARMS &LpcParm);
             void PreEmphasize(uint8 *pFrame);
             void PreEmphasize(short *pFrame);
             void Transfer(uint8 *pFrame);
             void Transfer(short *pFrame);
             void RemoveDcBias();
             void ApplyWindow();
             void CalcCovarMatrix(uint16 nMethod);
             void CalcReflCoeff(void);
             void CalcCrossSectAreas(void);
             dspError_t CalcPoles(void);
             void CalcFormants();
             void CalcEnergy(void);
             void CalcPredSignal(void);
             void CalcResidual(void);
             void CalcMeanSqError(void);
             void CalcPowerSpectrum(void);             
             SIG_PARMS  m_Signal;
             LPC_PARMS  m_LpcParm;
             uint16 m_wFFTLength;
            };   

 
#endif
