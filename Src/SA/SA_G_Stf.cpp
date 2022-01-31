//////////////////////////////////////////////////////////////////////////
// sa_g_stf.cpp:
// Implementation of the CPlotStaff class.
// Author: Todd Jones & Tim Erickson
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
//   08/14/2000 - DDO Moved the code tied to the convert button from
//                    sa_g_mbt.cpp to here because the button moved from
//                    the TWC controls window to the staff window.
//   08/16/2000 - DDO Added code to the convert function so the user doesn't
//                    have to explicitly paste the converted melogram data
//                    onto the staff control.
//   10/12/2000 - CLW Repaired TWC data process call.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_plot.h"
#include "sa_view.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "sa_g_stf.h"
#include "mainfrm.h"
#include "partiture.hpp"
#include "sa_doc.h"
#include "sa.h"
#include <mmsystem.h>
#include <math.h>
#include "DlgPlayer.h"
#include "Process\Process.h"
#include "Process\sa_p_twc.h"
#include "Process\sa_p_mel.h"
#include "Process\sa_p_lou.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern int INIT(HINSTANCE hInst);

extern CSaApp NEAR theApp;

//###########################################################################
// CPlotStaff
// Plot window for displaying melogram pitch data. This class only handles the
// special drawing of the data. All other work is done in the base class.

IMPLEMENT_DYNCREATE(CPlotStaff, CPlotWnd)

/////////////////////////////////////////////////////////////////////////////
// CPlotStaff message map

BEGIN_MESSAGE_MAP(CPlotStaff, CPlotWnd)
    ON_WM_SETFOCUS()
    ON_WM_PARENTNOTIFY()
    ON_WM_CREATE()
END_MESSAGE_MAP()

/***************************************************************************/
// CPlotStaff::CPlotStaff Constructor
/***************************************************************************/
CPlotStaff::CPlotStaff() {
    m_bCursors = false;
    m_bPrivateCursor = false;
    StaffControl = NULL;
    LastSize.left = 0;
    LastSize.top = 0;
    LastSize.right = 0;
    LastSize.bottom = 0;
    m_pView = NULL;
}

CPlotWnd * CPlotStaff::NewCopy() {
    CPlotWnd * pRet = new CPlotStaff;

    CopyTo(pRet);

    return pRet;
}


/***************************************************************************/
// CPlotStaff::~CPlotStaff Destructor
/***************************************************************************/
CPlotStaff::~CPlotStaff() {
    if (StaffControl) {
        ::DestroyWindow(StaffControl);
        StaffControl = NULL;
    }

    GraphHasFocus(FALSE);
}

/***************************************************************************/
// CPlotStaff::GraphHasFocus
/***************************************************************************/
void CPlotStaff::GraphHasFocus(BOOL bFocus) {
    if (bFocus) {
        if (StaffControl) {
            ::InvalidateRect(StaffControl, NULL, FALSE);
        }
    } else {
        if (m_pView) {
            CGraphWnd * pMelogram = m_pView->GraphIDtoPtr(IDD_MELOGRAM);
            if (pMelogram) {
                CPlotWnd * pMelPlot = pMelogram->GetPlot();
                if (pMelPlot) {
                    pMelPlot->ClearHighLightArea();
                    pMelPlot->UpdateWindow();
                }
            }
        }
    }
}

/***************************************************************************/
// CPlotStaff::SetWindowText - Call me after reading staff info from RIFF file
/***************************************************************************/
void CPlotStaff::SetWindowText(LPCTSTR lpsz) {
    if (StaffControl) {
        ::SetWindowText(StaffControl,lpsz);
    }
}

/***************************************************************************/
// CPlotStaff::GetWindowText - Call me before saving staff info into RIFF file
/***************************************************************************/
int CPlotStaff::GetWindowText(LPTSTR lpsz, int cbMax) {
    if (StaffControl) {
        return ::GetWindowText(StaffControl,lpsz,cbMax);
    } else {
        return 0;
    }
}

/***************************************************************************/
// CPlotStaff::GetWindowTextLength - Returns the needed buffer size for 'lpsz' in GetWindowText
/***************************************************************************/
int CPlotStaff::GetWindowTextLength(void) {
    if (StaffControl) {
        return ::GetWindowTextLength(StaffControl);
    } else {
        return 0;
    }
}

