/////////////////////////////////////////////////////////////////////////////
// DlgAnnotation.cpp:
// Implementation of the CDlgAnnotation class.
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

#include "dlgannotationimport.h"
#include "dlgannotation.h"
#include "result.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAnnotation dialog
static const char* psz_ImportEnd = "import";

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
	m_bPhoneticModified = false;
	m_bPhonemicModified = false;
	m_bOrthographicModified = false;
	m_bGlossModified = false; 
	// SDM 1.5Test8.2
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
	ON_BN_CLICKED(IDC_PHONETIC, OnCheck)
	ON_BN_CLICKED(IDC_PHONEMIC, OnCheck)
	ON_BN_CLICKED(IDC_GLOSS, OnCheck)
	ON_BN_CLICKED(IDC_ORTHOGRAPHIC, OnCheck)
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
void CDlgAnnotation::SetText(int nItem, UINT nIDS) {

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
const CSaString CDlgAnnotation::BuildString(int nSegment) {

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

void CDlgAnnotation::OnCheck()
{
	BOOL enable = ((IsDlgButtonChecked(IDC_PHONETIC))||
				   (IsDlgButtonChecked(IDC_PHONEMIC))||
				   (IsDlgButtonChecked(IDC_GLOSS))||
				   (IsDlgButtonChecked(IDC_ORTHOGRAPHIC)));
	SetEnable(IDC_NEXT,enable);
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

			if (result == IDCANCEL)
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
// CDlgAnnotation::OnRevert reverts to document annotation
/***************************************************************************/
void CDlgAnnotation::OnRevert()
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
// CDlgAnnotation::OnUpdateAnnotation records annotation changes
/***************************************************************************/
void CDlgAnnotation::OnUpdateAnnotation()
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

//SDM 1.06.8 Split from OnOk
/***************************************************************************/
// CDlgAnnotation::OK Execute changes
/***************************************************************************/
void CDlgAnnotation::OK() {

	// save state for undo ability
	m_pSaDoc->CheckPoint();
	m_pSaDoc->SetModifiedFlag(TRUE); // document has been modified
	m_pSaDoc->SetTransModifiedFlag(TRUE); // transcription has been modified

	POSITION pos = m_pSaDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)m_pSaDoc->GetNextView(pos);

	enum { CHARACTER_OFFSETS = 0,CHARACTER_DURATIONS = 1,WORD_OFFSETS = 2};
	CDWordArray pArray[3];

	// clean gloss string
	// remove trailing and leading spaces
	m_szGloss.Trim(EDIT_WORD_DELIMITER);

	if (m_pSaDoc->GetSegment(GLOSS)->IsEmpty()) {
		// auto parse
		if (!m_pSaDoc->AdvancedParseAuto()) {
			// process canceled by user
			m_pSaDoc->Undo(FALSE);
			return;
		}
	}

	CSegment* pSegment=m_pSaDoc->GetSegment(PHONETIC);
	
	//adjust character segments
	switch (m_nSegmentBy) 
	{
	case IDC_AUTOMATIC:
		{
			if (!m_pSaDoc->AdvancedSegment()) {
			// SDM 1.5Test8.2
				// process canceled by user
				m_pSaDoc->Undo(FALSE);
				return;
			}
			for (int i=0;i<pSegment->GetOffsetSize();i++) {
				pArray[CHARACTER_OFFSETS].InsertAt(i,pSegment->GetOffset(i)); // Copy Arrays
			}
			for (int i=0;i<pSegment->GetDurationSize();i++) {
				pArray[CHARACTER_DURATIONS].InsertAt(i,pSegment->GetDuration(i));
			}
			// Copy gloss segments SDM 1.5Test8.2
			for (int i=0;i<m_pSaDoc->GetSegment(GLOSS)->GetOffsetSize();i++) {
				pArray[WORD_OFFSETS].InsertAt(i,m_pSaDoc->GetSegment(GLOSS)->GetOffset(i)); 
			}
			// Create a gloss break at initial position SDM 1.5Test8.2
			if (pArray[WORD_OFFSETS][0] != pArray[CHARACTER_OFFSETS][0]) {
				CSaString szEmpty = "";
				m_pSaDoc->GetSegment(GLOSS)->Insert(0, &szEmpty, FALSE, pArray[CHARACTER_OFFSETS][0], pArray[WORD_OFFSETS][0]-pArray[CHARACTER_OFFSETS][0]);
				pArray[WORD_OFFSETS].InsertAt(0,pArray[CHARACTER_OFFSETS][0]);
				m_szGloss = CSaString(EDIT_WORD_DELIMITER) + m_szGloss;
				m_szPhonetic = CSaString(EDIT_WORD_DELIMITER) + m_szPhonetic;
				m_szPhonemic = CSaString(EDIT_WORD_DELIMITER) + m_szPhonemic;
				m_szOrthographic = CSaString(EDIT_WORD_DELIMITER) + m_szOrthographic;
			}
			break;
		}

	case IDC_MANUAL: 
		{
		// SDM 1.5Test8.2
			for (int i=0;i<m_pSaDoc->GetSegment(GLOSS)->GetOffsetSize();i++) {
				pArray[WORD_OFFSETS].InsertAt(i,m_pSaDoc->GetSegment(GLOSS)->GetOffset(i)); // Copy gloss segments SDM 1.5Test8.2
			}
			switch(m_nAlignBy) {
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
					for(int nGlossWordIndex = 0; nGlossWordIndex < pArray[WORD_OFFSETS].GetSize(); nGlossWordIndex++) {
						DWORD dwDuration;
						if ((nGlossWordIndex+1) < pArray[WORD_OFFSETS].GetSize()) {
							dwDuration = pArray[WORD_OFFSETS][nGlossWordIndex+1] - pArray[WORD_OFFSETS][nGlossWordIndex];
							szWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nWord, m_szPhonetic);
							nCharacters = pTable->GetLength(CFontTable::CHARACTER, szWord);
						} else {
							dwDuration = m_pSaDoc->GetUnprocessedDataSize() - pArray[WORD_OFFSETS][nGlossWordIndex];
							szWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nWord, m_szPhonetic);
							nCharacters = pTable->GetLength(CFontTable::CHARACTER, szWord);
							szWord = pTable->GetRemainder(CFontTable::DELIMITEDWORD, nWord, m_szPhonetic);
							if (szWord.GetLength() != 0) nCharacters++;  // remainder goes into one extra segment
						}
						if (nCharacters == 0) {
							nCharacters++;
						}

						DWORD dwOffset = pArray[WORD_OFFSETS][nGlossWordIndex];
						DWORD dwSize = dwDuration/nCharacters;

						if (m_pSaDoc->GetFmtParm()->wBlockAlign == 2) {
							dwSize &= ~1;
						}

						for(int nIndex = 0; nIndex < nCharacters; nIndex++) {
							pArray[CHARACTER_OFFSETS].Add(dwOffset);
							dwOffset = m_pSaDoc->SnapCursor(STOP_CURSOR, dwOffset + dwSize, dwOffset + (dwSize/2) & ~1, dwOffset + dwSize, SNAP_LEFT);
						}
					}
					int nIndex = 1;
					while(nIndex < pArray[CHARACTER_OFFSETS].GetSize()) {
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
			// Copy gloss segments SDM 1.5Test8.2
			for (int i=0;i<m_pSaDoc->GetSegment(GLOSS)->GetOffsetSize();i++) {
				DWORD offset = m_pSaDoc->GetSegment(GLOSS)->GetOffset(i);
				TRACE("word offset %d\n",offset);
				pArray[WORD_OFFSETS].InsertAt(i,offset);
			}
			
			// copy segment locations not character counts
			int nIndex = 0;
			while (nIndex != -1) {
				DWORD offset = pSegment->GetOffset(nIndex);
				pArray[CHARACTER_OFFSETS].Add(offset);
				TRACE("character offset %d\n",offset);
				DWORD duration = pSegment->GetDuration(nIndex);
				pArray[CHARACTER_DURATIONS].Add(duration);
				TRACE("character duration %d\n",offset);
				nIndex = pSegment->GetNext(nIndex);
			}
		}
	}

	CFontTable::tUnit nAlignMode = CFontTable::CHARACTER;
	switch (m_nAlignBy) {
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
		CSegment * pSegment = NULL;
		CFontTable * pTable = NULL;
		CSaString szNext;
		CSaString szNextWord;
		int nWordIndex = 0;
		int nStringIndex = 0;
		int nOffsetSize = 0;
		int nGlossIndex = 0;
		int nIndex = 0;

		// Process phonetic
		// SDM 1.06.8 only change if new segmentation or text changed
		if ((m_bPhonetic) && ((m_nSegmentBy != IDC_KEEP)||(m_bPhoneticModified))) {

			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = m_pSaDoc->GetSegment(PHONETIC);
			pTable = m_pSaDoc->GetFont(PHONETIC);
			pSegment->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for (nIndex = 0; nIndex < (nOffsetSize-1);nIndex++) {
				switch(m_nAlignBy) {
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonetic);
					if (szNext.GetLength()==0) {
						szNext+=SEGMENT_DEFAULT_CHAR;
					}
					pSegment->Insert(pSegment->GetOffsetSize(),&szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_WORD:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) { // No more word breaks continue one character at a time
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonetic);
					} else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex]) {
						// Insert default segment character if phonetic offset does not correspond to word boundary
						szNext = SEGMENT_DEFAULT_CHAR;
					} else { // Insert Word on Gloss Boundary
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonetic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength()==0) {
						szNext+=SEGMENT_DEFAULT_CHAR;
					}
					pSegment->Insert(pSegment->GetOffsetSize(),&szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					// the line is entered one character per segment
					szNext.Empty();
					while (true) 
					{
						CSaString szTemp = pTable->GetNext( nAlignMode, nStringIndex, m_szPhonetic);
						if (szTemp.GetLength()==0) 
						{
							// end of array
							szTemp=SEGMENT_DEFAULT_CHAR;
							break;
						} else if ((szTemp.GetLength()==1)&&(szTemp[0]==EDIT_WORD_DELIMITER)) {
							// time to stop!
							break;
						} else if (szTemp.GetLength()>1)  {
							// in some situations if the trailing character is not a break
							// it will be combined with the space.  we will break it here.
							if (szTemp[0]==EDIT_WORD_DELIMITER) {
								if (szNext.GetLength()==0) {
									// remove space and append
									szTemp.Delete(0,1);
								} else {
									// backup and let the next character go into the next segment
									nStringIndex--;
									break;
								}
							}
						}
						szNext += szTemp;
					}
					pSegment->Insert( pSegment->GetOffsetSize(),&szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				}
			}

			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, m_szPhonetic);
			if (szNext.GetLength()==0) {
				szNext+=SEGMENT_DEFAULT_CHAR;
			}
			pSegment->Insert(pSegment->GetOffsetSize(),&szNext,FALSE,pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
			
			// SDM 1.06.8 apply input filter to segment
			if (pSegment->GetInputFilter()) {
				(pSegment->GetInputFilter())(*pSegment->GetString());
			}
		}

		// Process phonemic
		// SDM 1.06.8 only change  if new segmentation or text changed
		if ((m_bPhonemic) && ((m_nSegmentBy != IDC_KEEP)||(m_bPhonemicModified))) {

			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = m_pSaDoc->GetSegment(PHONEMIC);
			pTable = m_pSaDoc->GetFont(PHONEMIC);
			pSegment->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for (nIndex = 0; nIndex < (nOffsetSize-1);nIndex++) {
				switch(m_nAlignBy) {
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonemic);
					if (szNext.GetLength()!=0) {
						// Skip Empty Segments
						pSegment->Insert(pSegment->GetOffsetSize(),&szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				case IDC_WORD:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) {
						// No more word breaks continue one character at a time
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonemic);
					} else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex]) {
						// Skip if phonetic offset does not correspond to word boundary
						continue;
					} else { // Insert Word on Gloss Boundary
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szPhonemic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength()==0) {
						// Skip NULL strings
						continue; 
					}
					pSegment->Insert(pSegment->GetOffsetSize(),&szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					// the line is entered one character per segment
					szNext.Empty();
					while (true) {
						CSaString szTemp = pTable->GetNext( nAlignMode, nStringIndex, m_szPhonemic);
						if (szTemp.GetLength()==0) {
							break;
						} else if ((szTemp.GetLength()==1)&&(szTemp[0]==EDIT_WORD_DELIMITER)) {
							// time to stop!
							break;
						} else if (szTemp.GetLength()>1) {
							// in some situations if the trailing character is not a break
							// it will be combined with the space.  we will break it here.
							if (szTemp[0]==EDIT_WORD_DELIMITER) {
								if (szNext.GetLength()==0) {
									// remove space and append
									szTemp.Delete(0,1);
								} else {
									// backup and let the next character go into the next segment
									nStringIndex--;
									break;
								}
							}
						}
						szNext += szTemp;
					}
					if (szNext.GetLength()>0) {
						pSegment->Insert(pSegment->GetOffsetSize(),&szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				}
			}

			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, m_szPhonemic);
			// Skip empty segments
			if (szNext.GetLength()!=0) {
				pSegment->Insert(pSegment->GetOffsetSize(),&szNext,FALSE, pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
			}
			// SDM 1.06.8 apply input filter to segment
			if (pSegment->GetInputFilter()) {
				(pSegment->GetInputFilter())(*pSegment->GetString());
			}
		}

		// Process tone
		if (m_nSegmentBy != IDC_KEEP) {
			// SDM 1.06.8 only change  if new segmentation or text changed
			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = m_pSaDoc->GetSegment(TONE);
			pSegment->DeleteContents();
		}

		// Process orthographic
		// SDM 1.06.8 only change  if new segmentation or text changed
		if ((m_bOrthographic) && ((m_nSegmentBy != IDC_KEEP)||(m_bOrthographicModified))) {

			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = m_pSaDoc->GetSegment(ORTHO);
			pTable = m_pSaDoc->GetFont(ORTHO);
			pSegment->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for (nIndex = 0; nIndex < (nOffsetSize-1);nIndex++) {
				switch(m_nAlignBy) {
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szOrthographic);
					if (szNext.GetLength()!=0) {
						// Skip Empty Segments
						pSegment->Insert(pSegment->GetOffsetSize(),&szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				case IDC_WORD:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) {
						// No more word breaks continue one character at a time
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szOrthographic);
					} else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex]) {
						// Skip if character offset does not correspond to word boundary
						continue;
					} else {
						// Insert Word on Boundary
						szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szOrthographic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength()==0) {
						// Skip NULL words
						continue; 
					}
					pSegment->Insert(pSegment->GetOffsetSize(),&szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					// the line is entered one character per segment
					szNext.Empty();
					while (true) {
						CSaString szTemp = pTable->GetNext( nAlignMode, nStringIndex, m_szOrthographic);
						if (szTemp.GetLength()==0) {
							break;
						} else if ((szTemp.GetLength()==1)&&(szTemp[0]==EDIT_WORD_DELIMITER)) {
							// time to stop!
							break;
						} else if (szTemp.GetLength()>1) {
							// in some situations if the trailing character is not a break
							// it will be combined with the space.  we will break it here.
							if (szTemp[0]==EDIT_WORD_DELIMITER) {
								if (szNext.GetLength()==0) {
									// remove space and append
									szTemp.Delete(0,1);
								} else {
									// backup and let the next character go into the next segment
									nStringIndex--;
									break;
								}
							}
						}
						szNext += szTemp;
					}
					if (szNext.GetLength()>0) {
						pSegment->Insert(pSegment->GetOffsetSize(),&szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				}
			}

			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, m_szOrthographic);
			if (szNext.GetLength()!=0) {
				// Skip empty segments
				pSegment->Insert(pSegment->GetOffsetSize(),&szNext,FALSE,pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
			}
			
			// SDM 1.06.8 apply input filter to segment
			if (pSegment->GetInputFilter()) {
				(pSegment->GetInputFilter())(*pSegment->GetString());
			}
		}

		// Process gloss
		// SDM 1.5Test8.2 only change if text changed
		if (m_bGlossModified) {
			nStringIndex = 0;
			pSegment = m_pSaDoc->GetSegment(GLOSS);
			pTable = m_pSaDoc->GetFont(GLOSS);

			bool poundDelimited = (m_szGloss.FindOneOf(CSaString(WORD_DELIMITER))!=-1);

			// align gloss by word SDM 1.5Test8.2
			nAlignMode = CFontTable::DELIMITEDWORD;
			nOffsetSize = pArray[WORD_OFFSETS].GetSize();
			// Don't Select this segment SDM 1.5Test8.2
			pSegment->SelectSegment(*m_pSaDoc,-1);
			// the gloss table uses a space as a delimiter,
			// the normally the text is delimited with a #.
			// if we see a # in the first position, we will continue
			// to scan the segments util we
			for (nIndex = 0; nIndex < (nOffsetSize-1);nIndex++) {

				szNext = pTable->GetNext(nAlignMode, nStringIndex, m_szGloss);
				if (szNext.GetLength()==0) {
					szNext = CSaString(WORD_DELIMITER);
				} else {
					if (szNext[0]==WORD_DELIMITER) {
						// do nothing
					} else {
						szNext = CSaString(WORD_DELIMITER)+szNext;
					}
				}

				// if the user used # delimiters, then if there are
				// embedded spaces, concatenate the lines
				if (poundDelimited) {
					int nTempIndex = nStringIndex;
					bool done = false;
					while (!done) {
						int nLastIndex = nTempIndex;
						CSaString szTemp = pTable->GetNext(nAlignMode, nTempIndex, m_szGloss);
						if (szTemp.GetLength()==0) {
							if (nTempIndex==nLastIndex) {
								// we are at the end of the data
								done = true;
							} else {
								szNext += CSaString(EDIT_WORD_DELIMITER);
							}
						} else {
							if (szTemp[0]==WORD_DELIMITER) {
								// we found the next line
								nStringIndex = nLastIndex;
								done = true;
							} else {
								szNext += CSaString(EDIT_WORD_DELIMITER) + szTemp;
							}
						}
					}
				}

				szNext.Remove(0x0d);
				szNext.Remove(0x0a);
				pSegment->SelectSegment(*m_pSaDoc,nIndex);
				((CGlossSegment*)pSegment)->ReplaceSelectedSegment(m_pSaDoc,szNext);
			};

			// take care of remainder
			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, m_szGloss);
			if (szNext.GetLength()==0) {
				szNext = CSaString(WORD_DELIMITER);
			} else {
				if (szNext[0]==WORD_DELIMITER) {
					// do nothing
				} else {
					szNext = CSaString(WORD_DELIMITER)+szNext;
				}
			}
			szNext.Remove(0x0d);
			szNext.Remove(0x0a);
			pSegment->SelectSegment(*m_pSaDoc,nIndex);
			((CGlossSegment*)pSegment)->ReplaceSelectedSegment(m_pSaDoc,szNext);
		}
	}

	pView->ChangeAnnotationSelection(pSegment, -1);

	CGraphWnd *pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);
	if (pGraph) {
		if (m_bPhonetic) {		
			pGraph->ShowAnnotation(PHONETIC, TRUE, TRUE);
		}
		if (m_bPhonemic) {
			pGraph->ShowAnnotation(PHONEMIC, TRUE, TRUE);
		}
		if (m_bOrthographic) {
			pGraph->ShowAnnotation(ORTHO, TRUE, TRUE);
		}
		if (m_bGloss) {
			pGraph->ShowAnnotation(GLOSS, TRUE, TRUE);
		}
	}
	pView->RefreshGraphs(); // redraw all graphs without legend window
}

