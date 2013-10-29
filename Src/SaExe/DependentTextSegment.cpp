#include "stdafx.h"
#include "DependentTextSegment.h"
#include "resource.h"
#include "Sa_Doc.h"
#include "SA_View.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//SDM 1.5Test8.1
/////////////////////////////////////////////////////////////////////////////
// CDependentTextSegment helper functions

CDependentTextSegment::CDependentTextSegment(int index, int master) :
    CTextSegment(index,master)
{
}

void CDependentTextSegment::LimitPosition(CSaDoc *, DWORD & dwStart, DWORD & dwStop, int /*nMode*/) const
{
    dwStart=GetOffset(GetSelection());
    dwStop=GetStop(GetSelection());
}

/***************************************************************************/
// CDependentTextSegment::Insert Insert/append a text segment
// Returns FALSE if an error occurred. If the pointer to the string is NULL
// there will be no string added.
//
// When inserting beyond the end of the current list, nIndex is the next
// available index.  For example if we are inserting at the 4 gloss segment
// but only reference segments 0 and 1 exist, then the next index is 2, not 3.
/***************************************************************************/
BOOL CDependentTextSegment::SetAt(const CSaString * pszString, bool, DWORD dwStart, DWORD dwDuration)
{

    try
    {
        if ((pszString) && (pszString->GetLength()))
        {
            int nIndex = FindOffset(dwStart);
            ASSERT(nIndex>=0);
            m_Texts.SetAtGrow(nIndex, *pszString);
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
// Returns FALSE if an error occurred. If the pointer to the string is NULL
// there will be no string added.
//
// When inserting beyond the end of the current list, nIndex is the next
// available index.  For example if we are inserting at the 4 gloss segment
// but only reference segments 0 and 1 exist, then the next index is 2, not 3.
/***************************************************************************/
BOOL CDependentTextSegment::Insert(int nIndex, LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration)
{

    if (pszString==NULL)
    {
        return TRUE;
    }

    try
    {
        if (wcslen(pszString)>0)
        {
            m_Texts.InsertAt(nIndex, pszString, 1);
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

BOOL CDependentTextSegment::InsertBlank(int nIndex, DWORD dwStart, DWORD dwDuration)
{
    try
    {
        InsertAt(nIndex,dwStart,dwDuration);
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
// Returns FALSE if an error occurred. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CDependentTextSegment::SetText(int nIndex, LPCTSTR pszString, int, DWORD, DWORD)
{

    if (pszString==NULL)
    {
        return TRUE;
    }

    try
    {
        if (wcslen(pszString)>0)
        {
            m_Texts.SetAt(nIndex, pszString);
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

// SDM 1.5 Test11.0
/***************************************************************************/
// CDependentSegment::CheckPositionToSegment Check positions for dep. segments
// Checks the positions for validation. If they are ok for a new segment
// (depending on master segment positions), it returns the
// index, where to put them in in the arrays (0 based), otherwise -1. The
// start must not be placed in the range of a segment, where already
// another segment is aligned to, but there must be a segment to align to.
/***************************************************************************/
int CDependentTextSegment::CheckPositionToMaster(CSaDoc * pSaDoc, DWORD dwAlignedStart, DWORD dwStop, EMode nMode) const
{

    int nTextIndex = GetSelection();
    if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(nTextIndex != -1))   // segment selected (edit)
    {
        return -1;
    }
    else     // Add
    {
        return CDependentSegment::CheckPositionToMaster(pSaDoc, dwAlignedStart, dwStop, nMode);
    }
}

/***************************************************************************/
// CDependentTextSegment::Add Add dependent annotation segment
/***************************************************************************/
void CDependentTextSegment::Add(CSaDoc * pDoc, DWORD dwStart, CSaString & szString, BOOL, BOOL bCheck)
{

    // get pointer to view
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    // get the offset and duration from master
    int nSegment = pDoc->GetSegment(GLOSS)->FindOffset(dwStart);
    if (nSegment == -1)
    {
        return; // return on error
    }

    DWORD dwDuration = pDoc->GetSegment(GLOSS)->GetDuration(nSegment);
	DWORD dwStop = dwStart + dwDuration;

    int nPos = CheckPosition(pDoc, dwStart, dwStop, MODE_ADD); // get the insert position
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
    if (!Insert(nPos, szString, 0, dwStart, dwDuration))
    {
        return; // return on error
    }

    pDoc->SetModifiedFlag(TRUE); // document has been modified
    pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
    pView->ChangeAnnotationSelection(this, nPos, dwStart, dwStop); // change the selection
    pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}
