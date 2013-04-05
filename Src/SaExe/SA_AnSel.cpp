/////////////////////////////////////////////////////////////////////////////
// sa_ansel.cpp
// Implementation of the CASegmentSelection class
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
#include "sa_ansel.h"
#include "Process\Process.h"
#include "Segment.h"
#include "sa_graph.h"

#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "DependentSegment.h"

CASegmentSelection::CASegmentSelection()
{
    m_Selection.nAnnotationIndex=-1;
    m_Selection.bVirtual = FALSE;
}

/***************************************************************************/
// CASegmentSelection::SelectFromPosition Select a annotation segment from
// a position. Returns TRUE if selection changed else FALSE
/***************************************************************************/
BOOL CASegmentSelection::SelectFromPosition(CSaView * pView, int nSegmentIndex, DWORD dwPosition, int nMode)
{
    // Get original selection for return test
    Update(pView);
    int nAnnotationIndex = m_Selection.nAnnotationIndex;
    DWORD dwStart = m_Selection.dwStart;

    // get pointer to segment object
    CSegment * pSegment = pView->GetDocument()->GetSegment(nSegmentIndex);
    CSaDoc * pDoc = pView->GetDocument();

    BOOL bWithin = TRUE;
    if ((nSegmentIndex == GLOSS)||(nSegmentIndex == PHONETIC))
    {
        bWithin = FALSE;
    }

    int nMaster = pSegment->GetMasterIndex();
    if (nMaster == -1)
    {
        nMaster = nSegmentIndex;
    }
    CSegment * pMaster = pView->GetDocument()->GetSegment(nMaster);

    // get pointer to annotation offsets
    if (!(pSegment->IsEmpty()))   // there is at least one segment
    {
        // get the segment index at the given position
        int nSelection = pSegment->FindOffset(dwPosition); // More precise less robust
        if ((nSelection==-1) && (!(bWithin&&(nMode&FIND_EXACT))))
        {
            // Better Coverage
            nSelection = pSegment->FindFromPosition(dwPosition, bWithin);
        }

        if ((nSelection==-1) && (bWithin&&(nMode&FIND_EXACT)) && (pSegment->GetSelection() == -1))
        {
            int nIndex = pMaster->FindOffset(dwPosition);
            if (nIndex != -1)
            {
                nSelection = pSegment->FindFromPosition(pMaster->GetOffset(nIndex) + pMaster->GetDuration(nIndex)/2, bWithin);
            }
        }
        // change the selection
        if (nSelection!=-1)
        {
            if (nSelection != pSegment->GetSelection())
            {
                //Don't deselect
                pView->ChangeAnnotationSelection(pSegment, nSelection, pSegment->GetOffset(nSelection), pSegment->GetStop(nSelection));
            }
        }
        else
        {
            pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
        }
    }
    else
    {
        pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
    }

    if ((nMaster != nSegmentIndex) && (pSegment->GetSelection() == -1))
    {

        int nSelection = pMaster->FindOffset(dwPosition); // More precise less robust
        if ((nSelection==-1))   /*&&!(bWithin&&(nMode&FIND_EXACT)))*/
        {
            nSelection = pMaster->FindFromPosition(dwPosition, bWithin);
        }
        if (nSelection==-1)
        {
            nSelection = pMaster->FindFromPosition(dwPosition, FALSE);
        }

        if (nSelection != -1)   //Set up virtual selection
        {
            // Deselect Virtual Selection
            if (m_Selection.bVirtual)
            {
                RefreshAnnotation(pView, m_Selection.nAnnotationIndex);
            }
            m_Selection.bVirtual = TRUE;
            m_Selection.nAnnotationIndex = nSegmentIndex;
            m_Selection.dwStart = pMaster->GetOffset(nSelection);
            m_Selection.dwDuration = pMaster->GetDuration(nSelection);
            m_Selection.dwStop = m_Selection.dwStart + m_Selection.dwDuration;
            pView->SetStartCursorPosition(m_Selection.dwStart, SNAP_RIGHT);
            pView->SetStopCursorPosition(m_Selection.dwStop, SNAP_LEFT);
        }
        else
        {
            // Deselect Virtual Selection
            if (m_Selection.bVirtual)
            {
                RefreshAnnotation(pView, m_Selection.nAnnotationIndex);
            }
            m_Selection.bVirtual = FALSE;
            m_Selection.nAnnotationIndex = -1;
            m_Selection.dwStart = 0;
            m_Selection.dwDuration = 0;
            m_Selection.dwStop = 0;
        }
    }

    // highlight possible insertion point
    if ((nMaster == nSegmentIndex) && (nMaster != PHONETIC) && (pSegment->GetSelection() == -1))
    {

        DWORD dwStart = dwPosition; // Start at current stop
        DWORD dwStop;

        // Snap Start Position
        dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, 0, dwStart, SNAP_LEFT);

        dwStop = (dwStart + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));

        if (pDoc->Is16Bit())
        {
            dwStop = (dwStop + 1) & ~1; // Round up
        }

        dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStop, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);

        int nInsertAt = pSegment->CheckPosition(pDoc,dwStart,dwStop,CSegment::MODE_ADD);
        if (nInsertAt != -1)
        {
            // Deselect Virtual Selection
            if (m_Selection.bVirtual)
            {
                RefreshAnnotation(pView, m_Selection.nAnnotationIndex);
            }

            if (nInsertAt > 0)
            {
                if ((pSegment->GetStop(nInsertAt-1) +  pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) > dwStart)
                {
                    dwStart = pSegment->GetStop(nInsertAt-1);
                }
            }

            if (nInsertAt < pSegment->GetOffsetSize())
            {
                if ((dwStart+pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < pSegment->GetOffset(nInsertAt))
                {
                    dwStop = (dwStart + pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
                }
                else
                {
                    dwStop = pSegment->GetOffset(nInsertAt);
                }
            }
            else
            {
                if ((dwStart+pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < pDoc->GetUnprocessedDataSize())
                {
                    dwStop = (dwStart + pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
                }
                else
                {
                    dwStop = pDoc->GetUnprocessedDataSize();
                }
            }

            if (pDoc->Is16Bit())
            {
                dwStop = (dwStop + 1) & ~1; // Round up
            }

            dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, dwStart, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);
            dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, 0, dwStop, SNAP_LEFT);

            m_Selection.bVirtual = TRUE;
            m_Selection.nAnnotationIndex = nSegmentIndex;
            m_Selection.dwStart = dwStart;
            m_Selection.dwDuration = dwStop-dwStart;
            m_Selection.dwStop = dwStop;
            pView->SetStartCursorPosition(m_Selection.dwStart, SNAP_RIGHT);
            pView->SetStopCursorPosition(m_Selection.dwStop, SNAP_LEFT);
        }
        else
        {
            // Deselect Virtual Selection
            if (m_Selection.bVirtual)
            {
                RefreshAnnotation(pView, m_Selection.nAnnotationIndex);
            }
            m_Selection.bVirtual = FALSE;
            m_Selection.nAnnotationIndex = -1;
            m_Selection.dwStart = 0;
            m_Selection.dwDuration = 0;
            m_Selection.dwStop = 0;
        }
    }
    Update(pView);
    return !((nAnnotationIndex == m_Selection.nAnnotationIndex)&&(dwStart == m_Selection.dwStart));
}

