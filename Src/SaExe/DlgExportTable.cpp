#include "stdafx.h"
#include "DlgExportTable.h"
#include "CSaString.h"
#include "Sa_Doc.h"
#include "SA_View.h"
#include "Sa_segm.h"
#include "MainFrm.h"
#include "GlossSegment.h"
#include "PhoneticSegment.h"
#include "DlgExportFW.h"
#include "FileUtils.h"
#include "Process\sa_proc.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_pit.h"
#include "Process\sa_p_cpi.h"
#include "Process\sa_p_spi.h"
#include "Process\sa_p_mel.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_spg.h"
#include "Process\FormantTracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExportTable dialog

CDlgExportTable::CDlgExportTable(const CSaString & szDocTitle, CWnd * pParent /*=NULL*/) : 
CDialog(CDlgExportTable::IDD, pParent) {

	m_bAnnotations = TRUE;
    m_bCalculations = FALSE;
    m_bF1 = FALSE;
    m_bF2 = FALSE;
    m_bF3 = FALSE;
    m_bF4 = FALSE;
    m_bFormants = FALSE;
    m_bGloss = TRUE;
    m_szIntervalTime = "20";
    m_bSegmentLength = TRUE;
    m_bMagnitude = TRUE;
    m_bOrtho = TRUE;
    m_bPhonemic = TRUE;
    m_bPhonetic = TRUE;
    m_bPitch = TRUE;
    m_bPOS = FALSE;
    m_bReference = FALSE;
    m_bSegmentStart = TRUE;
    m_bSampleTime = TRUE;
    m_bTone = FALSE;
    m_bZeroCrossings = TRUE;
    m_nSampleRate = 0;
    m_nCalculationMethod = 1;
    m_bPhonetic2 = TRUE;
    m_nRegion = 0;
    m_bMelogram = TRUE;
    m_szDocTitle = szDocTitle;

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    if (pDoc->GetSegment(PHONETIC)->IsEmpty()) { // no annotations
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = FALSE;
        m_bSegmentStart = m_bSegmentLength = FALSE; // no segments
        m_nSampleRate = 1;
    }
}

void CDlgExportTable::DoDataExchange(CDataExchange * pDX) {
    
	CDialog::DoDataExchange(pDX);
    
	DDX_Check(pDX, IDC_EXTAB_ANNOTATIONS, m_bAnnotations);
    DDX_Check(pDX, IDC_EXTAB_CALC, m_bCalculations);
    DDX_Check(pDX, IDC_EXTAB_F1, m_bF1);
    DDX_Check(pDX, IDC_EXTAB_F2, m_bF2);
    DDX_Check(pDX, IDC_EXTAB_F3, m_bF3);
    DDX_Check(pDX, IDC_EXTAB_F4, m_bF4);
    DDX_Check(pDX, IDC_EXTAB_FORMANTS, m_bFormants);
    DDX_Check(pDX, IDC_EXTAB_GLOSS, m_bGloss);
    DDX_Text(pDX, IDC_EXTAB_INTERVAL_TIME, m_szIntervalTime);
    DDV_MaxChars(pDX, m_szIntervalTime, 4);
    DDX_Check(pDX, IDC_EXTAB_LENGTH, m_bSegmentLength);
    DDX_Check(pDX, IDC_EXTAB_MAGNITUDE, m_bMagnitude);
    DDX_Check(pDX, IDC_EXTAB_ORTHO, m_bOrtho);
    DDX_Check(pDX, IDC_EXTAB_PHONEMIC, m_bPhonemic);
    DDX_Check(pDX, IDC_EXTAB_PHONETIC, m_bPhonetic);
    DDX_Check(pDX, IDC_EXTAB_PITCH, m_bPitch);
    DDX_Check(pDX, IDC_EXTAB_POS, m_bPOS);
    DDX_Check(pDX, IDC_EXTAB_REFERENCE, m_bReference);
    DDX_Check(pDX, IDC_EXTAB_START, m_bSegmentStart);
    DDX_Check(pDX, IDC_EXTAB_TIME, m_bSampleTime);
    DDX_Check(pDX, IDC_EXTAB_TONE, m_bTone);
    DDX_Check(pDX, IDC_EXTAB_ZERO, m_bZeroCrossings);
    DDX_Radio(pDX, IDC_EXTAB_SEGMENT, m_nSampleRate);
    DDX_Radio(pDX, IDC_EXTAB_MIDPOINT, m_nCalculationMethod);
    DDX_Check(pDX, IDC_EXTAB_PHONETIC2, m_bPhonetic2);
    DDX_Radio(pDX, IDC_REGION_BETWEEN, m_nRegion);
    DDX_Check(pDX, IDC_EXTAB_MELOGRAM, m_bMelogram);
}

