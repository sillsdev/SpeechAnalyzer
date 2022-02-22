/////////////////////////////////////////////////////////////////////////////
// fileInformation.cpp:
// Implementation of the CDlgInformationFilePage (property page)
//                       CDlgInformationWavePage (property page)
//                       CDlgInformationSourcePage (property page)
//                       CDlgInformationUserPage (property page)
//                       CDlgFileInformation (property sheet)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg2.cpp
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "fileInformation.h"
#include "Segment.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "TextSegment.h"
#include "SaParam.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CDlgInformationFilePage property page
// Displays information about the wave file containing this document.

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationFilePage message map

BEGIN_MESSAGE_MAP(CDlgInformationFilePage, CPropertyPage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationFilePage construction/destruction/creation

/***************************************************************************/
// CDlgInformationFilePage::CDlgInformationFilePage Constructor
/***************************************************************************/
CDlgInformationFilePage::CDlgInformationFilePage() : CPropertyPage(CDlgInformationFilePage::IDD) {
}

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationFilePage helper functions

/***************************************************************************/
// CDlgInformationFilePage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgInformationFilePage::DoDataExchange(CDataExchange * pDX) {
    CPropertyPage::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationFilePage message handlers

/***************************************************************************/
// CDlgInformationFilePage::OnInitDialog Dialog initialization
// All the necessary informations are put into the dialog controls.
/***************************************************************************/
BOOL CDlgInformationFilePage::OnInitDialog() {

    CPropertyPage::OnInitDialog();
    // get pointer to document
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    CSaView * pView = pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
    // get file status document member data
    CFileStatus * pFileStatus = pModel->GetFileStatus();
	// file name is defined
    if (pFileStatus->m_szFullName[0] != 0) { 
        // copy full path name
        CString szFilePath = pFileStatus->m_szFullName;
        // get dc and textmetrics of static text control
        CWnd * pWnd = GetDlgItem(IDC_FILENAME);
		// device context
        CDC * pDC = pWnd->GetDC();
		// select actual font
        CFont * oldFont = pDC->SelectObject(pWnd->GetFont()); 
        TEXTMETRIC tm;
        pDC->GetTextMetrics(&tm);
		// set back old font
        pDC->SelectObject(oldFont);  
        pWnd->ReleaseDC(pDC);
        // get coordinates of static text control
        CRect rWnd;
        pWnd->GetClientRect(rWnd);
        // calculate number of characters possible to display and limit the string
		// experience values
        int nChars = (rWnd.right / tm.tmAveCharWidth * 8 / 10); 
		// file path is too long
        if (szFilePath.GetLength() > nChars) { 
            CSaString szRightPath = szFilePath.Right(nChars - 6);
            szRightPath = szRightPath.Right(szRightPath.GetLength() - szRightPath.Find(_T("\\")));
			// drive...rest
            szFilePath = szFilePath.Left(3) + "..." + szRightPath; 
        }
        // draw the file path
		// write file name
        SetDlgItemText(IDC_FILENAME, szFilePath); 
		// if time defined write creation time
        if (pFileStatus->m_ctime != 0) { 
            SetDlgItemText(IDC_FILEDATE, pFileStatus->m_ctime.Format("%A, %B %d, %Y, %X"));
        }
		// if time defined write last edit time
        if (pFileStatus->m_mtime != 0) { 
            SetDlgItemText(IDC_EDITDATE, pFileStatus->m_mtime.Format("%A, %B %d, %Y, %X"));
        }
		// create and write size text
        TCHAR szBuffer[32]; 
        swprintf_s(szBuffer, _T("%lld Bytes"), pFileStatus->m_size);
        SetDlgItemText(IDC_FILESIZE, szBuffer);

        // get sa parameters document member data
        if (pModel->IsValidRecordFileFormat()) {
			// load and write file type string
			CString szTemp; 
            szTemp.LoadString((UINT)pModel->GetRecordFileFormat() + IDS_FILETYPE_UTT);
            SetDlgItemText(IDC_FILEFORMAT, szTemp);
        }
    }
    return TRUE;
}

//###########################################################################
// CDlgInformationWavePage property page
// Displays information about the waveform parameters in the wave file of
// this document.

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationWavePage message map

BEGIN_MESSAGE_MAP(CDlgInformationWavePage, CPropertyPage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationWavePage construction/destruction/creation

/***************************************************************************/
// CDlgInformationWavePage::CDlgInformationWavePage Constructor
/***************************************************************************/
CDlgInformationWavePage::CDlgInformationWavePage() : CPropertyPage(CDlgInformationWavePage::IDD) {
}

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationWavePage helper functions

/***************************************************************************/
// CDlgInformationWavePage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgInformationWavePage::DoDataExchange(CDataExchange * pDX) {
    CPropertyPage::DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationWavePage message handlers

/***************************************************************************/
// CDlgInformationWavePage::OnInitDialog Dialog initialization
// All the necessary informations are put into the dialog controls.
/***************************************************************************/
BOOL CDlgInformationWavePage::OnInitDialog() {

    CPropertyPage::OnInitDialog();
    // get pointer to document
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    CSaView * pView = pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pModel = (CSaDoc *)pView->GetDocument();

    // get sa parameters document member data
    TCHAR szBuffer[32];
    if (pModel->GetNumberOfSamples() > 0L) { 
		// there is an sa chunk
        if (pModel->GetSignalBandWidth() > 0) {
            // create and write bandwith text
            swprintf_s(szBuffer, _T("%ld Hz"), pModel->GetSignalBandWidth());
            SetDlgItemText(IDC_BANDWIDTH, szBuffer);
            // create and write highpass status text
            swprintf_s(szBuffer, _T("%s"), pModel->IsUsingHighPassFilter() ? _T("Yes"):_T("No"));
            SetDlgItemText(IDC_HPFSTATUS, szBuffer);
        }
        if (pModel->GetQuantization()>0) {
            // create and write quantization size text
            swprintf_s(szBuffer, _T("%u Bits"), pModel->GetQuantization());
            SetDlgItemText(IDC_SAMPLESIZE, szBuffer);
        }
    }

    // create and write length text
	// get sampled data size in seconds
    double fDataSec = pModel->GetTimeFromBytes(pModel->GetDataSize());  
    int nMinutes = (int)fDataSec / 60;
	// length is less than one minute
    if (nMinutes == 0) { 
        swprintf_s(szBuffer, _T("%5.3f Seconds"), fDataSec);
    } else { 
		// length is equal ore more than one minute
        fDataSec = fDataSec - (float)(nMinutes * 60);
        swprintf_s(szBuffer, _T("%i:%5.3f (Min:Sec)"), nMinutes, fDataSec);
    }
    SetDlgItemText(IDC_FILELENGTH, szBuffer);
    // get sa parameters format member data
    // create and write number of samples text
    swprintf_s(szBuffer, _T("%ld"), pModel->GetNumSamples());
    SetDlgItemText(IDC_NUMBERSAMPLES, szBuffer);
    // create and write sample rate text
    swprintf_s(szBuffer, _T("%ld Hz"), pModel->GetSamplesPerSec());
    SetDlgItemText(IDC_SAMPLERATE, szBuffer);
    // create and write sample format text
    CString szChannels;
    if (pModel->GetNumChannels()==1) {
        szChannels = "Mono";
    } else if (pModel->GetNumChannels()==2) {
        szChannels = "Stereo";
    } else {
        char buffer[256];
        sprintf_s(buffer,_countof(buffer),"%d Channels",pModel->GetNumChannels());
        szChannels = buffer;
    }
    CString szFormat;
    szFormat.Format(_T("%u Bits %s"), pModel->GetBitsPerSample(), szChannels.GetString());
    SetDlgItemText(IDC_SAMPLEFORMAT, szFormat);
    // create and write number of phones text
    int nNumber = 0;
    int nLoop = 0;
    if (pModel->GetSegment(PHONETIC)->GetContentLength() > 0) {
        // find number of phones
        while (TRUE) {
            nLoop++;
            nNumber = pModel->GetSegment(PHONETIC)->GetNext(nNumber);
            if (nNumber < 0) {
                break;
            }
        }
    }
    swprintf_s(szBuffer, _T("%u"), nLoop);
    SetDlgItemText(IDC_NUMPHONES, szBuffer);
    // create and write number of words text
    // SDM 1.06.6U2
    swprintf_s(szBuffer, _T("%u"), ((CTextSegment *)pModel->GetSegment(GLOSS))->CountWords());
    SetDlgItemText(IDC_NUMWORDS, szBuffer);
    return TRUE;
}

//###########################################################################
// CDlgInformationSourcePage property page
// Displays the users source information stored in this wave file.

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationSourcePage message map

BEGIN_MESSAGE_MAP(CDlgInformationSourcePage, CPropertyPage)
    ON_EN_UPDATE(IDC_ETHNOID, OnChangeEthnoid)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationSourcePage construction/destruction/creation

/***************************************************************************/
// CDlgInformationSourcePage::CDlgInformationSourcePage Constructor
/***************************************************************************/
CDlgInformationSourcePage::CDlgInformationSourcePage() : CPropertyPage(CDlgInformationSourcePage::IDD) {
    m_szTranscriber = "";

    // get pointer to document  SDM 1.5Test10.4
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    CSaView * pView = pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pModel = (CSaDoc *)pView->GetDocument();
    // get source parameters document member data
    SourceParm * pSourceParm = pModel->GetSourceParm();
    m_szCountry = (pSourceParm->szCountry);
    m_szDialect = pSourceParm->szDialect;
    m_szEthnoID = pSourceParm->szEthnoID;
    m_szFamily = pSourceParm->szFamily;
    m_szLanguage = pSourceParm->szLanguage;
    m_nGender = pSourceParm->nGender;
    m_szRegion = pSourceParm->szRegion;
    m_szSpeaker = pSourceParm->szSpeaker;
    m_szReference = pSourceParm->szReference;
    m_szTranscriber = pSourceParm->szTranscriber;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationSourcePage helper functions

/***************************************************************************/
// CDlgInformationSourcePage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgInformationSourcePage::DoDataExchange(CDataExchange * pDX) {
    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_COUNTRY, (CString &)m_szCountry);
    DDV_MaxChars(pDX, m_szCountry, 255);
    DDX_Text(pDX, IDC_DIALECT, m_szDialect);
    DDV_MaxChars(pDX, m_szDialect, 255);
    DDX_Text(pDX, IDC_ETHNOID, m_szEthnoID);
    DDX_Text(pDX, IDC_FAMILY, m_szFamily);
    DDV_MaxChars(pDX, m_szFamily, 255);
    DDX_Text(pDX, IDC_LANGUAGE, m_szLanguage);
    DDV_MaxChars(pDX, m_szLanguage, 255);
    DDX_Radio(pDX, IDC_MALE, m_nGender);
    DDX_Text(pDX, IDC_REGION, m_szRegion);
    DDV_MaxChars(pDX, m_szRegion, 255);
    DDX_Text(pDX, IDC_SPEAKER, m_szSpeaker);
    DDV_MaxChars(pDX, m_szSpeaker, 255);
    DDX_Text(pDX, IDC_REFERENCE, m_szReference);
    DDV_MaxChars(pDX, m_szReference, 255);
    DDX_Text(pDX, IDC_TRANSCRIBER, m_szTranscriber);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationSourcePage message handlers

/***************************************************************************/
// CDlgInformationSourcePage::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgInformationSourcePage::OnInitDialog() {
    CPropertyPage::OnInitDialog();
    UpdateData(FALSE);
    // SDM 1.5Test10.4
    CEdit * pWnd = (CEdit *) GetDlgItem(IDC_LANGUAGE);
    if (pWnd!=NULL) {
        pWnd->SetFocus();
        pWnd->SetSel(0,0);
    }
    return FALSE;
}

/***************************************************************************/
// CDlgInformationSourcePage::OnOK  File info update
/***************************************************************************/
void CDlgInformationSourcePage::OnOK() {
    CDialog::OnOK();
}

void CDlgInformationSourcePage::OnChangeEthnoid() {
    UpdateData(TRUE);
    int nLeft;
    int nRight;
    ((CEdit *)GetDlgItem(IDC_ETHNOID))->GetSel(nLeft,nRight);
    if (m_szEthnoID.GetLength()>3) {
        m_szEthnoID = m_szEthnoID.Left(3);
        UpdateData(FALSE);
        if (nLeft > 3) {
            nLeft = 3;
        }
        if (nRight > 3) {
            nRight = 3;
        }
        ((CEdit *)GetDlgItem(IDC_ETHNOID))->SetSel(nLeft,nRight);
    }
}

//###########################################################################
// CDlgInformationUserPage property page
// Displays the user comments stored in this wave file.

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationUserPage message map

BEGIN_MESSAGE_MAP(CDlgInformationUserPage, CPropertyPage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationUserPage construction/destruction/creation

/***************************************************************************/
// CDlgInformationUserPage::CDlgInformationUserPage Constructor
/***************************************************************************/
CDlgInformationUserPage::CDlgInformationUserPage() : CPropertyPage(CDlgInformationUserPage::IDD) {
    m_szFreeTranslation = "";
}

/***************************************************************************/
// CDlgInformationUserPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgInformationUserPage::DoDataExchange(CDataExchange * pDX) {
    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_FILEDESC, m_szFileDesc);
    DDV_MaxChars(pDX, m_szFileDesc, 255);
    DDX_Text(pDX, IDC_FREE_TRANSLATION, m_szFreeTranslation);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgInformationUserPage message handlers

/***************************************************************************/
// CDlgInformationUserPage::OnInitDialog Dialog initialization
// This property page contains a special window to display the transcription.
// This window is created and placed over the corresponding place holder in
// the dialog.
/***************************************************************************/
BOOL CDlgInformationUserPage::OnInitDialog() {
    CPropertyPage::OnInitDialog();
    // build and place the transcription display window
    m_TranscriptionDisp.Init(IDC_PHONETIC, this);
    // SDM 1.5Test10.4
    CEdit * pWnd = (CEdit *) GetDlgItem(IDC_FREE_TRANSLATION);
    if (pWnd!=NULL) {
        pWnd->SetFocus();
        pWnd->SetSel(0,0);
    }
    return TRUE;
}

//###########################################################################
// CDlgFileInformation property sheet
// Displays all the important information about the wave file of the actually
// opened document.

IMPLEMENT_DYNAMIC(CDlgFileInformation, CPropertySheet)

/////////////////////////////////////////////////////////////////////////////
// CDlgFileInformation message map

BEGIN_MESSAGE_MAP(CDlgFileInformation, CPropertySheet)
    ON_WM_CREATE()
    ON_COMMAND(IDHELP, OnHelpInformation)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFileInformation construction/destruction/creation

/***************************************************************************/
// CDlgFileInformation::CDlgFileInformation Constructor
/***************************************************************************/
CDlgFileInformation::CDlgFileInformation(LPCTSTR pszCaption, CWnd * pParent, UINT iSelectPage, BOOL bRecorder)
    : CPropertySheet(pszCaption, pParent, iSelectPage) {
    // add the property sheet pages
    if (!bRecorder) {
        AddPage(&m_dlgFilePage);
        AddPage(&m_dlgWavePage);
    }
    AddPage(&m_dlgSourcePage);
    AddPage(&m_dlgUserPage);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgFileInformation helper functions

/***************************************************************************/
// CDlgFileInformation::DoDataExchange Data exchange
/***************************************************************************/
void CDlgFileInformation::DoDataExchange(CDataExchange * pDX) {
    CPropertySheet::DoDataExchange(pDX);
}

/***************************************************************************/
// CDlgFileInformation::ChangeButtons
// This property sheet never uses an Apply button.
// The Apply button is deleted and a Help button inserted in its place.
/***************************************************************************/
void CDlgFileInformation::ChangeButtons() {
	// get pointers to the button objects
    CWnd * pWndApply = GetDlgItem(ID_APPLY_NOW); 
    CWnd * pWndOK = GetDlgItem(IDOK);
    CRect rButton;
	// get coordinates of apply button
    pWndApply->GetWindowRect(rButton); 
    ScreenToClient(rButton);
    m_cHelp.Create(_T("&Help"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, rButton, this, IDHELP);
    m_cHelp.SetFont(pWndOK->GetFont());
	// delete apply button (not needed)
    pWndApply->DestroyWindow(); 
}

/////////////////////////////////////////////////////////////////////////////
// CDlgFileInformation message handlers

/***************************************************************************/
// CDlgFileInformation::OnCreate Dialog creation
/***************************************************************************/
int CDlgFileInformation::OnCreate(LPCREATESTRUCT lpCreateStruct) {

	if (CPropertySheet::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    return 0;
}

/***************************************************************************/
// CDlgFileInformation::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgFileInformation::OnInitDialog() {

	CPropertySheet::OnInitDialog();
    // delete apply button and move cancel and ok buttons
    ChangeButtons();
    return TRUE;
}

/***************************************************************************/
// CDlgFileInformation::OnHelpInformation Call Information help
/***************************************************************************/
void CDlgFileInformation::OnHelpInformation() {

	// create the pathname
    long nActiveIndex = GetActiveIndex();
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/File/Information/";
    switch (nActiveIndex) {
    case 0:
        szPath += "Information_overview.htm";
        break;
    case 1:
        szPath += "Data_tab_File_Information.htm";
        break;
    case 2:
        szPath += "Source_tab_File_Information.htm";
        break;
    case 3:
        szPath += "Comments_tab_File_Information.htm";
        break;
    default:
        szPath += "Information_overview.htm";
    }
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}