/***************************************************************************/
// CASegmentSelection::SelectFromStopPosition Select a annotation segment from
// a position. Returns TRUE if selection changed else FALSE
/***************************************************************************/
BOOL CASegmentSelection::SelectFromStopPosition(CSaView * pView, int nSegmentIndex, DWORD dwStop, int nMode)
{
    // Get original selection for return test
    Update(pView);
    int nAnnotationIndex = m_Selection.nAnnotationIndex;
    DWORD dwStart = m_Selection.dwStart;

    // get pointer to segment object
    CSegment * pSegment = pView->GetDocument()->GetSegment(nSegmentIndex);
    CSaDoc * pDoc = pView->GetDocument();
    BOOL bWithin = TRUE;
    if ((nSegmentIndex == GLOSS)||(nSegmentIndex == PHONETIC))
    {
        bWithin = FALSE;
    }

    int nMaster = pSegment->GetMasterIndex();
    if (nMaster == -1)
    {
        nMaster = nSegmentIndex;
    }
    CSegment * pMaster = pView->GetDocument()->GetSegment(nMaster);

    // get pointer to annotation offsets
    if (!(pSegment->IsEmpty()))   // there is at least one segment
    {
        // get the segment index at the given position
        int nSelection = pSegment->FindStop(dwStop); // More precise less robust
        if ((nSelection==-1)&&!(bWithin&&(nMode&FIND_EXACT)))
        {
            nSelection = pSegment->FindFromPosition(dwStop, bWithin);    // Better Coverage
        }
        if ((nSelection==-1)&&(bWithin&&(nMode&FIND_EXACT))&&(pSegment->GetSelection() == -1))
        {
            int nIndex = pMaster->FindStop(dwStop);
            if (nIndex != -1)
            {
                nSelection = pSegment->FindFromPosition(pMaster->GetOffset(nIndex) + pMaster->GetDuration(nIndex)/2, bWithin);
            }
        }
        // change the selection
        if (nSelection!=-1)
        {
            if (nSelection != pSegment->GetSelection())   //Don't deselect
            {
                pView->ChangeAnnotationSelection(pSegment, nSelection, pSegment->GetOffset(nSelection), pSegment->GetStop(nSelection));
            }
        }
        else
        {
            pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
        }
    }
    else
    {
        pView->ChangeAnnotationSelection(pSegment, -1, 0, 0);
    }

    if ((nMaster != nSegmentIndex) && (pSegment->GetSelection() == -1))
    {
        int nSelection = pMaster->FindStop(dwStop); // More precise less robust
        if ((nSelection==-1))   /*&&!(bWithin&&(nMode&FIND_EXACT)))*/
        {
            nSelection = pMaster->FindFromPosition(dwStop, bWithin);
        }
        if (nSelection==-1)
        {
            nSelection = pMaster->FindFromPosition(dwStop, FALSE);
        }

        if (nSelection != -1)   //Set up virtual selection
        {
            // Deselect Virtual Selection
            if (m_Selection.bVirtual)
            {
                RefreshAnnotation(pView, m_Selection.nAnnotationIndex);
            }
            m_Selection.bVirtual = TRUE;
            m_Selection.nAnnotationIndex = nSegmentIndex;
            m_Selection.dwStart = pMaster->GetOffset(nSelection);
            m_Selection.dwDuration = pMaster->GetDuration(nSelection);
            m_Selection.dwStop = m_Selection.dwStart + m_Selection.dwDuration;
            pView->SetStartCursorPosition(m_Selection.dwStart, SNAP_RIGHT);
            pView->SetStopCursorPosition(m_Selection.dwStop, SNAP_LEFT);
        }
        else
        {
            // Deselect Virtual Selection
            if (m_Selection.bVirtual)
            {
                RefreshAnnotation(pView, m_Selection.nAnnotationIndex);
            }
            m_Selection.bVirtual = FALSE;
            m_Selection.nAnnotationIndex = -1;
            m_Selection.dwStart = 0;
            m_Selection.dwDuration = 0;
            m_Selection.dwStop = 0;
        }
    }
    // highlight possible insertion point
    if ((nMaster == nSegmentIndex) && (nMaster != PHONETIC) && (pSegment->GetSelection() == -1))
    {
        DWORD dwStart = 0;

        dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStop, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);

        // Snap Start Position
        if (dwStop > pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME))
        {
            dwStart = dwStop - pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME);
        }
        else
        {
            dwStart = 0;
        }

        if (pDoc->Is16Bit())   // SDM 1.5Test8.2
        {
            dwStart = (dwStart + 1) & ~1; // Round up
        }

        dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, 0, dwStart, SNAP_LEFT);

        int nInsertAt = pSegment->CheckPosition(pDoc,dwStart,dwStop,CSegment::MODE_ADD);
        if (nInsertAt != -1)
        {
            // Deselect Virtual Selection
            if (m_Selection.bVirtual)
            {
                RefreshAnnotation(pView, m_Selection.nAnnotationIndex);
            }

            if (nInsertAt < pSegment->GetOffsetSize())
                if (pSegment->GetOffset(nInsertAt) < (dwStop +  pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)))
                {
                    dwStop = pSegment->GetOffset(nInsertAt);
                }

            if (nInsertAt > 0)
            {
                if ((pSegment->GetStop(nInsertAt-1)+pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < dwStop)
                {
                    dwStart = (dwStop - pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
                }
                else
                {
                    dwStart = pSegment->GetStop(nInsertAt-1);
                }
            }
            else
            {
                if ((dwStart+pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME) + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < pDoc->GetUnprocessedDataSize())
                {
                    dwStart = (dwStop - pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME));
                }
                else
                {
                    dwStart = 0;
                }
            }

            if (pDoc->Is16Bit())
            {
                dwStart = (dwStart + 1) & ~1; // Round up
            }

            dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, dwStart, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);
            dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, 0, dwStop, SNAP_LEFT);

            m_Selection.bVirtual = TRUE;
            m_Selection.nAnnotationIndex = nSegmentIndex;
            m_Selection.dwStart = dwStart;
            m_Selection.dwDuration = dwStop-dwStart;
            m_Selection.dwStop = dwStop;
            pView->SetStartCursorPosition(m_Selection.dwStart, SNAP_RIGHT);
            pView->SetStopCursorPosition(m_Selection.dwStop, SNAP_LEFT);
        }
        else
        {
            // Deselect Virtual Selection
            if (m_Selection.bVirtual)
            {
                RefreshAnnotation(pView, m_Selection.nAnnotationIndex);
            }
            m_Selection.bVirtual = FALSE;
            m_Selection.nAnnotationIndex = -1;
            m_Selection.dwStart = 0;
            m_Selection.dwDuration = 0;
            m_Selection.dwStop = 0;
        }
    }
    Update(pView);
    return !((nAnnotationIndex == m_Selection.nAnnotationIndex)&&(dwStart == m_Selection.dwStart));
}

