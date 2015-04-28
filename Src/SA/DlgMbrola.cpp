// DlgMbrola.cpp : implementation file
//
#include "stdafx.h"
#include "sa.h"
#include "DlgMbrola.h"
#include "sa_doc.h"
#include "Segment.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "FileUtils.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_spi.h"
#include "Process\Process.h"
#include <IpaSampa.h>
#include <mbrola.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMbrola dialog


IMPLEMENT_DYNCREATE(CDlgMbrola, CPropertyPage)

CDlgMbrola::CDlgMbrola() : CPropertyPage(CDlgMbrola::IDD) {
    m_fPitchUpdateInterval = 0.01;
    int Err = load_MBR();
    UNUSED_ALWAYS(Err);
    m_bGetComplete = FALSE;
    m_bConvertComplete = FALSE;
    m_bGetDuration = TRUE;
    m_bGetIPA = TRUE;
    m_bGetPitch = TRUE;
}

CDlgMbrola::~CDlgMbrola() {
    // remove synthesized wavefile in SA
    FileUtils::Remove(m_szMBRolaName);
    unload_MBR();
}


void CDlgMbrola::DoDataExchange(CDataExchange * pDX) {
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MBROLA_OUTPUT_GRID, m_cOutputGrid);
    DDX_Check(pDX, IDC_MBROLA_IPA, m_bGetIPA);
    DDX_Control(pDX, IDC_MBROLA_SOURCE, m_cSource);
    DDX_Control(pDX, IDC_MBROLA_DICTIONARY, m_cDictionary);
    DDX_Check(pDX, IDC_MBROLA_DURATION, m_bGetDuration);
    DDX_Control(pDX, IDC_MBROLA_GRID, m_cGrid);
    DDX_Check(pDX, IDC_MBROLA_PITCH, m_bGetPitch);
}


BEGIN_MESSAGE_MAP(CDlgMbrola, CPropertyPage)
    ON_BN_CLICKED(IDC_MBROLA_GET, OnMbrolaGet)
    ON_BN_CLICKED(IDC_MBROLA_SYNTHESIZE, OnMbrolaSynthesize)
    ON_BN_CLICKED(IDC_MBROLA_PLAY_SYNTH, OnMbrolaPlaySynth)
    ON_BN_CLICKED(IDC_MBROLA_PLAY, OnMbrolaPlay)
    ON_BN_CLICKED(IDC_MBROLA_PLAY_SOURCE, OnMbrolaPlaySource)
    ON_BN_CLICKED(IDC_MBROLA_CONVERT, OnMbrolaConvert)
    ON_BN_CLICKED(IDC_MBROLA_DISPLAY, OnMbrolaDisplay)
    ON_CBN_DROPDOWN(IDC_MBROLA_SOURCE, OnDropdownSource)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMbrola message handlers

BEGIN_EVENTSINK_MAP(CDlgMbrola, CPropertyPage)
//{{AFX_EVENTSINK_MAP(CDlgMbrola)
ON_EVENT(CDlgMbrola, IDC_MBROLA_GRID, 72 /* LeaveCell */, OnLeaveCellMbrolaGrid, VTS_NONE)
//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

static void CurveFitPitch(CSaDoc * pDoc, double fSizeFactor, DWORD dwBeginWAV, DWORD dwEndWAV, double * offset, double * slope) {
    DWORD dwIndex;
    DWORD dwBegin = (DWORD)(dwBeginWAV/fSizeFactor);
    DWORD dwEnd = (DWORD)(dwEndWAV/fSizeFactor);

    int n = 0;
    double sumX = 0;
    double sumY = 0;
    double sumXX = 0;
    double sumYY = 0;
    double sumXY = 0;

    BOOL bRes = TRUE;
    for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex++) {
        // get data for this pixel
        int nHere = pDoc->GetSmoothedPitch()->GetProcessedData(dwIndex, &bRes); // SDM 1.5Test11.0
        if (nHere > 0) {
            double Y = double(nHere)/PRECISION_MULTIPLIER;
            double X = double(dwIndex-dwBegin)*fSizeFactor;

            sumX += X;
            sumXX += X*X;
            sumY += Y;
            sumXY += X*Y;
            sumYY += Y*Y;

            n++;
        }
    }
    if (n>0) {
        double localSlope = (n*sumXY - sumX*sumY)/(n*sumXX - sumX*sumX);
        double localOffset = sumY/n - localSlope*sumX/n;

        if (offset) {
            *offset = localOffset;
        }
        if (slope) {
            *slope = localSlope;
        }
    } else {
        if (offset) {
            *offset = - 1.;
        }
        if (slope) {
            *slope = 0;
        }
    }
}


