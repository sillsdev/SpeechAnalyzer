#include "stdafx.h"
#include "DlgRecorderOptions.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "fileInformation.h"
#include "sa_view.h"
#include "sa.h"
#include "mainfrm.h"
#include "FileUtils.h"
#include "WaveOutDevice.h"
#include "WaveInDevice.h"
#include "DlgFnKeys.h"
#include "objectostream.h"
#include "Sa_Doc.h"

//###########################################################################
// CDlgRecorderOptions dialog
// Displays the controls to change recorder options.

BEGIN_MESSAGE_MAP(CDlgRecorderOptions, CDialog)
    ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
    ON_BN_CLICKED(IDC_SOURCE, OnSource)
    ON_BN_CLICKED(IDC_SOURCE_AUTO, OnSourceAuto)
    ON_COMMAND(IDHELP, OnHelpRecorderOptions)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRecorderOptions construction/destruction/creation

CDlgRecorderOptions::CDlgRecorderOptions(CDlgRecorder * pParent) : CDialog(CDlgRecorderOptions::IDD, pParent), m_pRecorder(pParent) {
    m_bSourceAuto = CDlgRecorder::GetStaticSourceInfo().bEnable;
    m_nMode = 0;
    m_nRate = -1;
    m_nBits = -1;
    m_bHighpass = TRUE;
}

/***************************************************************************/
// CDlgRecorderOptions::DoDataExchange Data exchange
/***************************************************************************/
void CDlgRecorderOptions::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_11KHZ, m_nRate);
    DDX_Radio(pDX, IDC_8BIT, m_nBits);
    DDX_Check(pDX, IDC_HIGHPASS, m_bHighpass);
    DDX_Radio(pDX, IDC_MONO, m_nMode);
    DDX_Check(pDX, IDC_SOURCE_AUTO, m_bSourceAuto);
}

/***************************************************************************/
// CDlgRecorderOptions::OnInitDialog Dialog initialization
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CDlgRecorderOptions::OnInitDialog() {
    CDialog::OnInitDialog();
    CenterWindow(); // center dialog on recorder window
    return TRUE;
}

/***************************************************************************/
// CDlgRecorderOptions::OnDefault Default button hit
/***************************************************************************/
void CDlgRecorderOptions::OnDefault() {
    // set defaults
    m_nRate = 1;
    m_nBits = 1;
    m_bHighpass = TRUE;
    m_nMode = 0;
    UpdateData(FALSE);
}

/***************************************************************************/
// CDlgRecorderOptions::SetSamplingRate(int nRate)
/***************************************************************************/
void CDlgRecorderOptions::SetSamplingRate(int nRate) {
    switch (nRate) {
    case 11025:
        m_nRate = 0;
        break;
    case 22050:
        m_nRate = 1;
        break;
    case 44100:
        m_nRate = 2;
        break;
    default:
        ASSERT(FALSE);
        m_nRate = -1;
    }
}

/***************************************************************************/
// CDlgRecorderOptions::GetSamplingRate()
/***************************************************************************/
unsigned int CDlgRecorderOptions::GetSamplingRate() {
    int nRate = 22050;

    switch (m_nRate) {
    case 0:
        nRate = 11025;
        break;
    case 1:
        nRate = 22050;
        break;
    case 2:
        nRate = 44100;
        break;
    }
    return nRate;
}

/***************************************************************************/
// CDlgRecorderOptions::SetBitDepth(int nBits)
/***************************************************************************/
void CDlgRecorderOptions::SetBitDepth(int nBits) {
    switch (nBits) {
    case 8:
        m_nBits = 0;
        break;
    case 16:
        m_nBits = 1;
        break;
    default:
        ASSERT(FALSE);
        m_nBits = -1;
    }
}

/***************************************************************************/
// CDlgRecorderOptions::GetBitDepth()
/***************************************************************************/
short int CDlgRecorderOptions::GetBitDepth() {
    short int nBits = 16;
    switch (m_nBits) {
    case 0:
        nBits = 8;
        break;
    case 1:
        nBits = 16;
    }
    return nBits;
}

/***************************************************************************/
// CDlgRecorderOptions::SetHighpass(BOOL bHighpass)
/***************************************************************************/
void CDlgRecorderOptions::SetHighpass(BOOL bHighpass) {
    m_bHighpass = bHighpass;
}

/***************************************************************************/
// CDlgRecorderOptions::GetHighpass()
/***************************************************************************/
BOOL CDlgRecorderOptions::GetHighpass() {
    return m_bHighpass;
}

/***************************************************************************/
// CDlgRecorderOptions::SetChannels(int nChannels)
/***************************************************************************/
void CDlgRecorderOptions::SetChannels(int nChannels) {
    UNUSED(nChannels);
    ASSERT(nChannels == 1);
}
/***************************************************************************/
// CDlgRecorderOptions::GetChannels()
/***************************************************************************/
short CDlgRecorderOptions::GetChannels() {
    return 1;
}

