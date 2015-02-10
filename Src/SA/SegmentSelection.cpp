/////////////////////////////////////////////////////////////////////////////
// sa_ansel.cpp
// Implementation of the CSegmentSelection class
// Author: Steve MacLean (SDM)
// copyright 1999 JAARS Inc. SIL
//
// Revision History
//   1.06.5
//        SDM Original
//   1.06.6
//        SDM Added BOOL return to SelectFromPosition to note selection change
//   1.5Test8.1
//        SDM Added support for dependent gloss segments
//   1.5Test11.0
//        SDM added replaced GetOffset() + GetDuration() with CSegment::GetStop()
//   1.5Test11.4
//        SDM changed virtual selections to handle phonemic/ortho/tone spanning multiple segments
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SegmentSelection.h"
#include "Process\Process.h"
#include "Segment.h"
#include "sa_graph.h"

#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "DependentSegment.h"

CSegmentSelection::CSegmentSelection() {
    m_nIndex=-1;
    m_bVirtual = FALSE;
}

/***************************************************************************/
// CSegmentSelection::SelectFromPosition Select an annotation segment from
// a position. Returns TRUE if selection changed else FALSE
/***************************************************************************/
BOOL CSegmentSelection::SelectFromPosition(CSaView * pView, int nSegmentIndex, DWORD dwPosition, bool bFindExact) {

    TRACE("SelectFromPosition view=%lp segmentindex=%d position=%d findexact=%d\n",pView,nSegmentIndex,dwPosition,bFindExact);
    // Get original selection for return test
    Update(pView);
    // These are used later to see if we changed...
    int nAnnotationIndex = m_nIndex;
    DWORD dwStart = m_dwStart;

    // get pointer to segment object
    CSegment * pSegment = pView->GetDocument()->GetSegment(nSegmentIndex);
    CSaDoc * pDoc = pView->GetDocument();

    BOOL bWithin = TRUE;
    if ((nSegmentIndex == GLOSS)||(nSegmentIndex == PHONETIC)) {
        bWithin = FALSE;
    }

    // find the master segment
    // gloss->phonetic
    // phonemic->phonetic
    // ortho-.phonetic
    int nMaster = pSegment->GetMasterIndex();
    if (nMaster == -1) {
        nMaster = nSegmentIndex;
    }
    CSegment * pMaster = pView->GetDocument()->GetSegment(nMaster);

    // get pointer to annotation offsets
    // if there is at least one segment
    if (!pSegment->IsEmpty()) {
        // get the segment index at the given position
        // More precise less robust
        int nSelection = pSegment->FindOffset(dwPosition);
        if ((nSelection==-1) && (!(bWithin && bFindExact))) {
            // Better Coverage
            nSelection = pSegment->FindFromPosition(dwPosition, bWithin);
        }

        if ((nSelection==-1) && (bWithin && bFindExact) && (pSegment->GetSelection() == -1)) {
            int nIndex = pMaster->FindOffset(dwPosition);
            if (nIndex != -1) {
                nSelection = pSegment->FindFromPosition(pMaster->GetOffset(nIndex) + pMaster->GetDuration(nIndex)/2, bWithin);
            }
        }
        // change the selection
        if (nSelection!=-1) {
            if (nSelection != pSegment->GetSelection()) {
                //Don't deselect
                pView->ChangeAnnotationSelection(pSegment, nSelection, pSegment->GetOffset(nSelection), pSegment->GetStop(nSelection));
            }
        } else {
            pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
        }
    } else {
        pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
    }

    // Does the master have a different index?
    // Is our segment still unselected?
    if ((nMaster != nSegmentIndex) &&
            (pSegment->GetSelection() == -1)) {

        // More precise less robust
        int nSelection = pMaster->FindOffset(dwPosition);
        if ((nSelection==-1)) {
            nSelection = pMaster->FindFromPosition(dwPosition, bWithin);
        }
        if (nSelection==-1) {
            nSelection = pMaster->FindFromPosition(dwPosition, FALSE);
        }

        // special case for gloss segments.
        bool bUseable = true;
        if (nSegmentIndex == GLOSS) {
            bUseable = false;
        }

        // we've selected something via the master
        if ((nSelection != -1)  && (bUseable)) {

            //Set up virtual selection
            // Deselect Virtual Selection
            if (m_bVirtual) {
                RefreshAnnotation(pView, m_nIndex);
            }
            m_bVirtual = TRUE;
            m_nIndex = nSegmentIndex;
            m_dwStart = pMaster->GetOffset(nSelection);
            m_dwDuration = pMaster->GetDuration(nSelection);
            m_dwStop = m_dwStart + m_dwDuration;
            pView->SetStartCursorPosition(m_dwStart, SNAP_RIGHT);
            pView->SetStopCursorPosition(m_dwStop, SNAP_LEFT);
        } else {
            // Deselect Virtual Selection
            if (m_bVirtual) {
                RefreshAnnotation(pView, m_nIndex);
            }
            m_bVirtual = FALSE;
            m_nIndex = -1;
            m_dwStart = 0;
            m_dwDuration = 0;
            m_dwStop = 0;
        }
    }

    // highlight possible insertion point
    if ((nMaster == nSegmentIndex) &&
            (nMaster != PHONETIC) &&
            (pSegment->GetSelection() == -1)) {

        DWORD dwStart = dwPosition; // Start at current stop
        DWORD dwStop;

        // Snap Start Position
        dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, 0, dwStart, SNAP_LEFT);
        dwStop = (dwStart + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));

        if (pDoc->Is16Bit()) {
            dwStop = (dwStop + 1) & ~1; // Round up
        }

        dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStop, pDoc->GetDataSize(), SNAP_RIGHT);

        int nInsertAt = pSegment->CheckPosition(pDoc,dwStart,dwStop,CSegment::MODE_ADD);
        if (nInsertAt != -1) {
            // Deselect Virtual Selection
            if (m_bVirtual) {
                RefreshAnnotation(pView, m_nIndex);
            }

            if (nInsertAt > 0) {
                if ((pSegment->GetStop(nInsertAt-1) +  pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) > dwStart) {
                    dwStart = pSegment->GetStop(nInsertAt-1);
                }
            }

            if (nInsertAt < pSegment->GetOffsetSize()) {
                if ((dwStart+pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < pSegment->GetOffset(nInsertAt)) {
                    dwStop = (dwStart + pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
                } else {
                    dwStop = pSegment->GetOffset(nInsertAt);
                }
            } else {
                if ((dwStart+pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < pDoc->GetDataSize()) {
                    dwStop = (dwStart + pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
                } else {
                    dwStop = pDoc->GetDataSize();
                }
            }

            if (pDoc->Is16Bit()) {
                dwStop = (dwStop + 1) & ~1; // Round up
            }

            dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, dwStart, pDoc->GetDataSize(), SNAP_RIGHT);
            dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, 0, dwStop, SNAP_LEFT);

            m_bVirtual = TRUE;
            m_nIndex = nSegmentIndex;
            m_dwStart = dwStart;
            m_dwDuration = dwStop-dwStart;
            m_dwStop = dwStop;
            pView->SetStartCursorPosition(m_dwStart, SNAP_RIGHT);
            pView->SetStopCursorPosition(m_dwStop, SNAP_LEFT);

        } else {
            // Deselect Virtual Selection
            if (m_bVirtual) {
                RefreshAnnotation(pView, m_nIndex);
            }
            m_bVirtual = FALSE;
            m_nIndex = -1;
            m_dwStart = 0;
            m_dwDuration = 0;
            m_dwStop = 0;
        }
    }
    Update(pView);
    return !((nAnnotationIndex == m_nIndex)&&(dwStart == m_dwStart));
}

