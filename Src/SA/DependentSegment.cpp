#include "stdafx.h"
#include "DependentSegment.h"
#include "Sa_Doc.h"
#include "SA_View.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CDependentSegment
// class to do all the handling with dependent annotation segments. This
// class is the base class for segments, that depend on master annotation
// and must be aligned with it.

/////////////////////////////////////////////////////////////////////////////
// CDependentSegment helper functions

// SDM 1.5 Test 11.0
/***************************************************************************/
// CDependentSegment::AlignOffsetToSegment Align to the nearest nMaster segment
// Returns the index of the master segment, where *pdwOffset is within.
// If it's nowhere, it returns the index of the nearest (left) possible
// segment to align. If there is no alignment possible, it returns -1;
// If an index was found, the offset and the duration of the master
// segment will be placed in the memory spaces, the pointers pdwOffset and
// pdwDuration point to. The duration pointer may be NULL.
/***************************************************************************/
int CDependentSegment::AlignOffsetToMaster(ISaDoc * pSaDoc, DWORD * pdwOffset) const {
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    // get pointer to master offset array
    CSegment * pSegment = pDoc->GetSegment(m_nMasterType);
    int nLength = pSegment->GetOffsetSize();
    if (nLength == 0) {
        return -1;    // no master segment yet
    }
    // check the position
    int nLoop;
    for (nLoop = 0; nLoop < nLength; nLoop++) {
        DWORD dwOffset = pSegment->GetOffset(nLoop);
        if (*pdwOffset < dwOffset) { // this offset is larger
            break;
        }
    }
    nLoop = pSegment->GetPrevious(nLoop); // align to one position lower
    if (nLoop >= 0) {
        *pdwOffset = pSegment->GetOffset(nLoop); // get master offset
    }
    return nLoop;
}

// SDM 1.5 Test 11.0
/***************************************************************************/
// CDependentSegment::AlignStopToSegment Align to the nearest nMaster segment
/***************************************************************************/
int CDependentSegment::AlignStopToMaster(CDocument * pSaDoc, DWORD * pdwStop) const {
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    // get pointer to master offset array
    CSegment * pSegment = pDoc->GetSegment(m_nMasterType);
    int nLength = pSegment->GetOffsetSize();
    if (nLength == 0) {
        return -1;    // no master segment yet
    }
    // check the position
    int nLoop;
    for (nLoop = 0; nLoop < nLength; nLoop++) {
        DWORD dwStop = pSegment->GetStop(nLoop);
        if (*pdwStop < dwStop) { // this offset is larger
            break;
        }
    }
    nLoop = pSegment->GetPrevious(nLoop); // align to one position lower
    if (nLoop >= 0) {
        *pdwStop = pSegment->GetStop(nLoop); // get master offset
    }
    return nLoop;
}

// SDM 1.5 Test11.0 Modified to support new gloss stop position
/***************************************************************************/
// CDependentSegment::LimitPosition Limit positions for start and stop based
// on segment rules for update
/***************************************************************************/
void CDependentSegment::LimitPosition(CSaDoc * pSaDoc, DWORD & dwStart, DWORD & dwStop, ELimit) const {
    // get pointer to view
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    CSegment * pMaster = pDoc->GetSegment(m_nMasterType);

    int nTextIndex = GetSelection();

    if (nTextIndex != -1) {
        int nIndex;

        // Check for obscuring previous segment
        nIndex = GetPrevious(nTextIndex);
        if (nIndex != -1) {
            nIndex = pMaster->FindStop(GetStop(nIndex));
            if (nIndex !=-1) { // Properly Aligned to master
                nIndex = pMaster->GetNext(nIndex);
                if ((nIndex !=-1) && (dwStart < pMaster->GetOffset(nIndex))) {
                    dwStart = pMaster->GetOffset(nIndex);
                }
                if ((nIndex !=-1) && (dwStop < pMaster->GetStop(nIndex))) {
                    dwStop = pMaster->GetStop(nIndex);
                }
            }
        }
        // Check for obscuring next segment
        nIndex = GetNext(nTextIndex);
        if (nIndex != -1) {
            nIndex = pMaster->FindOffset(GetOffset(nIndex));
            if (nIndex !=-1) {
                nIndex = pMaster->GetPrevious(nIndex);
                if ((nIndex !=-1) && (dwStop > pMaster->GetStop(nIndex))) {
                    dwStop = pMaster->GetStop(nIndex);
                }
                if ((nIndex !=-1) && (dwStart > pMaster->GetOffset(nIndex))) {
                    dwStart = pMaster->GetOffset(nIndex);
                }
            }
        }

        AdjustPositionToMaster(pDoc, dwStart, dwStop);
    }
}


