#include "stdafx.h"
#include "DlgExportSFM.h"
#include "MainFrm.h"
#include "Sa_Doc.h"
#include "SA_View.h"
#include "Sa_segm.h"
#include "GlossSegment.h"
#include "TextSegment.h"
#include "DlgExportFW.h"
#include "FileUtils.h"
#include "sa_wbch.h"
#include "sa.h"
#include "sa_graph.h"
#include "doclist.h"
#include "result.h"
#include <math.h>
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
#include "Process\formanttracker.h"
#include "dsp\formants.h"
#include "dsp\ztransform.h"
#include "dsp\mathx.h"
#include "settings\obstream.h"

using std::ifstream;
using std::ios;
using std::streampos;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

LPCTSTR szCrLf = L"\r\n";

/////////////////////////////////////////////////////////////////////////////
// CDlgExportSFM dialog

/*
\name name of file
\wav  Audio FileName
\ph   Phonetic text
\tn   Tone
\pm   Phonemic text
\or   Orthographic
\gl   Gloss
\pos  Part of Speech
\ref  Reference
\ft   Free Translation
\np   Number of Phones
\nw   Number of Words

\od   Original Date (Creation Time)
\lud  Date Last updated
\size File size in bytes
\of   Original Format
\samp Number of Samples
\len  Length
\freq Sampling Frequency
\bw   Bandwidth
\hpf  HighPass Filter
\bits Storage Format
\qsize Quantization Size

\ln   Language Name
\dlct Dialect
\cnt  Country
\spkr Speaker Name
\gen  Gender
\id   Ethnologue ID number
\fam  Family
\reg  Region
\nbr  Notebook Reference
\tr   Transcriber
\desc Description
*/

CDlgExportSFM::CDlgExportSFM(const CSaString & szDocTitle, CWnd * pParent) :
    CDialog(CDlgExportSFM::IDD, pParent) {

    m_bAllSource = TRUE;
    m_bBandwidth = FALSE;
    m_bBits = FALSE;
    m_bComments = FALSE;
    m_bDialect = FALSE;
    m_bEthnologue = FALSE;
    m_bFamily = FALSE;
    m_bAllFile = TRUE;
    m_bFileSize = FALSE;
    m_bFileName = TRUE;
    m_bFree = TRUE;
    m_bGender = FALSE;
    m_bHighPass = FALSE;
    m_bInterlinear = FALSE;
    m_bMultiRecord = FALSE;
    m_bLanguage = FALSE;
    m_bLastModified = FALSE;
    m_bLength = FALSE;
    m_bNotebookRef = FALSE;
    m_bNumberSamples = FALSE;
    m_bOriginalDate = FALSE;
    m_bOriginalFormat = FALSE;
    m_bPhones = TRUE;
    m_bSampleRate = FALSE;
    m_bAllParameters = TRUE;
    m_bRegion = FALSE;
    m_bSpeaker = FALSE;
    m_bTranscriber = FALSE;
    m_bWords = TRUE;
    m_bAllAnnotations = TRUE;
    m_bGloss = TRUE;
    m_bOrtho = TRUE;
    m_bPhonemic = TRUE;
    m_bPhonetic = TRUE;
    m_bPOS = FALSE;
    m_bReference = FALSE;
    m_bTone = FALSE;
    m_bPhrase = FALSE;
    m_bCountry = FALSE;
    m_bQuantization = FALSE;
    m_szDocTitle = szDocTitle;
}

