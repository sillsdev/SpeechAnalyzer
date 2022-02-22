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
#include "Segment.h"
#include "sa_graph.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "DependentSegment.h"

CSegmentSelection::CSegmentSelection() {
	m_nType = -1;
	m_bVirtual = false;
}

/***************************************************************************/
// CSegmentSelection::SelectFromPosition Select an annotation segment from
// a position. 
// Returns TRUE if selection changed else FALSE
/***************************************************************************/
BOOL CSegmentSelection::SelectFromPosition(CSaView * pView, int type, DWORD dwPosition, bool bFindExact) {
	TRACE("SelectFromPosition\n");
	TRACE("SelectFromPosition view=%lp segmentindex=%d position=%d findexact=%d\n", pView, type, dwPosition, bFindExact);
	// Get original selection for return test
	Update(pView);
	// These are used later to see if we changed...
	int originalType = m_nType;
	DWORD originalStart = m_dwStart;

	// get pointer to segment object
	CSaDoc * pModel = pView->GetDocument();
	CSegment * pSegment = pModel->GetSegment(type);

	BOOL bWithin = TRUE;
	if ((type == GLOSS) || (type == PHONETIC)) {
		bWithin = FALSE;
	}

	// find the master segment
	// gloss->phonetic
	// phonemic->phonetic
	// ortho-.phonetic
	int nMaster = pSegment->GetMasterIndex();
	if (nMaster == -1) {
		nMaster = type;
	}
	CSegment * pMaster = pModel->GetSegment(nMaster);

	// get pointer to annotation offsets
	// if there is at least one segment
	if (!pSegment->IsEmpty()) {
		// get the segment index at the given position
		// More precise less robust
		int nSelection = pSegment->FindOffset(dwPosition);
		if ((nSelection == -1) && (!(bWithin && bFindExact))) {
			// Better Coverage
			nSelection = pSegment->FindFromPosition(dwPosition, bWithin);
		}

		if ((nSelection == -1) && (bWithin && bFindExact) && (pSegment->GetSelection() == -1)) {
			int nIndex = pMaster->FindOffset(dwPosition);
			if (nIndex != -1) {
				nSelection = pSegment->FindFromPosition(pMaster->GetOffset(nIndex) + pMaster->GetDuration(nIndex) / 2, bWithin);
			}
		}
		// change the selection
		if (nSelection != -1) {
			if (nSelection != pSegment->GetSelection()) {
				//Don't deselect
				pView->ChangeAnnotationSelection(pSegment, nSelection);
			}
		} else {
			pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
		}
	} else {
		pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
	}

	// Does the master have a different index?
	// Is our segment still unselected?
	if ((nMaster != type) && (pSegment->GetSelection() == -1)) {

		// More precise less robust
		int nSelection = pMaster->FindOffset(dwPosition);
		if ((nSelection == -1)) {
			nSelection = pMaster->FindFromPosition(dwPosition, bWithin);
		}
		if (nSelection == -1) {
			nSelection = pMaster->FindFromPosition(dwPosition, FALSE);
		}

		// special case for gloss segments.
		bool bUseable = true;
		if (type == GLOSS) {
			bUseable = false;
		}

		// we've selected something via the master
		if ((nSelection != -1) && (bUseable)) {

			//Set up virtual selection
			// Deselect Virtual Selection
			if (m_bVirtual) {
				RefreshAnnotation(pView, m_nType);
			}
			m_bVirtual = true;
			m_nType = type;
			m_dwStart = pMaster->GetOffset(nSelection);
			m_dwDuration = pMaster->GetDuration(nSelection);
			pView->SetStartCursorPosition(m_dwStart, SNAP_RIGHT);
			pView->SetStopCursorPosition(m_dwStart+m_dwDuration, SNAP_LEFT);
		} else {
			// Deselect Virtual Selection
			if (m_bVirtual) {
				RefreshAnnotation(pView, m_nType);
			}
			m_bVirtual = false;
			m_nType = -1;
			m_dwStart = 0;
			m_dwDuration = 0;
		}
	}

	// highlight possible insertion point
	if ((nMaster == type) &&
		(nMaster != PHONETIC) &&
		(pSegment->GetSelection() == -1)) {

		// Start at current stop
		DWORD dwStart = dwPosition;

		// Snap Start Position
		dwStart = pModel->SnapCursor(START_CURSOR, dwStart, 0, dwStart, SNAP_LEFT);
		DWORD dwStop = (dwStart + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));

		if (pModel->Is16Bit()) {
			// Round up
			dwStop = (dwStop + 1) & ~1;
		}

		dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStop, pModel->GetDataSize(), SNAP_RIGHT);

		int nInsertAt = pSegment->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_ADD);
		if (nInsertAt != -1) {
			// Deselect Virtual Selection
			if (m_bVirtual) {
				RefreshAnnotation(pView, m_nType);
			}

			if (nInsertAt > 0) {
				if ((pSegment->GetStop(nInsertAt - 1) + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) > dwStart) {
					dwStart = pSegment->GetStop(nInsertAt - 1);
				}
			}

			if (nInsertAt < pSegment->GetOffsetSize()) {
				if ((dwStart + pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < pSegment->GetOffset(nInsertAt)) {
					dwStop = (dwStart + pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
				} else {
					dwStop = pSegment->GetOffset(nInsertAt);
				}
			} else {
				if ((dwStart + pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < pModel->GetDataSize()) {
					dwStop = (dwStart + pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
				} else {
					dwStop = pModel->GetDataSize();
				}
			}

			if (pModel->Is16Bit()) {
				// Round up
				dwStop = (dwStop + 1) & ~1;
			}

			dwStart = pModel->SnapCursor(START_CURSOR, dwStart, dwStart, pModel->GetDataSize(), SNAP_RIGHT);
			dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, 0, dwStop, SNAP_LEFT);

			m_bVirtual = true;
			m_nType = type;
			m_dwStart = dwStart;
			m_dwDuration = dwStop - dwStart;
			pView->SetStartCursorPosition(m_dwStart, SNAP_RIGHT);
			pView->SetStopCursorPosition(m_dwStart+m_dwDuration, SNAP_LEFT);

		} else {
			// Deselect Virtual Selection
			if (m_bVirtual) {
				RefreshAnnotation(pView, m_nType);
			}
			m_bVirtual = false;
			m_nType = -1;
			m_dwStart = 0;
			m_dwDuration = 0;
		}
	}
	Update(pView);
	return !((originalType == m_nType) && (originalStart == m_dwStart));
}

/***************************************************************************/
// CSegmentSelection::SelectFromStopPosition Select a annotation segment from
// a position. Returns TRUE if selection changed else FALSE
/***************************************************************************/
BOOL CSegmentSelection::SelectFromStopPosition(CSaView * pView, int type, DWORD dwStop, bool bFindExact) {
	TRACE("SelectFromStopPosition\n");
	TRACE("SelectFromStopPosition view=%lp segmentindex=%d stop=%d findexact=%d\n", pView, type, dwStop, bFindExact);
	// Get original selection for return test
	Update(pView);
	int originalType = m_nType;
	DWORD originalStart = m_dwStart;

	// get pointer to segment object
	CSegment * pSegment = pView->GetDocument()->GetSegment(type);
	CSaDoc * pModel = pView->GetDocument();
	BOOL bWithin = TRUE;
	if ((type == GLOSS) || (type == PHONETIC)) {
		bWithin = FALSE;
	}

	int nMaster = pSegment->GetMasterIndex();
	if (nMaster == -1) {
		nMaster = type;
	}
	CSegment * pMaster = pView->GetDocument()->GetSegment(nMaster);

	// get pointer to annotation offsets
	if (!(pSegment->IsEmpty())) {
		// there is at least one segment
		// get the segment index at the given position
		// More precise less robust
		int nSelection = pSegment->FindStop(dwStop);
		if ((nSelection == -1) && (!(bWithin && bFindExact))) {
			// Better Coverage
			nSelection = pSegment->FindFromPosition(dwStop, bWithin);
		}

		if ((nSelection == -1) &&
			(bWithin && bFindExact) &&
			(pSegment->GetSelection() == -1)) {
			int nIndex = pMaster->FindStop(dwStop);
			if (nIndex != -1) {
				nSelection = pSegment->FindFromPosition(pMaster->GetOffset(nIndex) + pMaster->GetDuration(nIndex) / 2, bWithin);
			}
		}
		// change the selection
		if (nSelection != -1) {
			if (nSelection != pSegment->GetSelection()) {
				//Don't deselect
				pView->ChangeAnnotationSelection(pSegment, nSelection);
			}
		} else {
			pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
		}
	} else {
		pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
	}

	if ((nMaster != type) && (pSegment->GetSelection() == -1)) {
		// More precise less robust
		int nSelection = pMaster->FindStop(dwStop);
		if ((nSelection == -1)) {
			nSelection = pMaster->FindFromPosition(dwStop, bWithin);
		}
		if (nSelection == -1) {
			nSelection = pMaster->FindFromPosition(dwStop, FALSE);
		}

		//Set up virtual selection
		if (nSelection != -1) {
			// Deselect Virtual Selection
			if (m_bVirtual) {
				RefreshAnnotation(pView, m_nType);
			}
			m_bVirtual = true;
			m_nType = type;
			m_dwStart = pMaster->GetOffset(nSelection);
			m_dwDuration = pMaster->GetDuration(nSelection);
			pView->SetStartCursorPosition(m_dwStart, SNAP_RIGHT);
			pView->SetStopCursorPosition(m_dwStart + m_dwDuration, SNAP_LEFT);
		} else {
			// Deselect Virtual Selection
			if (m_bVirtual) {
				RefreshAnnotation(pView, m_nType);
			}
			m_bVirtual = false;
			m_nType = -1;
			m_dwStart = 0;
			m_dwDuration = 0;
		}
	}

	// highlight possible insertion point
	if ((nMaster == type) && (nMaster != PHONETIC) && (pSegment->GetSelection() == -1)) {
		DWORD dwStart = 0;

		dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStop, pModel->GetDataSize(), SNAP_RIGHT);

		// Snap Start Position
		if (dwStop > pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME)) {
			dwStart = dwStop - pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME);
		} else {
			dwStart = 0;
		}

		if (pModel->Is16Bit()) {
			// SDM 1.5Test8.2
			// Round up
			dwStart = (dwStart + 1) & ~1;
		}

		dwStart = pModel->SnapCursor(START_CURSOR, dwStart, 0, dwStart, SNAP_LEFT);

		int nInsertAt = pSegment->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_ADD);
		if (nInsertAt != -1) {
			// Deselect Virtual Selection
			if (m_bVirtual) {
				RefreshAnnotation(pView, m_nType);
			}

			if (nInsertAt < pSegment->GetOffsetSize())
				if (pSegment->GetOffset(nInsertAt) < (dwStop + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME))) {
					dwStop = pSegment->GetOffset(nInsertAt);
				}

			if (nInsertAt > 0) {
				if ((pSegment->GetStop(nInsertAt - 1) + pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < dwStop) {
					dwStart = (dwStop - pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
				} else {
					dwStart = pSegment->GetStop(nInsertAt - 1);
				}
			} else {
				if ((dwStart + pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < pModel->GetDataSize()) {
					dwStart = (dwStop - pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
				} else {
					dwStart = 0;
				}
			}

			if (pModel->Is16Bit()) {
				// Round up
				dwStart = (dwStart + 1) & ~1;
			}

			dwStart = pModel->SnapCursor(START_CURSOR, dwStart, dwStart, pModel->GetDataSize(), SNAP_RIGHT);
			dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, 0, dwStop, SNAP_LEFT);

			m_bVirtual = true;
			m_nType = type;
			m_dwStart = dwStart;
			m_dwDuration = dwStop - dwStart;
			pView->SetStartCursorPosition(m_dwStart, SNAP_RIGHT);
			pView->SetStopCursorPosition(m_dwStart+m_dwDuration, SNAP_LEFT);
		} else {
			// Deselect Virtual Selection
			if (m_bVirtual) {
				RefreshAnnotation(pView, m_nType);
			}
			m_bVirtual = false;
			m_nType = -1;
			m_dwStart = 0;
			m_dwDuration = 0;
		}
	}
	Update(pView);
	return !((originalType == m_nType) && (originalStart == m_dwStart));
}