/***************************************************************************/
// CPlotStaff::GetTempo - Returns Tempo (int between 50-200 = quarternotes per minute)
//                        Returns zero on error
/***************************************************************************/
int CPlotStaff::GetTempo() {
    if (StaffControl) {
        return ::SendMessage(StaffControl,WM_APP + 11,0,0);
    } else {
        return 0;
    }
}

/***************************************************************************/
// CPlotStaff::SaveAsMIDI
/***************************************************************************/
int CPlotStaff::SaveAsMIDI(char * szFileName) {
    if (StaffControl) {
        return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP + 1,0,(LPARAM)szFileName);
    } else {
        return 0;
    }
}

int CPlotStaff::ExportFile() {

    if (StaffControl) {
        int i;

        CSaView * pView = ((CMainFrame *)AfxGetMainWnd())->GetCurrSaView();
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        CString csWavName = pDoc->GetPathName();
        if (csWavName.IsEmpty()) {
            return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP+1,0,0);
        }

        int iWavNameSize = csWavName.GetLength();
        char * szSuggestName = new char[iWavNameSize+4];

        for (i=0; i < iWavNameSize; i++) {
            szSuggestName[i] = char((csWavName[i] < 128) ? csWavName[i] : 0);
        }
        szSuggestName[iWavNameSize] = 0;

        for (i=iWavNameSize; i&&(szSuggestName[i]!='.'); --i);
        if (!i) {
            i=iWavNameSize;
        }
        strcpy_s(szSuggestName+i,iWavNameSize+4-i,".xml");
        return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP+1,1,(LPARAM)szSuggestName);
    } else {
        return 0;
    }
}

int CPlotStaff::ImportFile() {
    if (StaffControl) {
        return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP+3,0,0);
    } else {
        return 0;
    }
}

/***************************************************************************/
// CPlotStaff::PlaySelection
/***************************************************************************/
int CPlotStaff::PlaySelection(BOOL bMidi, BOOL bWave) {
    if (StaffControl) {
        return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP + 2,(WPARAM)bMidi ,bWave);
    } else {
        return 0;
    }
}

/***************************************************************************/
// CPlotStaff::PausePlay
/***************************************************************************/
int CPlotStaff::PausePlay(void) {
    if (StaffControl) {
        return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP + 4,0,NULL);
    } else {
        return 0;
    }
}

/***************************************************************************/
// CPlotStaff::StopPlay
/***************************************************************************/
int CPlotStaff::StopPlay(void) {
    //TRE : How to get Save button to display when we need it?
    //      Here's a temporary solution
    if (StaffControl && ::SendMessage(StaffControl,(WPARAM)EM_GETMODIFY,0,0)) {
        CSaDoc * pDoc = (CSaDoc *)m_pView->GetDocument();
        pDoc->SetModifiedFlag(TRUE);
		// transcription has been modified
        pDoc->SetTransModifiedFlag(TRUE); 
    }

	// send message to stop player
    m_pView->SendMessage(WM_COMMAND, ID_PLAYER_STOP, 0L);

    if (StaffControl) {
        return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP + 5,0,NULL);
    } else {
        return 0;
    }
}

/***************************************************************************/
// CPlotStaff::LoopPlay
/***************************************************************************/
int CPlotStaff::LoopPlay(void) {
    if (StaffControl) {
        return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP + 6,0,NULL);
    } else {
        return 0;
    }
}

/***************************************************************************/
// CPlotStaff::HideButtons
/***************************************************************************/
int CPlotStaff::HideButtons(void) {
    if (StaffControl) {
        return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP + 7,0,NULL);
    } else {
        return 0;
    }
}

/***************************************************************************/
// CPlotStaff::ChooseVoice
/***************************************************************************/
int CPlotStaff::ChooseVoice(void) {
    if (StaffControl) {
        return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP + 8,0,NULL);
    } else {
        return 0;
    }
}


/***************************************************************************/
// CPlotStaff::ChooseTempo
/***************************************************************************/
int CPlotStaff::ChooseTempo(void) {
    if (StaffControl) {
        return (int) ::SendMessage(StaffControl,(WPARAM)WM_APP + 9,0,NULL);
    } else {
        return 0;
    }
}


