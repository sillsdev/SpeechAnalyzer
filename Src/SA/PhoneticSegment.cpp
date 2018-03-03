#include "stdafx.h"
#include "PhoneticSegment.h"
#include "Process\sa_p_fra.h"
#include "Process\SA_P_ZCR.H"
#include "Process\SA_P_CHA.H"
#include "Sa_Doc.h"
#include "MainFrm.h"
#include "SA_View.h"
#include "GlossSegment.h"
#include "PhonemicSegment.h"
#include "OrthoSegment.h"
#include "ToneSegment.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CPhoneticSegment
// class to do all the handling for the phonetic annotation segments.

CPhoneticSegment::CPhoneticSegment(EAnnotation index, int master) :
    CIndependentSegment(index,master) {
}

CFontTable * CPhoneticSegment::NewFontTable() const {
    return new CFontTableIPA;
}

/////////////////////////////////////////////////////////////////////////////
// CPhoneticSegment helper functions

/***************************************************************************/
// CPhoneticSegment::Exit Exit segmenting on error
// Standard exit function if an error occurred.
/***************************************************************************/
long CPhoneticSegment::Exit(int nError) {
    EndProcess(); // end data processing
    EndWaitCursor();
    SetDataReady(FALSE);
    return MAKELONG(nError, 100);
}

/***************************************************************************/
// CPhoneticSegment::ReplaceSelectedSegment
// Replaces the selected segment with the string passes in.
/***************************************************************************/
void CPhoneticSegment::ReplaceSelectedSegment(CSaDoc * pDoc, LPCTSTR replace, bool noSnap) {

	if (noSnap) {
		CSegment::ReplaceSelectedSegment( pDoc, replace, noSnap);
		return;
	}

    // get pointer to view
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    // get the new cursor positions
    AdjustCursorsToSnap(pDoc); 
	// snap cursors to appropriate position
    DWORD dwStart = pView->GetStartCursorPosition();
    DWORD dwStop = pView->GetStopCursorPosition();

    DWORD dwOldOffset = GetOffset(m_nSelection);
    RemoveAt(m_nSelection);
    if (dwOldOffset == -1) {
        return;
    }

    pDoc->SetModifiedFlag(TRUE);        // document has been modified
    pDoc->SetTransModifiedFlag(TRUE);   // transcription data has been modified

    // insert or append the new segment
    if (!Insert( m_nSelection, replace, 0, dwStart, dwStop - dwStart)) {
        return; // return on error
    }

    int nSaveSelection = m_nSelection;
    pView->ChangeAnnotationSelection(this, m_nSelection, dwStart, dwStop);      // deselect
    pView->ChangeAnnotationSelection(this, nSaveSelection, dwStart, dwStop);    // select again
    pView->RedrawGraphs(FALSE);													// refresh the graphs between cursors

    // adjust the aligned annotation offsets and durations
    for (int nLoop = 1; nLoop <= GLOSS; nLoop++) {
        //SDM 1.5Test8.1 Segments after gloss dependent on gloss
        CSegment * pSegment = pDoc->GetSegment(nLoop);
        if (pSegment!=NULL) {
            int nIndex = pSegment->FindOffset(dwOldOffset);
            if (nIndex != -1) {
				TRACE("adjust %s %d %d %d %d\n",pSegment->GetRuntimeClass()->m_lpszClassName,nLoop,nIndex,dwStart,(dwStop-dwStart));
                pSegment->Adjust( pDoc, nIndex, dwStart, dwStop - dwStart, false);
            }
        }
    }

	// refresh the entire graph
    pView->RedrawGraphs(TRUE); 
}