struct SDictionaryList {
    TCHAR * description;
    char * path;
};

SDictionaryList Dictionary[] = {
	// filename placeholder
    _T("American English - Male"), "c:\\mbrola\\voices\\us2", 
    _T("American English - Female"), "c:\\mbrola\\voices\\us1",
    0,0  // Null terminated list
};


BOOL CDlgMbrola::OnInitDialog() {
    CPropertyPage::OnInitDialog();

	// for some reason the arrow keys do not seem to work on a property sheet ??
    m_cGrid.FakeArrowKeys(TRUE); 

    // Label Grid
    m_cGrid.SetTextMatrix(rowHeading,columnDescription, _T("ITEM #"));
    m_cGrid.SetTextMatrix(rowIpa,columnDescription, _T("IPA"));
    m_cGrid.SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,columnFirst,1, -1);
    m_cGrid.SetRowHeight(rowIpa, 500);
    m_cGrid.SetTextMatrix(rowDuration,columnDescription, _T("Duration (ms)"));
    m_cGrid.SetTextMatrix(rowPitchAvg,columnDescription, _T("Avg Pitch (Hz)"));
    m_cGrid.SetColWidth(columnDescription,0, 2500);

    for (int i=columnFirst; i<m_cGrid.GetCols(0); i++) {
        CString number;
        number.Format(_T("%d"), i);
        m_cGrid.SetTextMatrix(0,i, number);
    }

    for (int i=rowPitchMs; i<m_cGrid.GetRows(); i++) {
        CString label;
        label.Format(_T("Pitch @ %gms (Hz)"), (i-rowPitchMs)*m_fPitchUpdateInterval*1000.);
        m_cGrid.SetTextMatrix(i,columnDescription, label);
    }

    // Label Output Grid
    m_cOutputGrid.SetTextMatrix(rowHeading,columnDescription, _T("ITEM #"));
    m_cOutputGrid.SetTextMatrix(rowSampa,columnDescription, _T("SAMPA"));
    // m_cOutputGrid.SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,columnFirst,1, -1);
    m_cOutputGrid.SetRowHeight(rowSampa, 500);
    m_cOutputGrid.SetTextMatrix(rowDuration,columnDescription, _T("Duration (ms)"));
    m_cOutputGrid.SetTextMatrix(rowPitchAvg,columnDescription, _T("Avg Pitch (Hz)"));
    m_cOutputGrid.SetColWidth(columnDescription,0, 2500);

    for (int i=columnFirst; i<m_cOutputGrid.GetCols(0); i++) {
        CString number;
        number.Format(_T("%d"), i);
        m_cOutputGrid.SetTextMatrix(0,i, number);
    }

    for (int i=rowPitchMs; i<m_cOutputGrid.GetRows(); i++) {
        CString label;
        label.Format(_T("Pitch @ %gms (Hz)"), (i-rowPitchMs)*m_fPitchUpdateInterval*1000.);
        m_cOutputGrid.SetTextMatrix(i,columnDescription, label);
    }

    // Populate Dictionary
    for (int i=0; Dictionary[i].description; i++) {
        m_cDictionary.AddString(Dictionary[i].description);
    }
    m_cDictionary.SetCurSel(0);

    OnDropdownSource(); // Populate Source

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgMbrola::OnLeaveCellMbrolaGrid() {
    // validate cell contents
    switch (m_cGrid.GetRow()) {
    case 2:
    case 3:
        double value;
        char dummy[3];
        CString cellText = m_cGrid.GetText();
        if ((cellText.GetLength()) && (swscanf(cellText,_T("%f%2s"), &value, &dummy) != 1)) {
            CString error;
            error.Format(_T("\"%s\" is not a number. Please correct"), LPCTSTR(cellText));
            AfxMessageBox(error, MB_OK | MB_ICONEXCLAMATION, 0);
            // data invalid - notify user and stay here
            m_cGrid.InvalidData();
        }
    }
}