/***************************************************************************/
// CPlotStaff::NoteNum2Name   DDO - 08/14/00 Moved here from sa_g_mbt.cpp
/***************************************************************************/
char * CPlotStaff::NoteNum2Name(double dMIDINumber, char * sMusique, size_t len, BOOL bHalfSharpFlat) {
    // $#+=-@!
    char sNoteNames[12][3] =
    {"C=", "C#", "D=", "E@", "E=", "F=", "F#", "G=", "G#", "A=", "B@", "B="};
    char sNoteNamesHalfSharp[12][3] =
    {"C-", "C+", "D-", "D+", "E-", "F-", "F+", "G-", "G+", "A-", "A+", "B-"};
    char pASCII[2] =
    {'\0','\0'};

    // limit range of input MIDI numbers
    dMIDINumber = (dMIDINumber < 12) ? 12 : dMIDINumber;
    // calculate interval and octave
    double dSamaNumber = (dMIDINumber - 12); // shifted slightly from MIDI Numbers
    short nOctave = (short)((dSamaNumber + 0.5 + 0.25*bHalfSharpFlat) / 12);
    if (!bHalfSharpFlat || !(short(dSamaNumber*2 + 0.5)&0x1)) {
        short nInterval = (short)(dSamaNumber - nOctave * 12 + 0.5);
        // construct note name
        strcpy_s(sMusique, len, &sNoteNames[nInterval][0]);
    } else {
        int nInterval = (short)(dSamaNumber) % 12;
        // construct note name
        strcpy_s(sMusique, len, &sNoteNamesHalfSharp[nInterval][0]);
    }
    _itoa_s(nOctave, pASCII, _countof(pASCII), 10);
    strcat_s(sMusique, len, pASCII);

    return sMusique;
}

double CPlotStaff::QNotes2Dur(double dNoteQtrNotes, char * sDuration, size_t len) {
    const double fNoteStdDur[21] = {1/8., 1/6.,  0.2, 0.25, 1/3., 3/8.,  0.4,  0.5, 2/3., 0.75,  0.8,   1.,  4/3.,   1.5,   1.8,    2.,  8/3.,    3.,   3.2,    4.,  6.};
    const double fNoteMinDur[21] = {  0.,   0.,   0.,   0.,  .33, .338, .395, .405, .662, .712, .795, .805, 1.328, 1.338, 1.795, 1.805, 2.662, 2.672, 3.195, 3.205, 5.9};
    const char   sNoteDur[21][3] = {"zn", "st", "sv", "sn", "it", "s.", "iv", "in", "qt", "i.", "qv", "qn", "ht", "q.", "hv", "hn", "wt", "h.", "wv", "wn", "w."};
    double dResult;

    USHORT nNoteIndex; // final index decision

    if (dNoteQtrNotes > fNoteStdDur[20])
        // We generally do not tie dotted whole notes together use whole notes
    {
        nNoteIndex = 19;
    } else {
        USHORT  nMinIndex,  // lowest valid index at current step in search
                nMaxIndex,  // highest valid index
                nMidIndex;  // watershed index (in the middle of valid indexes)

        // binary search for note duration
        nMinIndex = 0;
        nMaxIndex = 19;
        while (nMinIndex != nMaxIndex) {
            nMidIndex = (unsigned short)((nMaxIndex + nMinIndex + 1) / 2);
            if (dNoteQtrNotes < fNoteMinDur[nMidIndex]) {
                nMaxIndex = USHORT(nMidIndex - 1);
            } else {
                nMinIndex = (nMidIndex);
            }
        }
        nNoteIndex = nMinIndex;
    }

    // compose duration string
    strcpy_s(sDuration, len, sNoteDur[nNoteIndex]);

    dResult = dNoteQtrNotes - fNoteStdDur[nNoteIndex];

    return dResult;
}

double CPlotStaff::Note2String(double dNoteQtrNotes, double dMIDINumber, CString & sMusique, double dNoteTol, BOOL bHalfFlatSharp) {
    sMusique.Empty();

    // convert note
    char sNote[7] = "";
    BOOL bRest = (dMIDINumber <= 0);
    if (!bRest) {
        (void)NoteNum2Name(dMIDINumber, sNote, _countof(sNote), bHalfFlatSharp); // name, accidental, octave
        strcat_s(sNote, _countof(sNote), "*");
    } else {
        strcpy_s(sNote, _countof(sNote), "R=3*");
    }


    while (dNoteQtrNotes >= dNoteTol) {
        char sDur[4];

        dNoteQtrNotes = QNotes2Dur(dNoteQtrNotes, sDur, _countof(sDur));

        sMusique += sNote;
        sMusique += sDur;

        if (dNoteQtrNotes >= dNoteTol) {
            if (!bRest) {
                sMusique += "_";
            } else {
                sMusique += " ";
            }
        }
    }

    return dNoteQtrNotes;
}