/***************************************************************************/
// CDlgRecorderOptions::GetRecorder()
/***************************************************************************/
CDlgRecorder & CDlgRecorderOptions::GetRecorder() {
    return *m_pRecorder;
}

void CDlgRecorderOptions::OnSourceAuto() {
    UpdateData(TRUE);
    CDlgRecorder::GetStaticSourceInfo().bEnable = m_bSourceAuto;
}

/***************************************************************************/
// CDlgRecorderOptions::OnSource
/***************************************************************************/
void CDlgRecorderOptions::OnSource() {
    UpdateData(TRUE);
    CSaDoc * pModel = GetRecorder().GetDocument();
    SourceParm * pSourceParm = pModel->GetSourceParm();
    CDlgRecorder::sourceInfo & m_source = CDlgRecorder::GetStaticSourceInfo();
    if (m_source.bEnable) {
        pSourceParm->szCountry = m_source.source.szCountry;
        pSourceParm->szDialect = m_source.source.szDialect;
        pSourceParm->szEthnoID = m_source.source.szEthnoID.Left(3);
        pSourceParm->szFamily = m_source.source.szFamily;
        pSourceParm->szLanguage = m_source.source.szLanguage;
        pSourceParm->nGender  = m_source.source.nGender;
        pSourceParm->szRegion = m_source.source.szRegion;
        pSourceParm->szSpeaker = m_source.source.szSpeaker;
        pSourceParm->szReference = m_source.source.szReference;
        pSourceParm->szTranscriber = m_source.source.szTranscriber;
        pSourceParm->szDescription = m_source.source.szDescription;
        pSourceParm->szFreeTranslation = m_source.source.szFreeTranslation;
    }
    CString szCaption;
    szCaption.LoadString(IDS_DLGTITLE_FILEINFO);                 // load caption string
    CDlgFileInformation dlg(szCaption, NULL, 0, TRUE);  // create the property sheet
    // set file description string
    dlg.m_dlgUserPage.m_szFileDesc = pSourceParm->szDescription;
    dlg.m_dlgUserPage.m_szFreeTranslation = pSourceParm->szFreeTranslation;
    if (dlg.DoModal() == IDOK) {
        // get new file description string
        pModel->SetDescription(dlg.m_dlgUserPage.m_szFileDesc);
        pSourceParm->szDescription = dlg.m_dlgUserPage.m_szFileDesc;
        pSourceParm->szFreeTranslation = dlg.m_dlgUserPage.m_szFreeTranslation;
        pSourceParm->szCountry = dlg.m_dlgSourcePage.m_szCountry;
        pSourceParm->szDialect = dlg.m_dlgSourcePage.m_szDialect;
        if (dlg.m_dlgSourcePage.m_szEthnoID.GetLength() < 3) {
            dlg.m_dlgSourcePage.m_szEthnoID += "   ";
        }
        pSourceParm->szEthnoID = dlg.m_dlgSourcePage.m_szEthnoID.Left(3);
        pSourceParm->szFamily = dlg.m_dlgSourcePage.m_szFamily;
        pSourceParm->szLanguage = dlg.m_dlgSourcePage.m_szLanguage;
        pSourceParm->nGender = dlg.m_dlgSourcePage.m_nGender;
        pSourceParm->szRegion = dlg.m_dlgSourcePage.m_szRegion;
        pSourceParm->szSpeaker = dlg.m_dlgSourcePage.m_szSpeaker;
        pSourceParm->szReference = dlg.m_dlgSourcePage.m_szReference;
        pSourceParm->szTranscriber = dlg.m_dlgSourcePage.m_szTranscriber;

        m_source.source.szCountry = dlg.m_dlgSourcePage.m_szCountry;
        m_source.source.szDialect = dlg.m_dlgSourcePage.m_szDialect;
        m_source.source.szEthnoID = dlg.m_dlgSourcePage.m_szEthnoID.Left(3);
        m_source.source.szFamily = dlg.m_dlgSourcePage.m_szFamily;
        m_source.source.szLanguage = dlg.m_dlgSourcePage.m_szLanguage;
        m_source.source.nGender = dlg.m_dlgSourcePage.m_nGender;
        m_source.source.szRegion = dlg.m_dlgSourcePage.m_szRegion;
        m_source.source.szSpeaker = dlg.m_dlgSourcePage.m_szSpeaker;
        m_source.source.szReference = dlg.m_dlgSourcePage.m_szReference;
        m_source.source.szTranscriber = dlg.m_dlgSourcePage.m_szTranscriber;
        m_source.source.szFreeTranslation = dlg.m_dlgUserPage.m_szFreeTranslation;
        m_source.source.szDescription = dlg.m_dlgUserPage.m_szFileDesc;
    }
}

/***************************************************************************/
// CDlgRecorderOptions::OnHelpRecorderOptions Call Recorder Settings help
/***************************************************************************/
void CDlgRecorderOptions::OnHelpRecorderOptions() {
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/File/Recorder_Settings.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}
