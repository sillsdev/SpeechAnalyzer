/////////////////////////////////////////////////////////////////////////////
// sa_edit.cpp:
// Implementation of the CDlgEditor (dialog)
//                       CDlgAnnotationEdit (dialog)           classes.
//
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.06.5
//      SDM Original
// 1.06.6
//      SDM Added CDlgAnnotationEdit Class
// 1.06.6U2
//      SDM Changed CDlgEditor to auto hide on Empty documents
// 1.06.6U4
//      SDM Added OnCancel to undo changes on ESC (Alt+F4)
// SDM 1.5Test8.2
//      SDM Save TE position in preferences
//      SDM fix bug on close of TE which cancel previous change
// 1.5Test8.5
//      SDM CDlgEditor removed autohide restore
//      SDM changed CDlgEditor::PreTranslateMsg to use document accelerator table
// 1.5Test10.0
//      SDM fix bug on close of TE which cancel previous change & ESC would not cancel see 1.06.6U4 & 1.5Test8.2
// 1.5Test11.0
//      SDM replaced GetOffset() + GetDuration() with CSegment::GetStop()
//      SDM replaced GetOffsets()->GetSize() with GetSize()
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_edit.h"
#include "Process\Process.h"
#include "Segment.h"

#include "ch_dlg.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "DlgPlayer.h"
#include "sa_ipa.h"

//###########################################################################
// CDlgEditor dialog

/////////////////////////////////////////////////////////////////////////////
// CDlgEditor message map

BEGIN_MESSAGE_MAP(CDlgEditor, CDialog)
    ON_BN_CLICKED(IDC_CHARACTERCHART, OnCharacterChart)
    ON_BN_CLICKED(IDC_PLAY_SEGMENT, OnPlaybackSegment)
    ON_BN_CLICKED(IDC_PLAY_WORD, OnPlaybackWord)
    ON_BN_CLICKED(IDC_STOP_WORD, OnStopWord)
    ON_BN_CLICKED(IDC_PLAY_PHRASE_L1, OnPlaybackPhraseL1)
    ON_BN_CLICKED(IDC_STOP_PHRASE_L1, OnStopPhraseL1)
    ON_EN_UPDATE(IDC_INPUTSTRING, OnUpdateInputstring)
    ON_WM_DESTROY()
    ON_WM_ACTIVATE()
    ON_WM_PAINT()
    ON_WM_CLOSE()
    ON_COMMAND(IDHELP, OnHelpEdit)
    ON_COMMAND(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditor construction/destruction/creation

/***************************************************************************/
// CDlgEditor::CDlgEditor Constructor
/***************************************************************************/
CDlgEditor::CDlgEditor(CWnd * pParent) : CDialog(CDlgEditor::IDD, pParent) {
    bEditor = FALSE;
    m_pPreviousSaView = NULL;
    m_bActivated = FALSE;
}

BOOL CDlgEditor::OnCmdMsg(UINT nID, int nCode, void * pExtra, AFX_CMDHANDLERINFO * pHandlerInfo) {
    if (!CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo)) {
        return GetMainFrame()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgEditor helper functions

// SDM 1.5Test8.2
/***************************************************************************/
// CDlgEditor::CreateSafe Create dialog load save position
/***************************************************************************/
BOOL CDlgEditor::CreateSafe(UINT nIDTemplate, CWnd * pParentWnd, WINDOWPLACEMENT * pWPL) {
    if (!bEditor) {
        bEditor = CDialog::Create(nIDTemplate, pParentWnd);
        if ((pWPL!=NULL) && pWPL->length) {
            SetWindowPos(NULL,pWPL->rcNormalPosition.left,pWPL->rcNormalPosition.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        }
    }
    return bEditor;
};

/***************************************************************************/
// CDlgEditor::OnInitDialog Dialog initialisation
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CDlgEditor::OnInitDialog() {
    CDialog::OnInitDialog();

    CenterWindow(); // center dialog on frame window

    // Load button icons
    m_NextButton.SetIcon((HICON)LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDI_NEXT),RT_GROUP_ICON),MAKEINTRESOURCE(IDI_NEXT),IMAGE_ICON,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
    m_PreviousButton.SetIcon((HICON)LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDI_PREV),RT_GROUP_ICON),MAKEINTRESOURCE(IDI_PREV),IMAGE_ICON,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
    m_UpButton.SetIcon((HICON)LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDI_UP),RT_GROUP_ICON),MAKEINTRESOURCE(IDI_UP),IMAGE_ICON,0,0,LR_LOADMAP3DCOLORS));
    m_DownButton.SetIcon((HICON)LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDI_DOWN),RT_GROUP_ICON),MAKEINTRESOURCE(IDI_DOWN),IMAGE_ICON,17,15,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));

    m_NextButton.SetFocus();

    CMainFrame * pMainWnd = GetMainFrame();
    bool playing = false;
    if (pMainWnd!=NULL) {
        playing = (pMainWnd->IsPlayerPlaying())?true:false;
    }
    SetButtonState(IDC_PLAY_WORD,IDC_STOP_WORD,false);
    SetButtonState(IDC_PLAY_PHRASE_L1,IDC_STOP_PHRASE_L1,playing);

    return FALSE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgEditor::DoDataExchange Data exchange
