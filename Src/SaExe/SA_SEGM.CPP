/////////////////////////////////////////////////////////////////////////////
// sa_segm.cpp:
// Implementation of the CSegment
//                       CDependentSegment
//                       CTextSegment
//                       CDependentTextSegment
//                       CPhoneticSegment
//                       CToneSegment
//                       CPhonemicSegment
//                       COrthoSegment
//                       CGlossSegment
//                       CReferenceSegment    classes.
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
//         SDM added Mode LIMIT_NO_OVERLAP to CSegment::LimitPosition
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
//         SDM replaced m_pOffset->GetSize() with GetSize()
//         SDM changed m_pDurations to GetDurations() to force updateDurations()
//         SDM added CTextSegment::GetDurations() to calculate gloss duration
//         SDM added CDependentTextSegment::GetDurations()
//         SDM removed changes to m_pDurations for CTextSegment and children
//         SDM changed CGlossSegment::Process to
//               autoSnapUpdate before adding gloss
//               mark gloss at beginning of recording if less than brekwidth of silence
//               mark silence in phonetic segment
//               snap inserted segments
//               measure brekwidth per description in Advanced..Parameters dialog
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
#include "Process\sa_proc.h"
#include "sa_segm.h"
#include "Process\sa_p_cha.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_fra.h"

#include "sa_asert.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"

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
CSegment::CSegment( int index, int master)
{
	m_nAnnotationIndex = index;
	m_nMasterIndex = master;
	m_nSelection = - 1;					// no segment selected
	m_pAnnotation = new CSaString();	// create annotation string object
	m_pOffset = new CDWordArray();		// create offset array object
	m_pDuration = new CDWordArray();	// create duration array object
}

/***************************************************************************/
// CSegment::~CSegment Destructor
/***************************************************************************/
CSegment::~CSegment()
{
	if (m_pAnnotation) {
		delete m_pAnnotation;
		m_pAnnotation = NULL;
	}
	if (m_pOffset) {
		delete m_pOffset;
		m_pOffset = NULL;
	}
	if (m_pDuration) {
		delete m_pDuration;
		m_pDuration = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSegment helper functions

/***************************************************************************/
// CSegment::DeleteContents Delete all contents of the segment arrays
/***************************************************************************/
void CSegment::DeleteContents()
{
	m_pAnnotation->Empty();
	m_pOffset->RemoveAll();
	m_pDuration->RemoveAll();
	m_nSelection = -1; // SDM 1.5Test8.1 empty segments can not be selected
}

short int ArchiveTransfer::tShortInt(CArchive& ar, short int x)
{
	short int ret = 0;
	CSaString shortInt;

	if (ar.IsStoring())
	{
		swprintf_s(shortInt.GetBuffer(100),100,_T("%d"),x);
		shortInt.ReleaseBuffer();
		ar << shortInt;
	}
	else
	{
		ar >> shortInt;
		swscanf_s(shortInt.GetBuffer(100),_T("%d"),&ret);
		shortInt.ReleaseBuffer();
	}

	return ret;
}

int ArchiveTransfer::tInt(CArchive& ar, int x)
{
	int ret = 0;
	CSaString szInt;

	if (ar.IsStoring())
	{
		swprintf_s(szInt.GetBuffer(100),100,_T("%d"),x);
		szInt.ReleaseBuffer();
		ar << szInt;
	}
	else
	{
		ar >> szInt;
		swscanf_s(szInt.GetBuffer(100),_T("%d"),&ret);
		szInt.ReleaseBuffer();
	}

	return ret;
}

DWORD ArchiveTransfer::tDWORD(CArchive& ar, DWORD x)
{
	DWORD ret = 0;
	CSaString dWord;

	if (ar.IsStoring())
	{
		swprintf_s(dWord.GetBuffer(100),100,_T("%lu"),x);
		dWord.ReleaseBuffer();
		ar << dWord;
	}
	else
	{
		ar >> dWord;
		swscanf_s(dWord.GetBuffer(100),_T("%lu"),&ret);
		dWord.ReleaseBuffer();
	}

	return ret;
}

void CSegment::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << CSaString("integrityCheck");
		ar << CSaString(*m_pAnnotation);        // annotation string
		int x = GetOffsetSize();
		SA_ASSERT(x);
		ArchiveTransfer::tInt(ar, x);
		for (int i = 0; i<GetOffsetSize(); i++)
		{
			ArchiveTransfer::tDWORD(ar, GetOffset(i));
		}
		ArchiveTransfer::tInt(ar, GetDurationSize());
		for (int i = 0; i< GetDurationSize(); i++)
		{
			ArchiveTransfer::tDWORD(ar, GetDuration(i));
		}
		ArchiveTransfer::tInt(ar, m_nSelection);
	}
	else
	{
		CSaString integrityCheck;
		ar >> integrityCheck;
		SA_ASSERT(integrityCheck == "integrityCheck");
		ar >> *m_pAnnotation;        // annotation string
		int x = ArchiveTransfer::tInt(ar);
		SA_ASSERT(x);
		m_pOffset->SetSize(x);
		for (int i=0; i< GetOffsetSize(); i++)
		{
			m_pOffset->SetAt(i, ArchiveTransfer::tDWORD(ar));
		}
		m_pDuration->SetSize(ArchiveTransfer::tInt(ar));
		for (int i=0; i<m_pDuration->GetSize(); i++)
		{
			m_pDuration->SetAt(i, ArchiveTransfer::tDWORD(ar));
		}
		m_nSelection = ArchiveTransfer::tInt(ar);
	}
}


/***************************************************************************/
// CSegment::Remove Remove dependent annotation segment
/***************************************************************************/
void CSegment::Remove( CDocument* pSaDoc, BOOL bCheck)
{
	// get pointer to view
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);

	// save state for undo ability
	if (bCheck) pDoc->CheckPoint();

	RemoveNoRefresh(NULL);

	// refresh ui
	pDoc->SetModifiedFlag(TRUE); // document has been modified
	pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
	pView->ChangeAnnotationSelection(this, m_nSelection, 0, 0); // deselect
	pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}

/***************************************************************************/
// CSegment::ReplaceSelectedSegment
/***************************************************************************/
void CSegment::ReplaceSelectedSegment(CDocument* pSaDoc, const CSaString & str)
{
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);

	if (m_nSelection != -1) // SDM 1.5Test8.3
	{
		DWORD dwOffset = GetOffset(m_nSelection);
		DWORD dwDuration = GetDuration(m_nSelection);

		RemoveNoRefresh(NULL);

		// insert or append the new dependent segment
		if (!Insert(m_nSelection, &str, 0, dwOffset, dwDuration)) return; // return on error

		int nSaveSelection = m_nSelection;
		//SDM 1.06.5
		pDoc->SetModifiedFlag(TRUE); // document has been modified
		pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
		pView->ChangeAnnotationSelection(this, m_nSelection, dwOffset, dwOffset + dwDuration); // deselect
		pView->ChangeAnnotationSelection(this, nSaveSelection, dwOffset, dwOffset + dwDuration); // select again
		pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
	}
}

/***************************************************************************/
// CSegment::RemoveNoRefresh Remove dependent annotation segment
/***************************************************************************/
DWORD CSegment::RemoveNoRefresh( CDocument * )
{
	// find length of string to delete
	int nLength = GetSegmentLength(m_nSelection);

	// change the dependent arrays
	*m_pAnnotation = m_pAnnotation->Left(m_nSelection) + m_pAnnotation->Right(m_pAnnotation->GetLength() - nLength - m_nSelection);
	RemoveAt(m_nSelection,nLength);
	return 0;
}

/***************************************************************************
* CSegment::Insert Insert/append an annotation segment
* Returns FALSE if an error occured. 
* nDelimiter is not used in this basic version of the function.
* nIndex index into annotation string
***************************************************************************/
BOOL CSegment::SetAt( const CSaString * pszString, bool, DWORD dwStart, DWORD dwDuration) 
{
	try 
	{
		int nStringLength = pszString->GetLength(); // get the length of the new string
		if (m_pOffset->GetCount()==0) {
			ASSERT(m_pDuration->GetCount()==0);
			ASSERT(m_pAnnotation->GetLength()==0);
			*m_pAnnotation += *pszString;
			m_pOffset->InsertAt(0,dwStart,nStringLength);
			m_pDuration->InsertAt(0,dwDuration,nStringLength);
		} else {
			int nIndex = FindOffset( dwStart);
			if (nIndex>=0) {
				// index was found

				// remove the existing string
				// find length of string to delete
				int nSegmentLength = GetSegmentLength(nIndex);
				*m_pAnnotation = m_pAnnotation->Left(nIndex) + m_pAnnotation->Right(m_pAnnotation->GetLength() - nSegmentLength - nIndex);
				RemoveAt( nIndex, nSegmentLength);

				// now insert the new String
				if (nIndex < m_pAnnotation->GetLength()) 
				{ 
					// insert
					*m_pAnnotation = m_pAnnotation->Left(nIndex) + *pszString + m_pAnnotation->Right(m_pAnnotation->GetLength() - nIndex);
				}
				else
				{
					*m_pAnnotation += *pszString; // append
				}
				m_pOffset->InsertAt(nIndex,dwStart,nStringLength);
				m_pDuration->InsertAt(nIndex,dwDuration,nStringLength);
			} else {
				// index was at end of string
				nIndex = m_pOffset->GetCount();
				if (nIndex < m_pAnnotation->GetLength()) 
				{ 
					// insert
					*m_pAnnotation = m_pAnnotation->Left(nIndex) + *pszString + m_pAnnotation->Right(m_pAnnotation->GetLength() - nIndex);
				}
				else
				{
					*m_pAnnotation += *pszString; // append
				}
				m_pOffset->InsertAt(nIndex,dwStart,nStringLength);
				m_pDuration->InsertAt(nIndex,dwDuration,nStringLength);
			}
		}
	} 
	catch(CMemoryException e) 
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return FALSE;
	}
	int n2 = m_pAnnotation->GetLength();
	ASSERT(m_pAnnotation->GetLength()==GetOffsetSize());
	ASSERT(m_pAnnotation->GetLength()==m_pDuration->GetSize());
	return TRUE;
}

/***************************************************************************
* CSegment::Insert Insert/append an annotation segment
* Returns FALSE if an error occured. 
* nDelimiter is not used in this basic version of the function.
* nIndex index into annotation string
***************************************************************************/
BOOL CSegment::Insert( int nIndex, const CSaString * pszString, bool, DWORD dwStart, DWORD dwDuration) 
{
	int nStringLength = pszString->GetLength(); // get the length of the new string
	try 
	{
		if (nIndex < m_pAnnotation->GetLength()) 
		{ 
			// insert
			*m_pAnnotation = m_pAnnotation->Left(nIndex) + *pszString + m_pAnnotation->Right(m_pAnnotation->GetLength() - nIndex);
		}
		else
		{
			*m_pAnnotation += *pszString; // append
		}
		m_pOffset->InsertAt( nIndex, dwStart, nStringLength);
		m_pDuration->InsertAt( nIndex, dwDuration, nStringLength);
	} 
	catch(CMemoryException e) 
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return FALSE;
	}
	return TRUE;
}

/***************************************************************************/
// CSegment::SetSelection Sets the annotation selection
// The function only sets the index of the selected (highlighted) segment.
// If the segment was already selected, it deselects.
/***************************************************************************/
void CSegment::SetSelection(int nIndex)
{
	if (m_nSelection == nIndex) 
	{
		m_nSelection = -1; // deselect
	} 
	else 
	{
		m_nSelection = nIndex; // store the index
	}
}

/***************************************************************************/
// CSegment::GetPrevious Get previous annotation segment
// Returns the index of the previous annotation segment (first character in
// the string) before the one with the given index. If the return is -1,
// there is no previous segment. If the parameter nIndex delivers -1, the
// function takes the actual selected segment to look for.
/***************************************************************************/
int CSegment::GetPrevious(int nIndex) const
{
	// find out, which character is the reference
	int nReference;
	if (nIndex >= 0) 
	{
		nReference = nIndex - 1; // user defined reference
		if (nReference >= GetOffsetSize()) 
		{
			nReference = GetOffsetSize() - 1;
		}
	} 
	else 
	{ // take current selection
		if (m_nSelection != -1) 
		{
			nReference = m_nSelection - 1;
		} 
		else 
		{
			nReference = GetOffsetSize() - 1; // default for nothing has been selected yet
		}
	}
	if (nReference < 0) 
	{
		return -1; // there is no previous character
	}
	DWORD dwOffset = GetOffset(nReference);
	nReference--;
	while ((nReference >= 0) && (GetOffset(nReference) == dwOffset)) 
	{
		nReference--;
	}
	return ++nReference;
}