void CDlgMbrola::OnMbrolaGet() {
    m_bGetComplete = FALSE;
    UpdateData(TRUE);

    CString szFilename;

    int nSource = m_cSource.GetCurSel();
    if (nSource == CB_ERR) {
        return;
    }
    m_cSource.GetLBText(nSource, szFilename);

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pDoc = (CSaDoc *)pApp->IsFileOpened(szFilename);
    CSegment * pPhonetic = pDoc->GetSegment(PHONETIC);

    if (pPhonetic->IsEmpty()) { // no annotations
        return;
    }

    // process all flags
    CString szString;
    CString szCrLf = "\r\n";

    short int nResult;
    int column = columnFirst;
    enum {PITCH, CALCULATIONS};
    double fSizeFactor[CALCULATIONS];

    if (m_bGetPitch) { // formants need pitch info
        //CProcessGrappl* pPitch = pDoc->GetGrappl(); // SDM 1.5 Test 11.0
        CProcessSmoothedPitch * pPitch = pDoc->GetSmoothedPitch();
        nResult = LOWORD((short int)pPitch->Process(this, pDoc)); // process data
        if (nResult == PROCESS_ERROR) {
            m_bGetPitch = FALSE;
        } else if (nResult == PROCESS_CANCELED) {
            return;
        } else {
            fSizeFactor[PITCH] = (double)pDoc->GetDataSize() / (double)(pPitch->GetDataSize() - 1);
        }
    }

    DWORD dwOffset = 0;
    DWORD dwPrevOffset;
    DWORD dwDuration = 0;
    int   nIndex = 0;
    const DWORD dwMinSilence = pDoc->GetBytesFromTime(0.0005);

    // construct table entries
    while (nIndex != -1) {
        dwPrevOffset = dwOffset;
        dwOffset = pPhonetic->GetOffset(nIndex);
        if (dwPrevOffset + dwDuration + dwMinSilence < dwOffset) {
            if (m_bGetIPA) {
                szString.Format(_T("silence"));
                m_cGrid.SetTextMatrix(rowIpa,column,szString);
            }
            if (m_bGetDuration) {
                dwDuration = dwOffset - (dwPrevOffset + dwDuration);
                szString.Format(_T("%.4f"),pDoc->GetTimeFromBytes(dwDuration)*1000.);
                m_cGrid.SetTextMatrix(rowDuration,column,szString);
            }
            column++;
        }
        szString = pPhonetic->GetSegmentString(nIndex);
        dwDuration = pPhonetic->GetDuration(nIndex);
        if (m_bGetIPA) {
            m_cGrid.SetTextMatrix(rowIpa,column,szString);
        }


        if (m_bGetDuration) {
            szString.Format(_T("%.4f"),pDoc->GetTimeFromBytes(dwDuration)*1000.);
            m_cGrid.SetTextMatrix(rowDuration,column,szString);
        }

        if (m_bGetPitch) {
            double offset;
            double slope;
            DWORD dwStop = pPhonetic->GetStop(nIndex);

            CurveFitPitch(pDoc, fSizeFactor[PITCH], dwOffset, pPhonetic->GetStop(nIndex), &offset, &slope);
            if (offset > 0) {
                szString.Format(_T("%.5g"),offset + slope*pPhonetic->GetDuration(nIndex)/2.);
            } else {
                szString.Empty();
            }
            m_cGrid.SetTextMatrix(rowPitchAvg,column,szString);

            DWORD dwUpdateInterval = pDoc->GetBytesFromTime(m_fPitchUpdateInterval/2)*2;
            int row = rowPitchMs;

            for (DWORD dwHere = dwOffset; dwHere < dwStop; dwHere += dwUpdateInterval) {
                DWORD dwBegin = dwHere;
                DWORD dwEnd = dwHere + dwUpdateInterval;

                if (dwEnd > dwStop) {
                    dwEnd = dwStop;
                }
                if (dwHere >= dwOffset + dwUpdateInterval) {
                    dwBegin -= dwUpdateInterval;
                }

                CurveFitPitch(pDoc, fSizeFactor[PITCH], dwBegin, dwEnd, &offset, &slope);
                if (offset > 0) {
                    szString.Format(_T("%.5g"),offset + slope*(dwHere-dwBegin));
                } else {
                    szString.Empty();
                }
                m_cGrid.SetTextMatrix(row,column,szString);
                row++;
            }
        }


        nIndex = pPhonetic->GetNext(nIndex);
        if (nIndex == -1) {
            column++;
            if (m_bGetIPA) {
                szString.Format(_T("silence"));
                m_cGrid.SetTextMatrix(rowIpa,column,szString);
            }
            if (m_bGetDuration) {
                dwDuration = pDoc->GetDataSize() - (dwOffset + dwDuration);
                szString.Format(_T("%.4f"),pDoc->GetTimeFromBytes(dwDuration)*1000.);
                m_cGrid.SetTextMatrix(rowDuration,column,szString);
            }
        }

        column++;
        if (column >= m_cGrid.GetCols(0)) {
            m_cGrid.SetCols(0, column+10);
            m_cGrid.SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,column,1, -1);
        }
    }

    // clear residual columns
    for (; column < m_cGrid.GetCols(0); column++) {
        szString.Empty();
        if (m_bGetIPA) {
            m_cGrid.SetTextMatrix(rowIpa,column,szString);
        }
        if (m_bGetDuration) {
            m_cGrid.SetTextMatrix(rowDuration,column,szString);
        }
        if (m_bGetPitch) {
            m_cGrid.SetTextMatrix(rowPitchAvg,column,szString);

            for (int i = rowPitchMs; i < m_cGrid.GetRows(); i++) {
                m_cGrid.SetTextMatrix(i,column,szString);
            }
        }
    }

    // select dictionary based on gender
    EGender nGender = pDoc->GetGender();
    if (nGender > female) {
        nGender = female;    // force to female since no dictionary exists for child
    }
    m_cDictionary.SetCurSel(nGender);

    m_bGetComplete = TRUE;
}