/***************************************************************************/
void CDlgEditor::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, ID_EDIT_NEXT, m_NextButton);
    DDX_Control(pDX, ID_EDIT_PREVIOUS, m_PreviousButton);
    DDX_Control(pDX, ID_EDIT_UP, m_UpButton);
    DDX_Control(pDX, ID_EDIT_DOWN, m_DownButton);
}

/***************************************************************************/
// CDlgEditor::SaView Get a pointer to the active CSaView if any
/***************************************************************************/
CSaView * CDlgEditor::GetView() const {
    CSaView * pSaView = GetMainFrame()->GetCurrSaView();

    if ((pSaView!=NULL) && (!pSaView->IsKindOf(RUNTIME_CLASS(CSaView)))) {
        pSaView = NULL;    // Verify View is a CSaView type.
    }

    return pSaView;
}

CSaDoc * CDlgEditor::GetDoc() const {
    return (GetView()!=NULL) ? GetView()->GetDocument() : NULL;
};

CSegment * CDlgEditor::GetSelectedSegment() const {
    return (GetView()!=NULL) ? GetView()->FindSelectedAnnotation() : NULL;
};

/***************************************************************************/
// CDlgEditor::OnCharacterChart Input from character chart wanted
/***************************************************************************/
void CDlgEditor::OnCharacterChart() {
    // create the character chart property sheet
    CString szCaption;
    szCaption.LoadString(IDS_DLGTITLE_CHARCHART); // load caption string
    CDlgCharChart * pDlgCharChart;
    pDlgCharChart = new CDlgCharChart(szCaption, NULL, 0); // create the property sheet
    pDlgCharChart->SetInitialState(GetDlgItem(IDC_INPUTSTRING)->GetFont());
    if (pDlgCharChart->DoModal() == IDOK) {
        // insert the new string
        ((CEdit *)GetDlgItem(IDC_INPUTSTRING))->ReplaceSel(pDlgCharChart->m_szString);
        UpdateData(TRUE);
    }
    if (pDlgCharChart) {
        delete pDlgCharChart;
    }
    GotoDlgCtrl(GetDlgItem(IDC_INPUTSTRING)); // set focus to edit control
}

/***************************************************************************/
// CDlgEditor::OnPlaybackSegment Button playback segment hit
// Sends a message to the player to playback the current segment selection.
/***************************************************************************/
void CDlgEditor::OnPlaybackSegment() {
    if (!GetView()) {
        return;    // require doc and view
    }
    GetView()->OnPlaybackSegment();

    // display pending error messages
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    pApp->DisplayMessages();
}

/***************************************************************************/
// CDlgEditor::OnPlaybackWord Button playback word hit
// Sends a message to the player to playback the original gloss selection.
/***************************************************************************/
void CDlgEditor::OnPlaybackWord() {
    if (!GetView()) {
        return;    // require doc and view
    }

    GetView()->OnPlaybackWord();

    // display pending error messages
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    pApp->DisplayMessages();

    SetButtonState(IDC_PLAY_WORD,IDC_STOP_WORD,true);
}

/***************************************************************************/
// CDlgEditor::OnPlaybackWord Button playback word hit
// Sends a message to the player to playback the original gloss selection.
/***************************************************************************/
void CDlgEditor::OnStopWord() {

    GetMainFrame()->SendMessage(WM_USER_PLAYER, CDlgPlayer::STOPPED, MAKELONG(-1, FALSE));
    SetButtonState(IDC_PLAY_WORD,IDC_STOP_WORD,false);
}

