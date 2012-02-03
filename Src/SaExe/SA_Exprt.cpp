/////////////////////////////////////////////////////////////////////////////
// sa_exprt.cpp:
// Implementation of the CExportSFM
//                       CExportXML
//                       CExportTabbed class.
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
// 1.5Test8.5
//         SDM Changed Table headings to use Initial Uppercase ("Emic" & "Etic")
// 1.5Test10.7
//         SDM Changed Import to not pad extra phonetic with spaces
// 1.5Test11.0
//         SDM replaced GetOffset() + GetDuration() with CSegment::GetStop()
//         SDM fixed bug in export which crashed if phonetic segment empty
//         SDM changed export to export AutoPitch (Grappl)
// 1.5Test11.3
//         SDM changed CSegment::AdjustPositionAll to CSegment::Adjust
//
//    07/27/2000
//         DDO Changed so these dialogs display before the user is asked
//             what file to export. Therefore, I had to move the get filename
//             dialog call into this module instead of doing it in the view
//             class when the user picks one of the export menu items.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process\sa_proc.h"
#include "sa_segm.h"
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
#include "dsp\formants.h"
#include "dsp\ztransform.h"
#include "Process\formanttracker.h"
#include "dsp\mathx.h"

#include "sa_wbch.h"
#include "sa.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_graph.h"
#include "mainfrm.h"
#include "settings\obstream.h"
#include "doclist.h"
using std::ifstream;
using std::ios;
using std::streampos;

#include "sa_exprt.h"
#include "result.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static void WriteFileUtf8( CFile *pFile, const CSaString szString)
{
	std::string szUtf8 = szString.utf8();
	pFile->Write(szUtf8.c_str(),szUtf8.size());
}

//****************************************************************************
// Added on 07/27/200 by DDO.
//****************************************************************************
static CSaString GetExportFilename(CSaString szTitle, CSaString szFilter,TCHAR *szExtension=_T("txt"))
{
	//**************************************
	// Extract what's to left of :
	//**************************************
	int nFind = szTitle.Find(':');
	if (nFind != -1) szTitle = szTitle.Left(nFind);
	nFind = szTitle.ReverseFind('.');

	//**************************************
	// Remove extension if necessary.
	//**************************************
	if (nFind >= ((szTitle.GetLength() > 3) ? (szTitle.GetLength() - 4) : 0))
		szTitle = szTitle.Left(nFind);

	CFileDialog dlgFile(FALSE, szExtension, szTitle, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL);
	if (dlgFile.DoModal() != IDOK) return "";
	return dlgFile.GetPathName();
}

/////////////////////////////////////////////////////////////////////////////
// CExportBasicDialog dialog


CExportBasicDialog::CExportBasicDialog(const CSaString& szDocTitle, CWnd* pParent /*=NULL*/)
: CDialog(CExportBasicDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportBasicDialog)
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
	m_bOrtho = TRUE;
	m_bPhonemic = TRUE;
	m_bPhonetic = TRUE;
	m_bPOS = FALSE;
	m_bReference = FALSE;
	m_bTone = FALSE;
	m_bPhrase = FALSE;
	m_bCountry = FALSE;
	m_bQuantization = FALSE;
	//}}AFX_DATA_INIT
	m_szDocTitle = szDocTitle;
}

void CExportBasicDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportBasicDialog)
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
	DDX_Check(pDX, IDC_EXTAB_ORTHO, m_bOrtho);
	DDX_Check(pDX, IDC_EXTAB_PHONEMIC, m_bPhonemic);
	DDX_Check(pDX, IDC_EXTAB_PHONETIC, m_bPhonetic);
	DDX_Check(pDX, IDC_EXTAB_POS, m_bPOS);
	DDX_Check(pDX, IDC_EXTAB_REFERENCE, m_bReference);
	DDX_Check(pDX, IDC_EXTAB_TONE, m_bTone);
	DDX_Check(pDX, IDC_EXTAB_PHRASE, m_bPhrase);
	DDX_Check(pDX, IDC_EX_SFM_COUNTRY, m_bCountry);
	DDX_Check(pDX, IDC_EX_SFM_QUANTIZATION, m_bQuantization);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExportBasicDialog, CDialog)
	//{{AFX_MSG_MAP(CExportBasicDialog)
	ON_BN_CLICKED(IDC_EX_SFM_ALL_SOURCE, OnAllSource)
	ON_BN_CLICKED(IDC_EX_SFM_FILE_INFO, OnAllFileInfo)
	ON_BN_CLICKED(IDC_EX_SFM_RECORD_DATA, OnAllParameters)
	ON_BN_CLICKED(IDC_EXTAB_ANNOTATIONS, OnAllAnnotations)
	ON_COMMAND(IDHELP, OnHelpExportBasic)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CExportBasicDialog message handlers

void CExportBasicDialog::OnAllAnnotations()
{
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
	if (m_bAllAnnotations)
	{
		m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = TRUE;
		UpdateData(FALSE);
	}
	else 
	{
		m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = FALSE;
		UpdateData(FALSE);
	}

}

void CExportBasicDialog::OnAllFileInfo()
{
	UpdateData(TRUE);
	BOOL bEnable = !m_bAllFile;
	SetEnable(IDC_EX_SFM_FILE_SIZE, bEnable);
	SetEnable(IDC_EX_SFM_LAST_DATE, bEnable);
	SetEnable(IDC_EX_SFM_ORIGINAL_DATE, bEnable);
	SetEnable(IDC_EX_SFM_ORIGINAL_FORMAT, bEnable);
	if (m_bAllFile)
	{
		m_bOriginalDate = m_bLastModified = m_bOriginalFormat = m_bFileSize = TRUE;
		UpdateData(FALSE);
	} 
	else 
	{
		m_bOriginalDate = m_bLastModified = m_bOriginalFormat = m_bFileSize = FALSE;
		UpdateData(FALSE);
	}
}

void CExportBasicDialog::OnAllParameters()
{
	UpdateData(TRUE);
	BOOL bEnable = !m_bAllParameters;
	SetEnable(IDC_EX_SFM_BANDWIDTH, bEnable);
	SetEnable(IDC_EX_SFM_BITS, bEnable);
	SetEnable(IDC_EX_SFM_QUANTIZATION, bEnable);
	SetEnable(IDC_EX_SFM_HIGHPASS, bEnable);
	SetEnable(IDC_EX_SFM_LENGTH, bEnable);
	SetEnable(IDC_EX_SFM_NUMBER_OF_SAMPLES, bEnable);
	SetEnable(IDC_EX_SFM_RATE, bEnable);

	if (m_bAllParameters)
	{
		m_bNumberSamples = m_bLength = m_bSampleRate = m_bBandwidth = m_bHighPass = m_bBits = m_bQuantization = TRUE;
		UpdateData(FALSE);
	} 
	else 
	{
		m_bNumberSamples = m_bLength = m_bSampleRate = m_bBandwidth = m_bHighPass = m_bBits = m_bQuantization = FALSE;
		UpdateData(FALSE);
	}

}

void CExportBasicDialog::OnAllSource()
{
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
	if (m_bAllSource)
	{
		m_bLanguage = m_bDialect = m_bSpeaker = m_bGender = m_bEthnologue = m_bFamily = m_bRegion = m_bNotebookRef =
			m_bTranscriber = m_bComments = m_bCountry = TRUE;
		UpdateData(FALSE);
	} 
	else 
	{
		m_bLanguage = m_bDialect = m_bSpeaker = m_bGender = m_bEthnologue = m_bFamily = m_bRegion = m_bNotebookRef =
			m_bTranscriber = m_bComments = m_bCountry = FALSE;
		UpdateData(FALSE);
	}

}

BOOL CExportBasicDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	OnAllAnnotations();
	OnAllSource();
	OnAllParameters();
	OnAllFileInfo();

	CenterWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}


/***************************************************************************/
// CExportBasicDialog::SetEnable Enables/Disables controls
/***************************************************************************/
void CExportBasicDialog::SetEnable(int nItem, BOOL bEnable)
{
	CWnd* pWnd = GetDlgItem(nItem);

	if (pWnd)
	{
		pWnd->EnableWindow(bEnable);
	}
}

/***************************************************************************/
// CExportBasicDialog::OnHelpExportBasic Call Basic Export help
/***************************************************************************/
void CExportBasicDialog::OnHelpExportBasic()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath = szPath + "::/User_Interface/Menus/File/Export/Standard_format_or_extensible_markup.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CExportXML dialog

BEGIN_MESSAGE_MAP(CExportXML, CExportBasicDialog)
	//{{AFX_MSG_MAP(CExportXML)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// TRE 01-31-2001 Broke the header in two for size restrictions
const char CExportXML::XML_HEADER1[] =
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
"  <!ELEMENT LinguisticInfo (PhoneticText?,Tone?,PhonemicText?,Orthographic?,Gloss?,PartOfSpeech?,Reference?,PhraseLevel1?,PhraseLevel2?,PhraseLevel3?,PhraseLevel4?,FreeTranslation?,NumPhones?,NumWords?)>\r\n"
"    <!ELEMENT PhoneticText (#PCDATA)>\r\n"
"    <!ELEMENT Tone (#PCDATA)>\r\n"
"    <!ELEMENT PhonemicText (#PCDATA)>\r\n"
"    <!ELEMENT Orthographic (#PCDATA)>\r\n"
"    <!ELEMENT Gloss (#PCDATA)>\r\n"
"    <!ELEMENT PartOfSpeech (#PCDATA)>\r\n"
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
const char CExportXML::XML_HEADER2[] =
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

const char CExportXML::XML_FOOTER[] = "\r\n"
"\r\n"
"</SAdoc>";


void CExportXML::OutputXMLField(CFile* pFile,const TCHAR *szFieldName,const CSaString &szContents)
{
	if(!szContents || !*szContents) return;

	CSaString szString;
	szString = "\t<";
	szString += szFieldName;
	szString += ">";
	for(register int i=0;i<szContents.GetLength();++i)
	{
		if(szContents[i]=='<')
			szString += "&#60;";
		else if(szContents[i]=='>')
			szString += "&#62;";
		else if(szContents[i]=='&')
			szString += "&#38;";
		else if(szContents[i]<0)
		{
			CSaString szS;
			swprintf_s(szS.GetBuffer(25),25,_T("&#%ld;"),(unsigned char) szContents[i]);
			szString += szS;
		}
		else szString += szContents[i];
	}
	szString += "</";
	szString += szFieldName;
	szString += ">\r\n";
	WriteFileUtf8(pFile, szString);
}

/////////////////////////////////////////////////////////////////////////////
// CExportXML message handlers

void CExportXML::OnOK()
{
	if ((m_szFileName = GetExportFilename(m_szDocTitle, _T("Extensible Markup (*.xml) |*.xml||"),_T("xml"))) == "") return;

	UpdateData(TRUE);

	// process all flags
	if (m_bAllAnnotations)
		m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = TRUE;

	if (m_bAllFile)
		m_bOriginalDate = m_bLastModified = m_bOriginalFormat = m_bFileSize = TRUE;

	if (m_bAllParameters)
		m_bNumberSamples = m_bLength = m_bSampleRate = m_bBandwidth = m_bHighPass = m_bBits = m_bQuantization = TRUE;

	if (m_bAllSource)
		m_bLanguage = m_bDialect = m_bSpeaker = m_bGender = m_bEthnologue = m_bFamily = m_bRegion = m_bNotebookRef =
		m_bTranscriber = m_bComments = m_bCountry = TRUE;

	CFile* pFile = new CFile(m_szFileName, CFile::modeCreate|CFile::modeWrite);
	CSaString szString;
	CSaString szCrLf = "\r\n";

	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	pFile->Write(XML_HEADER1,sizeof(XML_HEADER1)-1);
	pFile->Write(XML_HEADER2,sizeof(XML_HEADER2)-1);

	pFile->Write("<FileInfo>\r\n",12);

	if (m_bComments) // %%% 1 %%%
	{
		OutputXMLField(pFile,_T("Comments"),pDoc->GetSaParm()->szDescription);
	}
	if (m_bFileName) // %%% 2 %%%
	{
		OutputXMLField(pFile,_T("AudioFile"),pDoc->GetPathName());
	}

	CFileStatus* pFileStatus = pDoc->GetFileStatus();
	if (pFileStatus->m_szFullName[0] != 0) // file name is defined
	{
		if (m_bOriginalDate) // %%% 3 %%%
		{
			OutputXMLField(pFile,_T("CreationTime"),pFileStatus->m_ctime.Format("%A, %B %d, %Y, %X"));
		}
		if (m_bLastModified) // %%% 4 %%%
		{
			OutputXMLField(pFile,_T("LastEdit"),pFileStatus->m_mtime.Format("%A, %B %d, %Y, %X"));
		}
		if (m_bFileSize) // %%% 5 %%%
		{
			swprintf_s(szString.GetBuffer(25),25,_T("%ld Bytes"),pFileStatus->m_size);
			szString.ReleaseBuffer();
			OutputXMLField(pFile,_T("FileSize"),szString);
		}
		if (m_bOriginalFormat) // %%% 6 %%%
		{
			SaParm* pSaParm = pDoc->GetSaParm();
			if (pSaParm->byRecordFileFormat <= FILE_FORMAT_TIMIT)
			{
				szString.LoadString((UINT)pSaParm->byRecordFileFormat + IDS_FILETYPE_UTT);
				OutputXMLField(pFile,_T("OrigFormat"),szString);
			}
		}
	}

	pFile->Write("</FileInfo>\r\n",13);

	pFile->Write("<LinguisticInfo>\r\n",18);
	if (!pDoc->GetSegment(PHONETIC)->IsEmpty())
	{
		CSaString szAnnotation[ANNOT_WND_NUMBER];
		CSaString szPOS;
		int nMaxLength = 0;
		int nNumber = 0;
		DWORD dwOffset;

		while (nNumber != -1)
		{
			BOOL bBreak = FALSE;

			dwOffset = pDoc->GetSegment(PHONETIC)->GetOffset(nNumber);

			int nFind = pDoc->GetSegment(GLOSS)->FindOffset(dwOffset);
			if ((nNumber > 0) && (nFind != -1))
				bBreak = TRUE;
			if (!m_bInterlinear && bBreak)
			{
				for (int nLoop = PHONETIC; nLoop < ANNOT_WND_NUMBER; nLoop++)
				{
					szAnnotation[nLoop] += " ";
				}
				szPOS +=" ";
			}
			else if (bBreak)
			{
				for (int nLoop = PHONETIC; nLoop < ANNOT_WND_NUMBER; nLoop++)
				{
					while (szAnnotation[nLoop].GetLength() <= nMaxLength)
						szAnnotation[nLoop] += " ";
				}
				while (szPOS.GetLength() <= nMaxLength)
					szPOS +=" ";
			}

			szAnnotation[PHONETIC] += pDoc->GetSegment(PHONETIC)->GetSegmentString(nNumber);

			for (int nLoop = PHONETIC+1; nLoop < ANNOT_WND_NUMBER; nLoop++)
			{
				nFind = pDoc->GetSegment(nLoop)->FindOffset(dwOffset);
				if (nFind != -1)
				{
					if (nLoop == GLOSS)
					{
						szAnnotation[nLoop] += pDoc->GetSegment(nLoop)->GetSegmentString(nFind).Mid(1);
						szPOS += ((CGlossSegment*)pDoc->GetSegment(GLOSS))->GetPOSs()->GetAt(nFind);
						if (szPOS.GetLength() > nMaxLength) nMaxLength = szPOS.GetLength();
					}
					else
					{
						szAnnotation[nLoop] += pDoc->GetSegment(nLoop)->GetSegmentString(nFind);
					}
				}
				if (szAnnotation[nLoop].GetLength() > nMaxLength) nMaxLength = szAnnotation[nLoop].GetLength();
			}

			nNumber = pDoc->GetSegment(PHONETIC)->GetNext(nNumber);
		}
		// write out results
		if (m_bPhonetic) // %%% 8 %%%
		{
			OutputXMLField(pFile,_T("PhoneticText"),szAnnotation[PHONETIC]);
		}
		if (m_bTone) // %%% 9 %%%
		{
			OutputXMLField(pFile,_T("Tone"),szAnnotation[TONE]);
		}
		if (m_bPhonemic) // %%% 10 %%%
		{
			OutputXMLField(pFile,_T("PhonemicText"),szAnnotation[PHONEMIC]);
		}
		if (m_bOrtho) // %%% 11 %%%
		{
			OutputXMLField(pFile,_T("Orthographic"),szAnnotation[ORTHO]);
		}
		if (m_bGloss) // %%% 12 %%%
		{
			OutputXMLField(pFile,_T("Gloss"),szAnnotation[GLOSS]);
		}
		if (m_bPOS) // %%% 13 %%%
		{
			OutputXMLField(pFile,_T("PartOfSpeech"),szPOS);
		}
		if (m_bReference) // %%% 14 %%%
		{
			OutputXMLField(pFile,_T("Reference"),szAnnotation[REFERENCE]);
		}
	}

	for (int nPhrase = MUSIC_PL1; nPhrase <= MUSIC_PL4; nPhrase++)
	{
		CSaString szPhrase;

		szString.Format(_T("PhraseLevel%d "), nPhrase - MUSIC_PL1 + 1);

		if (m_bPhrase && !pDoc->GetSegment(nPhrase)->IsEmpty())
		{
			int nNumber = 0;

			while (nNumber != -1)
			{
				szPhrase += pDoc->GetSegment(nPhrase)->GetSegmentString(nNumber);

				nNumber = pDoc->GetSegment(nPhrase)->GetNext(nNumber);
			}
		}
		if (m_bPhrase)// %%% 15 %%% - %%% 18 %%%
		{
			OutputXMLField(pFile,szString,szPhrase);
		}
	}

	if (m_bFree) // %%% 15 %%%
	{
		OutputXMLField(pFile,_T("FreeTranslation"),pDoc->GetSourceParm()->szFreeTranslation);
	}
	if (m_bPhones) // %%% 16 %%%
	{
		// create and write number of phones text
		int nNumber = 0;
		int nLoop = 0;
		if (pDoc->GetSegment(PHONETIC)->GetString()->GetLength() > 0)
		{
			// find number of phones
			while(TRUE)
			{ nLoop++;
			nNumber = pDoc->GetSegment(PHONETIC)->GetNext(nNumber);
			if (nNumber < 0) break;
			}
		}
		swprintf_s(szString.GetBuffer(25),25,_T("%u"), nLoop);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("NumPhones"),szString);
	}
	if (m_bWords) // %%% 17 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%u"), ((CTextSegment*)pDoc->GetSegment(GLOSS))->CountWords());
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("NumWords"),szString);
	}
	pFile->Write("</LinguisticInfo>\r\n",19);

	pFile->Write("<DataInfo>\r\n",12);
	if (m_bNumberSamples) // %%% 18 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%ld"), pDoc->GetDataSize() / pDoc->GetFmtParm()->wBlockAlign);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("NumSamples"),szString);
	}
	if (m_bLength) // %%% 19 %%%
	{
		// create and write length text
		double fDataSec = pDoc->GetTimeFromBytes(pDoc->GetUnprocessedDataSize()); // get sampled data size in seconds
		int nMinutes = (int)fDataSec / 60;

		if (nMinutes == 0) // length is less than one minute
			swprintf_s(szString.GetBuffer(25),25,_T("%5.3f Seconds"), fDataSec);
		else // length is equal or more than one minute
		{
			fDataSec = fDataSec - (float)(nMinutes * 60);
			swprintf_s(szString.GetBuffer(25),25,_T("%i:%5.3f (Min:Sec)"), nMinutes, fDataSec);
		}
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("Length"),szString);
	}
	if (m_bSampleRate) // %%% 20 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%lu Hz"),pDoc->GetFmtParm()->dwSamplesPerSec);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("SamplingFreq"),szString);
	}
	if (m_bBandwidth) // %%% 21 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%lu Hz"),pDoc->GetSaParm()->dwRecordBandWidth);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("Bandwidth"),szString);
	}
	if (m_bHighPass) // %%% 22 %%%
	{
		szString = pDoc->GetSaParm()->wFlags & SA_FLAG_HIGHPASS ? "Yes":"No";
		OutputXMLField(pFile,_T("HighPassFiltered"),szString);
	}
	if (m_bBits) // %%% 23 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%d Bits"),pDoc->GetFmtParm()->wBitsPerSample);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("StorageFormat"),szString);
	}
	if (m_bQuantization) // %%% 24 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%d Bits"),(int)pDoc->GetSaParm()->byRecordSmpSize);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("QuantizSize"),szString);
	}
	pFile->Write("</DataInfo>\r\n",13);

	pFile->Write("<SourceInfo>\r\n",14);
	if (m_bLanguage) // %%% 25 %%%
	{
		OutputXMLField(pFile,_T("Language"),pDoc->GetSourceParm()->szLanguage);
	}

	if (m_bDialect) // %%% 26 %%%
	{
		OutputXMLField(pFile,_T("Dialect"),pDoc->GetSourceParm()->szDialect);
	}
	if (m_bFamily) // %%% 27 %%%
	{
		OutputXMLField(pFile,_T("Family"),pDoc->GetSourceParm()->szFamily);
	}
	if (m_bCountry) // %%% 28 %%%
	{
		OutputXMLField(pFile,_T("Country"),pDoc->GetSourceParm()->szCountry);
	}
	if (m_bRegion) // %%% 29 %%%
	{
		OutputXMLField(pFile,_T("Region"),pDoc->GetSourceParm()->szRegion);
	}
	if (m_bEthnologue) // %%% 30 %%%
	{
		OutputXMLField(pFile,_T("EthnologueID"),pDoc->GetSourceParm()->szEthnoID);
	}
	if (m_bSpeaker) // %%% 31 %%%
	{
		OutputXMLField(pFile,_T("SpeakerName"),pDoc->GetSourceParm()->szSpeaker);
	}
	if (m_bGender) // %%% 32 %%%
	{
		switch(pDoc->GetSourceParm()->nGender)
		{
		case 0:  szString = "Adult Male";   break;
		case 1:  szString = "Adult Female"; break;
		case 2:  szString = "Child";        break;
		default: szString = "";             break;
		}
		OutputXMLField(pFile,_T("Gender"),szString);
	}
	if (m_bNotebookRef) // %%% 33 %%%
	{
		OutputXMLField(pFile,_T("NotebookReference"),pDoc->GetSourceParm()->szReference);
	}
	if (m_bTranscriber) // %%% 34 %%%
	{
		OutputXMLField(pFile,_T("Transcriber"),pDoc->GetSourceParm()->szTranscriber);
	}
	pFile->Write("</SourceInfo>\r\n",14);

	pFile->Write("<Data>\r\n",8);
	pFile->Write("</Data>\r\n",9);

	pFile->Write(XML_FOOTER,sizeof(XML_FOOTER)-1);

	if (pFile) delete pFile;
	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CExportSFM dialog

