#include "stdafx.h"
#include "GlossSegment.h"
#include "Sa_Doc.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_fra.h"
#include "mainfrm.h"
#include "sa_asert.h"
#include "PhoneticSegment.h"
#include "TextSegment.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CGlossSegment
// class to do all the handling for the gloss annotation segments.

CGlossSegment::CGlossSegment(int index, int master) : 
CTextSegment(index,master),
MARGIN(20)
{
}

CGlossSegment::~CGlossSegment()
{
}

CFontTable * CGlossSegment::NewFontTable() const
{
    return new CFontTableANSI();
}

/***************************************************************************/
// CGlossSegment::ReplaceSelectedSegment
// Replaces the selected segment with the string passes in.
/***************************************************************************/
void CGlossSegment::ReplaceSelectedSegment(CDocument * pSaDoc, const CSaString & str)
{
    CSaString POS;
    if (m_nSelection != -1)
    {
        POS = pos.GetAt(m_nSelection); //SDM 1.5Test8.2
    }
    CTextSegment::ReplaceSelectedSegment(pSaDoc, str);
    pos.SetAt(m_nSelection, POS); // SDM 1.5Test8.2

}

//SDM 1.5Test8.1
/***************************************************************************/
// CGlossSegment::Remove Remove annotation segment
// All the dependent segments that are aligned to the removed gloss
// segment have to be removed too. The user will be informed before.
/***************************************************************************/
void CGlossSegment::Remove(CDocument * pSaDoc, BOOL bCheck)
{

    CSaDoc * pDoc = (CSaDoc *) pSaDoc;

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
        CSegment * pSegment = pDoc->GetSegment(nLoop);
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
DWORD CGlossSegment::RemoveNoRefresh(CDocument * pDoc)
{
    // change the segment arrays
    pos.RemoveAt(m_nSelection, 1);
    return CTextSegment::RemoveNoRefresh(pDoc);
}

//SDM 1.5Test8.1
/***************************************************************************/
// CGlossSegment::DeleteContents Delete all contents of the gloss arrays
/***************************************************************************/
void CGlossSegment::DeleteContents()
{
    pos.RemoveAll();
    CTextSegment::DeleteContents(); // call the base class to delete positions
}

//SDM 1.5Test8.1
/***************************************************************************/
// CGlossSegment::Insert Insert/append a gloss segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CGlossSegment::SetAt(const CSaString * pszString, bool delimiter, DWORD dwStart, DWORD dwDuration)
{

    int nIndex = FindOffset(dwStart);
    ASSERT(nIndex>=0);
    try
    {
        pos.SetAtGrow(nIndex, NULL);
    }
    catch (CMemoryException e)
    {
        // memory allocation error
        ErrorMessage(IDS_ERROR_MEMALLOC);
        return FALSE;
    }
    return CTextSegment::SetAt(pszString, delimiter, dwStart, dwDuration);
}

//SDM 1.5Test8.1
/***************************************************************************/
// CGlossSegment::Insert Insert/append a gloss segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CGlossSegment::Insert(int nIndex, LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration)
{

    try
    {
        pos.InsertAt(nIndex, NULL,  1);
    }
    catch (CMemoryException e)
    {
        // memory allocation error
        ErrorMessage(IDS_ERROR_MEMALLOC);
        return FALSE;
    }
    return CTextSegment::Insert(nIndex, pszString, delimiter, dwStart, dwDuration);
}

//SDM 1.5Test8.1
void CGlossSegment::Serialize(CArchive & ar)
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
    pos.Serialize(ar);
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
long CGlossSegment::Process(void * pCaller, CSaDoc * pSaDoc, int nProgress, int nLevel)
{

    //TRACE(_T("Process: CGlossSegment\n"));
    if (IsCanceled())
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    
	if (IsDataReady())
    {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }

    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer

    // Finish fragmenting as needed.
    CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments();
    long lResult = LOWORD(pFragments->Process(this, pDoc)); // process data

    // create zero crossing data, it is needed to do parsing
    CProcessLoudness * pLoudness = (CProcessLoudness *)pDoc->GetLoudness(); // get pointer to zero crossing object
    lResult = pLoudness->Process(pCaller, pDoc, nProgress, ++nLevel); // process data
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

    // start parsing
    BeginWaitCursor();
    if (!StartProcess(pCaller, IDS_STATTXT_PARSING, FALSE))		// memory allocation failed or previous processing error
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    DWORD dwLoopEnd = pLoudness->GetDataSize();					// end of loop
    // prepare parameters
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	// get parsing parameters
    CParseParm * pCParseParm = pMainFrame->GetCParseParm(); 
    float fFactor = (float)pDoc->GetUnprocessedDataSize() / (float)dwLoopEnd;	// size factor

	// since detection is always too late or too early, we will add
	// and subtract a 0.1 margin from the beginning and end
	// calculate offset for 0.1 second prefix/postfix
	DWORD dwMargin = (DWORD)((pDoc->GetSamplesPerSec()*MARGIN)/1000);

    DWORD dwBreakWidth = (DWORD)(pDoc->GetBytesFromTime( pCParseParm->fBreakWidth) / fFactor); // break width in process words
    if (!dwBreakWidth)
    {
        dwBreakWidth = 1;
    }

    // CLW 1.07a
    int nMaxThreshold = (int)((long)pLoudness->GetMaxValue() * (long)pCParseParm->nMaxThreshold / 100); // threshold
    int nMinThreshold = (int)((long)pLoudness->GetMaxValue() * (long)pCParseParm->nMinThreshold / 100); // threshold
    DWORD dwLoopPos = 0;
    BOOL bRes = TRUE;

    int nBlockAlign = pDoc->GetBlockAlign();

    // prepare gloss data
    int nGlossIndex = 0;

    // remove all gloss and deselect gloss
    DeleteContents();
    if (GetSelection() != -1)
    {
        SetSelection(-1);
    }
    pDoc->SetModifiedFlag(TRUE);        // document has been modified
    pDoc->SetTransModifiedFlag(TRUE);   // transcription data has been modified

    DWORD dwBreakCount = dwBreakWidth;  // allow early word SDM 1.5Test 11.0
    DWORD dwBreakStart = 0;             // the start of the break
    DWORD dwBreakEnd = 0;               // the end of the break

    CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pDoc->GetSegment(PHONETIC); // get pointer to phonetic segment
    pDoc->AutoSnapUpdate(); // Snap phonetic SDM 1.5Test11.0

    // start processing
    while (dwLoopPos < dwLoopEnd)
    {
        // CLW 1.07a
        int nLoudnessData = pLoudness->GetProcessedData(dwLoopPos++, &bRes);		// read zero crossing data point
        if (!bRes)
        {
            return Exit(PROCESS_ERROR);			// error, reading zero crossing data failed
        }
        if (nLoudnessData > nMaxThreshold)		// point is over max threshold     // CLW 1.07a
        {
            if (dwBreakCount >= dwBreakWidth)
            {
                // ready to store gloss
                DWORD dwGlossStart = nBlockAlign * (DWORD)(dwBreakEnd * fFactor/nBlockAlign);

				// subtract 0.1 seconds from the detected start
				//dwGlossStart = (dwGlossStart<dwMargin) ? 0 : (dwGlossStart-dwMargin);

                dwGlossStart = pDoc->SnapCursor(START_CURSOR, dwGlossStart,0,dwGlossStart,SNAP_LEFT);

                // check if the start is within a phonetic segment
				// NOTE: if we are using this task within the context of 
				// the 'parse words' feature, than all previous phonetic and gloss segments
				// have already been deleted.
                int nPhonetic = pPhonetic->FindFromPosition(dwGlossStart, TRUE);
                // adjust to phonetic
                if (nPhonetic != -1)
                {
                    if ((pPhonetic->GetNext(nPhonetic) != -1) &&
                        ((pPhonetic->GetOffset(pPhonetic->GetNext(nPhonetic)) - dwGlossStart) < (dwGlossStart - pPhonetic->GetOffset(nPhonetic))))
                    {
                        // next segment is closer SDM 1.5Test11.0
                        dwGlossStart = pPhonetic->GetOffset(pPhonetic->GetNext(nPhonetic));
                    }
                    else
                    {
                        dwGlossStart = pPhonetic->GetOffset(nPhonetic);
                    }
                }
                else
                {
                    nPhonetic = pPhonetic->FindFromPosition(dwGlossStart);
                    if ((nPhonetic != -1) &&
                        ((dwGlossStart - pPhonetic->GetOffset(nPhonetic)) < pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)))
                    {
                        // Too close to existing segment SDM 1.5Test11.0
                        dwGlossStart = pPhonetic->GetOffset(nPhonetic);
                    }
                    else
                    {
                        // there is no phonetic segment, create one
                        if (nPhonetic == -1)
                        {
                            nPhonetic = 0; // no phonetic yet
                        }
                        else
                        {
                            if ( dwGlossStart > pPhonetic->GetOffset(nPhonetic))
                            {
                                nPhonetic++;    // append at end
                            }
                        }

                        // SDM 1.06.3a Insert 0 duration calculate later
                        CSaString szSegment = SEGMENT_DEFAULT_CHAR;
                        TRACE("phonetic start = %d\n", dwGlossStart);
                        bRes = pPhonetic->Insert( nPhonetic, szSegment, 0, dwGlossStart, 0);
                        if (!bRes)
                        {
                            return Exit(PROCESS_ERROR); // error, writing segment failed
                        }
                    }
                }

                // write gloss
                TRACE("gloss start = %d\n",dwGlossStart);
                bRes = Insert( nGlossIndex++, NULL, 0, dwGlossStart, 0);
                if (!bRes)
                {
                    return Exit(PROCESS_ERROR); // error, writing gloss failed
                }

                // adjust previous phonetic SDM 1.5Test11.0
				DWORD dwOffset = pPhonetic->FindOffset( dwGlossStart);
                nPhonetic = pPhonetic->GetPrevious( dwOffset);
                if ((nPhonetic != -1) && (pPhonetic->GetDuration(nPhonetic)==0))
                {
					
                    DWORD dwPhoneticStop = nBlockAlign * (DWORD)(dwBreakStart * fFactor/nBlockAlign);
					//dwPhoneticStop = ((dwPhoneticStop+dwMargin) < pDoc->GetUnprocessedDataSize()) ? (dwPhoneticStop+dwMargin) : pDoc->GetUnprocessedDataSize(); 
                    dwPhoneticStop = pDoc->SnapCursor( STOP_CURSOR, dwPhoneticStop, dwPhoneticStop, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);
                    if (dwPhoneticStop > pPhonetic->GetOffset(nPhonetic))
                    {
                        pPhonetic->Adjust(pDoc, nPhonetic, pPhonetic->GetOffset(nPhonetic), dwPhoneticStop - pPhonetic->GetOffset(nPhonetic));
                    }
                }

                pDoc->SetModifiedFlag(TRUE); // document has been modified
                pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
            }
            dwBreakCount = 0; // end of valley
        }
        else
        {
            // CLW 1.07a
            if (nLoudnessData <= nMinThreshold)    // point is under min threshold
            {
                dwBreakEnd = dwLoopPos;
                if (dwBreakCount == 0)
                {
                    dwBreakStart = dwLoopPos - 1;// SDM 1.5 Test 11.0
                }
                dwBreakCount++;
            }
            else if (dwBreakCount > 0)     // SDM 1.5 Test 11.0
            {
                dwBreakCount++;
            }
        }

        // set progress bar
        SetProgress(nProgress + (int)(100 * dwLoopPos / dwLoopEnd / (DWORD)nLevel));
        if (IsCanceled())
        {
            return Exit(PROCESS_CANCELED); // process canceled
        }
    }

    //adjust last phonetic SDM 1.5Test11.0
    int nPhonetic = pPhonetic->GetPrevious(-1);
    if ((nPhonetic != -1) && (pPhonetic->GetDuration(nPhonetic)==0))
    {
        DWORD dwPhoneticStop = nBlockAlign*(DWORD)(dwBreakStart * fFactor/nBlockAlign);
        dwPhoneticStop = pDoc->SnapCursor(STOP_CURSOR, dwPhoneticStop,dwPhoneticStop,pDoc->GetUnprocessedDataSize(),SNAP_RIGHT);
        if (dwPhoneticStop > pPhonetic->GetOffset(nPhonetic))
        {
            pPhonetic->Adjust(pDoc,nPhonetic, pPhonetic->GetOffset(nPhonetic),dwPhoneticStop - pPhonetic->GetOffset(nPhonetic));
        }
    }

    // Adjust durations of inserted phonetic segments
    if (!pPhonetic->IsEmpty())
    {
        int nPhonetic = 0;
        CSaString szSegment = SEGMENT_DEFAULT_CHAR;
        while (nPhonetic != -1)
        {
            int nNext = pPhonetic->GetNext(nPhonetic);
            if ((pPhonetic->GetDuration(nPhonetic)==0) && (pPhonetic->GetSegmentString(nPhonetic) == szSegment))
            {
                //Inserted phonetic segment
                if (nNext != -1)
                {
                    pPhonetic->Adjust(pDoc, nPhonetic, pPhonetic->GetOffset(nPhonetic),pPhonetic->GetOffset(nNext) - pPhonetic->GetOffset(nPhonetic));
                }
                else
                {
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

void CGlossSegment::CorrectGlossDurations(CSaDoc * pDoc)
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

CString CGlossSegment::GetPOSAt( int index)
{
	return pos.GetAt( index);
}

void CGlossSegment::POSSetAt( int index, LPCTSTR val)
{
	pos.SetAt( index, val);
}

void CGlossSegment::POSInsertAt( int index, LPCTSTR val)
{
	pos.InsertAt( index, val);
}

void CGlossSegment::POSSetAtGrow( int index, LPCTSTR val)
{
	pos.SetAtGrow( index, val);
}

