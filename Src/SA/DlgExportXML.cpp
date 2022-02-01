#include "stdafx.h"
#include "DlgExportXML.h"
#include "sa_doc.h"
#include "Segment.h"
#include "MainFrm.h"
#include "SA_View.h"
#include "GlossSegment.h"
#include "TextSegment.h"
#include "FileUtils.h"
#include "sa_wbch.h"
#include "sa.h"
#include "sa_graph.h"
#include "doclist.h"
#include "DlgResult.h"
#include <math.h>
#include "Process\Process.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_grappl.h"
#include "Process\sa_p_custompitch.h"
#include "Process\sa_p_smoothedpitch.h"
#include "Process\sa_p_melogram.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_spg.h"
#include "Process\FormantTracker.h"
#include "objectostream.h"

using std::ifstream;
using std::ios;
using std::streampos;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExportXML dialog

BEGIN_MESSAGE_MAP(CDlgExportXML, CDialog)
    ON_BN_CLICKED(IDC_EX_SFM_ALL_SOURCE, OnAllSource)
    ON_BN_CLICKED(IDC_EX_SFM_FILE_INFO, OnAllFileInfo)
    ON_BN_CLICKED(IDC_EX_SFM_RECORD_DATA, OnAllParameters)
    ON_BN_CLICKED(IDC_EXTAB_ANNOTATIONS, OnAllAnnotations)
    ON_COMMAND(IDHELP, OnHelpExportBasic)
    ON_BN_CLICKED(IDC_EX_SFM_INTERLINEAR, OnClickedExSfmInterlinear)
    ON_BN_CLICKED(IDC_EX_SFM_MULTIRECORD, OnClickedExSfmMultirecord)
END_MESSAGE_MAP()

// TRE 01-31-2001 Broke the header in two for size restrictions
const char CDlgExportXML::XML_HEADER1[] =
    "<?xml version = \"1.0\"?>\r\n"
    "\r\n"
#ifndef XML_EXCLUDE_DOCUMENT_DEFINITION
    "<!DOCTYPE SAData [\r\n"
    "  <!ELEMENT SAdoc (FileInfo?,LinguisticInfo?,SpeakerInfo?)>\r\n"
    "\r\n"
    "  <!ELEMENT FileInfo (FileDescription?,AudioFile?,CreationTime?,LastEdit?,FileSize?,OrigFormat?,DatabaseName?)>\r\n"
    "    <!ELEMENT FileDescription (#PCDATA)>\r\n"
    "    <!ELEMENT AudioFile (#PCDATA)>\r\n"
    "    <!ELEMENT CreationTime (#PCDATA)>\r\n"
    "    <!ELEMENT LastEdit (#PCDATA)>\r\n"
    "    <!ELEMENT FileSize (#PCDATA)>\r\n"
    "    <!ELEMENT OrigFormat (#PCDATA)>\r\n"
    "    <!ELEMENT DatabaseName (#PCDATA)>\r\n"
    "\r\n"
    "  <!ELEMENT LinguisticInfo (PhoneticText?,Tone?,PhonemicText?,Orthographic?,Gloss?,GlossNat?,Reference?,PhraseLevel1?,PhraseLevel2?,PhraseLevel3?,PhraseLevel4?,FreeTranslation?,NumPhones?,NumWords?)>\r\n"
    "    <!ELEMENT PhoneticText (#PCDATA)>\r\n"
    "    <!ELEMENT Tone (#PCDATA)>\r\n"
    "    <!ELEMENT PhonemicText (#PCDATA)>\r\n"
    "    <!ELEMENT Orthographic (#PCDATA)>\r\n"
    "    <!ELEMENT Gloss (#PCDATA)>\r\n"
    "    <!ELEMENT GlossNat (#PCDATA)>\r\n"
    "    <!ELEMENT Reference (#PCDATA)>\r\n"
    "    <!ELEMENT PhraseLevel1 (#PCDATA)>\r\n"
    "    <!ELEMENT PhraseLevel2 (#PCDATA)>\r\n"
    "    <!ELEMENT PhraseLevel3 (#PCDATA)>\r\n"
    "    <!ELEMENT PhraseLevel4 (#PCDATA)>\r\n"
    "    <!ELEMENT FreeTranslation (#PCDATA)>\r\n"
    "    <!ELEMENT NumPhones (#PCDATA)>\r\n"
    "    <!ELEMENT NumWords (#PCDATA)>\r\n"
    "\r\n";
