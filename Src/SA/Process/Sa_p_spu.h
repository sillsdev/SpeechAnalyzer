/////////////////////////////////////////////////////////////////////////////
// sa_p_spu.h:
// Interface of the CProcessSpectrum class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_SPU_H
#define _SA_P_SPU_H

#include "appdefs.h"

enum ESpectrum {CEPSTRAL_SPECTRUM = 0x0001, LPC_SPECTRUM = 0x0002};   // spectra selections

struct SSpectProcSelect
{
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
};

#pragma pack(1)
struct SSpectValue
{
    float Raw;
    float Smooth;
    float Lpc;
};
#pragma pack()

struct SSpectPowerRange
{
    SSpectValue Max;
    SSpectValue Min;
    float fdBRef;
};

#pragma pack(1)
struct SFormant
{
    SFormantValues Cepstral;
    SFormantValues Lpc;
};
#pragma pack()


#pragma pack(1)
struct SFormantFrame
{
    double LpcErrorInPercent;
    SFormant Formant[MAX_NUM_FORMANTS+1];
};
#pragma pack()

#define LPC_ERROR_THRESHOLD  15

class CProcessSpectrum : public CProcess
{

public:
    CProcessSpectrum();
    virtual ~CProcessSpectrum();
    void SetSpectrumParms(CSpectrumParm * pParmSpec);
    CSpectrumParm * GetSpectrumParms(void);
    virtual long Process(void * pCaller, ISaDoc * pDoc, DWORD dwFrameStart, DWORD dwFrameSize,
                         SSpectProcSelect SpectraSelected, int nProgress = 0, int nLevel = 1);
    virtual DWORD GetDataSize();
    virtual DWORD GetDataSize(size_t nElements);
    virtual void * GetProcessedData(DWORD dwOffset, BOOL bBlockBegin = FALSE); // return spectrum data pointer on given position (offset)
    virtual int GetProcessedData(DWORD dwOffset, BOOL *);           // return spectrum data from given position (offset)
    virtual unsigned short GetSpectralCount();
    virtual SSpectValue & GetSpectralData(unsigned short wIndex);   // return spectrum data from given position
    virtual SSpectPowerRange & GetSpectralPowerRange();             // return max and min of spectral power bands
    virtual unsigned short GetFormantCount();
    virtual SFormant & GetFormant(unsigned short wIndex);           // returns formant at index
    virtual SFormantFrame * GetFormants();                          // returns pointer to formants
    virtual float GetSpectralRegionPower(ISaDoc * pDoc, unsigned short wFreqLo, unsigned short wFreqHi); // returns average power in specified region of spectrum

protected:
    virtual long Exit(int nError, void * mem); // exit processing on error

private:
    DWORD m_dwFrameStart;
    DWORD m_dwFrameSize;
    CSpectrumParm m_stParmSpec;
    CSpectrumParm m_stParmProc;
    SSpectProcSelect m_stSpectraProc;
    unsigned short m_nSpectralBands;
    SSpectPowerRange m_stBandPower;
    unsigned short m_nFormants;
};

#endif
