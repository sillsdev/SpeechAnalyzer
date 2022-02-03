/////////////////////////////////////////////////////////////////////////////
// Segment.cpp:
// Implementation of the CSegment class.
//
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
//
// Revision History
// 1.06.1.2
//         SDM Added function
//               void CGlossSegment::AdjustCursorsToPhonetic(CDocument* pSaDoc)
//         SDM Added virtual function int CTextSegment::CheckCursors(CDocument* pSaDoc) const
//         SDM Modified CTextSegmnet::Add() to use AdjustCursorsToPhonetic()
//         SDM Added virtual functions LimitPosition(CDocument* pSaDoc, DWORD& dwStart,DWORD& dwStop) const
//         SDM Added virtual functions CheckPosition(CDocument* pSaDoc, DWORD& dwStart,DWORD& dwStop) const
// 1.06.3a
//         SDM Modified long CGlossSegment::Process(...) to match inserted phonetic durations to gloss
// 1.06.3b
//         SDM Added helper global function BOOL gInputFilter(CSaString&)
// 1.06.4
//         SDM Modified CSegment::GetWindowFont to return CFontTable
//         SDM Modified CPhoneticSegment::Process to move User Interface to message handler
// 1.06.5
//         SDM Added mode parameter to function CheckPosition()
//         SDM Removed old add & edit procedures
//         SDM Added new add function
// 1.06.6U2
//         SDM Added function CPhoneticSegment::AdjustPositionAll()
// 1.06.6U5
//         SDM Changed gIPAInputFilterProc
// 1.5Test8
//         SDM added changes by CLW in 1.07a
// 1.5Test8.1
//         SDM fixed bug in checkPosition which allowed Gloss to be added without Phonetic
//         SDM fixed bug in delete contents which allowed an empty segment to remain selected
//         SDM removed unused constructors/destructors
//         SDM changed alignToSegment to take a segment index to align to
//         SDM fixed bug which would fail to allow drag UpdateBoundaries of second to last gloss
//         SDM changed CTextSegment::RemoveNoRefresh to return offset like Phonetic
//         SDM added CDependentTextSegment
//         SDM changed CGlossSegment to handle embedded POS and dependentText segments
//         SDM added CReferenceSegment
//         SDM added Mode _NO_OVERLAP to CSegment::LimitPosition
//         SDM added bOverlap to CheckPosition CheckCursors
// 1.5Test8.2
//         SDM changed CTextSegment::AdjustPositionToPhonetic to return phonetic index
//         SDM changed CGlossSegement::ReplaceSelectedSegment to preserve POS
//         SDM CGlossSegment::Process remove delete question to OnAdvanceSegment
//         SDM CGlossSegment::Process add initial gloss segment a position 0
// 1.5Test8.3
//         SDM Changed ReplaceSelectedSegment to keep existing position for dependents except CPhoneticSegment
// 1.5Test10.0
//         SDM moved parse & segment PARMs to CMainFrame
// 1.5Test10.2
//         SDM fixed CTextSegment::ReplaceSelectSegment to force redraw of annotation
// 1.5Test10.7
//         SDM ifdef insert of inital word in CGlossSegment::Process
// 1.5Test11.0
//         SDM replaced GetOffset() + GetDuration() with CSegment::GetStop()
//         SDM replaced m_Offset.GetSize() with GetSize()
//         SDM changed m_Durations to GetDurations() to force updateDurations()
//         SDM added CTextSegment::GetDurations() to calculate gloss duration
//         SDM added CDependentTextSegment::GetDurations()
//         SDM removed changes to m_Durations for CTextSegment and children
//         SDM changed CGlossSegment::Process to
//               autoSnapUpdate before adding gloss
//               mark gloss at beginning of recording if less than break width of silence
//               mark silence in phonetic segment
//               snap inserted segments
//               measure break width per description in Advanced..Parameters dialog
//               mark gloss start on nearest Phonetic segment
//               limit minimum phonetic segment to MIN_ADD_SEGMENT_TIME
// 1.5Test11.3
//         SDM replaced changes to Gloss and children (see 11.0)
//         SDM changed CSegment::AdjustPositionAll to CSegment::Adjust
// 1.5Test11.1A
//         RLJ Moved parsing and segmenting parameters reference back to Doc
// 1.5Test11.4
//         SDM added support for editing PHONEMIC/TONE/ORTHO to span multiple segments
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Segment.h"
#include "sa_asert.h"
#include "ArchiveTransfer.h"
#include "sa_doc.h"
#include "sa_view.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CSegment
// class to do all the handling with annotation segments. The class owns the
// annotation data structures from the wave file.

/////////////////////////////////////////////////////////////////////////////
// CSegment construction/destruction/creation

/***************************************************************************/
// CSegment::CSegment Constructor
/***************************************************************************/
CSegment::CSegment(EAnnotation type, int masterType) {
	m_nAnnotationType = type;
	m_nMasterType = masterType;
	// no segment selected
	m_nSelection = -1;
}

/***************************************************************************/
// CSegment::~CSegment Destructor
/***************************************************************************/
CSegment::~CSegment() {
}

/////////////////////////////////////////////////////////////////////////////
// CSegment helper functions

/***************************************************************************/
// CSegment::DeleteContents Delete all contents of the segment arrays
/***************************************************************************/
void CSegment::DeleteContents() {
	m_Text.RemoveAll();
	m_Offset.RemoveAll();
	m_Duration.RemoveAll();
	// SDM 1.5Test8.1 empty segments cannot be selected
	m_nSelection = -1;
}