/***************************************************************************/
// CSegment::GetNext Get next annotation segment
// Returns the index of the next annotation segment (first character in the
// string) next to the one with the given index. If the return is -1, there
// is no next segment. If the parameter nIndex delivers -1, the function
// takes the actual selected segment to look for.
/***************************************************************************/
int CSegment::GetNext(int nIndex) const
{
	// find out, which character is the reference
	int nReference;
	if (nIndex >= 0) {
		nReference = nIndex; // user defined reference
	} else { // take current selection
		if (m_nSelection != -1) {
			nReference = m_nSelection;
		} else {
			nReference = 0; // default for nothing has been selected yet
		}
	}
	DWORD dwOffset = GetOffset(nReference);
	nReference++;
	while ((nReference < GetOffsetSize()) && (GetOffset(nReference) == dwOffset))
	{
		nReference++;
	}
	if (nReference == GetOffsetSize()) {
		return -1; // no next character
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

	DWORD *pFirst = &((*m_pOffset)[0]);
	// pLast should be set to the last position + 1
	DWORD *pLast = pFirst + GetOffsetSize();
	DWORD *pLowerBound = std::lower_bound( pFirst, pLast, dwOffset);
	if ((pLowerBound != pLast) && (*pLowerBound == dwOffset)) {
		return pLowerBound - pFirst;
	}

	return -1;
}

// SDM 1.5 Test 11.0
/***************************************************************************/
// CSegment::FindStop Find the annotation segment stop match
// Returns the index of the annotation segment (first character in the
// string) whichs offset matches the parameter. If there is no match, the
// function returns -1.
/***************************************************************************/
int CSegment::FindStop(DWORD dwStop) const {

	if (IsEmpty()) {
		return -1;
	}

	// Use std::lower_bound which has O(log(N)) for performance reasons
	// Needed fo large annotated files
	DWORD *pFirst = &((*m_pOffset)[0]);
	DWORD *pLast = pFirst + (GetOffsetSize()-1);
	DWORD *pLowerBound = std::lower_bound(pFirst, pLast, dwStop);

	int nLoop = 0;
	if (pLowerBound != pLast) {
		nLoop = pLowerBound - pFirst;
	} else {
		nLoop = GetOffsetSize()-1;
	}

	for ( ; nLoop >= 0; --nLoop) {
		if (GetStop(nLoop) < dwStop) {
			return -1; // no match
		}
		if (GetStop(nLoop) == dwStop) {
			while (nLoop && GetStop(nLoop -1) == dwStop) {
				--nLoop;
			}
			return nLoop; // match
		}
	}
	return -1; // no match
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
* If there is no segment, it returns -1.
*
* if bWithin=false, it returns last index if it is above last index.
*
* if bWithin=true, index is return if within segment.  -1 returned otherwise
*
***************************************************************************/
int CSegment::FindFromPosition( DWORD dwPosition, BOOL bWithin) const {
	
	if (IsEmpty()) {
		// there are no segments
		return -1;
	}

	// check if position above highest segment
	DWORD dwTempPos = dwPosition;

	// get end position of last segment
	DWORD dwStop = GetStop(GetOffsetSize()-1);
	if (dwStop <= dwPosition) {
		// if we are at or beyond the last end position then 
		// set position with last end position
		dwTempPos = dwStop - 1;
	}

	// Use std::lower_bound which has O(log(N)) for performance reasons
	// Needed for large annotated files
	DWORD *pFirst = &((*m_pOffset)[0]);
	DWORD *pLast = pFirst + (GetOffsetSize()-1);
	DWORD *pLowerBound = std::lower_bound(pFirst, pLast, dwPosition);

	int nLoop = 0;
	if (pLowerBound != pLast) {
		// we are not at end, use delta
		nLoop = pLowerBound - pFirst;
	} else {
		// use max index
		nLoop = GetOffsetSize()-1;
	}

	for ( ; nLoop >= 0; nLoop = GetPrevious(nLoop)) {
		if (GetStop(nLoop) <= dwTempPos) {
			break; // this is it
		}
	}

	if (nLoop >=0) {
		nLoop = GetNext(nLoop);  // selected character is one position higher
	} else {
		nLoop = 0;
	}

	if (bWithin) {
		if ((dwPosition < GetOffset(nLoop)) || (dwPosition > GetStop(nLoop))) {
			return -1;
		}
	}
	return nLoop;
}

/***************************************************************************/
// CSegment::GetSegmentLength Find the segment length
// Returns the length (in characters) of the given segment.
/***************************************************************************/
int CSegment::GetSegmentLength(int nIndex) const
{
	DWORD dwOffset = GetOffset(nIndex);
	int nLength = m_pAnnotation->GetLength();
	int nLoop;
	for (nLoop = nIndex + 1; nLoop < nLength; nLoop++)
	{
		if (dwOffset != GetOffset(nLoop)) break;
	}
	return (nLoop - nIndex); // length of segment
}

/***************************************************************************/
// CSegment::GetSegmentString Return the segment character string
/***************************************************************************/
CSaString CSegment::GetSegmentString(int nIndex)   const
{
	CSaString szSegment = m_pAnnotation->GetAt(nIndex);
	DWORD dwOffset = GetOffset(nIndex++);
	while((nIndex < m_pAnnotation->GetLength()) && (dwOffset == GetOffset(nIndex)))
	{
		szSegment += m_pAnnotation->GetAt(nIndex++);
	}
	return szSegment;
}

/***************************************************************************/
// CSegment::Adjust Adjusts positions of an annotation segment
/***************************************************************************/
void CSegment::Adjust(CSaDoc* pDoc, int nIndex, DWORD dwOffset, DWORD dwDuration)
{
	DWORD dwOldOffset = GetOffset(nIndex);
	DWORD dwOldStop = GetStop(nIndex);

	for (int nLoop = nIndex; nLoop < GetOffsetSize(); nLoop++)
	{
		if (GetOffset(nLoop) == dwOldOffset)
		{
			m_pOffset->SetAt(nLoop, dwOffset);			// set new offset
			m_pDuration->SetAt(nLoop, dwDuration);		// set new duration
		}
		else
		{
			break;
		}
	}

	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
	{
		CSegment* pSegment = pDoc->GetSegment(nLoop);
		if (pSegment && (pSegment->GetMasterIndex() == m_nAnnotationIndex))
		{
			int nIndex = pSegment->FindOffset(dwOldOffset);

			if (nIndex != -1)
			{
				pSegment->Adjust(pDoc, nIndex, dwOffset, pSegment->GetStop(nIndex) - dwOffset);
			}

			nIndex = pSegment->FindStop(dwOldStop);
			if (nIndex != -1)
			{
				pSegment->Adjust(pDoc, nIndex, pSegment->GetOffset(nIndex), dwOffset + dwDuration - pSegment->GetOffset(nIndex));
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
int CSegment::CheckCursors(CSaDoc* pDoc, BOOL bOverlap) const
{
	// get pointer to view
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);

	return CheckPosition(pDoc, pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), MODE_AUTOMATIC, bOverlap);
}



BOOL CSegment::NeedToScroll(CSaView & saView, int index) const
{
	ASSERT(index >= 0 && index < GetOffsetSize());

	DWORD    viewLeftEdge = (int) saView.GetDataPosition(0);
	DWORD    viewRightEdge = viewLeftEdge + saView.GetDataFrame();
	DWORD    anotLeftEdge = GetOffset(index);
	DWORD    anotRightEdge = anotLeftEdge + GetDuration(index);

	return ((anotLeftEdge < viewLeftEdge) ||
		(anotRightEdge > viewRightEdge));
}


int CSegment::FirstVisibleIndex(CSaDoc & SaDoc) const
{
	POSITION pos = SaDoc.GetFirstViewPosition();
	CSaView  * pView = (CSaView*)SaDoc.GetNextView(pos);

	for (int index=0;index<GetOffsetSize();index++)
	{
		if (!NeedToScroll(*pView,index))
		{
			return index;
		}
	}

	return 0;
}

int CSegment::LastVisibleIndex(CSaDoc & SaDoc) const
{
	POSITION pos = SaDoc.GetFirstViewPosition();
	CSaView  * pView = (CSaView*)SaDoc.GetNextView(pos);

	for (int index=GetOffsetSize()-1;index>=0;index--)
	{
		if (!NeedToScroll(*pView,index))
		{
			return index;
		}
	}

	return 0;
}

/***************************************************************************/
// CSegment::FindPrev find next segment matching strToFind and hilite it.
//***************************************************************************/
int CSegment::FindPrev(int fromIndex, const CSaString & strToFind, CSaDoc & )
{
	ASSERT(fromIndex >= -1);

	int ret = -1;
	CSaString sSegment(*m_pAnnotation);

	if (fromIndex > 0 || fromIndex == -1)
	{
		if (fromIndex>0)
		{
			sSegment = sSegment.Left(fromIndex);
		}

		int idxFirstChar = sSegment.ReverseFind(strToFind.GetAt(0));

		while (idxFirstChar >= 0)
		{
			CSaString str(sSegment.Mid(idxFirstChar,strToFind.GetLength()));
			if (str == strToFind)
			{
				break;
			}
			sSegment = sSegment.Left(idxFirstChar);
			idxFirstChar = sSegment.ReverseFind(strToFind.GetAt(0));
		}

		if (idxFirstChar >= 0)
		{
			int idxLastChar = idxFirstChar + strToFind.GetLength();

			if (idxLastChar >= sSegment.GetLength())
			{
				idxLastChar = sSegment.GetLength() - 1;
			}
			DWORD dwStart = GetOffset(idxFirstChar);
			DWORD dwEnd   = GetOffset(idxLastChar);

			if (dwEnd == dwStart)
			{
				dwEnd += GetDuration(idxLastChar);
			}
			ret = FindFromPosition(dwStart);
		}
	}
	return ret;
}



/***************************************************************************/
// CSegment::FindNext find next segment matching strToFind and hilite it.
//***************************************************************************/
int CSegment::FindNext(int fromIndex, const CSaString & strToFind, CSaDoc & )
{
	ASSERT(fromIndex >= -1);
	ASSERT(!IsEmpty());

	CSaString sSegment(*m_pAnnotation);
	int offset = 0;
	int ret = -1;

	if (fromIndex >= 0)
	{
		offset = fromIndex + GetSegmentLength(fromIndex);
		sSegment = sSegment.Right(sSegment.GetLength() - offset);
	}
	int idxFirstChar = sSegment.Find(strToFind);

	if (idxFirstChar >= 0)
	{
		idxFirstChar += offset;

		int idxLastChar = idxFirstChar + strToFind.GetLength();

		if (idxLastChar >= sSegment.GetLength())
		{
			idxLastChar = sSegment.GetLength() - 1 + offset;
		}
		DWORD dwStart = GetOffset(idxFirstChar);
		DWORD dwEnd   = GetOffset(idxLastChar);

		if (dwEnd == dwStart)
		{
			dwEnd += GetDuration(idxLastChar);
		}
		ret = FindFromPosition(dwStart);
	}

	return ret;
}



/***************************************************************************/
// CSegment::Match return TRUE if text at index matches strToFind
//***************************************************************************/
BOOL CSegment::Match(int index, const CSaString & strToFind)
{
	ASSERT(index >= -1);
	ASSERT(!IsEmpty());
	BOOL ret = FALSE;
	CSaString sSegment(*m_pAnnotation);

	if (index >= 0 &&
		((index + strToFind.GetLength() - 1) < sSegment.GetLength()))
	{
		sSegment = sSegment.Mid(index, strToFind.GetLength());
		if (sSegment == strToFind)
		{
			ret = TRUE;
		}
	}

	return ret;
}

void CSegment::SelectSegment(CSaDoc & SaDoc, int index)
{
	DWORD dwStart = GetOffset(index);
	DWORD dwEnd   = GetStop(index);
	POSITION pos = SaDoc.GetFirstViewPosition();
	CSaView  * pView = (CSaView*)SaDoc.GetNextView(pos);

	pView->ChangeAnnotationSelection(this, index, dwStart, dwEnd);
	pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}

/***************************************************************************/
// CSegment::AdjustCursorsToSnap Adjust cursors to apropriate snap position
// Adjust the current cursor positions to the positions, that they will have,
// when a segment will be selected.
//***************************************************************************/
void CSegment::AdjustCursorsToSnap(CDocument* pSaDoc)
{
	// get requested cursor alignment
	// snap the cursors first to appropriate position
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);
	DWORD dwNewOffset = pView->GetStartCursorPosition();
	DWORD dwNewDuration = pView->GetStopCursorPosition();
	pView->SetStartCursorPosition(dwNewOffset, SNAP_RIGHT);
	pView->SetStopCursorPosition(dwNewDuration, SNAP_LEFT);
}

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
int CDependentSegment::AlignOffsetToMaster(CDocument* pSaDoc, DWORD* pdwOffset) const
{
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	// get pointer to master offset array
	CSegment* pSegment = pDoc->GetSegment(m_nMasterIndex);
	int nLength = pSegment->GetOffsetSize();
	if (nLength == 0) return -1; // no master segment yet
	// check the position
	int nLoop;
	for (nLoop = 0; nLoop < nLength; nLoop++)
	{
		DWORD dwOffset = pSegment->GetOffset(nLoop);
		if (*pdwOffset < dwOffset) // this offset is larger
			break;
	}
	nLoop = pSegment->GetPrevious(nLoop); // align to one position lower
	if (nLoop >= 0)
	{
		*pdwOffset = pSegment->GetOffset(nLoop); // get master offset
	}
	return nLoop;
}

// SDM 1.5 Test 11.0
/***************************************************************************/
// CDependentSegment::AlignStopToSegment Align to the nearest nMaster segment
/***************************************************************************/
int CDependentSegment::AlignStopToMaster(CDocument* pSaDoc, DWORD* pdwStop) const
{
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	// get pointer to master offset array
	CSegment* pSegment = pDoc->GetSegment(m_nMasterIndex);
	int nLength = pSegment->GetOffsetSize();
	if (nLength == 0) return -1; // no master segment yet
	// check the position
	int nLoop;
	for (nLoop = 0; nLoop < nLength; nLoop++)
	{
		DWORD dwStop = pSegment->GetStop(nLoop);
		if (*pdwStop < dwStop) // this offset is larger
			break;
	}
	nLoop = pSegment->GetPrevious(nLoop); // align to one position lower
	if (nLoop >= 0)
	{
		*pdwStop = pSegment->GetStop(nLoop); // get master offset
	}
	return nLoop;
}

// SDM 1.5 Test11.0 Modified to support new gloss stop position
/***************************************************************************/
// CDependentSegment::LimitPosition Limit positions for start and stop based
// on segment rules for update
/***************************************************************************/
void CDependentSegment::LimitPosition(CSaDoc* pSaDoc, DWORD& dwStart, DWORD& dwStop, int) const
{
	// get pointer to view
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	CSegment* pMaster = pDoc->GetSegment(m_nMasterIndex);

	int nTextIndex = GetSelection();

	if (nTextIndex != -1)
	{
		int nIndex;

		// Check for obscuring previous segment
		nIndex = GetPrevious(nTextIndex);
		if (nIndex != -1)
		{
			nIndex = pMaster->FindStop(GetStop(nIndex));
			if (nIndex !=-1) // Properly Aligned to master
			{
				nIndex = pMaster->GetNext(nIndex);
				if ((nIndex !=-1) && (dwStart < pMaster->GetOffset(nIndex)))
					dwStart = pMaster->GetOffset(nIndex);
				if ((nIndex !=-1) && (dwStop < pMaster->GetStop(nIndex)))
					dwStop = pMaster->GetStop(nIndex);
			}
		}
		// Check for obscuring next segment
		nIndex = GetNext(nTextIndex);
		if (nIndex != -1)
		{
			nIndex = pMaster->FindOffset(GetOffset(nIndex));
			if (nIndex !=-1)
			{
				nIndex = pMaster->GetPrevious(nIndex);
				if ((nIndex !=-1) && (dwStop > pMaster->GetStop(nIndex)))
					dwStop = pMaster->GetStop(nIndex);
				if ((nIndex !=-1) && (dwStart > pMaster->GetOffset(nIndex)))
					dwStart = pMaster->GetOffset(nIndex);
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
int CDependentSegment::AdjustPositionToMaster(CDocument* pSaDoc, DWORD& dwNewOffset, DWORD& dwNewStop) const // SDM 1.5Test8.2
{
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	DWORD dwAlignStart[2] =
	{
		dwNewOffset
	};
	DWORD dwAlignStop[2] =
	{
		dwNewStop
	};
	CSegment* pMaster = pDoc->GetSegment(m_nMasterIndex);
	int ret; // SDM 1.5Test8.2 added return parameter

	// Find Nearest master Start
	int nIndex = AlignOffsetToMaster(pDoc, dwAlignStart);
	if (nIndex == -1)
	{
		if (GetOffsetSize() > 0)
		{
			dwNewOffset = GetOffset(0);
			ret = 0;
		}
		else
		{
			// Should never get here, but we may... SDM 1.5Test8.2
			dwNewOffset = 0; 
			ret = nIndex;
		}
	}
	else
	{
		// GetStart of Next Segment
		ret = nIndex;
		nIndex = pMaster->GetNext(nIndex);

		if (nIndex != -1)
		{
			dwAlignStart[1] = pMaster->GetOffset(nIndex);
			if ((dwAlignStart[1]-dwNewOffset) < (dwNewOffset - dwAlignStart[0]))
			{
				dwNewOffset = dwAlignStart[1];
				ret = nIndex;
			}
			else
			{
				dwNewOffset = dwAlignStart[0];
			}
		}
		else
		{
			dwNewOffset = dwAlignStart[0];
		}
	}

	// Find Nearest master Stop
	nIndex = AlignStopToMaster(pDoc, dwAlignStop);
	if (nIndex == -1)
	{
		if (GetOffsetSize() > 0) // SDM 1.5Test11.0 if less than one segment stop is end of file
			dwNewStop = GetStop(0);
		else
			dwNewStop = pMaster->GetStop(pMaster->GetOffsetSize() - 1);
	}
	else
	{
		// Get Start of next segment
		nIndex = pMaster->GetNext(nIndex);
		if (nIndex != -1)
		{
			dwAlignStop[1] = pMaster->GetStop(nIndex);
		}
		else
		{
			dwAlignStop[1] = pMaster->GetStop(pMaster->GetOffsetSize() - 1);
		}

		if ((dwAlignStop[1]-dwNewStop) < (dwNewStop - dwAlignStop[0]))
		{
			dwNewStop = dwAlignStop[1];
		}
		else
		{
			dwNewStop = dwAlignStop[0];
		}
	}

	if (dwNewOffset >= dwNewStop)  // Force separate alignment
	{
		nIndex = pMaster->FindStop(dwNewStop);
		if (nIndex!=-1)
		{
			nIndex = pMaster->GetNext(nIndex);
			if (nIndex != -1)
				dwNewStop = pMaster->GetStop(nIndex);
		}
	}
	if (dwNewOffset >= dwNewStop)  // Force separate alignment
	{
		nIndex = pMaster->FindOffset(dwNewOffset);
		if (nIndex!=-1)
		{
			nIndex = pMaster->GetPrevious(nIndex);
			if (nIndex != -1)
				dwNewOffset = pMaster->GetOffset(nIndex);
			else
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
void CDependentSegment::AdjustCursorsToMaster(CDocument* pSaDoc, BOOL bAdjust, DWORD* pdwOffset, DWORD* pdwStop) const
{
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);
	DWORD dwNewOffset = pView->GetStartCursorPosition();
	DWORD dwNewStop = pView->GetStopCursorPosition();

	AdjustPositionToMaster(pDoc, dwNewOffset, dwNewStop);

	if (pdwOffset) *pdwOffset = dwNewOffset;
	if (pdwStop) *pdwStop = dwNewStop;

	if (bAdjust)
	{
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
int CDependentSegment::CheckPositionToMaster(CSaDoc* pSaDoc, DWORD dwStart, DWORD dwStop, EMode nMode) const
{
	// get pointer to view
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	CSegment* pMaster = pDoc->GetSegment(m_nMasterIndex);

	int nTextIndex = GetSelection();

	if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(nTextIndex != -1)) // segment selected (edit)
	{
		int nIndex;

		// get the actual aligned position
		AdjustPositionToMaster(pDoc, dwStart, dwStop);

		if (dwStart >= dwStop) return -1; // zero duration (or negative)
		if (dwStart == GetOffset(nTextIndex) && (dwStop == GetStop(nTextIndex))) return -1; // no change

		// Check for obscuring previous segment
		nIndex = GetPrevious(nTextIndex);
		if (nIndex != -1)
		{
			nIndex = pMaster->FindStop(GetStop(nIndex));
			if (nIndex !=-1) // Properly Aligned to master
			{
				nIndex = pMaster->GetNext(nIndex);
				if ((nIndex !=-1) && (dwStart < pMaster->GetOffset(nIndex)))
					return -1;
				if ((nIndex !=-1) && (dwStop < pMaster->GetStop(nIndex)))
					return -1;
			}
		}
		// Check for obscuring next segment
		nIndex = GetNext(nTextIndex);
		if (nIndex != -1)
		{
			nIndex = pMaster->FindOffset(GetOffset(nIndex));
			if (nIndex !=-1)
			{
				nIndex = pMaster->GetPrevious(nIndex);
				if ((nIndex !=-1) && (dwStop > pMaster->GetStop(nIndex)))
					return -1;
				if ((nIndex !=-1) && (dwStart > pMaster->GetOffset(nIndex)))
					return -1;
			}
		}
		return nTextIndex;
	}
	else // Add
	{
		int nSegment = AlignOffsetToMaster(pSaDoc, &dwStart);
		if (nSegment == -1) return -1; // wrong place
		// now check if this position is already occupied
		int nLength = GetOffsetSize();
		if (nLength == 0) return 0; // no segment yet
		int nLoop;
		for (nLoop = 0; nLoop < nLength; nLoop++) {
			if (dwStart <= GetOffset(nLoop)) // this offset is bigger or equal
			{
				if (dwStart == GetOffset(nLoop))
					return -1; // already occupied
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
void CDependentSegment::Add( CSaDoc* pDoc, DWORD dwStart, CSaString& szString, BOOL , BOOL bCheck)
{
	// get pointer to view
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);

	// get the offset and duration from master
	int nSegment = pDoc->GetSegment(m_nMasterIndex)->FindOffset(dwStart);
	if (nSegment == -1) return; // return on error

	DWORD dwDuration = pDoc->GetSegment(m_nMasterIndex)->GetDuration(nSegment);

	int nPos = CheckPosition(pDoc, dwStart, dwStart + dwDuration, MODE_ADD); // get the insert position
	if (nPos == -1) return; // return on error

	// save state for undo ability
	if (bCheck) pDoc->CheckPoint();

	// insert or append the new dependent segment
	if (!Insert(nPos, &szString, 0, dwStart, dwDuration)) return; // return on error
	pDoc->SetModifiedFlag(TRUE); // document has been modified
	pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
	pView->ChangeAnnotationSelection(this, nPos, dwStart, dwStart + dwDuration); // change the selection
	pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}

//###########################################################################
// CTextSegment
// class to do all the handling with text annotation segments. This class is
// the base class for segments, that contain independent character strings
// (text) for each segment. They still depend on master annotation and must
// be aligned with it (base class is CDependentSegment class).

/////////////////////////////////////////////////////////////////////////////
// CTextSegment construction/destruction/creation

/***************************************************************************/
// CTextSegment::CTextSegment Constructor
/***************************************************************************/
CTextSegment::CTextSegment(int index, int master) : CDependentSegment(index,master)
{
	m_pTexts = new CStringArray; // create string array object
}

/***************************************************************************/
// CTextSegment::~CTextSegment Destructor
/***************************************************************************/
CTextSegment::~CTextSegment()
{
	if (m_pTexts) 
	{
		delete m_pTexts;
		m_pTexts = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTextSegment helper functions

/***************************************************************************/
// CTextSegment::Insert Insert/append a text segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CTextSegment::SetAt( const CSaString* pszString, bool delimiter, DWORD dwStart, DWORD dwDuration)
{
	// prepare delimiter
	CSaString szDelimiter = WORD_DELIMITER;
	if (delimiter) 
	{
		szDelimiter.SetAt(0, TEXT_DELIMITER);
	}

	try 
	{
		int nIndex = FindOffset( dwStart);
		ASSERT(nIndex>=0);
		if (pszString!=NULL) 
		{
			m_pTexts->SetAtGrow(nIndex, szDelimiter + *pszString);
		}
		else 
		{
			m_pTexts->SetAtGrow(nIndex, szDelimiter);
		}
		CSegment::SetAt( nIndex, dwStart, dwDuration);
	} 
	catch(CMemoryException e) 
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return FALSE;
	}
	return TRUE;
}

/***************************************************************************/
// CTextSegment::Insert Insert/append a text segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CTextSegment::Insert(int nIndex, const CSaString* pszString, bool delimiter, DWORD dwStart, DWORD dwDuration)
{
	// prepare delimiter
	CSaString szDelimiter = WORD_DELIMITER;
	if (delimiter) 
	{
		szDelimiter.SetAt(0, TEXT_DELIMITER);
	}

	try 
	{
		if (pszString!=NULL) 
		{
			m_pTexts->InsertAt(nIndex, szDelimiter + *pszString, 1);
		}
		else 
		{
			m_pTexts->InsertAt(nIndex, szDelimiter, 1);
		}
		InsertAt( nIndex, dwStart, dwDuration);
	} 
	catch(CMemoryException e) 
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return FALSE;
	}
	return TRUE;
}

/***************************************************************************/
// CTextSegment::CaluculateDuration calculate segment duration from master data
/***************************************************************************/
DWORD CTextSegment::CalculateDuration(CSaDoc* pDoc, const int nIndex) const
{
	CSegment* pMaster = (CSegment*) pDoc->GetSegment(m_nMasterIndex);

	if ((nIndex < 0) || (nIndex >= (GetOffsetSize())))
		return DWORD(-1);
	if ((nIndex + 1) == GetOffsetSize())
	{
		return pMaster->GetStop(pMaster->GetOffsetSize()-1) - GetOffset(nIndex);
	}
	else
	{
		DWORD dwStop = pMaster->GetStop(pMaster->FindOffset(GetOffset(GetNext(nIndex)))-1);
		if (dwStop == -1)
			return DWORD(-1);
		return  dwStop - GetOffset(nIndex);
	}
}

/***************************************************************************/
// CTextSegment::DeleteContents Delete all contents of the segment arrays
/***************************************************************************/
void CTextSegment::DeleteContents()
{
	m_pTexts->RemoveAll();
	CSegment::DeleteContents(); // call the base class to delete positions
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
void CTextSegment::LimitPosition(CSaDoc* pSaDoc, DWORD& dwStart, DWORD& dwStop, int) const
{
	// get pointer to view
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	CSegment* pMaster = pDoc->GetSegment(m_nMasterIndex);

	int nTextIndex = GetSelection();

	if (nTextIndex != -1)
	{
		int nIndex;

		// Check for obscuring previous segment
		nIndex = GetPrevious(nTextIndex);
		if (nIndex != -1)
		{
			nIndex = pMaster->FindOffset(GetOffset(nIndex));
			if (nIndex !=-1) // Properly Aligned to master
			{
				nIndex = pMaster->GetNext(nIndex);
				if ((nIndex !=-1) && (dwStart < pMaster->GetOffset(nIndex)))
					dwStart = pMaster->GetOffset(nIndex);
				if ((nIndex !=-1) && (dwStop < pMaster->GetStop(nIndex)))
					dwStop = pMaster->GetStop(nIndex);
			}
		}
		// Check for obscuring next segment
		nIndex = GetNext(nTextIndex);
		if (nIndex != -1)
		{
			nIndex = pMaster->FindStop(GetStop(nIndex));
			if (nIndex !=-1)
			{
				nIndex = pMaster->GetPrevious(nIndex);
				if ((nIndex !=-1) && (dwStop > pMaster->GetStop(nIndex)))
					dwStop = pMaster->GetStop(nIndex);
				if ((nIndex !=-1) && (dwStart > pMaster->GetOffset(nIndex)))
					dwStart = pMaster->GetOffset(nIndex);
			}
			else  // no master segment at stop boundary
			{
				nIndex = GetNext(nTextIndex); // 1.5Test8.1
				DWORD dwOffset = GetStop(nIndex);
				nIndex = AlignStopToMaster(pSaDoc, &dwOffset); // find the nearest master segment to the end of next text segment
				if ((nIndex !=-1) && (dwStop > pMaster->GetStop(nIndex)))
					dwStop = pMaster->GetStop(nIndex);
				if ((nIndex !=-1) && (dwStart > pMaster->GetOffset(nIndex)))
					dwStart = pMaster->GetOffset(nIndex);
			}
		}
		else // last gloss
		{
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
int CTextSegment::CheckPosition(CSaDoc* pSaDoc, DWORD dwStart, DWORD dwStop, EMode nMode, BOOL) const
{
	// get pointer to view
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer

	// get the actual aligned position
	DWORD dwAlignedStart = dwStart;
	DWORD dwAlignedStop = dwStop;

	AdjustPositionToMaster(pDoc, dwAlignedStart, dwAlignedStop);

	int nTextIndex = GetSelection();

	if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(nTextIndex != -1)) // segment selected (edit)
	{
		int nIndex;

		if (dwAlignedStart >= dwAlignedStop) return -1; // zero duration (or negative)
		if (dwAlignedStart == GetOffset(nTextIndex) && (dwAlignedStop == GetStop(nTextIndex))) return -1; // no change

		// Check for obscuring previous segment
		nIndex = GetPrevious(nTextIndex);
		if (nIndex != -1)
		{
			if (GetOffset(nIndex) >= dwAlignedStart) return -1;
		}
		// Check for obscuring present segment
		nIndex = GetNext(nTextIndex);
		if (nIndex != -1)
		{
			if (GetStop(nIndex) <= dwAlignedStop) return -1;
		}
		return nTextIndex;
	}
	else // Add
	{
		if (pDoc->GetSegment(m_nMasterIndex)->IsEmpty()) return -1;
		if (IsEmpty()) return 0; // No Text in Segment insert at 0 (Start)
		nTextIndex = 0; // GetNext does not work for -1
		while ((nTextIndex != -1) && (GetOffset(nTextIndex) < dwAlignedStart))
		{
			nTextIndex = GetNext(nTextIndex);
		}

		if (nTextIndex == -1) return GetOffsetSize(); // Insert at End

		if (GetOffset(nTextIndex) == dwAlignedStart) return -1;

		return nTextIndex;
	}
	return -1;
}

// SDM 1.06.5
/***************************************************************************/
// CTextSegment::Add Add text segment
/***************************************************************************/
void CTextSegment::Add(CSaDoc* pDoc, DWORD dwStart, CSaString& szString, bool bDelimiter, bool bCheck) // add a segment
{
	// get pointer to view
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);

	int nPos = FindFromPosition(dwStart,TRUE);
	if (nPos==-1)
		nPos = FindFromPosition(dwStart,FALSE);
	else
		nPos++;

	CSegment* pMaster = pDoc->GetSegment(m_nMasterIndex);
	int nMaster;

	if ((nPos == -1) || (nPos >= GetOffsetSize()) || dwStart > GetStop(nPos))
	{
		nMaster = pMaster->GetOffsetSize();
	}
	else
	{
		nMaster = pMaster->FindOffset(GetOffset(nPos));
	}

	ASSERT(nMaster > 0); // must be master segment to attach to

	DWORD dwStop = pMaster->GetStop(nMaster-1);

	ASSERT(dwStop > dwStart);

	nPos = CheckPosition(pDoc, dwStart, dwStop, CSegment::MODE_ADD ); // get the insert position
	if (nPos == -1) return; // return on error

	ASSERT(dwStop > dwStart);

	// save state for undo ability
	if (bCheck) pDoc->CheckPoint();

	// insert or append the new segment
	if (!Insert(nPos, &szString, bDelimiter, dwStart, dwStop - dwStart)) 
		return; // return on error

	// move the end of the previous text segment
	if (nPos > 0)
		Adjust(pDoc, nPos - 1, GetOffset(nPos - 1), CalculateDuration(pDoc, nPos -1));
	pDoc->SetModifiedFlag(TRUE); // document has been modified
	pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
	pView->ChangeAnnotationSelection(this, nPos); // change the selection
	pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}


// SDM 1.5Test11.3
/***************************************************************************/
// CTextSegment::Remove Remove text segment
/***************************************************************************/
void CTextSegment::Remove(CDocument* pSaDoc, BOOL bCheck)
{
	// get pointer to view
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer

	// save state for undo ability
	if (bCheck) pDoc->CheckPoint();

	// move the end of the previous text segment
	if (m_nSelection > 0)
	{
		SetDurationAt(m_nSelection - 1, GetStop(m_nSelection) - GetOffset(m_nSelection - 1));
	}
	CDependentSegment::Remove(pSaDoc, FALSE);
}

/***************************************************************************/
// CTextSegment::RemoveNoRefresh Remove text segment
/***************************************************************************/
DWORD CTextSegment::RemoveNoRefresh(CDocument*)
{
	//SDM 1.5Test8.1 return oldOffset
	DWORD dwOffset = GetOffset(m_nSelection);

	// change the segment arrays
	m_pTexts->RemoveAt(m_nSelection, 1);
	RemoveAt(m_nSelection,1);

	return dwOffset;
}



// SDM 1.5Test8.3
void CTextSegment::ReplaceSelectedSegment(CDocument* pSaDoc, const CSaString & str1)
{
	// get pointer to view
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);

	if (m_nSelection != -1) 
	{
		m_pTexts->SetAt(m_nSelection, str1);
	}

	pDoc->SetModifiedFlag(TRUE); // document has been modified
	pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified

	int nSaveSelection = m_nSelection; // 1.5Test10.2
	pView->ChangeAnnotationSelection(this, m_nSelection); // deselect // 1.5Test10.2
	pView->ChangeAnnotationSelection(this, nSaveSelection); // select again // 1.5Test10.2
	pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}



/***************************************************************************/
// CSegment::FindNext find next segment matching strToFind and hilite it.
//***************************************************************************/
int CTextSegment::FindNext(int fromIndex, const CSaString & strToFind, CSaDoc &)
{
	ASSERT(fromIndex >= -1);
	ASSERT(!IsEmpty());

	int index    = fromIndex + 1;

	for (;index<=m_pTexts->GetUpperBound();index++)
	{
		if (m_pTexts->GetAt(index) == strToFind)
		{
			break;
		}
	}

	return ((index<=m_pTexts->GetUpperBound()) ? index : -1);
}


/***************************************************************************/
// CSegment::Match find next segment matching strToFind and hilite it.
//***************************************************************************/
BOOL CTextSegment::Match(int index, const CSaString & strToFind)
{
	ASSERT(index >= -1);
	ASSERT(!IsEmpty());
	BOOL ret = FALSE;

	if (index >= 0 && index <= m_pTexts->GetUpperBound())
	{
		ret = (m_pTexts->GetAt(index) == strToFind);
	}

	return ret;
}




/***************************************************************************/
// CTextSegment::FindPrev find next segment matching strToFind and hilite it.
/***************************************************************************/
int CTextSegment::FindPrev(int fromIndex, const CSaString & strToFind, CSaDoc &)
{
	ASSERT(fromIndex >= -1);
	ASSERT(!IsEmpty());
	int index = -1;

	if (fromIndex > 0 || fromIndex == -1)
	{
		index    = (fromIndex == -1) ? m_pTexts->GetUpperBound() : fromIndex - 1;

		for (;index>=0;index--)
		{
			if (m_pTexts->GetAt(index) == strToFind)
			{
				break;
			}
		}
	}

	return index;
}


/***************************************************************************/
// CTextSegment::CountWords
//***************************************************************************/i
int CTextSegment::CountWords()
{
	int nIndex = 0;
	int nWords = 0;
	if (IsEmpty()) return nWords;
	while(nIndex!=-1)
	{
		if (GetText(nIndex)[0]==WORD_DELIMITER) nWords++;
		nIndex=GetNext(nIndex);
	}
	return nWords;

}

/***************************************************************************/
// CTextSegment::Serialize
//***************************************************************************/i
void CTextSegment::Serialize(CArchive& ar)
{
	CSegment::Serialize(ar);
	if (ar.IsStoring())
	{
		ar << CSaString("CTextSegmentDetail tag");
	}
	else
	{
		CSaString detailTagCheck;
		ar >> detailTagCheck;
		SA_ASSERT(detailTagCheck == "CTextSegmentDetail tag");
	}
	m_pTexts->Serialize(ar);
}

//SDM 1.5Test8.1
/////////////////////////////////////////////////////////////////////////////
// CDependentTextSegment helper functions

/***************************************************************************/
// CDependentTextSegment::Insert Insert/append a text segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
//
// When inserting beyond the end of the current list, nIndex is the next
// available index.  For example if we are inserting at the 4 gloss segment
// but only reference segments 0 and 1 exist, then the next index is 2, not 3.
/***************************************************************************/
BOOL CDependentTextSegment::SetAt( const CSaString* pszString, bool, DWORD dwStart, DWORD dwDuration) {
	try 
	{
		if ((pszString) && (pszString->GetLength())) 
		{
			int nIndex = FindOffset( dwStart);
			ASSERT(nIndex>=0);
			m_pTexts->SetAtGrow(nIndex, *pszString);
			CSegment::SetAt(nIndex,dwStart,dwDuration);
		}
	} 
	catch (CMemoryException e) 
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return FALSE;
	}
	return TRUE;
}

/***************************************************************************/
// CDependentTextSegment::Insert Insert/append a text segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
//
// When inserting beyond the end of the current list, nIndex is the next
// available index.  For example if we are inserting at the 4 gloss segment
// but only reference segments 0 and 1 exist, then the next index is 2, not 3.
/***************************************************************************/
BOOL CDependentTextSegment::Insert( int nIndex, const CSaString* pszString, bool, DWORD dwStart, DWORD dwDuration) {
	try 
	{
		if ((pszString) && (pszString->GetLength())) 
		{
			m_pTexts->InsertAt(nIndex, *pszString, 1);
			InsertAt(nIndex,dwStart,dwDuration);
		}
	} 
	catch (CMemoryException e) 
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return FALSE;
	}
	return TRUE;
}

//SDM 1.5Test8.1
/////////////////////////////////////////////////////////////////////////////
// CDependentTextSegment helper functions

/***************************************************************************/
// CDependentTextSegment::Insert Insert/append a text segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CDependentTextSegment::SetText( int nIndex, const CSaString* pszString, int, DWORD, DWORD)
{
	try
	{
		if ((pszString) && (pszString->GetLength()))
		{
			m_pTexts->SetAt(nIndex, *pszString);
		}
	} 
	catch(CMemoryException e)
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return FALSE;
	}
	return TRUE;
}

// SDM 1.5 Test11.0
/***************************************************************************/
// CDependentSegment::CheckPositionToSegment Check positions for dep. segments
// Checks the positions for validation. If they are ok for a new segment
// (depending on master segment positions), it returns the
// index, where to put them in in the arrays (0 based), otherwise -1. The
// start must not be placed in the range of a segment, where already
// another segment is aligned to, but there must be a segment to align to.
/***************************************************************************/
int CDependentTextSegment::CheckPositionToMaster(CSaDoc* pSaDoc, DWORD dwAlignedStart, DWORD dwStop, EMode nMode) const
{
	int nTextIndex = GetSelection();

	if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(nTextIndex != -1)) // segment selected (edit)
	{
		return -1;
	}
	else // Add
	{
		return CDependentSegment::CheckPositionToMaster(pSaDoc, dwAlignedStart, dwStop, nMode);
	}
}

/***************************************************************************/
// CDependentTextSegment::Add Add dependent annotation segment
/***************************************************************************/
void CDependentTextSegment::Add(CSaDoc* pDoc, DWORD dwStart, CSaString& szString, BOOL, BOOL bCheck)
{
	// get pointer to view
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);

	// get the offset and duration from master
	int nSegment = pDoc->GetSegment(GLOSS)->FindOffset(dwStart);
	if (nSegment == -1)
	{
		return; // return on error
	}

	DWORD dwDuration = pDoc->GetSegment(GLOSS)->GetDuration(nSegment);

	int nPos = CheckPosition(pDoc, dwStart, dwStart + dwDuration, MODE_ADD); // get the insert position
	if (nPos == -1)
	{
		return; // return on error
	}

	// save state for undo ability
	if (bCheck)
	{
		pDoc->CheckPoint();
	}

	// insert or append the new dependent segment
	if (!Insert( nPos, &szString, 0, dwStart, dwDuration))
	{
		return; // return on error
	}

	pDoc->SetModifiedFlag(TRUE); // document has been modified
	pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
	pView->ChangeAnnotationSelection(this, nPos, dwStart, dwStart + dwDuration); // change the selection
	pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}

//###########################################################################
// CIndependentSegment
// class to do all the handling for the master annotation segments.

/////////////////////////////////////////////////////////////////////////////
// CIndependentSegment helper functions


/***************************************************************************/
// CIndependentSegment::LimitsPosition Limits positions for annotation window
// Adjusts the position to acceptable position to adjust boundaries
/***************************************************************************/
void CIndependentSegment::LimitPosition(CSaDoc* pSaDoc, DWORD& dwStart,DWORD& dwStop, int nMode) const
{
	if ((m_nSelection != -1)&&!(nMode&LIMIT_NO_OVERLAP)) 
	{ 
		// segment selected (edit)
		if (m_nSelection > 0) 
		{
			DWORD dwStartMinus = pSaDoc->SnapCursor(START_CURSOR, dwStart,0,dwStart,SNAP_LEFT);
			DWORD dwStopMinus = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStart,dwStop,SNAP_LEFT);

			// check if not more than 40% overlap of start with previous character
			if (dwStartMinus < (GetOffset(m_nSelection - 1) + GetDuration(m_nSelection - 1) *3/5)) 
			{
				dwStart = GetOffset(m_nSelection - 1) + GetDuration(m_nSelection - 1) *3/5;
				dwStart = pSaDoc->SnapCursor(START_CURSOR, dwStart,dwStart,dwStop-2,SNAP_RIGHT);
				dwStartMinus = dwStart; // snap will not move
			}
			// check if more than 40% overlap of previous stop with current character
			if ((dwStartMinus*3 + dwStopMinus*2)/5 <= GetStop(m_nSelection - 1)) 
			{
				if (nMode==LIMIT_MOVING_STOP) 
				{
					dwStop = (GetStop(m_nSelection - 1)*5 - dwStart*3)/2 + 2;
					dwStop = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStop,pSaDoc->GetUnprocessedDataSize(),SNAP_RIGHT);
				} 
				else 
				{
					dwStart = (GetStop(m_nSelection - 1)*5 - dwStop*2)/3 + 2;
					dwStart = pSaDoc->SnapCursor(START_CURSOR, dwStart,dwStart,dwStop-2,SNAP_RIGHT);
				}
			}
		}
		// find offset of next character
		int nNext = GetNext(m_nSelection);
		if (nNext != -1) 
		{
			DWORD dwStartPlus = pSaDoc->SnapCursor(START_CURSOR, dwStart,dwStart,pSaDoc->GetUnprocessedDataSize(),SNAP_RIGHT);
			DWORD dwStopPlus = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStop,pSaDoc->GetUnprocessedDataSize(),SNAP_RIGHT);

			// check if not more than 40% overlap with stop
			if (dwStopPlus > (GetOffset(nNext) + GetDuration(nNext) *2/5)) 
			{
				dwStop = GetOffset(nNext) + GetDuration(nNext) *2/5;
				dwStop = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStart,dwStop,SNAP_LEFT);
				dwStopPlus = dwStop; // snap will not move
			}
			// check if next character overlaps more than 40% of current character
			if ((dwStartPlus*2 + dwStopPlus*3)/5 >= GetOffset(nNext)) 
			{
				if (nMode==LIMIT_MOVING_START) 
				{
					dwStart = (GetOffset(nNext)*5 - dwStop*3)/2 - 2;
					dwStart = pSaDoc->SnapCursor(START_CURSOR, dwStart,0,dwStart,SNAP_LEFT);
				} 
				else 
				{
					dwStop = (GetOffset(nNext)*5 - dwStart*2)/3 - 2;
					dwStop = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStart+2,dwStop,SNAP_LEFT);
				}
			}
		}
	} 
	else if ((m_nSelection != -1)&&(nMode&LIMIT_NO_OVERLAP)) 
	{ 
		// SDM 1.5Test8.1
		DWORD dwStartMin=0;
		DWORD dwStopMax= pSaDoc->GetUnprocessedDataSize();

		if (m_nSelection > 0) 
		{
			int nPrevious = GetPrevious(m_nSelection);
			dwStartMin = GetOffset(nPrevious) + pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
			if (nPrevious > 0) 
			{
				if (GetOffset(nPrevious) < GetStop(nPrevious-1)) 
				{ 
					// overlap
					DWORD dwOverlap = GetStop(nPrevious-1) - GetOffset(nPrevious);
					if (dwStartMin < (GetOffset(nPrevious) + dwOverlap*5/2 + 2)) 
					{
						dwStartMin = (GetOffset(nPrevious) + dwOverlap*5/2 + 2);
					}
				}
			}
			int nBlockAlign = pSaDoc->GetFmtParm()->wBlockAlign;
			if ((nBlockAlign == 2)&&(dwStartMin & 1)) dwStartMin++;
		}
		// find offset of next character
		int nNext = GetNext(m_nSelection);
		if (nNext != -1) 
		{
			dwStopMax = GetStop(nNext);
			if (dwStopMax > pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME)) 
			{
				dwStopMax -= pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
			} 
			else 
			{
				dwStopMax = 0;
			}

			if (GetNext(nNext) > 0) 
			{
				if (GetStop(nNext) > GetOffset(GetNext(nNext))) 
				{ 
					// overlap
					DWORD dwOverlap = GetStop(nNext) - GetOffset(GetNext(nNext));
					DWORD dwStopNext = GetStop(nNext);
					if (dwStopNext > (dwOverlap*5/2 + 2)) 
					{
						dwStopNext -= (dwOverlap*5/2 + 2);
					} 
					else 
					{ 
						dwStopNext = 0;
					}
					if (dwStopMax > dwStopNext) 
					{
						dwStopMax = dwStopNext;
					}
				}
			}
			int nBlockAlign = pSaDoc->GetFmtParm()->wBlockAlign;
			if ((nBlockAlign == 2)&&(dwStopMax & 1)) dwStopMax --;
		}
		dwStartMin = pSaDoc->SnapCursor(START_CURSOR, dwStartMin,dwStartMin,dwStopMax,SNAP_RIGHT);
		dwStopMax = pSaDoc->SnapCursor(STOP_CURSOR, dwStopMax,dwStartMin,dwStopMax,SNAP_LEFT);
		if ((dwStartMin + pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME))> dwStopMax)
			return; // no hope ERROR

		if (dwStart < dwStartMin) dwStart = dwStartMin;
		if (dwStop > dwStopMax) dwStop = dwStopMax;
		if (dwStop < (dwStart + pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME))) 
		{
			int nBlockAlign = pSaDoc->GetFmtParm()->wBlockAlign;
			if (nMode & LIMIT_MOVING_STOP) 
			{
				dwStop = dwStart + pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
				if ((nBlockAlign == 2)&&(dwStop & 1)) dwStop++;
				dwStop = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStop,dwStopMax+2,SNAP_RIGHT);
				if (dwStop > dwStopMax) 
				{
					dwStop = dwStopMax;
					dwStart = dwStop - pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
					if ((nBlockAlign == 2)&&(dwStart & 1)) dwStart--;
					dwStart = pSaDoc->SnapCursor(START_CURSOR, dwStart,dwStartMin,dwStart,SNAP_LEFT);
				}
			} 
			else 
			{
				dwStart = dwStop - pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
				if ((nBlockAlign == 2)&&(dwStart & 1)) dwStart--;
				dwStart = pSaDoc->SnapCursor(START_CURSOR, dwStart,0,dwStart,SNAP_LEFT);
				if (dwStart < dwStartMin) 
				{
					dwStart = dwStartMin;
					dwStop = dwStart + pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
					if ((nBlockAlign == 2)&&(dwStop & 1)) 
						dwStop++;
					dwStop = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStop,dwStopMax,SNAP_RIGHT);
				}
			}
		}
	}
}