/***************************************************************************/
// CSegmentSelection::SelectFromStopPosition Select a annotation segment from
// a position. Returns TRUE if selection changed else FALSE
/***************************************************************************/
BOOL CSegmentSelection::SelectFromStopPosition(CSaView * pView, int nSegmentIndex, DWORD dwStop, bool bFindExact) {

    TRACE("SelectFromStopPosition view=%lp segmentindex=%d stop=%d findexact=%d\n",pView,nSegmentIndex,dwStop,bFindExact);

    // Get original selection for return test
    Update(pView);
    int nAnnotationIndex = m_nIndex;
    DWORD dwStart = m_dwStart;

    // get pointer to segment object
    CSegment * pSegment = pView->GetDocument()->GetSegment(nSegmentIndex);
    CSaDoc * pDoc = pView->GetDocument();
    BOOL bWithin = TRUE;
    if ((nSegmentIndex == GLOSS)||(nSegmentIndex == PHONETIC)) {
        bWithin = FALSE;
    }

    int nMaster = pSegment->GetMasterIndex();
    if (nMaster == -1) {
        nMaster = nSegmentIndex;
    }
    CSegment * pMaster = pView->GetDocument()->GetSegment(nMaster);

    // get pointer to annotation offsets
    if (!(pSegment->IsEmpty())) { // there is at least one segment
        // get the segment index at the given position
        int nSelection = pSegment->FindStop(dwStop); // More precise less robust
        if ((nSelection==-1) &&
                (!(bWithin && bFindExact))) {
            nSelection = pSegment->FindFromPosition(dwStop, bWithin);    // Better Coverage
        }

        if ((nSelection==-1) &&
                (bWithin && bFindExact) &&
                (pSegment->GetSelection() == -1)) {
            int nIndex = pMaster->FindStop(dwStop);
            if (nIndex != -1) {
                nSelection = pSegment->FindFromPosition(pMaster->GetOffset(nIndex) + pMaster->GetDuration(nIndex)/2, bWithin);
            }
        }
        // change the selection
        if (nSelection!=-1) {
            if (nSelection != pSegment->GetSelection()) { //Don't deselect
                pView->ChangeAnnotationSelection(pSegment, nSelection, pSegment->GetOffset(nSelection), pSegment->GetStop(nSelection));
            }
        } else {
            pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
        }
    } else {
        pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
    }

    if ((nMaster != nSegmentIndex) && (pSegment->GetSelection() == -1)) {
        // More precise less robust
        int nSelection = pMaster->FindStop(dwStop);
        if ((nSelection==-1)) {
            nSelection = pMaster->FindFromPosition(dwStop, bWithin);
        }
        if (nSelection==-1) {
            nSelection = pMaster->FindFromPosition(dwStop, FALSE);
        }

        //Set up virtual selection
        if (nSelection != -1) {
            // Deselect Virtual Selection
            if (m_bVirtual) {
                RefreshAnnotation(pView, m_nIndex);
            }
            m_bVirtual = TRUE;
            m_nIndex = nSegmentIndex;
            m_dwStart = pMaster->GetOffset(nSelection);
            m_dwDuration = pMaster->GetDuration(nSelection);
            m_dwStop = m_dwStart + m_dwDuration;
            pView->SetStartCursorPosition(m_dwStart, SNAP_RIGHT);
            pView->SetStopCursorPosition(m_dwStop, SNAP_LEFT);
        } else {
            // Deselect Virtual Selection
            if (m_bVirtual) {
                RefreshAnnotation(pView, m_nIndex);
            }
            m_bVirtual = FALSE;
            m_nIndex = -1;
            m_dwStart = 0;
            m_dwDuration = 0;
            m_dwStop = 0;
        }
    }

    // highlight possible insertion point
    if ((nMaster == nSegmentIndex) &&
            (nMaster != PHONETIC) &&
            (pSegment->GetSelection() == -1)) {
        DWORD dwStart = 0;

        dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStop, pDoc->GetDataSize(), SNAP_RIGHT);

        // Snap Start Position
        if (dwStop > pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME)) {
            dwStart = dwStop - pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME);
        } else {
            dwStart = 0;
        }

        if (pDoc->Is16Bit()) { // SDM 1.5Test8.2
            dwStart = (dwStart + 1) & ~1; // Round up
        }

        dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, 0, dwStart, SNAP_LEFT);

        int nInsertAt = pSegment->CheckPosition(pDoc,dwStart,dwStop,CSegment::MODE_ADD);
        if (nInsertAt != -1) {
            // Deselect Virtual Selection
            if (m_bVirtual) {
                RefreshAnnotation(pView, m_nIndex);
            }

            if (nInsertAt < pSegment->GetOffsetSize())
                if (pSegment->GetOffset(nInsertAt) < (dwStop +  pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME))) {
                    dwStop = pSegment->GetOffset(nInsertAt);
                }

            if (nInsertAt > 0) {
                if ((pSegment->GetStop(nInsertAt-1)+pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < dwStop) {
                    dwStart = (dwStop - pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
                } else {
                    dwStart = pSegment->GetStop(nInsertAt-1);
                }
            } else {
                if ((dwStart+pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < pDoc->GetDataSize()) {
                    dwStart = (dwStop - pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
                } else {
                    dwStart = 0;
                }
            }

            if (pDoc->Is16Bit()) {
                dwStart = (dwStart + 1) & ~1; // Round up
            }

            dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, dwStart, pDoc->GetDataSize(), SNAP_RIGHT);
            dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, 0, dwStop, SNAP_LEFT);

            m_bVirtual = TRUE;
            m_nIndex = nSegmentIndex;
            m_dwStart = dwStart;
            m_dwDuration = dwStop-dwStart;
            m_dwStop = dwStop;
            pView->SetStartCursorPosition(m_dwStart, SNAP_RIGHT);
            pView->SetStopCursorPosition(m_dwStop, SNAP_LEFT);
        } else {
            // Deselect Virtual Selection
            if (m_bVirtual) {
                RefreshAnnotation(pView, m_nIndex);
            }
            m_bVirtual = FALSE;
            m_nIndex = -1;
            m_dwStart = 0;
            m_dwDuration = 0;
            m_dwStop = 0;
        }
    }
    Update(pView);
    return !((nAnnotationIndex == m_nIndex)&&(dwStart == m_dwStart));
}

