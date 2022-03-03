#ifndef FORMANTTRACKER_H
#define FORMANTTRACKER_H

#include "TrackState.h"

typedef std::complex<double> CDBL;
typedef std::vector<CDBL> VECTOR_CDBL;
typedef std::deque<CDBL> DEQUE_CDBL;
typedef double DBL;
typedef std::vector<DBL> VECTOR_DBL;

class CProcessFormantTracker : public CProcess {
public:
    CProcessFormantTracker(Context& context, CProcess & Real, CProcess & Imag, CProcess & Pitch);

    virtual long Process(void * pCaller,int nProgress = 0, int nLevel = 1);
    SFormantFreq * GetFormant(DWORD dwIndex);

private:
    CProcess * m_pReal;
    CProcess * m_pImag;
    CProcess * m_pPitch;

    void AdvanceData(STrackState & state, DWORD dwDataPos, int nSamples);
    void WriteTrack(STrackState & state, double samplingRate, int pitch); // write a block into the temporary file
    bool BuildTrack(STrackState & state, double samplingRate, int pitch);
};

#endif