/***************************************************************************/
// CDlgEditor::OnPlaybackPhraseL1 Button playback Phrase L2 hit
// Sends a message to the player to playback the original phrase selection.
/***************************************************************************/
void CDlgEditor::OnPlaybackPhraseL1() {
    if (!GetView()) {
        return;    // require doc and view
    }

    GetView()->OnPlaybackPhraseL1();

    // display pending error messages
    CSaApp * pApp = (CSaApp *)AfxGetApp();  // get pointer to application
    pApp->DisplayMessages();

    SetButtonState(IDC_PLAY_PHRASE_L1,IDC_STOP_PHRASE_L1,true);
}

/***************************************************************************/
// CDlgEditor::OnPlaybackPhraseL1 Button playback Phrase L2 hit
// Sends a message to the player to playback the original phrase selection.
/***************************************************************************/
void CDlgEditor::OnStopPhraseL1() {

    GetMainFrame()->SendMessage(WM_USER_PLAYER, CDlgPlayer::STOPPED, MAKELONG(-1, FALSE));
    SetButtonState(IDC_PLAY_PHRASE_L1,IDC_STOP_PHRASE_L1,false);
}

/***************************************************************************/
// CDlgEditor::OnUpdateInputstring Inputstring is changed about to update
// Filter input stream dialog input
/***************************************************************************/
void CDlgEditor::OnUpdateInputstring() {

    if (!GetView()) {
        UpdateDialog();
        return;
    }

    GetView()->UpdateSelection();
    if (GetView()->GetSelectionIndex()== -1) {
        UpdateDialog();
        return;
    }

    CEdit * pEdit = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
    CSaString szString;
    pEdit->GetWindowText(szString);

    // Filter input string if input filter specified
    CSegment * pSegment = GetView()->GetAnnotation(GetView()->GetSelectionIndex());
    if (pSegment->Filter(szString)) {
        DWORD dwSelection = pEdit->GetSel();
        pEdit->SetWindowText(szString);
        pEdit->SetSel(dwSelection);
    }

    if (!IsDifferent(FALSE)) { // Insure selection has not changed
        GetView()->SetSelectedAnnotationString(szString, FALSE, m_bCheck);
        // We only want one checkpoint per changed segment
        m_bCheck = FALSE;
        // Update IsDifferent check to changed string
        IsDifferent(TRUE);
    } else {
        UpdateDialog();    // Lose keystroke, avoid corrupting data...should never get here
    }
}

/***************************************************************************/
// CDlgEditor::OnUpdateDialog Update dialog to current state
/***************************************************************************/
void CDlgEditor::UpdateDialog() {

	// CSaView not active or no WAV data
    if ((!GetView()) || (GetDoc()->GetDataSize()==0)) { 
        if (IsWindowVisible()) {
            ShowWindow(SW_HIDE);
        }
        return;
    }

    {
        CFont * pFont = NULL;
        CString szText;
        szText.LoadString(IDS_DISABLED_SEGMENT);
        BOOL bEnable = FALSE;
        BOOL bEnableChart = FALSE;
        BOOL bFontASAPSIL = FALSE;

        // Update all command buttons
        UpdateDialogControls(GetMainFrame(), FALSE);

        GetView()->UpdateSelection();
        if (GetView()->GetSelectionIndex() != -1) {
            //selection available
            int nAnnotation = GetView()->GetSelectionIndex();
            pFont = GetDoc()->GetFont(nAnnotation);

            szText = GetView()->GetSelectedAnnotationString(TRUE);

            bEnable = TRUE;
            CString fontFace = GetMainFrame()->GetFontFace(nAnnotation);
            bFontASAPSIL = (fontFace.Left(8) == "ASAP SIL");

        } else {
            pFont = GetDoc()->GetFont(GLOSS);
        }

        if ((pFont)&&(((CFontTable *)pFont)->IsIPA())) {
            bEnableChart = bFontASAPSIL;
        }

        CWnd * pChart = GetDlgItem(IDC_CHARACTERCHART);
        if (pChart) {
            pChart->EnableWindow(TRUE);
        }

        CEdit * pWnd = (CEdit *)GetDlgItem(IDC_INPUTSTRING);
        if (pWnd!=NULL) {
            if (pFont) {
                pWnd->SetFont(pFont);
            }
            if (IsDifferent(TRUE)) {
                pWnd->SetWindowText(szText);
                if (bEnable) {
                    pWnd->SetSel(0,-1);         // Select all text
                } else {
                    pWnd->SetSel(-1,-1);        // Select all text
                }
                pWnd->EnableWindow(bEnable);
                if ((bEnable) && (m_bActivated)) {
                    pWnd->SetFocus();
                } else {
                    if ((m_NextButton.m_hWnd!=NULL) && (m_bActivated) && (!bEnable)) {
                        m_NextButton.SetFocus();
                    }
                }
            }
            m_bCheck = TRUE;
        }
    }
}

