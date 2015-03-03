/////////////////////////////////////////////////////////////////////////////
// sa_p_spg.h:
// Interface of the CProcessSpectrogram class
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   10.5Test11.1A
//      RLJ Add SetSpectroAB parameter to GetIndex call so we that we know
//             which set (A or B) of spectrogram parameters to save.
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_SPG_H
#define _SA_P_SPG_H

#include "Appdefs.h"
#include "SpectroParm.h"

class CSpectroParm;
class CProcessSpectroFormants;

class CProcessSpectrogram : public CProcessAreaData {
public:
    CProcessSpectrogram(const CSpectroParm & cParm, ISaDoc * pDoc, BOOL bRealTime = TRUE);
    virtual ~CProcessSpectrogram();

    long Process(void * pCaller, ISaDoc * pDoc, CSaView * pView, int nWidth, int nHeight, int nProgress = 0, int nLevel = 1);
    void * GetSpectroSlice(DWORD dwIndex);              // return spectrogram slice data
    const CSpectroParm & GetSpectroParm() const;
    void SetSpectroParm(const CSpectroParm & cValue);   // set spectrogram parameters
    int GetWindowWidth() const;
    int GetWindowHeight() const;
    CProcessSpectroFormants * GetFormantProcess();

protected:
    virtual long Exit(int nError);                      // exit processing on error

private:
    int SpectraBandwidth();
    int NyquistSpectraInterval(double dSourceSamplingRate);
    virtual ISaDoc * GetDocument() const;

    int m_nWindowWidth;
    int m_nWindowHeight;
    CSpectroParm m_SpectroParm;
    CProcessSpectroFormants * m_pSpectroFormants;
    const BOOL m_bRealTime;
    ISaDoc * m_pDoc;
};

#endif