BEGIN_MESSAGE_MAP(CExportSFM, CExportBasicDialog)
	//{{AFX_MSG_MAP(CExportSFM)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



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

/////////////////////////////////////////////////////////////////////////////
// CExportSFM message handlers

void CExportSFM::OnOK()
{
	if ((m_szFileName = GetExportFilename(m_szDocTitle, _T("Standard Format (*.sfm) |*.sfm||"),_T("sfm"))) == "") return;

	UpdateData(TRUE);

	// process all flags
	if (m_bAllAnnotations)
		m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = TRUE;

	if (m_bAllFile)
		m_bOriginalDate = m_bLastModified = m_bOriginalFormat = m_bFileSize = TRUE;

	if (m_bAllParameters)
		m_bNumberSamples = m_bLength = m_bSampleRate = m_bBandwidth = m_bHighPass = m_bBits = m_bQuantization = TRUE;

	if (m_bAllSource)
		m_bLanguage = m_bDialect = m_bSpeaker = m_bGender = m_bEthnologue = m_bFamily = m_bRegion = m_bNotebookRef =
		m_bTranscriber = m_bComments = m_bCountry = TRUE;

	CFile* pFile = new CFile(m_szFileName, CFile::modeCreate|CFile::modeWrite);
	CSaString szString;
	CSaString szCrLf = "\r\n";

	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	// \name write filename
	szString = "\\name " + m_szFileName + szCrLf;
	WriteFileUtf8(pFile, szString);

	// \date write current time
	CTime time = CTime::GetCurrentTime();
	szString = "\\date " + time.Format("%A, %B %d, %Y, %X") + "\r\n";
	WriteFileUtf8(pFile, szString);

	if (m_bFileName) // \wav  Audio FileName
	{
		szString = "\\wav " + pDoc->GetPathName() + szCrLf;
		WriteFileUtf8(pFile, szString);
	}

	if (!pDoc->GetSegment(PHONETIC)->IsEmpty())
	{
		CSaString szAnnotation[ANNOT_WND_NUMBER];
		CSaString szPOS;
		int nMaxLength = 0;
		int nNumber = 0;
		DWORD dwOffset;

		while (nNumber != -1)
		{
			BOOL bBreak = FALSE;

			dwOffset = pDoc->GetSegment(PHONETIC)->GetOffset(nNumber);

			int nFind = pDoc->GetSegment(GLOSS)->FindOffset(dwOffset);
			if ((nNumber > 0) && (nFind != -1))
				bBreak = TRUE;
			if (!m_bInterlinear && bBreak)
			{
				for (int nLoop = PHONETIC; nLoop < ANNOT_WND_NUMBER; nLoop++)
				{
					szAnnotation[nLoop] += " ";
				}
				szPOS +=" ";
			}
			else if (bBreak)
			{
				for (int nLoop = PHONETIC; nLoop < ANNOT_WND_NUMBER; nLoop++)
				{
					while (szAnnotation[nLoop].GetLength() <= nMaxLength)
						szAnnotation[nLoop] += " ";
				}
				while (szPOS.GetLength() <= nMaxLength)
					szPOS +=" ";
			}

			szAnnotation[PHONETIC] += pDoc->GetSegment(PHONETIC)->GetSegmentString(nNumber);

			for (int nLoop = PHONETIC+1; nLoop < ANNOT_WND_NUMBER; nLoop++)
			{
				nFind = pDoc->GetSegment(nLoop)->FindOffset(dwOffset);
				if (nFind != -1)
				{
					if (nLoop == GLOSS)
					{
						szAnnotation[nLoop] += pDoc->GetSegment(nLoop)->GetSegmentString(nFind).Mid(1);
						szPOS += ((CGlossSegment*)pDoc->GetSegment(GLOSS))->GetPOSs()->GetAt(nFind);
						if (szPOS.GetLength() > nMaxLength) nMaxLength = szPOS.GetLength();
					}
					else
					{
						szAnnotation[nLoop] += pDoc->GetSegment(nLoop)->GetSegmentString(nFind);
					}
				}
				if (szAnnotation[nLoop].GetLength() > nMaxLength) nMaxLength = szAnnotation[nLoop].GetLength();
			}

			nNumber = pDoc->GetSegment(PHONETIC)->GetNext(nNumber);
		}
		// write out results
		if (m_bReference)// \ref  Reference
		{
			szString = "\\ref " + szAnnotation[REFERENCE] + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
		if (m_bPhonetic)// \ph   Phonetic text
		{
			szString = "\\ph  " + szAnnotation[PHONETIC] + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
		if (m_bTone)// \tn   Tone
		{
			szString = "\\tn  " + szAnnotation[TONE] + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
		if (m_bPhonemic)// \pm   Phonemic text
		{
			szString = "\\pm  " + szAnnotation[PHONEMIC] + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
		if (m_bOrtho)// \or   Orthographic
		{
			szString = "\\or  " + szAnnotation[ORTHO] + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
		if (m_bGloss)// \gl   Gloss
		{
			szString = "\\gl  " + szAnnotation[GLOSS] + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
		if (m_bPOS)// \pos  Part of Speech
		{
			szString = "\\pos " + szPOS + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
	}

	for (int nPhrase = MUSIC_PL1; nPhrase <= MUSIC_PL4; nPhrase++)
	{
		szString.Format(_T("\\phr%d "), nPhrase - MUSIC_PL1 + 1);
		CSaString szPhrase;

		if (m_bPhrase && !pDoc->GetSegment(nPhrase)->IsEmpty())
		{
			int nNumber = 0;

			while (nNumber != -1)
			{
				szPhrase += pDoc->GetSegment(nPhrase)->GetSegmentString(nNumber);

				nNumber = pDoc->GetSegment(nPhrase)->GetNext(nNumber);
			}
		}
		if (m_bPhrase)// \phr1-\phr3  Phrase Level
		{
			szString = szString + szPhrase + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
	}

	if (m_bFree)// \ft   Free Translation
	{
		szString = "\\ft " + pDoc->GetSourceParm()->szFreeTranslation + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bPhones)// \np   Number of Phones
	{
		// create and write number of phones text
		int nNumber = 0;
		int nLoop = 0;
		if (pDoc->GetSegment(PHONETIC)->GetString()->GetLength() > 0)
		{
			// find number of phones
			while(TRUE)
			{ nLoop++;
			nNumber = pDoc->GetSegment(PHONETIC)->GetNext(nNumber);
			if (nNumber < 0) break;
			}
		}
		swprintf_s(szString.GetBuffer(25),25,_T("%u"), nLoop);
		szString.ReleaseBuffer();
		szString = "\\np " +  szString + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bWords)// \nw   Number of Words
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%u"), ((CTextSegment*)pDoc->GetSegment(GLOSS))->CountWords());
		szString.ReleaseBuffer();
		szString = "\\nw " +  szString + szCrLf;
		WriteFileUtf8(pFile, szString);
	}

	CFileStatus* pFileStatus = pDoc->GetFileStatus();
	if (pFileStatus->m_szFullName[0] != 0) // file name is defined
	{
		if (m_bOriginalDate) // \ct   Creation Time
		{
			szString = "\\ct " + pFileStatus->m_ctime.Format("%A, %B %d, %Y, %X") + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
		if (m_bLastModified) // \le  Last Edit
		{
			szString = "\\le " + pFileStatus->m_mtime.Format("%A, %B %d, %Y, %X") + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
		if (m_bFileSize) // \size File size in bytes
		{
			swprintf_s(szString.GetBuffer(25),25,_T("%ld Bytes"),pFileStatus->m_size);
			szString.ReleaseBuffer();
			szString = "\\size " +  szString + szCrLf;
			WriteFileUtf8(pFile, szString);
		}
		if (m_bOriginalFormat) // \of   Original Format
		{
			SaParm* pSaParm = pDoc->GetSaParm();
			if (pSaParm->byRecordFileFormat <= FILE_FORMAT_TIMIT)
			{
				szString.LoadString((UINT)pSaParm->byRecordFileFormat + IDS_FILETYPE_UTT);
				szString = "\\of " + szString + szCrLf;
				WriteFileUtf8(pFile, szString);
			}
		}
	}

	if (m_bNumberSamples)// \samp Number of Samples
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%ld"), pDoc->GetDataSize() / pDoc->GetFmtParm()->wBlockAlign);
		szString.ReleaseBuffer();
		szString = "\\samp " +  szString + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bLength)// \len  Length
	{
		// create and write length text
		double fDataSec = pDoc->GetTimeFromBytes(pDoc->GetUnprocessedDataSize()); // get sampled data size in seconds
		int nMinutes = (int)fDataSec / 60;

		if (nMinutes == 0) // length is less than one minute
			swprintf_s(szString.GetBuffer(25),25,_T("%5.3f Seconds"), fDataSec);
		else // length is equal or more than one minute
		{
			fDataSec = fDataSec - (float)(nMinutes * 60);
			swprintf_s(szString.GetBuffer(25),25,_T("%i:%5.3f (Min:Sec)"), nMinutes, fDataSec);
		}
		szString.ReleaseBuffer();
		szString = "\\len " +  szString + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bSampleRate)// \freq Sampling Frequency
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%lu Hz"),pDoc->GetFmtParm()->dwSamplesPerSec);
		szString.ReleaseBuffer();
		szString = "\\freq " +  szString + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bBandwidth)// \bw   Bandwidth
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%lu Hz"),pDoc->GetSaParm()->dwRecordBandWidth);
		szString.ReleaseBuffer();
		szString = "\\bw " +  szString + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bHighPass)// \hpf  HighPass Filter
	{
		szString = pDoc->GetSaParm()->wFlags & SA_FLAG_HIGHPASS ? "Yes":"No";
		szString = "\\hpf " +  szString + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bBits)// \bits Storage Format
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%d Bits"),pDoc->GetFmtParm()->wBitsPerSample);
		szString.ReleaseBuffer();
		szString = "\\bits " +  szString + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bQuantization)// \size Quantization Size
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%d Bits"),(int)pDoc->GetSaParm()->byRecordSmpSize);
		szString.ReleaseBuffer();
		szString = "\\qsize " +  szString + szCrLf;
		WriteFileUtf8(pFile, szString);
	}

	if (m_bLanguage)// \ln   Language Name
	{
		szString = "\\ln " + pDoc->GetSourceParm()->szLanguage + szCrLf;
		WriteFileUtf8(pFile, szString);
	}

	if (m_bDialect)// \dlct Dialect
	{
		szString = "\\dlct " + pDoc->GetSourceParm()->szDialect + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bFamily)// \fam  Family
	{
		szString = "\\fam " + pDoc->GetSourceParm()->szFamily + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bEthnologue)// \id   Ethnologue ID number
	{
		szString = "\\id " + pDoc->GetSourceParm()->szEthnoID + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bCountry)// \cnt  Country
	{
		szString = "\\cnt " + pDoc->GetSourceParm()->szCountry + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bRegion)// \reg  Region
	{
		szString = "\\reg " + pDoc->GetSourceParm()->szRegion + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bSpeaker)// \spkr Speaker Name
	{
		szString = "\\spkr " + pDoc->GetSourceParm()->szSpeaker + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bGender)// \gen  Gender
	{
		switch(pDoc->GetSourceParm()->nGender)
		{
		case 0:  szString = "Adult Male";   break;
		case 1:  szString = "Adult Female"; break;
		case 2:  szString = "Child";        break;
		default: szString = "";             break;
		}
		szString = "\\gen " + szString + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bNotebookRef)// \nbr  Notebook Reference
	{
		szString = "\\nbr " + pDoc->GetSourceParm()->szReference + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bTranscriber)// \tr   Transcriber
	{
		szString = "\\tr " + pDoc->GetSourceParm()->szTranscriber + szCrLf;
		WriteFileUtf8(pFile, szString);
	}
	if (m_bComments)// \desc Description
	{
		szString = "\\desc " + pDoc->GetSaParm()->szDescription + szCrLf;
		WriteFileUtf8(pFile, szString);
	}

	if (pFile) delete pFile;
	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CExportTable dialog


CExportTable::CExportTable(const CSaString& szDocTitle, CWnd* pParent /*=NULL*/)
: CDialog(CExportTable::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportTable)
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
	//}}AFX_DATA_INIT
	m_szDocTitle = szDocTitle;

	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	if(pDoc->GetSegment(PHONETIC)->IsEmpty()) // no annotations
	{
		m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = FALSE;
		m_bSegmentStart = m_bSegmentLength = FALSE; // no segments
		m_nSampleRate = 1;
	}
}

void CExportTable::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportTable)
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
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExportTable, CDialog)
	//{{AFX_MSG_MAP(CExportTable)
	ON_BN_CLICKED(IDC_EXTAB_ANNOTATIONS, OnAllAnnotations)
	ON_BN_CLICKED(IDC_EXTAB_FORMANTS, OnAllFormants)
	ON_BN_CLICKED(IDC_EXTAB_CALC, OnAllCalculations)
	ON_EN_UPDATE(IDC_EXTAB_INTERVAL_TIME, OnUpdateIntervalTime)
	ON_BN_CLICKED(IDC_EXTAB_INTERVAL, OnSample)
	ON_BN_CLICKED(IDC_EXTAB_SEGMENT, OnSample)
	ON_BN_CLICKED(IDC_EXTAB_PHONETIC2, OnPhonetic)
	ON_BN_CLICKED(IDC_EXTAB_PHONETIC, OnPhonetic)
	ON_COMMAND(IDHELP, OnHelpExportTable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CExportTable message handlers

void CExportTable::OnAllAnnotations()
{
	UpdateData(TRUE);
	if(m_nSampleRate == 0)
	{
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
	}
	else
	{
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
	if (m_bAnnotations)
	{
		m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = TRUE;

		UpdateData(FALSE);
	}
}

void CExportTable::OnAllFormants()
{
	UpdateData(TRUE);
	BOOL bEnable = !m_bFormants;
	SetEnable(IDC_EXTAB_F1, bEnable && !m_bFormants);
	SetEnable(IDC_EXTAB_F2, bEnable && !m_bFormants);
	SetEnable(IDC_EXTAB_F3, bEnable && !m_bFormants);
	SetEnable(IDC_EXTAB_F4, bEnable && !m_bFormants);
	if (m_bFormants)
	{
		m_bF1 = m_bF2 = m_bF3 = m_bF4 = TRUE;

		UpdateData(FALSE);
	}
}

void CExportTable::OnAllCalculations()
{
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
	if (m_bCalculations)
	{
		m_bMagnitude = m_bZeroCrossings = m_bPitch = m_bMelogram  = m_bFormants = TRUE;
		UpdateData(FALSE);

		OnAllFormants();
	}
}

void CExportTable::OnOK()
{
	if ((m_szFileName = GetExportFilename(m_szDocTitle, _T("SFM Time Table (*.sft) |*.sft||"),_T("sft"))) == "") return;

	UpdateData(TRUE);
	// process all flags
	if (m_bAnnotations)  m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = TRUE;
	if (m_bCalculations)
		m_bMagnitude = m_bZeroCrossings = m_bPitch = m_bMelogram = m_bFormants = TRUE;
	if (m_bFormants)     
		m_bF1 = m_bF2 = m_bF3 = m_bF4 = TRUE; 
	if (m_nSampleRate == 0)
	{
		m_bSampleTime = FALSE;
	}
	else
	{
		m_bReference = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = FALSE;
		m_bSegmentStart = m_bSegmentLength = FALSE;
	}

	CSaString szString;
	CSaString szFTFormantString = "";
	CSaString szCrLf = "\r\n";
	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos); // get pointer to view
	CSegment* pPhonetic = pDoc->GetSegment(PHONETIC);

	if (pPhonetic->IsEmpty()) // no annotations
	{
		m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = FALSE;
		m_bSegmentStart = m_bSegmentLength = FALSE; // no segments
		if (m_nSampleRate == 0)
			m_nSampleRate = 1;
	}

	DWORD dwOffset = pView->GetStartCursorPosition();
	DWORD dwStopPosition = pView->GetStopCursorPosition();
	if(m_nRegion != 0) // entire file
	{
		dwOffset = 0;
		dwStopPosition = pDoc->GetDataSize() - pDoc->GetFmtParm()->wBlockAlign;
	}

	int nIndex = 0;

	if (m_nSampleRate != 1) // phonetic sampling
	{
		nIndex = pPhonetic->FindFromPosition(dwStopPosition);

		if ((nIndex != -1) && dwStopPosition < pPhonetic->GetOffset(nIndex))
			nIndex = pPhonetic->GetPrevious(nIndex);

		if ((nIndex != -1)  && dwStopPosition < pPhonetic->GetStop(nIndex))
			dwStopPosition = pPhonetic->GetStop(nIndex);
	}

	nIndex = pPhonetic->FindFromPosition(dwOffset);

	if (m_nSampleRate != 1) // phonetic sampling
	{
		if (nIndex != -1)
			dwOffset = pPhonetic->GetOffset(nIndex);
	}

	DWORD dwNext = 0;
	DWORD dwIncrement = 0;

	if (m_nSampleRate == 1) // interval sampling
	{
		int nInterval = 20;
		if(m_szIntervalTime.GetLength() != 0)
		{
			swscanf_s(m_szIntervalTime, _T("%d"), &nInterval);
		}
		if (nInterval < 1) nInterval = 20;
		dwIncrement = pDoc->GetBytesFromTime(nInterval/1000.0);
		if (dwIncrement < 1) dwIncrement++;
		if (pDoc->GetFmtParm()->wBlockAlign == 2)
		{
			dwIncrement++;
			dwIncrement &= ~1;
		}
		swprintf_s(m_szIntervalTime.GetBuffer(20),20, _T("%d"), nInterval);
		m_szIntervalTime.ReleaseBuffer();
	}

	short int nResult;
	enum {MAG, PITCH, MELOGRAM, ZCROSS, FMTTRACKER, CALCULATIONS};
	double fSizeFactor[CALCULATIONS];

	if (m_bMagnitude)
	{
		CProcessLoudness* pLoudness = (CProcessLoudness*)pDoc->GetLoudness(); // get pointer to loudness object
		nResult = LOWORD(pLoudness->Process(this, pDoc)); // process data
		if (nResult == PROCESS_ERROR) m_bMagnitude = FALSE;
		else if (nResult == PROCESS_CANCELED) return;
		else fSizeFactor[MAG] = (double)pDoc->GetDataSize() / (double)(pLoudness->GetDataSize() - 1);
	}
	if (m_bPitch || m_bF1 || m_bF2 || m_bF3 || m_bF4) // formants need pitch info
	{
		CProcessGrappl* pPitch = pDoc->GetGrappl(); // SDM 1.5 Test 11.0
		// We also want raw and smoothed
		CProcessPitch* pRawPitch = pDoc->GetPitch();
		CProcessSmoothedPitch* pSmoothedPitch = pDoc->GetSmoothedPitch();
		nResult = LOWORD(pPitch->Process(this, pDoc)); // process data
		if (nResult == PROCESS_ERROR) m_bPitch = FALSE;
		else if (nResult == PROCESS_CANCELED) return;
		nResult = LOWORD(pRawPitch->Process(this, pDoc));
		if (nResult == PROCESS_ERROR) m_bPitch = m_bPitch && FALSE;
		else if (nResult == PROCESS_CANCELED) return;
		nResult = LOWORD(pSmoothedPitch->Process(this, pDoc));
		if (nResult == PROCESS_ERROR) m_bPitch = m_bPitch && FALSE;
		else if (nResult == PROCESS_CANCELED) return;
		else fSizeFactor[PITCH] = (double)pDoc->GetDataSize() / (double)(pPitch->GetDataSize() - 1);
	}
	if (m_bMelogram)
	{
		CProcessMelogram* pMelogram = pDoc->GetMelogram(); // SDM 1.5 Test 11.0
		nResult = LOWORD(pMelogram->Process(this, pDoc)); // process data
		if (nResult == PROCESS_ERROR) m_bMelogram = FALSE;
		else if (nResult == PROCESS_CANCELED) return;
		else fSizeFactor[MELOGRAM] = (double)pDoc->GetDataSize() / (double)(pMelogram->GetDataSize() - 1);
	}
	if (m_bZeroCrossings || m_bF1 || m_bF2 || m_bF3 || m_bF4)
	{
		CProcessZCross*   pZCross = pDoc->GetZCross();
		nResult = LOWORD(pZCross->Process(this, pDoc)); // process data
		if (nResult == PROCESS_ERROR) m_bZeroCrossings = FALSE;
		else if (nResult == PROCESS_CANCELED) return;
		else fSizeFactor[ZCROSS] = (double)pDoc->GetDataSize() / (double)(pZCross->GetDataSize() - 1);
	}
	if (m_bF1 || m_bF2 || m_bF3 || m_bF4)
	{
		CFormantTracker* pSpectroFormants = pDoc->GetFormantTracker();

		// If pitch processed successfully, generate formant data.
		if (pDoc->GetGrappl()->IsDataReady())
		{
			short int nResult = LOWORD(pSpectroFormants->Process(pView, pDoc));
			if (nResult == PROCESS_ERROR) m_bF1 = m_bF2 = m_bF3 = m_bF4 = FALSE;
			else if (nResult == PROCESS_CANCELED) return;
			else fSizeFactor[FMTTRACKER] = (double)pDoc->GetDataSize() / (double)(pSpectroFormants->GetDataSize() - 1);
		}
		else
		{
			m_bF1 = m_bF2 = m_bF3 = m_bF4 = FALSE;
		}
	}

	CFile* pFile = new CFile(m_szFileName, CFile::modeCreate|CFile::modeWrite);

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
	if(m_nCalculationMethod == 0)
		szString = "\\calc "  "midpoint" + szCrLf;
	else
		szString = "\\calc "  "average" + szCrLf;
	WriteFileUtf8(pFile, szString);

	if (m_bSampleTime) szString = "\\table time " "every "+ m_szIntervalTime + "ms" "\r\nTime\t";
	else szString = "\\table Etic\r\n";
	if (m_bSegmentStart) szString += "Start\t";
	if (m_bSegmentLength) szString += "Length\t";
	if (m_bReference) szString += "Ref\t";
	if (m_bPhonetic) szString += "Etic\t";
	if (m_bTone) szString += "Tone\t";
	if (m_bPhonemic) szString += "Emic\t";
	if (m_bOrtho) szString += "Ortho\t";;
	if (m_bGloss) szString += "Gloss\t";
	if (m_bPOS) szString += "POS\t";

	if (m_bMagnitude) szString += "Int(dB)\t";
	if (m_bPitch) szString += "Pitch(Hz)\tRawPitch\tSmPitch\t";
	if (m_bMelogram) szString += "Melogram(st)\t";
	if (m_bZeroCrossings) szString += "ZCross\t";
	if (m_bF1) szString += "F1(Hz)\t";
	if (m_bF2) szString += "F2(Hz)\t";
	if (m_bF3) szString += "F3(Hz)\t";
	if (m_bF4) szString += "F4(Hz)\t";
	szString += "\r\n";
	WriteFileUtf8(pFile, szString);

	// construct table entries
	while(dwOffset < dwStopPosition)
	{
		if (m_nSampleRate == 1) // interval sampling
		{
			dwNext = dwOffset+dwIncrement;
		}
		else // phonetic segment samples
		{
			if (nIndex != -1)
				dwNext = pPhonetic->GetStop(nIndex);
			else
				dwNext = pDoc->GetUnprocessedDataSize();
		}

		if (m_bSampleTime)
		{
			swprintf_s(szString.GetBuffer(25),25,_T("%.3f\t"),pDoc->GetTimeFromBytes(dwOffset));
			szString.ReleaseBuffer();
			WriteFileUtf8(pFile, szString);
		}

		if ((m_nSampleRate==0) && (nIndex != -1) && (pPhonetic->GetOffset(nIndex) < dwNext))
		{
			DWORD dwPhonetic = pPhonetic->GetOffset(nIndex);
			if (m_bSegmentStart)
			{
				swprintf_s(szString.GetBuffer(25),25,_T("%.3f\t"),pDoc->GetTimeFromBytes(dwPhonetic));
				szString.ReleaseBuffer();
				WriteFileUtf8(pFile, szString);
			}
			if (m_bSegmentLength)
			{
				swprintf_s(szString.GetBuffer(25),25,_T("%.3f\t"),pDoc->GetTimeFromBytes(pPhonetic->GetDuration(nIndex)));
				szString.ReleaseBuffer();
				WriteFileUtf8(pFile, szString);
			}

			if (m_bReference)
			{
				int nIndex = pDoc->GetSegment(REFERENCE)->FindOffset(dwPhonetic);
				if (nIndex != -1)
					szString = pDoc->GetSegment(REFERENCE)->GetSegmentString(nIndex) + "\t";
				else
					szString = "\t";
				WriteFileUtf8(pFile, szString);
			}
			if (m_bPhonetic)
			{
				szString = pPhonetic->GetSegmentString(nIndex) + "\t";
				WriteFileUtf8(pFile, szString);
			}
			if (m_bTone)
			{
				int nIndex = pDoc->GetSegment(TONE)->FindOffset(dwPhonetic);
				if (nIndex != -1)
					szString = pDoc->GetSegment(TONE)->GetSegmentString(nIndex) + "\t";
				else
					szString = "\t";
				WriteFileUtf8(pFile, szString);
			}
			if(m_bPhonemic)
			{
				int nIndex = pDoc->GetSegment(PHONEMIC)->FindOffset(dwPhonetic);
				if(nIndex != -1)
					szString = pDoc->GetSegment(PHONEMIC)->GetSegmentString(nIndex) + "\t";
				else
					szString = "\t";
				WriteFileUtf8(pFile, szString);
			}
			if(m_bOrtho)
			{
				int nIndex = pDoc->GetSegment(ORTHO)->FindOffset(dwPhonetic);
				if(nIndex != -1)
					szString = pDoc->GetSegment(ORTHO)->GetSegmentString(nIndex) + "\t";
				else
					szString = "\t";
				WriteFileUtf8(pFile, szString);
			}
			if(m_bGloss)
			{
				int nIndex = pDoc->GetSegment(GLOSS)->FindOffset(dwPhonetic);
				if(nIndex != -1)
				{
					// SDM 1.5Test10.1
					szString = pDoc->GetSegment(GLOSS)->GetSegmentString(nIndex);
					if((szString.GetLength() > 1)&&(szString[0] == WORD_DELIMITER))
						szString = szString.Mid(1); // Remove Word Delimiter
					szString += "\t";
				}
				else
					szString = "\t";
				WriteFileUtf8(pFile, szString);
				if(m_bPOS)
				{
					if(nIndex != -1)
						szString = ((CGlossSegment*) pDoc->GetSegment(GLOSS))->GetPOSs()->GetAt(nIndex) + "\t";
					else
						szString = "\t";
					WriteFileUtf8(pFile, szString);
				}
			}
			nIndex = pPhonetic->GetNext(nIndex);
		}
		else if(m_bPhonetic && (m_nSampleRate==1) && !pPhonetic->IsEmpty())
		{
			nIndex = 0;

			while((nIndex != -1) && (pPhonetic->GetStop(nIndex) < dwOffset))
				nIndex = pPhonetic->GetNext(nIndex);

			if((nIndex != -1) && pPhonetic->GetOffset(nIndex) < dwNext) // this one overlaps
			{
				int nLast = pPhonetic->GetNext(nIndex);
				szString = pPhonetic->GetSegmentString(nIndex);

				while((nLast != -1) && (pPhonetic->GetOffset(nLast) < dwNext))
				{
					szString += " " + pPhonetic->GetSegmentString(nLast);
					nLast = pPhonetic->GetNext(nLast);
				}
				szString +="\t";
			}
			else
				szString = "\t";
			WriteFileUtf8(pFile, szString);
		}

		DWORD dwBegin;
		DWORD dwEnd;
		DWORD dwCalcIncrement;
		DWORD dwIndex;
		if(m_nCalculationMethod == 0)
		{
			dwBegin = dwEnd = (dwOffset + dwNext)/2;
			dwEnd++;
			dwCalcIncrement = 10;
		}
		else
		{
			dwBegin = dwOffset;
			dwEnd = dwNext;
			dwCalcIncrement = (dwEnd - dwBegin)/20;
			if(!dwCalcIncrement) dwCalcIncrement = 1;
		}

		if(m_bMagnitude)
		{
			int dwSamples = 0;
			BOOL bRes = TRUE;
			double fData = 0;
			for(dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement)
			{
				DWORD dwProcData = (DWORD) (dwIndex/fSizeFactor[MAG]);
				// get data for this pixel
				fData += pDoc->GetLoudness()->GetProcessedData(dwProcData, &bRes);
				dwSamples++;
			}
			if(dwSamples && bRes)
			{
				fData = fData/ dwSamples;
				double fLoudnessMax = pDoc->GetLoudness()->GetMaxValue();
				if(fData*10000. < fLoudnessMax)
					fData = fLoudnessMax/10000.;

				double db = 20.0 * log10(fData/32767.) + 9.;  // loudness is rms full scale would be 9dB over recommended recording level
				swprintf_s(szString.GetBuffer(25),25,_T("%0.1f\t"),db);
				szString.ReleaseBuffer();
			}
			else
				szString = "\t";
			WriteFileUtf8(pFile, szString);
		}
		if(m_bPitch)
		{
			int dwSamples = 0;
			BOOL bRes = TRUE;
			long nData = 0;
			for(dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement)
			{
				DWORD dwProcData = (DWORD) (dwIndex/fSizeFactor[PITCH]);
				// get data for this pixel
				int nHere = pDoc->GetGrappl()->GetProcessedData(dwProcData, &bRes); // SDM 1.5Test11.0
				if (nHere > 0)
				{
					nData += nHere;
					dwSamples++;
				}
			}
			if(dwSamples && bRes)
			{
				double fData = double(nData) / PRECISION_MULTIPLIER/ dwSamples;
				swprintf_s(szString.GetBuffer(25),25,_T("%.1f\t"),fData);
				szString.ReleaseBuffer();
			}
			else
				szString = "\t";
			WriteFileUtf8(pFile, szString);

			// Raw Pitch
			dwSamples = 0;
			bRes = TRUE;
			nData = 0;
			for(dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement)
			{
				DWORD dwProcData = (DWORD) (dwIndex/fSizeFactor[PITCH]);
				// get data for this pixel
				int nHere = pDoc->GetPitch()->GetProcessedData(dwProcData, &bRes);
				if (nHere > 0)
				{
					nData += nHere;
					dwSamples++;
				}
			}
			if(dwSamples && bRes)
			{
				double fData = double(nData) / PRECISION_MULTIPLIER/ dwSamples;
				swprintf_s(szString.GetBuffer(25),25,_T("%.1f\t"),fData);
				szString.ReleaseBuffer();
			}
			else
				szString = "\t";
			WriteFileUtf8(pFile, szString);

			// Smoothed Pitch
			dwSamples = 0;
			bRes = TRUE;
			nData = 0;
			for(dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement)
			{
				DWORD dwProcData = (DWORD) (dwIndex/fSizeFactor[PITCH]);
				// get data for this pixel
				int nHere = pDoc->GetSmoothedPitch()->GetProcessedData(dwProcData, &bRes);
				if (nHere > 0)
				{
					nData += nHere;
					dwSamples++;
				}
			}
			if(dwSamples && bRes)
			{
				double fData = double(nData) / PRECISION_MULTIPLIER/ dwSamples;
				swprintf_s(szString.GetBuffer(25),25,_T("%.1f\t"),fData);
				szString.ReleaseBuffer();
			}
			else
				szString = "\t";
			WriteFileUtf8(pFile, szString);
		}
		if(m_bMelogram)
		{
			int dwSamples = 0;                
			BOOL bRes = TRUE;
			long nData = 0;
			for(dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement)
			{
				DWORD dwProcData = (DWORD) (dwIndex/fSizeFactor[MELOGRAM]);
				// get data for this pixel
				int nHere = pDoc->GetMelogram()->GetProcessedData(dwProcData, &bRes); // SDM 1.5Test11.0
				if (nHere > 0)
				{   
					nData += nHere;
					dwSamples++;
				}
			}
			if(dwSamples && bRes)
			{
				double fData = double(nData) / 100.0 / dwSamples;
				swprintf_s(szString.GetBuffer(25),25,_T("%.2f\t"),fData);
				szString.ReleaseBuffer();
			}
			else
				szString = "\t";
			WriteFileUtf8(pFile, szString);
		}
		if(m_bZeroCrossings)
		{
			int dwSamples = 0;
			BOOL bRes = TRUE;
			long nData = 0;
			for(dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement)
			{
				DWORD dwProcData = (DWORD) (dwIndex/fSizeFactor[ZCROSS]);
				// get data for this pixel
				nData += pDoc->GetZCross()->GetProcessedData(dwProcData, &bRes);
				dwSamples++;
			}
			if(dwSamples && bRes)
			{
				nData = nData/ dwSamples;
				swprintf_s(szString.GetBuffer(25),25,_T("%d\t"),(int)nData);
				szString.ReleaseBuffer();
			}
			else
				szString = "\t";
			WriteFileUtf8(pFile, szString);
		}

		if(m_bF1 || m_bF2 || m_bF3 || m_bF4)
		{
			int dwSamples[5] = {0,0,0,0,0};
			double pFormFreq[5] = {0,0,0,0,0};
			int bFormOn[5] = {0, m_bF1, m_bF2, m_bF3, m_bF4};

			// get FormantTracker data
			for(dwIndex = dwBegin; dwIndex < dwEnd; dwIndex++)
			{
				DWORD dwProcData = (DWORD) ((DWORD)(dwIndex/fSizeFactor[FMTTRACKER]/sizeof(FORMANT_FREQ)))*sizeof(FORMANT_FREQ);
				FORMANT_FREQ* pFormFreqCurr = (FORMANT_FREQ*)pDoc->GetFormantTracker()->GetProcessedData(dwProcData, sizeof(FORMANT_FREQ));
				for(int n = 1; n<5; n++)
				{
					if(pFormFreqCurr->F[n] == (float)NA)
					{
						continue;
					}
					pFormFreq[n] += (double)pFormFreqCurr->F[n];
					dwSamples[n]++;
				}
			}
			for(int n = 1; n<5; n++)
			{
				pFormFreq[n] /= dwSamples[n];
				if (bFormOn[n])
				{
					if(dwSamples[n])
					{
						swprintf_s(szString.GetBuffer(25),25,_T("%.1f\t"),(double) pFormFreq[n]);
						szString.ReleaseBuffer();
					}
					else
						szString = "\t";
					WriteFileUtf8(pFile, szString);
				}
			}
		}

		szString = "\r\n"; // next line of table
		WriteFileUtf8(pFile, szString);

		if (m_nSampleRate == 1) // interval sampling
		{
			dwOffset = dwNext < dwStopPosition ? dwNext : dwStopPosition;
		}
		else // phonetic segment samples
		{
			if (nIndex >= 0)
				dwOffset = pPhonetic->GetOffset(nIndex);
			else
				dwOffset = dwStopPosition;
		}
	}
	if(pFile) delete pFile;

	CDialog::OnOK();
}

/***************************************************************************/
// CExportTable::OnHelpExportTable Call Export Table help
/***************************************************************************/
void CExportTable::OnHelpExportTable()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath = szPath + "::/User_Interface/Menus/File/Export/SFM_time_table.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

BOOL CExportTable::OnInitDialog()
{
	CDialog::OnInitDialog();

	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	if(pDoc->GetSegment(PHONETIC)->IsEmpty()) // no annotations
	{
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
// CExportTable::SetEnable Enables/Disables controls
/***************************************************************************/
void CExportTable::SetEnable(int nItem, BOOL bEnable)
{
	CWnd* pWnd = GetDlgItem(nItem);

	if(pWnd)
	{
		pWnd->EnableWindow(bEnable);
	}
}

/***************************************************************************/
// CExportTable::SetVisible Sets the visibility of dialog items
/***************************************************************************/
void CExportTable::SetVisible(int nItem, BOOL bVisible, BOOL bEnable /*=TRUE*/)
{
	CWnd* pWnd = GetDlgItem(nItem);

	if(pWnd)
	{
		pWnd->EnableWindow(bVisible && bEnable);// disable invisible items, enable on show
		if(bVisible)
		{
			pWnd->SetWindowPos(NULL, 0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOZORDER+SWP_SHOWWINDOW);
		}
		else
		{
			pWnd->SetWindowPos(NULL, 0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOZORDER+SWP_HIDEWINDOW);
		}
	}
}


/***************************************************************************/
// CExportTable::OnUpdateIntervalTime Filter edit box to only accept digits
/***************************************************************************/
void CExportTable::OnUpdateIntervalTime()
{
	CWnd* pWnd = GetDlgItem(IDC_EXTAB_INTERVAL_TIME);

	if(pWnd)
	{
		CSaString szText;
		BOOL bChanged = FALSE;

		pWnd->GetWindowText(szText);

		for(int nIndex=0; nIndex < szText.GetLength(); nIndex++)
		{
			if((szText[nIndex] < '0') || (szText[nIndex] > '9'))
			{
				szText = szText.Left(nIndex) + szText.Mid(nIndex+1);
				bChanged = TRUE;
			}
			if(bChanged) // only change if necessary (will cause infinite loop)
				pWnd->SetWindowText(szText);
		}
	}
}


/***************************************************************************/
// CExportTable::OnSample disable Phonetic Sampling on empty phonetic segment
// and update visibility of check boxes
/***************************************************************************/
void CExportTable::OnSample()
{
	UpdateData(TRUE);
	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	if(pDoc->GetSegment(PHONETIC)->IsEmpty()) // no annotations
	{
		if(m_nSampleRate == 0)
		{
			AfxMessageBox(IDS_ERROR_NOSEGMENTS,MB_OK,0);
			m_nSampleRate = 1;
			UpdateData(FALSE);
		}
	}
	OnAllAnnotations();
}


/***************************************************************************/
// CExportTable::OnPhonetic keep invisible phonetic check box in sync with visible
/***************************************************************************/
void CExportTable::OnPhonetic()
{
	UpdateData(TRUE);
	if(m_nSampleRate == 0)
		m_bPhonetic2 = m_bPhonetic;
	else
		m_bPhonetic = m_bPhonetic2;
	UpdateData(FALSE);
}

static const char* psz_Phonetic = "ph";
static const char* psz_Phonemic = "pm";
static const char* psz_Orthographic = "or";
static const char* psz_Gloss = "gl";
static const char* psz_FreeTranslation = "ft"; // Free Translation
static const char* psz_Language ="ln"; // Language Name
static const char* psz_Dialect = "dlct"; // Dialect
static const char* psz_Family = "fam"; // Family
static const char* psz_Ethno = "id"; // Ethnologue ID number
static const char* psz_Country = "cnt"; // Country
static const char* psz_Region = "reg"; // Region
static const char* psz_Speaker = "spkr"; // Speaker Name
static const char* psz_Gender = "gen"; // Gender
static const char* psz_NotebookReference = "nbr"; // Notebook Reference
static const char* psz_Transcriber = "tr"; // Transcriber
static const char* psz_Comments = "cmnt"; // Comments
static const char* psz_Description = "desc"; // Description
static const char* psz_ImportEnd = "import";
static const char* psz_Table = "table";



/***************************************************************************/
// CImport::Import read import file
/***************************************************************************/
BOOL CImport::Import(int nMode)
{
	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	pDoc->CheckPoint();
	pDoc->SetModifiedFlag(TRUE);
	pDoc->SetTransModifiedFlag(TRUE); // transcription has been modified

	BOOL ret = TRUE;
	CSaString String;
	CSaString Imported("Imported...\r\n");
	CSaString Skipped("Skipped... \r\n");
	CSaString *pPhonetic = NULL;
	CSaString *pPhonemic = NULL;
	CSaString *pOrtho = NULL;
	CSaString *pGloss = NULL;
	BOOL bTable = FALSE;
	const CSaString CrLf("\r\n");
	try
	{
		Object_istream obs(m_szPath);
		if ( !obs.bAtBackslash() )
		{
			obs.SkipBOM();
			if ( !obs.bAtBackslash() )
				return FALSE;
		}
		while ( !obs.bAtEnd() )
		{
			streampos pos = obs.getIos().tellg();
			const char* pszMarkerRead;
			CSaString szStringRead;
			obs.peekMarkedString(&pszMarkerRead, &szStringRead);

			if ( obs.bReadString(psz_Table, &String) )
			{
				obs.getIos().seekg(pos);  // start before marker
				obs.getIos().clear();
				BOOL result = ReadTable(obs, nMode);
				if(result)
				{
					Imported += "\\" + CSaString(pszMarkerRead) + " (Entire Table)" + CrLf;
					bTable = TRUE;
				}
				else
					Skipped += "\\" + CSaString(pszMarkerRead) + " (Entire Table)" + CrLf;
				break;  // this must be last marker
			}
			else if ( obs.bReadString(psz_FreeTranslation, &String) )
				pDoc->GetSourceParm()->szFreeTranslation = String;
			else if ( obs.bReadString(psz_Language, &String) )
				pDoc->GetSourceParm()->szLanguage = String;
			else if ( obs.bReadString(psz_Dialect, &String) )
				pDoc->GetSourceParm()->szDialect = String;
			else if ( obs.bReadString(psz_Family, &String) )
				pDoc->GetSourceParm()->szFamily = String;
			else if ( obs.bReadString(psz_Ethno, &String) )
				pDoc->GetSourceParm()->szEthnoID = String;
			else if ( obs.bReadString(psz_Country, &String) )
				pDoc->GetSourceParm()->szCountry = String;
			else if ( obs.bReadString(psz_Region, &String) )
				pDoc->GetSourceParm()->szRegion = String;
			else if ( obs.bReadString(psz_Speaker, &String) )
				pDoc->GetSourceParm()->szSpeaker = String;
			else if ( obs.bReadString(psz_Gender, &String) )
			{
				int nGender = pDoc->GetSourceParm()->nGender;

				String.MakeUpper();

				if(String == "ADULT MALE") nGender = 0;         // male
				else if(String == "ADULT FEMALE") nGender = 1;  // female
				else if(String == "CHILD") nGender = 2;  // child

				pDoc->GetSourceParm()->nGender = nGender;
			}
			else if ( obs.bReadString(psz_NotebookReference, &String) )
				pDoc->GetSourceParm()->szReference = String;
			else if ( obs.bReadString(psz_Transcriber, &String) )
				pDoc->GetSourceParm()->szTranscriber = String;
			else if ( obs.bReadString(psz_Comments, &String) )
				pDoc->GetSaParm()->szDescription = String;
			else if ( obs.bReadString(psz_Description, &String) )
				pDoc->GetSaParm()->szDescription = String;
			else if ( obs.bReadString(psz_Phonetic, &String) )
			{
				if(!pPhonetic)
					pPhonetic = new CSaString;
				*pPhonetic = String;
				continue;
			}
			else if ( obs.bReadString(psz_Phonemic, &String) )
			{
				if(!pPhonemic)
					pPhonemic = new CSaString;
				*pPhonemic = String;
				continue;
			}
			else if ( obs.bReadString(psz_Orthographic, &String) )
			{
				if(!pOrtho)
					pOrtho = new CSaString;
				*pOrtho = String;
				continue;
			}
			else if ( obs.bReadString(psz_Gloss, &String) )
			{
				if(!pGloss)
					pGloss = new CSaString;
				*pGloss = " " + String;


				int nSpace;
				while((nSpace = pGloss->Find(_T(" "))) != -1)
				{
					pGloss->SetAt(nSpace,'#');
				}
				continue;
			}
			else if ( obs.bEnd(psz_ImportEnd) )
				break;
			else
			{
				Skipped += "\\" + CSaString(pszMarkerRead) + " " + szStringRead + CrLf;
				continue;
			}
			Imported += "\\" + CSaString(pszMarkerRead) + " " + szStringRead + CrLf;
		}
	}
	catch(...)
	{
	}

	if(pPhonetic || pPhonemic || pOrtho || pGloss)
	{
		if(!bTable)
		{
			CDlgAnnotation Align(NULL, CDlgAnnotation::ALIGN, pDoc);

			Align.AutoAlign(pPhonetic, pPhonemic, pOrtho, pGloss);
		}

		CSaString Report;

		if(pPhonetic)
		{
			Report += "\\" + CSaString(psz_Phonetic) + " " + *pPhonetic + CrLf;
			delete pPhonetic;
		}
		if(pPhonemic)
		{
			Report += "\\" + CSaString(psz_Phonemic) + " " + *pPhonemic + CrLf;
			delete pPhonemic;
		}
		if(pOrtho)
		{
			Report += "\\" + CSaString(psz_Orthographic) + " " + *pOrtho + CrLf;
			delete pOrtho;
		}
		if(pGloss)
		{
			Report += "\\" + CSaString(psz_Gloss) + " " + *pGloss + CrLf;
			delete pGloss;
		}

		if(bTable)
			Skipped += Report;
		else
			Imported += Report;
	}

	if(!m_bBatch)
		CResult result(Imported + CrLf + Skipped, AfxGetMainWnd());

	return ret;
}


/***************************************************************************/
// extractTabField local helper function to get field from tab delimited string
/***************************************************************************/
static const CSaString extractTabField(const CSaString& szLine, const int nField)
{
	int nCount = 0;
	int nLoop = 0;

	if (nField < 0) return ""; // SDM 1.5Test10.1

	while((nLoop < szLine.GetLength()) && (nCount < nField))
	{
		if(szLine[nLoop] == '\t')
			nCount++;
		nLoop++;
	}
	int nBegin = nLoop;
	while((nLoop < szLine.GetLength()) && (szLine[nLoop] != '\t'))
	{
		nLoop++;
	}
	return szLine.Mid(nBegin, nLoop-nBegin);
}

static void CreateWordSegments(const int nWord, int& nSegments)
{
	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	if(pDoc->GetSegment(GLOSS)->GetSize() > nWord)
	{
		DWORD dwStart;
		DWORD dwStop;
		int nPhonetic;
		CPhoneticSegment* pPhonetic = (CPhoneticSegment*) pDoc->GetSegment(PHONETIC);

		if(nWord == -1)
		{
			dwStart = 0;
			if(pDoc->GetSegment(GLOSS)->IsEmpty())
			{
				dwStop = pDoc->GetUnprocessedDataSize();
			}
			else
			{
				dwStop = pDoc->GetSegment(GLOSS)->GetOffset(0);
			}
			if(dwStart + pDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME) > dwStop) return;
			nPhonetic = 0;
		}
		else
		{
			ASSERT(nSegments);
			dwStart = pDoc->GetSegment(GLOSS)->GetOffset(nWord);
			dwStop = pDoc->GetSegment(GLOSS)->GetDuration(nWord) + dwStart;
			nPhonetic = pPhonetic->FindOffset(dwStart);
		}
		// Limit number of segments
		if(nSegments*pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME) > (dwStop -dwStart))
		{
			nSegments = (int)((dwStop -dwStart)/pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));
			if(!nSegments) nSegments = 1;
			if(nSegments*pDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME) > (dwStop -dwStart))
				return;
		}
		// remove excess segments
		int nCount = 0;
		int nIndex = nPhonetic;
		while ((nIndex != -1)&&(pPhonetic->GetOffset(nIndex) < dwStop))
		{
			if(nCount >= nSegments)
			{
				pPhonetic->SetSelection(nIndex);
				pPhonetic->Remove(pDoc, FALSE); // no checkpoint
				if(nIndex >= pPhonetic->GetSize()) break;
			}
			else
			{
				DWORD dwBegin = dwStart + nCount;
				pPhonetic->Adjust(pDoc, nIndex, dwBegin, 1);
				nIndex = pPhonetic->GetNext(nIndex);
				nCount++;
			}
		}
		if (nSegments == 0) return; // done
		// add segments
		while (nCount < nSegments)
		{
			if(nIndex == -1) nIndex = pPhonetic->GetSize();
			DWORD dwBegin = dwStart + nCount;
			CSaString szEmpty(SEGMENT_DEFAULT_CHAR);
			pPhonetic->Insert(nIndex, &szEmpty, FALSE, dwBegin, 1);
			nIndex = pPhonetic->GetNext(nIndex);
			nCount++;
		}
		// adjust segment spacing
		DWORD dwSize = (dwStop - dwStart)/nSegments;
		if(pDoc->GetFmtParm()->wBlockAlign==2)
		{
			dwSize &= ~1;
		};
		dwSize += pDoc->GetFmtParm()->wBlockAlign;
		if(nIndex == -1) nIndex = pPhonetic->GetSize();
		nIndex = pPhonetic->GetPrevious(nIndex);
		while((nIndex != -1)&&(pPhonetic->GetOffset(nIndex) >= dwStart))
		{
			nCount--;
			DWORD dwBegin = dwStart+nCount*dwSize;
			DWORD dwDuration = dwSize;
			if((dwBegin + dwDuration) > dwStop) dwDuration = dwStop - dwBegin;
			pPhonetic->Adjust(pDoc, nIndex, dwBegin, dwDuration);
			nIndex = pPhonetic->GetPrevious(nIndex);
		}
	}
	nSegments = 0;
}
/***************************************************************************/
// CImport::ReadTable read table from import file
// file position should be '\' of table marker
/***************************************************************************/
BOOL CImport::ReadTable(Object_istream &obs, int nMode)
{
	CSaView* pView = (CSaView*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView();

	CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();

	const int MAXLINE = 32000;
	char* pUtf8 = new char[MAXLINE];
	CSaString szLine;

	// eat table marker
	obs.getIos().getline(pUtf8,MAXLINE);
	szLine.setUtf8(pUtf8);
	if(szLine.GetLength() >= (MAXLINE - 1)) // error
		return FALSE;
	if(szLine.Find(CString(psz_Table)) ==-1) // error
		return FALSE;

	// read header
	obs.getIos().getline(pUtf8,MAXLINE);
	szLine.setUtf8(pUtf8);
	if(szLine.GetLength() >= (MAXLINE - 1)) // error
		return FALSE;

	// parse header
	int nAnnotField[ANNOT_WND_NUMBER+1];
	CSaString szField;

	for(int nLoop = 0;nLoop < ANNOT_WND_NUMBER+1; nLoop++) nAnnotField[nLoop] = -1;


	for(int nLoop = 0;nLoop < 20; nLoop++)
	{
		szField = extractTabField(szLine, nLoop);

		if(szField.Find(_T("Time")) != -1)
			return FALSE;  // table is not built on phonetic segment boundaries
		else if(szField.Find(_T("Ref")) != -1)
			nAnnotField[REFERENCE] = nLoop;
		else if(szField.Find(_T("Etic")) != -1)
			nAnnotField[PHONETIC] = nLoop;
		else if(szField.Find(_T("Tone")) != -1)
			nAnnotField[TONE] = nLoop;
		else if(szField.Find(_T("Emic")) != -1)
			nAnnotField[PHONEMIC] = nLoop;
		else if(szField.Find(_T("Ortho")) != -1)
			nAnnotField[ORTHO] = nLoop;
		else if(szField.Find(_T("Gloss")) != -1)
			nAnnotField[GLOSS] = nLoop;
		else if(szField.Find(_T("POS")) != -1)
			nAnnotField[ANNOT_WND_NUMBER] = nLoop;
	}
	// create new segmentation
	if(nMode == QUERY)
	{
		CImportDlg* pImport = new CImportDlg;
		if(pImport->DoModal() != IDOK)
		{
			// process canceled by user
			pDoc->Undo(FALSE);
			return FALSE;
		}

		nMode = pImport->m_nMode;
		if (pImport) delete pImport;
	}
	if((pDoc->GetSegment(GLOSS)->IsEmpty())&& (nMode!=KEEP))
	{
		// do equal segmentation (replaces auto parse)
		streampos pos = obs.getIos().tellg();  // save top of file position

		// find number of segments in each word
		int nSegmentCount = 0;
		int nWords = 0;
		int nSegmentToBeginWord[MAX_INT];
		while(obs.getIos().peek() != EOF)
		{
			// read line
			obs.getIos().getline(pUtf8,MAXLINE);
			szLine.setUtf8(pUtf8);
			if(szLine.GetLength() >= (MAXLINE - 1)) // error
				return FALSE;
			if(extractTabField(szLine, nAnnotField[GLOSS]).GetLength()) // gloss found
			{
				nSegmentToBeginWord[nWords] = nSegmentCount;
				nWords++;
			}
			nSegmentCount++;
		}
		obs.getIos().seekg(pos);  // return to top of table
		obs.getIos().clear();  // clear the EOF flag
		if (nSegmentCount == 0) nSegmentCount = 1;
		if (nWords == 0) nWords = 1;
		nSegmentToBeginWord[nWords] = nSegmentCount;

		// add, remove and adjust segments as needed
		CPhoneticSegment* pPhonetic = (CPhoneticSegment*) pDoc->GetSegment(PHONETIC);
		CSaString szEmpty(SEGMENT_DEFAULT_CHAR);
		CSaString szEmptyGloss = "";
		CGlossSegment* pGloss = (CGlossSegment*)pDoc->GetSegment(GLOSS);
		DWORD dwFileLength = pDoc->GetUnprocessedDataSize();
		int nIndex = pPhonetic->FindFromPosition(0);
		int nWordCurr = 0;
		DWORD dwStart = 0;
		DWORD dwDuration = 0;
		while(nWordCurr < nWords || nIndex != -1)
		{
			CSaString szTest = pPhonetic->GetText(nIndex);
			if (nWordCurr < nWords)
			{
				dwDuration = (DWORD)((float)dwFileLength * (float)(nSegmentToBeginWord[nWordCurr + 1] - nSegmentToBeginWord[nWordCurr]) / (float)nSegmentCount);
				if (nIndex != -1) // adjust existing segments
					pPhonetic->Adjust(pDoc, nWordCurr, dwStart, dwDuration);
				else // add segments
					pPhonetic->Insert(nWordCurr, &szEmpty, FALSE, dwStart, dwDuration);
				pGloss->Insert(nWordCurr, &szEmptyGloss, 0, dwStart, dwDuration);
				nIndex = nWordCurr;
				nWordCurr++;
				dwStart += dwDuration;
			}
			else // remove extra segments
			{
				pPhonetic->SetSelection(nIndex);
				pPhonetic->Remove(pDoc, FALSE);
				nIndex--;
			}
			nIndex = pPhonetic->GetNext(nIndex);
		}
	}

	if((nMode == MANUAL) && (nAnnotField[GLOSS] == -1))
		nMode = AUTO;

	if(nMode == MANUAL)
	{
		streampos pos = obs.getIos().tellg();  // save top of file position

		int nSegmentCount = 0;
		int nWordCount = -1;
		while(obs.getIos().peek() != EOF)
		{
			// read line
			obs.getIos().getline(pUtf8,MAXLINE);
			szLine.setUtf8(pUtf8);
			if(szLine.GetLength() >= (MAXLINE - 1)) // error
				return FALSE;
			if(extractTabField(szLine, nAnnotField[GLOSS]).GetLength()) // gloss found
			{
				CreateWordSegments(nWordCount, nSegmentCount);
				nWordCount++;
			};
			nSegmentCount++;
		}
		CreateWordSegments(nWordCount, nSegmentCount);
		obs.getIos().seekg(pos);  // return to top of table
		obs.getIos().clear();  // clear the EOF flag
	}
	else if (nMode == AUTO)
	{
		if (!pDoc->AdvancedSegment())
		{
			// process canceled by user
			pDoc->Undo(FALSE);
			return FALSE;
		}
	}

	// clear import fields
	CSegment* pPhonetic = pDoc->GetSegment(PHONETIC);
	CGlossSegment* pGloss = (CGlossSegment*) pDoc->GetSegment(GLOSS);
	CSaString szString = WORD_DELIMITER;
	if(nAnnotField[GLOSS] != -1)
	{
		for(int nIndex = 0; nIndex < pGloss->GetSize(); nIndex++)
		{
			if(pGloss->GetSelection() != nIndex) pGloss->SetSelection(nIndex);
			pGloss->ReplaceSelectedSegment(pDoc, szString);
		}
	}
	szString = SEGMENT_DEFAULT_CHAR;
	if(nAnnotField[PHONETIC] != -1)
	{
		for(int nIndex = 0; nIndex < pPhonetic->GetSize(); nIndex++)
		{
			pView->ASelection().SelectFromPosition(pView, PHONETIC, pPhonetic->GetOffset(nIndex), CASegmentSelection::FIND_EXACT);
			pView->ASelection().SetSelectedAnnotationString(pView, szString, TRUE, FALSE);
		}
	}
	for(int nIndex = PHONETIC+1; nIndex < ANNOT_WND_NUMBER; nIndex++)
	{
		if((nAnnotField[nIndex] != -1)&& (nIndex != GLOSS))
			pDoc->GetSegment(nIndex)->DeleteContents();
	}
	if(nAnnotField[ANNOT_WND_NUMBER/* POS*/] != -1)
	{
		for(int nIndex = 0; nIndex < pDoc->GetSegment(GLOSS)->GetSize(); nIndex++)
			((CGlossSegment*)pDoc->GetSegment(GLOSS))->GetPOSs()->SetAt(nIndex, "");
	}

	// insert fields into segments
	int nIndexPhonetic = 0;
	int nIndexGloss = -1;
	BOOL bAppendGloss;
	BOOL bAppendPhonetic = FALSE;
	if(pPhonetic->IsEmpty()) return FALSE; // no where to go
	while(obs.getIos().peek() != EOF)
	{
		// read line
		obs.getIos().getline(pUtf8,MAXLINE);
		szLine.setUtf8(pUtf8);
		if(szLine.GetLength() >= (MAXLINE - 1)) // error
			return FALSE;

		szString = extractTabField(szLine, nAnnotField[GLOSS]);
		if(szString.GetLength()) // gloss found
		{
			nIndexGloss++;
			if(nIndexGloss >= pGloss->GetSize())
			{
				nIndexGloss--;
				if(nIndexPhonetic == pPhonetic->GetPrevious(pPhonetic->GetSize()))
				{
					bAppendPhonetic = TRUE;
				}
				else
				{
					nIndexPhonetic = pPhonetic->GetPrevious(pPhonetic->GetSize());
					bAppendPhonetic = FALSE;
				}
				bAppendGloss = TRUE;
			}
			else
			{
				nIndexPhonetic = pPhonetic->FindOffset(pGloss->GetOffset(nIndexGloss));
				bAppendPhonetic = FALSE;
				bAppendGloss = FALSE;
			}
			if(bAppendGloss)
				szString = pGloss->GetSegmentString(nIndexGloss) + " " + szString;
			if(pGloss->GetSelection() != nIndexGloss) pGloss->SetSelection(nIndexGloss);
			if((szString[0] != WORD_DELIMITER) || (szString[0] != TEXT_DELIMITER))
				szString = WORD_DELIMITER + szString;
			pGloss->ReplaceSelectedSegment(pDoc, szString);

			// POS
			szString = extractTabField(szLine, nAnnotField[ANNOT_WND_NUMBER/*POS*/]);
			if(szString.GetLength())
			{
				if(bAppendGloss)
					szString = pGloss->GetPOSs()->GetAt(nIndexGloss) + " " + szString;
				pGloss->GetPOSs()->SetAt(nIndexGloss, szString);
			}
			// Reference
			szString = extractTabField(szLine, nAnnotField[REFERENCE]);
			if(szString.GetLength())
			{
				pView->ASelection().SelectFromPosition(pView, REFERENCE, pGloss->GetOffset(nIndexGloss), CASegmentSelection::FIND_EXACT);
				if(bAppendGloss)
					szString = pView->ASelection().GetSelectedAnnotationString(pView,FALSE) + " " + szString;
				pView->ASelection().SetSelectedAnnotationString(pView, szString, TRUE, FALSE);
			}
		}
		for(int nIndex = PHONETIC; nIndex < GLOSS; nIndex++)
		{
			szString = extractTabField(szLine, nAnnotField[nIndex]);
			if(szString.GetLength())
			{
				pView->ASelection().SelectFromPosition(pView, nIndex, pPhonetic->GetOffset(nIndexPhonetic), CASegmentSelection::FIND_EXACT);
				if(bAppendPhonetic)
					szString = pView->ASelection().GetSelectedAnnotationString(pView,FALSE) + /*" " +*/ szString; // SDM 1.5Test10.7 remove spaces
				pView->ASelection().SetSelectedAnnotationString(pView, szString, TRUE, FALSE);
			}
		}

		nIndexPhonetic = pPhonetic->GetNext(nIndexPhonetic);
		if(nIndexPhonetic == -1)
		{
			nIndexPhonetic = pPhonetic->GetPrevious(pPhonetic->GetSize());
			bAppendPhonetic = TRUE;
		}
		else if(((nIndexGloss + 1) < pGloss->GetSize()) && (pPhonetic->GetOffset(nIndexPhonetic) >= pGloss->GetOffset(nIndexGloss + 1)))
		{
			nIndexPhonetic = pPhonetic->GetPrevious(nIndexPhonetic);
			bAppendPhonetic = TRUE;
		}
		else
			bAppendPhonetic = FALSE;
	}
	// deselect everything // SDM 1.5Test10.1
	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
	{ pDoc->GetSegment(nLoop)->SetSelection(-1);
	}

	return TRUE;

}
/////////////////////////////////////////////////////////////////////////////
// CImportDlg dialog


CImportDlg::CImportDlg(CWnd* pParent /*=NULL*/)
: CDialog(CImportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportDlg)
	m_nMode = 0;
	//}}AFX_DATA_INIT
}

void CImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportDlg)
	DDX_Radio(pDX, IDC_KEEP, m_nMode);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImportDlg, CDialog)
	//{{AFX_MSG_MAP(CImportDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CImportDlg message handlers

