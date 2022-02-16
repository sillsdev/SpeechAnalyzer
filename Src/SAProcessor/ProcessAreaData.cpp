/////////////////////////////////////////////////////////////////////////////
// ProcessAreaData.cpp:
// Implementation of the CAreaDataProcess classes
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ProcessAreaData.h"
#include "FileUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CAreaDataProcess
// Base class for all area data processing classes. Does all jobs, common to
// all area data processing derived classes. The area process only processes
// an area in the whole raw data and allows multiple areas to be processed.
// For each area, determined by an index number, there will be a temporary
// file, where the processed data is stored in.

CProcessAreaData::CProcessAreaData(Context * pContext) : CProcess(pContext) {
    // create the area arrays
    m_dwAreaPos = 0;
    m_dwAreaLength = 0;
}

BOOL CProcessAreaData::SetArea(View * pView) {
    if (IsStatusFlag(KEEP_AREA)) {
        // not a new area
        return FALSE;
    }

    // get new area boundaries
    m_dwAreaPos = pView->GetStartCursorPosition();

    Model * pModel = pView->GetDocument();
    DWORD wSmpSize = pModel->GetSampleSize();
    m_dwAreaLength = pView->GetStopCursorPosition() - m_dwAreaPos + wSmpSize;

    SetStatusFlag(KEEP_AREA,TRUE);
    return TRUE;
}

/***************************************************************************/
// CAreaDataProcess::SetArea
/***************************************************************************/
BOOL CProcessAreaData::SetArea(DWORD dwAreaPos, DWORD dwAreaLength) {
    // get new area boundaries
    m_dwAreaPos = dwAreaPos;
    m_dwAreaLength = dwAreaLength;

    return TRUE;
}

void CProcessAreaData::UpdateArea() {
    SetStatusFlag(PROCESS_IDLE | KEEP_AREA,FALSE);
    SetDataInvalid();
}

// return area position
DWORD CProcessAreaData::GetAreaPosition() {
    return m_dwAreaPos;
}

// return area length
DWORD CProcessAreaData::GetAreaLength() {
    return m_dwAreaLength;
}
