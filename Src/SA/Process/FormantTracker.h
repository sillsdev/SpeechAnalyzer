#ifndef FORMANTTRACKER_H
#define FORMANTTRACKER_H

#include "Process\TrackState.h"

typedef std::complex<double> CDBL;
typedef std::vector<CDBL> VECTOR_CDBL;
typedef std::deque<CDBL> DEQUE_CDBL;
typedef double DBL;
typedef std::vector<DBL> VECTOR_DBL;

class CProcessFormantTracker : public CProcess {
public:
    CProcessFormantTracker(CProcess & Real, CProcess & Imag, CProcess & Pitch);
    virtual ~CProcessFormantTracker();

    virtual long Process(void * pCaller, ISaDoc *, int nProgress = 0, int nLevel = 1);
    SFormantFreq * GetFormant(DWORD dwIndex);
    void Dump(LPCSTR ofilename);

private:
    CProcess * m_pReal;
    CProcess * m_pImag;
    CProcess * m_pPitch;

    void AdvanceData(STrackState & state, DWORD dwDataPos, int nSamples);
    void WriteTrack(STrackState & state, double samplingRate, int pitch); // write a block into the temporary file
    bool BuildTrack(STrackState & state, double samplingRate, int pitch);
};

#endif