void CImportDlg::OnOK()
{
	UpdateData(TRUE); // retrieve data

	CDialog::OnOK();
}

BOOL CImportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CenterWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}


/////////////////////////////////////////////////////////////////////////////
// CDlgAnnotationImport private dialog

class CDlgAnnotationImport : public CDialog
{
	// Construction
public:
	CDlgAnnotationImport(BOOL bPhonetic, BOOL bPhonemic, BOOL bOrtho, CWnd* pParent = NULL);   // standard constructor

	// Dialog Data
	//{{AFX_DATA(CDlgAnnotationImport)
	enum { IDD = IDD_ANNOTATION_WZD_IMPORT };
	BOOL    m_bGloss;
	BOOL    m_bPhonemic;
	BOOL    m_bPhonetic;
	CSaString m_szPhonemic;
	CSaString m_szGloss;
	CSaString m_szPhonetic;
	BOOL    m_bOrthographic;
	CSaString m_szOrthographic;
	//}}AFX_DATA

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void SetEnable(int nItem, BOOL bEnable);

	// Generated message map functions
	//{{AFX_MSG(CDlgAnnotationImport)
	afx_msg void OnImportPlainText();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDlgAnnotationImport dialog


CDlgAnnotationImport::CDlgAnnotationImport(BOOL bPhonetic, BOOL bPhonemic, BOOL bOrtho, CWnd* pParent /*=NULL*/)
: CDialog(CDlgAnnotationImport::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAnnotationImport)
	m_bGloss = FALSE;
	m_bPhonemic = bPhonemic;
	m_bPhonetic = bPhonetic;
	m_szPhonemic = psz_Phonemic;
	m_szGloss = psz_Gloss;
	m_szPhonetic = psz_Phonetic;
	m_bOrthographic = bOrtho;
	m_szOrthographic = psz_Orthographic;
	//}}AFX_DATA_INIT
}