BOOL CDlgExportSFM::OnInitDialog() {

	CDialog::OnInitDialog();

	OnAllAnnotations();
	OnAllSource();
	OnAllParameters();
	OnAllFileInfo();

	CenterWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgExportSFM::DoDataExchange(CDataExchange * pDX) {

    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_EX_SFM_ALL_SOURCE, m_bAllSource);
    DDX_Check(pDX, IDC_EX_SFM_BANDWIDTH, m_bBandwidth);
    DDX_Check(pDX, IDC_EX_SFM_BITS, m_bBits);
    DDX_Check(pDX, IDC_EX_SFM_COMMENTS, m_bComments);
    DDX_Check(pDX, IDC_EX_SFM_DIALECT, m_bDialect);
    DDX_Check(pDX, IDC_EX_SFM_ETHNOLOGUE_ID, m_bEthnologue);
    DDX_Check(pDX, IDC_EX_SFM_FAMILY, m_bFamily);
    DDX_Check(pDX, IDC_EX_SFM_FILE_INFO, m_bAllFile);
    DDX_Check(pDX, IDC_EX_SFM_FILE_SIZE, m_bFileSize);
    DDX_Check(pDX, IDC_EX_SFM_FILENAME, m_bFileName);
    DDX_Check(pDX, IDC_EX_SFM_FREE, m_bFree);
    DDX_Check(pDX, IDC_EX_SFM_GENDER, m_bGender);
    DDX_Check(pDX, IDC_EX_SFM_HIGHPASS, m_bHighPass);
    DDX_Check(pDX, IDC_EX_SFM_INTERLINEAR, m_bInterlinear);
    DDX_Check(pDX, IDC_EX_SFM_MULTIRECORD, m_bMultiRecord);
    DDX_Check(pDX, IDC_EX_SFM_LANGUAGE, m_bLanguage);
    DDX_Check(pDX, IDC_EX_SFM_LAST_DATE, m_bLastModified);
    DDX_Check(pDX, IDC_EX_SFM_LENGTH, m_bLength);
    DDX_Check(pDX, IDC_EX_SFM_NOTEBOOKREF, m_bNotebookRef);
    DDX_Check(pDX, IDC_EX_SFM_NUMBER_OF_SAMPLES, m_bNumberSamples);
    DDX_Check(pDX, IDC_EX_SFM_ORIGINAL_DATE, m_bOriginalDate);
    DDX_Check(pDX, IDC_EX_SFM_ORIGINAL_FORMAT, m_bOriginalFormat);
    DDX_Check(pDX, IDC_EX_SFM_PHONES, m_bPhones);
    DDX_Check(pDX, IDC_EX_SFM_RATE, m_bSampleRate);
    DDX_Check(pDX, IDC_EX_SFM_RECORD_DATA, m_bAllParameters);
    DDX_Check(pDX, IDC_EX_SFM_REGION, m_bRegion);
    DDX_Check(pDX, IDC_EX_SFM_SPEAKER, m_bSpeaker);
    DDX_Check(pDX, IDC_EX_SFM_TRANSCRIBER, m_bTranscriber);
    DDX_Check(pDX, IDC_EX_SFM_WORDS, m_bWords);
    DDX_Check(pDX, IDC_EXTAB_ANNOTATIONS, m_bAllAnnotations);
    DDX_Check(pDX, IDC_EXTAB_GLOSS, m_bGloss);
    DDX_Check(pDX, IDC_EXTAB_ORTHO, m_bOrtho);
    DDX_Check(pDX, IDC_EXTAB_PHONEMIC, m_bPhonemic);
    DDX_Check(pDX, IDC_EXTAB_PHONETIC, m_bPhonetic);
    DDX_Check(pDX, IDC_EXTAB_POS, m_bPOS);
    DDX_Check(pDX, IDC_EXTAB_REFERENCE, m_bReference);
    DDX_Check(pDX, IDC_EXTAB_TONE, m_bTone);
    DDX_Check(pDX, IDC_EXTAB_PHRASE, m_bPhrase);
    DDX_Check(pDX, IDC_EX_SFM_COUNTRY, m_bCountry);
    DDX_Check(pDX, IDC_EX_SFM_QUANTIZATION, m_bQuantization);
}

BEGIN_MESSAGE_MAP(CDlgExportSFM, CDialog)
    ON_BN_CLICKED(IDC_EX_SFM_ALL_SOURCE, OnAllSource)
    ON_BN_CLICKED(IDC_EX_SFM_FILE_INFO, OnAllFileInfo)
    ON_BN_CLICKED(IDC_EX_SFM_RECORD_DATA, OnAllParameters)
    ON_BN_CLICKED(IDC_EXTAB_ANNOTATIONS, OnAllAnnotations)
    ON_COMMAND(IDHELP, OnHelpExportBasic)
    ON_BN_CLICKED(IDC_EX_SFM_INTERLINEAR, OnClickedExSfmInterlinear)
    ON_BN_CLICKED(IDC_EX_SFM_MULTIRECORD, OnClickedExSfmMultirecord)
END_MESSAGE_MAP()