void CDlgMbrola::OnMbrolaConvert() {
    m_bConvertComplete = FALSE;

    if (!m_bGetComplete) {
        OnMbrolaGet();
    }
    if (!m_bGetComplete) {
        return;
    }

    for (int i=columnFirst; i<m_cGrid.GetCols(0); i++) {
        TCHAR * ptr; // unused result from _tcstod()

        CString IPA = m_cGrid.GetTextMatrix(rowIpa,i);

        if (!IPA.IsEmpty()) {
            if (i >= m_cOutputGrid.GetCols(0)) {
                m_cOutputGrid.SetCols(0,i+1);
            }


            CString SAMPA = (IPA == _T("silence"))?_T("_"):IpaToSampa(IPA);
            if (!SAMPA) {
                SAMPA = "n/a";
            }
            m_cOutputGrid.SetTextMatrix(rowSampa,i,SAMPA);
            if (SAMPA == "n/a") {
                continue;
            }

            CString durationText = m_cGrid.GetTextMatrix(rowDuration,i);
            double duration = _tcstod(durationText,&ptr)/1000.;

            CString pitchText = m_cGrid.GetTextMatrix(rowPitchAvg,i);
            double pitch = _tcstod(pitchText, &ptr);

            BOOL bCombine = FALSE;
            if (i < m_cGrid.GetCols(0) - 1) {
                CString NextIPA = m_cGrid.GetTextMatrix(rowIpa, i+1);
                bCombine = (IPA == "b|" && NextIPA == "b") ||
                           (IPA == "d|" && NextIPA == "d") ||
                           (IPA == "g|" && NextIPA == "g") ||
                           (IPA == "p|" && NextIPA == "p") ||
                           (IPA == "t|" && NextIPA == "t") ||
                           (IPA == "k|" && NextIPA == "k");
                if (bCombine) {
                    pitch = duration * pitch;
                    duration += _tcstod(m_cGrid.GetTextMatrix(rowDuration,i+1), &ptr)/1000.;
                    pitch += _tcstod(m_cGrid.GetTextMatrix(rowDuration,i+1), &ptr)/1000. *
                             _tcstod(m_cGrid.GetTextMatrix(rowPitchAvg,i+1), &ptr)/1000.;
                    pitch /= duration;
                    if (pitch > 0) {
                        pitchText.Format(_T("%.5g"), pitch);
                    }
                }
            }
            durationText.Format(_T("%d"), (int)(duration*1000.+0.5));
            m_cOutputGrid.SetTextMatrix(rowDuration,i,durationText);
            m_cOutputGrid.SetTextMatrix(rowPitchAvg,i,pitchText);

            // Copy pitch values
            durationText = m_cGrid.GetTextMatrix(rowDuration,i);
            duration = _tcstod(durationText,&ptr)/1000.;
            int rows = int(duration/m_fPitchUpdateInterval);
            BOOL bPitch = FALSE;
            for (int row = 0; row <= rows; row++) {
                pitchText = m_cGrid.GetTextMatrix(row + rowPitchMs,i);
                pitch = _tcstod(pitchText, &ptr);
                if (pitch > 0) {
                    pitchText.Format(_T("%d"), (int)(pitch+0.5));
                    if (!bPitch) {
                        for (int fillrow = 0; fillrow < row; fillrow++) {
                            m_cOutputGrid.SetTextMatrix(fillrow + rowPitchMs,i,pitchText);
                        }
                        bPitch = TRUE;
                    }
                    m_cOutputGrid.SetTextMatrix(row + rowPitchMs,i,pitchText);
                }
            }
            if (bCombine) {
                int appendRow = rows;
                durationText = m_cGrid.GetTextMatrix(rowDuration,i+1);
                duration = _tcstod(durationText,&ptr)/1000.;
                rows = int(duration/m_fPitchUpdateInterval);
                for (int row = 0; row <= rows; row++) {
                    pitchText = m_cGrid.GetTextMatrix(row + rowPitchMs,i+1);
                    pitch = _tcstod(pitchText, &ptr);
                    if (pitch > 0) {
                        pitchText.Format(_T("%d"), (int)(pitch+0.5));
                        m_cOutputGrid.SetTextMatrix(rowPitchMs + appendRow++,i,pitchText);
                    }
                }
                i++;
            }
        }
    }
    m_bConvertComplete = TRUE;
}