void CDlgAnnotationImport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAnnotationImport)
	DDX_Check(pDX, IDC_GLOSS_ENABLED, m_bGloss);
	DDX_Check(pDX, IDC_PHONEMIC_ENABLED, m_bPhonemic);
	DDX_Check(pDX, IDC_PHONETIC_ENABLED, m_bPhonetic);
	DDX_Text(pDX, IDC_PHONEMIC, m_szPhonemic);
	DDX_Text(pDX, IDC_GLOSS, m_szGloss);
	DDX_Text(pDX, IDC_PHONETIC, m_szPhonetic);
	DDX_Check(pDX, IDC_ORTHO_ENABLED, m_bOrthographic);
	DDX_Text(pDX, IDC_ORTHOGRAPHIC, m_szOrthographic);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgAnnotationImport, CDialog)
	//{{AFX_MSG_MAP(CDlgAnnotationImport)
	ON_BN_CLICKED(IDC_IMPORT_PLAIN_TEXT, OnImportPlainText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/***************************************************************************/
// CDlgAnnotation::SetEnable Enables/Disables controls
/***************************************************************************/
void CDlgAnnotationImport::SetEnable(int nItem, BOOL bEnable)
{
	CWnd* pWnd = GetDlgItem(nItem);

	if (pWnd)
	{
		pWnd->EnableWindow(bEnable);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAnnotationImport message handlers

/***************************************************************************/
// CDlgAnnotation::OnImportPlainText Plain Text Button hit
/***************************************************************************/
void CDlgAnnotationImport::OnImportPlainText()
{
	EndDialog(IDC_IMPORT_PLAIN_TEXT);
}


/***************************************************************************/
// CDlgAnnotation::OnInitDialog
/***************************************************************************/
BOOL CDlgAnnotationImport::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetEnable(IDC_GLOSS, m_bGloss);
	SetEnable(IDC_PHONETIC, m_bPhonetic);
	SetEnable(IDC_PHONEMIC, m_bPhonemic);
	SetEnable(IDC_ORTHOGRAPHIC, m_bOrthographic);
	SetEnable(IDC_GLOSS_ENABLED, m_bGloss);
	SetEnable(IDC_PHONETIC_ENABLED, m_bPhonetic);
	SetEnable(IDC_PHONEMIC_ENABLED, m_bPhonemic);
	SetEnable(IDC_ORTHO_ENABLED, m_bOrthographic);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAnnotation dialog


CDlgAnnotation::CDlgAnnotation(CWnd* pParent, MODE mode, CSaDoc* pSaDoc)
: CDialog(CDlgAnnotation::IDD, pParent),  m_szPhonemicNullSegment("^"), m_szOrthographicNullSegment("_")
{
	// SDM 1.5Test8.2 changed default initialization
	//{{AFX_DATA_INIT(CDlgAnnotation)
	m_bOrthographic = FALSE;
	m_bPhonemic = FALSE;
	m_bPhonetic = TRUE;
	m_bGloss = FALSE;
	//}}AFX_DATA_INIT
	m_mode = mode;
	m_nState = 0;
	m_bPhoneticModified = FALSE;
	m_bPhonemicModified = FALSE;
	m_bOrthographicModified = FALSE;
	m_bGlossModified = FALSE; // SDM 1.5Test8.2
	// Font used for instructions and details
	m_pFont = new CFont;
	m_pFont->CreateFont( 8, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_SWISS+TMPF_TRUETYPE+DEFAULT_PITCH,_T("MS Sans Serif") );
	m_pSaDoc = pSaDoc;
	m_szPhonetic = BuildString(PHONETIC);
	m_szPhonemic = BuildString(PHONEMIC);
	m_szOrthographic = BuildString(ORTHO);
	m_szGloss = BuildString(GLOSS);
}

CDlgAnnotation::~CDlgAnnotation()
{
	delete m_pFont;
	CDialog::~CDialog();//Call Base class destructor
}

void CDlgAnnotation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAnnotation)
	DDX_Check(pDX, IDC_ORTHOGRAPHIC, m_bOrthographic);
	DDX_Check(pDX, IDC_PHONEMIC, m_bPhonemic);
	DDX_Check(pDX, IDC_PHONETIC, m_bPhonetic);
	DDX_Check(pDX, IDC_GLOSS, m_bGloss);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgAnnotation, CDialog)
	//{{AFX_MSG_MAP(CDlgAnnotation)
	ON_BN_CLICKED(IDC_BACK, OnBack)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_CHARACTER, OnRadio)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_REVERT, OnRevert)
	ON_EN_UPDATE(IDC_ANNOTATION, OnUpdateAnnotation)
	ON_BN_CLICKED(IDC_KEEP, OnRadio)
	ON_BN_CLICKED(IDC_MANUAL, OnRadio)
	ON_BN_CLICKED(IDC_NONE, OnRadio)
	ON_BN_CLICKED(IDC_WORD, OnRadio)
	ON_BN_CLICKED(IDC_AUTOMATIC, OnRadio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/***************************************************************************/
// CDlgAnnotation::SetVisible Sets the visibility of dialog items
/***************************************************************************/
void CDlgAnnotation::SetVisible(int nItem, BOOL bVisible)
{
	CWnd* pWnd = GetDlgItem(nItem);

	if (pWnd)
	{
		pWnd->EnableWindow(bVisible);// disable invisible items, enable on show
		if (bVisible)
		{
			pWnd->SetWindowPos(NULL, 0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOZORDER+SWP_SHOWWINDOW);
		}
		else
		{
			pWnd->SetWindowPos(NULL, 0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOZORDER+SWP_HIDEWINDOW);
		}
	}
}

/***************************************************************************/
// CDlgAnnotation::SetEnable Enables/Disables controls
/***************************************************************************/
void CDlgAnnotation::SetEnable(int nItem, BOOL bEnable)
{
	CWnd* pWnd = GetDlgItem(nItem);

	if (pWnd)
	{
		pWnd->EnableWindow(bEnable);
	}
}

/***************************************************************************/
// CDlgAnnotation::SetText Sets control text to string
/***************************************************************************/
void CDlgAnnotation::SetText(int nItem, CSaString szText)
{
	CWnd* pWnd = GetDlgItem(nItem);

	if (pWnd)
	{
		pWnd->SetWindowText(szText);
	}
}

/***************************************************************************/
// CDlgAnnotation::SetText Sets control text to string from IDS
/***************************************************************************/
void CDlgAnnotation::SetText(int nItem, UINT nIDS)
{
	CWnd* pWnd = GetDlgItem(nItem);
	CSaString szText;

	szText.LoadString(nIDS);

	if (pWnd)
	{
		pWnd->SetWindowText(szText);
	}
}

/***************************************************************************/
// CDlgAnnotation::BuildString builds an annotation string
/***************************************************************************/
const CSaString CDlgAnnotation::BuildString(int nSegment)
{
	CSaString szBuild = "";
	CSaString szWorking = "";
	CSaString szSegmentDefault = SEGMENT_DEFAULT_CHAR;

	if (m_pSaDoc == NULL) return szBuild;

	CSegment* pSegment = m_pSaDoc->GetSegment(nSegment);

	CSegment* pPhonetic = m_pSaDoc->GetSegment(PHONETIC);

	CSegment* pGloss = m_pSaDoc->GetSegment(GLOSS);

	if (pSegment == NULL) return szBuild;

	if (pSegment->IsEmpty()) return szBuild;

	int nIndex = 0;

	switch(nSegment)
	{
	case PHONETIC:
		while (nIndex != -1)
		{
			szWorking = pSegment->GetSegmentString(nIndex);
			if (szWorking != szSegmentDefault)
			{
				szBuild += pSegment->GetSegmentString(nIndex);
			}
			nIndex = pSegment->GetNext(nIndex);
			// SDM 1.06.8 Skip first word break
			if ((pGloss!=NULL)&&(nIndex != -1))
			{
				// Check For Gloss Word Break
				if (pGloss->FindOffset(pSegment->GetOffset(nIndex))!=-1) szBuild += m_pSaDoc->GetFont(nSegment)->m_wordDelimiter;
			}
		}
		break;
	case PHONEMIC:
		if (pPhonetic == NULL) break;
		while (nIndex != -1)
		{
			int nCrossIndex = pSegment->FindOffset(pPhonetic->GetOffset(nIndex));

			if (nCrossIndex != -1) // Segment exists
			{
				szWorking = pSegment->GetSegmentString(nCrossIndex);
				if (szWorking != szSegmentDefault) // Normal segment
				{
					szBuild += szWorking;
				}
			}
			nIndex = pPhonetic->GetNext(nIndex);
			// SDM 1.06.8 Skip first word break
			if ((pGloss!=NULL)&&(nIndex != -1))
			{
				// Check For Gloss Word Break
				if (pGloss->FindOffset(pPhonetic->GetOffset(nIndex))!=-1) szBuild += m_pSaDoc->GetFont(nSegment)->m_wordDelimiter;
			}
		}
		break;
	case ORTHO:
		if (pPhonetic == NULL) break;
		while (nIndex != -1)
		{
			int nCrossIndex = pSegment->FindOffset(pPhonetic->GetOffset(nIndex));

			if (nCrossIndex != -1) // Segment exists
			{
				szWorking = pSegment->GetSegmentString(nCrossIndex);
				if (szWorking != szSegmentDefault) // Normal Segment
				{
					szBuild += szWorking;
				}
			}
			nIndex = pPhonetic->GetNext(nIndex);
			// SDM 1.06.8 Skip first word break
			if ((pGloss!=NULL)&&(nIndex != -1))
			{
				// Check For Gloss Word Break
				if (pGloss->FindOffset(pPhonetic->GetOffset(nIndex))!=-1) szBuild += m_pSaDoc->GetFont(nSegment)->m_wordDelimiter;
			}
		}
		break;
	case GLOSS:
		while (nIndex != -1)
		{
			szWorking = pSegment->GetSegmentString(nIndex);
			szBuild += szWorking.Left(1) + " " + szWorking.Mid(1) + " ";// SDM 1.5Test8.2
			nIndex = pSegment->GetNext(nIndex);
		}
		break;
	default:
		;
	}
	return szBuild;
}


/***************************************************************************/
// CDlgAnnotation::SetState set state of wizard
/***************************************************************************/
void CDlgAnnotation::SetState(int nState)
{
	// Set controls to default
	SaveAnnotation();
	SetVisible(IDC_GROUP,FALSE);
	SetVisible(IDC_PHONETIC,FALSE);
	SetVisible(IDC_PHONEMIC,FALSE);
	SetVisible(IDC_ORTHOGRAPHIC,FALSE);
	SetVisible(IDC_GLOSS,FALSE);
	SetVisible(IDOK,FALSE);
	SetVisible(IDC_NEXT,TRUE);
	SetEnable(IDC_BACK,TRUE);
	SetEnable(IDC_NEXT,TRUE);
	SetDefID(IDC_NEXT);
	SetVisible(IDC_ANNOTATION,FALSE);
	SetVisible(IDC_KEEP,FALSE);
	SetVisible(IDC_AUTOMATIC,FALSE);
	SetVisible(IDC_MANUAL,FALSE);
	SetVisible(IDC_NONE,FALSE);
	SetVisible(IDC_WORD,FALSE);
	SetVisible(IDC_CHARACTER,FALSE);
	SetVisible(IDC_SFM_LABEL,FALSE);
	SetVisible(IDC_SFM_EDIT,FALSE);
	SetVisible(IDC_IMPORT,FALSE);
	SetVisible(IDC_REVERT,FALSE);
	SetVisible(IDC_DETAILS,FALSE);
	GotoDlgCtrl(GetDlgItem(IDC_NEXT)); // Set Default Focus to Next

	// set control for specific cases
	switch(nState)
	{
	case BACK:
		switch(m_nState)
		{
		case ALIGNBY:
			if (m_bGloss)
			{
				SetState(GLOSSTEXT);
				break;
			}
		case GLOSSTEXT:
			if (m_bOrthographic)
			{
				SetState(ORTHOGRAPHICTEXT);
				break;
			}
		case ORTHOGRAPHICTEXT:
			if (m_bPhonemic)
			{
				SetState(PHONEMICTEXT);
				break;
			}
		case PHONEMICTEXT:
			if (m_bPhonetic)
			{
				SetState(PHONETICTEXT);
				break;
			}
			else
			{
				SetState(INIT);
				break;
			}
		default:
			SetState(m_nState-1);
		}
		break;
	case NEXT:
		SetState(m_nState+1);
		break;
	case INIT:
		SetEnable(IDC_BACK,FALSE);
		SetVisible(IDC_GROUP,TRUE);
		SetVisible(IDC_PHONETIC,TRUE);
		SetVisible(IDC_PHONEMIC,TRUE);
		SetVisible(IDC_ORTHOGRAPHIC,TRUE);
		//SetVisible(IDC_GLOSS,TRUE);//SDM 1.5 Test10.0
		SetVisible(IDC_DETAILS,TRUE);
		SetText(IDC_TITLE, IDS_AWIZ_TITLE_INIT);
		SetText(IDC_INSTRUCTION, IDS_AWIZ_INST_INIT);
		SetText(IDC_GROUP, "Modify");
		SetText(IDC_DETAILS, IDS_AWIZ_DTL_INIT);
		m_nState = INIT;
		break;
	case PHONETICTEXT:
		if (m_bPhonetic)
		{
			SetVisible(IDC_ANNOTATION,TRUE);
			SetVisible(IDC_SFM_LABEL,TRUE);
			SetVisible(IDC_SFM_EDIT,TRUE);
			SetVisible(IDC_IMPORT,TRUE);
			SetVisible(IDC_REVERT,TRUE);
			SetEnable(IDC_REVERT,m_bPhoneticModified);
			SetText(IDC_SFM_EDIT, IDS_AWIZ_PH_MARKER);
			SetText(IDC_TITLE, IDS_AWIZ_TITLE_PHONETIC);
			SetText(IDC_INSTRUCTION, IDS_AWIZ_INST_PHONETIC);
			m_nState = PHONETICTEXT;
			break;
		}
	case PHONEMICTEXT:
		if (m_bPhonemic)
		{
			SetVisible(IDC_ANNOTATION,TRUE);
			SetVisible(IDC_SFM_LABEL,TRUE);
			SetVisible(IDC_SFM_EDIT,TRUE);
			SetVisible(IDC_IMPORT,TRUE);
			SetVisible(IDC_REVERT,TRUE);
			SetEnable(IDC_REVERT,m_bPhonemicModified);
			SetText(IDC_SFM_EDIT, IDS_AWIZ_PM_MARKER);
			SetText(IDC_TITLE, IDS_AWIZ_TITLE_PHONEMIC);
			SetText(IDC_INSTRUCTION, IDS_AWIZ_INST_PHONEMIC);
			m_nState = PHONEMICTEXT;
			break;
		}
	case ORTHOGRAPHICTEXT:
		if (m_bOrthographic)
		{
			SetVisible(IDC_ANNOTATION,TRUE);
			SetVisible(IDC_SFM_LABEL,TRUE);
			SetVisible(IDC_SFM_EDIT,TRUE);
			SetVisible(IDC_IMPORT,TRUE);
			SetVisible(IDC_REVERT,TRUE);
			SetEnable(IDC_REVERT,m_bOrthographicModified);
			SetText(IDC_SFM_EDIT, IDS_AWIZ_OR_MARKER);
			SetText(IDC_TITLE, IDS_AWIZ_TITLE_ORTHOGRAPHIC);
			SetText(IDC_INSTRUCTION, IDS_AWIZ_INST_ORTHOGRAPHIC);
			m_nState = ORTHOGRAPHICTEXT;
			break;
		}
	case GLOSSTEXT:
		if (m_bGloss)
		{
			SetVisible(IDC_ANNOTATION,TRUE);
			SetVisible(IDC_SFM_LABEL,TRUE);
			SetVisible(IDC_SFM_EDIT,TRUE);
			SetVisible(IDC_IMPORT,TRUE);
			SetVisible(IDC_REVERT,TRUE);
			SetEnable(IDC_REVERT,m_bGlossModified);
			SetText(IDC_SFM_EDIT, IDS_AWIZ_GL_MARKER);
			SetText(IDC_TITLE, IDS_AWIZ_TITLE_GLOSS);
			SetText(IDC_INSTRUCTION, IDS_AWIZ_INST_GLOSS);
			m_nState = GLOSSTEXT;
			break;
		}
	case ALIGNBY:
		SetVisible(IDC_GROUP,TRUE);
		SetVisible(IDC_NONE,TRUE);
		SetVisible(IDC_WORD,TRUE);
		SetVisible(IDC_CHARACTER,TRUE);
		SetVisible(IDC_DETAILS,TRUE);
		SetText(IDC_TITLE, IDS_AWIZ_TITLE_ALIGNBY);
		SetText(IDC_INSTRUCTION, IDS_AWIZ_INST_ALIGNBY);
		SetText(IDC_GROUP, "Alignment");
		m_nState = ALIGNBY;
		CheckRadioButton(IDC_NONE, IDC_CHARACTER, m_nAlignBy);
		OnRadio(); // Set Detail Text
		break;
	case SEGMENTBY:
		SetVisible(IDC_GROUP,TRUE);
		SetVisible(IDC_KEEP,TRUE);
		SetVisible(IDC_AUTOMATIC,TRUE);
		SetVisible(IDC_MANUAL,TRUE);
		SetVisible(IDC_DETAILS,TRUE);
		SetText(IDC_TITLE, IDS_AWIZ_TITLE_SEGMENTBY);
		SetText(IDC_INSTRUCTION, IDS_AWIZ_INST_SEGMENTBY);
		SetText(IDC_GROUP, "Segment");
		m_nState = SEGMENTBY;
		CheckRadioButton(IDC_KEEP, IDC_AUTOMATIC,m_nSegmentBy);
		OnRadio();// Set Detail Text
		break;
	case FINISH:
		SetVisible(IDOK,TRUE);
		SetVisible(IDC_NEXT,FALSE);
		SetEnable(IDC_NEXT,FALSE);
		SetDefID(IDOK);
		SetText(IDC_TITLE, IDS_AWIZ_TITLE_FINISH);
		SetText(IDC_INSTRUCTION, IDS_AWIZ_INST_FINISH);
		GotoDlgCtrl(GetDlgItem(IDOK));
		m_nState = FINISH;
		break;
	default:
		SetState(INIT);
	}
	SetAnnotation();
}


/***************************************************************************/
// CDlgAnnotation::SetAnnotation Sets annotation font and text
/***************************************************************************/
void CDlgAnnotation::SetAnnotation(void)
{
	CFont* pFont = NULL;

	switch(m_nState)
	{
	case PHONETICTEXT:
		SetText(IDC_ANNOTATION, m_szPhonetic);
		pFont = m_pSaDoc->GetFont(PHONETIC);
		break;
	case PHONEMICTEXT:
		SetText(IDC_ANNOTATION, m_szPhonemic);
		pFont = m_pSaDoc->GetFont(PHONEMIC);
		break;
	case ORTHOGRAPHICTEXT:
		SetText(IDC_ANNOTATION, m_szOrthographic);
		pFont = m_pSaDoc->GetFont(ORTHO);
		break;
	case GLOSSTEXT:
		SetText(IDC_ANNOTATION, m_szGloss);
		pFont = m_pSaDoc->GetFont(GLOSS);
		break;
	}

	CWnd* pWnd = GetDlgItem(IDC_ANNOTATION);
	if ((pWnd!=NULL) && (pFont!=NULL)) pWnd->SetFont(pFont);
}

/***************************************************************************/
// CDlgAnnotation::SaveAnnotation saves the annotation text
/***************************************************************************/
void CDlgAnnotation::SaveAnnotation(void)
{
	CWnd* pWnd = GetDlgItem(IDC_ANNOTATION);

	if (pWnd == NULL) return;

	switch(m_nState)
	{
	case PHONETICTEXT:
		pWnd->GetWindowText(m_szPhonetic);
		break;
	case PHONEMICTEXT:
		pWnd->GetWindowText(m_szPhonemic);
		break;
	case ORTHOGRAPHICTEXT:
		pWnd->GetWindowText(m_szOrthographic);
		break;
	case GLOSSTEXT:
		pWnd->GetWindowText(m_szGloss);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAnnotation message handlers


/***************************************************************************/
// CDlgAnnotation::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgAnnotation::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow(); // center dialog on frame window
	CSaString szCaption;
	switch(m_mode)
	{
	case ALIGN:
		szCaption.LoadString(IDS_AWIZ_CAPTION_ALIGN); // load caption string
		break;
	default:
		szCaption.LoadString(IDS_AWIZ_CAPTION_IMPORT); // load caption string
	}
	SetWindowText(szCaption);
	GetDlgItem(IDC_INSTRUCTION)->SetFont(m_pFont);
	GetDlgItem(IDC_DETAILS)->SetFont(m_pFont);
	m_nAlignBy = IDC_CHARACTER;
	m_nSegmentBy = IDC_KEEP;
	SetState(INIT);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgAnnotation::OnBack()
{
	UpdateData(TRUE);
	SetState(BACK);
	UpdateData(FALSE);
}

void CDlgAnnotation::OnNext()
{
	UpdateData(TRUE);
	SetState(NEXT);
	UpdateData(FALSE);
}

void CDlgAnnotation::OnRadio()
{
	switch(m_nState)
	{
	case ALIGNBY:
		m_nAlignBy = GetCheckedRadioButton(IDC_NONE,IDC_CHARACTER);
		switch(m_nAlignBy)
		{
		case IDC_NONE:
			SetText(IDC_DETAILS, IDS_AWIZ_DTL_NONE);
			break;
		case IDC_WORD:
			SetText(IDC_DETAILS, IDS_AWIZ_DTL_WORD);
			break;
		case IDC_CHARACTER:
			SetText(IDC_DETAILS, IDS_AWIZ_DTL_CHARACTER);
			break;
		default:
			SetText(IDC_DETAILS, "");
		}
		break;
	case SEGMENTBY:
		m_nSegmentBy = GetCheckedRadioButton(IDC_KEEP,IDC_MANUAL);
		switch(m_nSegmentBy)
		{
		case IDC_KEEP:
			SetText(IDC_DETAILS, IDS_AWIZ_DTL_KEEP);
			break;
		case IDC_AUTOMATIC:
			SetText(IDC_DETAILS, IDS_AWIZ_DTL_AUTOMATIC);
			break;
		case IDC_MANUAL:
			SetText(IDC_DETAILS, IDS_AWIZ_DTL_MANUAL);
			break;
		default:
			SetText(IDC_DETAILS, "");
		}
		break;
	}
}

/***************************************************************************/
// CDlgAnnotation::OnImport Opens a plain text file and imports
/***************************************************************************/
void CDlgAnnotation::OnImport()
{
	CFileDialog* pFileDialog = new CFileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Standard Format (*.sfm) |*.sfm| Text Files (*.txt) |*.txt| All Files (*.*) |*.*||"), this );
	if (pFileDialog->DoModal() == IDOK)
	{
		CSaString path = pFileDialog->GetPathName();
		Object_istream obs(path);
		if ( obs.getIos().fail() )
		{
			delete pFileDialog;
			return;
		}

		BOOL SFM = FALSE;

		SaveAnnotation();

		if ( obs.bAtBackslash() ) // assume SFM
		{
			CDlgAnnotationImport Import(m_bPhonetic,m_bPhonemic,m_bOrthographic);

			int result = Import.DoModal();

			if(result == IDCANCEL)
			{
				delete pFileDialog;
				return;
			}

			if(result == IDOK) // User confirmed it is an SFM
				SFM = TRUE;

			while ( SFM && !obs.bAtEnd())
			{
				if ( Import.m_bPhonetic && obs.bReadString(Import.m_szPhonetic, &m_szPhonetic) );
				else if ( Import.m_bPhonemic && obs.bReadString(Import.m_szPhonemic, &m_szPhonemic) );
				else if ( Import.m_bOrthographic && obs.bReadString(Import.m_szOrthographic, &m_szOrthographic) );
				else if ( Import.m_bGloss  && obs.bReadString(Import.m_szGloss, &m_szGloss) );
				else if ( obs.bEnd(psz_ImportEnd) )
					break;
			}
			if(SFM)
				SetAnnotation();
		}

		if(!SFM)
		{
			const int MAXLINE = 32000;
			char* pBuf = new char[MAXLINE];
			obs.getIos().seekg(0);  // start to file start
			obs.getIos().getline(pBuf,MAXLINE, '\000');
			SetText(IDC_ANNOTATION, pBuf);
			OnUpdateAnnotation(); // Set Modified
			delete [] pBuf;
		}
	}
	delete pFileDialog;
}

/***************************************************************************/
// CDlgAnnotation::OnRevert reverts to document annotation
/***************************************************************************/
void CDlgAnnotation::OnRevert()
{
	switch(m_nState)
	{
	case PHONETICTEXT:
		m_bPhoneticModified = FALSE;
		m_szPhonetic = BuildString(PHONETIC);
		break;
	case PHONEMICTEXT:
		m_bPhonemicModified = FALSE;
		m_szPhonemic = BuildString(PHONEMIC);
		break;
	case ORTHOGRAPHICTEXT:
		m_bOrthographicModified = FALSE;
		m_szOrthographic = BuildString(ORTHO);
		break;
	case GLOSSTEXT:
		m_bGlossModified = FALSE;
		m_szGloss = BuildString(GLOSS);
		break;
	}
	SetEnable(IDC_REVERT,FALSE);
	SetAnnotation();
}

/***************************************************************************/
// CDlgAnnotation::OnUpdateAnnotation records annotation changes
/***************************************************************************/
void CDlgAnnotation::OnUpdateAnnotation()
{
	switch(m_nState)
	{
	case PHONETICTEXT:
		m_bPhoneticModified = TRUE;
		SetEnable(IDC_REVERT,TRUE);
		break;
	case PHONEMICTEXT:
		m_bPhonemicModified = TRUE;
		SetEnable(IDC_REVERT,TRUE);
		break;
	case ORTHOGRAPHICTEXT:
		m_bOrthographicModified = TRUE;
		SetEnable(IDC_REVERT,TRUE);
		break;
	case GLOSSTEXT:
		m_bGlossModified = TRUE;
		SetEnable(IDC_REVERT,TRUE);
		break;
	}
}

//SDM 1.06.8 Split from OnOk
/***************************************************************************/
// CDlgAnnotation::OK Execute changes
/***************************************************************************/
void CDlgAnnotation::OK()
{
	// save state for undo ability
	m_pSaDoc->CheckPoint();
	m_pSaDoc->SetModifiedFlag(TRUE); // document has been modified
	m_pSaDoc->SetTransModifiedFlag(TRUE); // transcription has been modified

	POSITION pos = m_pSaDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)m_pSaDoc->GetNextView(pos);

	enum { CHARACTER_OFFSETS = 0,CHARACTER_DURATIONS = 1,WORD_OFFSETS = 2};
	CDWordArray* pArray = new CDWordArray[3];

	// Clean GLOSS string SDM 1.5Test8.2
	for(int nString = 0; nString < m_szGloss.GetLength(); nString++)
	{
		if (!nString && (m_szGloss[nString] == 0x20 /*space*/))
		{
			// remove leading spaces
			m_szGloss = m_szGloss.Mid(nString+1);
			nString --; // deleted a char
		}
		else if (nString && (m_szGloss[nString-1] != 0x20 /*space*/) && ((m_szGloss[nString] == WORD_DELIMITER) || (m_szGloss[nString] == TEXT_DELIMITER)))
		{
			// need to insert a space
			m_szGloss = m_szGloss.Left(nString) + " " + m_szGloss.Mid(nString);
			nString ++; // added a char
		}
		else if (nString && (m_szGloss[nString-1] == 0x20 /*space*/) && !((m_szGloss[nString] == WORD_DELIMITER) || (m_szGloss[nString] == TEXT_DELIMITER)))
		{
			// need to replace a space
			m_szGloss = m_szGloss.Left(nString-1) + "." + m_szGloss.Mid(nString);
		}
		else if (nString && (m_szGloss[nString] == 0x20 /*space*/) && ((m_szGloss[nString-1] == WORD_DELIMITER) || (m_szGloss[nString-1] == TEXT_DELIMITER)))
		{
			// need to remove a space
			m_szGloss = m_szGloss.Left(nString) + m_szGloss.Mid(nString+1);
			nString --; // deleted a char
		}
	}

	if (m_pSaDoc->GetSegment(GLOSS)->IsEmpty())
	{
		// auto parse
		if (!m_pSaDoc->AdvancedParse())
		{
			// process canceled by user
			m_pSaDoc->Undo(FALSE);
			return;
		}
	}

	CSegment* pSegment=m_pSaDoc->GetSegment(PHONETIC);
	//adjust character segments
	switch(m_nSegmentBy)
	{
	case IDC_AUTOMATIC:
		{
			if (!m_pSaDoc->AdvancedSegment()) // SDM 1.5Test8.2
			{
				// process canceled by user
				m_pSaDoc->Undo(FALSE);
				return;
			}
			pArray[CHARACTER_OFFSETS].InsertAt(0,pSegment->GetOffsets()); // Copy Arrays
			pArray[CHARACTER_DURATIONS].InsertAt(0,pSegment->GetDurations());

			pArray[WORD_OFFSETS].InsertAt(0,m_pSaDoc->GetSegment(GLOSS)->GetOffsets()); // Copy gloss segments SDM 1.5Test8.2
			// Create a gloss break at initial position SDM 1.5Test8.2
			if (pArray[WORD_OFFSETS][0] != pArray[CHARACTER_OFFSETS][0])
			{
				CSaString szEmpty = "";
				m_pSaDoc->GetSegment(GLOSS)->Insert(0, &szEmpty, FALSE, pArray[CHARACTER_OFFSETS][0], pArray[WORD_OFFSETS][0]-pArray[CHARACTER_OFFSETS][0]);
				pArray[WORD_OFFSETS].InsertAt(0,pArray[CHARACTER_OFFSETS][0]);
				m_szGloss = CSaString(WORD_DELIMITER) + " " + m_szGloss;
				m_szPhonetic = " " + m_szPhonetic;
				m_szPhonemic = " " + m_szPhonemic;
				m_szOrthographic = " " + m_szOrthographic;
			}
			break;
		}
	case IDC_MANUAL: // SDM 1.5Test8.2
		{
			pArray[WORD_OFFSETS].InsertAt(0,m_pSaDoc->GetSegment(GLOSS)->GetOffsets()); // Copy gloss segments SDM 1.5Test8.2
			switch(m_nAlignBy)
			{
			case IDC_NONE:
			case IDC_WORD:
				{
					pArray[CHARACTER_OFFSETS].InsertAt(0,&pArray[WORD_OFFSETS]); // Copy gloss segments
					// build duration list
					int nIndex = 1;

					while(nIndex < pArray[CHARACTER_OFFSETS].GetSize())
					{
						pArray[CHARACTER_DURATIONS].Add(pArray[CHARACTER_OFFSETS][nIndex] - pArray[CHARACTER_OFFSETS][nIndex - 1]);
						nIndex++;
					}
					// Add final duration to fill remainder of waveform
					pArray[CHARACTER_DURATIONS].Add(m_pSaDoc->GetUnprocessedDataSize() - pArray[CHARACTER_OFFSETS][nIndex - 1]);
					break;
				}
			case IDC_CHARACTER:
				{
					CFontTable* pTable = m_pSaDoc->GetFont(PHONETIC);
					int nWord = 0;
					int nCharacters;
					CSaString szWord;
					for(int nGlossWordIndex = 0; nGlossWordIndex < pArray[WORD_OFFSETS].GetSize(); nGlossWordIndex++)
					{
						DWORD dwDuration;
						if ((nGlossWordIndex+1) < pArray[WORD_OFFSETS].GetSize())
						{
							dwDuration = pArray[WORD_OFFSETS][nGlossWordIndex+1] - pArray[WORD_OFFSETS][nGlossWordIndex];
							szWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nWord, m_szPhonetic);
							nCharacters = pTable->GetLength(CFontTable::CHARACTER, szWord);
						}
						else
						{
							dwDuration = m_pSaDoc->GetUnprocessedDataSize() - pArray[WORD_OFFSETS][nGlossWordIndex];
							szWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nWord, m_szPhonetic);
							nCharacters = pTable->GetLength(CFontTable::CHARACTER, szWord);
							szWord = pTable->GetRemainder(CFontTable::DELIMITEDWORD, nWord, m_szPhonetic);
							if (szWord.GetLength() != 0) nCharacters++;  // remainder goes into one extra segment
						}
						if (nCharacters == 0) nCharacters++;

						DWORD dwOffset = pArray[WORD_OFFSETS][nGlossWordIndex];
						DWORD dwSize = dwDuration/nCharacters;

						if (m_pSaDoc->GetFmtParm()->wBlockAlign == 2) dwSize &= ~1;

						for(int nIndex = 0; nIndex < nCharacters; nIndex++)
						{
							pArray[CHARACTER_OFFSETS].Add(dwOffset);
							dwOffset = m_pSaDoc->SnapCursor(STOP_CURSOR, dwOffset + dwSize, dwOffset + (dwSize/2) & ~1, dwOffset + dwSize, SNAP_LEFT);
						}
					}
					int nIndex = 1;
					while(nIndex < pArray[CHARACTER_OFFSETS].GetSize())
					{
						pArray[CHARACTER_DURATIONS].Add(pArray[CHARACTER_OFFSETS][nIndex] - pArray[CHARACTER_OFFSETS][nIndex - 1]);
						nIndex++;
					}
					// Add final duration to fill remainder of waveform
					pArray[CHARACTER_DURATIONS].Add(m_pSaDoc->GetUnprocessedDataSize() - pArray[CHARACTER_OFFSETS][nIndex - 1]);
					break;
				}
			}
			break;
		}
	case IDC_KEEP: // SDM 1.5Test8.2
		{
			pArray[WORD_OFFSETS].InsertAt(0,m_pSaDoc->GetSegment(GLOSS)->GetOffsets()); // Copy gloss segments SDM 1.5Test8.2
			// copy segment locations not character counts
			int nIndex = 0;

			while(nIndex != -1)
			{
				pArray[CHARACTER_OFFSETS].Add(pSegment->GetOffset(nIndex));
				pArray[CHARACTER_DURATIONS].Add(pSegment->GetDuration(nIndex));
				nIndex = pSegment->GetNext(nIndex);
			}
		}
	}



	CFontTable::tUnit nAlignMode = CFontTable::CHARACTER;
	switch(m_nAlignBy)
	{
	case IDC_NONE:
		nAlignMode = CFontTable::NONE;
		break;
	case IDC_WORD:
		nAlignMode = CFontTable::DELIMITEDWORD;
		break;
	case IDC_CHARACTER:
	default:
		nAlignMode = CFontTable::CHARACTER;
	}

	// Insert Annotations
	{
		CSegment* pSegment;
		CFontTable* pTable;
		CSaString szNext;
		CSaString szNextWord;
		int nWordIndex;
		int nStringIndex;
		int nOffsetSize;
		int nGlossIndex;
		int nIndex;

		// Process phonetic
		if (m_bPhonetic || (m_nSegmentBy != IDC_KEEP)) // SDM 1.06.8 only change  if new segmentation or text changed
		{
			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = m_pSaDoc->GetSegment(PHONETIC);
			pTable = m_pSaDoc->GetFont(PHONETIC);
			pSegment->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for(nIndex = 0; nIndex < (nOffsetSize-1);nIndex++)
			{
				switch(m_nAlignBy)
				{
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonetic);
					if (szNext.GetLength()==0)szNext+=SEGMENT_DEFAULT_CHAR;
					pSegment->Insert(pSegment->GetSize(),&szNext, FALSE,
						pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_WORD:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) // No more word breaks continue one character at a time
					{
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonetic);
					}
					// Insert default segment character if phonetic offset does not correspond to word boundary
					else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex])
					{
						szNext = SEGMENT_DEFAULT_CHAR;
					}
					else // Insert Word on Gloss Boundary
					{
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonetic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength()==0)szNext+=SEGMENT_DEFAULT_CHAR;
					pSegment->Insert(pSegment->GetSize(),&szNext, FALSE,
						pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) // No more word breaks continue one character at a time
					{
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonetic);
					}
					else // SDM 1.5Test8.2
					{
						if (nStringIndex == 0) // Load first word
						{
							szNextWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nStringIndex, m_szPhonetic);
							nGlossIndex++;  // Increment word index
						}
						// If next phonetic offset not new word append one character
						int nOffset = pArray[CHARACTER_OFFSETS][nIndex+1];
						int nNextWordOffset = pArray[CHARACTER_OFFSETS][nIndex+1] + 1;
						if (nGlossIndex < pArray[WORD_OFFSETS].GetSize())
							nNextWordOffset = pArray[WORD_OFFSETS][nGlossIndex];

						if (nOffset < nNextWordOffset)
						{
							szNext = pTable->GetNext(nAlignMode, nWordIndex, szNextWord);
						}
						else // finish word and get next word
						{
							szNext = pTable->GetRemainder(nAlignMode, nWordIndex, szNextWord);
							nGlossIndex++;  // Increment word index
							if (nGlossIndex<pArray[WORD_OFFSETS].GetSize()) // More word breaks load next word
							{
								szNextWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nStringIndex, m_szPhonetic);
								nWordIndex = 0; // Start at beginning of next word
							}
						}
					}

					if (szNext.GetLength()==0)szNext+=SEGMENT_DEFAULT_CHAR;
					pSegment->Insert(pSegment->GetSize(),&szNext, FALSE,
						pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				}
			}
			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, m_szPhonetic);
			if (szNext.GetLength()==0)szNext+=SEGMENT_DEFAULT_CHAR;
			pSegment->Insert(pSegment->GetSize(),&szNext,FALSE,
				pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
			// SDM 1.06.8 apply input filter to segment
			if (pSegment->GetInputFilter())
			{
				(pSegment->GetInputFilter())(*pSegment->GetString());
			}
		}

		// Process phonemic
		if (m_bPhonemic || (m_nSegmentBy != IDC_KEEP)) // SDM 1.06.8 only change  if new segmentation or text changed
		{
			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = m_pSaDoc->GetSegment(PHONEMIC);
			pTable = m_pSaDoc->GetFont(PHONEMIC);
			pSegment->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for(nIndex = 0; nIndex < (nOffsetSize-1);nIndex++)
			{
				switch(m_nAlignBy)
				{
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonemic);
					if (szNext.GetLength()!=0) // Skip Empty Segments
						pSegment->Insert(pSegment->GetSize(),&szNext, FALSE,
						pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_WORD:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) // No more word breaks continue one character at a time
					{
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonemic);
					}
					// Skip if phonetic offset does not correspond to word boundary
					else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex])
					{
						continue;
					}
					else // Insert Word on Gloss Boundary
					{
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonemic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength()==0) continue; // Skip NULL strings
					pSegment->Insert(pSegment->GetSize(),&szNext, FALSE,
						pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) // No more word breaks continue one character at a time
					{
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonemic);
					}
					else // SDM 1.5Test8.2
					{
						if (nStringIndex == 0) // Load first word
						{
							szNextWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nStringIndex, m_szPhonemic);
							nGlossIndex++;  // Increment word index
						}
						// If next phonetic offset not new word append one character
						int nOffset = pArray[CHARACTER_OFFSETS][nIndex+1];
						int nNextWordOffset = pArray[CHARACTER_OFFSETS][nIndex+1] + 1;
						if (nGlossIndex < pArray[WORD_OFFSETS].GetSize())
							nNextWordOffset = pArray[WORD_OFFSETS][nGlossIndex];

						if (nOffset < nNextWordOffset)
						{
							szNext = pTable->GetNext(nAlignMode, nWordIndex, szNextWord);
						}
						else // finish word and get next word
						{
							szNext = pTable->GetRemainder(nAlignMode, nWordIndex, szNextWord);
							nGlossIndex++;  // Increment word index
							if (nGlossIndex<pArray[WORD_OFFSETS].GetSize()) // More word breaks load next word
							{
								szNextWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nStringIndex, m_szPhonemic);
								nWordIndex = 0; // Start at beginning of next word
							}
						}
					}

					if (szNext.GetLength()==0) continue; // Skip NULL strings
					pSegment->Insert(pSegment->GetSize(),&szNext, FALSE,
						pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				}
			}
			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, m_szPhonemic);
			if (szNext.GetLength()!=0) // Skip empty segments
				pSegment->Insert(pSegment->GetSize(),&szNext,FALSE,
				pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
			// SDM 1.06.8 apply input filter to segment
			if (pSegment->GetInputFilter())
			{
				(pSegment->GetInputFilter())(*pSegment->GetString());
			}
		}

		// Process tone
		if (m_nSegmentBy != IDC_KEEP) // SDM 1.06.8 only change  if new segmentation or text changed
		{
			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = m_pSaDoc->GetSegment(TONE);
			pSegment->DeleteContents();
		}

		// Process orthographic
		if (m_bOrthographic || (m_nSegmentBy != IDC_KEEP)) // SDM 1.06.8 only change  if new segmentation or text changed
		{
			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = m_pSaDoc->GetSegment(ORTHO);
			pTable = m_pSaDoc->GetFont(ORTHO);
			pSegment->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for(nIndex = 0; nIndex < (nOffsetSize-1);nIndex++)
			{
				switch(m_nAlignBy)
				{
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szOrthographic);
					if (szNext.GetLength()!=0) // Skip Empty Segments
						pSegment->Insert(pSegment->GetSize(),&szNext, FALSE,
						pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_WORD:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) // No more word breaks continue one character at a time
					{
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szOrthographic);
					}
					// Skip if character offset does not correspond to word boundary
					else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex])
					{
						continue;
					}
					else // Insert Word on Boundary
					{
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szOrthographic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength()==0) continue; // Skip NULL words
					pSegment->Insert(pSegment->GetSize(),&szNext, FALSE,
						pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) // No more word breaks continue one character at a time
					{
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szOrthographic);
					}
					else // SDM 1.5Test8.2
					{
						if (nStringIndex == 0) // Load first word
						{
							szNextWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nStringIndex, m_szOrthographic);
							nGlossIndex++;  // Increment word index
						}
						// If next phonetic offset not new word append one character
						int nOffset = pArray[CHARACTER_OFFSETS][nIndex+1];
						int nNextWordOffset = pArray[CHARACTER_OFFSETS][nIndex+1] + 1;
						if (nGlossIndex < pArray[WORD_OFFSETS].GetSize())
							nNextWordOffset = pArray[WORD_OFFSETS][nGlossIndex];

						if (nOffset < nNextWordOffset)
						{
							szNext = pTable->GetNext(nAlignMode, nWordIndex, szNextWord);
						}
						else // finish word and get next word
						{
							szNext = pTable->GetRemainder(nAlignMode, nWordIndex, szNextWord);
							nGlossIndex++;  // Increment word index
							if (nGlossIndex<pArray[WORD_OFFSETS].GetSize()) // More word breaks load next word
							{
								szNextWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nStringIndex, m_szOrthographic);
								nWordIndex = 0; // Start at beginning of next word
							}
						}
					}

					if (szNext.GetLength()==0) continue; // Skip NULL strings
					pSegment->Insert(pSegment->GetSize(),&szNext, FALSE,
						pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				}
			}
			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, m_szOrthographic);
			if (szNext.GetLength()!=0) // Skip empty segments
				pSegment->Insert(pSegment->GetSize(),&szNext,FALSE,
				pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
			// SDM 1.06.8 apply input filter to segment
			if (pSegment->GetInputFilter())
			{
				(pSegment->GetInputFilter())(*pSegment->GetString());
			}
		}

		// Process gloss
		if (m_bGlossModified) // SDM 1.5Test8.2 only change if text changed
		{
			nStringIndex = 0;
			pSegment = m_pSaDoc->GetSegment(GLOSS);
			pTable = m_pSaDoc->GetFont(GLOSS);

			// align gloss by word SDM 1.5Test8.2
			nAlignMode = CFontTable::DELIMITEDWORD;
			nOffsetSize = pArray[WORD_OFFSETS].GetSize();
			pSegment->SelectSegment(*m_pSaDoc,-1); // Don't Select this segment SDM 1.5Test8.2
			for(nIndex = 0; nIndex < (nOffsetSize-1);nIndex++)
			{
				szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szGloss);
				if (szNext.GetLength()==0) szNext = WORD_DELIMITER; //SDM 1.5Test8.2
				pSegment->SelectSegment(*m_pSaDoc,nIndex);
				((CGlossSegment*)pSegment)->ReplaceSelectedSegment(m_pSaDoc,szNext);
			};
			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, m_szGloss);
			if (szNext.GetLength()==0) szNext = WORD_DELIMITER; //SDM 1.5Test8.2
			pSegment->SelectSegment(*m_pSaDoc,nIndex);
			((CGlossSegment*)pSegment)->ReplaceSelectedSegment(m_pSaDoc,szNext);
		}
	}

	pView->ChangeAnnotationSelection(pSegment, -1);

	CGraphWnd *pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);

	if(pGraph)
	{
		if(m_bPhonetic)
			pGraph->ShowAnnotation(PHONETIC, TRUE, TRUE);
		if(m_bPhonemic)
			pGraph->ShowAnnotation(PHONEMIC, TRUE, TRUE);
		if(m_bOrthographic)
			pGraph->ShowAnnotation(ORTHO, TRUE, TRUE);
		if(m_bGloss)
			pGraph->ShowAnnotation(GLOSS, TRUE, TRUE);
	}


	pView->RefreshGraphs(); // redraw all graphs without legend window

	delete [] pArray;
}