void CDlgExportSFM::OnOK() {

    wstring filename;
    int result = GetSaveAsFilename(m_szDocTitle, _T("Standard Format (*.sfm) |*.sfm||"), _T("sfm"), NULL, filename);
    if (result!=IDOK) {
        return;
    }
    m_szFileName = filename.c_str();
    if (m_szFileName == "") {
        return;
    }

    UpdateData(TRUE);

    // process all flags
    if (m_bAllAnnotations) {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = TRUE;
    }

    if (m_bAllFile) {
        m_bOriginalDate = m_bLastModified = m_bOriginalFormat = m_bFileSize = TRUE;
    }

    if (m_bAllParameters) {
        m_bNumberSamples = m_bLength = m_bSampleRate = m_bBandwidth = m_bHighPass = m_bBits = m_bQuantization = TRUE;
    }

    if (m_bAllSource)
        m_bLanguage = m_bDialect = m_bSpeaker = m_bGender = m_bEthnologue = m_bFamily = m_bRegion = m_bNotebookRef =
                m_bTranscriber = m_bComments = m_bCountry = TRUE;

    if (m_bMultiRecord) {
        ExportMultiRec();
    } else {
        ExportStandard();
    }

    CDialog::OnOK();
}

/**
* output the SFM file in standard format
*/
void CDlgExportSFM::ExportStandard() {

    CFile file(m_szFileName, CFile::modeCreate|CFile::modeWrite);
    CSaString szString;

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    ExportFile(pDoc, file);

    if (!pDoc->GetSegment(PHONETIC)->IsEmpty()) {
        CSaString szAnnotation[ANNOT_WND_NUMBER];
        CSaString szPOS;
        int nMaxLength = 0;
        int nNumber = 0;
        DWORD dwOffset;

        while (nNumber != -1) {
            BOOL bBreak = FALSE;

            dwOffset = pDoc->GetSegment(PHONETIC)->GetOffset(nNumber);

            int nFind = pDoc->GetSegment(GLOSS)->FindOffset(dwOffset);
            if ((nNumber > 0) && (nFind != -1)) {
                bBreak = TRUE;
            }
            if (!m_bInterlinear && bBreak) {
                for (int nLoop = PHONETIC; nLoop < ANNOT_WND_NUMBER; nLoop++) {
                    szAnnotation[nLoop] += " ";
                }
                szPOS +=" ";
            } else if (bBreak) {
                for (int nLoop = PHONETIC; nLoop < ANNOT_WND_NUMBER; nLoop++) {
                    while (szAnnotation[nLoop].GetLength() <= nMaxLength) {
                        szAnnotation[nLoop] += " ";
                    }
                }
                while (szPOS.GetLength() <= nMaxLength) {
                    szPOS +=" ";
                }
            }

            szAnnotation[PHONETIC] += pDoc->GetSegment(PHONETIC)->GetSegmentString(nNumber);

            for (int nLoop = PHONETIC+1; nLoop < ANNOT_WND_NUMBER; nLoop++) {
                nFind = pDoc->GetSegment(nLoop)->FindOffset(dwOffset);
                if (nFind != -1) {
                    if (nLoop == GLOSS) {
                        szAnnotation[nLoop] += pDoc->GetSegment(nLoop)->GetSegmentString(nFind).Mid(1);
                        szPOS += ((CGlossSegment *)pDoc->GetSegment(GLOSS))->GetPOSs()->GetAt(nFind);
                        if (szPOS.GetLength() > nMaxLength) {
                            nMaxLength = szPOS.GetLength();
                        }
                    } else {
                        szAnnotation[nLoop] += pDoc->GetSegment(nLoop)->GetSegmentString(nFind);
                    }
                }
                if (szAnnotation[nLoop].GetLength() > nMaxLength) {
                    nMaxLength = szAnnotation[nLoop].GetLength();
                }
            }

            nNumber = pDoc->GetSegment(PHONETIC)->GetNext(nNumber);
        }
        // write out results
        if (m_bReference) { // \ref  Reference
            szString = "\\ref " + szAnnotation[REFERENCE] + szCrLf;
            WriteFileUtf8(&file, szString);
        }
        if (m_bPhonetic) { // \ph   Phonetic text
            szString = "\\ph  " + szAnnotation[PHONETIC] + szCrLf;
            WriteFileUtf8(&file, szString);
        }
        if (m_bTone) { // \tn   Tone
            szString = "\\tn  " + szAnnotation[TONE] + szCrLf;
            WriteFileUtf8(&file, szString);
        }
        if (m_bPhonemic) { // \pm   Phonemic text
            szString = "\\pm  " + szAnnotation[PHONEMIC] + szCrLf;
            WriteFileUtf8(&file, szString);
        }
        if (m_bOrtho) { // \or   Orthographic
            szString = "\\or  " + szAnnotation[ORTHO] + szCrLf;
            WriteFileUtf8(&file, szString);
        }
        if (m_bGloss) { // \gl   Gloss
            szString = "\\gl  " + szAnnotation[GLOSS] + szCrLf;
            WriteFileUtf8(&file, szString);
        }
        if (m_bPOS) { // \pos  Part of Speech
            szString = "\\pos " + szPOS + szCrLf;
            WriteFileUtf8(&file, szString);
        }
    }

    for (int nPhrase = MUSIC_PL1; nPhrase <= MUSIC_PL4; nPhrase++) {
        szString.Format(_T("\\phr%d "), nPhrase - MUSIC_PL1 + 1);
        CSaString szPhrase;

        if (m_bPhrase && !pDoc->GetSegment(nPhrase)->IsEmpty()) {
            int nNumber = 0;

            while (nNumber != -1) {
                szPhrase += pDoc->GetSegment(nPhrase)->GetSegmentString(nNumber);

                nNumber = pDoc->GetSegment(nPhrase)->GetNext(nNumber);
            }
        }
        if (m_bPhrase) { // \phr1-\phr3  Phrase Level
            szString = szString + szPhrase + szCrLf;
            WriteFileUtf8(&file, szString);
        }
    }

    ExportCounts(pDoc, file);
    ExportAllFileInformation(pDoc, file);
    ExportAllParameters(pDoc, file);
    ExportAllSource(pDoc, file);

    file.Close();
}