BEGIN_MESSAGE_MAP(CDlgExportTable, CDialog)
    ON_BN_CLICKED(IDC_EXTAB_ANNOTATIONS, OnAllAnnotations)
    ON_BN_CLICKED(IDC_EXTAB_FORMANTS, OnAllFormants)
    ON_BN_CLICKED(IDC_EXTAB_CALC, OnAllCalculations)
    ON_EN_UPDATE(IDC_EXTAB_INTERVAL_TIME, OnUpdateIntervalTime)
    ON_BN_CLICKED(IDC_EXTAB_INTERVAL, OnSample)
    ON_BN_CLICKED(IDC_EXTAB_SEGMENT, OnSample)
    ON_BN_CLICKED(IDC_EXTAB_PHONETIC2, OnPhonetic)
    ON_BN_CLICKED(IDC_EXTAB_PHONETIC, OnPhonetic)
    ON_COMMAND(IDHELP, OnHelpExportTable)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDlgExportTable message handlers

void CDlgExportTable::OnAllAnnotations() {
    
	UpdateData(TRUE);
    if (m_nSampleRate == 0) {
        SetVisible(IDC_EXTAB_TIME, FALSE);
        SetVisible(IDC_EXTAB_START, TRUE);
        SetVisible(IDC_EXTAB_LENGTH, TRUE);
        SetVisible(IDC_EXTAB_PHONETIC2, FALSE);
        SetVisible(IDC_EXTAB_ANNOTATIONS, TRUE, TRUE);
        BOOL bEnable = !m_bAnnotations;
        SetVisible(IDC_EXTAB_PHONETIC, TRUE, bEnable);
        SetVisible(IDC_EXTAB_TONE, TRUE, bEnable);
        SetVisible(IDC_EXTAB_PHONEMIC, TRUE, bEnable);
        SetVisible(IDC_EXTAB_ORTHO, TRUE, bEnable);
        SetVisible(IDC_EXTAB_GLOSS, TRUE, bEnable);
        SetVisible(IDC_EXTAB_REFERENCE, TRUE, bEnable);
        SetVisible(IDC_EXTAB_POS, TRUE, bEnable);
    } else {
        SetVisible(IDC_EXTAB_TIME, TRUE);
        SetVisible(IDC_EXTAB_START, FALSE);
        SetVisible(IDC_EXTAB_LENGTH, FALSE);
        SetVisible(IDC_EXTAB_PHONETIC2, TRUE);
        SetVisible(IDC_EXTAB_ANNOTATIONS, FALSE);
        SetVisible(IDC_EXTAB_PHONETIC, FALSE);
        SetVisible(IDC_EXTAB_TONE, FALSE);
        SetVisible(IDC_EXTAB_PHONEMIC, FALSE);
        SetVisible(IDC_EXTAB_ORTHO, FALSE);
        SetVisible(IDC_EXTAB_GLOSS, FALSE);
        SetVisible(IDC_EXTAB_REFERENCE, FALSE);
        SetVisible(IDC_EXTAB_POS, FALSE);
    }
    if (m_bAnnotations) {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = TRUE;

        UpdateData(FALSE);
    }
}

void CDlgExportTable::OnAllFormants() {
    
	UpdateData(TRUE);
    BOOL bEnable = !m_bFormants;
    SetEnable(IDC_EXTAB_F1, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F2, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F3, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F4, bEnable && !m_bFormants);
    if (m_bFormants) {
        m_bF1 = m_bF2 = m_bF3 = m_bF4 = TRUE;

        UpdateData(FALSE);
    }
}

void CDlgExportTable::OnAllCalculations() {
    
	UpdateData(TRUE);
    BOOL bEnable = !m_bCalculations;
    SetEnable(IDC_EXTAB_FORMANTS, bEnable);
    SetEnable(IDC_EXTAB_F1, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F2, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F3, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F4, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_MAGNITUDE, bEnable);
    SetEnable(IDC_EXTAB_PITCH, bEnable);
    SetEnable(IDC_EXTAB_MELOGRAM, bEnable);
    SetEnable(IDC_EXTAB_ZERO, bEnable);
    if (m_bCalculations) {
        m_bMagnitude = m_bZeroCrossings = m_bPitch = m_bMelogram  = m_bFormants = TRUE;
        UpdateData(FALSE);

        OnAllFormants();
    }
}