// SDM 1.06.8 Split
/***************************************************************************/
// CDlgAnnotation::OnOK Execute changes
/***************************************************************************/
void CDlgAnnotation::OnOK()
{
	OK();
	CDialog::OnOK();
}

/***************************************************************************/
// CDlgAnnotation::AutoAlign Execute changes by request from batch file
/***************************************************************************/
void CDlgAnnotation::AutoAlign(const CSaString& Phonetic, const CSaString& Phonemic,
									 const CSaString& Ortho, const CSaString& Gloss,
									 CSaString& Alignment, CSaString& Segmentation)
{
	m_bPhonetic = (Phonetic.GetLength() != 0);
	m_bPhonemic = (Phonemic.GetLength() != 0);
	m_bOrthographic = (Ortho.GetLength() != 0);
	m_bGloss = (Gloss.GetLength() != 0);

	// SDM 1.5Test8.2
	m_bPhoneticModified = m_bPhonetic;
	m_bPhonemicModified = m_bPhonemic;
	m_bOrthographicModified = m_bOrthographic;
	m_bGlossModified = m_bGloss;

	if (m_bPhonetic) m_szPhonetic = ReadFile(Phonetic);
	if (m_bPhonemic) m_szPhonemic = ReadFile(Phonemic);
	if (m_bOrthographic) m_szOrthographic = ReadFile(Ortho);
	if (m_bGloss) m_szGloss = ReadFile(Gloss);

	m_nAlignBy = IDC_CHARACTER;
	Alignment.MakeUpper();
	if (Alignment=="NONE") m_nAlignBy = IDC_NONE;
	if (Alignment=="WORD") m_nAlignBy = IDC_WORD;

	m_nSegmentBy = IDC_KEEP;
	Segmentation.MakeUpper();
	if (Segmentation=="MANUAL") m_nSegmentBy = IDC_MANUAL;
	if (Segmentation=="AUTOMATIC") m_nSegmentBy = IDC_AUTOMATIC;

	OK();
}