// SDM Split function 1.06.1.2
// SDM 1.06.6U2 require not more than 40% overlap, require MIN_EDIT_SEGMENT_TIME on edit
/***************************************************************************/
// CIndependentSegment::CheckPosition Check positions for annotation window
// Checks the positions for validation. If they are ok for a new annotation
// segment it returns the index, where to put it in the annotation array
// (0 based), otherwise -1. The function allows an overlap of 50% of
// existing annotation segments at both ends.
/***************************************************************************/
int CIndependentSegment::CheckPosition(CSaDoc* pDoc, DWORD dwStart,DWORD dwStop, EMode nMode, BOOL bOverlap) const
{
	if (IsEmpty()) {
		return 0; // no character yet, ok
	}

	if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(m_nSelection != -1)) // segment selected (edit)
	{
		if (bOverlap) //SDM 1.5Test8.1
		{
			if (m_nSelection > 0)
			{
				// check if not more than 40% overlap of start with previous character
				if (dwStart < (GetOffset(m_nSelection - 1) + GetDuration(m_nSelection - 1) *3/5)) return -1; // error
				// check if more than 40% overlap of previous stop with current character
				if ((dwStart+(dwStop-dwStart)*2/5) <= GetStop(m_nSelection - 1)) return -1; // error
			}
			// find offset of next character
			int nNext = GetNext(m_nSelection);
			if (nNext != -1)
			{
				// check if not more than 40% overlap with stop
				if (dwStop > (GetOffset(nNext) + GetDuration(nNext) *2/5)) return -1; // error
				// check if next character overlaps more than 40% of current character
				if ((dwStart+(dwStop-dwStart)*3/5) >= GetOffset(nNext)) return -1; // error
			}
			if (dwStop < dwStart) return -1;
			if (pDoc->GetTimeFromBytes(dwStop-dwStart) < MIN_EDIT_SEGMENT_TIME) return -1;
			return m_nSelection;
		}
		else // SDM 1.5Test8.1
		{
			int nPrevious = GetPrevious(m_nSelection);
			if (nPrevious > 0)
			{
				if (GetOffset(nPrevious) < GetStop(nPrevious-1)) // overlap
				{
					DWORD dwOverlap = GetStop(nPrevious-1) - GetOffset(nPrevious);
					if (dwStart < (GetOffset(nPrevious) + dwOverlap*5/2 + 2)) return -1;
				}
			}
			// find offset of next character
			int nNext = GetNext(m_nSelection);
			if (GetNext(nNext) != -1)
			{
				if (GetStop(nNext) > GetOffset(GetNext(nNext))) // overlap
				{
					DWORD dwOverlap = GetStop(nNext) - GetOffset(GetNext(nNext));
					DWORD dwStopNext = GetStop(nNext);
					if (dwStopNext > (dwOverlap*5/2 + 2)) dwStopNext -= (dwOverlap*5/2 + 2);
					else dwStopNext = 0;
					if (dwStop > dwStopNext) return -1;
				}
			}
			if (dwStop < dwStart) return -1;
			if (pDoc->GetTimeFromBytes(dwStop-dwStart) < MIN_EDIT_SEGMENT_TIME) return -1;
			return m_nSelection;
		}
	}
	else if ((nMode==MODE_ADD)||(nMode==MODE_AUTOMATIC))
	{
		//SDM 1.06.5 add will make room by shifting adjacent segments
		// check the positions
		if ((dwStop - dwStart)  < pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) return -1; // segment too small
		int nLength = GetOffsetSize();
		int nLoop;
		for (nLoop = 0; nLoop < nLength; nLoop++)
		{
			DWORD dwOffset = GetOffset(nLoop);
			if (dwStart <= dwOffset) // this offset
			{
				// Check if insertion will leave at least minimum segment length
				if ((dwStop+pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) > (dwOffset + GetDuration(nLoop))) return -1;
				if ((nLoop > 0)&&
					((dwStart-pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < GetOffset(nLoop - 1))) return -1;
				int nNext = GetNext(nLoop);
				if (nNext != -1)
				{
					// check if next has more than 40% overlap
					if ((dwStop+(dwOffset+GetDuration(nLoop)-dwStop)*3/5)  > GetOffset(nNext) ) return -1; // error
				}
				int nPrevious = GetPrevious(nLoop);
				if (nPrevious > 0)
				{
					// check if segment before previous will overlap 40%
					if ((GetOffset(nPrevious) + (dwStart-GetOffset(nPrevious))*2/5 ) < GetStop(nPrevious - 1)) return -1; // error
				}
				return nLoop; // ok
			}
		}
		if ((nLoop >0)&&((dwStart - GetOffset(nLoop-1)) < pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME))) return -1;
		int nPrevious = GetPrevious(nLoop);
		if (nPrevious > 0)
		{
			// check if segment before previous will overlap 40%
			if ((dwStart + GetOffset(nPrevious))/2 < GetStop(nPrevious - 1)) return -1; // error
		}
		return nLoop; // append at the end
	}
	return -1;
}