// SDM 1.06.8 Split
/***************************************************************************/
// CDlgAnnotation::OnOK Execute changes
/***************************************************************************/
void CDlgAnnotation::OnOK() {
	OK();
	CDialog::OnOK();
}

/***************************************************************************/
// CDlgAnnotation::AutoAlign Execute changes by request from batch file
/***************************************************************************/
void CDlgAnnotation::AutoAlign(const CSaString& Phonetic, 
							   const CSaString& Phonemic,
							   const CSaString& Ortho, 
							   const CSaString& Gloss,
							   CSaString& Alignment, 
							   CSaString& Segmentation)
{
	m_bPhonetic = (Phonetic.GetLength() != 0);
	m_bPhonemic = (Phonemic.GetLength() != 0);
	m_bOrthographic = (Ortho.GetLength() != 0);
	m_bGloss = (Gloss.GetLength() != 0);

	// SDM 1.5Test8.2
	m_bPhoneticModified = (m_bPhonetic!=FALSE);
	m_bPhonemicModified = (m_bPhonemic!=FALSE);
	m_bOrthographicModified = (m_bOrthographic!=FALSE);
	m_bGlossModified = (m_bGloss!=FALSE);

	if (m_bPhonetic)
	{
		m_szPhonetic = ReadFile(Phonetic);
	}
	if (m_bPhonemic)
	{
		m_szPhonemic = ReadFile(Phonemic);
	}
	if (m_bOrthographic)
	{
		m_szOrthographic = ReadFile(Ortho);
	}
	if (m_bGloss)
	{
		m_szGloss = ReadFile(Gloss);
	}

	m_nAlignBy = IDC_CHARACTER;
	Alignment.MakeUpper();
	if (Alignment=="NONE")
	{
		m_nAlignBy = IDC_NONE;
	}
	if (Alignment=="WORD")
	{
		m_nAlignBy = IDC_WORD;
	}

	m_nSegmentBy = IDC_KEEP;
	Segmentation.MakeUpper();
	if (Segmentation=="MANUAL")
	{
		m_nSegmentBy = IDC_MANUAL;
	}
	if (Segmentation=="AUTOMATIC")
	{
		m_nSegmentBy = IDC_AUTOMATIC;
	}

	OK();
}


/***************************************************************************/
// CDlgAnnotation::AutoAlign Execute changes by request from batch file
/***************************************************************************/
void CDlgAnnotation::AutoAlign(const CSaString* Phonetic, 
							   const CSaString* Phonemic,
							   const CSaString* Ortho, 
							   const CSaString* Gloss)
{
	m_bPhonetic = (Phonetic != 0);
	m_bPhonemic = (Phonemic != 0);
	m_bOrthographic = (Ortho != 0);
	m_bGloss = (Gloss != 0);

	m_bPhoneticModified = (m_bPhonetic!=FALSE);
	m_bPhonemicModified = (m_bPhonemic!=FALSE);
	m_bOrthographicModified = (m_bOrthographic!=FALSE);
	m_bGlossModified = (m_bGloss!=FALSE);

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