/***************************************************************************/
// CDlgEditor::IsDifferent Checks if selection has changed since last update
/***************************************************************************/
BOOL CDlgEditor::IsDifferent(BOOL bUpdate) {
    BOOL ret = FALSE;
    CSaView * pView = GetView();

    if (m_pPreviousSaView != pView) {
        ret = TRUE;
        if (bUpdate) {
            m_pPreviousSaView = pView;
        }
    }

    if (pView==NULL) {
        return ret;
    }

    GetView()->UpdateSelection();

    if (m_nPreviousAnnotationIndex != pView->GetSelectionIndex()) {
        ret = TRUE;
        if (bUpdate) {
            m_nPreviousAnnotationIndex = pView->GetSelectionIndex();
        }
    }
    if (pView->GetSelectionIndex()==-1) {
        return ret;
    }

    if ((m_dwPreviousStart != pView->GetSelectionStart()) ||
            (m_dwPreviousStop != pView->GetSelectionStop())) {
        ret = TRUE;
        if (bUpdate) {
            m_dwPreviousStart = pView->GetSelectionStart();
            m_dwPreviousStop = pView->GetSelectionStop();
        }
    }

    if (m_szPreviousString != pView->GetSelectedAnnotationString(FALSE)) {
        ret = TRUE;
        if (bUpdate) {
            m_szPreviousString = pView->GetSelectedAnnotationString(FALSE);
        }
    }
    return ret;
}


void CDlgEditor::OnDestroy() {
    CDialog::OnDestroy();
    bEditor = FALSE;
}

/***************************************************************************/
// CDlgEditor::PreTranslateMessage
// Translates accelerators BEFORE giving ALL characters to dialog
// disables VK_RETURN & VK_DELETE accelerators
/***************************************************************************/
BOOL CDlgEditor::PreTranslateMessage(MSG * pMsg) {
    CMainFrame * pMain = GetMainFrame();
    // translate accelerators for frame and any children
    if (pMain && (pMain->GetCurrSaView()) && (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)) { // 1.5Test8.5
        switch (pMsg->wParam) {
        case VK_RETURN:
            if (pMsg->message == WM_KEYDOWN) {
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_NEXT, 0);    //SDM 1.5Test10.0
            }
            return TRUE;
        case VK_DELETE:
            if (GetKeyState(VK_SHIFT) >= 0) {
                break;    // Only translate if shift key is down (fall through)
            }
        case VK_ESCAPE:
            if (pMsg->message == WM_KEYDOWN) {
                OnCancel();    //SDM 1.5Test10.0
            }
            return TRUE;
        case VK_LEFT: {
            int nFirst = 0, nLast = 0;
            CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
            if (pWnd && pWnd->IsWindowEnabled()) {
                pWnd->GetSel(nFirst, nLast);
            }
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) && nFirst == 0 && nLast == nFirst) {
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_PREVIOUS, 0);
                return TRUE;
            } else {
                return FALSE;
            }
        }
        case VK_RIGHT: {
            int nFirst = 0, nLast = 0, nLength = 0;
            CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
            if (pWnd && pWnd->IsWindowEnabled()) {
                pWnd->GetSel(nFirst, nLast);
                nLength = pWnd->LineLength();
            }
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) && nFirst == nLength && nLast == nFirst) {
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_NEXT, 0);
                return TRUE;
            } else {
                return FALSE;
            }
        }
        case VK_UP:
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)) {
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_UP, 0);
            }
            return TRUE;
        case VK_DOWN:
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)) {
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_DOWN, 0);
            }
            return TRUE;
        default:
            // Translate these accelerators
            if (pMain->GetCurrSaView()->PreTranslateMessage(pMsg)) { // 1.5Test8.5
                return TRUE;
            }
        }
    }

    // Then call default handling
    return CDialog::PreTranslateMessage(pMsg);
}