/***************************************************************************/
// CSegmentSelection::Update update selection data for outside activity
// This keeps the virtual selection up to date with activity which modified
// the segments or their selection
/***************************************************************************/
void CSegmentSelection::Update(CSaView * pView, BOOL bClearVirtual) {

    if (pView->IsAnyAnnotationSelected()) {
        // Deselect Virtual Selection
        if (m_bVirtual) {
            RefreshAnnotation(pView, m_nIndex);
        }
        m_bVirtual = FALSE;
        m_nIndex = pView->FindSelectedAnnotationIndex();

        CSegment * pSegment = pView->GetDocument()->GetSegment(m_nIndex);
        int nSelection = pSegment->GetSelection();

        m_dwStart = pSegment->GetOffset(nSelection);
        m_dwDuration = pSegment->GetDuration(nSelection);
        m_dwStop = m_dwStart + m_dwDuration;
    } else {
        if (bClearVirtual && m_bVirtual) {
            RefreshAnnotation(pView, m_nIndex);
            m_bVirtual = FALSE;
        }
        if (!m_bVirtual) {
            m_nIndex = -1;
            m_dwStart = 0;
            m_dwDuration = 0;
            m_dwStop = 0;
        }
    }
};

/***************************************************************************/
// CSegmentSelection::RefreshAnnotation
// Redraw annotation losing selection as we find out.
/***************************************************************************/
void CSegmentSelection::RefreshAnnotation(CSaView * pView, int nAnnotationIndex) {

    if ((nAnnotationIndex < 0) || (nAnnotationIndex >= ANNOT_WND_NUMBER)) {
        return;
    }

    for (int nGraphLoop = 0; nGraphLoop < MAX_GRAPHS_NUMBER; nGraphLoop++) {
        if (pView->m_apGraphs[nGraphLoop]) {
            pView->m_apGraphs[nGraphLoop]->ChangeAnnotationSelection(nAnnotationIndex);
        }
    }
}

