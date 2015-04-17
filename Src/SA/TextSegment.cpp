#include "stdafx.h"
#include "TextSegment.h"
#include "Process\Process.h"
#include "Segment.h"
#include "Process\sa_p_cha.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_fra.h"

#include "sa_asert.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "ArchiveTransfer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CTextSegment::CTextSegment(EAnnotation index, int master) : CDependentSegment(index,master) {
}

CTextSegment::~CTextSegment() {
}

/***************************************************************************/
// CTextSegment::Insert Insert a text segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CTextSegment::Insert(int nIndex, LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration) {

    // prepare delimiter
    CSaString szDelimiter = WORD_DELIMITER;
    if (delimiter) {
        szDelimiter.SetAt(0, TEXT_DELIMITER);
    }
    CString text;
    if (pszString!=NULL) {
        text = szDelimiter + pszString;
    } else {
        text = szDelimiter;
    }
    InsertAt(nIndex, (LPCTSTR)text, dwStart, dwDuration);
    return TRUE;
}

/***************************************************************************/
// CTextSegment::Append append a text segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CTextSegment::Append(LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration) {
    return Insert(GetOffsetSize(), pszString, delimiter, dwStart, dwDuration);
}

/***************************************************************************/
// CTextSegment::CaluculateDuration calculate segment duration from master data
/***************************************************************************/
DWORD CTextSegment::CalculateDuration(ISaDoc * pDoc, const int nIndex) const {

    CSegment * pMaster = (CSegment *)pDoc->GetSegment(m_nMasterType);
    int offset_size = GetOffsetSize();
    if ((nIndex < 0) || (nIndex >= offset_size)) {
        return DWORD(-1);
    }
    if ((nIndex + 1) == offset_size) {
        return pMaster->GetStop(pMaster->GetOffsetSize()-1) - GetOffset(nIndex);
    } else {
        DWORD next = GetNext(nIndex);
        DWORD offset = GetOffset(next);
        DWORD find_offset = pMaster->FindOffset(offset)-1;
        DWORD dwStop = pMaster->GetStop(find_offset);
        if (dwStop == -1) {
            return DWORD(-1);
        }
        return  dwStop - GetOffset(nIndex);
    }
}

// SDM 1.5 Test11.0 Modified to support new gloss stop position
/***************************************************************************/
// CTextSegment::CheckPositions Check positions for text segments
// Checks the positions for validation. If they are ok for a new segment
// (depending on master segment positions), it returns the
// index, where to put them in in the arrays (0 based), otherwise -1. The
// start cursor must not be placed in the range of a segment, where already
// another segment is aligned to, but there must be a segment to align to.
/***************************************************************************/
void CTextSegment::LimitPosition(CSaDoc * pSaDoc, DWORD & dwStart, DWORD & dwStop, ELimit) const {

    // get pointer to view
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    CSegment * pMaster = pDoc->GetSegment(m_nMasterType);

    int nTextIndex = GetSelection();

    if (nTextIndex != -1) {
        int nIndex;

        // Check for obscuring previous segment
        nIndex = GetPrevious(nTextIndex);
        if (nIndex != -1) {
            nIndex = pMaster->FindOffset(GetOffset(nIndex));
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
            nIndex = pMaster->FindStop(GetStop(nIndex));
            if (nIndex !=-1) {
                nIndex = pMaster->GetPrevious(nIndex);
                if ((nIndex !=-1) && (dwStop > pMaster->GetStop(nIndex))) {
                    dwStop = pMaster->GetStop(nIndex);
                }
                if ((nIndex !=-1) && (dwStart > pMaster->GetOffset(nIndex))) {
                    dwStart = pMaster->GetOffset(nIndex);
                }
            } else { // no master segment at stop boundary
                nIndex = GetNext(nTextIndex); // 1.5Test8.1
                DWORD dwOffset = GetStop(nIndex);
                nIndex = AlignStopToMaster(pSaDoc, &dwOffset); // find the nearest master segment to the end of next text segment
                if ((nIndex !=-1) && (dwStop > pMaster->GetStop(nIndex))) {
                    dwStop = pMaster->GetStop(nIndex);
                }
                if ((nIndex !=-1) && (dwStart > pMaster->GetOffset(nIndex))) {
                    dwStart = pMaster->GetOffset(nIndex);
                }
            }
        } else { // last gloss
            dwStop = pMaster->GetStop(pMaster->GetOffsetSize() - 1);
        }

        AdjustPositionToMaster(pDoc, dwStart, dwStop);
    }
}

