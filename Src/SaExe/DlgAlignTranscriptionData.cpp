/////////////////////////////////////////////////////////////////////////////
// DlgAnnotation.cpp:
// Implementation of the CDlgAlignTranscriptionData class.
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

#include "dlgaligntranscriptiondata.h"
#include "result.h"
#include <math.h>
#include "TranscriptionDataSettings.h"
#include "DlgImportSFM.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAlignTranscriptionData dialog
static const char* psz_ImportEnd = "import";

CDlgAlignTranscriptionData::CDlgAlignTranscriptionData(CWnd* pParent, CSaDoc* pSaDoc) : 
CDialog(CDlgAlignTranscriptionData::IDD, pParent)
{

	// SDM 1.5Test8.2 changed default initialization
	//{{AFX_DATA_INIT(CDlgAlignTranscriptionData)
	m_bOrthographic = FALSE;
	m_bPhonemic = FALSE;
	m_bPhonetic = TRUE;
	m_bGloss = FALSE;
	//}}AFX_DATA_INIT
	m_nState = 0;
	m_bPhoneticModified = false;
	m_bPhonemicModified = false;
	m_bOrthographicModified = false;
	m_bGlossModified = false; 
	// SDM 1.5Test8.2
	// Font used for instructions and details
	m_pFont = new CFont;
	m_pFont->CreateFont( 8, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_SWISS+TMPF_TRUETYPE+DEFAULT_PITCH,_T("MS Sans Serif") );
	m_pSaDoc = pSaDoc;
	m_szPhonetic = BuildString(PHONETIC);
	m_szPhonemic = BuildString(PHONEMIC);
	m_szOrthographic = BuildString(ORTHO);
	m_szGloss = BuildString(GLOSS);
}

CDlgAlignTranscriptionData::~CDlgAlignTranscriptionData()
{
	delete m_pFont;
}