/**
* Output the SFM file in multi-record format
*/
void CDlgExportSFM::ExportMultiRec() {

    CFile file(m_szFileName, CFile::modeCreate|CFile::modeWrite);
    CSaString szString;

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    ExportFile(pDoc, file);

    if (!TryExportSegmentsBy(REFERENCE,pDoc,file)) {
        if (!TryExportSegmentsBy(GLOSS,pDoc,file)) {
            if (!TryExportSegmentsBy(ORTHO,pDoc,file)) {
                if (!TryExportSegmentsBy(PHONEMIC,pDoc,file)) {
                    if (!TryExportSegmentsBy(TONE,pDoc,file)) {
                        TryExportSegmentsBy(PHONETIC,pDoc,file);
                    }
                }
            }
        }
    }

    ExportCounts(pDoc, file);
    ExportAllFileInformation(pDoc, file);
    ExportAllParameters(pDoc, file);
    ExportAllSource(pDoc, file);

    file.Close();
}

bool CDlgExportSFM::TryExportSegmentsBy(Annotations master, CSaDoc * pDoc, CFile & file) {

    if (!GetFlag(master)) {
        return false;
    }

    CSegment * pSeg = pDoc->GetSegment(master);

    if (pSeg->GetOffsetSize() == 0) {
        return false;
    }

    WriteFileUtf8(&file, szCrLf);

    CSaString results[ANNOT_WND_NUMBER];
    for (int i=0; i<ANNOT_WND_NUMBER; i++) {
        results[i] = L"";
    }
    DWORD last = pSeg->GetOffset(0)-1;
    for (int i=0; i<pSeg->GetOffsetSize(); i++) {
        DWORD dwStart = pSeg->GetOffset(i);
        DWORD dwStop = pSeg->GetStop(i);
        if (dwStart==last) {
            continue;
        }
        last = dwStart;
        for (int j=master; j>=0; j--) {
            Annotations target = GetAnnotation(j);
            if (!GetFlag(target)) {
                continue;
            }
            results[target] = BuildRecord(target,dwStart,dwStop,pDoc);
        }

        if (m_bPhrase) {
            results[MUSIC_PL1] = BuildPhrase(MUSIC_PL1, dwStart, dwStop, pDoc);
            results[MUSIC_PL2] = BuildPhrase(MUSIC_PL2, dwStart, dwStop, pDoc);
            results[MUSIC_PL3] = BuildPhrase(MUSIC_PL3, dwStart, dwStop, pDoc);
            results[MUSIC_PL4] = BuildPhrase(MUSIC_PL4, dwStart, dwStop, pDoc);
        }

        if (results[REFERENCE].GetLength()>0) {
            WriteFileUtf8(&file, results[REFERENCE]);
        }
        if (results[PHONETIC].GetLength()>0) {
            WriteFileUtf8(&file, results[PHONETIC]);
        }
        if (results[TONE].GetLength()>0) {
            WriteFileUtf8(&file, results[TONE]);
        }
        if (results[PHONEMIC].GetLength()>0) {
            WriteFileUtf8(&file, results[PHONEMIC]);
        }
        if (results[ORTHO].GetLength()>0) {
            WriteFileUtf8(&file, results[ORTHO]);
        }
        if (results[GLOSS].GetLength()>0) {
            WriteFileUtf8(&file, results[GLOSS]);
        }
        if (results[MUSIC_PL1].GetLength()>0) {
            WriteFileUtf8(&file, results[MUSIC_PL1]);
        }
        if (results[MUSIC_PL2].GetLength()>0) {
            WriteFileUtf8(&file, results[MUSIC_PL2]);
        }
        if (results[MUSIC_PL3].GetLength()>0) {
            WriteFileUtf8(&file, results[MUSIC_PL3]);
        }
        if (results[MUSIC_PL4].GetLength()>0) {
            WriteFileUtf8(&file, results[MUSIC_PL4]);
        }
        WriteFileUtf8(&file, szCrLf);
    }

    return true;
}