// SDM 1.5 Test11.0
/***************************************************************************/
// CDependentSegment::AdjustPositionToMasterAdjusts
// Position to apropriate master
// Adjust the positions to the nearest master boundary
//***************************************************************************/
int CDependentSegment::AdjustPositionToMaster(CDocument * pSaDoc, DWORD & dwNewOffset, DWORD & dwNewStop) const { // SDM 1.5Test8.2
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    DWORD dwAlignStart[2] = {
        dwNewOffset
    };
    DWORD dwAlignStop[2] = {
        dwNewStop
    };
    CSegment * pMaster = pDoc->GetSegment(m_nMasterType);
    int ret; // SDM 1.5Test8.2 added return parameter

    // Find Nearest master Start
    int nIndex = AlignOffsetToMaster(pDoc, dwAlignStart);
    if (nIndex == -1) {
        if (GetOffsetSize() > 0) {
            dwNewOffset = GetOffset(0);
            ret = 0;
        } else {
            // Should never get here, but we may... SDM 1.5Test8.2
            dwNewOffset = 0;
            ret = nIndex;
        }
    } else {
        // GetStart of Next Segment
        ret = nIndex;
        nIndex = pMaster->GetNext(nIndex);

        if (nIndex != -1) {
            dwAlignStart[1] = pMaster->GetOffset(nIndex);
            if ((dwAlignStart[1]-dwNewOffset) < (dwNewOffset - dwAlignStart[0])) {
                dwNewOffset = dwAlignStart[1];
                ret = nIndex;
            } else {
                dwNewOffset = dwAlignStart[0];
            }
        } else {
            dwNewOffset = dwAlignStart[0];
        }
    }

    // Find Nearest master Stop
    nIndex = AlignStopToMaster(pDoc, dwAlignStop);
    if (nIndex == -1) {
        if (GetOffsetSize() > 0) { // SDM 1.5Test11.0 if less than one segment stop is end of file
            dwNewStop = GetStop(0);
        } else {
            dwNewStop = 0;
        }
    } else {
        // Get Start of next segment
        nIndex = pMaster->GetNext(nIndex);
        if (nIndex != -1) {
            dwAlignStop[1] = pMaster->GetStop(nIndex);
        } else {
            dwAlignStop[1] = pMaster->GetStop(pMaster->GetOffsetSize() - 1);
        }

        if ((dwAlignStop[1]-dwNewStop) < (dwNewStop - dwAlignStop[0])) {
            dwNewStop = dwAlignStop[1];
        } else {
            dwNewStop = dwAlignStop[0];
        }
    }

    if (dwNewOffset >= dwNewStop) { // Force separate alignment
        nIndex = pMaster->FindStop(dwNewStop);
        if (nIndex!=-1) {
            nIndex = pMaster->GetNext(nIndex);
            if (nIndex != -1) {
                dwNewStop = pMaster->GetStop(nIndex);
            }
        }
    }
    if (dwNewOffset >= dwNewStop) { // Force separate alignment
        nIndex = pMaster->FindOffset(dwNewOffset);
        if (nIndex!=-1) {
            nIndex = pMaster->GetPrevious(nIndex);
            if (nIndex != -1) {
                dwNewOffset = pMaster->GetOffset(nIndex);
            } else
                ; // should never get here;
        }
    }
    return ret;
}

// SDM Added function 1.06.1.2
/***************************************************************************/
// CDependentSegment::AdjustCursorsToMaster
// Adjusts cursors to apropriate master
// Adjust the current cursor positions to the positions, that they will have,
// when a segment will be selected.
//***************************************************************************/
void CDependentSegment::AdjustCursorsToMaster(CDocument * pSaDoc, BOOL bAdjust, DWORD * pdwOffset, DWORD * pdwStop) const {
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);
    DWORD dwNewOffset = pView->GetStartCursorPosition();
    DWORD dwNewStop = pView->GetStopCursorPosition();

    AdjustPositionToMaster(pDoc, dwNewOffset, dwNewStop);

    if (pdwOffset) {
        *pdwOffset = dwNewOffset;
    }
    if (pdwStop) {
        *pdwStop = dwNewStop;
    }

    if (bAdjust) {
        // Adjust Cursors
        pView->SetStartCursorPosition(dwNewOffset);
        pView->SetStopCursorPosition(dwNewStop);
    }
}