//###########################################################################
// CPhoneticSegment
// class to do all the handling for the phonetic annotation segments.

/////////////////////////////////////////////////////////////////////////////
// CPhoneticSegment helper functions

/***************************************************************************/
// CPhoneticSegment::Exit Exit segmenting on error
// Standard exit function if an error occured. 
/***************************************************************************/
long CPhoneticSegment::Exit(int nError)
{
	EndProcess(); // end data processing
	EndWaitCursor();
	SetDataReady(FALSE);
	return MAKELONG(nError, 100);
}

/***************************************************************************/
// CPhoneticSegment::ReplaceSelectedSegment
// Replaces the selected segment with the string passes in.
/***************************************************************************/
void CPhoneticSegment::ReplaceSelectedSegment(CDocument* pSaDoc, const CSaString & str)
{
	// get pointer to view
	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);

	// get the new cursor positions
	AdjustCursorsToSnap(pDoc); // snap cursors to appropriate position
	DWORD dwStart = pView->GetStartCursorPosition();
	DWORD dwStop = pView->GetStopCursorPosition();


	DWORD dwOldOffset = RemoveNoRefresh(pDoc);
	if (dwOldOffset == -1)
	{
		return;
	}

	pDoc->SetModifiedFlag(TRUE); // document has been modified
	pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified

	// insert or append the new segment
	if (!Insert(m_nSelection, &str, 0, dwStart, dwStop - dwStart))
	{
		return; // return on error
	}

	int nSaveSelection = m_nSelection;
	pView->ChangeAnnotationSelection(this, m_nSelection, dwStart, dwStop); // deselect
	pView->ChangeAnnotationSelection(this, nSaveSelection, dwStart, dwStop); // select again
	pView->RefreshGraphs(FALSE); // refresh the graphs between cursors

	// adjust the aligned annotation offsets and durations
	for (int nLoop = 1; nLoop <= GLOSS; nLoop++) //SDM 1.5Test8.1 Segments after gloss dependent on gloss
	{
		CSegment* pSegment = pDoc->GetSegment(nLoop);
		if (pSegment)
		{
			int nIndex = pSegment->FindOffset(dwOldOffset);
			if (nIndex != -1) pSegment->Adjust(pDoc, nIndex, dwStart, dwStop - dwStart);
		}
	}
}