/***************************************************************************/
// CPhoneticSegment::Remove Remove annotation segment
// All the dependent segments that are aligned to the removed master
// segment have to be removed too. The user will be informed before.
/***************************************************************************/
void CPhoneticSegment::Remove(CSaDoc * pDoc, int sel, BOOL bCheck) {
	TRACE("Remove\n");
    // save state for undo ability
    if (bCheck) {
        pDoc->CheckPoint();
    }
    DWORD dwOldOffset = GetOffset(sel);
    DWORD dwOldStop = GetStop(sel);
    // handle dependent gloss separately
    CGlossSegment * pGloss = (CGlossSegment *)pDoc->GetSegment(GLOSS);
    if (pGloss != NULL) {
        int gindex = pGloss->FindOffset(dwOldOffset);
        if (gindex != -1) {
            if (pGloss->GetStop(gindex) == dwOldStop) {
                // stop and start match
                pGloss->Remove(pDoc, gindex, FALSE);
            } else {
                // stop doesn't match
                int refcount = GetReferenceCount(pGloss, gindex);
                if (refcount<=1) {
                    // this is the last segment, delete it
                    pGloss->Remove(pDoc, gindex, FALSE);
                } else {
                    // start matches, stop doesn't
                    int next = GetNext(sel);
                    if (next!=-1) {
                        DWORD offset = GetOffset(next);
                        DWORD stop = pGloss->GetStop(gindex);
                        pGloss->Adjust(pDoc, gindex, offset, stop - offset, false);
                    }
                }
            }
        } else {
            // start doesn't match
            // are we deleting the last master segment?
            gindex = pGloss->FindStop(dwOldStop);
            if (gindex != -1) {
                // stop matches
                int refcount = GetReferenceCount(pGloss, gindex);
                if (refcount<=1) {
                    // this is the last segment, delete it
                    pGloss->Remove(pDoc, gindex, FALSE);
                } else {
                    // find the previous phonetic segment
                    // adjust gloss to match the prevous segment.
                    int prev = GetPrevious(sel);
                    if (prev!=-1) {
                        DWORD offset = pGloss->GetOffset(gindex);
                        DWORD stop = GetStop(prev);
                        pGloss->Adjust(pDoc, gindex, offset, stop - offset, false);
                    }
                }
            }
        }
    }

    // delete aligned dependent segments and gloss
    for (int nLoop = 1; nLoop < GLOSS; nLoop++) {
        //SDM 1.5Test8.1 segments after gloss are dependent on gloss
        CSegment * pSegment = pDoc->GetSegment(nLoop);
        if (pSegment == NULL) {
            continue;
        }
        int nIndex = pSegment->FindOffset(dwOldOffset);
        if (nIndex != -1) {
            // start matches
            if (pSegment->GetStop(nIndex) == dwOldStop) {
                // stop and start match
                pSegment->RemoveAt(nIndex);
            } else {
                // start matches, stop doesn't
                DWORD offset = GetOffset(sel);
                DWORD stop = pSegment->GetStop(nIndex);
                pSegment->Adjust(pDoc, nIndex, offset, stop - offset, false);
            }
        } else {
            // start doesn't match
            nIndex = pSegment->FindStop(dwOldStop);
            if (nIndex != -1) {
                DWORD offset = pSegment->GetOffset(nIndex);
                int prev = GetPrevious(sel);
                DWORD stop = GetStop(prev);
                pSegment->Adjust(pDoc, nIndex, offset, stop - offset, false);
            }
        }
    }

    RemoveAt(sel);
    // get pointer to view
    CSaView * pView = pDoc->GetFirstView();
    pDoc->SetModifiedFlag(TRUE);						// document has been modified
    pDoc->SetTransModifiedFlag(TRUE);					// transcription data has been modified
    pView->ChangeAnnotationSelection(this, sel, 0, 0);	// deselect
    pView->RedrawGraphs(TRUE,FALSE);
}