/***************************************************************************/
// CPlotStaff::Convert   DDO - 08/14/00 Moved here from sa_g_mbt.cpp
/***************************************************************************/
void CPlotStaff::Convert() {

    BeginWaitCursor();

    CString sMelody;

    // get melogram data
    CSaDoc * pDoc = (CSaDoc *)m_pView->GetDocument(); // cast pointer
    CProcessMelogram * pMelogram = (CProcessMelogram *)pDoc->GetMelogram(); // get pointer to melogram object
    long lResult = pMelogram->Process(this, pDoc); // process data
    short int nLevel = LOWORD(lResult);
    if (nLevel < 0) {
        ::SendMessage(StaffControl, WM_SETTEXT, 0, (LPARAM)((LPCTSTR)sMelody));
        return;
    }
    int nProgress = HIWORD(lResult);
    DWORD dwMelDataLength = pMelogram->GetDataSize(); // size of melogram data

    // Get Loudness Data
    CProcessLoudness * pLoudness = (CProcessLoudness *)pDoc->GetLoudness(); // get pointer to loudness object
    lResult = pLoudness->Process(this, pDoc); // process data
    nLevel = LOWORD(lResult);
    if (nLevel < 0) {
        ::SendMessage(StaffControl, WM_SETTEXT, 0, (LPARAM)((LPCTSTR)sMelody));
        return;
    }

    // get TWC data
    const CMusicParm * pParm = pDoc->GetMusicParm();

    int nUpperBound = pParm->nUpperBound;
    int nLowerBound = pParm->nLowerBound;

    if (pParm->nRangeMode == 0) {
        CMusicParm::GetAutoRange(pDoc, nUpperBound, nLowerBound);
    }
    short nMinSemitone = (short)nLowerBound;
    short nMaxSemitone = (short)nUpperBound;
    CProcessTonalWeightChart * pTWC = (CProcessTonalWeightChart *)pDoc->GetTonalWeightChart(); // get pointer to TWC object
    lResult = pTWC->Process(this, pDoc, 0, dwMelDataLength, nMinSemitone, nMaxSemitone); // process data
    nLevel = LOWORD(lResult);
    nProgress = HIWORD(lResult);
    if (nLevel < 0) {
        ::SendMessage(StaffControl, WM_SETTEXT, 0, (LPARAM)((LPCTSTR)sMelody));
        return;
    }

    // Find size, max, min and max bin
    DWORD dwTWCDataLength = pTWC->GetDataSize(); // size of TWC data
    double dMinSemitone = pTWC->GetMinSemitone();
    double dMaxSemitone = pTWC->GetMaxSemitone();
    short nMaxBinValue = 0;
    DWORD dwMaxBin = 0, i;
    short int * pTWCData = NULL;
    pTWCData = (short int *)pTWC->GetProcessedData(0, TRUE);
    for (i=0; i<dwTWCDataLength; i++) {
        if (pTWCData[i] > nMaxBinValue) {
            nMaxBinValue = pTWCData[i];
            dwMaxBin = i;
        }
    }
    // calibrate semitones
    double dCalSemitone = dMinSemitone + (dMaxSemitone - dMinSemitone) * (double)dwMaxBin / (float)dwTWCDataLength; // find calibration point
    short nCalSemitone = (short)(dCalSemitone + .5);
    double dSemitoneShift = nCalSemitone - dCalSemitone;

    // get sampling rate
    double dQNotesPerFrame = GetTempo()*pDoc->GetNumSamples()/pMelogram->GetDataSize()/60.0/pDoc->GetSamplesPerSec();
    const int nMelogramAverageInterval = int(1./(8*dQNotesPerFrame) + 1.0);  // Average over grace note interval

    // parse into notes
    double dMel2Loud = double(pLoudness->GetDataSize())/pMelogram->GetDataSize();

    double dQNotes = 0;
    double dQNotesPrev = 0;
    double dSemitoneStringPrev = -1;
    DWORD dwOffsetBegin = 0;
    DWORD dwOffsetNoteTrans = 0;
    BOOL bVoiced = FALSE;
    const BOOL bHalfFlatSharps = FALSE;
    double dReleaseThreshold = 0;
    for (DWORD dwDataOffset = 0; dwDataOffset < dwMelDataLength; dwDataOffset++) {
        short int * pMelData = (short int *)pMelogram->GetProcessedDataBlock(dwDataOffset*sizeof(short int), sizeof(short int));
        short int * pLoudData = (short int *)pLoudness->GetProcessedDataBlock(DWORD(dwDataOffset*dMel2Loud)*sizeof(short int), sizeof(short int));

        BOOL bVoiced2Unvoiced = FALSE;
        BOOL bUnvoiced2Voiced = FALSE;
        BOOL bRelease = FALSE;

        if (bVoiced && pMelData[0] <= 0) {
            bVoiced2Unvoiced = TRUE;
        }
        if (!bVoiced && pMelData[0] > 0) {
            bUnvoiced2Voiced = TRUE;
            dReleaseThreshold += 10000000; // disable release for about 20 samples
        }

        if (bVoiced && pLoudData[0] > dReleaseThreshold) {
            bRelease = TRUE;
        }

        dReleaseThreshold = dReleaseThreshold*0.80 + 1.2*(1-0.80)*pLoudData[0];

        if (bVoiced2Unvoiced || bUnvoiced2Voiced || bRelease || (dwDataOffset+1) >= dwMelDataLength) {
            double dHistogramThreshold = 0.25;
            int nStableFrames = 0;

            CHistogram * pHistogram = NULL;
            if (bVoiced) {
                pHistogram = pTWC->MakeTwcHistogram(pMelogram, 10, dMinSemitone, dMaxSemitone, dwOffsetBegin, (dwDataOffset-dwOffsetBegin));

                for (USHORT bin=0; bin < pHistogram->GetHistogramParms().nBins; bin++) {
                    int nValue = 0;
                    pHistogram->GetBin(nValue, bin, 1.0, COUNTS);
                    if (nValue > pHistogram->GetMaxValue(this)*dHistogramThreshold) {
                        nStableFrames += nValue;
                    }
                }
            }

            pMelData = (short int *)pMelogram->GetProcessedDataBlock(dwOffsetBegin*sizeof(short int), sizeof(short int)*(dwDataOffset-dwOffsetBegin));
            pLoudData = (short int *)pLoudness->GetProcessedDataBlock(int(dwOffsetBegin*dMel2Loud)*sizeof(short int), sizeof(short int)*int(dMel2Loud*(dwDataOffset-dwOffsetBegin)));

            double dSemitone = -1;
            double dSemitoneString = -1;
            short nNoteLength = 0; // length of note in frames
            // We will need to save the state when we rewind the processing
            double dSemitoneSave = 0;
            double dSemitoneStringSave = 0;
            short nNoteLengthSave = 0;
            int nStableFramesSave = 0;
            for (i = 0; i < dwDataOffset-dwOffsetBegin; i++) {
                dQNotes += dQNotesPerFrame;

                if (bVoiced) {
                    int nHistogramCount = 0;
                    pHistogram->GetBinByData(nHistogramCount, pMelData[i], 1.0, COUNTS);
                    BOOL bStableValue = (nHistogramCount > pHistogram->GetMaxValue(this)*dHistogramThreshold);
                    if (bStableValue) {
                        int nWeight = nNoteLength > nMelogramAverageInterval ? nMelogramAverageInterval : nNoteLength;
                        dSemitone = (dSemitone*nWeight + pMelData[i] / 100.) / (nWeight + 1);
                        nStableFrames--;
                        nNoteLength++;
                        if (nNoteLength <= nMelogramAverageInterval) {
                            dSemitoneString = dSemitone;    // We want a good solid average reference
                        }
                    }
                } else {
                    nNoteLength++;
                    dSemitone = pMelData[i] / 100.;
                }

                short nQuartertone = (short)((dSemitone + dSemitoneShift)*(1+bHalfFlatSharps) + .5);
                short nQuartertoneCurr = (short)((pMelData[i] / 100. + dSemitoneShift)*(1+bHalfFlatSharps) + .5);
                short nQuartertoneLast = 0;
                if (i > 0) {
                    nQuartertoneLast = (short)((pMelData[i-1] / 100. + dSemitoneShift)*(1+bHalfFlatSharps) + .5);
                }
                if (nQuartertoneCurr != nQuartertoneLast) { // keep track of the last quartertone transition
                    dwOffsetNoteTrans = i;                    // so we can pinpoint the note change more accurately
                    dSemitoneSave = dSemitone;
                    dSemitoneStringSave = dSemitoneString;    // Also, save the state in case we need to rewind the processing
                    nNoteLengthSave = nNoteLength;
                    nStableFramesSave = nStableFrames;
                }

                double dNoteTol = 4./32.;

                BOOL bPitchChange = nQuartertone != short((dSemitoneString + dSemitoneShift)*(1+bHalfFlatSharps) + .5) && fabs(dSemitone - dSemitoneString)*(1+bHalfFlatSharps) > 0.90;
                BOOL bRoomForMoreNotes = (nStableFrames*dQNotesPerFrame > dNoteTol);
                BOOL bLast = ((i+1) == (dwDataOffset-dwOffsetBegin));


                if (bLast || (bPitchChange && bRoomForMoreNotes)) {
                    if (dQNotes >= dNoteTol) {
                        // convert previous note
                        CString sNote;

                        DWORD dwOffsetBackup = i - dwOffsetNoteTrans;       // back up to last note transition candidate
                        if (bPitchChange) {
                            dQNotes -= dwOffsetBackup * dQNotesPerFrame;
                            // i -= dwOffsetBackup;
                            bLast = ((i+1) == (dwDataOffset-dwOffsetBegin));    // be sure this really is the last note
                        }

                        if ((dSemitoneString == -1.) && sMelody.GetLength()) { // if this is a rest but not the beginning
                            dQNotesPrev = dQNotesPrev + dQNotes;    // keep accumulating in case we need to combine it
                        } else {
                            dQNotesPrev = dQNotes;    // otherwise just keep this note
                        }
                        dQNotes = Note2String(dQNotes, dSemitoneString, sNote, dNoteTol, bHalfFlatSharps);
                        if ((sNote[0] == 'R') && sMelody.GetLength()
                                && (dQNotesPrev < 1.) && (dQNotesPrev - dQNotes < 1.)) { // if this is a rest that's shorter than a quarter rest and not at the beginning
                            sMelody = sMelody.Left(sMelody.GetLength() - 7);
                            dQNotes = Note2String(dQNotesPrev, dSemitoneStringPrev, sNote, dNoteTol, bHalfFlatSharps);
                        }
                        dSemitoneStringPrev = dSemitoneString;              // hang on to the current semitone number

                        if (bLast) {
                            sNote += ' ';    // voicing/loudness changes are not slurred
                        } else {
                            sNote += '_';    // pitch change is slurred
                        }

                        sMelody += sNote;

                        // start new note
                        dSemitoneString = dSemitone;
                        nNoteLength = 0;
                        if (bPitchChange) {
                            i -= dwOffsetBackup;                    // rewind processing
                            dSemitoneSave = dSemitone;              // and restore the state
                            dSemitoneStringSave = dSemitoneString;  // at the last note transition
                            nNoteLengthSave = nNoteLength;
                            nStableFrames = nStableFramesSave;
                        }
                    }
                }
            }
            dwOffsetBegin = dwDataOffset;

            if (pHistogram) {
                delete pHistogram;
            }
        }
        bVoiced = (pMelData[0] > 0);
    }
    ::SendMessage(StaffControl, WM_SETTEXT, 0, (LPARAM)((LPCTSTR)sMelody));

    // Document has been modified
    pDoc->SetModifiedFlag(TRUE);
    pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified

    EndWaitCursor();
}