struct SMBRolaVector {
    CString Sampa;
    double AvgPitch;
    double Duration;
};

void CDlgMbrola::OnMbrolaSynthesize() {
    if (!m_bConvertComplete) {
        OnMbrolaConvert();
    }
    if (!m_bConvertComplete) {
        return;
    }

    int Voice = m_cDictionary.GetCurSel();
    int Err = init_MBR(Dictionary[Voice].path);
    if (Err) {
        reset_MBR();
    }
    setFreq_MBR(16000);

    // create and open the synthesis file
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    // create temp filename for synthesized waveform
    if (m_szMBRolaName.IsEmpty()) {
        FileUtils::GetTempFileName(_T("MBR"), m_szMBRolaName.GetBuffer(_MAX_PATH), _MAX_PATH);
        m_szMBRolaName.ReleaseBuffer();
    }


    HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(LPCTSTR(m_szMBRolaName)), NULL, MMIO_CREATE | MMIO_WRITE | MMIO_EXCLUSIVE);
    if (!(hmmioFile)) {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_FILEOPEN, m_szMBRolaName);
        return;
    }
    // create a 'RIFF' chunk with a 'WAVE' form type
    MMCKINFO mmckinfoParent;
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
    // set chunk size
    mmckinfoParent.cksize = 0;
    if (mmioCreateChunk(hmmioFile, &mmckinfoParent, MMIO_CREATERIFF)) { // create the 'RIFF' chunk
        // error creating RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, m_szMBRolaName);
        return;
    }
    // create the 'fmt ' subchunk
    MMCKINFO mmckinfoSubchunk;
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    // set chunk size
    mmckinfoSubchunk.cksize = 16;
    if (mmioCreateChunk(hmmioFile, &mmckinfoSubchunk, 0)) { // create the 'data' chunk
        // error creating format chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szMBRolaName);
        return;
    }

    // write the format parameters into 'fmt ' chunk
    CFmtParm format;
    format.wTag = 1;   // PCM
    format.wBitsPerSample = 16;
    format.wChannels = 1;
    format.wBlockAlign = 2;
    format.dwSamplesPerSec = 16000;
    format.dwAvgBytesPerSec = format.wBlockAlign*format.dwSamplesPerSec;

    long lError = mmioWrite(hmmioFile, (HPSTR)&format.wTag, sizeof(WORD));
    if (lError != -1) {
        lError = mmioWrite(hmmioFile, (HPSTR)&format.wChannels, sizeof(WORD));
    }
    if (lError != -1) {
        lError = mmioWrite(hmmioFile, (HPSTR)&format.dwSamplesPerSec, sizeof(DWORD));
    }
    if (lError != -1) {
        lError = mmioWrite(hmmioFile, (HPSTR)&format.dwAvgBytesPerSec, sizeof(DWORD));
    }
    if (lError != -1) {
        lError = mmioWrite(hmmioFile, (HPSTR)&format.wBlockAlign, sizeof(WORD));
    }
    if (lError != -1) {
        lError = mmioWrite(hmmioFile, (HPSTR)&format.wBitsPerSample, sizeof(WORD));
    }
    if (lError == -1) {
        // error writing format chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szMBRolaName);
        return;
    }

    // get out of 'fmt ' chunk
    if (mmioAscend(hmmioFile, &mmckinfoSubchunk, 0)) {
        // error writing format chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szMBRolaName);
        return;
    }
    // create the 'data' subchunk
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    // set chunk size
    mmckinfoSubchunk.cksize = 0;
    if (mmioCreateChunk(hmmioFile, &mmckinfoSubchunk, 0)) { // create the 'data' chunk
        // error creating data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szMBRolaName);
        return;
    }

    static short MbrolaDataBuffer[16384];
    DWORD dwMbrolaDataBlockSize;
    CString szSegmentParms;

    for (int i=columnFirst; i<m_cGrid.GetCols(0); i++) {
        SMBRolaVector thisColumn;
        TCHAR * ptr; // unused result from _tcstod()

        thisColumn.Sampa = m_cOutputGrid.GetTextMatrix(rowSampa,i);

        if (!thisColumn.Sampa.IsEmpty()) {
            // There is an Sampa character assume valid column - write vectors to file

            CString pitchText = m_cOutputGrid.GetTextMatrix(rowPitchAvg,i);
            thisColumn.AvgPitch = _tcstod(pitchText,&ptr);

            CString durationText = m_cOutputGrid.GetTextMatrix(rowDuration,i);
            thisColumn.Duration = _tcstod(durationText,&ptr);

            szSegmentParms.Empty();
            szSegmentParms.Format(_T("%s %d "), thisColumn.Sampa, (int)thisColumn.Duration);

            CString szPitchContour;
            int nPitchPoints = (int)((thisColumn.Duration/1000.)/m_fPitchUpdateInterval);
            for (int pitch = 0; pitch < nPitchPoints; pitch++) {
                CString szPitch = m_cOutputGrid.GetTextMatrix(pitch + rowPitchMs,i);
                if (!szPitch.IsEmpty()) {
                    int percent = int(pitch*m_fPitchUpdateInterval*1000./thisColumn.Duration*100 + 0.5);
                    szPitchContour.Format(_T("%d %s "), percent, szPitch);
                    szSegmentParms += szPitchContour;
                }
            }
            szSegmentParms += "\n";
            int Err = write_MBR((char *)((LPCTSTR)szSegmentParms));
            UNUSED_ALWAYS(Err);
            dwMbrolaDataBlockSize = read_MBR(MbrolaDataBuffer, 16384) * sizeof(MbrolaDataBuffer[0]);
            do {
                if (dwMbrolaDataBlockSize < 0) {
                    static char ErrMessage[256];
                    lastErrorStr_MBR(ErrMessage, sizeof(ErrMessage));
                    reset_MBR();
                    return;
                }
                if (mmioWrite(hmmioFile, (HPSTR)MbrolaDataBuffer, (long)dwMbrolaDataBlockSize) == -1) {
                    // error writing wave file
                    pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szMBRolaName);
                    mmioClose(hmmioFile, 0);
                    return;
                }
            } while (dwMbrolaDataBlockSize == sizeof(MbrolaDataBuffer));
        }
    }
    flush_MBR();
    do {
        dwMbrolaDataBlockSize = read_MBR(MbrolaDataBuffer, sizeof(MbrolaDataBuffer)) * sizeof(MbrolaDataBuffer[0]);
        if (mmioWrite(hmmioFile, (HPSTR)MbrolaDataBuffer, (long)dwMbrolaDataBlockSize) == -1) {
            // error writing wave file
            pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szMBRolaName);
            mmioClose(hmmioFile, 0);
            return;
        }
    } while (dwMbrolaDataBlockSize == sizeof(MbrolaDataBuffer));
    // get out of 'data' chunk
    if (mmioAscend(hmmioFile, &mmckinfoSubchunk, 0)) {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szMBRolaName);
        mmioClose(hmmioFile, 0);
        return;
    }
    // get out of 'RIFF' chunk, to write RIFF size
    if (mmioAscend(hmmioFile, &mmckinfoParent, 0)) {
        // error writing RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, m_szMBRolaName);
        mmioClose(hmmioFile, 0);
        return;
    }
    mmioClose(hmmioFile, 0); // close file
}