void CSegment::Serialize(CArchive & ar) {
	if (ar.IsStoring()) {
		ar << CSaString("integrityCheck");
		ar << CSaString(GetContent());        // annotation string
		int x = GetOffsetSize();
		SA_ASSERT(x);
		CArchiveTransfer::tInt(ar, x);
		for (int i = 0; i < m_Offset.GetSize(); i++) {
			CArchiveTransfer::tDWORD(ar, GetOffset(i));
		}
		CArchiveTransfer::tInt(ar, m_Duration.GetSize());
		for (int i = 0; i < m_Duration.GetSize(); i++) {
			CArchiveTransfer::tDWORD(ar, GetDuration(i));
		}
		CArchiveTransfer::tInt(ar, m_nSelection);
	} else {
		CSaString integrityCheck;
		ar >> integrityCheck;
		SA_ASSERT(integrityCheck == "integrityCheck");
		CString temp;
		ar >> temp;                         // annotation string
		int x = CArchiveTransfer::tInt(ar);
		SA_ASSERT(x);
		m_Offset.SetSize(x);
		for (int i = 0; i < m_Offset.GetSize(); i++) {
			m_Offset.SetAt(i, CArchiveTransfer::tDWORD(ar));
		}
		m_Duration.SetSize(CArchiveTransfer::tInt(ar));
		for (int i = 0; i < m_Duration.GetSize(); i++) {
			m_Duration.SetAt(i, CArchiveTransfer::tDWORD(ar));
		}
		m_nSelection = CArchiveTransfer::tInt(ar);
	}
	m_Text.Serialize(ar);
}

/***************************************************************************/
// CSegment::Remove Remove dependent annotation segment
/***************************************************************************/
void CSegment::Remove(CSaDoc * pModel, int sel, BOOL bCheck) {
	TRACE("Remove\n");
	// save state for undo ability
	if (bCheck) {
		pModel->CheckPoint();
	}
	RemoveAt(sel);
	// get pointer to view
	CSaView * pView = pModel->GetFirstView();
	// refresh ui
	pModel->SetModifiedFlag(TRUE);                        // document has been modified
	pModel->SetTransModifiedFlag(TRUE);                   // transcription data has been modified
	pView->ChangeAnnotationSelection(this, sel, 0, 0);	// deselect
	pView->RedrawGraphs(FALSE);							// refresh the graphs between cursors
}

/***************************************************************************/
// CSegment::Replace
/***************************************************************************/
void CSegment::Replace(CSaDoc * pModel, int index, LPCTSTR find, LPCTSTR replace) {
	ASSERT(index == -1);
	if (index == -1) {
		return;
	}
	if (IsEmpty()) {
		return;
	}
	if (wcslen(find) == 0) {
		return;
	}
	if (wcslen(replace) == 0) {
		return;
	}

	// return of zero means nothing was changed.
	if (m_Text[index].Replace(find, replace) == 0) {
		return;
	}

	// get pointer to view
	CSaView * pView = pModel->GetFirstView();

	pModel->SetModifiedFlag(TRUE);        // document has been modified
	pModel->SetTransModifiedFlag(TRUE);   // transcription data has been modified
	pView->ChangeAnnotationSelection(this, index);		// deselect
	pView->ChangeAnnotationSelection(this, index);		// select again
	pView->RedrawGraphs(FALSE);							// refresh the graphs between cursors
}

/***************************************************************************/
// CSegment::ReplaceSelectedSegment
/***************************************************************************/
void CSegment::ReplaceSelectedSegment(CSaDoc * pModel, LPCTSTR replace, bool /*noSnap*/) {
	if (m_nSelection == -1) {
		return;
	}

	m_Text.SetAt(m_nSelection, replace);

	pModel->SetModifiedFlag(TRUE);        // document has been modified
	pModel->SetTransModifiedFlag(TRUE);   // transcription data has been modified

	int nSaveSelection = m_nSelection;  // 1.5Test10.2

	// get pointer to view
	CSaView * pView = pModel->GetFirstView();

	// deselect // 1.5Test10.2
	pView->ChangeAnnotationSelection(this, m_nSelection);
	// select again // 1.5Test10.2
	pView->ChangeAnnotationSelection(this, nSaveSelection);
	// refresh the graphs between cursors
	pView->RedrawGraphs(FALSE);
}

/***************************************************************************
* CSegment::Insert Insert an annotation segment
* Returns FALSE if an error occured.
* nDelimiter is not used in this basic version of the function.
* nIndex index into annotation string
***************************************************************************/
BOOL CSegment::SetAt(const CSaString & text, bool delimiter, DWORD dwStart, DWORD dwDuration, bool textSegment) {
	CSaString data = text;

	if (textSegment) {
		if (text.GetLength() == 0) {
			return TRUE;
		}
		// this should only be used for 'text' segments
		// prepare delimiter
		CSaString szDelimiter = WORD_DELIMITER;
		if (delimiter) {
			szDelimiter.SetAt(0, TEXT_DELIMITER);
		}
		if (text.GetLength() != 0) {
			data = szDelimiter + data;
		} else {
			data = szDelimiter;
		}
	}

	return SetAt(data, dwStart, dwDuration);
}

/***************************************************************************
* CSegment::Insert Insert an annotation segment
* Returns FALSE if an error occured.
* nDelimiter is not used in this basic version of the function.
* nIndex index into annotation string
***************************************************************************/
BOOL CSegment::SetAt(const CSaString & text, DWORD dwStart, DWORD dwDuration) {
	if (m_Offset.GetCount() == 0) {
		ASSERT(m_Duration.GetCount() == 0);
		ASSERT(GetContentLength() == 0);
		InsertAt(0, (LPCTSTR)text, dwStart, dwDuration);
	} else {
		int nIndex = FindOffset(dwStart);
		if (nIndex >= 0) {
			// index was found
			// remove the existing string
			RemoveAt(nIndex);
			InsertAt(nIndex, (LPCTSTR)text, dwStart, dwDuration);
		} else {
			// index was at end of string
			nIndex = m_Offset.GetCount();
			InsertAt(nIndex, CString(text), dwStart, dwDuration);
		}
	}

	ASSERT(m_Text.GetSize() == m_Offset.GetSize());
	ASSERT(m_Text.GetSize() == m_Duration.GetSize());
	return TRUE;
}