/***************************************************************************/
// CASegmentSelection::Update update selection data for outside activity
// This keeps the virtual selection up to date with activity which modified
// the segments or their selection
/***************************************************************************/
void CASegmentSelection::Update(CSaView * pView, BOOL bClearVirtual)
{
    if (pView->IsAnyAnnotationSelected())
    {
        // Deselect Virtual Selection
        if (m_Selection.bVirtual)
        {
            RefreshAnnotation(pView, m_Selection.nAnnotationIndex);
        }
        m_Selection.bVirtual = FALSE;
        m_Selection.nAnnotationIndex = pView->FindSelectedAnnotationIndex();

        CSegment * pSegment = pView->GetDocument()->GetSegment(m_Selection.nAnnotationIndex);
        int nSelection = pSegment->GetSelection();

        m_Selection.dwStart = pSegment->GetOffset(nSelection);
        m_Selection.dwDuration = pSegment->GetDuration(nSelection);
        m_Selection.dwStop = m_Selection.dwStart + m_Selection.dwDuration;
    }
    else
    {
        if (bClearVirtual && m_Selection.bVirtual)
        {
            RefreshAnnotation(pView, m_Selection.nAnnotationIndex);
            m_Selection.bVirtual = FALSE;
        }
        if (!m_Selection.bVirtual)
        {
            m_Selection.nAnnotationIndex = -1;
            m_Selection.dwStart = 0;
            m_Selection.dwDuration = 0;
            m_Selection.dwStop = 0;
        }
    }
};