/***************************************************************************/
// CPhoneticSegment::Remove Remove annotation segment
// All the dependent segments that are aligned to the removed master
// segment have to be removed too. The user will be informed before.
/***************************************************************************/
void CPhoneticSegment::Remove( CDocument* pDoc, BOOL bCheck)
{
	// get pointer to view
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos);
	CSaDoc* pSaDoc = (CSaDoc*)pDoc;

	// save state for undo ability
	if (bCheck) pSaDoc->CheckPoint();
	int nSelection = m_nSelection;
	DWORD dwOldStop = GetStop(nSelection);
	DWORD dwOldOffset = RemoveNoRefresh(pSaDoc);
	if (dwOldOffset == -1)
	{
		return;
	}

	pDoc->SetModifiedFlag(TRUE); // document has been modified
	pSaDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
	pView->ChangeAnnotationSelection(this, m_nSelection, 0, 0); // deselect
	pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
	// delete aligned dependent segments and gloss
	for (int nLoop = 1; nLoop <= GLOSS; nLoop++) //SDM 1.5Test8.1 segments after gloss are dependent on gloss
	{
		CSegment* pSegment = pSaDoc->GetSegment(nLoop);
		if (pSegment)
		{
			int nIndex = pSegment->FindOffset(dwOldOffset);
			if (nIndex != -1)
			{
				if (pSegment->GetStop(nIndex) == dwOldStop)
				{
					pSegment->SetSelection(nIndex);
					pSegment->Remove(pDoc, FALSE); // no checkpoint
				}
				else
				{
					DWORD dwOffset = GetOffset(nSelection);
					pSegment->Adjust( pSaDoc, nIndex, dwOffset, pSegment->GetStop(nIndex) - dwOffset);
				}
			}
			nIndex = pSegment->FindStop(dwOldStop);
			if (nIndex != -1)
			{
				DWORD dwStop = GetStop(GetPrevious(nSelection));
				pSegment->Adjust( pSaDoc, nIndex, pSegment->GetOffset(nIndex), dwStop - pSegment->GetOffset(nIndex));
			}
		}
	}
	pView->RefreshGraphs(TRUE,FALSE);
}