/***************************************************************************
* CSegmentSelection::Update
* update selection data for outside activity
* This keeps the virtual selection up to date with activity which modified
* the segments or their selection
***************************************************************************/
void CSegmentSelection::Update(CSaView * pView, BOOL bClearVirtual) {

	if (pView->IsAnyAnnotationSelected()) {
		// Deselect Virtual Selection
		if (m_bVirtual) {
			RefreshAnnotation(pView, m_nType);
		}
		m_bVirtual = false;
		m_nType = (EAnnotation)pView->FindSelectedAnnotationIndex();

		CSegment * pSegment = pView->GetDocument()->GetSegment(m_nType);
		int nSelection = pSegment->GetSelection();

		m_dwStart = pSegment->GetOffset(nSelection);
		m_dwDuration = pSegment->GetDuration(nSelection);
		return;
	}
	// no annotations were selected
	if ((bClearVirtual) && (m_bVirtual)) {
		RefreshAnnotation(pView, m_nType);
		m_bVirtual = false;
	}
	if (!m_bVirtual) {
		m_nType = -1;
		m_dwStart = 0;
		m_dwDuration = 0;
	}
};

/***************************************************************************/
// CSegmentSelection::RefreshAnnotation
// Redraw annotation losing selection as we find out.
/***************************************************************************/
void CSegmentSelection::RefreshAnnotation(CSaView * pView, int nAnnotationIndex) {
	TRACE("RefreshAnnotation\n");
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
	TRACE("GetSelectedAnnotationString\n");
	if (m_bVirtual) {
		return "";
	}
	CString ret = pView->GetSelectedAnnotationString();
	if (bRemoveDelimiter) {
		if ((ret[0] == SEGMENT_DEFAULT_CHAR) && (m_nType != GLOSS)) {
			return ret.Mid(1);
		}
		if (((ret[0] == TEXT_DELIMITER) || (ret[0] == WORD_DELIMITER)) && (m_nType == GLOSS)) {
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
BOOL CSegmentSelection::SetSelectedAnnotationString(CSaView * pView, CSaString & szString, bool bIncludesDelimiter, bool bCheck) {
	TRACE("SetSelectedAnnotationString\n");
	if (m_nType == -1) {
		return FALSE;
	}

	int nType = m_nType;
	CSegment * pSegment = pView->GetAnnotation(nType);
	CSaDoc * pModel = pView->GetDocument();

	// include delimiter;
	if ((!bIncludesDelimiter) && (nType == GLOSS)) {
		CString szDelimiter = GetSelectedAnnotationString(pView, FALSE).Left(1);
		szString = szDelimiter + szString;
	}

	if (szString.GetLength() == 0) {
		if (nType == GLOSS) {
			szString = WORD_DELIMITER;
		} else if (nType == PHONETIC) {
			szString = SEGMENT_DEFAULT_CHAR;
		}
	}

	// Save cursor positions
	DWORD dwStart = pView->GetStartCursorPosition();
	DWORD dwStop = pView->GetStopCursorPosition();

	// Set cursors to segment boundaries to avoid changes
	pView->SetStartCursorPosition(m_dwStart);
	pView->SetStopCursorPosition(m_dwStart+m_dwDuration);

	if (m_bVirtual) {
		if ((nType == GLOSS) || (nType == PHONETIC)) {
			return FALSE;
		}

		// 'Add' refreshes graphs, set modified flag, & check point
		if (pSegment->GetMasterIndex() != -1) {
			((CDependentSegment *)pSegment)->Add(pModel, pView, m_dwStart, szString, false, bCheck);
			pView->RedrawGraphs(FALSE);
		} else {
			int nInsertAt = pSegment->CheckPosition(pModel, m_dwStart, m_dwStart+m_dwDuration, CSegment::MODE_ADD);
			if (nInsertAt != -1) {
				if (bCheck) {
					pModel->CheckPoint();
				}
				// document has been modified
				pModel->SetModifiedFlag(TRUE);
				// transcription has been modified
				pModel->SetTransModifiedFlag(TRUE);
				pSegment->Insert(nInsertAt, szString, true, m_dwStart, m_dwDuration);
				pView->ChangeAnnotationSelection(pSegment, nInsertAt);
			}
		}
	} else if (szString.GetLength() == 0) {
		// We need to remove the dependent segment
		DWORD dwPosition = m_dwStart;
		// Remove refreshes graphs, set modified flag, & check point
		int index = pSegment->GetSelection();
		if (index != -1) {
			pSegment->Remove(pView->GetDocument(), index, bCheck);
		}
		SelectFromPosition(pView, nType, dwPosition, true);
	} else {
		if (bCheck) {
			pModel->CheckPoint();
		}
		// ReplaceSelectedSegment refreshes graphs, sets modified flag
		TRACE("replace segment\n");
		pSegment->ReplaceSelectedSegment(pModel, szString, true);
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
	return m_dwStart+m_dwDuration;
}

/**
* returns the index of the selected segment, or -1
*/
int CSegmentSelection::GetSelectionIndex() {
	return m_nType;
}

bool CSegmentSelection::IsSelectionVirtual() {
	return m_bVirtual;
}

void CSegmentSelection::DeselectAnnotations(CSaView * pView) {
	TRACE("DeselectionAnnotations\n");
	CSegment * pSegment = pView->FindSelectedAnnotation();
	if (pSegment != NULL) {
		TRACE("deselecting annotation %lp\n", pSegment);
		pView->ChangeAnnotationSelection(pSegment, -1);
	}
	// clear virtual selection
	Update(pView, TRUE);
}