void CDlgExportTable::OnOK() {
    
	wstring filename;
    int result = GetSaveAsFilename(m_szDocTitle, _T("SFM Time Table (*.sft) |*.sft||"), _T("sft"), NULL, filename);
    if (result!=IDOK) {
        return;
    }
    m_szFileName = filename.c_str();
    if (m_szFileName == "") {
        return;
    }

    UpdateData(TRUE);
    // process all flags
    if (m_bAnnotations) {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = TRUE;
    }
    if (m_bCalculations) {
        m_bMagnitude = m_bZeroCrossings = m_bPitch = m_bMelogram = m_bFormants = TRUE;
    }
    if (m_bFormants) {
        m_bF1 = m_bF2 = m_bF3 = m_bF4 = TRUE;
    }
    if (m_nSampleRate == 0) {
        m_bSampleTime = FALSE;
    } else {
        m_bReference = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = FALSE;
        m_bSegmentStart = m_bSegmentLength = FALSE;
    }

    CSaString szString;
    CSaString szFTFormantString = "";
    CSaString szCrLf = "\r\n";
    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos); // get pointer to view
    CSegment * pPhonetic = pDoc->GetSegment(PHONETIC);

    if (pPhonetic->IsEmpty()) { // no annotations
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = FALSE;
        m_bSegmentStart = m_bSegmentLength = FALSE; // no segments
        if (m_nSampleRate == 0) {
            m_nSampleRate = 1;
        }
    }

    DWORD dwOffset = pView->GetStartCursorPosition();
    DWORD dwStopPosition = pView->GetStopCursorPosition();
    if (m_nRegion != 0) { // entire file
        dwOffset = 0;
        dwStopPosition = pDoc->GetDataSize() - pDoc->GetFmtParm()->wBlockAlign;
    }

    int nIndex = 0;

    if (m_nSampleRate != 1) { // phonetic sampling
        nIndex = pPhonetic->FindFromPosition(dwStopPosition);

        if ((nIndex != -1) && dwStopPosition < pPhonetic->GetOffset(nIndex)) {
            nIndex = pPhonetic->GetPrevious(nIndex);
        }

        if ((nIndex != -1)  && dwStopPosition < pPhonetic->GetStop(nIndex)) {
            dwStopPosition = pPhonetic->GetStop(nIndex);
        }
    }

    nIndex = pPhonetic->FindFromPosition(dwOffset);

    if (m_nSampleRate != 1) { // phonetic sampling
        if (nIndex != -1) {
            dwOffset = pPhonetic->GetOffset(nIndex);
        }
    }

    DWORD dwNext = 0;
    DWORD dwIncrement = 0;

    if (m_nSampleRate == 1) { // interval sampling
        int nInterval = 20;
        if (m_szIntervalTime.GetLength() != 0) {
            swscanf_s(m_szIntervalTime, _T("%d"), &nInterval);
        }
        if (nInterval < 1) {
            nInterval = 20;
        }
        dwIncrement = pDoc->GetBytesFromTime(nInterval/1000.0);
        if (dwIncrement < 1) {
            dwIncrement++;
        }
        if (pDoc->GetFmtParm()->wBlockAlign == 2) {
            dwIncrement++;
            dwIncrement &= ~1;
        }
        swprintf_s(m_szIntervalTime.GetBuffer(20),20, _T("%d"), nInterval);
        m_szIntervalTime.ReleaseBuffer();
    }

    short int nResult;
    enum {MAG, PITCH, MELOGRAM, ZCROSS, FMTTRACKER, CALCULATIONS};
    double fSizeFactor[CALCULATIONS];

    if (m_bMagnitude) {
        CProcessLoudness * pLoudness = (CProcessLoudness *)pDoc->GetLoudness(); // get pointer to loudness object
        nResult = LOWORD(pLoudness->Process(this, pDoc)); // process data
        if (nResult == PROCESS_ERROR) {
            m_bMagnitude = FALSE;
        } else if (nResult == PROCESS_CANCELED) {
            return;
        } else {
            fSizeFactor[MAG] = (double)pDoc->GetDataSize() / (double)(pLoudness->GetDataSize() - 1);
        }
    }
    if (m_bPitch || m_bF1 || m_bF2 || m_bF3 || m_bF4) { // formants need pitch info
        CProcessGrappl * pPitch = pDoc->GetGrappl(); // SDM 1.5 Test 11.0
        // We also want raw and smoothed
        CProcessPitch * pRawPitch = pDoc->GetPitch();
        CProcessSmoothedPitch * pSmoothedPitch = pDoc->GetSmoothedPitch();
        nResult = LOWORD(pPitch->Process(this, pDoc)); // process data
        if (nResult == PROCESS_ERROR) {
            m_bPitch = FALSE;
        } else if (nResult == PROCESS_CANCELED) {
            return;
        }
        nResult = LOWORD(pRawPitch->Process(this, pDoc));
        if (nResult == PROCESS_ERROR) {
            m_bPitch = m_bPitch && FALSE;
        } else if (nResult == PROCESS_CANCELED) {
            return;
        }
        nResult = LOWORD(pSmoothedPitch->Process(this, pDoc));
        if (nResult == PROCESS_ERROR) {
            m_bPitch = m_bPitch && FALSE;
        } else if (nResult == PROCESS_CANCELED) {
            return;
        } else {
            fSizeFactor[PITCH] = (double)pDoc->GetDataSize() / (double)(pPitch->GetDataSize() - 1);
        }
    }
    if (m_bMelogram) {
        CProcessMelogram * pMelogram = pDoc->GetMelogram(); // SDM 1.5 Test 11.0
        nResult = LOWORD(pMelogram->Process(this, pDoc)); // process data
        if (nResult == PROCESS_ERROR) {
            m_bMelogram = FALSE;
        } else if (nResult == PROCESS_CANCELED) {
            return;
        } else {
            fSizeFactor[MELOGRAM] = (double)pDoc->GetDataSize() / (double)(pMelogram->GetDataSize() - 1);
        }
    }
    if (m_bZeroCrossings || m_bF1 || m_bF2 || m_bF3 || m_bF4) {
        CProcessZCross  * pZCross = pDoc->GetZCross();
        nResult = LOWORD(pZCross->Process(this, pDoc)); // process data
        if (nResult == PROCESS_ERROR) {
            m_bZeroCrossings = FALSE;
        } else if (nResult == PROCESS_CANCELED) {
            return;
        } else {
            fSizeFactor[ZCROSS] = (double)pDoc->GetDataSize() / (double)(pZCross->GetDataSize() - 1);
        }
    }
    if (m_bF1 || m_bF2 || m_bF3 || m_bF4) {
        CFormantTracker * pSpectroFormants = pDoc->GetFormantTracker();

        // If pitch processed successfully, generate formant data.
        if (pDoc->GetGrappl()->IsDataReady()) {
            short int nResult = LOWORD(pSpectroFormants->Process(pView, pDoc));
            if (nResult == PROCESS_ERROR) {
                m_bF1 = m_bF2 = m_bF3 = m_bF4 = FALSE;
            } else if (nResult == PROCESS_CANCELED) {
                return;
            } else {
                fSizeFactor[FMTTRACKER] = (double)pDoc->GetDataSize() / (double)(pSpectroFormants->GetDataSize() - 1);
            }
        } else {
            m_bF1 = m_bF2 = m_bF3 = m_bF4 = FALSE;
        }
    }

    CFile * pFile = new CFile(m_szFileName, CFile::modeCreate|CFile::modeWrite);

    // write header

    // \name write filename
    szString = "\\name " + m_szFileName + szCrLf;
    WriteFileUtf8(pFile, szString);

    // \date write current time
    CTime time = CTime::GetCurrentTime();
    szString = "\\date " + time.Format("%A, %B %d, %Y, %X") + "\r\n";
    WriteFileUtf8(pFile, szString);

    // \wav  Audio FileName
    szString = "\\wav " + pDoc->GetPathName() + szCrLf;
    WriteFileUtf8(pFile, szString);

    // \calc calculation method
    if (m_nCalculationMethod == 0) {
        szString = "\\calc "  "midpoint" + szCrLf;
    } else {
        szString = "\\calc "  "average" + szCrLf;
    }
    WriteFileUtf8(pFile, szString);

    if (m_bSampleTime) {
        szString = "\\table time " "every "+ m_szIntervalTime + "ms" "\r\nTime\t";
    } else {
        szString = "\\table Etic\r\n";
    }
    if (m_bSegmentStart) {
        szString += "Start\t";
    }
    if (m_bSegmentLength) {
        szString += "Length\t";
    }
    if (m_bReference) {
        szString += "Ref\t";
    }
    if (m_bPhonetic) {
        szString += "Etic\t";
    }
    if (m_bTone) {
        szString += "Tone\t";
    }
    if (m_bPhonemic) {
        szString += "Emic\t";
    }
    if (m_bOrtho) {
        szString += "Ortho\t";
    };
    if (m_bGloss) {
        szString += "Gloss\t";
    }
    if (m_bPOS) {
        szString += "POS\t";
    }

    if (m_bMagnitude) {
        szString += "Int(dB)\t";
    }
    if (m_bPitch) {
        szString += "Pitch(Hz)\tRawPitch\tSmPitch\t";
    }
    if (m_bMelogram) {
        szString += "Melogram(st)\t";
    }
    if (m_bZeroCrossings) {
        szString += "ZCross\t";
    }
    if (m_bF1) {
        szString += "F1(Hz)\t";
    }
    if (m_bF2) {
        szString += "F2(Hz)\t";
    }
    if (m_bF3) {
        szString += "F3(Hz)\t";
    }
    if (m_bF4) {
        szString += "F4(Hz)\t";
    }
    szString += "\r\n";
    WriteFileUtf8(pFile, szString);

    // construct table entries
    while (dwOffset < dwStopPosition) {
        if (m_nSampleRate == 1) { // interval sampling
            dwNext = dwOffset+dwIncrement;
        } else { // phonetic segment samples
            if (nIndex != -1) {
                dwNext = pPhonetic->GetStop(nIndex);
            } else {
                dwNext = pDoc->GetUnprocessedDataSize();
            }
        }

        if (m_bSampleTime) {
            swprintf_s(szString.GetBuffer(25),25,_T("%.3f\t"),pDoc->GetTimeFromBytes(dwOffset));
            szString.ReleaseBuffer();
            WriteFileUtf8(pFile, szString);
        }

        if ((m_nSampleRate==0) && (nIndex != -1) && (pPhonetic->GetOffset(nIndex) < dwNext)) {
            DWORD dwPhonetic = pPhonetic->GetOffset(nIndex);
            if (m_bSegmentStart) {
                swprintf_s(szString.GetBuffer(25),25,_T("%.3f\t"),pDoc->GetTimeFromBytes(dwPhonetic));
                szString.ReleaseBuffer();
                WriteFileUtf8(pFile, szString);
            }
            if (m_bSegmentLength) {
                swprintf_s(szString.GetBuffer(25),25,_T("%.3f\t"),pDoc->GetTimeFromBytes(pPhonetic->GetDuration(nIndex)));
                szString.ReleaseBuffer();
                WriteFileUtf8(pFile, szString);
            }

            if (m_bReference) {
                int nIndex = pDoc->GetSegment(REFERENCE)->FindOffset(dwPhonetic);
                if (nIndex != -1) {
                    szString = pDoc->GetSegment(REFERENCE)->GetSegmentString(nIndex) + "\t";
                } else {
                    szString = "\t";
                }
                WriteFileUtf8(pFile, szString);
            }
            if (m_bPhonetic) {
                szString = pPhonetic->GetSegmentString(nIndex) + "\t";
                WriteFileUtf8(pFile, szString);
            }
            if (m_bTone) {
                int nIndex = pDoc->GetSegment(TONE)->FindOffset(dwPhonetic);
                if (nIndex != -1) {
                    szString = pDoc->GetSegment(TONE)->GetSegmentString(nIndex) + "\t";
                } else {
                    szString = "\t";
                }
                WriteFileUtf8(pFile, szString);
            }
            if (m_bPhonemic) {
                int nIndex = pDoc->GetSegment(PHONEMIC)->FindOffset(dwPhonetic);
                if (nIndex != -1) {
                    szString = pDoc->GetSegment(PHONEMIC)->GetSegmentString(nIndex) + "\t";
                } else {
                    szString = "\t";
                }
                WriteFileUtf8(pFile, szString);
            }
            if (m_bOrtho) {
                int nIndex = pDoc->GetSegment(ORTHO)->FindOffset(dwPhonetic);
                if (nIndex != -1) {
                    szString = pDoc->GetSegment(ORTHO)->GetSegmentString(nIndex) + "\t";
                } else {
                    szString = "\t";
                }
                WriteFileUtf8(pFile, szString);
            }
            if (m_bGloss) {
                int nIndex = pDoc->GetSegment(GLOSS)->FindOffset(dwPhonetic);
                if (nIndex != -1) {
                    // SDM 1.5Test10.1
                    szString = pDoc->GetSegment(GLOSS)->GetSegmentString(nIndex);
                    if ((szString.GetLength() > 1)&&(szString[0] == WORD_DELIMITER)) {
                        szString = szString.Mid(1);    // Remove Word Delimiter
                    }
                    szString += "\t";
                } else {
                    szString = "\t";
                }
                WriteFileUtf8(pFile, szString);
                if (m_bPOS) {
                    if (nIndex != -1) {
                        szString = ((CGlossSegment *) pDoc->GetSegment(GLOSS))->GetPOSs()->GetAt(nIndex) + "\t";
                    } else {
                        szString = "\t";
                    }
                    WriteFileUtf8(pFile, szString);
                }
            }
            nIndex = pPhonetic->GetNext(nIndex);
        } else if (m_bPhonetic && (m_nSampleRate==1) && !pPhonetic->IsEmpty()) {
            nIndex = 0;

            while ((nIndex != -1) && (pPhonetic->GetStop(nIndex) < dwOffset)) {
                nIndex = pPhonetic->GetNext(nIndex);
            }

            if ((nIndex != -1) && pPhonetic->GetOffset(nIndex) < dwNext) { // this one overlaps
                int nLast = pPhonetic->GetNext(nIndex);
                szString = pPhonetic->GetSegmentString(nIndex);

                while ((nLast != -1) && (pPhonetic->GetOffset(nLast) < dwNext)) {
                    szString += " " + pPhonetic->GetSegmentString(nLast);
                    nLast = pPhonetic->GetNext(nLast);
                }
                szString +="\t";
            } else {
                szString = "\t";
            }
            WriteFileUtf8(pFile, szString);
        }

        DWORD dwBegin;
        DWORD dwEnd;
        DWORD dwCalcIncrement;
        DWORD dwIndex;
        if (m_nCalculationMethod == 0) {
            dwBegin = dwEnd = (dwOffset + dwNext)/2;
            dwEnd++;
            dwCalcIncrement = 10;
        } else {
            dwBegin = dwOffset;
            dwEnd = dwNext;
            dwCalcIncrement = (dwEnd - dwBegin)/20;
            if (!dwCalcIncrement) {
                dwCalcIncrement = 1;
            }
        }

        if (m_bMagnitude) {
            int dwSamples = 0;
            BOOL bRes = TRUE;
            double fData = 0;
            for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
                DWORD dwProcData = (DWORD)(dwIndex/fSizeFactor[MAG]);
                // get data for this pixel
                fData += pDoc->GetLoudness()->GetProcessedData(dwProcData, &bRes);
                dwSamples++;
            }
            if (dwSamples && bRes) {
                fData = fData/ dwSamples;
                double fLoudnessMax = pDoc->GetLoudness()->GetMaxValue();
                if (fData*10000. < fLoudnessMax) {
                    fData = fLoudnessMax/10000.;
                }

                double db = 20.0 * log10(fData/32767.) + 9.;  // loudness is rms full scale would be 9dB over recommended recording level
                swprintf_s(szString.GetBuffer(25),25,_T("%0.1f\t"),db);
                szString.ReleaseBuffer();
            } else {
                szString = "\t";
            }
            WriteFileUtf8(pFile, szString);
        }
        if (m_bPitch) {
            int dwSamples = 0;
            BOOL bRes = TRUE;
            long nData = 0;
            for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
                DWORD dwProcData = (DWORD)(dwIndex/fSizeFactor[PITCH]);
                // get data for this pixel
                int nHere = pDoc->GetGrappl()->GetProcessedData(dwProcData, &bRes); // SDM 1.5Test11.0
                if (nHere > 0) {
                    nData += nHere;
                    dwSamples++;
                }
            }
            if (dwSamples && bRes) {
                double fData = double(nData) / PRECISION_MULTIPLIER/ dwSamples;
                swprintf_s(szString.GetBuffer(25),25,_T("%.1f\t"),fData);
                szString.ReleaseBuffer();
            } else {
                szString = "\t";
            }
            WriteFileUtf8(pFile, szString);

            // Raw Pitch
            dwSamples = 0;
            bRes = TRUE;
            nData = 0;
            for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
                DWORD dwProcData = (DWORD)(dwIndex/fSizeFactor[PITCH]);
                // get data for this pixel
                int nHere = pDoc->GetPitch()->GetProcessedData(dwProcData, &bRes);
                if (nHere > 0) {
                    nData += nHere;
                    dwSamples++;
                }
            }
            if (dwSamples && bRes) {
                double fData = double(nData) / PRECISION_MULTIPLIER/ dwSamples;
                swprintf_s(szString.GetBuffer(25),25,_T("%.1f\t"),fData);
                szString.ReleaseBuffer();
            } else {
                szString = "\t";
            }
            WriteFileUtf8(pFile, szString);

            // Smoothed Pitch
            dwSamples = 0;
            bRes = TRUE;
            nData = 0;
            for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
                DWORD dwProcData = (DWORD)(dwIndex/fSizeFactor[PITCH]);
                // get data for this pixel
                int nHere = pDoc->GetSmoothedPitch()->GetProcessedData(dwProcData, &bRes);
                if (nHere > 0) {
                    nData += nHere;
                    dwSamples++;
                }
            }
            if (dwSamples && bRes) {
                double fData = double(nData) / PRECISION_MULTIPLIER/ dwSamples;
                swprintf_s(szString.GetBuffer(25),25,_T("%.1f\t"),fData);
                szString.ReleaseBuffer();
            } else {
                szString = "\t";
            }
            WriteFileUtf8(pFile, szString);
        }
        if (m_bMelogram) {
            int dwSamples = 0;
            BOOL bRes = TRUE;
            long nData = 0;
            for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
                DWORD dwProcData = (DWORD)(dwIndex/fSizeFactor[MELOGRAM]);
                // get data for this pixel
                int nHere = pDoc->GetMelogram()->GetProcessedData(dwProcData, &bRes); // SDM 1.5Test11.0
                if (nHere > 0) {
                    nData += nHere;
                    dwSamples++;
                }
            }
            if (dwSamples && bRes) {
                double fData = double(nData) / 100.0 / dwSamples;
                swprintf_s(szString.GetBuffer(25),25,_T("%.2f\t"),fData);
                szString.ReleaseBuffer();
            } else {
                szString = "\t";
            }
            WriteFileUtf8(pFile, szString);
        }
        if (m_bZeroCrossings) {
            int dwSamples = 0;
            BOOL bRes = TRUE;
            long nData = 0;
            for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
                DWORD dwProcData = (DWORD)(dwIndex/fSizeFactor[ZCROSS]);
                // get data for this pixel
                nData += pDoc->GetZCross()->GetProcessedData(dwProcData, &bRes);
                dwSamples++;
            }
            if (dwSamples && bRes) {
                nData = nData/ dwSamples;
                swprintf_s(szString.GetBuffer(25),25,_T("%d\t"),(int)nData);
                szString.ReleaseBuffer();
            } else {
                szString = "\t";
            }
            WriteFileUtf8(pFile, szString);
        }

        if (m_bF1 || m_bF2 || m_bF3 || m_bF4) {
            int dwSamples[5] = {0,0,0,0,0};
            double pFormFreq[5] = {0,0,0,0,0};
            int bFormOn[5] = {0, m_bF1, m_bF2, m_bF3, m_bF4};

            // get FormantTracker data
            for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex++) {
                DWORD dwProcData = (DWORD)((DWORD)(dwIndex/fSizeFactor[FMTTRACKER]/sizeof(FORMANT_FREQ)))*sizeof(FORMANT_FREQ);
                FORMANT_FREQ * pFormFreqCurr = (FORMANT_FREQ *)pDoc->GetFormantTracker()->GetProcessedData(dwProcData, sizeof(FORMANT_FREQ));
                for (int n = 1; n<5; n++) {
                    if (pFormFreqCurr->F[n] == (float)NA) {
                        continue;
                    }
                    pFormFreq[n] += (double)pFormFreqCurr->F[n];
                    dwSamples[n]++;
                }
            }
            for (int n = 1; n<5; n++) {
                pFormFreq[n] /= dwSamples[n];
                if (bFormOn[n]) {
                    if (dwSamples[n]) {
                        swprintf_s(szString.GetBuffer(25),25,_T("%.1f\t"),(double) pFormFreq[n]);
                        szString.ReleaseBuffer();
                    } else {
                        szString = "\t";
                    }
                    WriteFileUtf8(pFile, szString);
                }
            }
        }

        szString = "\r\n"; // next line of table
        WriteFileUtf8(pFile, szString);

        if (m_nSampleRate == 1) { // interval sampling
            dwOffset = dwNext < dwStopPosition ? dwNext : dwStopPosition;
        } else { // phonetic segment samples
            if (nIndex >= 0) {
                dwOffset = pPhonetic->GetOffset(nIndex);
            } else {
                dwOffset = dwStopPosition;
            }
        }
    }
    if (pFile) {
        delete pFile;
    }

    CDialog::OnOK();
}