/***************************************************************************
* CSegment::Insert Insert an annotation segment
* Returns FALSE if an error occurred.
* nDelimiter is not used in this basic version of the function.
* nIndex index into annotation string
***************************************************************************/
BOOL CSegment::Insert(int nIndex, LPCTSTR pszString, bool /*delimiter*/, DWORD dwStart, DWORD dwDuration) {
	InsertAt(nIndex, pszString, dwStart, dwDuration);
	return TRUE;
}

/***************************************************************************
* CSegment::Append append an annotation segment
* Returns FALSE if an error occurred.
* nDelimiter is not used in this basic version of the function.
* nIndex index into annotation string
***************************************************************************/
BOOL CSegment::Append(LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration) {
	return Insert(GetOffsetSize(), pszString, delimiter, dwStart, dwDuration);
}

/***************************************************************************/
// CSegment::SetSelection Sets the annotation selection
// The function only sets the index of the selected (highlighted) segment.
// If the segment was already selected, it deselects.
/***************************************************************************/
void CSegment::SetSelection(int nIndex) {
	if (m_nSelection == nIndex) {
		// deselect
		m_nSelection = -1;
	} else {
		// store the index
		m_nSelection = nIndex;
	}
}

/***************************************************************************/
// CSegment::GetPrevious Get previous annotation segment
// Returns the index of the previous annotation segment (first character in
// the string) before the one with the given index. If the return is -1,
// there is no previous segment. If the parameter nIndex delivers -1, the
// function takes the actual selected segment to look for.
/***************************************************************************/
int CSegment::GetPrevious(int nIndex) const {
	// find out, which character is the reference
	int nReference = 0;
	if (nIndex >= 0) {
		// user defined reference
		nReference = nIndex - 1;
		if (nReference >= m_Offset.GetSize()) {
			nReference = m_Offset.GetSize() - 1;
		}
	} else {
		// take current selection
		if (m_nSelection != -1) {
			nReference = m_nSelection - 1;
		} else {
			// default for nothing has been selected yet
			nReference = m_Offset.GetSize() - 1;
		}
	}
	if (nReference < 0) {
		// there is no previous character
		return -1;
	}
	DWORD dwOffset = GetOffset(nReference);
	nReference--;
	while ((nReference >= 0) && (GetOffset(nReference) == dwOffset)) {
		nReference--;
	}
	return ++nReference;
}

/***************************************************************************/
// CSegment::GetNext Get next annotation segment
// Returns the index of the next annotation segment
// (first character in the string) next to the one with the given index.
// If the return is -1, there is no next segment.
// If the parameter nIndex delivers -1, the function takes the actual
// selected segment to look for.
/***************************************************************************/
int CSegment::GetNext(int nIndex) const {
	// find out, which character is the reference
	int nReference = 0;
	if (nIndex >= 0) {
		// user defined reference
		nReference = nIndex;
	} else {
		// take current selection
		if (m_Offset.GetCount() == 0) {
			return -1;
		}
		if (m_nSelection != -1) {
			nReference = m_nSelection;
		} else {
			// default for nothing has been selected yet
			nReference = 0;
		}
	}
	DWORD dwOffset = GetOffset(nReference);
	nReference++;
	while ((nReference < m_Offset.GetSize()) && (GetOffset(nReference) == dwOffset)) {
		nReference++;
	}
	if (nReference >= m_Offset.GetSize()) {
		// no next character
		return -1;
	}
	return nReference;
}

/***************************************************************************/
// CSegment::FindOffset Find the annotation segment offset match
// Returns the index of the annotation segment (first character in the
// string) which offset matches the parameter.
// If there is no match, the function returns -1.
/***************************************************************************/
int CSegment::FindOffset(DWORD dwOffset) const {
	if (IsEmpty()) {
		return -1;
	}

	DWORD * pFirst = (DWORD *)m_Offset.GetData();
	// pLast should be set to the last position + 1
	DWORD * pLast = pFirst + m_Offset.GetSize();
	DWORD * pLowerBound = std::lower_bound(pFirst, pLast, dwOffset);
	if ((pLowerBound != pLast) && (*pLowerBound == dwOffset)) {
		return pLowerBound - pFirst;
	}

	return -1;
}

// SDM 1.5 Test 11.0
/***************************************************************************/
// CSegment::FindStop Find the annotation segment stop match
// Returns the index of the annotation segment (first character in the
// string) whichs offset matches the parameter.
// If there is no match, the function returns -1.
/***************************************************************************/
int CSegment::FindStop(DWORD dwStop) const {
	if (IsEmpty()) {
		return -1;
	}

	// Use std::lower_bound which has O(log(N)) for performance reasons
	// Needed for large annotated files
	DWORD * pFirst = (DWORD *)m_Offset.GetData();
	DWORD * pLast = pFirst + (m_Offset.GetSize() - 1);
	DWORD * pLowerBound = std::lower_bound(pFirst, pLast, dwStop);

	int nLoop = 0;
	if (pLowerBound != pLast) {
		nLoop = pLowerBound - pFirst;
	} else {
		nLoop = m_Offset.GetSize() - 1;
	}

	for (; nLoop >= 0; --nLoop) {
		if (GetStop(nLoop) < dwStop) {
			// no match
			return -1;
		}
		if (GetStop(nLoop) == dwStop) {
			while (nLoop && GetStop(nLoop - 1) == dwStop) {
				--nLoop;
			}
			// match
			return nLoop;
		}
	}
	// no match
	return -1;
}