/***************************************************************************/
// CSegmentSelection::GetSelectedAnnotationString
// returns selected annotation string or null string if virtual selection
// optionally removes delimiters
/***************************************************************************/
CString CSegmentSelection::GetSelectedAnnotationString(CSaView * pView, BOOL bRemoveDelimiter) const {

    if (m_bVirtual) {
        return "";
    }

    CString ret = pView->GetSelectedAnnotationString();

    if (bRemoveDelimiter) {
        if ((ret[0] == SEGMENT_DEFAULT_CHAR) && (m_nIndex != GLOSS)) {
            return ret.Mid(1);
        }

        if (((ret[0] == TEXT_DELIMITER)||(ret[0] == WORD_DELIMITER)) && (m_nIndex == GLOSS)) {
            return ret.Mid(1);
        }
    }

    return ret;
}

/***************************************************************************/
// CSegmentSelection::SetSelectedAnnotationString
// Sets the selected annotation string.  Adds delimiter if necessary,
// removes empty dependent segments.  Refreshes graphs, sets modified flag
// and optionally adds CheckPoint()
/***************************************************************************/
BOOL CSegmentSelection::SetSelectedAnnotationString( CSaView * pView, CSaString & szString, BOOL bIncludesDelimiter, BOOL bCheck) {
    
	if (m_nIndex == -1) {
        return FALSE;
    }

    int nIndex = m_nIndex;
    CSegment * pSegment = pView->GetAnnotation(nIndex);
    CSaDoc * pDoc = pView->GetDocument();

    // include delimiter;
    if ((!bIncludesDelimiter) && (nIndex == GLOSS)) {
        CString szDelimiter = GetSelectedAnnotationString(pView, FALSE).Left(1);
        szString = szDelimiter + szString;
    }

    if (szString.GetLength() == 0) {
        if (nIndex == GLOSS) {
            szString = WORD_DELIMITER;
        } else if (nIndex == PHONETIC) {
            szString = SEGMENT_DEFAULT_CHAR;
        }
    }

    // Save cursor positions
    DWORD dwStart = pView->GetStartCursorPosition();
    DWORD dwStop = pView->GetStopCursorPosition();

    // Set cursors to segment boundaries to avoid changes
    pView->SetStartCursorPosition(m_dwStart);
    pView->SetStopCursorPosition(m_dwStop);

    if (m_bVirtual) {
        if ((nIndex == GLOSS)||(nIndex == PHONETIC)) {
            return FALSE;
        }

        // Add refreshes graphs, set modified flag, & check point
        if (pSegment->GetMasterIndex() != -1) {
            ((CDependentSegment *) pSegment)->Add(pView->GetDocument(), pView, m_dwStart, szString, FALSE, bCheck);
        } else {
            int nInsertAt = pSegment->CheckPosition( pDoc, m_dwStart, m_dwStop, CSegment::MODE_ADD);
            if (nInsertAt != -1) {
                if (bCheck) {
                    pDoc->CheckPoint();
                }
                pDoc->SetModifiedFlag(TRUE); // document has been modified
                pDoc->SetTransModifiedFlag(TRUE); // transcription has been modified
                pSegment->Insert(nInsertAt, szString, true, m_dwStart,m_dwDuration);
                pView->ChangeAnnotationSelection( pSegment, nInsertAt);
            }
        }
    } else if (szString.GetLength() == 0) { 
		// We need to remove the dependent segment
        DWORD dwPosition = m_dwStart;
        // Remove refreshes graphs, set modified flag, & check point
		int index = pSegment->GetSelection();
		if (index != -1) {
			pSegment->Remove( pView->GetDocument(), index, bCheck);
		}
        SelectFromPosition(pView, nIndex, dwPosition, true);
    } else {
        if (bCheck) {
            pView->GetDocument()->CheckPoint();
        }
        // ReplaceSelectedSegment refreshes graphs, sets modified flag
        pSegment->ReplaceSelectedSegment(pView->GetDocument(),szString);
    }

    pView->SetStartCursorPosition(dwStart, SNAP_RIGHT);
    pView->SetStopCursorPosition(dwStop, SNAP_LEFT);

    Update(pView);
    return TRUE;
}

DWORD CSegmentSelection::GetSelectionStart() {
    return m_dwStart;
}

DWORD CSegmentSelection::GetSelectionStop() {
    return m_dwStop;
}

int CSegmentSelection::GetSelectionIndex() {
    return m_nIndex;
}

bool CSegmentSelection::IsSelectionVirtual() {
    return m_bVirtual;
}