/***************************************************************************/
// CDlgExportTable::OnHelpExportTable Call Export Table help
/***************************************************************************/
void CDlgExportTable::OnHelpExportTable() {
    
	// create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/File/Export/SFM_time_table.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

BOOL CDlgExportTable::OnInitDialog() {
    
	CDialog::OnInitDialog();

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    if (pDoc->GetSegment(PHONETIC)->IsEmpty()) { // no annotations
        BOOL bEnable = FALSE;
        SetEnable(IDC_EXTAB_PHONETIC, bEnable);
        SetEnable(IDC_EXTAB_TONE, bEnable);
        SetEnable(IDC_EXTAB_PHONEMIC, bEnable);
        SetEnable(IDC_EXTAB_ORTHO, bEnable);
        SetEnable(IDC_EXTAB_GLOSS, bEnable);
        SetEnable(IDC_EXTAB_REFERENCE, bEnable);
        SetEnable(IDC_EXTAB_POS, bEnable);
        SetEnable(IDC_EXTAB_ANNOTATIONS, bEnable);
        SetEnable(IDC_EXTAB_LENGTH, bEnable);
        SetEnable(IDC_EXTAB_START, bEnable);
    }

    OnAllAnnotations();
    OnAllCalculations();
    OnAllFormants();

    CenterWindow();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgExportTable::SetEnable Enables/Disables controls
/***************************************************************************/
void CDlgExportTable::SetEnable(int nItem, BOOL bEnable) {
    
	CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd) {
        pWnd->EnableWindow(bEnable);
    }
}

/***************************************************************************/
// CDlgExportTable::SetVisible Sets the visibility of dialog items
/***************************************************************************/
void CDlgExportTable::SetVisible(int nItem, BOOL bVisible, BOOL bEnable /*=TRUE*/) {

	CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd) {
        pWnd->EnableWindow(bVisible && bEnable);// disable invisible items, enable on show
        if (bVisible) {
            pWnd->SetWindowPos(NULL, 0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOZORDER+SWP_SHOWWINDOW);
        } else {
            pWnd->SetWindowPos(NULL, 0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOZORDER+SWP_HIDEWINDOW);
        }
    }
}


