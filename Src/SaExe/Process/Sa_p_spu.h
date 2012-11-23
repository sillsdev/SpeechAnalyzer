/////////////////////////////////////////////////////////////////////////////
// sa_p_spu.h:
// Interface of the CProcessSpectrum class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_SPU_H

#include "appdefs.h"
#include "dsp\spectrum.h"
 
#define _SA_P_SPU_H

enum ESPECTRUM {CEPSTRAL_SPECTRUM = 0x0001, LPC_SPECTRUM = 0x0002};   // spectra selections
                                                                            
typedef struct SSPECT_PROC_SELECT {
                 BOOL bCepstralSpectrum   : 1;
                 BOOL bLpcSpectrum        : 1;
                 BOOL bSpare4             : 1;
                 BOOL bSpare8             : 1;
                 BOOL bSpare16            : 1;
                 BOOL bSpare32            : 1;
                 BOOL bSpare64            : 1;
                 BOOL bSpare128           : 1;
                 BOOL bSpare256           : 1;
                 BOOL bSpare512           : 1;
                 BOOL bSpare1k            : 1;         
                 BOOL bSpare2k            : 1;
                 BOOL bSpare4k            : 1;
                 BOOL bSpare8k            : 1;
                 BOOL bSpare16k           : 1;                
                 BOOL bSpare32k           : 1;
                } SPECT_PROC_SELECT;
               
#pragma pack(1)
  typedef struct SSPECT_VALUE{
                 float Raw;
                 float Smooth;
                 float Lpc;
                } SPECT_VALUE;
#pragma pack()

              
  typedef struct SSPECT_PWR_RANGE{
                 SPECT_VALUE Max;
                 SPECT_VALUE Min;
                 float       fdBRef;
                }SPECT_PWR_RANGE;

#pragma pack(1)
  typedef struct SFORMANT{
                 FORMANT_VALUES Cepstral;
                 FORMANT_VALUES Lpc; 
                }FORMANT;
#pragma pack()


#pragma pack(1)
typedef struct SFORMANT_FRAME{
				double LpcErrorInPercent;
				FORMANT Formant[MAX_NUM_FORMANTS+1];
              } FORMANT_FRAME;
#pragma pack()

#define LPC_ERROR_THRESHOLD  15

               
//###########################################################################
// CProcessSpectrum data processing

class CProcessSpectrum : public CDataProcess
{

// Construction/destruction/creation
public:
    CProcessSpectrum();
    virtual ~CProcessSpectrum();

// Attributes
private:
    DWORD             m_dwFrameStart;
    DWORD             m_dwFrameSize;
    SpectrumParm      m_stParmSpec;
    SpectrumParm      m_stParmProc;
    SPECT_PROC_SELECT m_stSpectraProc;    
    unsigned short    m_nSpectralBands;
    SPECT_PWR_RANGE   m_stBandPower;
    unsigned short    m_nFormants;

// Operations 
protected:
    virtual long Exit(int nError, void *mem); // exit processing on error
    
public:
    void SetSpectrumParms(SpectrumParm* pParmSpec) {m_stParmSpec = *pParmSpec;}
    SpectrumParm* GetSpectrumParms(void) {return &m_stParmSpec;}
    virtual long Process(void* pCaller, ISaDoc* pDoc, DWORD dwFrameStart, DWORD dwFrameSize,
                       SPECT_PROC_SELECT SpectraSelected, int nProgress = 0, int nLevel = 1);
    virtual DWORD GetDataSize() {return GetDataSize(sizeof(SPECT_VALUE));} // return processed data size in spectrum data structures
    virtual DWORD GetDataSize(size_t nElements) {return (DWORD)CDataProcess::GetDataSize(nElements);} // return processed data size in LPC data structures
    virtual void* GetProcessedData(DWORD dwOffset, BOOL bBlockBegin = FALSE); // return spectrum data pointer on given position (offset)
    virtual int GetProcessedData(DWORD dwOffset, BOOL*); // return spectrum data from given position (offset)    
    virtual unsigned short GetSpectralCount() {return m_nSpectralBands;}    
    virtual SPECT_VALUE& GetSpectralData(unsigned short wIndex); // return spectrum data from given position
    virtual SPECT_PWR_RANGE& GetSpectralPowerRange() {return m_stBandPower;};  // return max and min of spectral power bands    
    virtual unsigned short GetFormantCount() {return m_nFormants;}  // returns number of formants calculated, including fundamental (F0)
    virtual FORMANT& GetFormant(unsigned short wIndex);  // returns formant at index
    virtual FORMANT_FRAME* GetFormants();                        // returns pointer to formants
	virtual float GetSpectralRegionPower(ISaDoc* pDoc, unsigned short wFreqLo, unsigned short wFreqHi);  // returns average power in specified region of spectrum
};

#endif //_SA_P_SPU_H