CSaString CDlgExportSFM::BuildRecord(Annotations target, DWORD dwStart, DWORD dwStop, CSaDoc * pDoc) {

    CSaString szTag = GetTag(target);
    CSegment * pSegment = pDoc->GetSegment(target);
    CSaString szText = pSegment->GetContainedText(dwStart,dwStop);
    szText = szText.Trim();
    if (szText.GetLength()==0) {
        return L"";
    }
    if (target==GLOSS) {
        if (szText[0]==WORD_DELIMITER) {
            szText = szText.Right(szText.GetLength()-1);
        }
    }
    return szTag + L" " + szText + szCrLf;
}

CSaString CDlgExportSFM::BuildPhrase(Annotations target, DWORD dwStart, DWORD dwStop, CSaDoc * pDoc) {

    CSaString szTag = GetTag(target);
    CSegment * pSegment = pDoc->GetSegment(GetIndex(target));
    CSaString szText =  pSegment->GetOverlappingText(dwStart,dwStop);
    szText = szText.Trim();
    if (szText.GetLength()==0) {
        return L"";
    }
    return szTag + L" " + szText + szCrLf;
}

BOOL CDlgExportSFM::GetFlag(Annotations val) {

    switch (val) {
    case PHONETIC:
        return m_bPhonetic;
    case TONE:
        return m_bTone;
    case PHONEMIC:
        return m_bPhonemic;
    case ORTHO:
        return m_bOrtho;
    case GLOSS:
        return m_bGloss;
    case REFERENCE:
        return m_bReference;
    case MUSIC_PL1:
        return m_bPhrase;
    case MUSIC_PL2:
        return m_bPhrase;
    case MUSIC_PL3:
        return m_bPhrase;
    case MUSIC_PL4:
        return m_bPhrase;
    }
    return false;
}

int CDlgExportSFM::GetIndex(Annotations val) {

    switch (val) {
    case PHONETIC:
        return 0;
    case TONE:
        return 1;
    case PHONEMIC:
        return 2;
    case ORTHO:
        return 3;
    case GLOSS:
        return 4;
    case REFERENCE:
        return 5;
    case MUSIC_PL1:
        return 6;
    case MUSIC_PL2:
        return 7;
    case MUSIC_PL3:
        return 8;
    case MUSIC_PL4:
        return 9;
    }
    return false;
}

LPCTSTR CDlgExportSFM::GetTag(Annotations val) {

    switch (val) {
    case PHONETIC:
        return L"\\ph";
    case TONE:
        return L"\\tn";
    case PHONEMIC:
        return L"\\pm";
    case ORTHO:
        return L"\\or";
    case GLOSS:
        return L"\\gl";
    case REFERENCE:
        return L"\\ref";
    case MUSIC_PL1:
        return L"\\phr1";
    case MUSIC_PL2:
        return L"\\phr2";
    case MUSIC_PL3:
        return L"\\phr3";
    case MUSIC_PL4:
        return L"\\phr4";
    }
    return L"";
}

Annotations CDlgExportSFM::GetAnnotation(int val) {

    switch (val) {
    case 0:
        return PHONETIC;
    case 1:
        return TONE;
    case 2:
        return PHONEMIC;
    case 3:
        return ORTHO;
    case 4:
        return GLOSS;
    case 5:
        return REFERENCE;
    case 6:
        return MUSIC_PL1;
    case 7:
        return MUSIC_PL2;
    case 8:
        return MUSIC_PL3;
    case 9:
        return MUSIC_PL4;
    }
    return PHONETIC;
}

