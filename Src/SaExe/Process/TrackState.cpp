#include "stdafx.h"
#include "Process\TrackState.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CProcessTrackState::CProcessTrackState() {
}

CProcessTrackState::~CProcessTrackState() {
}

DEQUE_CDBL & CProcessTrackState::GetData() {
    return m_data;
}

VECTOR_DBL & CProcessTrackState::GetWindow() {
    return m_window;
}

// Previous track position
VECTOR_CDBL & CProcessTrackState::GetTrackIn() {
    return m_trackIn;
}

// Result track position
VECTOR_CDBL & CProcessTrackState::GetTrackOut() {
    return m_trackOut;
}

// Working intermediates to eliminate memory thrashing
VECTOR_CDBL & CProcessTrackState::GetWindowed() {
    return m_windowed;
}

VECTOR_CDBL & CProcessTrackState::GetFiltered() {
    return m_filtered;
}

VECTOR_CDBL & CProcessTrackState::GetZeroFilterCDBL() {
    return m_zeroFilterCDBL;
}

VECTOR_DBL & CProcessTrackState::GetZeroFilterDBL() {
    return m_zeroFilterDBL;
}