/***************************************************************************/
// CDlgEditor::OnActivate
// Set Focus to edit control, or if disabled to next button
/***************************************************************************/
void CDlgEditor::OnActivate(UINT nState, CWnd * pWndOther, BOOL bMinimized) {
    
	UpdateDialog();

    CDialog::OnActivate(nState, pWndOther, bMinimized);

    m_bActivated = (nState != WA_INACTIVE);

    CEdit * pWnd = (CEdit *)GetDlgItem(IDC_INPUTSTRING);
    if ((pWnd!=NULL) && (pWnd->IsWindowEnabled())) {
        pWnd->SetFocus();
    } else {
        if ((m_NextButton.m_hWnd!=NULL) && (m_NextButton.IsWindowEnabled())) {
            m_NextButton.SetFocus();
        }
    }
}


/***************************************************************************/
// CDlgEditor::OnPaint
// Paint cursor color between cursor adjust buttons
/***************************************************************************/
void CDlgEditor::OnPaint() {
    CPaintDC dc(this); // device context for painting

    CWnd * pWnd = GetDlgItem(IDC_START);
    if (pWnd) {
        CRect rWnd;
        pWnd->GetWindowRect(&rWnd); //In screen coordinates
        ScreenToClient(&rWnd);
        CPen pen(PS_SOLID, 1, GetMainFrame()->GetColors()->cPlotStartCursor);
        CPen * pOldPen = dc.SelectObject(&pen);
        CBrush brush(GetMainFrame()->GetColors()->cPlotStartCursor);
        CBrush * pOldBrush = (CBrush *)dc.SelectObject(&brush);
        dc.Rectangle(rWnd.left, rWnd.top, rWnd.right, rWnd.bottom);
        dc.SelectObject(pOldBrush);
        dc.SelectObject(pOldPen);
    }

    pWnd = GetDlgItem(IDC_STOP);
    if (pWnd) {
        CRect rWnd;
        pWnd->GetWindowRect(rWnd); //In screen coordinates
        ScreenToClient(&rWnd);
        CPen pen(PS_SOLID, 1, GetMainFrame()->GetColors()->cPlotStopCursor);
        CPen * pOldPen = dc.SelectObject(&pen);
        CBrush brush(GetMainFrame()->GetColors()->cPlotStopCursor);
        CBrush * pOldBrush = (CBrush *)dc.SelectObject(&brush);
        dc.Rectangle(rWnd.left, rWnd.top, rWnd.right, rWnd.bottom);
        dc.SelectObject(pOldBrush);
        dc.SelectObject(pOldPen);
    }

    // Do not call CDialog::OnPaint() for painting messages
}

/***************************************************************************/
// CDlgEditor::OnCancel undo current change and close
/***************************************************************************/
void CDlgEditor::OnCancel() {
    if ((GetView()!=NULL) && (!m_bCheck) && !IsDifferent(FALSE)) {
        GetView()->SendMessage(WM_COMMAND, ID_EDIT_UNDO, 0);
    }
    CDialog::OnCancel();
}

// SDM 1.5Test8.2
/***************************************************************************/
// CDlgEditor::OnClose close TE don't undo current change
/***************************************************************************/
void CDlgEditor::OnClose() {
    SendMessage(WM_COMMAND, IDOK, 0);
}

/***************************************************************************/
// CDlgEditor::OnHelpEdit Call Information help
/***************************************************************************/
void CDlgEditor::OnHelpEdit() {
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/Edit/Transcription/Transcription_Editor.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAnnotationEdit dialog

CDlgAnnotationEdit::CDlgAnnotationEdit(CWnd * pParent /*=NULL*/)
    : CDialog(CDlgAnnotationEdit::IDD, pParent) {
    m_bClosing = FALSE;
    m_bChanged = FALSE;
}

void CDlgAnnotationEdit::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgAnnotationEdit, CDialog)
    ON_WM_SIZE()
    ON_EN_UPDATE(IDC_INPUTSTRING, OnUpdateInputstring)
    ON_WM_ACTIVATE()
    ON_WM_CLOSE()
    ON_COMMAND(IDOK, OnOK)
    ON_COMMAND(IDCANCEL, OnCancel)
    ON_WM_PARENTNOTIFY()
END_MESSAGE_MAP()