#endif
const char CDlgExportXML::XML_HEADER2[] =
#ifndef XML_EXCLUDE_DOCUMENT_DEFINITION
    "  <!ELEMENT DataInfo (NumSamples?,Length?,SamplingFreq?,Bandwidth?,HighPassFiltered?,StorageFormat?,QuantizSize?)>\r\n"
    "    <!ELEMENT NumSamples (#PCDATA)>\r\n"
    "    <!ELEMENT Length (#PCDATA)>\r\n"
    "    <!ELEMENT SamplingFreq (#PCDATA)>\r\n"
    "    <!ELEMENT Bandwidth (#PCDATA)>\r\n"
    "    <!ELEMENT HighPassFiltered (#PCDATA)>\r\n"
    "    <!ELEMENT StorageFormat (#PCDATA)>\r\n"
    "    <!ELEMENT QuantizSize (#PCDATA)>\r\n"
    "\r\n"
    "  <!ELEMENT SourceInfo (Language?,Dialect?,Country?,family?,Region?,EthnologueID?,Speaker?,Gender?,NotebookReference?,Transcriber?)>\r\n"
    "    <!ELEMENT Language (#PCDATA)>\r\n"
    "    <!ELEMENT Dialect (#PCDATA)>\r\n"
    "    <!ELEMENT Family (#PCDATA)>\r\n"
    "    <!ELEMENT Country (#PCDATA)>\r\n"
    "    <!ELEMENT Region (#PCDATA)>\r\n"
    "    <!ELEMENT EthnologueID (#PCDATA)>\r\n"
    "    <!ELEMENT Speaker (#PCDATA)>\r\n"
    "    <!ELEMENT Gender (#PCDATA)>\r\n"
    "    <!ELEMENT NotebookReference (#PCDATA)>\r\n"
    "    <!ELEMENT Transcriber (#PCDATA)>\r\n"
    "  <!ELEMENT Data (Staff?)>\r\n"
    "    <!ELEMENT Staff (#PCDATA)>\r\n"
    "]>\r\n"
    "\r\n"
#endif
    "<SAdoc>\r\n"
    "\r\n";

const char CDlgExportXML::XML_FOOTER[] = "\r\n\r\n</SAdoc>";

CDlgExportXML::CDlgExportXML(const CSaString & szDocTitle, CWnd * pParent) :
    CDialog(CDlgExportXML::IDD, pParent) {

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
    m_bGlossNat = TRUE;
    m_bOrtho = TRUE;
    m_bPhonemic = TRUE;
    m_bPhonetic = TRUE;
    m_bReference = FALSE;
    m_bTone = FALSE;
    m_bPhrase = FALSE;
    m_bCountry = FALSE;
    m_bQuantization = FALSE;
    m_szDocTitle = szDocTitle;
}