/***************************************************************************/
// CPhoneticSegment::RemoveNoRefresh Remove annotation segment
// All the dependent segments that are aligned to the removed phonetic
// segment have to be removed too. The user will be informed before.
/***************************************************************************/
DWORD CPhoneticSegment::RemoveNoRefresh(CDocument * )
{
	// find length of string to delete
	int nLength = GetSegmentLength(m_nSelection); // find length of string to delete

	// check if there are aligned dependent segments
	DWORD dwOldOffset = GetOffset(m_nSelection);

	// change the phonetic arrays
	*m_pAnnotation = m_pAnnotation->Left(m_nSelection) + m_pAnnotation->Right(m_pAnnotation->GetLength() - nLength - m_nSelection);
	RemoveAt(m_nSelection,nLength);

	return dwOldOffset;
}


/***************************************************************************/
// CPhoneticSegment::Process Segmenting wave data
// The process stores the resulting segmentation in the phonetic data arrays.
// While processing a process bar, placed on the status bar, has to be up-
// dated. The level tells which processing level this process has been
// called, start process start on which processing percentage this process
// starts (for the progress bar). The status bar process bar will be updated
// depending on the level and the progress start. The return value returns
// the highest level througout the calling queue, or -1 in case of an error
// in the lower word of the long value and the end process progress percen-
// tage in the higher word.
/***************************************************************************/
long CPhoneticSegment::Process(void* pCaller, CSaDoc* pDoc, int nProgress, int nLevel)
{
	TRACE(_T("Process: CPhoneticSegment\n"));
	if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
	if (IsDataReady()) return MAKELONG(--nLevel, nProgress); // data is already ready

	// create change and zero crossing data, it is needed to do segmentation
	CProcessChange* pChange = (CProcessChange*)pDoc->GetChange(); // get pointer to change object
	pChange->Process(pCaller, pDoc, nProgress, ++nLevel); // process change data

	// Finish fragmenting as needed.
	CProcessFragments* pFragments = (CProcessFragments*)pDoc->GetFragments();
	long lResult = LOWORD(pFragments->Process(this, pDoc)); // process data

	// ******** added by CLW 6/98 - 9/24/98 ******************************************
	CProcessZCross* pZCross = (CProcessZCross*)pDoc->GetZCross(); // get pointer to zero crossing object
	lResult = pZCross->Process(pCaller, pDoc, nProgress, nLevel); // process zero crossing data
	// *******************************************************************************

	nLevel = (short int)LOWORD(lResult);
	nProgress = HIWORD(lResult);

	if (nLevel < 0) // previous processing error
	{
		if ((nLevel == PROCESS_CANCELED))
		{
			CancelProcess(); // set your own cancel flag
		}
		return MAKELONG(nLevel, nProgress);
	}

	// start segmenting
	BeginWaitCursor(); // wait cursor
	if (!StartProcess(pCaller, IDS_STATTXT_SEGMENTING, FALSE)) // memory allocation failed
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}

	DWORD dwLoopEnd = pChange->GetDataSize();; // end of loop
	// prepare parameters
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	SegmentParm* pSegParm = pMainFrame->GetSegmentParm(); // get segment parameters // RLJ 1.5Test11.1A
	// float fFactor = (float)pDoc->GetUnprocessedDataSize() / (float)dwLoopEnd; // size factor
	// fFactor based on frame-length for accuracy (CLW 1/20/98)
	FmtParm* pFmtParm = pDoc->GetFmtParm();
	float fFactor = (float) CALCULATION_INTERVAL(pFmtParm->dwSamplesPerSec) * pFmtParm->wBlockAlign; // size factor
	// Flank Width needs to be rounded up to guarantee minimum width (CLW 1/19/98)
	DWORD dwFlankWidth = (DWORD)((pDoc->GetBytesFromTime(pSegParm->fSegmentWidth) + fFactor - 1) / 2 / fFactor); // flank width in process words
	// DWORD dwFlankWidth = (DWORD)(pDoc->GetBytesFromTime(pSegParm->fSegmentWidth) / 2 / fFactor); // CLW 10/12/98
	if (!dwFlankWidth) dwFlankWidth = 1;
	int nChThreshold = (int)((long)pChange->GetMaxValue() * (long)pSegParm->nChThreshold / 100); // change threshold
	int nZCThreshold = pSegParm->nZCThreshold; // zero crossing threshold (CLW)
	DWORD dwLoopPos = 0;
	BOOL bRes = TRUE;
	DWORD dwFlankUp = 0;
	DWORD dwFlankDown = 0;
	DWORD dwSegmentStart = 0;
	DWORD dwSegmentStop = 0;
	DWORD dwHillTop = 0;
	// prepare segment data
	int nSegmentIndex = 0;
	//    if (!pSegParm->bKeepSegments) // not yet implemented
	if (TRUE)
	{
		// SDM 1.06.4 move question to Command Level
		// remove all annotations and deselect everything
		for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
		{
			if (pDoc->GetSegment(nLoop))
			{
				pDoc->GetSegment(nLoop)->DeleteContents();
				if (pDoc->GetSegment(nLoop)->GetSelection() != -1) pDoc->GetSegment(nLoop)->SetSelection(-1);
			}
		}
		pDoc->SetModifiedFlag(TRUE); // document has been modified
		pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
		// invalidate parsing information
		pDoc->GetSegment(GLOSS)->SetDataInvalid();
	}

	/*********************************************************************/
	// Added by CLW 6/98 - 9/25/98
	// Modified segmentation algorithm uses both Change and Zero Crossing.
	// Zero Crossing has precedence, as follows:
	//  - search for next Zero Crossing peak
	//  - segment based on Change from last Zero Crossing peak or start of
	//    file, to next Zero Crossing peak
	//  - store Zero Crossing peak (segments)
	//  - repeat until end of data
	// This yields better segmentation for fricatives.
	/*********************************************************************/
	DWORD dwZCStart = 0; // start of zero crossing peak
	DWORD dwZCStop = 0; // end of zero crossing peak
	DWORD dwShortZCStart = 0; // start of previous peak if it was too short,
	// otherwise 0
	DWORD dwShortZCStop = 0; // end of previous peak if short, otherwise 0
	DWORD dwOldZCStart = 0; // start of last valid peak
	DWORD dwOldZCStop = 0; // end of last valid peak
	DWORD dwLastSample = pDoc->GetUnprocessedDataSize() - pFmtParm->wBlockAlign;
	DWORD dwZBase; // candidate for zero crossing peak base
	/*********************************************************************/
	DWORD dwChLoopEnd;

	// start processing
	while (dwLoopPos < dwLoopEnd)
	{
		/*********************************************************************/
		// Added by CLW 6/98 - 9/25/98
		/*********************************************************************/
		// find zero crossing peak
		while (!dwZCStop)
		{
			dwZBase = dwLoopPos + 1;
			// read t-2 zero crossing data point
			int nOldZCData_2 =  pZCross->GetProcessedData(dwLoopPos++, &bRes);
			if (!bRes) return Exit(PROCESS_ERROR); // error, reading zero crossing data failed
			// read t-1 zero crossing data point
			int nOldZCData_1 =  pZCross->GetProcessedData(dwLoopPos++, &bRes);
			if (!bRes) return Exit(PROCESS_ERROR); // error, reading failed
			while ((!dwZCStop) && (dwLoopPos < dwLoopEnd))
			{
				// read current data point
				int nZCData =  pZCross->GetProcessedData(dwLoopPos, &bRes);
				if (!bRes) return Exit(PROCESS_ERROR); // error, reading failed
				// check for start of peak
				if (!dwZCStart)
				{
					// check for base candidate
					if ((nOldZCData_2 >= nOldZCData_1) && (nZCData > nOldZCData_1))
						dwZBase = dwLoopPos;
					// over threshold?
					if (nZCData >= nZCThreshold)
						dwZCStart = dwZBase;
				}
				else // check for end of peak
				{
					if (nOldZCData_1 < nZCThreshold)
					{
						if ((nOldZCData_2 > nOldZCData_1) && (nOldZCData_1 <= nZCData))
							dwZCStop = dwLoopPos - 1;
					}
				}
				// shift 2 most recent points into history
				nOldZCData_2 = nOldZCData_1;
				nOldZCData_1 = nZCData;
				dwLoopPos++;
			}
			// no ZCEnd, dwLoopPos = end of data (ZC peak was cut off at end of data)
			if ((!dwZCStop) && (dwLoopPos == dwLoopEnd))
				dwZCStart = dwZCStop = dwLoopEnd; // default to end of file

			// The next several lines check to see if the current peak or the
			// previous peak is too short. If so, SA attempts to join the two peaks
			// if they are close enough. If the current peak is too short, the search
			// continues, whether it was joined or not, since there will be no room
			// between two joined peaks for Change to find any segments.

			// is the previous peak short?
			if (dwShortZCStart)
			{
				// is the previous peak adjacent?
				if (dwZCStart - dwOldZCStop < 4 * dwFlankWidth)
				{
					// join the two peaks
					dwZCStart = dwOldZCStart;
				}
				dwShortZCStart = dwShortZCStop = 0;  // reset short segment start and end
			}

			// is the current peak (still) short?
			if ((dwZCStop - dwZCStart < 2 * dwFlankWidth) && (dwZCStop < dwLoopEnd))
			{
				// save this segment in case the next ZC segment is adjacent
				dwShortZCStart = dwZCStart;
				dwShortZCStop = dwZCStop;
				// adjacent to old segment?
				if (dwZCStart - dwOldZCStop < 4 * dwFlankWidth)
				{
					// join the two peaks (adjust most recently stored segment)
					dwZCStart = dwOldZCStart; // rewind start of current segment to start of old
					dwOldZCStop = dwZCStop; // fast forward end of old segment to end of current
					dwSegmentStart = (DWORD)(((float)dwZCStart + 0.5F) * fFactor); // CLW 1/20/98
					dwSegmentStop = (DWORD)(((float)dwZCStop + 0.5F) * fFactor); // CLW 1/20/98
					Adjust(pDoc, nSegmentIndex - 1, dwSegmentStart, dwSegmentStop - dwSegmentStart); // store it
					dwSegmentStart = dwSegmentStop;
					dwSegmentStop = 0;
					dwShortZCStart = dwShortZCStop = 0; // reset short segment start and end

				}
				// restart search
				dwLoopPos = dwZCStop - 1;
				dwZCStart = dwZCStop = 0;
			}
		}
		/*********************************************************************/

		// process change peaks
		/*********************************************************************/
		// Added by CLW 6/98 - 9/25/98
		/*********************************************************************/
		// Process change data between zero crossing peaks,
		// with a pad of dwFlankWidth at either end.
		dwLoopPos = dwOldZCStop + dwFlankWidth;
		dwChLoopEnd = dwZCStart - dwFlankWidth;
		/*********************************************************************/
		int nOldChData = pChange->GetProcessedData(dwLoopPos++, &bRes); // read change data point
		if (!bRes) return Exit(PROCESS_ERROR); // error, reading change data failed
		while ((dwLoopPos < dwChLoopEnd) && (dwZCStart - dwOldZCStop > 4 * dwFlankWidth))
		{
			int nChangeData = pChange->GetProcessedData(dwLoopPos, &bRes); // read change data point
			if (!bRes) return Exit(PROCESS_ERROR); // error, reading change data failed
			if (nChangeData > nChThreshold) // point is over threshold
			{
				if (nOldChData < nChangeData) // rising flank
				{
					if (dwFlankDown)
					{
						// there was a falling flank before
						dwFlankUp = dwFlankDown = 0; // new hill
						dwHillTop = 0;
					}
					dwFlankUp++;
				}
				else
				{
					if (nOldChData > nChangeData) // falling flank
					{
						if (!dwHillTop) dwHillTop = dwLoopPos - 1;
						dwFlankDown++;
						if ((dwFlankUp >= dwFlankWidth) && (dwFlankDown >= dwFlankWidth))
						{
							// segment start or stop found
							if ((!dwSegmentStart) && (nSegmentIndex > 0)) // CLW mod 1/19/98
								dwSegmentStart = (DWORD)((float)dwHillTop * fFactor); // CLW 1/20/98
							else // start already found before
							{
								if (!dwSegmentStop) dwSegmentStop = (DWORD)((float)dwHillTop * fFactor); // CLW 1/20/98
								// ready to store segment
								CSaString szSegment = SEGMENT_DEFAULT_CHAR;
								if (dwSegmentStart >= dwSegmentStop || dwSegmentStop > dwLastSample)
									break;
								bRes = Insert(nSegmentIndex++, &szSegment, 0, dwSegmentStart, dwSegmentStop - dwSegmentStart);
								if (!bRes) return Exit(PROCESS_ERROR); // error, writing segment failed
								dwSegmentStart = dwSegmentStop;
								dwSegmentStop = 0;
								pDoc->SetModifiedFlag(TRUE); // document has been modified
								pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
							}
							dwFlankUp = dwFlankDown = dwHillTop = 0; // look for new hill
						}
					}
				}
			}
			else dwFlankUp = 0;
			nOldChData = nChangeData;
			dwLoopPos++;
		}
		dwFlankUp = dwFlankDown = dwHillTop = 0; // look for new hill

		/*********************************************************************/
		// Added by CLW 6/98 - 9/25/98
		/*********************************************************************/
		// Store Zero Crossing segment
		if (dwZCStart == dwLoopEnd) break;
		{
			// Figure out start and stop
			if ((!dwSegmentStart) && (nSegmentIndex)) // CLW mod 1/19/98
				dwSegmentStart = (DWORD)(((float)dwZCStart + 0.5F) * fFactor); // CLW 1/20/98
			else // start already found before
			{
				dwSegmentStop = (DWORD)(((float)dwZCStart + 0.5F) * fFactor); // CLW 1/20/98
				if (dwSegmentStop - dwSegmentStart > 2 * dwFlankWidth)
				{
					// store previous segment
					CSaString szSegment = SEGMENT_DEFAULT_CHAR;
					if (dwSegmentStart >= dwSegmentStop || dwSegmentStop > dwLastSample)
						break;
					bRes = Insert(nSegmentIndex++, &szSegment, 0, dwSegmentStart, dwSegmentStop - dwSegmentStart);
					if (!bRes) return Exit(PROCESS_ERROR); // error, writing segment failed
					dwSegmentStart = dwSegmentStop;
				}
				dwSegmentStop = 0;
			}
			if (!dwSegmentStop) dwSegmentStop = (DWORD)((float)dwZCStop * fFactor); // CLW 1/20/98
			{
				// Store new zero crossing segment
				CSaString szSegment = SEGMENT_DEFAULT_CHAR;
				if (dwSegmentStart >= dwSegmentStop || dwSegmentStop > dwLastSample)
					break;
				bRes = Insert(nSegmentIndex++, &szSegment, 0, dwSegmentStart, dwSegmentStop - dwSegmentStart);
				if (!bRes) return Exit(PROCESS_ERROR); // error, writing segment failed
				dwSegmentStart = dwSegmentStop;
				dwSegmentStop = 0;
				pDoc->SetModifiedFlag(TRUE); // document has been modified
				pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
			}
		}
		dwLoopPos = dwZCStop - 1;
		dwOldZCStart = dwZCStart;
		dwOldZCStop = dwZCStop;
		dwZCStop = dwZCStart = 0;
		/*********************************************************************/

		// set progress bar
		SetProgress(nProgress + (int)(100 * dwLoopPos / dwLoopEnd / (DWORD)nLevel));
		if (IsCanceled()) return Exit(PROCESS_CANCELED); // process canceled
	}
	// Add segment for silence at end
	DWORD dwLastStop = GetStop(nSegmentIndex - 1);
	if (dwLastStop < dwLastSample)
	{
		// store silence segment
		dwSegmentStart = dwLastStop;
		dwSegmentStop = dwLastSample;
		CSaString szSegment = SEGMENT_DEFAULT_CHAR;
		bRes = Insert(nSegmentIndex++, &szSegment, 0, dwSegmentStart, dwSegmentStop - dwSegmentStart);
		if (!bRes) return Exit(PROCESS_ERROR); // error, writing segment failed
	}

	pDoc->AutoSnapUpdate();

	// calculate the actual progress
	nProgress = nProgress + (int)(100 / nLevel);
	EndProcess((nProgress >= 95)); // end data processing
	EndWaitCursor();
	SetDataReady();
	return MAKELONG(nLevel, nProgress);
}