void CDlgMbrola::OnMbrolaPlaySynth() {
    OnMbrolaSynthesize();
    if (m_szMBRolaName.GetLength()) {
        PlaySound(m_szMBRolaName, 0, SND_SYNC | SND_NODEFAULT | SND_FILENAME);
    }
}

void CDlgMbrola::OnMbrolaPlaySource() {
    CString szFilename;
    int nSource = m_cSource.GetCurSel();
    if (nSource == CB_ERR) {
        return;
    }
    m_cSource.GetLBText(nSource, szFilename);

    PlaySound(szFilename, 0, SND_SYNC | SND_NODEFAULT | SND_FILENAME);
}

void CDlgMbrola::OnMbrolaPlay() {
    OnMbrolaPlaySynth();

    OnMbrolaPlaySource();
}


void CDlgMbrola::OnMbrolaDisplay() {
    if (m_szMBRolaName.IsEmpty()) {
        OnMbrolaSynthesize();
    }

    // open synthesized wavefile in SA
    CFileStatus status; // file status
    if (CFile::GetStatus(m_szMBRolaName, status)) {
        if (status.m_size) {
            // file created open in SA
            CSaApp * pApp = (CSaApp *)(AfxGetApp());

            pApp->OpenWavFileAsNew(m_szMBRolaName);
            m_szMBRolaName.Empty();
        }
    }
}