/***************************************************************************/
// CDlgAnnotationEdit::OnOK add checkpoint and save string
/***************************************************************************/
void CDlgAnnotationEdit::OnOK() {
    if (!m_bClosing) {
        m_bClosing = TRUE;
        if ((GetView()) && m_bChanged) {
            GetView()->UpdateSelection();
            CEdit * pEdit = (CEdit *)GetDlgItem(IDC_INPUTSTRING);
            CSaString szString;
            pEdit->GetWindowText(szString);
            GetView()->SetSelectedAnnotationString(szString, FALSE, TRUE);
        }
        DestroyWindow();
    }
}

// SDM 1.5Test8.2
/***************************************************************************/
// CDlgEditor::OnClose close TE don't undo current change
/***************************************************************************/
void CDlgAnnotationEdit::OnClose() {
    OnOK();
}

/***************************************************************************/
// CDlgAnnotationEdit::OnCancel destroy window ignore string
/***************************************************************************/
void CDlgAnnotationEdit::OnCancel() {
    if (!m_bClosing) {
        m_bClosing = TRUE;
        DestroyWindow();
    }
}

/***************************************************************************/
// CAnnotationEdit::PostNcDestroy remove this
/***************************************************************************/
void CDlgAnnotationEdit::PostNcDestroy() {
    CDialog::PostNcDestroy();
    delete this;
}

/***************************************************************************/
// CDlgAnnotationEdit::OnSize resize edit box to match dialog
/***************************************************************************/
void CDlgAnnotationEdit::OnSize(UINT nType, int cx, int cy) {
    CDialog::OnSize(nType, cx, cy);

    CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);

    if (pWnd && pWnd->m_hWnd) {
        pWnd->SetWindowPos(NULL,0,0,cx,cy,SWP_NOACTIVATE);
    }
}

/***************************************************************************/
// CDlgAnnotationEdit::SetText Set current editted text in edit box
/***************************************************************************/
CString CDlgAnnotationEdit::SetText(const CString & szString) {
    OnUpdateInputstring();
    CEdit * pEdit = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
    CString szOldString;
    pEdit->GetWindowText(szOldString);
    pEdit->SetWindowText(szString);
    int nLength = szString.GetLength();
    pEdit->SetSel(nLength,nLength); // Select all text
    OnUpdateInputstring();

    return szOldString;
}

/***************************************************************************/
// CDlgAnnotationEdit::OnUpdateInputString filter string for invalid characters
/***************************************************************************/
void CDlgAnnotationEdit::OnUpdateInputstring() {
    if (!GetView()) {
        OnCancel();// Should never happen
    }

    GetView()->UpdateSelection();
    if (GetView()->GetSelectionIndex() == -1) {
        OnCancel();// Should never happen
    }

    m_bChanged = TRUE;

    // Filter Inputstring if input filter specified
    CSegment * pSegment = GetView()->GetAnnotation(GetView()->GetSelectionIndex());
    CEdit * pEdit = (CEdit *)GetDlgItem(IDC_INPUTSTRING);
    CSaString szString;
    pEdit->GetWindowText(szString);
    if (pSegment->Filter(szString)) {
        DWORD dwSelection = pEdit->GetSel();
        pEdit->SetWindowText(szString);
        pEdit->SetSel(dwSelection);
    }
}

/***************************************************************************/
// CDlgAnnotationEdit::GetView Get a pointer to the active CSaView if any
/***************************************************************************/
CSaView * CDlgAnnotationEdit::GetView() const {
    CSaView * pSaView = GetMainFrame()->GetCurrSaView();
    // Verify View is a CSaView type.
    if ((pSaView!=NULL) && (!pSaView->IsKindOf(RUNTIME_CLASS(CSaView)))) {
        pSaView = NULL;
    }
    return pSaView;
}

CSaDoc * CDlgAnnotationEdit::GetDoc() const {
    return GetView() ? GetView()->GetDocument() : NULL;
};

/***************************************************************************/
// CDlgAnnotationEdit::OnActivate close window on deactivate
/***************************************************************************/
void CDlgAnnotationEdit::OnActivate(UINT nState, CWnd * pWndOther, BOOL bMinimized) {
    CDialog::OnActivate(nState, pWndOther, bMinimized);

    if (!m_bClosing && (nState == WA_INACTIVE)) {
        OnOK();
    }
}