/***************************************************************************/
// CASegmentSelection::RefreshAnnotation
// Redraw annotation losing selection as we find out.
/***************************************************************************/
void CASegmentSelection::RefreshAnnotation(CSaView * pView, int nAnnotationIndex)
{
    if ((nAnnotationIndex < 0)||(nAnnotationIndex >= ANNOT_WND_NUMBER))
    {
        return;
    }

    for (int nGraphLoop = 0; nGraphLoop < MAX_GRAPHS_NUMBER; nGraphLoop++)
    {
        if (pView->m_apGraphs[nGraphLoop])
        {
            pView->m_apGraphs[nGraphLoop]->ChangeAnnotationSelection(nAnnotationIndex);
        }
    }
}


/***************************************************************************/
// CASegmentSelection::GetSelectedAnnotationString
// returns selected annotation string or null string if virtual selection
// optionally removes delimiters
/***************************************************************************/
CString CASegmentSelection::GetSelectedAnnotationString(CSaView * pView, BOOL bRemoveDelimiter) const
{
    CString ret;

    if (m_Selection.bVirtual)
    {
        return "";
    }

    ret = pView->GetSelectedAnnotationString();

    if (bRemoveDelimiter)
    {
        if ((ret[0] == SEGMENT_DEFAULT_CHAR) && (m_Selection.nAnnotationIndex != GLOSS))
        {
            return ret.Mid(1);
        }

        if (((ret[0] == TEXT_DELIMITER)||(ret[0] == WORD_DELIMITER)) && (m_Selection.nAnnotationIndex == GLOSS))
        {
            return ret.Mid(1);
        }
    }

    return ret;
}

