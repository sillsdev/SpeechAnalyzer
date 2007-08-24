#ifndef SPECTGRM_H
 #define SPECTGRM_H

 #include "Error.h"
 #include "dspTypes.h"
 #include "Signal.h"
 #include "DSPWins.h"
 #include "fft.h"
 #include "MathX.h"
 #include "Formants.h"
 
 #define UNDEF      0                                 

 enum{NARROWBAND=-2, WIDEBAND=-1};
 #define NARROW_BW   45.F 
 #define MEDIUM_BW  172.F   
 #define WIDE_BW    300.F   
 
 enum{PENDING=0, END_OF_SPGM=1, END_FMNT_CALC=1};

 typedef struct{
                uint16 WinLen;
                uint16 FFTLen;
                uint16 FFTIntv;
               }SpgmTblRowStruct;
              
 #define DB_MIN              0
 #define DB_MAX            233  //!!was 234
 #define FMNT_TRACK        234
 #define BLANK             235
 #define NA                 -1

 #define CHIRP_DECAY         0.96F
 
 enum{SPGM_NOT_READY=0, SPGM_PENDING=1, SPGM_CALC=2, FORMANTS_CALC=3};  //dspError_t definitions
 
 typedef struct SPGM_SETTINGS {
   SPGM_SETTINGS() { windowType = CWindowSettings::kGaussian;}
                float    LwrFreq;           //lowest frequency to calculate in Hz
                float    UprFreq;           //highest frequency to calculate in Hz
                float    Bandwidth;         //spectral resolution in Hz
                uint16   FreqCnt;           //number of frequency samples from low to high
                uint16   FFTLength;         //desired length of FFT 
                uint32    SpectCnt;          //number of spectra to calculate
                bool PreEmphSw;         //pre-emphasis switch
                bool FmntTrackSw;       //formant tracking switch
                uint16   NumFmnt;           //number of formants to track
                uint32    SigBlkOffset;      //number of samples offset into speech data 
                uint32    SigBlkLength;      //number of speech samples over which spectra 
                                            //  will be calculated
                uint16   SpectBatchLength;  //number of spectra to be calculated at one time
                CWindowSettings::Type windowType;
               }SPGM_SETTINGS;
               

class Spectrogram{
           public:
             static char *Spectrogram::Copyright(void);
             static float Spectrogram::Version(void);
             static dspError_t CreateObject(Spectrogram** Spgm, SPGM_SETTINGS SpgmSetting, SIG_PARMS Signal);
             //dspError_t ChangeSettings(SPGM_SETTINGS New);
             dspError_t GetMetrics(float *FreqScale, 
                            float *SpectScale, float *SpectRes, float *SpectIntv, 
                            uint8 *MaxPwr);
             dspError_t Generate(void);
             dspError_t ReadPower(short *Power, uint16 SpgmX, uint16 SpgmY);
			 static dspError_t CalcPower(float *PowerInDb, float fFrequency, float fDspWinBandwidth, CWindowSettings::Type, SIG_PARMS Signal, bool bPreEmphasis, float DbRef = 0.F);
			 static dspError_t CalcPower(float *PowerInDb, float Frequency, int32 nBandwidthSelect, CWindowSettings::Type, SIG_PARMS Signal, bool bPreEmphasis);
             uint8 * ReadPowerSlice(dspError_t* dspError_t, uint16 SpgmX); 
             dspError_t ReadFormants(FORMANT_FREQ *Freq, uint16 SpgmX);
             DspWin& GetWidebandWindow() {return m_Window;}
             DspWin& GetNarrowbandWindow() {return m_NBWindow;}
             //dspError_t CalcFormants(FORMANT_FREQ *Formants, uint16 SpectIndex);
             dspError_t CalcFormants(void);
             ~Spectrogram();
             bool IsAliased() {return m_Aliased;}
           public:
             uint8   *m_ScreenData;
                        

           private:
             static dspError_t ValidateSignalParms(SIG_PARMS Signal);
             static dspError_t ValidateSettings(SPGM_SETTINGS Setting);
             Spectrogram(SPGM_SETTINGS SpgmSetting, uint8 *SpgmData, FORMANT_FREQ *FmntData,
                         DspWin &Window, DspWin &NBWindow, 
                         SIG_PARMS Signal, uint8 *ScreenData);            
             dspError_t CalcSpectra(uint8 *BlockStart, uint32 NumSpect);
             dspError_t CalcSpectra(short *BlockStart, uint32 NumSpect);
             dspError_t PreProc(short *Frame, DspWin &Window);
             dspError_t PreProc(uint8 *Frame, DspWin &Window);
             dspError_t PwrFFT(uint8 *PwrSpect);     
             void BlankSpectrum(uint8 *Spect);   //!!Validate Spect?
             void BlankFmntSet(FORMANT_FREQ *Fmnt);
             dspError_t SeekFormants(uint8 *BlockStart, uint32 NumFmntSets);
             dspError_t SeekFormants(short *BlockStart, uint32 NumFmntSets);
             dspError_t GetFormants(FORMANT_FREQ *F);
             bool       m_Aliased;             
             FORMANT_FREQ *m_FmntData;             
             bool       m_PreEmphSw;
             bool       m_FmntTrackSw;
             uint16         m_NumFmnt;
             void     *m_SigStart;
             uint32          m_SigLength;
             int8          m_SmpDataFmt;
             uint16         m_SmpRate;
             uint32          m_SigBlkOffset;
             uint32          m_SigBlkLength;
             uint8    *m_SpgmData;
             uint16         m_SpgmHgt;
             float          m_SpectIntv;
             uint32          m_SpectCnt;
             uint16         m_SpectLen;
             float          m_SpectRes;
             float          m_SpectScale;
             uint16         m_FFTLen;    
             float          m_FreqScale;
             float          m_LwrFreq;
             float          m_UprFreq;
             uint32          m_SpectToProc;
             uint32          m_FmntSetToProc;
             uint16         m_SpectBatchLength;
             DspWin         m_Window;
             DspWin         m_NBWindow;
             float         *m_WinFrame;
             uint16         m_MinPitchPeriod;
             uint16         m_MaxPitchPeriod;
             uint8          m_MaxPwr;
             uint8          m_Status;
          };
         
          
int32 CalcSpectgram(
                   float  **Spectgram,      //pointer to spectrogram base addr
                   float        *SpectMax,       //maximum spectral value
                   uint16       *SpectrumLen,    //number of samples in spectrum
                   uint16       *NumSpectra,     //number of spectra
                   uint16       *SpectraIntv,    //spectra calculation interval
                   uint8         BandType,       //type of spectrogram
                   int8   *Signal,         //pointer to signal data
                   uint16        SigLength,         //number of signal samples
                   int8         SmpType,        //sample word type
                   uint16        SmpRate         //sampling rate
                  );
                  
#endif