// SDM Added function 1.06.1.2
/***************************************************************************/
// CTextSegment::CheckPositions Check positions for text segments
// Checks the positions for validation. If they are ok for a new segment
// (depending on master segment positions), it returns the
// index, where to put them in in the arrays (0 based), otherwise -1. The
// start cursor must not be placed in the range of a segment, where already
// another segment is aligned to, but there must be a segment to align to.
/***************************************************************************/
int CTextSegment::CheckPosition(ISaDoc * pSaDoc, DWORD dwStart, DWORD dwStop, EMode nMode, BOOL) const {

    // get pointer to view
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer

    // get the actual aligned position
    DWORD dwAlignedStart = dwStart;
    DWORD dwAlignedStop = dwStop;

    AdjustPositionToMaster(pDoc, dwAlignedStart, dwAlignedStop);

    int nTextIndex = GetSelection();

    if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&
            (nTextIndex != -1)) { // segment selected (edit)
        int nIndex;
        if (dwAlignedStart >= dwAlignedStop) {
            TRACE("start exceeds stop\n");
            return -1;    // zero duration (or negative)
        }
        if ((dwAlignedStart == GetOffset(nTextIndex)) &&
                (dwAlignedStop == GetStop(nTextIndex))) {
            TRACE("no change detected\n");
            return -1;    // no change
        }

        // Check for obscuring previous segment
        nIndex = GetPrevious(nTextIndex);
        if (nIndex != -1) {
            if (GetOffset(nIndex) >= dwAlignedStart) {
                TRACE("previous segment covered\n");
                return -1;
            }
        }
        // Check for obscuring present segment
        nIndex = GetNext(nTextIndex);
        if (nIndex != -1) {
            if (GetStop(nIndex) <= dwAlignedStop) {
                TRACE("next segment covered\n");
                return -1;
            }
        }
        return nTextIndex;
    } else { // Add
        if (pDoc->GetSegment(m_nMasterType)->IsEmpty()) {
            //TRACE("no segment\n");
            return -1;
        }
        if (IsEmpty()) {
            return 0;    // No Text in Segment insert at 0 (Start)
        }
        nTextIndex = 0; // GetNext does not work for -1
        while ((nTextIndex != -1) && (GetOffset(nTextIndex) < dwAlignedStart)) {
            nTextIndex = GetNext(nTextIndex);
        }

        if (nTextIndex == -1) {
            return GetOffsetSize();    // Insert at End
        }

        if (GetOffset(nTextIndex) == dwAlignedStart) {
            return -1;
        }

        return nTextIndex;
    }
    TRACE("unhandled rejection\n");
    return -1;
}

// SDM 1.06.5
/***************************************************************************/
// CTextSegment::Add Add text segment
/***************************************************************************/
void CTextSegment::Add(CSaDoc * pDoc, CSaView * pView, DWORD dwStart, CSaString & szString, bool bDelimiter, bool bCheck) {
    
	int nPos = FindFromPosition(dwStart,TRUE);
    if (nPos==-1) {
        nPos = FindFromPosition(dwStart,FALSE);
    } else {
        nPos++;
    }

    CSegment * pMaster = pDoc->GetSegment(m_nMasterType);
    int nMaster = -1;

    if ((nPos == -1) || (nPos >= GetOffsetSize()) || dwStart > GetStop(nPos)) {
        nMaster = pMaster->GetOffsetSize();
    } else {
        nMaster = pMaster->FindOffset(GetOffset(nPos));
    }

    ASSERT(nMaster > 0); // must be master segment to attach to

    DWORD dwStop = pMaster->GetStop(nMaster-1);

    ASSERT(dwStop > dwStart);

    nPos = CheckPosition(pDoc, dwStart, dwStop, CSegment::MODE_ADD);  // get the insert position
    if (nPos == -1) {
        return;    // return on error
    }

    ASSERT(dwStop > dwStart);

    // save state for undo ability
    if (bCheck) {
        pDoc->CheckPoint();
    }

    // insert or append the new segment
    if (!Insert(nPos, szString, bDelimiter, dwStart, dwStop - dwStart)) {
        return;    // return on error
    }

    // move the end of the previous text segment
    if (nPos > 0) {
        Adjust(pDoc, nPos - 1, GetOffset(nPos - 1), CalculateDuration( pDoc, nPos -1), false);
    }

	// document has been modified
    pDoc->SetModifiedFlag(TRUE);
	// transcription data has been modified
    pDoc->SetTransModifiedFlag(TRUE); 
	// change the selection
    pView->ChangeAnnotationSelection(this, nPos);
}