/***************************************************************************/
// CPhoneticSegment::Process Segmenting wave data
// The process stores the resulting segmentation in the phonetic data arrays.
// While processing, a process bar, placed on the status bar, has to be up-
// dated. The level tells which processing level this process has been
// called, start process start on which processing percentage this process
// starts (for the progress bar). The status bar process bar will be updated
// depending on the level and the progress start. The return value returns
// the highest level throughout the calling queue, or -1 in case of an error
// in the lower word of the long value and the end process progress percen-
// tage in the higher word.
/***************************************************************************/
long CPhoneticSegment::Process( void * pCaller, CSaDoc * pDoc, int nProgress, int nLevel) {

    //TRACE(_T("Process: CPhoneticSegment\n"));
    if (IsCanceled()) {
		// process canceled
        return MAKELONG(PROCESS_CANCELED, nProgress);
    }
    if (IsDataReady()) {
		// data is already ready
        return MAKELONG(--nLevel, nProgress);
    }

    // create change and zero crossing data, it is needed to do segmentation
	// get pointer to change object
    CProcessChange * pChange = (CProcessChange *)pDoc->GetChange();
	// process change data
    pChange->Process(pCaller, pDoc, nProgress, ++nLevel);

    // Finish fragmenting as needed.
    CProcessFragments * pFragments = (CProcessFragments *)pDoc->GetFragments();
	// process data
    long lResult = LOWORD(pFragments->Process(this, pDoc));

    // ******** added by CLW 6/98 - 9/24/98 ******************************************
	// get pointer to zero crossing object
    CProcessZCross * pZCross = (CProcessZCross *)pDoc->GetZCross();
	// process zero crossing data
    lResult = pZCross->Process(pCaller, pDoc, nProgress, nLevel);
    // *******************************************************************************

    nLevel = (short int)LOWORD(lResult);
    nProgress = HIWORD(lResult);

	// previous processing error
    if (nLevel < 0) {
        if ((nLevel == PROCESS_CANCELED)) {
			// set your own cancel flag
            CancelProcess();
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start segmenting
	// wait cursor
    BeginWaitCursor(); 
	// memory allocation failed
    if (!StartProcess(pCaller, IDS_STATTXT_SEGMENTING, FALSE)) { 
		// end data processing
        EndProcess(); 
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

	// end of loop
    DWORD dwLoopEnd = pChange->GetDataSize();
    // prepare parameters
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	// get segment parameters // RLJ 1.5Test11.1A
    CSegmentParm * pSegParm = pMainFrame->GetSegmentParm(); 
    // fFactor based on frame-length for accuracy (CLW 1/20/98)
	// size factor
    float fFactor = (float) CALCULATION_INTERVAL(pDoc->GetSamplesPerSec()) * pDoc->GetBlockAlign(); 
    // Flank Width needs to be rounded up to guarantee minimum width (CLW 1/19/98)
	// flank width in process words
    DWORD dwFlankWidth = (DWORD)((pDoc->GetBytesFromTime(pSegParm->fSegmentWidth) + fFactor - 1) / 2 / fFactor); 
    if (!dwFlankWidth) {
        dwFlankWidth = 1;
    }

	// change threshold
    int nChThreshold = (int)((long)pChange->GetMaxValue() * (long)pSegParm->nChThreshold / 100); 
	// zero crossing threshold (CLW)
    int nZCThreshold = pSegParm->nZCThreshold; 
    DWORD dwLoopPos = 0;
    BOOL bRes = TRUE;
    DWORD dwFlankUp = 0;
    DWORD dwFlankDown = 0;
    DWORD dwSegmentStart = 0;
    DWORD dwSegmentStop = 0;
    DWORD dwHillTop = 0;
    // prepare segment data
    int nSegmentIndex = 0;
    // SDM 1.06.4 move question to Command Level
    // remove all annotations and deselect everything
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        CSegment * pSegment = pDoc->GetSegment(nLoop);
        if (pSegment!=NULL) {
            pSegment->DeleteContents();
            if (pSegment->GetSelection() != -1) {
                pSegment->SetSelection(-1);
            }
        }
    }
	// document has been modified
    pDoc->SetModifiedFlag(TRUE);
	// transcription data has been modified
    pDoc->SetTransModifiedFlag(TRUE);
    // invalidate parsing information
    pDoc->GetSegment(GLOSS)->SetDataInvalid();

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
	// start of zero crossing peak
    DWORD dwZCStart = 0;
	// end of zero crossing peak
    DWORD dwZCStop = 0;
	// start of previous peak if it was too short,
    DWORD dwShortZCStart = 0; 
    // otherwise 0
	// end of previous peak if short, otherwise 0
    DWORD dwShortZCStop = 0;
	// start of last valid peak
    DWORD dwOldZCStart = 0;
	// end of last valid peak
    DWORD dwOldZCStop = 0;
    DWORD dwLastSample = pDoc->GetDataSize() - pDoc->GetBlockAlign();
	// candidate for zero crossing peak base
    DWORD dwZBase;
    /*********************************************************************/
    DWORD dwChLoopEnd;

    // start processing
    while (dwLoopPos < dwLoopEnd) {
        /*********************************************************************/
        // Added by CLW 6/98 - 9/25/98
        /*********************************************************************/
        // find zero crossing peak
        while (!dwZCStop) {
            dwZBase = dwLoopPos + 1;
            // read t-2 zero crossing data point
            int nOldZCData_2 =  pZCross->GetProcessedData(dwLoopPos++, &bRes);
            if (!bRes) {
				// error, reading zero crossing data failed
                return Exit(PROCESS_ERROR);
            }
            // read t-1 zero crossing data point
            int nOldZCData_1 =  pZCross->GetProcessedData(dwLoopPos++, &bRes);
            if (!bRes) {
				// error, reading failed
                return Exit(PROCESS_ERROR);
            }
            while ((!dwZCStop) && (dwLoopPos < dwLoopEnd)) {
                // read current data point
                int nZCData =  pZCross->GetProcessedData(dwLoopPos, &bRes);
                if (!bRes) {
					// error, reading failed
                    return Exit(PROCESS_ERROR);
                }
                // check for start of peak
                if (!dwZCStart) {
                    // check for base candidate
                    if ((nOldZCData_2 >= nOldZCData_1) && (nZCData > nOldZCData_1)) {
                        dwZBase = dwLoopPos;
                    }
                    // over threshold?
                    if (nZCData >= nZCThreshold) {
                        dwZCStart = dwZBase;
                    }
                } else { // check for end of peak
                    if (nOldZCData_1 < nZCThreshold) {
                        if ((nOldZCData_2 > nOldZCData_1) && (nOldZCData_1 <= nZCData)) {
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
            if ((!dwZCStop) && (dwLoopPos == dwLoopEnd)) {
				// default to end of file
                dwZCStart = dwZCStop = dwLoopEnd;
            }

            // The next several lines check to see if the current peak or the
            // previous peak is too short. If so, SA attempts to join the two peaks
            // if they are close enough. If the current peak is too short, the search
            // continues, whether it was joined or not, since there will be no room
            // between two joined peaks for Change to find any segments.

            // is the previous peak short?
            if (dwShortZCStart) {
                // is the previous peak adjacent?
                if (dwZCStart - dwOldZCStop < 4 * dwFlankWidth) {
                    // join the two peaks
                    dwZCStart = dwOldZCStart;
                }
				// reset short segment start and end
                dwShortZCStart = dwShortZCStop = 0;  
            }

            // is the current peak (still) short?
            if ((dwZCStop - dwZCStart < 2 * dwFlankWidth) && (dwZCStop < dwLoopEnd)) {
                // save this segment in case the next ZC segment is adjacent
                dwShortZCStart = dwZCStart;
                dwShortZCStop = dwZCStop;
                // adjacent to old segment? and was there an old segment?
                if ((dwOldZCStart) && (dwZCStart - dwOldZCStop < 4 * dwFlankWidth)) {
                    // join the two peaks (adjust most recently stored segment)
					// rewind start of current segment to start of old
                    dwZCStart = dwOldZCStart; 
					// fast forward end of old segment to end of current
                    dwOldZCStop = dwZCStop; 
                    dwSegmentStart = (DWORD)(((float)dwZCStart + 0.5F) * fFactor); 
                    dwSegmentStop = (DWORD)(((float)dwZCStop + 0.5F) * fFactor); 
					// store it
                    Adjust(pDoc, nSegmentIndex - 1, dwSegmentStart, dwSegmentStop - dwSegmentStart, false); 
                    dwSegmentStart = dwSegmentStop;
                    dwSegmentStop = 0;
					// reset short segment start and end
                    dwShortZCStart = dwShortZCStop = 0; 

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
		// read change data point
        int nOldChData = pChange->GetProcessedData(dwLoopPos++, &bRes); 
        if (!bRes) {
			// error, reading change data failed
            return Exit(PROCESS_ERROR);    
        }
        while ((dwLoopPos < dwChLoopEnd) && (dwZCStart - dwOldZCStop > 4 * dwFlankWidth)) {
			// read change data point
            int nChangeData = pChange->GetProcessedData(dwLoopPos, &bRes); 
            if (!bRes) {
				// error, reading change data failed
                return Exit(PROCESS_ERROR);    
            }
			// point is over threshold
            if (nChangeData > nChThreshold) { 
				// rising flank
                if (nOldChData < nChangeData) { 
                    if (dwFlankDown) {
                        // there was a falling flank before new hill
                        dwFlankUp = dwFlankDown = 0; 
                        dwHillTop = 0;
                    }
                    dwFlankUp++;
                } else {
					// falling flank
                    if (nOldChData > nChangeData) { 
                        if (!dwHillTop) {
                            dwHillTop = dwLoopPos - 1;
                        }
                        dwFlankDown++;
                        if ((dwFlankUp >= dwFlankWidth) && (dwFlankDown >= dwFlankWidth)) {
                            // segment start or stop found
                            if ((!dwSegmentStart) && (nSegmentIndex > 0)) { 
                                dwSegmentStart = (DWORD)((float)dwHillTop * fFactor);    
                            } else { 
								// start already found before
                                if (!dwSegmentStop) {
                                    dwSegmentStop = (DWORD)((float)dwHillTop * fFactor);    
                                }
                                // ready to store segment
                                if ((dwSegmentStart >= dwSegmentStop) || (dwSegmentStop > dwLastSample)) {
                                    break;
                                }
								AddAt( pDoc, nSegmentIndex++, dwSegmentStart, dwSegmentStop - dwSegmentStart);
                                dwSegmentStart = dwSegmentStop;
                                dwSegmentStop = 0;
								// document has been modified
                                pDoc->SetModifiedFlag(TRUE); 
								// transcription data has been modified
                                pDoc->SetTransModifiedFlag(TRUE); 
                            }
							// look for new hill
                            dwFlankUp = dwFlankDown = dwHillTop = 0; 
                        }
                    }
                }
            } else {
                dwFlankUp = 0;
            }
            nOldChData = nChangeData;
            dwLoopPos++;
        }
		// look for new hill
        dwFlankUp = dwFlankDown = dwHillTop = 0; 

        // Store Zero Crossing segment
        if (dwZCStart == dwLoopEnd) {
            break;
        }
        {
            // Figure out start and stop
            if ((!dwSegmentStart) && (nSegmentIndex)) { 
                dwSegmentStart = (DWORD)(((float)dwZCStart + 0.5F) * fFactor);    
            } else { 
				// start already found before
                dwSegmentStop = (DWORD)(((float)dwZCStart + 0.5F) * fFactor); 
                if (dwSegmentStop - dwSegmentStart > 2 * dwFlankWidth) {
                    // store previous segment
                    if (dwSegmentStart >= dwSegmentStop || dwSegmentStop > dwLastSample) {
                        break;
                    }
					AddAt( pDoc, nSegmentIndex++, dwSegmentStart, dwSegmentStop - dwSegmentStart);
                    dwSegmentStart = dwSegmentStop;
                }
                dwSegmentStop = 0;
            }
            if (!dwSegmentStop) {
                dwSegmentStop = (DWORD)((float)dwZCStop * fFactor);
            }
            {
                // Store new zero crossing segment
                if (dwSegmentStart >= dwSegmentStop || dwSegmentStop > dwLastSample) {
                    break;
                }
                AddAt( pDoc, nSegmentIndex++, dwSegmentStart, dwSegmentStop - dwSegmentStart);
                dwSegmentStart = dwSegmentStop;
                dwSegmentStop = 0;
				// document has been modified
                pDoc->SetModifiedFlag(TRUE); 
				// transcription data has been modified
                pDoc->SetTransModifiedFlag(TRUE); 
            }
        }
        dwLoopPos = dwZCStop - 1;
        dwOldZCStart = dwZCStart;
        dwOldZCStop = dwZCStop;
        dwZCStop = dwZCStart = 0;
        /*********************************************************************/

        // set progress bar
        SetProgress(nProgress + (int)(100 * dwLoopPos / dwLoopEnd / (DWORD)nLevel));
        if (IsCanceled()) {
			// process canceled
            return Exit(PROCESS_CANCELED);    
        }
    }
    // Add segment for silence at end
    DWORD dwLastStop = GetStop(nSegmentIndex - 1);
    if (dwLastStop < dwLastSample) {
        // store silence segment
        dwSegmentStart = dwLastStop;
        dwSegmentStop = dwLastSample;
        AddAt( pDoc, nSegmentIndex++, dwSegmentStart, dwSegmentStop - dwSegmentStart);
    }

    pDoc->AutoSnapUpdate();

    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
	// end data processing
    EndProcess((nProgress >= 95)); 
    EndWaitCursor();
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

bool CPhoneticSegment::Filter() {
    bool changed = false;
    for (int i=0; i<m_Text.GetCount(); i++) {
        changed |= Filter(m_Text[i]);
    }
    return changed;
}

bool CPhoneticSegment::Filter(CString & text) {
	// Box Character
    TCHAR cIPASpaceReplace = 0xFFFD;        
    bool bChanged = false;
    for (int i=0; i<text.GetLength(); i++) {
        if (text[i]==0) {
            break;
        }
        if (text[i] < 0x20) {
            text.SetAt(i, cIPASpaceReplace);
            bChanged = true;
        }
    }
    return bChanged;
}

bool CPhoneticSegment::ContainsText( DWORD offset, DWORD stop) {
	for (int i=0;i<GetOffsetSize();i++) {
		DWORD thisOffset = GetOffset(i);
		DWORD thisStop = GetStop(i);
		if ((thisOffset>=offset)&&(thisStop<=stop)) {
			CString text = GetText(i).Trim();
			if (text.GetLength()==0) continue;
			if ((text.GetLength()==1) && (text[0]==0xFFFD)) continue;
			TRACE("text %s is blocking operation for %d\n",(LPCTSTR)text,this->m_nAnnotationType);
			return true;
		}
	}
	return false;
}

int CPhoneticSegment::Add( CSaDoc * pDoc, DWORD offset, DWORD duration) {
	int index = CIndependentSegment::Add( offset, duration);
	CPhonemicSegment * pPhonemic = (CPhonemicSegment*)pDoc->GetSegment(PHONEMIC);
	pPhonemic->Add( offset, duration);
	COrthographicSegment * pOrtho = (COrthographicSegment*)pDoc->GetSegment(ORTHO);
	pOrtho->Add( offset, duration);
	CToneSegment * pTone = (CToneSegment*)pDoc->GetSegment(TONE);
	pTone->Add( offset, duration);
	return index;
}

void CPhoneticSegment::AddAt( CSaDoc * pDoc, int index, DWORD offset, DWORD duration) {
	InsertAt( index, offset, duration);
	CPhonemicSegment * pPhonemic = (CPhonemicSegment*)pDoc->GetSegment(PHONEMIC);
	pPhonemic->InsertAt( index, offset, duration);
	COrthographicSegment * pOrtho = (COrthographicSegment*)pDoc->GetSegment(ORTHO);
	pOrtho->InsertAt( index, offset, duration);
	CToneSegment * pTone = (CToneSegment*)pDoc->GetSegment(TONE);
	pTone->InsertAt( index, offset, duration);
}
