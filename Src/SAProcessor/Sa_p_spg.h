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

#include "SpectroParm.h"
#include "processareadata.h"

class CSpectroParm;
class CProcessSpectroFormants;

class CProcessSpectrogram : public CProcessAreaData {
public:
    CProcessSpectrogram(Context& context, const CSpectroParm & cParm, BOOL bRealTime = TRUE);
    virtual ~CProcessSpectrogram();

    // default nProgress = 0;
    // default nLevel = 1;
    // dataPosition = view.GetDataPosition(0)
    // dataFrame = view.GetDataFrame()
    long Process(void * pCaller, int nWidth, int nHeight, int nProgress, int nLevel, double dataPosition, DWORD dataFrame);
    void * GetSpectroSlice(DWORD dwIndex);              // return spectrogram slice data
    const CSpectroParm & GetSpectroParm() const;
    void SetSpectroParm(const CSpectroParm & cValue);   // set spectrogram parameters
    int GetWindowWidth() const;
    int GetWindowHeight() const;
    CProcessSpectroFormants * GetFormantProcess();
    void SetShowFormants(bool value);
    void SetProcessDataInvalid();
    bool IsProcessCanceled();
    void InvalidateAndRestart();
    void SetColor(int color) {
        m_SpectroParm.SetColor(color);
    };
    int GetColor() {
        return m_SpectroParm.GetColor();
    }

protected:
    virtual long Exit(int nError);                      // exit processing on error

private:
    int SpectraBandwidth();
    int NyquistSpectraInterval(double dSourceSamplingRate);

    int m_nWindowWidth;
    int m_nWindowHeight;
    CSpectroParm m_SpectroParm;
    CProcessSpectroFormants * m_pSpectroFormants;
    const BOOL m_bRealTime;
};

#endif