/////////////////////////////////////////////////////////////////////////////
// CPlotStaff message handlers

/***************************************************************************/
// CPlotStaff::OnDraw Drawing
/***************************************************************************/
void CPlotStaff::OnDraw(CDC *, CRect, CRect, CSaView * pView) {
    if (!m_pView && pView) { // first access initialize score

        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        CString szMusicScore = pDoc->GetMusicScore();
        int nMusicScoreSize = szMusicScore.GetLength();
        if (nMusicScoreSize) {
            TCHAR * pMusicScore = szMusicScore.GetBuffer(nMusicScoreSize);
            SetWindowText(pMusicScore);
            szMusicScore.ReleaseBuffer(nMusicScoreSize);
        }
    }
    m_pView = pView;

    RECT NewSize;
    ::GetWindowRect(m_hWnd,&NewSize);

    if (NewSize.left != LastSize.left
            || NewSize.top != LastSize.top
            || NewSize.right != LastSize.right
            || NewSize.bottom != LastSize.bottom) {
        int width = NewSize.right-NewSize.left;
        int height = NewSize.bottom-NewSize.top;

        ::MoveWindow(StaffControl,0,0,width,height,1);
        LastSize.left = NewSize.left;
        LastSize.top = NewSize.top;
        LastSize.right = NewSize.right;
        LastSize.bottom = NewSize.bottom;
    }

    ::UpdateWindow(StaffControl);
    ::ShowWindow(StaffControl,SW_SHOW);
}