/***************************************************************************/
// CMusicPhraseSegment::CheckPosition Check positions for annotation window
// Checks the positions for validation. If they are ok for a new annotation
// segment it returns the index, where to put it in the annotation array
// (0 based), otherwise -1. The function allows an overlap of 50% of
// existing annotation segments at both ends.
/***************************************************************************/
int CMusicPhraseSegment::CheckPosition(CSaDoc* pDoc, DWORD dwStart,DWORD dwStop, EMode nMode, BOOL bOverlap) const
{
	int nLength = GetOffsetSize();
	if (nLength == 0) return 0; // no character yet, ok

	if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(m_nSelection != -1)) // segment selected (edit)
	{
		return CIndependentSegment::CheckPosition(pDoc, dwStart, dwStop, nMode, bOverlap);
	}
	else if ((nMode==MODE_ADD)||(nMode==MODE_AUTOMATIC))
	{
		if ((dwStop - dwStart)  < pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) return -1; // segment too small
		int nLoop;
		for (nLoop = 0; nLoop < nLength; nLoop++)
		{
			DWORD dwOffset = GetOffset(nLoop);
			if (dwStart <= dwOffset) { // this offset
				if (dwStop > dwOffset) {
					return -1;
				}
				if ((nLoop > 0)&&(dwStart < GetStop(nLoop - 1))) {
					return -1;
				}
				return nLoop; // ok
			}
		}
		if ((nLoop >0)&&(dwStart < GetStop(nLoop-1))) {
			return -1;
		}
		return nLoop; // append at the end
	} else {
		return -1;
	}
}

//###########################################################################
// CGlossSegment
// class to do all the handling for the gloss annotation segments.

/////////////////////////////////////////////////////////////////////////////
// CGlossSegment construction/destruction/creation

/***************************************************************************/
// CGlossSegment::CGlossSegment Constructor
/***************************************************************************/
CGlossSegment::CGlossSegment(int index, int master) : CTextSegment(index,master)
{
	m_pPOS = new CStringArray();
}

/***************************************************************************/
// CGlossSegment::~CGlossSegment Destructor
/***************************************************************************/
CGlossSegment::~CGlossSegment()
{
	if (m_pPOS!=NULL) {
		delete m_pPOS;
		m_pPOS = NULL;
	}
}

/***************************************************************************/
// CGlossSegment::ReplaceSelectedSegment
// Replaces the selected segment with the string passes in.
/***************************************************************************/
void CGlossSegment::ReplaceSelectedSegment(CDocument* pSaDoc, const CSaString & str) {

	CSaString POS;
	if (m_nSelection != -1) {
		POS = m_pPOS->GetAt(m_nSelection); //SDM 1.5Test8.2
	}

	CTextSegment::ReplaceSelectedSegment(pSaDoc, str);
	m_pPOS->SetAt(m_nSelection, POS); // SDM 1.5Test8.2

}

//SDM 1.5Test8.1
/***************************************************************************/
// CGlossSegment::Remove Remove annotation segment
// All the dependent segments that are aligned to the removed gloss
// segment have to be removed too. The user will be informed before.
/***************************************************************************/
void CGlossSegment::Remove(CDocument* pSaDoc, BOOL bCheck)
{
	CSaDoc* pDoc = (CSaDoc*) pSaDoc;

	DWORD dwOldOffset = DWORD(~0);
	if (m_nSelection != -1) 
	{
		dwOldOffset = GetOffset(m_nSelection);
	}
	CTextSegment::Remove(pSaDoc, bCheck);

	if (dwOldOffset == ~0) 
	{ 
		return;
	}
	for (int nLoop = GLOSS + 1; nLoop < ANNOT_WND_NUMBER; nLoop++)
	{
		CSegment* pSegment = pDoc->GetSegment(nLoop);
		if (pSegment)
		{
			int nIndex = pSegment->FindOffset(dwOldOffset);
			if (nIndex != -1)
			{
				pSegment->SetSelection(nIndex);
				pSegment->Remove(pDoc, FALSE); // no checkpoint
			}
		}
	}
}


//SDM 1.5Test8.1
/***************************************************************************/
// CGlossSegment::RemoveNoRefresh Remove gloss segment
/***************************************************************************/
DWORD CGlossSegment::RemoveNoRefresh(CDocument* pDoc)
{
	// change the segment arrays
	m_pPOS->RemoveAt(m_nSelection, 1);
	return CTextSegment::RemoveNoRefresh(pDoc);
}

//SDM 1.5Test8.1
/***************************************************************************/
// CGlossSegment::DeleteContents Delete all contents of the gloss arrays
/***************************************************************************/
void CGlossSegment::DeleteContents()
{
	m_pPOS->RemoveAll();
	CTextSegment::DeleteContents(); // call the base class to delete positions
}

//SDM 1.5Test8.1
/***************************************************************************/
// CGlossSegment::Insert Insert/append a gloss segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CGlossSegment::SetAt( const CSaString * pszString, bool delimiter, DWORD dwStart, DWORD dwDuration)
{
	int nIndex = FindOffset( dwStart);
	ASSERT(nIndex>=0);
	try 
	{
		m_pPOS->SetAtGrow( nIndex, NULL);
	} 
	catch(CMemoryException e) 
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return FALSE;
	}
	return CTextSegment::SetAt( pszString, delimiter, dwStart, dwDuration);
}

//SDM 1.5Test8.1
/***************************************************************************/
// CGlossSegment::Insert Insert/append a gloss segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CGlossSegment::Insert(int nIndex, const CSaString * pszString, bool delimiter, DWORD dwStart, DWORD dwDuration)
{
	try 
	{
		m_pPOS->InsertAt( nIndex, NULL,  1);
	} 
	catch(CMemoryException e) 
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return FALSE;
	}
	return CTextSegment::Insert(nIndex, pszString, delimiter, dwStart, dwDuration);
}

//SDM 1.5Test8.1
void CGlossSegment::Serialize(CArchive& ar)
{
	CSegment::Serialize(ar);
	CTextSegment::Serialize(ar);
	if (ar.IsStoring())
	{
		ar << CSaString("CGlossSegmentDetail tag");
	}
	else
	{
		CSaString detailTagCheck;
		ar >> detailTagCheck;
		SA_ASSERT(detailTagCheck == "CGlossSegmentDetail tag");
	}
	m_pPOS->Serialize(ar);
}


/***************************************************************************/
// CGlossSegment::Exit Exit parsing on error
// Standard exit function if an error occured. 
/***************************************************************************/
long CGlossSegment::Exit(int nError)
{
	EndProcess(); // end data processing
	EndWaitCursor();
	SetDataReady(FALSE);
	return MAKELONG(nError, 100);
}

