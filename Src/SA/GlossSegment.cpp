#include "stdafx.h"
#include "GlossSegment.h"
#include "Sa_Doc.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_fra.h"
#include "mainfrm.h"
#include "sa_asert.h"
#include "PhoneticSegment.h"
#include "TextSegment.h"
#include "ScopedCursor.h"
#include "ReferenceSegment.h"
#include "GlossNatSegment.h"
#include "SA_View.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CGlossSegment
// class to do all the handling for the gloss annotation segments.

CGlossSegment::CGlossSegment(EAnnotation index, int master) :
    CTextSegment(index,master) {
}

CGlossSegment::~CGlossSegment() {
}

CFontTable * CGlossSegment::NewFontTable() const {
    return new CFontTableANSI();
}

//SDM 1.5Test8.1
/***************************************************************************/
// CGlossSegment::Remove Remove annotation segment
// All the dependent segments that are aligned to the removed gloss
// segment have to be removed too. The user will be informed before.
/***************************************************************************/
void CGlossSegment::Remove(CSaDoc * pDoc, int sel, BOOL bCheck) {

    // save state for undo ability
    if (bCheck) {
        pDoc->CheckPoint();
    }

    DWORD dwOldOffset = GetOffset(sel);

    // remove aligned dependent segments (ref and gn)
    for (int nLoop = GLOSS + 1; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        CSegment * pSegment = pDoc->GetSegment(nLoop);
        if (pSegment->GetMasterIndex()==GLOSS) {
            int nIndex = pSegment->FindOffset(dwOldOffset);
            if (nIndex != -1) {
                pSegment->RemoveAt(nIndex);
            }
        }
    }

    RemoveAt(sel);

    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    pDoc->SetModifiedFlag(TRUE);                                // document has been modified
    pDoc->SetTransModifiedFlag(TRUE);                           // transcription data has been modified
    pView->ChangeAnnotationSelection(this, sel, 0, 0);			// deselect
    pView->RedrawGraphs(TRUE,FALSE);
}

//SDM 1.5Test8.1
void CGlossSegment::Serialize(CArchive & ar) {
    CSegment::Serialize(ar);
    if (ar.IsStoring()) {
        ar << CSaString("CGlossSegmentDetail tag");
    } else {
        CSaString detailTagCheck;
        ar >> detailTagCheck;
        SA_ASSERT(detailTagCheck == "CGlossSegmentDetail tag");
    }
    // use to be POS
    CStringArray temp;
    temp.Serialize(ar);
}