/***************************************************************************
* CSegment::FindFromPosition
* Find annotation segment from position
*
* Returns the index of the annotation segment (first character in the string)
* which position-duration area contains the given position.
*
* If the position is not inside an area, it returns the next higher segment index.
*
* If there are no segments, it returns -1.
*
* if bWithin=false, it returns last index if it is above last index.
*
* if bWithin=true, index is returned if within segment.  -1 returned otherwise
*
***************************************************************************/
int CSegment::FindFromPosition(DWORD dwPosition, BOOL bWithin) const {
	if (IsEmpty()) {
		// there are no segments
		return -1;
	}

	// check if position above highest segment
	DWORD dwTempPos = dwPosition;

	// get end position of last segment
	DWORD dwStop = GetStop(m_Offset.GetSize() - 1);
	if (dwStop <= dwTempPos) {
		// if we are at or beyond the last end position then
		// set position with last end position
		dwTempPos = dwStop - 1;
	}

	// Use std::lower_bound which has O(log(N)) for performance reasons
	// Needed for large annotated files
	DWORD * pFirst = (DWORD *)m_Offset.GetData();
	DWORD * pLast = pFirst + (m_Offset.GetSize() - 1);
	// Returns an iterator pointing to the first element in the range [first,last) which does not compare less than val.
	// Returns an iterator to the lower bound of val in the range.
	// If all the element in the range compare less than val, the function returns last.
	DWORD * pLowerBound = std::lower_bound(pFirst, pLast, dwPosition);

	int nLoop = 0;
	if (pLowerBound != pLast) {
		// we are not at end, use delta
		nLoop = pLowerBound - pFirst;
	} else {
		// use max index
		nLoop = m_Offset.GetSize() - 1;
	}

	// walk backwards and find the first stop position we are greater than
	// the stop position is offset+duration, which may not span the entire
	// area between offsets(?)
	// dwTempPos is placed at stop - 1
	for (; nLoop >= 0; nLoop = GetPrevious(nLoop)) {
		if (GetStop(nLoop) <= dwTempPos) {
			break; // this is it
		}
	}

	// advance one position.  if index is negative - set to first position
	if (nLoop >= 0) {
		// selected character is one position higher
		nLoop = GetNext(nLoop);
	} else {
		nLoop = 0;
	}

	if (bWithin) {
		//TRACE("offset[%d]=%d\n",i,m_Offset[i]);
		DWORD tstart = GetOffset(nLoop);
		DWORD tstop = GetStop(nLoop);
		if ((dwPosition < tstart) || (dwPosition > tstop)) {
			return -1;
		}
	}
	return nLoop;
}

/***************************************************************************/
// CSegment::GetSegmentLength Find the segment length
// Returns the length (in characters) of the given segment.
//
/***************************************************************************/
int CSegment::GetSegmentLength(int index) const {
	ASSERT(index < m_Offset.GetCount());
	return m_Text[index].GetLength();
}

/***************************************************************************/
// CSegment::GetSegmentString Return the segment character string
/***************************************************************************/
CSaString CSegment::GetSegmentString(int nIndex)   const {
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < m_Text.GetCount());
	return m_Text[nIndex];
}

/**
* Adjust the segment, but not it's dependents
*/
void CSegment::Adjust(int nIndex, DWORD newOffset, DWORD newDuration) {
	m_Offset[nIndex] = newOffset;
	m_Duration[nIndex] = newDuration;
}

/***************************************************************************/
// CSegment::Adjust Adjusts positions of an annotation segment
/***************************************************************************/
void CSegment::Adjust(ISaDoc * pModel, int index, DWORD newOffset, DWORD newDuration, bool segmental) {
	// for use later
	DWORD dwOldOffset = GetOffset(index);
	DWORD dwOldStop = GetStop(index);

	Adjust(index, newOffset, newDuration);

	// adjust all dependent segments
	for (int nWnd = 0; nWnd < ANNOT_WND_NUMBER; nWnd++) {
		CSegment * pSegment = pModel->GetSegment(nWnd);
		if ((pSegment != NULL) && (pSegment->IsDependent(*this))) {
			// for segmental, only adjust segments that match the existing segment
			if (segmental) {
				int nIndex = pSegment->FindOffset(dwOldOffset);
				if (nIndex != -1) {
					DWORD curOffset = pSegment->GetOffset(nIndex);
					if (curOffset == dwOldOffset) {
						pSegment->Adjust(pModel, nIndex, newOffset, pSegment->GetStop(nIndex) - newOffset, false);
					}
				}
				nIndex = pSegment->FindStop(dwOldStop);
				if (nIndex != -1) {
					DWORD curOffset = pSegment->GetOffset(nIndex);
					if (curOffset == dwOldOffset) {
						pSegment->Adjust(pModel, nIndex, curOffset, newOffset + newDuration - curOffset, false);
					}
				}
			} else {
				// nonsegmental - offset and duration should match
				int nIndex = pSegment->FindOffset(dwOldOffset);
				if (nIndex != -1) {
					pSegment->Adjust(pModel, nIndex, newOffset, pSegment->GetStop(nIndex) - newOffset, false);
				}

				nIndex = pSegment->FindStop(dwOldStop);
				if (nIndex != -1) {
					DWORD curOffset = pSegment->GetOffset(nIndex);
					pSegment->Adjust(pModel, nIndex, curOffset, newOffset + newDuration - curOffset, false);
				}
			}
		}
	}
}

// SDM Split function 1.06.1.2
/***************************************************************************/
// CSegment::CheckCursors Check cursor positions for annotation window
// Checks the positions of the cursors for validation. If they are ok for
// a new annotation segment it returns the index, where to put it in the
// annotation array (0 based), otherwise -1.
/***************************************************************************/
int CSegment::CheckCursors(CSaDoc * pModel, BOOL bOverlap) const {
	// get pointer to view
	POSITION pos = pModel->GetFirstViewPosition();
	CSaView * pView = (CSaView *)pModel->GetNextView(pos);
	return CheckPosition(pModel, pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), MODE_AUTOMATIC, bOverlap);
}

BOOL CSegment::NeedToScroll(CSaView & saView, int index) const {
	ASSERT(index >= 0 && index < m_Offset.GetSize());

	DWORD viewLeftEdge = (int)saView.GetDataPosition(0);
	DWORD viewRightEdge = viewLeftEdge + saView.GetDataFrame();
	DWORD anotLeftEdge = GetOffset(index);
	DWORD anotRightEdge = anotLeftEdge + GetDuration(index);

	return ((anotLeftEdge < viewLeftEdge) || (anotRightEdge > viewRightEdge));
}