// SDM 1.5 Test11.3
/***************************************************************************/
// CGlossSegment::Process parsing wave data
// The process stores the resulting parse in the gloss data arrays. While
// processing a process bar, placed on the status bar, has to be updated.
// The level tells which processing level this process has been called,
// start process start on which processing percentage this process starts
// (for the progress bar). The status bar process bar will be updated
// depending on the level and the progress start. The return value returns
// the highest level througout the calling queue, or -1 in case of an error
// in the lower word of the long value and the end process progress percen-
// tage in the higher word.
/***************************************************************************/
long CGlossSegment::Process( void* pCaller, CSaDoc* pSaDoc, int nProgress, int nLevel) {
	TRACE(_T("Process: CGlossSegment\n"));
	if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
	if (IsDataReady()) return MAKELONG(--nLevel, nProgress); // data is already ready

	CSaDoc* pDoc = (CSaDoc*)pSaDoc; // cast pointer

	// Finish fragmenting as needed.
	CProcessFragments* pFragments = (CProcessFragments*)pDoc->GetFragments();
	long lResult = LOWORD(pFragments->Process(this, pDoc)); // process data

	// create zero crossing data, it is needed to do parsing
	CProcessLoudness* pLoudness = (CProcessLoudness*)pDoc->GetLoudness(); // get pointer to zero crossing object
	lResult = pLoudness->Process(pCaller, pDoc, nProgress, ++nLevel); // process data
	nLevel = (short int)LOWORD(lResult);
	nProgress = HIWORD(lResult);

	if (nLevel < 0) { // previous processing error
		if ((nLevel == PROCESS_CANCELED)) {
			CancelProcess(); // set your own cancel flag
		}
		return MAKELONG(nLevel, nProgress);
	}

	// start parsing
	BeginWaitCursor(); // wait cursor
	if (!StartProcess(pCaller, IDS_STATTXT_PARSING, FALSE)) { // memory allocation failed or previous processing error
		EndProcess(); // end data processing
		EndWaitCursor();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}
	DWORD dwLoopEnd = pLoudness->GetDataSize();; // end of loop
	// prepare parameters
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	ParseParm* pParseParm = pMainFrame->GetParseParm(); // get parsing parameters
	float fFactor = (float)pDoc->GetUnprocessedDataSize() / (float)dwLoopEnd; // size factor
	DWORD dwBreakWidth = (DWORD)(pDoc->GetBytesFromTime(pParseParm->fBreakWidth) / fFactor); // break width in process words
	if (!dwBreakWidth) {
		dwBreakWidth = 1;
	}

	// CLW 1.07a
	int nMaxThreshold = (int)((long)pLoudness->GetMaxValue() * (long)pParseParm->nMaxThreshold / 100); // threshold
	int nMinThreshold = (int)((long)pLoudness->GetMaxValue() * (long)pParseParm->nMinThreshold / 100); // threshold
	DWORD dwLoopPos = 0;
	BOOL bRes = TRUE;
	int nBlockAlign = pDoc->GetFmtParm()->wBlockAlign;

	// prepare gloss data
	int nGlossIndex = 0;

	// remove all gloss and deselect gloss
	DeleteContents();
	if (GetSelection() != -1) {
		SetSelection(-1);
	}
	pDoc->SetModifiedFlag(TRUE);		// document has been modified
	pDoc->SetTransModifiedFlag(TRUE);	// transcription data has been modified

	DWORD dwBreakCount = dwBreakWidth;	// allow early word SDM 1.5Test 11.0
	DWORD dwBreakStart = 0;				// the start of the break
	DWORD dwBreakEnd = 0;				// the end of the break

	CPhoneticSegment* pPhonetic = (CPhoneticSegment*)pDoc->GetSegment(PHONETIC); // get pointer to phonetic segment
	pDoc->AutoSnapUpdate(); // Snap phonetic SDM 1.5Test11.0

	// start processing
	while (dwLoopPos < dwLoopEnd)
	{
		// CLW 1.07a
		int nLoudnessData = pLoudness->GetProcessedData(dwLoopPos++, &bRes); // read zero crossing data point
		if (!bRes) return Exit(PROCESS_ERROR); // error, reading zero crossing data failed
		if (nLoudnessData > nMaxThreshold) { // point is over max threshold     // CLW 1.07a
			if (dwBreakCount >= dwBreakWidth) {
				// ready to store gloss
				DWORD dwGlossStart = nBlockAlign * (DWORD)(dwBreakEnd * fFactor/nBlockAlign);
				dwGlossStart = pDoc->SnapCursor(START_CURSOR, dwGlossStart,0,dwGlossStart,SNAP_LEFT);

				// check if the start is within a phonetic segment
				int nPhonetic = pPhonetic->FindFromPosition(dwGlossStart, TRUE);
				// adjust to phonetic
				if (nPhonetic != -1) {
					if ((pPhonetic->GetNext(nPhonetic) != -1) &&
						((pPhonetic->GetOffset(pPhonetic->GetNext(nPhonetic)) - dwGlossStart) < (dwGlossStart - pPhonetic->GetOffset(nPhonetic)))) {
						// next segment is closer SDM 1.5Test11.0
						dwGlossStart = pPhonetic->GetOffset(pPhonetic->GetNext(nPhonetic));
					} else {
						dwGlossStart = pPhonetic->GetOffset(nPhonetic);
					}
				} else {
					nPhonetic = pPhonetic->FindFromPosition(dwGlossStart);
					if ((nPhonetic != -1) &&
						((dwGlossStart - pPhonetic->GetOffset(nPhonetic)) < pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME))) {
						// Too close to existing segment SDM 1.5Test11.0
						dwGlossStart = pPhonetic->GetOffset(nPhonetic);
					} else {
						// there is no phonetic segment, create one
						if (nPhonetic == -1) {
							nPhonetic = 0; // no phonetic yet
						} else {
							if (dwGlossStart > pPhonetic->GetOffset(nPhonetic)) nPhonetic++; // append at end
						}
						// SDM 1.06.3a Insert 0 duration calculate later
						CSaString szSegment = SEGMENT_DEFAULT_CHAR;
						TRACE("phonetic start = %d\n",dwGlossStart);
						bRes = pPhonetic->Insert(nPhonetic, &szSegment, 0, dwGlossStart, 0);
						if (!bRes) {
							return Exit(PROCESS_ERROR); // error, writing segment failed
						}
					}
				}
				// write gloss
				TRACE("gloss start = %d\n",dwGlossStart);
				bRes = Insert(nGlossIndex++, NULL, 0, dwGlossStart, 0);
				if (!bRes) {
					return Exit(PROCESS_ERROR); // error, writing gloss failed
				}

				//adjust previous phonetic SDM 1.5Test11.0
				nPhonetic = pPhonetic->GetPrevious(pPhonetic->FindOffset(dwGlossStart));
				if ((nPhonetic != -1) && (pPhonetic->GetDuration(nPhonetic)==0)) {
					DWORD dwPhoneticStop = nBlockAlign * (DWORD)(dwBreakStart * fFactor/nBlockAlign);
					dwPhoneticStop = pDoc->SnapCursor(STOP_CURSOR, dwPhoneticStop,dwPhoneticStop,pDoc->GetUnprocessedDataSize(),SNAP_RIGHT);
					if (dwPhoneticStop > pPhonetic->GetOffset(nPhonetic)) {
						pPhonetic->Adjust(pDoc, nPhonetic, pPhonetic->GetOffset(nPhonetic), dwPhoneticStop - pPhonetic->GetOffset(nPhonetic));
					}
				}

				pDoc->SetModifiedFlag(TRUE); // document has been modified
				pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
			}
			dwBreakCount = 0; // end of valley
		} else {
			// CLW 1.07a
			if (nLoudnessData <= nMinThreshold) {  // point is under min threshold
				dwBreakEnd = dwLoopPos;
				if (dwBreakCount == 0) {
					dwBreakStart = dwLoopPos - 1;// SDM 1.5 Test 11.0
				}
				dwBreakCount++;
			} else if (dwBreakCount > 0) { // SDM 1.5 Test 11.0
				dwBreakCount++;
			}
		}
		// set progress bar
		SetProgress(nProgress + (int)(100 * dwLoopPos / dwLoopEnd / (DWORD)nLevel));
		if (IsCanceled()) {
			return Exit(PROCESS_CANCELED); // process canceled
		}
	}

	//adjust last phonetic SDM 1.5Test11.0
	int nPhonetic = pPhonetic->GetPrevious(-1);
	if ((nPhonetic != -1) && (pPhonetic->GetDuration(nPhonetic)==0)) {
		DWORD dwPhoneticStop = nBlockAlign*(DWORD)(dwBreakStart * fFactor/nBlockAlign);
		dwPhoneticStop = pDoc->SnapCursor(STOP_CURSOR, dwPhoneticStop,dwPhoneticStop,pDoc->GetUnprocessedDataSize(),SNAP_RIGHT);
		if (dwPhoneticStop > pPhonetic->GetOffset(nPhonetic)) {
			pPhonetic->Adjust(pDoc,nPhonetic, pPhonetic->GetOffset(nPhonetic),dwPhoneticStop - pPhonetic->GetOffset(nPhonetic));
		}
	}

	// Adjust durations of inserted phonetic segments
	if (!(pPhonetic->IsEmpty())) {
		int nPhonetic = 0;
		CSaString szSegment = SEGMENT_DEFAULT_CHAR;
		while (nPhonetic != -1) {
			int nNext = pPhonetic->GetNext(nPhonetic);
			if ((pPhonetic->GetDuration(nPhonetic)==0) && (pPhonetic->GetSegmentString(nPhonetic) == szSegment)) {
				//Inserted phonetic segment
				if (nNext != -1) {
					pPhonetic->Adjust(pDoc, nPhonetic, pPhonetic->GetOffset(nPhonetic),pPhonetic->GetOffset(nNext) - pPhonetic->GetOffset(nPhonetic));
				} else {
					// Last phonetic segment
					pPhonetic->Adjust(pDoc, nPhonetic, pPhonetic->GetOffset(nPhonetic),pDoc->GetUnprocessedDataSize() - 1 - pPhonetic->GetOffset(nPhonetic));
				}
			}
			nPhonetic = nNext;
		}
	}

	CorrectGlossDurations(pDoc);

	// calculate the actual progress
	nProgress = nProgress + (int)(100 / nLevel);
	EndProcess((nProgress >= 95)); // end data processing
	EndWaitCursor();
	SetDataReady();
	return MAKELONG(nLevel, nProgress);
}

void CGlossSegment::CorrectGlossDurations(CSaDoc* pDoc)
{
	// Adjust Gloss Durations
	if (!IsEmpty())
	{
		int nGloss = 0;
		while (nGloss != -1)
		{
			Adjust(pDoc, nGloss, GetOffset(nGloss), CalculateDuration(pDoc, nGloss));
			nGloss = GetNext(nGloss);
		}
	}
}

/***************************************************************************/
// gIPAInputFilter
/***************************************************************************/
BOOL CALLBACK EXPORT gIPAInputFilter(CSaString& szString)
{
	TCHAR cIPASpaceReplace = 0xFFFD; // Box Character
	int nIndex = 0;

	BOOL bChanged = FALSE;

	while ((nIndex < szString.GetLength())&&(szString[nIndex] != 0))
	{
		if (szString[nIndex] < 0x20)
		{
			szString.SetAt(nIndex, cIPASpaceReplace);
			bChanged = TRUE;
		}

		nIndex++;
	}
	return bChanged;
}

int CSegment::GetOffsetSize() const 
{
	ASSERT(m_pOffset);
	return m_pOffset->GetSize();
}

int CSegment::GetDurationSize() const 
{
	ASSERT(m_pDuration);
	return m_pDuration->GetSize();
}

DWORD CSegment::GetOffset(const int nIndex) const 
{
	//TRACE("nIndex=%d\n",nIndex);
	//TRACE("offset size=%d\n",GetOffsetSize());
	//if (nIndex>=0) {
		//TRACE("getat=%d\n",m_pOffset->GetAt(nIndex));
	//}
	return ((nIndex < GetOffsetSize()) && (nIndex >= 0)) ? m_pOffset->GetAt(nIndex) : 0L;
}

DWORD CSegment::GetDuration(const int nIndex) const 
{
	return (nIndex < GetDurationSize() && (nIndex >= 0)) ? m_pDuration->GetAt(nIndex) : 0L;
}

DWORD CSegment::GetStop(const int nIndex) const 
{
	return GetOffset(nIndex)+GetDuration(nIndex);
}

/** returns true if there are no offsets */
BOOL CSegment::IsEmpty() const { 
	return (m_pOffset->GetSize() == 0);
}

// return pointer to annotation string
CSaString * CSegment::GetString() {
	return m_pAnnotation;
}

// return pointer to text string array object
const CStringArray* CSegment::GetTexts() 
{
	return NULL;
} 

DWORD CSegment::GetDurationAt(int index) const 
{
	return m_pDuration->GetAt(index);
}

void CSegment::SetAt(int index, DWORD offset, DWORD duration) 
{
	m_pOffset->SetAtGrow(index,offset);
	m_pDuration->SetAtGrow(index,duration);
}

void CSegment::InsertAt(int index, DWORD offset, DWORD duration) 
{
	m_pOffset->InsertAt(index,offset,1);
	m_pDuration->InsertAt(index,duration,1);
}

/**
* remove offset and duration
*/
void CSegment::RemoveAt(int index, int length) 
{
	m_pOffset->RemoveAt(index,length);
	m_pDuration->RemoveAt(index,length);
}

void CSegment::SetDurationAt(int index, DWORD duration) 
{
	m_pDuration->SetAt(index,duration);
}

int CDependentSegment::CheckPosition(CSaDoc* pDoc, DWORD dwStart, DWORD dwStop, EMode nMode, BOOL bUnused) const
{ 
	return CheckPositionToMaster(pDoc, dwStart, dwStop, nMode);
}

