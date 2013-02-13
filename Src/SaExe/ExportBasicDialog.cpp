/////////////////////////////////////////////////////////////////////////////
// ExportBasicDialog.cpp:
// Implementation of the CExportBasicDialog class.
// Author: Steve MacLean
// copyright 2012 JAARS Inc. SIL
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

#include "exportbasicdialog.h"
#include "result.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportBasicDialog dialog

CExportBasicDialog::CExportBasicDialog( const CSaString& szDocTitle, CWnd* pParent /*=NULL*/)
: CDialog(CExportBasicDialog::IDD, pParent)
{
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

void CExportBasicDialog::DoDataExchange(CDataExchange* pDX)
{
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

BEGIN_MESSAGE_MAP(CExportBasicDialog, CDialog)
	ON_BN_CLICKED(IDC_EX_SFM_ALL_SOURCE, OnAllSource)
	ON_BN_CLICKED(IDC_EX_SFM_FILE_INFO, OnAllFileInfo)
	ON_BN_CLICKED(IDC_EX_SFM_RECORD_DATA, OnAllParameters)
	ON_BN_CLICKED(IDC_EXTAB_ANNOTATIONS, OnAllAnnotations)
	ON_COMMAND(IDHELP, OnHelpExportBasic)
	ON_BN_CLICKED(IDC_EX_SFM_INTERLINEAR, &CExportBasicDialog::OnClickedExSfmInterlinear)
	ON_BN_CLICKED(IDC_EX_SFM_MULTIRECORD, &CExportBasicDialog::OnClickedExSfmMultirecord)
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

	InitializeDialog();

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

void CExportBasicDialog::SetCheck(int nItem, BOOL bChecked)
{
	CButton * pWnd = (CButton*)GetDlgItem(nItem);
	if (pWnd)
	{
		pWnd->SetCheck(bChecked);
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

void CExportBasicDialog::OnClickedExSfmInterlinear()
{
	CButton * pWnd = (CButton*)GetDlgItem(IDC_EX_SFM_MULTIRECORD);
	if (pWnd) {
		pWnd->SetCheck(FALSE);
	}
}

void CExportBasicDialog::OnClickedExSfmMultirecord()
{
	CButton * pWnd = (CButton*)GetDlgItem(IDC_EX_SFM_INTERLINEAR);
	if (pWnd) {
		pWnd->SetCheck(FALSE);
	}
}