void CDlgExportSFM::ExportFile(CSaDoc * pDoc, CFile & file) {

    CSaString szString;

    // \name write filename
    szString = "\\name " + m_szFileName + szCrLf;
    WriteFileUtf8(&file, szString);

    // \date write current time
    CTime time = CTime::GetCurrentTime();
    szString = "\\date " + time.Format("%A, %B %d, %Y, %X") + "\r\n";
    WriteFileUtf8(&file, szString);

    if (m_bFileName) { // \wav  Audio FileName
        szString = "\\wav " + pDoc->GetPathName() + szCrLf;
        WriteFileUtf8(&file, szString);
    }
}

void CDlgExportSFM::ExportCounts(CSaDoc * pDoc, CFile & file) {

    WriteFileUtf8(&file, szCrLf);

    CSaString szString;
    if (m_bFree) { // \ft   Free Translation
        szString = "\\ft " + pDoc->GetSourceParm()->szFreeTranslation + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bPhones) { // \np   Number of Phones
        // create and write number of phones text
        int nNumber = 0;
        int nLoop = 0;
        if (pDoc->GetSegment(PHONETIC)->GetString()->GetLength() > 0) {
            // find number of phones
            while (TRUE) {
                nLoop++;
                nNumber = pDoc->GetSegment(PHONETIC)->GetNext(nNumber);
                if (nNumber < 0) {
                    break;
                }
            }
        }
        swprintf_s(szString.GetBuffer(25),25,_T("%u"), nLoop);
        szString.ReleaseBuffer();
        szString = "\\np " +  szString + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bWords) { // \nw   Number of Words
        swprintf_s(szString.GetBuffer(25),25,_T("%u"), ((CTextSegment *)pDoc->GetSegment(GLOSS))->CountWords());
        szString.ReleaseBuffer();
        szString = "\\nw " +  szString + szCrLf;
        WriteFileUtf8(&file, szString);
    }
}

void CDlgExportSFM::ExportAllFileInformation(CSaDoc * pDoc, CFile & file) {

    WriteFileUtf8(&file, szCrLf);

    CSaString szString;
    CFileStatus * pFileStatus = pDoc->GetFileStatus();
    if (pFileStatus->m_szFullName[0] != 0) { // file name is defined
        if (m_bOriginalDate) { // \ct   Creation Time
            szString = "\\ct " + pFileStatus->m_ctime.Format("%A, %B %d, %Y, %X") + szCrLf;
            WriteFileUtf8(&file, szString);
        }
        if (m_bLastModified) { // \le  Last Edit
            szString = "\\le " + pFileStatus->m_mtime.Format("%A, %B %d, %Y, %X") + szCrLf;
            WriteFileUtf8(&file, szString);
        }
        if (m_bFileSize) { // \size File size in bytes
            swprintf_s(szString.GetBuffer(25),25,_T("%ld Bytes"),pFileStatus->m_size);
            szString.ReleaseBuffer();
            szString = "\\size " +  szString + szCrLf;
            WriteFileUtf8(&file, szString);
        }
        if (m_bOriginalFormat) { // \of   Original Format
            SaParm * pSaParm = pDoc->GetSaParm();
            if (pSaParm->byRecordFileFormat <= FILE_FORMAT_TIMIT) {
                szString.LoadString((UINT)pSaParm->byRecordFileFormat + IDS_FILETYPE_UTT);
                szString = "\\of " + szString + szCrLf;
                WriteFileUtf8(&file, szString);
            }
        }
    }

}

void CDlgExportSFM::ExportAllParameters(CSaDoc * pDoc, CFile & file) {

    WriteFileUtf8(&file, szCrLf);

    CSaString szString;

    if (m_bNumberSamples) { // \samp Number of Samples
        swprintf_s(szString.GetBuffer(25),25,_T("%ld"), pDoc->GetDataSize() / pDoc->GetFmtParm()->wBlockAlign);
        szString.ReleaseBuffer();
        szString = "\\samp " +  szString + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bLength) { // \len  Length
        // create and write length text
        double fDataSec = pDoc->GetTimeFromBytes(pDoc->GetUnprocessedDataSize()); // get sampled data size in seconds
        int nMinutes = (int)fDataSec / 60;

        if (nMinutes == 0) { // length is less than one minute
            swprintf_s(szString.GetBuffer(25),25,_T("%5.3f Seconds"), fDataSec);
        } else { // length is equal or more than one minute
            fDataSec = fDataSec - (float)(nMinutes * 60);
            swprintf_s(szString.GetBuffer(25),25,_T("%i:%5.3f (Min:Sec)"), nMinutes, fDataSec);
        }
        szString.ReleaseBuffer();
        szString = "\\len " +  szString + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bSampleRate) { // \freq Sampling Frequency
        swprintf_s(szString.GetBuffer(25),25,_T("%lu Hz"),pDoc->GetFmtParm()->dwSamplesPerSec);
        szString.ReleaseBuffer();
        szString = "\\freq " +  szString + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bBandwidth) { // \bw   Bandwidth
        swprintf_s(szString.GetBuffer(25),25,_T("%lu Hz"),pDoc->GetSaParm()->dwRecordBandWidth);
        szString.ReleaseBuffer();
        szString = "\\bw " +  szString + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bHighPass) { // \hpf  HighPass Filter
        szString = pDoc->GetSaParm()->wFlags & SA_FLAG_HIGHPASS ? "Yes":"No";
        szString = "\\hpf " +  szString + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bBits) { // \bits Storage Format
        swprintf_s(szString.GetBuffer(25),25,_T("%d Bits"),pDoc->GetFmtParm()->wBitsPerSample);
        szString.ReleaseBuffer();
        szString = "\\bits " +  szString + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bQuantization) { // \size Quantization Size
        swprintf_s(szString.GetBuffer(25),25,_T("%d Bits"),(int)pDoc->GetSaParm()->byRecordSmpSize);
        szString.ReleaseBuffer();
        szString = "\\qsize " +  szString + szCrLf;
        WriteFileUtf8(&file, szString);
    }
}

