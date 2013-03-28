/////////////////////////////////////////////////////////////////////////////
// DlgAnnotationImport.cpp:
// Implementation of the CDlgImportSFM class.
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
#include "Segment.h"
#include "sa_wbch.h"
#include "sa.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_graph.h"
#include "mainfrm.h"
#include "doclist.h"
#include "dlgimportsfm.h"
#include "TranscriptionDataSettings.h"
#include "settings\obstream.h"
#include "Process\Process.h"
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
#include "dsp\formants.h"
#include "dsp\ztransform.h"
#include "dsp\mathx.h"

using std::ifstream;
using std::ios;
using std::streampos;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgImportSFM dialog
static const char * psz_Phonemic = "pm";
static const char * psz_Gloss = "gl";
static const char * psz_Phonetic = "ph";
static const char * psz_Orthographic = "or";

CDlgImportSFM::CDlgImportSFM(BOOL bPhonetic, BOOL bPhonemic, BOOL bOrtho, CWnd * pParent /*=NULL*/)
    : CDialog(CDlgImportSFM::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDlgImportSFM)
    m_bGloss = FALSE;
    m_bPhonemic = bPhonemic;
    m_bPhonetic = bPhonetic;
    m_bOrthographic = bOrtho;
    m_szPhonemic = psz_Phonemic;
    m_szGloss = psz_Gloss;
    m_szPhonetic = psz_Phonetic;
    m_szOrthographic = psz_Orthographic;
    //}}AFX_DATA_INIT
}

void CDlgImportSFM::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgImportSFM)
    DDX_Check(pDX, IDC_GLOSS_ENABLED, m_bGloss);
    DDX_Check(pDX, IDC_PHONEMIC_ENABLED, m_bPhonemic);
    DDX_Check(pDX, IDC_PHONETIC_ENABLED, m_bPhonetic);
    DDX_Check(pDX, IDC_ORTHO_ENABLED, m_bOrthographic);
    DDX_Text(pDX, IDC_GLOSS, m_szGloss);
    DDX_Text(pDX, IDC_PHONEMIC, m_szPhonemic);
    DDX_Text(pDX, IDC_PHONETIC, m_szPhonetic);
    DDX_Text(pDX, IDC_ORTHOGRAPHIC, m_szOrthographic);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgImportSFM, CDialog)
    //{{AFX_MSG_MAP(CDlgImportSFM)
    ON_BN_CLICKED(IDC_IMPORT_PLAIN_TEXT, OnImportPlainText)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/***************************************************************************/
// CDlgAnnotation::SetEnable Enables/Disables controls
/***************************************************************************/
void CDlgImportSFM::SetEnable(int nItem, BOOL bEnable)
{
    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd)
    {
        pWnd->EnableWindow(bEnable);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgImportSFM message handlers

/***************************************************************************/
// CDlgAnnotation::OnImportPlainText Plain Text Button hit
/***************************************************************************/
void CDlgImportSFM::OnImportPlainText()
{
    EndDialog(IDC_IMPORT_PLAIN_TEXT);
}


/***************************************************************************/
// CDlgAnnotation::OnInitDialog
/***************************************************************************/
BOOL CDlgImportSFM::OnInitDialog()
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