void CPlotStaff::SetModifiedFlag(BOOL Modified) {
    if (StaffControl) {
        ::SendMessage(StaffControl,EM_SETMODIFY,(WPARAM)Modified,0);
    }
}

void CPlotStaff::OnParentNotify(UINT msg,LPARAM lParam) {
    switch (msg) {
    case EM_SETMODIFY: {
        CSaDoc * pDoc = (CSaDoc *)m_pView->GetDocument();
        pDoc->SetModifiedFlag(TRUE);
		// transcription data has been modified
        pDoc->SetTransModifiedFlag(TRUE); 
        break;
    }
    case WM_SETFOCUS: {
		// sets the focused graph pointer
        m_pView->SetFocusedGraph(m_pParent); 
        break;
    }
    case IDC_CONVERT: {
        Convert();
        break;
    }
    case WM_PAINT: {
        if (m_pView) {
            CGraphWnd * pMelogram = m_pView->GraphIDtoPtr(IDD_MELOGRAM);
            if (pMelogram) {
                CPlotWnd * pMelPlot = pMelogram->GetPlot();
                if (pMelPlot) {
                    SPartSelectionMS * pSel = (SPartSelectionMS *)lParam;
					// cast pointer
                    CSaDoc * pDoc = (CSaDoc *)m_pView->GetDocument(); 
                    pMelPlot->SetHighLightArea(pDoc->GetBytesFromTime(pSel->begin), pDoc->GetBytesFromTime(pSel->end), TRUE, TRUE);
                    pMelPlot->UpdateWindow();
                }
            }
        }
        break;
    }
    case WM_APP + 2: { 
		// play recording of selection
        CMainFrame * pMain = (CMainFrame *) AfxGetMainWnd();
        if (m_pView && pMain->GetCurrSaView() == m_pView) {
            SPartSelectionMS & sel = *(SPartSelectionMS *)lParam;
            SSpecific specific;
            specific.begin = sel.begin;
            specific.end = sel.end;
            pMain->PlaySpecific( specific);
        }
        break;
    }
    }
}