void CDlgExportSFM::ExportAllSource(CSaDoc * pDoc, CFile & file) {

    WriteFileUtf8(&file, szCrLf);

    CSaString szString;

    if (m_bLanguage) { // \ln   Language Name
        szString = "\\ln " + pDoc->GetSourceParm()->szLanguage + szCrLf;
        WriteFileUtf8(&file, szString);
    }

    if (m_bDialect) { // \dlct Dialect
        szString = "\\dlct " + pDoc->GetSourceParm()->szDialect + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bFamily) { // \fam  Family
        szString = "\\fam " + pDoc->GetSourceParm()->szFamily + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bEthnologue) { // \id   Ethnologue ID number
        szString = "\\id " + pDoc->GetSourceParm()->szEthnoID + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bCountry) { // \cnt  Country
        szString = "\\cnt " + pDoc->GetSourceParm()->szCountry + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bRegion) { // \reg  Region
        szString = "\\reg " + pDoc->GetSourceParm()->szRegion + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bSpeaker) { // \spkr Speaker Name
        szString = "\\spkr " + pDoc->GetSourceParm()->szSpeaker + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bGender) { // \gen  Gender
        switch (pDoc->GetSourceParm()->nGender) {
        case 0:
            szString = "Adult Male";
            break;
        case 1:
            szString = "Adult Female";
            break;
        case 2:
            szString = "Child";
            break;
        default:
            szString = "";
            break;
        }
        szString = "\\gen " + szString + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bNotebookRef) { // \nbr  Notebook Reference
        szString = "\\nbr " + pDoc->GetSourceParm()->szReference + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bTranscriber) { // \tr   Transcriber
        szString = "\\tr " + pDoc->GetSourceParm()->szTranscriber + szCrLf;
        WriteFileUtf8(&file, szString);
    }
    if (m_bComments) { // \desc Description
        szString = "\\desc " + pDoc->GetSaParm()->szDescription + szCrLf;
        WriteFileUtf8(&file, szString);
    }
}

void CDlgExportSFM::OnAllAnnotations() {

    UpdateData(TRUE);
    BOOL bEnable = !m_bAllAnnotations;
    SetEnable(IDC_EXTAB_PHONETIC, bEnable);
    SetEnable(IDC_EXTAB_TONE, bEnable);
    SetEnable(IDC_EXTAB_PHONEMIC, bEnable);
    SetEnable(IDC_EXTAB_ORTHO, bEnable);
    SetEnable(IDC_EXTAB_GLOSS, bEnable);
    SetEnable(IDC_EXTAB_REFERENCE, bEnable);
    SetEnable(IDC_EXTAB_POS, bEnable);
    SetEnable(IDC_EXTAB_PHRASE, bEnable);
    if (m_bAllAnnotations) {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = TRUE;
        UpdateData(FALSE);
    } else {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = FALSE;
        UpdateData(FALSE);
    }
}