/***************************************************************************/
// CDlgExportTable::OnUpdateIntervalTime Filter edit box to only accept digits
/***************************************************************************/
void CDlgExportTable::OnUpdateIntervalTime() {

	CWnd * pWnd = GetDlgItem(IDC_EXTAB_INTERVAL_TIME);
    if (pWnd) {
        CSaString szText;
        BOOL bChanged = FALSE;

        pWnd->GetWindowText(szText);

        for (int nIndex=0; nIndex < szText.GetLength(); nIndex++) {
            if ((szText[nIndex] < '0') || (szText[nIndex] > '9')) {
                szText = szText.Left(nIndex) + szText.Mid(nIndex+1);
                bChanged = TRUE;
            }
            if (bChanged) { // only change if necessary (will cause infinite loop)
                pWnd->SetWindowText(szText);
            }
        }
    }
}


/***************************************************************************/
// CDlgExportTable::OnSample disable Phonetic Sampling on empty phonetic segment
// and update visibility of check boxes
/***************************************************************************/
void CDlgExportTable::OnSample() {

	UpdateData(TRUE);
    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    if (pDoc->GetSegment(PHONETIC)->IsEmpty()) { // no annotations
        if (m_nSampleRate == 0) {
            AfxMessageBox(IDS_ERROR_NOSEGMENTS,MB_OK,0);
            m_nSampleRate = 1;
            UpdateData(FALSE);
        }
    }
    OnAllAnnotations();
}