int CSegment::FirstVisibleIndex(CSaDoc & SaDoc) const {
	POSITION pos = SaDoc.GetFirstViewPosition();
	CSaView  * pView = (CSaView *)SaDoc.GetNextView(pos);
	for (int index = 0; index < m_Offset.GetSize(); index++) {
		if (!NeedToScroll(*pView, index)) {
			return index;
		}
	}
	return 0;
}

int CSegment::LastVisibleIndex(CSaDoc & SaDoc) const {
	POSITION pos = SaDoc.GetFirstViewPosition();
	CSaView  * pView = (CSaView *)SaDoc.GetNextView(pos);

	for (int index = m_Offset.GetSize() - 1; index >= 0; index--) {
		if (!NeedToScroll(*pView, index)) {
			return index;
		}
	}

	return 0;
}

/***************************************************************************/
// CSegment::FindPrev find next segment matching strToFind.
//***************************************************************************/
int CSegment::FindPrev(int fromIndex, LPCTSTR strToFind) {
	ASSERT(fromIndex >= -1);
	ASSERT(!IsEmpty());
	int index = -1;
	if (fromIndex > 0 || fromIndex == -1) {
		index = (fromIndex == -1) ? m_Text.GetUpperBound() : fromIndex - 1;
		for (; index >= 0; index--) {
			if (m_Text.GetAt(index).Find(strToFind) != -1) {
				return index;
			}
		}
	}
	return -1;
}

/***************************************************************************/
// CSegment::FindNext find next segment matching strToFind.
//***************************************************************************/
int CSegment::FindNext(int fromIndex, LPCTSTR strToFind) {
	ASSERT(fromIndex >= -1);
	ASSERT(!IsEmpty());

	for (int i = fromIndex + 1; i < m_Text.GetCount(); i++) {
		if (m_Text.GetAt(i).Find(strToFind) != -1) {
			return i;
		}
	}
	return -1;
}

/***************************************************************************/
// CSegment::Match find next segment matching strToFind and hilite it.
//***************************************************************************/
bool CSegment::Match(int index, LPCTSTR strToFind) {
	if (index < 0) {
		return false;
	}
	if (IsEmpty()) {
		return false;
	}
	if (wcslen(strToFind) == 0) {
		return false;
	}

	// invalid index
	if (index >= m_Text.GetCount()) {
		return false;
	}

	return (m_Text[index].Find(strToFind) != -1);
}

/***************************************************************************/
// CSegment::AdjustCursorsToSnap Adjust cursors to apropriate snap position
// Adjust the current cursor positions to the positions, that they will have,
// when a segment will be selected.
//***************************************************************************/
void CSegment::AdjustCursorsToSnap(CDocument * pSaDoc) {
	// get requested cursor alignment
	// snap the cursors first to appropriate position
	CSaDoc * pModel = (CSaDoc *)pSaDoc; // cast pointer
	POSITION pos = pModel->GetFirstViewPosition();
	CSaView * pView = (CSaView *)pModel->GetNextView(pos);
	DWORD dwNewOffset = pView->GetStartCursorPosition();
	DWORD dwNewDuration = pView->GetStopCursorPosition();
	pView->SetStartCursorPosition(dwNewOffset, SNAP_RIGHT);
	pView->SetStopCursorPosition(dwNewDuration, SNAP_LEFT);
}

int CSegment::GetOffsetSize() const {
	return m_Offset.GetSize();
}

int CSegment::GetDurationSize() const {
	return m_Duration.GetSize();
}

DWORD CSegment::GetOffset(const int nIndex) const {
	return ((nIndex < m_Offset.GetSize()) && (nIndex >= 0)) ? m_Offset[nIndex] : 0L;
}

DWORD CSegment::GetDuration(const int nIndex) const {
	return (nIndex < m_Duration.GetSize() && (nIndex >= 0)) ? m_Duration[nIndex] : 0L;
}

DWORD CSegment::GetStop(const int nIndex) const {
	return (GetOffset(nIndex) + GetDuration(nIndex));
}

/** returns true if there are no offsets */
BOOL CSegment::IsEmpty() const {
	return (m_Offset.GetSize() == 0);
}

DWORD CSegment::GetDurationAt(int index) const {
	return m_Duration[index];
}

/**
* remove text, offset and duration
*/
void CSegment::RemoveAt(int index) {
	TRACE("RemoveAt\n");
	ASSERT(index >= 0);
	ASSERT(index < m_Offset.GetCount());
	if (index < 0) {
		return;
	}
	if (index >= m_Offset.GetCount()) {
		return;
	}
	m_Text.RemoveAt(index);
	m_Offset.RemoveAt(index);
	m_Duration.RemoveAt(index);
}

/**
* remove text, offset and duration
*/
void CSegment::ClearText(int index) {
	ASSERT(index >= 0);
	ASSERT(index < m_Text.GetCount());
	if (index < 0) {
		return;
	}
	if (index >= m_Text.GetCount()) {
		return;
	}
	m_Text[index] = "";
}
/*
* Returns the text that is within the start and stop markers
*/
CSaString CSegment::GetContainedText(DWORD dwStart, DWORD dwStop) {
	CSaString szText;
	for (int i = 0; i < m_Offset.GetSize(); i++) {
		DWORD begin = m_Offset[i];
		// offset can't be dwStop, because then the the segments length would be zero.
		if ((begin >= dwStart) && (begin < dwStop)) {
			szText.Append(m_Text[i]);
		}
	}
	return szText;
}

/*
* Returns the text that contains the start and stop markers
*/
CSaString CSegment::GetOverlappingText(DWORD dwStart, DWORD dwStop) {
	CSaString szText;
	for (int i = 0; i < m_Offset.GetSize(); i++) {
		DWORD begin = m_Offset[i];
		DWORD end = begin + m_Duration[i];
		if ((end > dwStart) && (begin < dwStop)) {
			szText.Append(m_Text[i]);
		}
	}
	return szText;
}

