#include "stdafx.h"
#include "PhoneticSegment.h"
#include "Process\sa_p_fra.h"
#include "Process\SA_P_ZCR.H"
#include "Process\SA_P_CHA.H"
#include "Sa_Doc.h"
#include "MainFrm.h"
#include "SA_View.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPhoneticSegment
// class to do all the handling for the phonetic annotation segments.

CPhoneticSegment::CPhoneticSegment(int index, int master) :
    CIndependentSegment(index,master)
{
}

CSegment::TpInputFilterProc CPhoneticSegment::GetInputFilter(void) const
{
    return gIPAInputFilter;
}

CFontTable * CPhoneticSegment::NewFontTable() const
{
    return new CFontTableIPA;
}

/////////////////////////////////////////////////////////////////////////////
// CPhoneticSegment helper functions

/***************************************************************************/
// CPhoneticSegment::Exit Exit segmenting on error
// Standard exit function if an error occurred.
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
void CPhoneticSegment::ReplaceSelectedSegment(CDocument * pSaDoc, const CSaString & str)
{
    // get pointer to view
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

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
    if (!Insert(m_nSelection, str, 0, dwStart, dwStop - dwStart))
    {
        return; // return on error
    }

    int nSaveSelection = m_nSelection;
    pView->ChangeAnnotationSelection(this, m_nSelection, dwStart, dwStop); // deselect
    pView->ChangeAnnotationSelection(this, nSaveSelection, dwStart, dwStop); // select again
    pView->RefreshGraphs(FALSE); // refresh the graphs between cursors

    // adjust the aligned annotation offsets and durations
    for (int nLoop = 1; nLoop <= GLOSS; nLoop++)   //SDM 1.5Test8.1 Segments after gloss dependent on gloss
    {
        CSegment * pSegment = pDoc->GetSegment(nLoop);
        if (pSegment)
        {
            int nIndex = pSegment->FindOffset(dwOldOffset);
            if (nIndex != -1)
            {
                pSegment->Adjust(pDoc, nIndex, dwStart, dwStop - dwStart);
            }
        }
    }
}

