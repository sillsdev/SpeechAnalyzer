#ifndef TRACKSTATE_H
#define TRACKSTATE_H

#include "dsp\Signal.h"
#include "dsp\Formants.h"
#include "dsp\dspTypes.h"
#include "Process.h"
#include "Process\FormantTracker.h"

class CProcessTrackState : public CProcessFormantTracker::CTrackState
{

public:
    CProcessTrackState();
    virtual ~CProcessTrackState();

    virtual DEQUE_CDBL & GetData();
    virtual VECTOR_DBL & GetWindow();

    // Previous track position
    virtual VECTOR_CDBL & GetTrackIn();

    // Result track position
    virtual VECTOR_CDBL & GetTrackOut();

    // Working intermediates to eliminate memory thrashing
    virtual VECTOR_CDBL & GetWindowed();
    virtual VECTOR_CDBL & GetFiltered();
    virtual VECTOR_CDBL & GetZeroFilterCDBL();
    virtual VECTOR_DBL & GetZeroFilterDBL();

protected:
    DEQUE_CDBL m_data;
    VECTOR_DBL m_window;

    VECTOR_CDBL m_trackIn;
    VECTOR_CDBL m_trackOut;
    VECTOR_CDBL m_windowed;
    VECTOR_CDBL m_filtered;
    VECTOR_CDBL m_zeroFilterCDBL;
    VECTOR_DBL m_zeroFilterDBL;
};

#endif