int CSegment::GetMasterIndex(void) const {
	return m_nMasterType;
}

bool CSegment::Is(EAnnotation type) const {
	return (type == m_nAnnotationType);
}

EAnnotation CSegment::GetAnnotationIndex(void) const {
	return m_nAnnotationType;
}

//overridden by derived classes
CString CSegment::GetText(int index) const {
	ASSERT(index >= 0);
	return m_Text[index];
}

/**
* Returns the selection index, or -1 if no segment is selected
*/
int CSegment::GetSelection() const {
	return m_nSelection;
}

long CSegment::Process(void * /*pCaller*/, ISaDoc * /*pModel*/, int /*nProgress*/, int /*nLevel*/) {
	return PROCESS_ERROR;
}

/**
* Find the index for the specified offset
*/
int CSegment::FindIndex(DWORD offset) {
	for (INT_PTR i = 0; i < m_Offset.GetSize(); i++) {
		if (offset == m_Offset[i]) {
			return i;
		}
	}
	return -1;
}

/**
* Find the segment the offset is located within
* Returns -1 if not found
* sounds new-agey..
*/
int CSegment::FindWithin(DWORD offset) {
	for (INT_PTR i = 0; i < m_Offset.GetSize(); i++) {
		DWORD start = m_Offset[i];
		DWORD stop = start + m_Duration[i];
		if ((start <= offset) && (offset < stop)) {
			return i;
		}
	}
	return -1;
}

/**
* Find the index for the specified offset and duration
*/
int CSegment::FindIndex(DWORD offset, DWORD duration) {
	for (INT_PTR i = 0; i < m_Offset.GetSize(); i++) {
		if ((offset == m_Offset[i]) && (duration == m_Duration[i])) {
			return i;
		}
	}
	return -1;
}

void CSegment::AdjustDuration(DWORD offset, DWORD duration) {
	// adjust this segment
	for (INT_PTR i = 0; i < m_Offset.GetSize(); i++) {
		if (m_Offset[i] == offset) {
			// set new duration
			m_Duration[i] = duration;
		}
	}
}

/**
* Split this segment
* @param start the offset of the phonetic segment
*/
bool CSegment::Split(DWORD thisOffset, DWORD newStopStart) {
	if (m_Offset.GetSize() == 0) {
		return false;
	}
	int index = FindIndex(thisOffset);
	if (index == -1) {
		return false;
	}

	// store old stop location
	DWORD stop = GetStop(index);
	// shorten segment
	AdjustDuration(thisOffset, newStopStart - thisOffset);
	Insert(index + 1, GetDefaultText(), FALSE, newStopStart, stop - newStopStart);
	return true;
}

/**
* Split this segment
* @param start the offset of the phonetic segment
*/
bool CSegment::Merge(DWORD thisOffset, DWORD prevOffset, DWORD thisStop) {
	if (m_Offset.GetSize() == 0) {
		return false;
	}
	int index = FindIndex(thisOffset);
	if (index == -1) {
		return false;
	}
	int prev = FindIndex(prevOffset);
	if (prev == -1) {
		return false;
	}

	// shorten text
	RemoveAt(index);
	// increase segment size
	AdjustDuration(prevOffset, thisStop - prevOffset);
	return true;
}

/**
* SAB use
* return true if modified
*/
bool CSegment::MoveDataLeftSAB(DWORD offset, CString text) {
	ASSERT(m_Text.GetCount() == m_Offset.GetCount());
	ASSERT(m_Text.GetCount() == m_Duration.GetCount());

	int index = FindOffset(offset);
	if (index == -1) return false;

	text = text.Trim();
	if (text.GetLength() == 0) {
		text = GetDefaultText();
	}

	// shift the text left
	// for sab, for the last segment, retrieve data from the file.
	m_Text.RemoveAt(index);

	m_Text.Add(text);
	return true;
}

/*
* non-sab use
* returns true if modified
*/
bool CSegment::MoveDataLeft(DWORD offset) {
	ASSERT(m_Text.GetCount() == m_Offset.GetCount());
	ASSERT(m_Text.GetCount() == m_Duration.GetCount());

	int index = FindOffset(offset);
	if (index == -1) return false;

	// shift the text left
	m_Text.RemoveAt(index);
	// remove the last segment
	size_t end = m_Offset.GetCount() - 1;
	m_Offset.RemoveAt(end);
	m_Duration.RemoveAt(end);
	return true;
}

bool CSegment::MoveDataRight(DWORD offset, bool sab) {
	ASSERT(m_Text.GetCount() == m_Offset.GetCount());
	ASSERT(m_Text.GetCount() == m_Duration.GetCount());

	int index = FindOffset(offset);
	if (index == -1) return false;

	CString defaultChar = GetDefaultText();
	if (sab) {
		// for sab - lose the data at the end

		m_Text.InsertAt(index, defaultChar);
		size_t end = m_Text.GetSize() - 1;
		m_Text.RemoveAt(end);
	} else {
		// for non-sab add overflow to buffer at end
		// add text to buffer at end

		m_Text.InsertAt(index, defaultChar);
		size_t end = m_Offset.GetSize() - 1;
		DWORD lastOffset = GetOffset(end);
		DWORD lastDuration = GetDuration(end);
		m_Offset.Add(lastOffset + lastDuration);
		m_Duration.Add(20);
	}
	return true;
}

CString CSegment::GetContent() const {
	ASSERT(m_Offset.GetCount() == m_Text.GetCount());
	CString text;
	for (int i = 0; i < m_Text.GetCount(); i++) {
		text.Append(m_Text[i]);
	}
	return text;
}

size_t CSegment::GetContentLength() const {
	if (m_Text.GetCount() == 0) {
		return 0;
	}
	CString text = GetContent();
	return text.GetLength();
}