void CDlgMbrola::OnDropdownSource() {
    CString szFilename;
    int nSource = m_cSource.GetCurSel();
    if (nSource != CB_ERR) {
        m_cSource.GetLBText(nSource, szFilename);
    }

    m_cSource.ResetContent();

    // Populate Source
    CDocTemplate * pDocTemplate;
    POSITION docPosition = AfxGetApp()->GetFirstDocTemplatePosition();
    pDocTemplate = AfxGetApp()->GetNextDocTemplate(docPosition);
    POSITION position = pDocTemplate->GetFirstDocPosition();
    while (position != NULL) {
        CDocument * pDoc = pDocTemplate->GetNextDoc(position); // get pointer to document
        CString path = pDoc->GetPathName();
        if (!path.IsEmpty()) {
            m_cSource.AddString(path);
        }
    }
    m_cSource.SetCurSel(0);
    if (!szFilename.IsEmpty()) {
        int sel = m_cSource.FindStringExact(0,szFilename);
        if (sel != CB_ERR) {
            m_cSource.SetCurSel(sel);
        }
    } else {
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        if (pMDIFrameWnd) {
            CView * pView = pMDIFrameWnd->GetCurrSaView();
            if (pView && !pView->GetDocument()->GetPathName().IsEmpty()) {
                m_cSource.SelectString(-1,pView->GetDocument()->GetPathName());
            }
        }
    }

}