BOOL CDlgExportXML::OnInitDialog() {

    CDialog::OnInitDialog();

    OnAllAnnotations();
    OnAllSource();
    OnAllParameters();
    OnAllFileInfo();

    SetEnable(IDC_EX_SFM_INTERLINEAR, FALSE);

    CenterWindow();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgExportXML::OutputXMLField(CFile * pFile, LPCTSTR szFieldName, const CSaString & szContents) {

    if (!szContents || !*szContents) {
        return;
    }

    CSaString szString;
    szString = "\t<";
    szString += szFieldName;
    szString += ">";
    for (register int i=0; i<szContents.GetLength(); ++i) {
        if (szContents[i]=='<') {
            szString += "&#60;";
        } else if (szContents[i]=='>') {
            szString += "&#62;";
        } else if (szContents[i]=='&') {
            szString += "&#38;";
        } else if (szContents[i]<0) {
            CSaString szS;
            swprintf_s(szS.GetBuffer(25),25,_T("&#%ld;"),(unsigned char) szContents[i]);
            szString += szS;
        } else {
            szString += szContents[i];
        }
    }
    szString += "</";
    szString += szFieldName;
    szString += ">\r\n";
    WriteFileUtf8(pFile, szString);
}

void CDlgExportXML::OnOK() {

    wstring filename;
    int result = CSaDoc::GetSaveAsFilename(m_szDocTitle, _T("Extensible Markup (*.xml) |*.xml||"), _T("xml"), NULL, filename);
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
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bGlossNat = m_bPhrase = TRUE;
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

    CFile * pFile = new CFile(m_szFileName, CFile::modeCreate|CFile::modeWrite);
    CSaString szString;

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    pFile->Write(XML_HEADER1,sizeof(XML_HEADER1)-1);
    pFile->Write(XML_HEADER2,sizeof(XML_HEADER2)-1);

    pFile->Write("<FileInfo>\r\n",12);

    if (m_bComments) {
        OutputXMLField(pFile,_T("Comments"),pDoc->GetDescription());
    }
    if (m_bFileName) {
        OutputXMLField(pFile,_T("AudioFile"),pDoc->GetPathName());
    }

    CFileStatus * pFileStatus = pDoc->GetFileStatus();
    if (pFileStatus->m_szFullName[0] != 0) { // file name is defined
        if (m_bOriginalDate) {
            OutputXMLField(pFile,_T("CreationTime"),pFileStatus->m_ctime.Format("%A, %B %d, %Y, %X"));
        }
        if (m_bLastModified) {
            OutputXMLField(pFile,_T("LastEdit"),pFileStatus->m_mtime.Format("%A, %B %d, %Y, %X"));
        }
        if (m_bFileSize) {
            swprintf_s(szString.GetBuffer(25),25,_T("%lld Bytes"),pFileStatus->m_size);
            szString.ReleaseBuffer();
            OutputXMLField(pFile,_T("FileSize"),szString);
        }
        if (m_bOriginalFormat) {
            if (pDoc->IsValidRecordFileFormat()) {
                szString.LoadString((UINT)pDoc->GetRecordFileFormat() + IDS_FILETYPE_UTT);
                OutputXMLField(pFile,_T("OrigFormat"),szString);
            }
        }
    }

    pFile->Write("</FileInfo>\r\n",13);

    pFile->Write("<LinguisticInfo>\r\n",18);
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
            if ((!m_bInterlinear) && (bBreak)) {
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
        if (m_bPhonetic) {
            OutputXMLField(pFile,_T("PhoneticText"),szAnnotation[PHONETIC]);
        }
        if (m_bTone) {
            OutputXMLField(pFile,_T("Tone"),szAnnotation[TONE]);
        }
        if (m_bPhonemic) {
            OutputXMLField(pFile,_T("PhonemicText"),szAnnotation[PHONEMIC]);
        }
        if (m_bOrtho) {
            OutputXMLField(pFile,_T("Orthographic"),szAnnotation[ORTHO]);
        }
        if (m_bGloss) {
            OutputXMLField(pFile,_T("Gloss"),szAnnotation[GLOSS]);
        }
        if (m_bGlossNat) {
            OutputXMLField(pFile,_T("GlossNat"),szAnnotation[GLOSS_NAT]);
        }
        if (m_bReference) {
            OutputXMLField(pFile,_T("Reference"),szAnnotation[REFERENCE]);
        }
    }

    for (int nPhrase = MUSIC_PL1; nPhrase <= MUSIC_PL4; nPhrase++) {
        CSaString szPhrase;
        szString.Format(_T("PhraseLevel%d"), nPhrase - MUSIC_PL1 + 1);
        if ((m_bPhrase) && (!pDoc->GetSegment(nPhrase)->IsEmpty())) {
            int nNumber = 0;
            while (nNumber != -1) {
                szPhrase += pDoc->GetSegment(nPhrase)->GetSegmentString(nNumber);
                szPhrase += L" ";
                nNumber = pDoc->GetSegment(nPhrase)->GetNext(nNumber);
            }
        }
        if (m_bPhrase) {
            szPhrase.TrimRight();
            OutputXMLField(pFile,szString,szPhrase);
        }
    }

    if (m_bFree) {
        OutputXMLField(pFile,_T("FreeTranslation"),pDoc->GetSourceParm()->szFreeTranslation);
    }
    if (m_bPhones) {
        // create and write number of phones text
        int nNumber = 0;
        int nLoop = 0;
        if (pDoc->GetSegment(PHONETIC)->GetContentLength() > 0) {
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
        OutputXMLField(pFile,_T("NumPhones"),szString);
    }
    if (m_bWords) {
        swprintf_s(szString.GetBuffer(25),25,_T("%u"), ((CTextSegment *)pDoc->GetSegment(GLOSS))->CountWords());
        szString.ReleaseBuffer();
        OutputXMLField(pFile,_T("NumWords"),szString);
    }
    pFile->Write("</LinguisticInfo>\r\n",19);

    pFile->Write("<DataInfo>\r\n",12);
    if (m_bNumberSamples) {
        swprintf_s(szString.GetBuffer(25),25,_T("%ld"), pDoc->GetNumSamples());
        szString.ReleaseBuffer();
        OutputXMLField(pFile,_T("NumSamples"),szString);
    }
    if (m_bLength) {
        // create and write length text
        double fDataSec = pDoc->GetTimeFromBytes(pDoc->GetDataSize()); // get sampled data size in seconds
        int nMinutes = (int)fDataSec / 60;

        if (nMinutes == 0) {
            // length is less than one minute
            swprintf_s(szString.GetBuffer(25),25,_T("%5.3f Seconds"), fDataSec);
        } else {
            // length is equal or more than one minute
            fDataSec = fDataSec - (float)(nMinutes * 60);
            swprintf_s(szString.GetBuffer(25),25,_T("%i:%5.3f (Min:Sec)"), nMinutes, fDataSec);
        }
        szString.ReleaseBuffer();
        OutputXMLField(pFile,_T("Length"),szString);
    }
    if (m_bSampleRate) {
        swprintf_s(szString.GetBuffer(25),25,_T("%lu Hz"),pDoc->GetSamplesPerSec());
        szString.ReleaseBuffer();
        OutputXMLField(pFile,_T("SamplingFreq"),szString);
    }
    if (m_bBandwidth) {
        swprintf_s(szString.GetBuffer(25),25,_T("%lu Hz"),pDoc->GetRecordBandWidth());
        szString.ReleaseBuffer();
        OutputXMLField(pFile,_T("Bandwidth"),szString);
    }
    if (m_bHighPass) {
        szString = pDoc->IsUsingHighPassFilter() ? "Yes":"No";
        OutputXMLField(pFile,_T("HighPassFiltered"),szString);
    }
    if (m_bBits) {
        swprintf_s(szString.GetBuffer(25),25,_T("%d Bits"),pDoc->GetBitsPerSample());
        szString.ReleaseBuffer();
        OutputXMLField(pFile,_T("StorageFormat"),szString);
    }
    if (m_bQuantization) {
        swprintf_s(szString.GetBuffer(25),25,_T("%d Bits"),(int)pDoc->GetRecordSampleSize());
        szString.ReleaseBuffer();
        OutputXMLField(pFile,_T("QuantizSize"),szString);
    }
    pFile->Write("</DataInfo>\r\n",13);

    pFile->Write("<SourceInfo>\r\n",14);
    if (m_bLanguage) {
        OutputXMLField(pFile,_T("Language"),pDoc->GetSourceParm()->szLanguage);
    }

    if (m_bDialect) {
        OutputXMLField(pFile,_T("Dialect"),pDoc->GetSourceParm()->szDialect);
    }
    if (m_bFamily) {
        OutputXMLField(pFile,_T("Family"),pDoc->GetSourceParm()->szFamily);
    }
    if (m_bCountry) {
        OutputXMLField(pFile,_T("Country"),pDoc->GetSourceParm()->szCountry);
    }
    if (m_bRegion) {
        OutputXMLField(pFile,_T("Region"),pDoc->GetSourceParm()->szRegion);
    }
    if (m_bEthnologue) {
        OutputXMLField(pFile,_T("EthnologueID"),pDoc->GetSourceParm()->szEthnoID);
    }
    if (m_bSpeaker) {
        OutputXMLField(pFile,_T("SpeakerName"),pDoc->GetSourceParm()->szSpeaker);
    }
    if (m_bGender) {
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
        OutputXMLField(pFile,_T("Gender"),szString);
    }
    if (m_bNotebookRef) {
        OutputXMLField(pFile,_T("NotebookReference"),pDoc->GetSourceParm()->szReference);
    }
    if (m_bTranscriber) {
        OutputXMLField(pFile,_T("Transcriber"),pDoc->GetSourceParm()->szTranscriber);
    }
    pFile->Write("</SourceInfo>\r\n",14);

    pFile->Write("<Data>\r\n",8);
    pFile->Write("</Data>\r\n",9);

    pFile->Write(XML_FOOTER,sizeof(XML_FOOTER)-1);

    if (pFile) {
        delete pFile;
    }
    CDialog::OnOK();
}

void CDlgExportXML::DoDataExchange(CDataExchange * pDX) {

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
    DDX_Check(pDX, IDC_EXTAB_GLOSS_NAT, m_bGlossNat);
    DDX_Check(pDX, IDC_EXTAB_ORTHO, m_bOrtho);
    DDX_Check(pDX, IDC_EXTAB_PHONEMIC, m_bPhonemic);
    DDX_Check(pDX, IDC_EXTAB_PHONETIC, m_bPhonetic);
    DDX_Check(pDX, IDC_EXTAB_REFERENCE, m_bReference);
    DDX_Check(pDX, IDC_EXTAB_TONE, m_bTone);
    DDX_Check(pDX, IDC_EXTAB_PHRASE, m_bPhrase);
    DDX_Check(pDX, IDC_EX_SFM_COUNTRY, m_bCountry);
    DDX_Check(pDX, IDC_EX_SFM_QUANTIZATION, m_bQuantization);
}

void CDlgExportXML::OnAllAnnotations() {

    UpdateData(TRUE);
    BOOL bEnable = !m_bAllAnnotations;
    SetEnable(IDC_EXTAB_PHONETIC, bEnable);
    SetEnable(IDC_EXTAB_TONE, bEnable);
    SetEnable(IDC_EXTAB_PHONEMIC, bEnable);
    SetEnable(IDC_EXTAB_ORTHO, bEnable);
    SetEnable(IDC_EXTAB_GLOSS, bEnable);
    SetEnable(IDC_EXTAB_GLOSS_NAT, bEnable);
    SetEnable(IDC_EXTAB_REFERENCE, bEnable);
    SetEnable(IDC_EXTAB_PHRASE, bEnable);
    if (m_bAllAnnotations) {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bGlossNat = m_bPhrase = TRUE;
        UpdateData(FALSE);
    } else {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bGlossNat = m_bPhrase = FALSE;
        UpdateData(FALSE);
    }
}

void CDlgExportXML::OnAllFileInfo() {

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

void CDlgExportXML::OnAllParameters() {

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

void CDlgExportXML::OnAllSource() {

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

void CDlgExportXML::SetEnable(int nItem, BOOL bEnable) {

    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd) {
        pWnd->EnableWindow(bEnable);
    }
}

void CDlgExportXML::SetCheck(int nItem, BOOL bChecked) {

    CButton * pWnd = (CButton *)GetDlgItem(nItem);
    if (pWnd) {
        pWnd->SetCheck(bChecked);
    }
}

void CDlgExportXML::OnHelpExportBasic() {

    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/File/Export/Standard_format_or_extensible_markup.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

void CDlgExportXML::OnClickedExSfmInterlinear() {

    CButton * pWnd = (CButton *)GetDlgItem(IDC_EX_SFM_MULTIRECORD);
    if (pWnd) {
        pWnd->SetCheck(FALSE);
    }
}

void CDlgExportXML::OnClickedExSfmMultirecord() {
    CButton * pWnd = (CButton *)GetDlgItem(IDC_EX_SFM_INTERLINEAR);
    if (pWnd) {
        pWnd->SetCheck(FALSE);
    }
}

void CDlgExportXML::WriteFileUtf8(CFile * pFile, const CSaString szString) {

    std::string szUtf8 = szString.utf8();
    pFile->Write(szUtf8.c_str(), szUtf8.size());
}