/***************************************************************************/
// CSegment::Insert Insert/append a text segment
// Returns FALSE if an error occurred.
// If the pointer to the string is NULL there will be no string added.
/***************************************************************************/
BOOL CSegment::SetText(int nIndex, LPCTSTR pszString) {
	if ((pszString == NULL) || (wcslen(pszString) == 0)) {
		m_Text.SetAt(nIndex, "");
	} else {
		m_Text.SetAt(nIndex, pszString);
	}
	return TRUE;
}

/**
* Cefault filter does nothing
* Only phonemic and phonetic use the filter
*/
bool CSegment::Filter() {
	return false;
}

bool CSegment::Filter(CString & /*text*/) {
	return false;
}

/***************************************************************************/
// CTextSegment::CountWords
//***************************************************************************/i
int CSegment::CountWords() {
	if (m_Text.IsEmpty()) {
		return 0;
	}
	int nWords = 0;
	for (int i = 0; i < m_Text.GetCount(); i++) {
		if (m_Text[i][0] == WORD_DELIMITER) {
			nWords++;
		}
	}
	return nWords;
}

/**
* Returns the number segments that this transcription has that
* are contained within the selected segment.
*/
int CSegment::GetReferenceCount(CSegment * pOther, int sel) {
	if (sel == -1) {
		return 0;
	}
	DWORD thatOffset = pOther->GetOffset(sel);
	DWORD thatStop = pOther->GetStop(sel);
	int count = 0;
	for (int i = 0; i < m_Offset.GetCount(); i++) {
		DWORD thisOffset = m_Offset[i];
		DWORD thisStop = m_Offset[i] + m_Duration[i];
		if ((thisOffset >= thatOffset) && (thisStop <= thatStop)) {
			count++;
		}
	}
	return count;
}

/***************************************************************************
* CSaDoc::ShrinkSegments Adjust segments to new file size
* The function adjusts the segments to the new file size.
* The file size changed at the position dwSectionStart by dwSectionLength
* bytes and it shrunk.
* The segments in the deleted section will be deleted.
* The ones that overlap into the section will be adjusted if valid.
* NOTE: This operation to NOT call dependents.
***************************************************************************/
void CSegment::ShrinkSegment(CSaDoc & document, DWORD dwSectionStart, DWORD dwSectionLength) {
	TRACE("ShrinkSegment start=%d length=%d end=%d\n", dwSectionStart, dwSectionLength, (dwSectionStart + dwSectionLength));

	// section in file deleted, find segments to delete
	int nIndex = FindFromPosition(dwSectionStart);
	while (nIndex != -1) {
		DWORD dwOldOffset = GetOffset(nIndex);
		DWORD dwOldDuration = GetDuration(nIndex);

		TRACE("Testing %d %d start=%d length=%d end=%d\n", m_nAnnotationType, nIndex, dwOldOffset, dwOldDuration, (dwOldOffset + dwOldDuration));
		if (dwOldOffset >= (dwSectionStart + dwSectionLength)) {
			break; // no more to delete
		}
		if (GetSelection() != nIndex) {
			SetSelection(nIndex);
		}
		//SDM 1.06.6U1 if segments overlap adjust if its valid
		BOOL bAdjusted = FALSE;
		if (dwOldOffset < dwSectionStart) {
			if ((dwOldOffset + dwOldDuration) >(dwSectionStart + dwSectionLength)) {
				//Segment overlaps entire cut region
				DWORD dwNewStop = dwOldDuration + dwOldOffset - dwSectionLength;
				if (CheckPosition(&document, dwOldOffset, dwNewStop, CSegment::MODE_EDIT) != -1) {
					bAdjusted = TRUE;
					Adjust(nIndex, dwOldOffset, dwNewStop - dwOldOffset);
				}
			} else if ((dwOldOffset + dwOldDuration) > dwSectionStart) {
				//Segment ends in cut region
				if (CheckPosition(&document, dwOldOffset, dwSectionStart, CSegment::MODE_EDIT) != -1) {
					bAdjusted = TRUE;
					Adjust(nIndex, dwOldOffset, dwSectionStart - dwOldOffset);
				}
			} else {
				// segment ends before the cut region
				bAdjusted = TRUE;
			}
		} else if ((dwOldOffset + dwOldDuration) > (dwSectionStart + dwSectionLength)) {
			//Segment starts in cut region
			DWORD dwNewStop = dwOldDuration + dwSectionStart - (dwSectionStart + dwSectionLength - dwOldOffset);
			if (CheckPosition(&document, dwSectionStart, dwNewStop, CSegment::MODE_EDIT) != -1) {
				bAdjusted = TRUE;
				Adjust(nIndex, dwSectionStart, dwNewStop - dwSectionStart);
			}
		}
		if (!bAdjusted) {
			// delete segment
			TRACE("removing %d\n", nIndex);
			RemoveAt(nIndex);
			// segment deleted decrement to point to last segment
			nIndex--;
		}
		if (nIndex == -1) {
			// GetNext does not work correctly for (nIndex==-1)
			nIndex = 0;
			if (IsEmpty()) {
				// nothing more to do
				return;
			}
		} else {
			// find next index
			nIndex = GetNext(nIndex);
		}
	}
	// now change all the offsets of the following segment
	while (nIndex != -1) {
		DWORD dwOldOffset = GetOffset(nIndex);
		DWORD dwOldDuration = GetDuration(nIndex);
		Adjust(nIndex, dwOldOffset - dwSectionLength, dwOldDuration);
		// find next index
		nIndex = GetNext(nIndex);
	}
}

