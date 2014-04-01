/////////////////////////////////////////////////////////////////////////////
// sa_p_sfmt.h:
// Interface of the CProcessSpectroFormants class
// Author: Steve MacLean
// copyright 2001 JAARS Inc. SIL
//
// Revision History
// 6/27/2001 SDM Split from process contained in CProcessSpectrogram
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_SFMT_H
#define _SA_P_SFMT_H

//###########################################################################
// CProcessSpectroFormants data processing

#define FRICTION_THRESHOLD        3000.F  // threshold in Hz above which zero crossing rate implies friction in the signal
#define RELATIVE_POWER_THRESHOLD    40.F  // power threshold in dB, relative to that of F1, above which formant tracks are displayed

class CProcessSpectroFormants : public CProcess
{

public:
    CProcessSpectroFormants();
    virtual ~CProcessSpectroFormants();

    virtual DWORD GetDataSize();
	// return processed data size in LPC data structures
    virtual DWORD GetDataSize(size_t nElements);
    long Process(void * pCaller, CView * pView, int nWidth, int nHeight, int nProgress = 0, int nLevel = 1);
    SFormantFreq * GetFormant(DWORD dwIndex); // return spectrogram slice data
    long ExtractFormants(ISaDoc * pDoc, DWORD dwWaveDataStart, DWORD dwWaveDataLength, BOOL bSmooth = TRUE, int nProgress = 0, int nLevel = 1);
    BOOL AreFormantTracksReady(); // return TRUE if processed formants data is ready
};

#endif
