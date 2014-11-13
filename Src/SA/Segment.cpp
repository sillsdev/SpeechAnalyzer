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
CSegment::CSegment( EAnnotation index, int master)
{
    m_nAnnotationIndex = index;
    m_nMasterIndex = master;
    m_nSelection = - 1;                 // no segment selected
    m_pAnnotation = new CSaString();    // create annotation string object
}

/***************************************************************************/
// CSegment::~CSegment Destructor
/***************************************************************************/
CSegment::~CSegment()
{
    if (m_pAnnotation!=NULL)
    {
        delete m_pAnnotation;
        m_pAnnotation = NULL;
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
    m_Offset.RemoveAll();
    m_Duration.RemoveAll();
    m_nSelection = -1; // SDM 1.5Test8.1 empty segments can not be selected
}

void CSegment::Serialize(CArchive & ar)
{
    if (ar.IsStoring())
    {
        ar << CSaString("integrityCheck");
        ar << CSaString(*m_pAnnotation);        // annotation string
        int x = GetOffsetSize();
        SA_ASSERT(x);
        CArchiveTransfer::tInt(ar, x);
        for (int i = 0; i<GetOffsetSize(); i++)
        {
            CArchiveTransfer::tDWORD(ar, GetOffset(i));
        }
        CArchiveTransfer::tInt(ar, GetDurationSize());
        for (int i = 0; i< GetDurationSize(); i++)
        {
            CArchiveTransfer::tDWORD(ar, GetDuration(i));
        }
        CArchiveTransfer::tInt(ar, m_nSelection);
    }
    else
    {
        CSaString integrityCheck;
        ar >> integrityCheck;
        SA_ASSERT(integrityCheck == "integrityCheck");
        ar >> *m_pAnnotation;        // annotation string
        int x = CArchiveTransfer::tInt(ar);
        SA_ASSERT(x);
        m_Offset.SetSize(x);
        for (int i=0; i< GetOffsetSize(); i++)
        {
            m_Offset.SetAt(i, CArchiveTransfer::tDWORD(ar));
        }
        m_Duration.SetSize(CArchiveTransfer::tInt(ar));
        for (int i=0; i<m_Duration.GetSize(); i++)
        {
            m_Duration.SetAt(i, CArchiveTransfer::tDWORD(ar));
        }
        m_nSelection = CArchiveTransfer::tInt(ar);
    }
}


/***************************************************************************/
// CSegment::Remove Remove dependent annotation segment
/***************************************************************************/
void CSegment::Remove(CDocument * pSaDoc, BOOL bCheck)
{
    // get pointer to view
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    // save state for undo ability
    if (bCheck)
    {
        pDoc->CheckPoint();
    }

    RemoveNoRefresh(NULL);

    // refresh ui
    pDoc->SetModifiedFlag(TRUE); // document has been modified
    pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
    pView->ChangeAnnotationSelection(this, m_nSelection, 0, 0); // deselect
    pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}

/***************************************************************************/
// CSegment::Replace
/***************************************************************************/
void CSegment::Replace( CSaDoc * pDoc, int index, LPCTSTR find, LPCTSTR replace)
{
	if (index==-1) return;
	if (IsEmpty()) return;
	if (wcslen(find)==0) return;
	if (wcslen(replace)==0) return;

	DWORD dwOffset = GetOffset(index);
	DWORD dwDuration = GetDuration(index);
	DWORD dwStop = dwOffset+dwDuration;

    // find length of string to delete
    int length = GetSegmentLength(index); 

	CString segment = GetSegmentString(index);

	// no replacement occurred, just leave
	if (segment.Replace( find, replace)==0) return;

	// set in the new string
	CString left = m_pAnnotation->Left(index);
	CString right = m_pAnnotation->Right( m_pAnnotation->GetLength() - length - index);
	*m_pAnnotation = left + segment + right;

	// adjust the offset and duration tables
	int diff = segment.GetLength()-length;
	if (diff==0)
	{
		// if the lengths are the same, we don't need to adjust the offsets
	}
	else if (diff<0)
	{
		//it shrank
		for (int j=0;j<abs(diff);j++)
		{
			m_Offset.RemoveAt(index,1);
			m_Duration.RemoveAt(index,1);
		}
	}
	else if (diff>0)
	{
		// it grew
		for (int j=0;j<diff;j++)
		{
			m_Offset.InsertAt(index,dwOffset,1);
			m_Duration.InsertAt(index,dwDuration,1);
		}
	}


    // get pointer to view
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    pDoc->SetModifiedFlag(TRUE);		// document has been modified
    pDoc->SetTransModifiedFlag(TRUE);	// transcription data has been modified
    pView->ChangeAnnotationSelection(this, index, dwOffset, dwStop);	// deselect
    pView->ChangeAnnotationSelection(this, index, dwOffset, dwStop);	// select again
    pView->RefreshGraphs(FALSE);		// refresh the graphs between cursors
}

/***************************************************************************/
// CSegment::ReplaceSelectedSegment
/***************************************************************************/
void CSegment::ReplaceSelectedSegment(CSaDoc * pSaDoc, LPCTSTR replace)
{
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

	if (m_nSelection==-1) return;

    DWORD dwOffset = GetOffset(m_nSelection);
    DWORD dwDuration = GetDuration(m_nSelection);
	DWORD dwStop = dwOffset + dwDuration;

    RemoveNoRefresh(NULL);

    // insert or append the new dependent segment
	// return on error
    if (!Insert( m_nSelection, replace, 0, dwOffset, dwDuration)) return;

    int nSaveSelection = m_nSelection;
    //SDM 1.06.5
    pDoc->SetModifiedFlag(TRUE);        // document has been modified
    pDoc->SetTransModifiedFlag(TRUE);   // transcription data has been modified
    pView->ChangeAnnotationSelection( this, m_nSelection, dwOffset, dwStop);		// deselect
    pView->ChangeAnnotationSelection( this, nSaveSelection, dwOffset, dwStop);	// select again
    pView->RefreshGraphs(FALSE);        // refresh the graphs between cursors
}

/***************************************************************************/
// CSegment::RemoveNoRefresh Remove dependent annotation segment
/***************************************************************************/
DWORD CSegment::RemoveNoRefresh(CDocument *)
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
BOOL CSegment::SetAt(const CSaString * pszString, bool, DWORD dwStart, DWORD dwDuration)
{
    try
    {
        int nStringLength = pszString->GetLength(); // get the length of the new string
        if (m_Offset.GetCount()==0)
        {
            ASSERT(m_Duration.GetCount()==0);
            ASSERT(m_pAnnotation->GetLength()==0);
            *m_pAnnotation += *pszString;
            m_Offset.InsertAt(0,dwStart,nStringLength);
            m_Duration.InsertAt(0,dwDuration,nStringLength);
        }
        else
        {
            int nIndex = FindOffset(dwStart);
            if (nIndex>=0)
            {
                // index was found

                // remove the existing string
                // find length of string to delete
                int nSegmentLength = GetSegmentLength(nIndex);
                *m_pAnnotation = m_pAnnotation->Left(nIndex) + m_pAnnotation->Right(m_pAnnotation->GetLength() - nSegmentLength - nIndex);
                RemoveAt(nIndex, nSegmentLength);

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
                m_Offset.InsertAt(nIndex,dwStart,nStringLength);
                m_Duration.InsertAt(nIndex,dwDuration,nStringLength);
            }
            else
            {
                // index was at end of string
                nIndex = m_Offset.GetCount();
                if (nIndex < m_pAnnotation->GetLength())
                {
                    // insert
                    *m_pAnnotation = m_pAnnotation->Left(nIndex) + *pszString + m_pAnnotation->Right(m_pAnnotation->GetLength() - nIndex);
                }
                else
                {
                    *m_pAnnotation += *pszString; // append
                }
                m_Offset.InsertAt(nIndex,dwStart,nStringLength);
                m_Duration.InsertAt(nIndex,dwDuration,nStringLength);
            }
        }
    }
    catch (CMemoryException e)
    {
        // memory allocation error
        ErrorMessage(IDS_ERROR_MEMALLOC);
        return FALSE;
    }
    ASSERT(m_pAnnotation->GetLength()==GetOffsetSize());
    ASSERT(m_pAnnotation->GetLength()==m_Duration.GetSize());
    return TRUE;
}

/***************************************************************************
* CSegment::Insert Insert/append an annotation segment
* Returns FALSE if an error occurred.
* nDelimiter is not used in this basic version of the function.
* nIndex index into annotation string
***************************************************************************/
BOOL CSegment::Insert( int nIndex, LPCTSTR pszString, bool /*delimiter*/, DWORD dwStart, DWORD dwDuration)
{

    // get the length of the new string
    int nStringLength = wcslen(pszString);
    try
    {
        if (nIndex < m_pAnnotation->GetLength())
        {
            // insert
            *m_pAnnotation = m_pAnnotation->Left(nIndex) + pszString + m_pAnnotation->Right(m_pAnnotation->GetLength() - nIndex);
        }
        else
        {
            *m_pAnnotation += pszString;
        }
        m_Offset.InsertAt(nIndex, dwStart, nStringLength);
        m_Duration.InsertAt(nIndex, dwDuration, nStringLength);
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
    int nReference = 0;
    if (nIndex >= 0)
    {
        nReference = nIndex - 1; // user defined reference
        if (nReference >= GetOffsetSize())
        {
            nReference = GetOffsetSize() - 1;
        }
    }
    else
    {
        // take current selection
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
    if (nIndex >= 0)
    {
        nReference = nIndex; // user defined reference
    }
    else     // take current selection
    {
        if (m_nSelection != -1)
        {
            nReference = m_nSelection;
        }
        else
        {
            nReference = 0; // default for nothing has been selected yet
        }
    }
    DWORD dwOffset = GetOffset(nReference);
    nReference++;
    while ((nReference < GetOffsetSize()) && (GetOffset(nReference) == dwOffset))
    {
        nReference++;
    }
    if (nReference == GetOffsetSize())
    {
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
int CSegment::FindOffset(DWORD dwOffset) const
{

    if (IsEmpty())
    {
        return -1;
    }

    DWORD * pFirst = (DWORD *)m_Offset.GetData();
    // pLast should be set to the last position + 1
    DWORD * pLast = pFirst + GetOffsetSize();
    DWORD * pLowerBound = std::lower_bound(pFirst, pLast, dwOffset);
    if ((pLowerBound != pLast) && (*pLowerBound == dwOffset))
    {
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
int CSegment::FindStop(DWORD dwStop) const
{

    if (IsEmpty())
    {
        return -1;
    }

    // Use std::lower_bound which has O(log(N)) for performance reasons
    // Needed for large annotated files
    DWORD * pFirst = (DWORD *)m_Offset.GetData();
    DWORD * pLast = pFirst + (GetOffsetSize()-1);
    DWORD * pLowerBound = std::lower_bound(pFirst, pLast, dwStop);

    int nLoop = 0;
    if (pLowerBound != pLast)
    {
        nLoop = pLowerBound - pFirst;
    }
    else
    {
        nLoop = GetOffsetSize()-1;
    }

    for (; nLoop >= 0; --nLoop)
    {
        if (GetStop(nLoop) < dwStop)
        {
            return -1; // no match
        }
        if (GetStop(nLoop) == dwStop)
        {
            while (nLoop && GetStop(nLoop -1) == dwStop)
            {
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
* If there are no segments, it returns -1.
*
* if bWithin=false, it returns last index if it is above last index.
*
* if bWithin=true, index is returned if within segment.  -1 returned otherwise
*
***************************************************************************/
int CSegment::FindFromPosition( DWORD dwPosition, BOOL bWithin) const
{
    if (IsEmpty())
    {
        // there are no segments
        return -1;
    }

    // check if position above highest segment
    DWORD dwTempPos = dwPosition;

    // get end position of last segment
    DWORD dwStop = GetStop(GetOffsetSize()-1);
    if (dwStop <= dwTempPos)
    {
        // if we are at or beyond the last end position then
        // set position with last end position
        dwTempPos = dwStop - 1;
    }

    // Use std::lower_bound which has O(log(N)) for performance reasons
    // Needed for large annotated files
    DWORD * pFirst = (DWORD *)m_Offset.GetData();
    DWORD * pLast = pFirst + (GetOffsetSize()-1);
	// Returns an iterator pointing to the first element in the range [first,last) which does not compare less than val.
	// Returns an iterator to the lower bound of val in the range.
	// If all the element in the range compare less than val, the function returns last.
    DWORD * pLowerBound = std::lower_bound( pFirst, pLast, dwPosition);

    int nLoop = 0;
    if (pLowerBound != pLast)
    {
        // we are not at end, use delta
        nLoop = pLowerBound - pFirst;
    }
    else
    {
        // use max index
        nLoop = GetOffsetSize()-1;
    }

	// walk backwards and find the first stop position we are greater than
	// the stop position is offset+duration, which may not span the entire
	// area between offsets(?)
	// dwTempPos is placed at stop - 1
    for ( ; nLoop >= 0; nLoop = GetPrevious(nLoop))
    {
        if (GetStop(nLoop) <= dwTempPos)
        {
            break; // this is it
        }
    }

	// advance one position.  if index is negative - set to first position
    if (nLoop >=0)
    {
        nLoop = GetNext(nLoop);  // selected character is one position higher
    }
    else
    {
        nLoop = 0;
    }

    if (bWithin)
    {
        if ((dwPosition < GetOffset(nLoop)) || (dwPosition > GetStop(nLoop)))
        {
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
int CSegment::GetSegmentLength( int index) const
{
    DWORD dwOffset = GetOffset(index);
    int length = m_pAnnotation->GetLength();
	// breaks when the offsets no longer match
    for ( int i = index + 1; i < length; i++)
    {
        if (dwOffset != GetOffset(i))
        {
			// length of segment
            return (i-index);
        }
    }
	// all offsets match to end
    return (length - index); 
}

/***************************************************************************/
// CSegment::GetSegmentString Return the segment character string
/***************************************************************************/
CSaString CSegment::GetSegmentString(int nIndex)   const
{
    CSaString szSegment = m_pAnnotation->GetAt(nIndex);
    DWORD dwOffset = GetOffset(nIndex++);
    while ((nIndex < m_pAnnotation->GetLength()) && (dwOffset == GetOffset(nIndex)))
    {
        szSegment += m_pAnnotation->GetAt(nIndex++);
    }
    return szSegment;
}

/***************************************************************************/
// CSegment::Adjust Adjusts positions of an annotation segment
/***************************************************************************/
void CSegment::Adjust( ISaDoc * pDoc, int nIndex, DWORD dwOffset, DWORD dwDuration)
{
    DWORD dwOldOffset = GetOffset(nIndex);
    DWORD dwOldStop = GetStop(nIndex);

	// adjust this segment
    for (int nLoop = nIndex; nLoop < GetOffsetSize(); nLoop++)
    {
        if (GetOffset(nLoop) == dwOldOffset)
        {
            m_Offset.SetAt(nLoop, dwOffset);          // set new offset
            m_Duration.SetAt(nLoop, dwDuration);      // set new duration
        }
        else
        {
            break;
        }
    }

	// adjust all dependent segments
    for (int nWnd = 0; nWnd < ANNOT_WND_NUMBER; nWnd++)
    {
        CSegment * pSegment = pDoc->GetSegment(nWnd);
        if ((pSegment!=NULL) && (pSegment->GetMasterIndex() == m_nAnnotationIndex))
        {
            int nIndex = pSegment->FindOffset(dwOldOffset);
            if (nIndex != -1)
            {
                pSegment->Adjust( pDoc, nIndex, dwOffset, pSegment->GetStop(nIndex) - dwOffset);
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
int CSegment::CheckCursors( CSaDoc * pDoc, BOOL bOverlap) const
{
    // get pointer to view
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);
    return CheckPosition( pDoc, pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), MODE_AUTOMATIC, bOverlap);
}

BOOL CSegment::NeedToScroll(CSaView & saView, int index) const
{
    ASSERT(index >= 0 && index < GetOffsetSize());

    DWORD viewLeftEdge = (int) saView.GetDataPosition(0);
    DWORD viewRightEdge = viewLeftEdge + saView.GetDataFrame();
    DWORD anotLeftEdge = GetOffset(index);
    DWORD anotRightEdge = anotLeftEdge + GetDuration(index);

    return ((anotLeftEdge < viewLeftEdge) || (anotRightEdge > viewRightEdge));
}


int CSegment::FirstVisibleIndex(CSaDoc & SaDoc) const
{
    POSITION pos = SaDoc.GetFirstViewPosition();
    CSaView  * pView = (CSaView *)SaDoc.GetNextView(pos);
    for (int index=0; index<GetOffsetSize(); index++)
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
    CSaView  * pView = (CSaView *)SaDoc.GetNextView(pos);

    for (int index=GetOffsetSize()-1; index>=0; index--)
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
int CSegment::FindPrev(int fromIndex, LPCTSTR strToFind)
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

        int idxFirstChar = sSegment.ReverseFind(strToFind[0]);

        while (idxFirstChar >= 0)
        {
            CSaString str(sSegment.Mid(idxFirstChar,wcslen(strToFind)));
            if (str == strToFind)
            {
                break;
            }
            sSegment = sSegment.Left(idxFirstChar);
            idxFirstChar = sSegment.ReverseFind(strToFind[0]);
        }

        if (idxFirstChar >= 0)
        {
            int idxLastChar = idxFirstChar + wcslen(strToFind);

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
int CSegment::FindNext(int fromIndex, LPCTSTR strToFind)
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

        int idxLastChar = idxFirstChar + wcslen(strToFind);

        if (idxLastChar >= sSegment.GetLength())
        {
            idxLastChar = sSegment.GetLength() - 1 + offset;
        }
        DWORD dwStart = GetOffset(idxFirstChar);
        DWORD dwEnd = GetOffset(idxLastChar);

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
BOOL CSegment::Match( int index, LPCTSTR strToFind)
{
	if (index<0) return FALSE;
	if (IsEmpty()) return FALSE;
	int findLength = wcslen(strToFind);
	if (findLength==0) return FALSE;

    CSaString sSegment = *m_pAnnotation;

	int segLength = GetSegmentLength( index);
	CSaString segText = GetSegmentString( index);

	// our search string is longer than the segment - we will never match
	if (findLength>segLength) return FALSE;

	int nFind = segText.Find( strToFind);
	return (nFind!=-1);
}

void CSegment::SelectSegment(CSaDoc & SaDoc, int index)
{
    DWORD dwStart = GetOffset(index);
    DWORD dwEnd   = GetStop(index);
    POSITION pos = SaDoc.GetFirstViewPosition();
    CSaView  * pView = (CSaView *)SaDoc.GetNextView(pos);

    pView->ChangeAnnotationSelection(this, index, dwStart, dwEnd);
    pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}

/***************************************************************************/
// CSegment::AdjustCursorsToSnap Adjust cursors to apropriate snap position
// Adjust the current cursor positions to the positions, that they will have,
// when a segment will be selected.
//***************************************************************************/
void CSegment::AdjustCursorsToSnap(CDocument * pSaDoc)
{

    // get requested cursor alignment
    // snap the cursors first to appropriate position
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);
    DWORD dwNewOffset = pView->GetStartCursorPosition();
    DWORD dwNewDuration = pView->GetStopCursorPosition();
    pView->SetStartCursorPosition(dwNewOffset, SNAP_RIGHT);
    pView->SetStopCursorPosition(dwNewDuration, SNAP_LEFT);
}

/***************************************************************************/
// gIPAInputFilter
/***************************************************************************/
BOOL CALLBACK EXPORT gIPAInputFilter(CSaString & szString)
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
    return m_Offset.GetSize();
}

int CSegment::GetDurationSize() const
{
    return m_Duration.GetSize();
}

DWORD CSegment::GetOffset(const int nIndex) const
{
    return ((nIndex < GetOffsetSize()) && (nIndex >= 0)) ? m_Offset.GetAt(nIndex) : 0L;
}

DWORD CSegment::GetDuration(const int nIndex) const
{
    return (nIndex < GetDurationSize() && (nIndex >= 0)) ? m_Duration.GetAt(nIndex) : 0L;
}

DWORD CSegment::GetStop(const int nIndex) const
{
    return GetOffset(nIndex)+GetDuration(nIndex);
}

/** returns true if there are no offsets */
BOOL CSegment::IsEmpty() const
{
    return (m_Offset.GetSize() == 0);
}

DWORD CSegment::GetDurationAt(int index) const
{
    return m_Duration.GetAt(index);
}

void CSegment::SetAt(int index, DWORD offset, DWORD duration)
{
    m_Offset.SetAtGrow(index,offset);
    m_Duration.SetAtGrow(index,duration);
}

void CSegment::InsertAt(int index, DWORD offset, DWORD duration)
{
    m_Offset.InsertAt(index,offset,1);
    m_Duration.InsertAt(index,duration,1);
}

/**
* remove offset and duration
*/
void CSegment::RemoveAt(int index, int length)
{
    m_Offset.RemoveAt(index,length);
    m_Duration.RemoveAt(index,length);
}

void CSegment::SetDurationAt(int index, DWORD duration)
{
    m_Duration.SetAt(index,duration);
}

/*
* Returns the text that is within the start and stop markers
*/
CSaString CSegment::GetContainedText(DWORD dwStart, DWORD dwStop)
{

    CSaString szText;
    for (int i=0; i<GetOffsetSize(); i++)
    {
        DWORD offset = GetOffset(i);
		// offset can't be dwStop, because then the the segments length would be zero.
        if ((offset>=dwStart)&&(offset<dwStop))
        {
            szText.AppendChar(m_pAnnotation->GetAt(i));
        }
    }
    return szText;
}

/*
* Returns the text that contains the start and stop markers
*/
CSaString CSegment::GetOverlappingText(DWORD dwStart, DWORD dwStop)
{

    CSaString szText;
    for (int i=0; i<GetOffsetSize(); i++)
    {
        DWORD begin = GetOffset(i);
        DWORD end = begin+GetDurationAt(i);
        if (end<=dwStart)
        {
            continue;
        }
        if (begin>=dwStop)
        {
            continue;
        }
        szText.AppendChar(m_pAnnotation->GetAt(i));
    }
    return szText;
}

int CSegment::GetMasterIndex(void) const
{
    return m_nMasterIndex;
}

EAnnotation CSegment::GetAnnotationIndex(void) const
{
    return m_nAnnotationIndex;
}

CSegment::TpInputFilterProc CSegment::GetInputFilter(void) const
{
    // filter function for input
    return NULL;
}

TCHAR CSegment::GetChar(int nIndex) const
{
    return m_pAnnotation->GetAt(nIndex);
}

// return pointer to annotation string
CSaString * CSegment::GetString()
{
    return m_pAnnotation;
}

size_t CSegment::GetStringLength()
{
    if (m_pAnnotation==NULL)
    {
        return 0;
    }
    return m_pAnnotation->GetLength();
}

//overridden by derived classes
CSaString CSegment::GetText(int /*nIndex*/) const
{
    if (m_pAnnotation==NULL)
    {
        return L"";
    }
    // return text string
    return *m_pAnnotation;
}

void CSegment::SetString(LPCTSTR val)
{
    if (m_pAnnotation!=NULL)
    {
        delete m_pAnnotation;
        m_pAnnotation=NULL;
    }
    m_pAnnotation = new CSaString();
    *m_pAnnotation = val;
}

/**
* returns the selection index, or -1 if
* no segment is selected
*/
int CSegment::GetSelection() const
{
    return m_nSelection;   // return the index of the selected character
}

long CSegment::Process(void * /*pCaller*/, ISaDoc * /*pDoc*/, int /*nProgress*/, int /*nLevel*/)
{
    return PROCESS_ERROR;
}

void CSegment::Validate() 
{
	if (m_Offset.GetCount()==1) return;
	for (int i=1;i<m_Offset.GetCount();i++) {
		if (m_Offset[i-1]>m_Offset[i]) 
		{
			TRACE("offset[%d]=%lu %lu %lu\n",i-2,m_Offset[i-3],m_Offset[i-2]+m_Duration[i-3],m_Duration[i-3]);
			TRACE("offset[%d]=%lu %lu %lu\n",i-2,m_Offset[i-2],m_Offset[i-2]+m_Duration[i-2],m_Duration[i-2]);
			TRACE("offset[%d]=%lu %lu %lu\n",i-1,m_Offset[i-1],m_Offset[i-1]+m_Duration[i-1],m_Duration[i-1]);
			TRACE("offset[%d]=%lu %lu %lu\n",i,m_Offset[i],m_Offset[i]+m_Duration[i],m_Duration[i-1]);
			ASSERT(m_Offset[i-1]<=m_Offset[i]);
		}
	}
}