/***************************************************************************/
// CASegmentSelection::SetSelectedAnnotationString
// Sets the selected annotation string.  Adds delimiter if necessary,
// removes empty dependent segments.  Refreshes graphs, sets modified flag
// and optionally adds CheckPoint()
/***************************************************************************/
BOOL CASegmentSelection::SetSelectedAnnotationString(CSaView * pView, CSaString & szString, BOOL bIncludesDelimiter, BOOL bCheck)
{
    if (m_Selection.nAnnotationIndex == -1)
    {
        return FALSE;
    }

    int nIndex = m_Selection.nAnnotationIndex;
    CSegment * pSegment = pView->GetAnnotation(nIndex);
    CSaDoc * pDoc = pView->GetDocument();

    // include delimiter;
    if ((!bIncludesDelimiter) && (nIndex == GLOSS))
    {
        CString szDelimiter = GetSelectedAnnotationString(pView, FALSE).Left(1);
        szString = szDelimiter + szString;
    }

    if (szString.GetLength() == 0)
    {
        if (nIndex == GLOSS)
        {
            szString = WORD_DELIMITER;
        }
        else if (nIndex == PHONETIC)
        {
            szString = SEGMENT_DEFAULT_CHAR;
        }
    }

    // Save cursor positions
    DWORD dwStart = pView->GetStartCursorPosition();
    DWORD dwStop = pView->GetStopCursorPosition();

    // Set cursors to segment boundaries to avoid changes
    pView->SetStartCursorPosition(m_Selection.dwStart);
    pView->SetStopCursorPosition(m_Selection.dwStop);

    if (m_Selection.bVirtual)
    {
        if ((nIndex == GLOSS)||(nIndex == PHONETIC))
        {
            return FALSE;
        }

        // Add refreshes graphs, set modified flag, & check point
        if (pSegment->GetMasterIndex() != -1)
        {
            ((CDependentSegment *) pSegment)->Add(pView->GetDocument(), m_Selection.dwStart, szString, FALSE, bCheck);
        }
        else
        {
            int nInsertAt = pSegment->CheckPosition(pDoc,m_Selection.dwStart,m_Selection.dwStop,CSegment::MODE_ADD);
            if (nInsertAt != -1)
            {
                if (bCheck)
                {
                    pDoc->CheckPoint();
                }
                pDoc->SetModifiedFlag(TRUE); // document has been modified
                pDoc->SetTransModifiedFlag(TRUE); // transcription has been modified
                pSegment->Insert(nInsertAt, szString, true, m_Selection.dwStart,m_Selection.dwDuration);
                pView->ChangeAnnotationSelection(pSegment, nInsertAt);
            }
        }
    }
    else if (szString.GetLength() == 0)     // We need to remove the dependent segment
    {
        DWORD dwPosition = m_Selection.dwStart;

        // Remove refreshes graphs, set modified flag, & check point
        pSegment->Remove(pView->GetDocument(), bCheck);
        SelectFromPosition(pView, nIndex, dwPosition, FIND_EXACT);
    }
    else
    {
        if (bCheck)
        {
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