/***************************************************************************/
// CDependentSegment::CheckPositionToMaster Check positions for dep. segments
// Checks the positions for validation. If they are ok for a new segment
// (depending on master segment positions), it returns the
// index, where to put them in in the arrays (0 based), otherwise -1. The
// start must not be placed in the range of a segment, where already
// another segment is aligned to, but there must be a segment to align to.
/***************************************************************************/
int CDependentSegment::CheckPositionToMaster(ISaDoc * pSaDoc, DWORD dwStart, DWORD dwStop, EMode nMode) const {
    // get pointer to view
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    CSegment * pMaster = pDoc->GetSegment(m_nMasterType);

    int nTextIndex = GetSelection();

    if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(nTextIndex != -1)) { // segment selected (edit)
        int nIndex;

        // get the actual aligned position
        AdjustPositionToMaster(pDoc, dwStart, dwStop);

        if (dwStart >= dwStop) {
            return -1;    // zero duration (or negative)
        }
        if (dwStart == GetOffset(nTextIndex) && (dwStop == GetStop(nTextIndex))) {
            return -1;    // no change
        }

        // Check for obscuring previous segment
        nIndex = GetPrevious(nTextIndex);
        if (nIndex != -1) {
            nIndex = pMaster->FindStop(GetStop(nIndex));
            if (nIndex !=-1) { // Properly Aligned to master
                nIndex = pMaster->GetNext(nIndex);
                if ((nIndex !=-1) && (dwStart < pMaster->GetOffset(nIndex))) {
                    return -1;
                }
                if ((nIndex !=-1) && (dwStop < pMaster->GetStop(nIndex))) {
                    return -1;
                }
            }
        }
        // Check for obscuring next segment
        nIndex = GetNext(nTextIndex);
        if (nIndex != -1) {
            nIndex = pMaster->FindOffset(GetOffset(nIndex));
            if (nIndex !=-1) {
                nIndex = pMaster->GetPrevious(nIndex);
                if ((nIndex !=-1) && (dwStop > pMaster->GetStop(nIndex))) {
                    return -1;
                }
                if ((nIndex !=-1) && (dwStart > pMaster->GetOffset(nIndex))) {
                    return -1;
                }
            }
        }
        return nTextIndex;
    } else { // Add
        int nSegment = AlignOffsetToMaster(pSaDoc, &dwStart);
        if (nSegment == -1) {
            return -1;    // wrong place
        }
        // now check if this position is already occupied
        int nLength = GetOffsetSize();
        if (nLength == 0) {
            return 0;    // no segment yet
        }
        int nLoop;
        for (nLoop = 0; nLoop < nLength; nLoop++) {
            if (dwStart <= GetOffset(nLoop)) { // this offset is bigger or equal
                if (dwStart == GetOffset(nLoop)) {
                    return -1;    // already occupied
                }
                return nLoop; // this is the position
            }
        }
        return nLoop; // append at the end
    }
}

// SDM 1.06.5
/***************************************************************************/
// CDependentSegment::Add Add dependent annotation segment
/***************************************************************************/
void CDependentSegment::Add(CSaDoc * pDoc, CSaView * pView, DWORD dwStart, CSaString & szString, bool /*delimiter*/, bool bCheck) {
    
	// get the offset and duration from master
    int nSegment = pDoc->GetSegment(m_nMasterType)->FindOffset(dwStart);
    if (nSegment == -1) {
		// return on error
        return;    
    }

    DWORD dwDuration = pDoc->GetSegment(m_nMasterType)->GetDuration(nSegment);
    DWORD dwStop = dwStart + dwDuration;

	// get the insert position
    int nPos = CheckPosition(pDoc, dwStart, dwStop, MODE_ADD); 
    if (nPos == -1) {
		// return on error
        return;    
    }

    // save state for undo ability
    if (bCheck) {
        pDoc->CheckPoint();
    }

    // insert or append the new dependent segment
    if (!Insert(nPos, szString, 0, dwStart, dwDuration)) {
		// return on error
        return;    
    }

	// document has been modified
    pDoc->SetModifiedFlag(TRUE);
	// transcription data has been modified
    pDoc->SetTransModifiedFlag(TRUE);
	// change the selection
    pView->ChangeAnnotationSelection(this, nPos, dwStart, dwStop);
}

int CDependentSegment::CheckPosition(ISaDoc * pDoc, DWORD dwStart, DWORD dwStop, EMode nMode, BOOL /*bUnused*/) const {
    return CheckPositionToMaster(pDoc, dwStart, dwStop, nMode);
}

/***************************************************************************/
// CDependentSegment::Remove Remove dependent annotation segment
/***************************************************************************/
void CDependentSegment::Remove(CSaDoc * pDoc, int sel, BOOL bCheck) {

    // save state for undo ability
    if (bCheck) {
        pDoc->CheckPoint();
    }

    RemoveAt(sel,false);

    // get pointer to view
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    // refresh ui
    pDoc->SetModifiedFlag(TRUE);                        // document has been modified
    pDoc->SetTransModifiedFlag(TRUE);                   // transcription data has been modified
    pView->ChangeAnnotationSelection(this, sel, 0, 0);  // deselect
    pView->RefreshGraphs(FALSE);                        // refresh the graphs between cursors
}

int CDependentSegment::Add( DWORD offset, DWORD duration) {
	return CSegment::Add(offset,duration);
}