/***************************************************************************/
// CGlossSegment::Exit Exit parsing on error
// Standard exit function if an error occured.
/***************************************************************************/
long CGlossSegment::Exit(int nError) {
    EndProcess(); // end data processing
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
long CGlossSegment::Process(void * pCaller, ISaDoc * pSaDoc, int nProgress, int nLevel) {

    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }

    if (IsDataReady()) {
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

    if (nLevel < 0) { // previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            CancelProcess(); // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    CScopedCursor waitCursor(this);

    // start parsing
	// memory allocation failed or previous processing error
    if (!StartProcess(pCaller, IDS_STATTXT_PARSING, FALSE)) {   
		// end data processing
        EndProcess();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

	// end of loop
    DWORD dwLoopEnd = pLoudness->GetDataSize();                 
    // prepare parameters
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

    // get parsing parameters
	// size factor
    float fFactor = (float)pDoc->GetDataSize() / (float)dwLoopEnd;  
    TRACE("fFactor %f\n",fFactor);

	// break width in process words
    DWORD dwBreakWidth = (DWORD)(pDoc->GetBytesFromTime( pMainFrame->GetActiveBreakWidth()) / fFactor);  
    if (!dwBreakWidth) {
        dwBreakWidth = 1;
    }

	int nMaxThreshold = (int)((long)pLoudness->GetMaxValue() * (long)pMainFrame->GetMaxThreshold() / 100);
    int nMinThreshold = (int)((long)pLoudness->GetMaxValue() * (long)pMainFrame->GetMinThreshold() / 100);

    int nBlockAlign = pDoc->GetBlockAlign();

    // prepare gloss data
    int nGlossIndex = 0;

    // remove all gloss and deselect gloss
    DeleteContents();
    if (GetSelection() != -1) {
        SetSelection(-1);
    }
	// document has been modified
    pDoc->SetModifiedFlag(TRUE);
	// transcription data has been modified
    pDoc->SetTransModifiedFlag(TRUE);

	// allow early word SDM 1.5Test 11.0
    DWORD dwBreakCount = dwBreakWidth;
	// the start of the break
    DWORD dwBreakStart = 0;
	// the end of the break
    DWORD dwBreakEnd = 0;

	// get pointer to phonetic segment
    CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pDoc->GetSegment(PHONETIC);
	// Snap phonetic SDM 1.5Test11.0
    pDoc->AutoSnapUpdate();

    // start processing
    DWORD dwLoopPos = 0;

    while (dwLoopPos < dwLoopEnd) {
        BOOL bRes = TRUE;
		// read zero crossing data point
        int nLoudnessData = pLoudness->GetProcessedData(dwLoopPos++, &bRes);
        if (!bRes) {
			// error, reading zero crossing data failed
            return Exit(PROCESS_ERROR);
        }
		// point is over max threshold 
        if (nLoudnessData > nMaxThreshold) {
            // note end of break
            if (dwBreakCount >= dwBreakWidth) {
                // ready to store gloss
                DWORD dwGlossStart = nBlockAlign*(DWORD)((dwBreakEnd * fFactor)/nBlockAlign);

                dwGlossStart = pDoc->SnapCursor(START_CURSOR, dwGlossStart, 0, dwGlossStart, SNAP_LEFT);

                // check if the start is within a phonetic segment
                // NOTE: if we are using this task within the context of
                // the 'parse words' feature, than all previous phonetic and gloss segments
                // have already been deleted.
                int nPhonetic = pPhonetic->FindFromPosition(dwGlossStart, TRUE);
                // adjust to phonetic
                if (nPhonetic != -1) {
                    DWORD thisOffset = pPhonetic->GetOffset(nPhonetic);
                    int next = pPhonetic->GetNext(nPhonetic);
                    DWORD nextOffset = pPhonetic->GetOffset(next);
                    // if there is a next segment and
                    if ((next != -1) && ((nextOffset - dwGlossStart) < (dwGlossStart - thisOffset))) {
                        // the distance from gloss start to the next phonetic is less than
                        // the distance from the current offset to gloss start.
                        // next segment is closer SDM 1.5Test11.0
                        dwGlossStart = nextOffset;
                    } else {
                        dwGlossStart = thisOffset;
                    }
                } else {
                    // we could not locate gloss start between two phonetic segments
                    nPhonetic = pPhonetic->FindFromPosition(dwGlossStart);
                    if (nPhonetic != -1) {
                        DWORD thisOffset = pPhonetic->GetOffset(nPhonetic);
                        // if the distance from this offset to gloss start is under a minimum
                        // then use the current offset
                        if ((dwGlossStart - thisOffset) < pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) {
                            // Too close to existing segment SDM 1.5Test11.0
                            // don't use this segment
                            dwGlossStart = thisOffset;
                        } else {
                            // there is no phonetic segment, create one
                            if (dwGlossStart > thisOffset) {
                                nPhonetic++;    // append at end
                            }

                            {
                                int previousSeg = pPhonetic->GetPrevious(nPhonetic);
                                //TRACE("previous segment %d\n",previousSeg);

                                // if there is a previous segment, that does not have a duration yet
                                // now is the time to adjust it
                                if ((previousSeg != -1) && (pPhonetic->GetDuration(previousSeg)==0)) {
                                    // snap up to the next phonetic segment?
                                    DWORD dwPhoneticStop = nBlockAlign*(DWORD)((dwBreakStart * fFactor)/nBlockAlign);
                                    dwPhoneticStop = pDoc->SnapCursor(STOP_CURSOR, dwPhoneticStop, dwPhoneticStop, pDoc->GetDataSize(), SNAP_RIGHT);
                                    DWORD dwPhoneticOffset = pPhonetic->GetOffset(previousSeg);
                                    //TRACE("stop %d %d\n",dwPhoneticOffset,dwPhoneticStop);
                                    if (dwPhoneticStop > dwPhoneticOffset) {
                                        // adjust the duration
                                        //TRACE("adjust offset[%d] %lu %lu %lu\n",previousSeg,dwPhoneticOffset,dwPhoneticStop,dwPhoneticStop - dwPhoneticOffset);
                                        pPhonetic->Adjust( pDoc, previousSeg, dwPhoneticOffset, dwPhoneticStop - dwPhoneticOffset, false);
                                        DWORD offset = pPhonetic->GetOffset(previousSeg);
                                        DWORD duration = pPhonetic->GetDuration(previousSeg);
                                        //TRACE("adjusted offset[%d]=%lu %lu %lu\n",previousSeg,offset,(offset+duration),duration);
                                        if ((offset+duration)>=dwGlossStart) {
                                            //TRACE("moving gloss start %lu->%lu\n",dwGlossStart,(offset+duration));
                                            dwGlossStart = offset+duration+1;
                                        }
                                    }
                                }
                            }
                            // SDM 1.06.3a Insert 0 duration calculate later
							pPhonetic->AddAt( pDoc, nPhonetic, dwGlossStart, 0);
                        }
                    } else {
                        // there is no phonetic segment, create one
						nPhonetic = pPhonetic->Add( pDoc, dwGlossStart, 0);
                    }
                }

                // write gloss
				AddAt( pDoc, nGlossIndex++, dwGlossStart, 0);

				// document has been modified
                pDoc->SetModifiedFlag(TRUE);
				// transcription data has been modified
                pDoc->SetTransModifiedFlag(TRUE);   
            }
			// end of valley
            dwBreakCount = 0; 
        } else {
			// point is under min threshold
            if (nLoudnessData <= nMinThreshold) {   
                // reset end of break
                dwBreakEnd = dwLoopPos;
                if (dwBreakCount == 0) {
                    // note beginning of break
					// SDM 1.5 Test 11.0
                    dwBreakStart = dwLoopPos - 1;   
                }
                dwBreakCount++;
            } else if (dwBreakCount > 0) {          // SDM 1.5 Test 11.0
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
        dwPhoneticStop = pDoc->SnapCursor(STOP_CURSOR, dwPhoneticStop,dwPhoneticStop,pDoc->GetDataSize(),SNAP_RIGHT);
        if (dwPhoneticStop > pPhonetic->GetOffset(nPhonetic)) {
            pPhonetic->Adjust(pDoc,nPhonetic, pPhonetic->GetOffset(nPhonetic),dwPhoneticStop - pPhonetic->GetOffset(nPhonetic), false);
        }
    }

    // Adjust durations of inserted phonetic segments
    if (!pPhonetic->IsEmpty()) {
        int nPhonetic = 0;
        CSaString szSegment = SEGMENT_DEFAULT_CHAR;
        while (nPhonetic != -1) {
            int nNext = pPhonetic->GetNext(nPhonetic);
            if ((pPhonetic->GetDuration(nPhonetic)==0) && (pPhonetic->GetSegmentString(nPhonetic) == szSegment)) {
                //Inserted phonetic segment
                if (nNext != -1) {
                    pPhonetic->Adjust(pDoc, nPhonetic, pPhonetic->GetOffset(nPhonetic), pPhonetic->GetOffset(nNext) - pPhonetic->GetOffset(nPhonetic), false);
                } else {
                    // Last phonetic segment
                    pPhonetic->Adjust(pDoc, nPhonetic, pPhonetic->GetOffset(nPhonetic),pDoc->GetDataSize() - 1 - pPhonetic->GetOffset(nPhonetic), false);
                }
            }
            nPhonetic = nNext;
        }
    }

    CorrectGlossDurations(pDoc);

    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    EndProcess((nProgress >= 95)); // end data processing
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

void CGlossSegment::CorrectGlossDurations(ISaDoc * pDoc) {
    // Adjust Gloss Durations
    if (!IsEmpty()) {
        int nGloss = 0;
        while (nGloss != -1) {
            Adjust(pDoc, nGloss, GetOffset(nGloss), CalculateDuration(pDoc, nGloss), false);
            nGloss = GetNext(nGloss);
        }
    }
}

bool CGlossSegment::ContainsText( DWORD offset, DWORD stop) {
	for (int i=0;i<GetOffsetSize();i++) {
		DWORD thisOffset = GetOffset(i);
		DWORD thisStop = GetStop(i);
		if ((thisOffset>=offset)&&(thisStop<=stop)) {
			CString text = GetText(i).Trim();
			if (text.GetLength()==0) continue;
			if ((text.GetLength()==1) && ((text[0]==WORD_DELIMITER)||(text[0]==TEXT_DELIMITER))) continue;
			TRACE("text %s is blocking operation for %d\n",(LPCTSTR)text,this->m_nAnnotationType);
			return true;
		}
	}
	return false;
}

void CGlossSegment::AddAt( CSaDoc * pDoc, int index, DWORD offset, DWORD duration) {
    InsertAt( index, offset, duration);
	CGlossNatSegment * pGlossNat = (CGlossNatSegment*)pDoc->GetSegment(GLOSS_NAT);
	pGlossNat->InsertAt( index, offset, duration);
	CReferenceSegment * pRef = (CReferenceSegment*)pDoc->GetSegment(REFERENCE);
	pRef->InsertAt( index, offset, duration);
}