/***************************************************************************/
// CDlgExportTable::OnPhonetic keep invisible phonetic check box in sync with visible
/***************************************************************************/
void CDlgExportTable::OnPhonetic() {

	UpdateData(TRUE);
    if (m_nSampleRate == 0) {
        m_bPhonetic2 = m_bPhonetic;
    } else {
        m_bPhonetic = m_bPhonetic2;
    }
    UpdateData(FALSE);
}

static const char * psz_Phonemic = "pm";
static const char * psz_Gloss = "gl";
static const char * psz_Phonetic = "ph";
static const char * psz_Orthographic = "or";
static const char * psz_ImportEnd = "import";

static const char * psz_FreeTranslation = "ft"; // Free Translation
static const char * psz_Language ="ln"; // Language Name
static const char * psz_Dialect = "dlct"; // Dialect
static const char * psz_Family = "fam"; // Family
static const char * psz_Ethno = "id"; // Ethnologue ID number
static const char * psz_Country = "cnt"; // Country
static const char * psz_Region = "reg"; // Region
static const char * psz_Speaker = "spkr"; // Speaker Name
static const char * psz_Gender = "gen"; // Gender
static const char * psz_NotebookReference = "nbr"; // Notebook Reference
static const char * psz_Transcriber = "tr"; // Transcriber
static const char * psz_Comments = "cmnt"; // Comments
static const char * psz_Description = "desc"; // Description
static const char * psz_Table = "table";