/***************************************************************************
* CSaDoc::GrowSegments Adjust segments to new file size
* The function adjusts the segments to the new file size.
* The file size changed at the position dwSectionStart by dwSectionLength
* bytes and it shrunk, if the flag bShrink is TRUE, otherwise it grew.
* In case of growth all the segments offsets after dwSectionStart change.
* In the other way, the segments in the deleted section will be deleted.
* The ones that overlap into the section will be adjusted if valid
***************************************************************************/
void CSegment::GrowSegment(CSaDoc & /*document*/, DWORD dwSectionStart, DWORD dwSectionLength) {
	SetSelection(-1);                // make sure nothing selected
	int nIndex = GetPrevious(-1);    // find last index (works if nothing selected)

	// now change all the offsets of the following segment
	while (nIndex != -1) {
		DWORD dwOldOffset = GetOffset(nIndex);
		DWORD dwOldDuration = GetDuration(nIndex);
		if (dwOldOffset < dwSectionStart) {
			break;
		}
		Adjust(nIndex, dwOldOffset + dwSectionLength, dwOldDuration);
		nIndex = GetPrevious(nIndex); // find next index
	}

	// section in file added, adjust gloss duration and offsets
	nIndex = FindFromPosition(dwSectionStart);
	if (nIndex != -1) {
		DWORD dwOldOffset = GetOffset(nIndex);
		DWORD dwOldDuration = GetDuration(nIndex);

		if (dwOldOffset <= dwSectionStart) {
			// check if insertion into segment happened
			DWORD dwOldCenter = dwOldOffset + dwOldDuration / 2;
			// we are to the right of center
			if (dwOldCenter <= dwSectionStart) {
				// but still within the old segment
				if (dwSectionStart < (dwOldOffset + dwOldDuration)) {
					Adjust(nIndex, dwOldOffset, dwSectionStart - dwOldOffset);
				}
			} else {
				// insert the new segment before this overlapping one
				Adjust(nIndex, dwSectionStart + dwSectionLength, dwOldOffset + dwOldDuration - dwSectionStart);
			}
		}
	}
}

bool CSegment::IsDependent(CSegment & parent) {
	if (m_nMasterType == -1) return false;
	int parentType = parent.GetAnnotationIndex();
	return (parentType == m_nMasterType);
}

int CSegment::GetLastNonEmptyValue() {
	for (int i = m_Text.GetCount() - 1; i >= 0; i--) {
		if (m_Text[i].Trim().GetLength()>0) return i;
	}
	return -1;
}

bool CSegment::ContainsText(DWORD offset, DWORD stop) {
	for (int i = 0; i < GetOffsetSize(); i++) {
		DWORD thisOffset = GetOffset(i);
		DWORD thisStop = GetStop(i);
		if ((thisOffset >= offset) && (thisStop <= stop)) {
			CString text = GetText(i).Trim();
			if (text.GetLength() > 0) {
				TRACE("text %s is blocking operation for %d\n", (LPCTSTR)text, this->m_nAnnotationType);
				return true;
			}
		}
	}
	return false;
}

int CSegment::Add(DWORD offset, DWORD duration) {
	CString text = GetDefaultText();
	return Add(text, offset, duration);
}

/**
* Insert a segment into the list
* Allows for overlapping segments
* Keeps offset values in sorted order
*/
int CSegment::Add(LPCTSTR text, DWORD offset, DWORD duration) {
	ASSERT(m_Text.GetCount() == m_Offset.GetCount());
	ASSERT(m_Text.GetCount() == m_Duration.GetCount());
	DWORD stop = offset + duration;
	for (int i = 0; i < m_Offset.GetCount(); i++) {
		DWORD curOffset = m_Offset[i];
		DWORD curStop = curOffset + m_Duration[i];
		if (offset < curOffset) {
			InsertAt(i, text, offset, duration);
			return i;
		} else if ((offset == curOffset) && (stop == curStop)) {
			TRACE("segment using existing offset=%d duration=%d\n", offset, duration);
			return i;
		} else if (stop < curOffset) {
			// we've gone far enough
			break;
		}
	}
	// empty or at end
	int index = m_Text.Add(text);
	m_Offset.Add(offset);
	m_Duration.Add(duration);
	TRACE("segment add type=%d index=%d offset=%d duration=%d\n", m_nAnnotationType, index, offset, duration);
	return index;
}

void CSegment::InsertAt(int index, DWORD offset, DWORD duration) {
	InsertAt(index, GetDefaultText(), offset, duration);
}

void CSegment::InsertAt(int index, LPCTSTR text, DWORD offset, DWORD duration) {
	ASSERT(index >= 0);
	if (index < 0) {
		TRACE("invalid index given on InsertAt\n");
		return;
	}

	if (m_nAnnotationType == ORTHO) {
		//TRACE("segment insertat type=%d index=%d offset=%d duration=%d\n",m_nAnnotationType,index,offset,duration);
	}
	m_Text.InsertAt(index, CString(text));
	m_Offset.InsertAt(index, offset);
	m_Duration.InsertAt(index, duration);
	ASSERT(m_Text.GetCount() == m_Offset.GetCount());
	ASSERT(m_Text.GetCount() == m_Duration.GetCount());
}

/**
* Split the segments
* @param pView the view to update
* @param sel the selected phonetic segment
* @param segmental true if there are multiple phonetic segments to a gloss segment
* returns true if modified
*/
bool CSegment::SplitSegment(CSaDoc & document, int sel, DWORD splitPosition) {
	if (sel == -1) {
		return false;
	}

	DWORD offset = GetOffset(sel);
	DWORD duration = GetDuration(sel);
	splitPosition = (splitPosition == -1) ? (offset + (duration / 2)) : splitPosition;
	splitPosition = document.SnapCursor(STOP_CURSOR, splitPosition, splitPosition, GetDataSize(), SNAP_RIGHT);

	bool modified = false;

	modified |= Split(offset, splitPosition);

	for (int n = 0; n < ANNOT_WND_NUMBER; n++) {
		CSegment * pSeg = document.GetSegment(n);
		// only call our dependents
		if (!pSeg->IsDependent(*this)) continue;
		int nIndex = pSeg->FindIndex(offset, duration);
		if (nIndex != -1) {
			modified |= pSeg->SplitSegment(document, nIndex, splitPosition);
		}
	}
	return modified;
}

// remove all text content
void CSegment::DeleteText() {
	for (int i = 0; i < m_Offset.GetCount(); i++) {
		m_Text[i] = L"";
	}
}