/////////////////////////////////////////////////////////////////////////////
// sa_p_dur.cpp:
// Implementation of the CProcessDuration
//                       CDurationEdit classes.
// Author: Alec Epting
// copyright 1998 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"
#include "sa_p_dur.h"
#include "Segment.h"

#include <limits.h>
#include "isa_doc.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CProcessDurations::CProcessDurations()
{
}

CProcessDurations::~CProcessDurations()
{
}

/***************************************************************************/
// CProcessDuration::Process Processing Duration data
// This function points to document duration data is stored as a word array.
// create it helper functions of the base class are used. While processing a
// process bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessDurations::Process(void * pCaller, ISaDoc * pDoc, int nProgress, int nLevel)
{

    //TRACE(_T("Process: CProcessDurations\n"));
    if (IsCanceled())
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady())
    {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSDUR))   // memory allocation failed
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // set pointer to segment durations
    CSegment * pSegment = pDoc->GetSegment(PHONETIC);
    CString * pPhonetic = pSegment->GetString();
	m_Durations.RemoveAll();
	for (int i=0;i<pSegment->GetDurationSize();i++) 
	{
		m_Durations.Add(pSegment->GetDuration(i));
	}

    // find min and max durations
    m_dwMaxValue = 0;
    m_dwMinValue = ULONG_MAX;
    for (int i = 0; i < pPhonetic->GetLength(); i++)
    {
        m_dwMaxValue = max(m_dwMaxValue, pSegment->GetDurationAt(i));
        m_dwMinValue = min(m_dwMinValue, pSegment->GetDurationAt(i));
    }

    SetDataReady();
    EndProcess();
    EndWaitCursor();
    return MAKELONG(nLevel, 100);
}

DWORD CProcessDurations::GetProcessedData(int nSegmentIndex)
{
    return m_Durations.GetAt(nSegmentIndex);
}

DWORD CProcessDurations::GetDataSize()
{
    return (DWORD)m_Durations.GetSize();   //return number of durations
}

void CProcessDurations::SetDataInvalid()
{
    CProcess::SetDataInvalid();
}

DWORD CProcessDurations::GetMaxDuration()
{
    return m_dwMaxValue;   // return maximum value
}

DWORD CProcessDurations::GetMinDuration()
{
    return m_dwMinValue;   // return minimum value
}