/***************************************************************************/
// extractTabField local helper function to get field from tab delimited string
/***************************************************************************/
static const CSaString extractTabField(const CSaString & szLine, const int nField) {
    
	int nCount = 0;
    int nLoop = 0;

    if (nField < 0) {
        return "";    // SDM 1.5Test10.1
    }

    while ((nLoop < szLine.GetLength()) && (nCount < nField)) {
        if (szLine[nLoop] == '\t') {
            nCount++;
        }
        nLoop++;
    }
    int nBegin = nLoop;
    while ((nLoop < szLine.GetLength()) && (szLine[nLoop] != '\t')) {
        nLoop++;
    }
    return szLine.Mid(nBegin, nLoop-nBegin);
}

static void CreateWordSegments(const int nWord, int & nSegments) {
    
	CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    if (pDoc->GetSegment(GLOSS)->GetOffsetSize() > nWord) {
        DWORD dwStart;
        DWORD dwStop;
        int nPhonetic;
        CPhoneticSegment * pPhonetic = (CPhoneticSegment *) pDoc->GetSegment(PHONETIC);

        if (nWord == -1) {
            dwStart = 0;
            if (pDoc->GetSegment(GLOSS)->IsEmpty()) {
                dwStop = pDoc->GetUnprocessedDataSize();
            } else {
                dwStop = pDoc->GetSegment(GLOSS)->GetOffset(0);
            }
            if (dwStart + pDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME) > dwStop) {
                return;
            }
            nPhonetic = 0;
        } else {
            ASSERT(nSegments);
            dwStart = pDoc->GetSegment(GLOSS)->GetOffset(nWord);
            dwStop = pDoc->GetSegment(GLOSS)->GetDuration(nWord) + dwStart;
            nPhonetic = pPhonetic->FindOffset(dwStart);
        }
        // Limit number of segments
        if (nSegments*pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME) > (dwStop -dwStart)) {
            nSegments = (int)((dwStop -dwStart)/pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));
            if (!nSegments) {
                nSegments = 1;
            }
            if (nSegments*pDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME) > (dwStop -dwStart)) {
                return;
            }
        }
        // remove excess segments
        int nCount = 0;
        int nIndex = nPhonetic;
        while ((nIndex != -1)&&(pPhonetic->GetOffset(nIndex) < dwStop)) {
            if (nCount >= nSegments) {
                pPhonetic->SetSelection(nIndex);
                pPhonetic->Remove(pDoc, FALSE); // no checkpoint
                if (nIndex >= pPhonetic->GetOffsetSize()) {
                    break;
                }
            } else {
                DWORD dwBegin = dwStart + nCount;
                pPhonetic->Adjust(pDoc, nIndex, dwBegin, 1);
                nIndex = pPhonetic->GetNext(nIndex);
                nCount++;
            }
        }
        if (nSegments == 0) {
            return;    // done
        }
        // add segments
        while (nCount < nSegments) {
            if (nIndex == -1) {
                nIndex = pPhonetic->GetOffsetSize();
            }
            DWORD dwBegin = dwStart + nCount;
            CSaString szEmpty(SEGMENT_DEFAULT_CHAR);
            pPhonetic->Insert(nIndex, &szEmpty, FALSE, dwBegin, 1);
            nIndex = pPhonetic->GetNext(nIndex);
            nCount++;
        }
        // adjust segment spacing
        DWORD dwSize = (dwStop - dwStart)/nSegments;
        if (pDoc->GetFmtParm()->wBlockAlign==2) {
            dwSize &= ~1;
        };
        dwSize += pDoc->GetFmtParm()->wBlockAlign;
        if (nIndex == -1) {
            nIndex = pPhonetic->GetOffsetSize();
        }
        nIndex = pPhonetic->GetPrevious(nIndex);
        while ((nIndex != -1)&&(pPhonetic->GetOffset(nIndex) >= dwStart)) {
            nCount--;
            DWORD dwBegin = dwStart+nCount*dwSize;
            DWORD dwDuration = dwSize;
            if ((dwBegin + dwDuration) > dwStop) {
                dwDuration = dwStop - dwBegin;
            }
            pPhonetic->Adjust(pDoc, nIndex, dwBegin, dwDuration);
            nIndex = pPhonetic->GetPrevious(nIndex);
        }
    }
    nSegments = 0;
}

void CDlgExportTable::WriteFileUtf8(CFile * pFile, const CSaString szString) {

	std::string szUtf8 = szString.utf8();
	pFile->Write(szUtf8.c_str(), szUtf8.size());
}