void CDlgAlignTranscriptionData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAlignTranscriptionData)
	DDX_Check(pDX, IDC_ORTHOGRAPHIC, m_bOrthographic);
	DDX_Check(pDX, IDC_PHONEMIC, m_bPhonemic);
	DDX_Check(pDX, IDC_PHONETIC, m_bPhonetic);
	DDX_Check(pDX, IDC_GLOSS, m_bGloss);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgAlignTranscriptionData, CDialog)
	//{{AFX_MSG_MAP(CDlgAlignTranscriptionData)
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
	ON_BN_CLICKED(IDC_PHONETIC, OnCheck)
	ON_BN_CLICKED(IDC_PHONEMIC, OnCheck)
	ON_BN_CLICKED(IDC_GLOSS, OnCheck)
	ON_BN_CLICKED(IDC_ORTHOGRAPHIC, OnCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDlgAlignTranscriptionData::SetVisible(int nItem, BOOL bVisible)
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
// CDlgAlignTranscriptionData::SetEnable Enables/Disables controls
/***************************************************************************/
void CDlgAlignTranscriptionData::SetEnable(int nItem, BOOL bEnable)
{
	
	CWnd* pWnd = GetDlgItem(nItem);
	if (pWnd) 
	{
		pWnd->EnableWindow(bEnable);
	}
}

/***************************************************************************/
// CDlgAlignTranscriptionData::SetText Sets control text to string
/***************************************************************************/
void CDlgAlignTranscriptionData::SetText(int nItem, CSaString szText) 
{
	CWnd* pWnd = GetDlgItem(nItem);
	if (pWnd) 
	{
		pWnd->SetWindowText(szText);
	}
}

/***************************************************************************/
// CDlgAlignTranscriptionData::SetText Sets control text to string from IDS
/***************************************************************************/
void CDlgAlignTranscriptionData::SetText(int nItem, UINT nIDS) 
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
// CDlgAlignTranscriptionData::BuildString builds an annotation string
/***************************************************************************/
const CSaString CDlgAlignTranscriptionData::BuildString(int nSegment)
{

	CSaString szBuild = "";
	CSaString szWorking = "";
	CSaString szSegmentDefault = SEGMENT_DEFAULT_CHAR;

	if (m_pSaDoc == NULL)
	{
		return szBuild;
	}

	CSegment* pSegment = m_pSaDoc->GetSegment(nSegment);

	if (pSegment == NULL)
	{
		return szBuild;
	}
	if (pSegment->IsEmpty())
	{
		return szBuild;
	}

	const char wordDelimiter = m_pSaDoc->GetFont(nSegment)->m_wordDelimiter;

	int nIndex = 0;

	switch (nSegment) 
	{

	case PHONETIC:
		while (nIndex != -1)
		{
			szWorking = pSegment->GetSegmentString(nIndex);
			if (szWorking != szSegmentDefault)
			{
				szBuild += szWorking;
				szBuild += wordDelimiter;
			}
			nIndex = pSegment->GetNext(nIndex);
		}
		break;

	case PHONEMIC:
	case ORTHO:
		while (nIndex != -1) 
		{
			szWorking = pSegment->GetSegmentString(nIndex);
			szBuild += szWorking;
			szBuild += wordDelimiter;
			nIndex = pSegment->GetNext(nIndex);
		}
		break;

	case GLOSS:
		while (nIndex != -1) 
		{
			szWorking = pSegment->GetSegmentString(nIndex);
			szBuild += szWorking;
			szBuild += wordDelimiter;
			nIndex = pSegment->GetNext(nIndex);
		}
		break;
	default:
		;
	}
	return szBuild;
}


/***************************************************************************/
// CDlgAlignTranscriptionData::SetState set state of wizard
/***************************************************************************/
void CDlgAlignTranscriptionData::SetState(int nState)
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
		SetVisible(IDC_GLOSS,TRUE);	//SDM 1.5 Test10.0
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
// CDlgAlignTranscriptionData::SetAnnotation Sets annotation font and text
/***************************************************************************/
void CDlgAlignTranscriptionData::SetAnnotation(void)
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
// CDlgAlignTranscriptionData::SaveAnnotation saves the annotation text
/***************************************************************************/
void CDlgAlignTranscriptionData::SaveAnnotation(void)
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
// CDlgAlignTranscriptionData message handlers


/***************************************************************************/
// CDlgAlignTranscriptionData::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgAlignTranscriptionData::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow(); // center dialog on frame window
	CSaString szCaption;
	szCaption.LoadString(IDS_AWIZ_CAPTION_ALIGN); // load caption string
	SetWindowText(szCaption);
	GetDlgItem(IDC_INSTRUCTION)->SetFont(m_pFont);
	GetDlgItem(IDC_DETAILS)->SetFont(m_pFont);
	m_nAlignBy = IDC_CHARACTER;
	m_nSegmentBy = IDC_KEEP;
	SetState(INIT);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgAlignTranscriptionData::OnBack()
{
	UpdateData(TRUE);
	SetState(BACK);
	UpdateData(FALSE);
}

void CDlgAlignTranscriptionData::OnNext()
{
	UpdateData(TRUE);
	SetState(NEXT);
	UpdateData(FALSE);
}

void CDlgAlignTranscriptionData::OnRadio()
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

void CDlgAlignTranscriptionData::OnCheck()
{

	BOOL enable = ((IsDlgButtonChecked(IDC_PHONETIC))||
				   (IsDlgButtonChecked(IDC_PHONEMIC))||
				   (IsDlgButtonChecked(IDC_GLOSS))||
				   (IsDlgButtonChecked(IDC_ORTHOGRAPHIC)));
	SetEnable(IDC_NEXT,enable);
}

/***************************************************************************/
// CDlgAlignTranscriptionData::OnImport Opens a plain text file and imports
/***************************************************************************/
void CDlgAlignTranscriptionData::OnImport()
{

	CFileDialog* pFileDialog = new CFileDialog(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Standard Format (*.sfm) |*.sfm| Text Files (*.txt) |*.txt| All Files (*.*) |*.*||"), this );
	if (pFileDialog->DoModal() == IDOK)
	{
		CSaString path = pFileDialog->GetPathName();
		Object_istream obs(path);
		if ( obs.getIos().fail()) 
		{
			delete pFileDialog;
			return;
		}

		BOOL SFM = FALSE;

		SaveAnnotation();

		if ( obs.bAtBackslash() )
		{ // assume SFM
			CDlgImportSFM dlg(m_bPhonetic,m_bPhonemic,m_bOrthographic);
			int result = dlg.DoModal();

			if (result == IDCANCEL) 
			{
				delete pFileDialog;
				return;
			}

			if (result == IDOK) // User confirmed it is an SFM
				SFM = TRUE;

			while ( SFM && !obs.bAtEnd()) 
			{
				if ( dlg.m_bPhonetic && obs.bReadString(dlg.m_szPhonetic, &m_szPhonetic) );
				else if ( dlg.m_bPhonemic && obs.bReadString(dlg.m_szPhonemic, &m_szPhonemic) );
				else if ( dlg.m_bOrthographic && obs.bReadString(dlg.m_szOrthographic, &m_szOrthographic) );
				else if ( dlg.m_bGloss  && obs.bReadString(dlg.m_szGloss, &m_szGloss) );
				else if ( obs.bEnd(psz_ImportEnd) )
					break;
			}
			if (SFM)
				SetAnnotation();
		}

		if (!SFM)
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
// CDlgAlignTranscriptionData::OnRevert reverts to document annotation
/***************************************************************************/
void CDlgAlignTranscriptionData::OnRevert()
{
	
	switch (m_nState)
	{
	case PHONETICTEXT:
		m_bPhoneticModified = false;
		m_szPhonetic = BuildString(PHONETIC);
		break;
	case PHONEMICTEXT:
		m_bPhonemicModified = false;
		m_szPhonemic = BuildString(PHONEMIC);
		break;
	case ORTHOGRAPHICTEXT:
		m_bOrthographicModified = false;
		m_szOrthographic = BuildString(ORTHO);
		break;
	case GLOSSTEXT:
		m_bGlossModified = false;
		m_szGloss = BuildString(GLOSS);
		break;
	}
	SetEnable(IDC_REVERT,FALSE);
	SetAnnotation();
}

/***************************************************************************/
// CDlgAlignTranscriptionData::OnUpdateAnnotation records annotation changes
/***************************************************************************/
void CDlgAlignTranscriptionData::OnUpdateAnnotation()
{
	
	switch(m_nState)
	{
	case PHONETICTEXT:
		m_bPhoneticModified = true;
		SetEnable(IDC_REVERT,TRUE);
		break;
	case PHONEMICTEXT:
		m_bPhonemicModified = true;
		SetEnable(IDC_REVERT,TRUE);
		break;
	case ORTHOGRAPHICTEXT:
		m_bOrthographicModified = true;
		SetEnable(IDC_REVERT,TRUE);
		break;
	case GLOSSTEXT:
		m_bGlossModified = true;
		SetEnable(IDC_REVERT,TRUE);
		break;
	}
}

/***************************************************************************/
// CDlgAlignTranscriptionData::ReadFile Execute changes
/***************************************************************************/
const CSaString CDlgAlignTranscriptionData::ReadFile(const CSaString& Pathname)
{

	CFileStatus TheStatus;
	if (!CFile::GetStatus(Pathname, TheStatus))
	{
		return CSaString(_T("")); // file does not exist
	}
	CFile* pFile = new CFile(Pathname,CFile::modeRead+CFile::typeBinary);
	char* pBuf = new char[32768];
	int nLength = pFile->Read(pBuf,32767);
	pBuf[nLength] = NULL; // Null terminated
	CSaString String = pBuf;
	delete [] pBuf;
	delete pFile;
	return String;
}

void CDlgAlignTranscriptionData::GetSettings( CTranscriptionDataSettings & result)
{
	result.m_bPhoneticModified = m_bPhoneticModified;
	result.m_bPhonemicModified = m_bPhonemicModified;
	result.m_bOrthographicModified = m_bOrthographicModified;
	result.m_bGlossModified = m_bGlossModified;
	result.m_nAlignBy = m_nAlignBy;
	result.m_nSegmentBy = m_nSegmentBy;
	result.m_bOrthographic = (m_bOrthographic!=FALSE);
	result.m_bPhonemic = (m_bPhonemic!=FALSE);
	result.m_bPhonetic = (m_bPhonetic!=FALSE);
	result.m_bGloss = (m_bGloss!=FALSE);
	result.m_szPhonetic = m_szPhonetic;
	result.m_szPhonemic = m_szPhonemic;
	result.m_szOrthographic = m_szOrthographic;
	result.m_szGloss = m_szGloss;
}
