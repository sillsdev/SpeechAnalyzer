#include "stdafx.h"
#include "Process\TrackState.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CProcessTrackState::CProcessTrackState()
{
}

CProcessTrackState::~CProcessTrackState()
{
}

DEQUE_CDBL & CProcessTrackState::GetData()
{
    return m_data;
}

VECTOR_DBL & CProcessTrackState::GetWindow()
{
    return m_window;
}

// Previous track position
VECTOR_CDBL & CProcessTrackState::GetTrackIn()
{
    return m_trackIn;
}

// Result track position
VECTOR_CDBL & CProcessTrackState::GetTrackOut()
{
    return m_trackOut;
}

// Working intermediates to eliminate memory thrashing
VECTOR_CDBL & CProcessTrackState::GetWindowed()
{
    return m_windowed;
}

VECTOR_CDBL & CProcessTrackState::GetFiltered()
{
    return m_filtered;
}

VECTOR_CDBL & CProcessTrackState::GetZeroFilterCDBL()
{
    return m_zeroFilterCDBL;
}

VECTOR_DBL & CProcessTrackState::GetZeroFilterDBL()
{
    return m_zeroFilterDBL;
}

void CProcessTrackState::Dump()
{
	TRACE("m_data ");
	for (size_t j=0;j<min(m_data.size(),6);j++)
	{
		TRACE("%f ",m_data[j]);
	}
	TRACE("\n");

	TRACE("m_window ");
	for (size_t j=0;j<min(m_window.size(),6);j++)
	{
		TRACE("%f ",m_window[j]);
	}
	TRACE("\n");

    DEQUE_CDBL m_data;
    VECTOR_DBL m_window;
    VECTOR_CDBL m_trackIn;
    VECTOR_CDBL m_trackOut;
    VECTOR_CDBL m_windowed;
    VECTOR_CDBL m_filtered;
    VECTOR_CDBL m_zeroFilterCDBL;
    VECTOR_DBL m_zeroFilterDBL;
}