/***************************************************************************/
// CPhoneticSegment::Remove Remove annotation segment
// All the dependent segments that are aligned to the removed master
// segment have to be removed too. The user will be informed before.
/***************************************************************************/
void CPhoneticSegment::Remove(CDocument * pDoc, BOOL bCheck)
{
    // get pointer to view
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);
    CSaDoc * pSaDoc = (CSaDoc *)pDoc;

    // save state for undo ability
    if (bCheck)
    {
        pSaDoc->CheckPoint();
    }
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
    for (int nLoop = 1; nLoop <= GLOSS; nLoop++)   //SDM 1.5Test8.1 segments after gloss are dependent on gloss
    {
        CSegment * pSegment = pSaDoc->GetSegment(nLoop);
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
                    pSegment->Adjust(pSaDoc, nIndex, dwOffset, pSegment->GetStop(nIndex) - dwOffset);
                }
            }
            nIndex = pSegment->FindStop(dwOldStop);
            if (nIndex != -1)
            {
                DWORD dwStop = GetStop(GetPrevious(nSelection));
                pSegment->Adjust(pSaDoc, nIndex, pSegment->GetOffset(nIndex), dwStop - pSegment->GetOffset(nIndex));
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
DWORD CPhoneticSegment::RemoveNoRefresh(CDocument *)
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
// the highest level throughout the calling queue, or -1 in case of an error
// in the lower word of the long value and the end process progress percen-
// tage in the higher word.
/***************************************************************************/
long CPhoneticSegment::Process(void * pCaller, CSaDoc * pDoc, int nProgress, int nLevel)
{
    TRACE(_T("Process: CPhoneticSegment\n"));
    if (IsCanceled())
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady())
    {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }

    // create change and zero crossing data, it is needed to do segmentation
    CProcessChange * pChange = (CProcessChange *)pDoc->GetChange(); // get pointer to change object
    pChange->Process(pCaller, pDoc, nProgress, ++nLevel); // process change data

    // Finish fragmenting as needed.
    CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments();
    long lResult = LOWORD(pFragments->Process(this, pDoc)); // process data

    // ******** added by CLW 6/98 - 9/24/98 ******************************************
    CProcessZCross * pZCross = (CProcessZCross *)pDoc->GetZCross(); // get pointer to zero crossing object
    lResult = pZCross->Process(pCaller, pDoc, nProgress, nLevel); // process zero crossing data
    // *******************************************************************************

    nLevel = (short int)LOWORD(lResult);
    nProgress = HIWORD(lResult);

    if (nLevel < 0)   // previous processing error
    {
        if ((nLevel == PROCESS_CANCELED))
        {
            CancelProcess(); // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start segmenting
    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_SEGMENTING, FALSE))   // memory allocation failed
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    DWORD dwLoopEnd = pChange->GetDataSize();; // end of loop
    // prepare parameters
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

    SegmentParm * pSegParm = pMainFrame->GetSegmentParm(); // get segment parameters // RLJ 1.5Test11.1A
    // float fFactor = (float)pDoc->GetUnprocessedDataSize() / (float)dwLoopEnd; // size factor
    // fFactor based on frame-length for accuracy (CLW 1/20/98)
    FmtParm * pFmtParm = pDoc->GetFmtParm();
    float fFactor = (float) CALCULATION_INTERVAL(pFmtParm->dwSamplesPerSec) * pFmtParm->wBlockAlign; // size factor
    // Flank Width needs to be rounded up to guarantee minimum width (CLW 1/19/98)
    DWORD dwFlankWidth = (DWORD)((pDoc->GetBytesFromTime(pSegParm->fSegmentWidth) + fFactor - 1) / 2 / fFactor); // flank width in process words
    // DWORD dwFlankWidth = (DWORD)(pDoc->GetBytesFromTime(pSegParm->fSegmentWidth) / 2 / fFactor); // CLW 10/12/98
    if (!dwFlankWidth)
    {
        dwFlankWidth = 1;
    }
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
                if (pDoc->GetSegment(nLoop)->GetSelection() != -1)
                {
                    pDoc->GetSegment(nLoop)->SetSelection(-1);
                }
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
            if (!bRes)
            {
                return Exit(PROCESS_ERROR);    // error, reading zero crossing data failed
            }
            // read t-1 zero crossing data point
            int nOldZCData_1 =  pZCross->GetProcessedData(dwLoopPos++, &bRes);
            if (!bRes)
            {
                return Exit(PROCESS_ERROR);    // error, reading failed
            }
            while ((!dwZCStop) && (dwLoopPos < dwLoopEnd))
            {
                // read current data point
                int nZCData =  pZCross->GetProcessedData(dwLoopPos, &bRes);
                if (!bRes)
                {
                    return Exit(PROCESS_ERROR);    // error, reading failed
                }
                // check for start of peak
                if (!dwZCStart)
                {
                    // check for base candidate
                    if ((nOldZCData_2 >= nOldZCData_1) && (nZCData > nOldZCData_1))
                    {
                        dwZBase = dwLoopPos;
                    }
                    // over threshold?
                    if (nZCData >= nZCThreshold)
                    {
                        dwZCStart = dwZBase;
                    }
                }
                else     // check for end of peak
                {
                    if (nOldZCData_1 < nZCThreshold)
                    {
                        if ((nOldZCData_2 > nOldZCData_1) && (nOldZCData_1 <= nZCData))
                        {
                            dwZCStop = dwLoopPos - 1;
                        }
                    }
                }
                // shift 2 most recent points into history
                nOldZCData_2 = nOldZCData_1;
                nOldZCData_1 = nZCData;
                dwLoopPos++;
            }
            // no ZCEnd, dwLoopPos = end of data (ZC peak was cut off at end of data)
            if ((!dwZCStop) && (dwLoopPos == dwLoopEnd))
            {
                dwZCStart = dwZCStop = dwLoopEnd;    // default to end of file
            }

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
        if (!bRes)
        {
            return Exit(PROCESS_ERROR);    // error, reading change data failed
        }
        while ((dwLoopPos < dwChLoopEnd) && (dwZCStart - dwOldZCStop > 4 * dwFlankWidth))
        {
            int nChangeData = pChange->GetProcessedData(dwLoopPos, &bRes); // read change data point
            if (!bRes)
            {
                return Exit(PROCESS_ERROR);    // error, reading change data failed
            }
            if (nChangeData > nChThreshold)   // point is over threshold
            {
                if (nOldChData < nChangeData)   // rising flank
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
                    if (nOldChData > nChangeData)   // falling flank
                    {
                        if (!dwHillTop)
                        {
                            dwHillTop = dwLoopPos - 1;
                        }
                        dwFlankDown++;
                        if ((dwFlankUp >= dwFlankWidth) && (dwFlankDown >= dwFlankWidth))
                        {
                            // segment start or stop found
                            if ((!dwSegmentStart) && (nSegmentIndex > 0))   // CLW mod 1/19/98
                            {
                                dwSegmentStart = (DWORD)((float)dwHillTop * fFactor);    // CLW 1/20/98
                            }
                            else     // start already found before
                            {
                                if (!dwSegmentStop)
                                {
                                    dwSegmentStop = (DWORD)((float)dwHillTop * fFactor);    // CLW 1/20/98
                                }
                                // ready to store segment
                                CSaString szSegment = SEGMENT_DEFAULT_CHAR;
                                if (dwSegmentStart >= dwSegmentStop || dwSegmentStop > dwLastSample)
                                {
                                    break;
                                }
                                bRes = Insert(nSegmentIndex++, szSegment, 0, dwSegmentStart, dwSegmentStop - dwSegmentStart);
                                if (!bRes)
                                {
                                    return Exit(PROCESS_ERROR);    // error, writing segment failed
                                }
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
            else
            {
                dwFlankUp = 0;
            }
            nOldChData = nChangeData;
            dwLoopPos++;
        }
        dwFlankUp = dwFlankDown = dwHillTop = 0; // look for new hill

        /*********************************************************************/
        // Added by CLW 6/98 - 9/25/98
        /*********************************************************************/
        // Store Zero Crossing segment
        if (dwZCStart == dwLoopEnd)
        {
            break;
        }
        {
            // Figure out start and stop
            if ((!dwSegmentStart) && (nSegmentIndex))   // CLW mod 1/19/98
            {
                dwSegmentStart = (DWORD)(((float)dwZCStart + 0.5F) * fFactor);    // CLW 1/20/98
            }
            else     // start already found before
            {
                dwSegmentStop = (DWORD)(((float)dwZCStart + 0.5F) * fFactor); // CLW 1/20/98
                if (dwSegmentStop - dwSegmentStart > 2 * dwFlankWidth)
                {
                    // store previous segment
                    CSaString szSegment = SEGMENT_DEFAULT_CHAR;
                    if (dwSegmentStart >= dwSegmentStop || dwSegmentStop > dwLastSample)
                    {
                        break;
                    }
                    bRes = Insert(nSegmentIndex++, szSegment, 0, dwSegmentStart, dwSegmentStop - dwSegmentStart);
                    if (!bRes)
                    {
                        return Exit(PROCESS_ERROR);    // error, writing segment failed
                    }
                    dwSegmentStart = dwSegmentStop;
                }
                dwSegmentStop = 0;
            }
            if (!dwSegmentStop)
            {
                dwSegmentStop = (DWORD)((float)dwZCStop * fFactor);    // CLW 1/20/98
            }
            {
                // Store new zero crossing segment
                CSaString szSegment = SEGMENT_DEFAULT_CHAR;
                if (dwSegmentStart >= dwSegmentStop || dwSegmentStop > dwLastSample)
                {
                    break;
                }
                bRes = Insert(nSegmentIndex++, szSegment, 0, dwSegmentStart, dwSegmentStop - dwSegmentStart);
                if (!bRes)
                {
                    return Exit(PROCESS_ERROR);    // error, writing segment failed
                }
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
        if (IsCanceled())
        {
            return Exit(PROCESS_CANCELED);    // process canceled
        }
    }
    // Add segment for silence at end
    DWORD dwLastStop = GetStop(nSegmentIndex - 1);
    if (dwLastStop < dwLastSample)
    {
        // store silence segment
        dwSegmentStart = dwLastStop;
        dwSegmentStop = dwLastSample;
        CSaString szSegment = SEGMENT_DEFAULT_CHAR;
        bRes = Insert(nSegmentIndex++, szSegment, 0, dwSegmentStart, dwSegmentStop - dwSegmentStart);
        if (!bRes)
        {
            return Exit(PROCESS_ERROR);    // error, writing segment failed
        }
    }

    pDoc->AutoSnapUpdate();

    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    EndProcess((nProgress >= 95)); // end data processing
    EndWaitCursor();
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}