BOOL CPlotStaff::PreTranslateMessage(MSG * pMsg) {
	// these have already been translated
    if (pMsg->message == WM_CHAR) { 
        return FALSE;
    }
	
	if (pMsg->message == WM_KEYDOWN) {
        switch (pMsg->wParam) { 
		// these are keystrokes (with or without control, shift or alt) defined as
        // accelerators in SA.  But we want them as keystrokes for this control.
        case VK_DELETE:
        case VK_LEFT:
        case VK_RIGHT:
        case 'C':
        case 'W':
        case 'M':
        case 'L':
        case 'V':
        case 'X':
            ::SendMessage(StaffControl,WM_KEYDOWN,pMsg->wParam,pMsg->lParam);
            return TRUE;
        default:
            break;
        }
    }

    return CPlotWnd::PreTranslateMessage(pMsg);
}

int CPlotStaff::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CPlotWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    if (!StaffControl) {
        INIT(theApp.m_hInstance);
        StaffControl = CreateWindow(_T("Partiture"),_T("(CLEF2)"),
                                    WS_CHILD|WS_VISIBLE|WS_BORDER,
                                    20,0,100,50,m_hWnd,NULL,theApp.m_hInstance,NULL);
    }

    return 0;
}

void CPlotStaff::SetFocusedGraph(CGraphWnd * cgw) {
    if (m_pView) {
        m_pView->SetFocusedGraph(cgw);
    }
    OnSetFocus(cgw);
}

void CPlotStaff::OnSetFocus(CWnd *) {
    if ((HWND)StaffControl) {
        ::SetFocus((HWND)StaffControl);
    }
}
 