void CDlgExportSFM::OnAllFileInfo() {

    UpdateData(TRUE);
    BOOL bEnable = !m_bAllFile;
    SetEnable(IDC_EX_SFM_FILE_SIZE, bEnable);
    SetEnable(IDC_EX_SFM_LAST_DATE, bEnable);
    SetEnable(IDC_EX_SFM_ORIGINAL_DATE, bEnable);
    SetEnable(IDC_EX_SFM_ORIGINAL_FORMAT, bEnable);
    if (m_bAllFile) {
        m_bOriginalDate = m_bLastModified = m_bOriginalFormat = m_bFileSize = TRUE;
        UpdateData(FALSE);
    } else {
        m_bOriginalDate = m_bLastModified = m_bOriginalFormat = m_bFileSize = FALSE;
        UpdateData(FALSE);
    }
}

void CDlgExportSFM::OnAllParameters() {

    UpdateData(TRUE);
    BOOL bEnable = !m_bAllParameters;
    SetEnable(IDC_EX_SFM_BANDWIDTH, bEnable);
    SetEnable(IDC_EX_SFM_BITS, bEnable);
    SetEnable(IDC_EX_SFM_QUANTIZATION, bEnable);
    SetEnable(IDC_EX_SFM_HIGHPASS, bEnable);
    SetEnable(IDC_EX_SFM_LENGTH, bEnable);
    SetEnable(IDC_EX_SFM_NUMBER_OF_SAMPLES, bEnable);
    SetEnable(IDC_EX_SFM_RATE, bEnable);

    if (m_bAllParameters) {
        m_bNumberSamples = m_bLength = m_bSampleRate = m_bBandwidth = m_bHighPass = m_bBits = m_bQuantization = TRUE;
        UpdateData(FALSE);
    } else {
        m_bNumberSamples = m_bLength = m_bSampleRate = m_bBandwidth = m_bHighPass = m_bBits = m_bQuantization = FALSE;
        UpdateData(FALSE);
    }

}

void CDlgExportSFM::OnAllSource() {

    UpdateData(TRUE);
    BOOL bEnable = !m_bAllSource;
    SetEnable(IDC_EX_SFM_LANGUAGE, bEnable);
    SetEnable(IDC_EX_SFM_DIALECT, bEnable);
    SetEnable(IDC_EX_SFM_SPEAKER, bEnable);
    SetEnable(IDC_EX_SFM_GENDER, bEnable);
    SetEnable(IDC_EX_SFM_ETHNOLOGUE_ID, bEnable);
    SetEnable(IDC_EX_SFM_FAMILY, bEnable);
    SetEnable(IDC_EX_SFM_REGION, bEnable);
    SetEnable(IDC_EX_SFM_COUNTRY, bEnable);
    SetEnable(IDC_EX_SFM_NOTEBOOKREF, bEnable);
    SetEnable(IDC_EX_SFM_TRANSCRIBER, bEnable);
    SetEnable(IDC_EX_SFM_COMMENTS, bEnable);
    if (m_bAllSource) {
        m_bLanguage = m_bDialect = m_bSpeaker = m_bGender = m_bEthnologue = m_bFamily = m_bRegion = m_bNotebookRef =
                m_bTranscriber = m_bComments = m_bCountry = TRUE;
        UpdateData(FALSE);
    } else {
        m_bLanguage = m_bDialect = m_bSpeaker = m_bGender = m_bEthnologue = m_bFamily = m_bRegion = m_bNotebookRef =
                m_bTranscriber = m_bComments = m_bCountry = FALSE;
        UpdateData(FALSE);
    }
}

void CDlgExportSFM::SetEnable(int nItem, BOOL bEnable) {

    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd) {
        pWnd->EnableWindow(bEnable);
    }
}

void CDlgExportSFM::SetCheck(int nItem, BOOL bChecked) {

    CButton * pWnd = (CButton *)GetDlgItem(nItem);
    if (pWnd) {
        pWnd->SetCheck(bChecked);
    }
}

void CDlgExportSFM::OnHelpExportBasic() {

    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/File/Export/Standard_format_or_extensible_markup.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

void CDlgExportSFM::OnClickedExSfmInterlinear() {

    CButton * pWnd = (CButton *)GetDlgItem(IDC_EX_SFM_MULTIRECORD);
    if (pWnd) {
        pWnd->SetCheck(FALSE);
    }
}

void CDlgExportSFM::OnClickedExSfmMultirecord() {

    CButton * pWnd = (CButton *)GetDlgItem(IDC_EX_SFM_INTERLINEAR);
    if (pWnd) {
        pWnd->SetCheck(FALSE);
    }
}

void CDlgExportSFM::WriteFileUtf8(CFile * pFile, const CSaString szString) {

	std::string szUtf8 = szString.utf8();
	pFile->Write(szUtf8.c_str(), szUtf8.size());
}