/***************************************************************************/
// CDlgAnnotation::AutoAlign Execute changes by request from batch file
/***************************************************************************/
void CDlgAnnotation::AutoAlign(const CSaString* Phonetic, const CSaString* Phonemic,
									 const CSaString* Ortho, const CSaString* Gloss)
{
	m_bPhonetic = (Phonetic != 0);
	m_bPhonemic = (Phonemic != 0);
	m_bOrthographic = (Ortho != 0);
	m_bGloss = (Gloss != 0);

	m_bPhoneticModified = m_bPhonetic;
	m_bPhonemicModified = m_bPhonemic;
	m_bOrthographicModified = m_bOrthographic;
	m_bGlossModified = m_bGloss;

	if (m_bPhonetic) m_szPhonetic = *(Phonetic);
	if (m_bPhonemic) m_szPhonemic = *(Phonemic);
	if (m_bOrthographic) m_szOrthographic = *(Ortho);
	if (m_bGloss) m_szGloss = *(Gloss);

	m_nAlignBy = IDC_CHARACTER;

	m_nSegmentBy = IDC_KEEP;

	OK();
}


/***************************************************************************/
// CDlgAnnotation::ReadFile Execute changes
/***************************************************************************/
const CSaString CDlgAnnotation::ReadFile(const CSaString& Pathname)
{
	CFileStatus TheStatus;
	if (!CFile::GetStatus(Pathname, TheStatus)) return CSaString(_T("")); // file does not exist
	CFile* pFile = new CFile(Pathname,CFile::modeRead+CFile::typeBinary);
	char* pBuf = new char[32768];
	int nLength = pFile->Read(pBuf,32767);
	pBuf[nLength] = NULL; // Null terminated
	CSaString String = pBuf;
	delete [] pBuf;
	delete pFile;
	return String;
}