/***************************************************************************/
// CAnnotationEdit::OnInitDialog set font, text, selection, and style
/***************************************************************************/
BOOL CDlgAnnotationEdit::OnInitDialog() {
    CDialog::OnInitDialog();

    if (!GetView()) { // CSaView not active
        OnCancel();
    } else {
        CFont * pFont = NULL;

        GetView()->UpdateSelection();
        if (GetView()->GetSelectionIndex() != -1) { //selection
            int nAnnotation = GetView()->GetSelectionIndex();
            pFont = GetDoc()->GetFont(nAnnotation);
            CString szText = GetView()->GetSelectedAnnotationString(TRUE);

            CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
            if (pWnd!=NULL) {
                if (pFont) {
                    pWnd->SetFont(pFont);
                }
                pWnd->SetWindowText(szText);
				// Select all text
                pWnd->SetSel(0,-1);
            }
        } else {
            OnCancel();
        }
    }
	// return TRUE  unless you set the focus to a control
    return TRUE;
}

//SDM 1.5Test10.0
/***************************************************************************/
// CDlgAnnotationEdit::PreTranslateMessage
// Translates accelerators BEFORE giving ALL characters to dialog
// disables VK_RETURN & VK_DELETE accelerators
/***************************************************************************/
BOOL CDlgAnnotationEdit::PreTranslateMessage(MSG * pMsg) {
    
	CMainFrame * pMain = GetMainFrame();
    // translate accelerators for frame and any children
    if ((pMain!=NULL) && (pMain->GetCurrSaView()) && (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)) {
        switch (pMsg->wParam) {
        case VK_ESCAPE:
            if (pMsg->message == WM_KEYDOWN) {
                OnCancel();    //SDM 1.5Test8.6
            }
            return TRUE;
        case VK_LEFT: {
            int nFirst = 0, nLast = 0;
            CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
            if (pWnd && pWnd->IsWindowEnabled()) {
                pWnd->GetSel(nFirst, nLast);
            }
            if (((pMsg->message == WM_KEYDOWN) || 
				(pMsg->message == WM_SYSKEYDOWN)) && 
				(nFirst == 0) && 
				(nLast == nFirst)) {
                OnOK();
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_PREVIOUS, 0);
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
                return TRUE;
            } else {
                return FALSE;
            }
        }
        case VK_RIGHT: {
            int nFirst = 0, nLast = 0, nLength = 0;
            CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
            if (pWnd && pWnd->IsWindowEnabled()) {
                pWnd->GetSel(nFirst, nLast);
                nLength = pWnd->LineLength();
            }
            if (((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN)) &&
                (nFirst == nLength) &&
                (nLast == nFirst)) {
                OnOK();
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_NEXT, 0);
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
                return TRUE;
            } else {
                return FALSE;
            }
        }
        case VK_UP:
            if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN)) {
                OnOK();
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_UP, 0);
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
            }
            return TRUE;
        case VK_DOWN:
            if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN)) {
                OnOK();
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_DOWN, 0);
                GetView()->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
            }
            return TRUE;
        }
    }

    // Then call default handling
    return CDialog::PreTranslateMessage(pMsg);
}

CMainFrame * CDlgAnnotationEdit::GetMainFrame() const {
    return (CMainFrame *)AfxGetMainWnd();
};

BOOL CDlgAnnotationEdit::Create(UINT nIDTemplate, CWnd * pParentWnd) {
    return CDialog::Create(nIDTemplate, pParentWnd);
}

CMainFrame * CDlgEditor::GetMainFrame() const {
    return (CMainFrame *)AfxGetMainWnd();
}

void CDlgEditor::UpdatePlayer() {
    CMainFrame * pMainWnd = GetMainFrame();
    if (pMainWnd==NULL) {
        return;
    }
    if (!pMainWnd->IsPlayerPlaying()) {
        SetButtonState(IDC_PLAY_WORD,IDC_STOP_WORD,false);
        SetButtonState(IDC_PLAY_PHRASE_L1,IDC_STOP_PHRASE_L1,false);
    }
}

void CDlgEditor::SetButtonState(int playButton1, int stopButton, bool play) {
    CButton * pButton = (CButton *) GetDlgItem(playButton1);
    if (pButton!=NULL) {
        pButton->ShowWindow((!play)?SW_SHOW:SW_HIDE);
    }
    pButton = (CButton *) GetDlgItem(stopButton);
    if (pButton!=NULL) {
        pButton->ShowWindow((play)?SW_SHOW:SW_HIDE);
    }
}

