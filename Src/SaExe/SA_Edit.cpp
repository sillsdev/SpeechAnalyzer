/////////////////////////////////////////////////////////////////////////////
// sa_edit.cpp:
// Implementation of the CDlgEditor (dialog)
//                       CAnnotationEdit (dialog)           classes.
//
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.06.5
//      SDM Original
// 1.06.6
//      SDM Added CAnnotationEdit Class
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

//###########################################################################
// CDlgEditor dialog

/////////////////////////////////////////////////////////////////////////////
// CDlgEditor message map

BEGIN_MESSAGE_MAP(CDlgEditor, CDialog)
    //{{AFX_MSG_MAP(CDlgEditor)
    ON_BN_CLICKED(IDC_CHARACTERCHART, OnCharacterChart)
    ON_BN_CLICKED(IDC_PLAY_SEGMENT, OnPlaybackSegment)
    ON_BN_CLICKED(IDC_PLAY_WORD, OnPlaybackWord)
    ON_BN_CLICKED(IDC_PLAY_PHRASE_L1, OnPlaybackPhraseL1)
    ON_EN_UPDATE(IDC_INPUTSTRING, OnUpdateInputstring)
    ON_WM_DESTROY()
    ON_WM_ACTIVATE()
    ON_WM_PAINT()
    ON_WM_CLOSE()
    ON_COMMAND(IDHELP, OnHelpEdit)
    ON_COMMAND(IDCANCEL, OnCancel)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgEditor construction/destruction/creation

/***************************************************************************/
// CDlgEditor::CDlgEditor Constructor
/***************************************************************************/
CDlgEditor::CDlgEditor(CWnd * pParent) : CDialog(CDlgEditor::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDlgEditor)
    //}}AFX_DATA_INIT
    bEditor = FALSE;
    m_pPreviousSaView = NULL;
    m_bActivated = FALSE;
}

BOOL CDlgEditor::OnCmdMsg(UINT nID, int nCode, void * pExtra, AFX_CMDHANDLERINFO * pHandlerInfo)
{
    if (!CDialog::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
    {
        return MainFrame()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgEditor helper functions

// SDM 1.5Test8.2
/***************************************************************************/
// CDlgEditor::CreateSafe Create dialog load save position
/***************************************************************************/
BOOL CDlgEditor::CreateSafe(UINT nIDTemplate, CWnd * pParentWnd, WINDOWPLACEMENT * pWPL)
{
    if (!bEditor)
    {
        bEditor = CDialog::Create(nIDTemplate, pParentWnd);

        if (pWPL && pWPL->length)
        {
            SetWindowPos(NULL,pWPL->rcNormalPosition.left,pWPL->rcNormalPosition.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        }
    }
    return bEditor;
};

/***************************************************************************/
// CDlgEditor::DoDataExchange Data exchange
/***************************************************************************/
void CDlgEditor::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgEditor)
    DDX_Control(pDX, ID_EDIT_NEXT, m_cNextButton);
    DDX_Control(pDX, ID_EDIT_PREVIOUS, m_cPreviousButton);
    DDX_Control(pDX, ID_EDIT_UP, m_cUpButton);
    DDX_Control(pDX, ID_EDIT_DOWN, m_cDownButton);
    //}}AFX_DATA_MAP
}

/***************************************************************************/
// CDlgEditor::SaView Get a pointer to the active CSaView if any
/***************************************************************************/
CSaView * CDlgEditor::SaView() const
{
    CSaView * pSaView = MainFrame()->GetCurrSaView();

    if (pSaView && !pSaView->IsKindOf(RUNTIME_CLASS(CSaView)))
    {
        pSaView = NULL;    // Verify View is a CSaView type.
    }

    return pSaView;
}

CSaDoc * CDlgEditor::SaDoc() const
{
    return SaView() ? SaView()->GetDocument() : NULL;
};
CSegment * CDlgEditor::GetSelectedSegment() const
{
    return SaView() ? SaView()->FindSelectedAnnotation() : NULL;
};
/////////////////////////////////////////////////////////////////////////////
// CDlgEditor message handlers

/***************************************************************************/
// CDlgEditor::OnInitDialog Dialog initialisation
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CDlgEditor::OnInitDialog()
{
    CDialog::OnInitDialog();

    CenterWindow(); // center dialog on frame window

    // Load button icons
    m_cNextButton.SetIcon((HICON)LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDI_NEXT),RT_GROUP_ICON),MAKEINTRESOURCE(IDI_NEXT),IMAGE_ICON,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
    m_cPreviousButton.SetIcon((HICON)LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDI_PREV),RT_GROUP_ICON),MAKEINTRESOURCE(IDI_PREV),IMAGE_ICON,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
    m_cUpButton.SetIcon((HICON)LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDI_UP),RT_GROUP_ICON),MAKEINTRESOURCE(IDI_UP),IMAGE_ICON,0,0,LR_LOADMAP3DCOLORS));
    m_cDownButton.SetIcon((HICON)LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDI_DOWN),RT_GROUP_ICON),MAKEINTRESOURCE(IDI_DOWN),IMAGE_ICON,17,15,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));

    m_cNextButton.SetFocus();
    return FALSE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgEditor::OnCharacterChart Input from character chart wanted
/***************************************************************************/
void CDlgEditor::OnCharacterChart()
{
    // create the character chart property sheet
    CString szCaption;
    szCaption.LoadString(IDS_DLGTITLE_CHARCHART); // load caption string
    CDlgCharChart * pDlgCharChart;
    pDlgCharChart = new CDlgCharChart(szCaption, NULL, 0); // create the property sheet
    pDlgCharChart->SetInitialState(GetDlgItem(IDC_INPUTSTRING)->GetFont());
    if (pDlgCharChart->DoModal() == IDOK)
    {
        // insert the new string
        ((CEdit *)GetDlgItem(IDC_INPUTSTRING))->ReplaceSel(pDlgCharChart->m_szString);
        UpdateData(TRUE);
    }
    if (pDlgCharChart)
    {
        delete pDlgCharChart;
    }
    GotoDlgCtrl(GetDlgItem(IDC_INPUTSTRING)); // set focus to edit control
}

/***************************************************************************/
// CDlgEditor::OnPlaybackSegment Button playback segment hit
// Sends a message to the player to playback the current segment selection.
/***************************************************************************/
void CDlgEditor::OnPlaybackSegment()
{
    if (!SaView())
    {
        return;    // require doc and view
    }
    SaView()->OnPlaybackSegment();

    // display pending error messages
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    pApp->DisplayMessages();
}

/***************************************************************************/
// CDlgEditor::OnPlaybackWord Button playback word hit
// Sends a message to the player to playback the original gloss selection.
/***************************************************************************/
void CDlgEditor::OnPlaybackWord()
{
    if (!SaView())
    {
        return;    // require doc and view
    }
    SaView()->OnPlaybackWord();

    // display pending error messages
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    pApp->DisplayMessages();
}

/***************************************************************************/
// CDlgEditor::OnPlaybackPhraseL1 Button playback Phrase L2 hit
// Sends a message to the player to playback the original phrase selection.
/***************************************************************************/
void CDlgEditor::OnPlaybackPhraseL1()
{
    if (!SaView())
    {
        return;    // require doc and view
    }
    SaView()->OnPlaybackPhraseL1();

    // display pending error messages
    CSaApp * pApp = (CSaApp *)AfxGetApp();  // get pointer to application
    pApp->DisplayMessages();
}

/***************************************************************************/
// CDlgEditor::OnUpdateInputstring Inputstring is changed about to update
// Filter input stream dialog input
/***************************************************************************/
void CDlgEditor::OnUpdateInputstring()
{
    if (!SaView())
    {
        UpdateDialog();
        return;
    }

    SaView()->ASelection().Update(SaView());
    if (SaView()->ASelection().GetSelection().nAnnotationIndex == -1)
    {
        UpdateDialog();
        return;
    }

    CEdit * pEdit = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
    CSaString szString;
    pEdit->GetWindowText(szString);
    // Filter Inputstring if input filter specified
    if (SaView()->GetAnnotation(SaView()->ASelection().GetSelection().nAnnotationIndex)->GetInputFilter() != NULL)
    {
        BOOL bChanged = (SaView()->GetAnnotation(SaView()->ASelection().GetSelection().nAnnotationIndex)->GetInputFilter())(szString);
        if (bChanged)
        {
            DWORD dwSelection = pEdit->GetSel();
            pEdit->SetWindowText(szString);
            pEdit->SetSel(dwSelection);
        }
    }

    if (!IsDifferent(FALSE))   // Insure selection has not changed
    {
        SaView()->ASelection().SetSelectedAnnotationString(SaView(), szString, FALSE, m_bCheck);
        // We only want one checkpoint per changed segment
        m_bCheck = FALSE;
        // Update IsDifferent check to changed string
        IsDifferent(TRUE);
    }
    else
    {
        UpdateDialog();    // Lose keystroke, avoid corrupting data...should never get here
    }
}

/***************************************************************************/
// CDlgEditor::OnUpdateDialog Update dialog to current state
/***************************************************************************/
void CDlgEditor::UpdateDialog()
{
    if ((!SaView())||(SaDoc()->GetUnprocessedDataSize()==0))   // CSaView not active or no WAV data
    {
        if (IsWindowVisible())
        {
            ShowWindow(SW_HIDE);
        }
        return;
    }

    {
        CFont * pFont = NULL;

        CString     szText("Disabled - select or add a segment");

        BOOL bEnable = FALSE;
        BOOL bEnableChart = FALSE;
        BOOL bFontASAPSIL = FALSE;

        // Update all command buttons
        UpdateDialogControls(MainFrame(), FALSE);

        SaView()->ASelection().Update(SaView());
        if (SaView()->ASelection().GetSelection().nAnnotationIndex != -1)   //selection
        {
            int nAnnotation = SaView()->ASelection().GetSelection().nAnnotationIndex;
            pFont = SaDoc()->GetFont(nAnnotation);

            szText = SaView()->ASelection().GetSelectedAnnotationString(SaView());

            bEnable = TRUE;
            CString fontFace = MainFrame()->GetFontFace(nAnnotation);
            bFontASAPSIL = (fontFace.Left(8) == "ASAP SIL");

        }
        else
        {
            pFont = SaDoc()->GetFont(GLOSS);
        }

        if ((pFont)&&(((CFontTable *)pFont)->IsIPA()))
        {
            bEnableChart = bFontASAPSIL;
        }

        CWnd * pChart = GetDlgItem(IDC_CHARACTERCHART);
        if (pChart)
        {
            pChart->EnableWindow(TRUE);
        }

        CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
        if (pWnd!=NULL)
        {
            if (pFont)
            {
                pWnd->SetFont(pFont);
            }
            if (IsDifferent(TRUE))
            {
                pWnd->SetWindowText(szText);
                if (bEnable)
                {
                    pWnd->SetSel(0,-1);    // Select all text
                }
                else
                {
                    pWnd->SetSel(-1,-1);    // Select all text
                }
                pWnd->EnableWindow(bEnable);
                if (bEnable && m_bActivated)
                {
                    pWnd->SetFocus();
                }
                else
                {
                    CWnd * pWnd = &m_cNextButton;
                    if (pWnd && m_bActivated && !bEnable)
                    {
                        pWnd->SetFocus();
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
BOOL CDlgEditor::IsDifferent(BOOL bUpdate)
{
    BOOL ret = FALSE;
    if (m_pPreviousSaView != SaView())
    {
        ret = TRUE;
        if (bUpdate)
        {
            m_pPreviousSaView = SaView();
        }
    }
    if (!SaView())
    {
        return ret;
    }


    SaView()->ASelection().Update(SaView());
    CASegmentSelection::CASelection cSelection = SaView()->ASelection().GetSelection();
    if (m_nPreviousAnnotationIndex != cSelection.nAnnotationIndex)
    {
        ret = TRUE;
        if (bUpdate)
        {
            m_nPreviousAnnotationIndex = cSelection.nAnnotationIndex;
        }
    }
    if (cSelection.nAnnotationIndex==-1)
    {
        return ret;
    }

    if ((m_dwPreviousStart != cSelection.dwStart)||(m_dwPreviousStop != cSelection.dwStop))
    {
        ret = TRUE;
        if (bUpdate)
        {
            m_dwPreviousStart = cSelection.dwStart;
            m_dwPreviousStop = cSelection.dwStop;
        }
    }

    if (m_szPreviousString != SaView()->ASelection().GetSelectedAnnotationString(SaView(), FALSE))
    {
        ret = TRUE;
        if (bUpdate)
        {
            m_szPreviousString = SaView()->ASelection().GetSelectedAnnotationString(SaView(),FALSE);
        }
    }

    return ret;
}


void CDlgEditor::OnDestroy()
{
    CDialog::OnDestroy();
    bEditor = FALSE;
}


/***************************************************************************/
// CDlgEditor::PreTranslateMessage
// Translates accelerators BEFORE giving ALL characters to dialog
// disables VK_RETURN & VK_DELETE accelerators
/***************************************************************************/
BOOL CDlgEditor::PreTranslateMessage(MSG * pMsg)
{
    CMainFrame * pMain = MainFrame();
    // translate accelerators for frame and any children
    if (pMain && (pMain->GetCurrSaView()) && (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST))   // 1.5Test8.5
    {
        switch (pMsg->wParam)
        {
        case VK_RETURN:
            if (pMsg->message == WM_KEYDOWN)
            {
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_NEXT, 0);    //SDM 1.5Test10.0
            }
            return TRUE;
        case VK_DELETE:
            if (GetKeyState(VK_SHIFT) >= 0)
            {
                break;    // Only translate if shift key is down (fall through)
            }
        case VK_ESCAPE:
            if (pMsg->message == WM_KEYDOWN)
            {
                OnCancel();    //SDM 1.5Test10.0
            }
            return TRUE;
        case VK_LEFT:
        {
            int nFirst = 0, nLast = 0;
            CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
            if (pWnd && pWnd->IsWindowEnabled())
            {
                pWnd->GetSel(nFirst, nLast);
            }
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) && nFirst == 0 && nLast == nFirst)
            {
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_PREVIOUS, 0);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        case VK_RIGHT:
        {
            int nFirst = 0, nLast = 0, nLength = 0;
            CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
            if (pWnd && pWnd->IsWindowEnabled())
            {
                pWnd->GetSel(nFirst, nLast);
                nLength = pWnd->LineLength();
            }
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) && nFirst == nLength && nLast == nFirst)
            {
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_NEXT, 0);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        case VK_UP:
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN))
            {
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_UP, 0);
            }
            return TRUE;
        case VK_DOWN:
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN))
            {
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_DOWN, 0);
            }
            return TRUE;
        default:
            // Translate these accelerators
            if (pMain->GetCurrSaView()->PreTranslateMessage(pMsg))   // 1.5Test8.5
            {
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
void CDlgEditor::OnActivate(UINT nState, CWnd * pWndOther, BOOL bMinimized)
{
    UpdateDialog();

    CDialog::OnActivate(nState, pWndOther, bMinimized);

    m_bActivated = (nState != WA_INACTIVE);

    CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);

    if (pWnd && pWnd->IsWindowEnabled())
    {
        pWnd->SetFocus();
    }
    else
    {
        CWnd * pWnd = &m_cNextButton;
        if (pWnd && pWnd->IsWindowEnabled())
        {
            pWnd->SetFocus();
        }
    }
}


/***************************************************************************/
// CDlgEditor::OnPaint
// Paint cursor color between cursor adjust buttons
/***************************************************************************/
void CDlgEditor::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    CWnd * pWnd = GetDlgItem(IDC_START);
    if (pWnd)
    {
        CRect rWnd;
        pWnd->GetWindowRect(&rWnd); //In screen coordinates
        ScreenToClient(&rWnd);
        CPen pen(PS_SOLID, 1, MainFrame()->GetColors()->cPlotStartCursor);
        CPen * pOldPen = dc.SelectObject(&pen);
        CBrush brush(MainFrame()->GetColors()->cPlotStartCursor);
        CBrush * pOldBrush = (CBrush *)dc.SelectObject(&brush);
        dc.Rectangle(rWnd.left, rWnd.top, rWnd.right, rWnd.bottom);
        dc.SelectObject(pOldBrush);
        dc.SelectObject(pOldPen);
    }

    pWnd = GetDlgItem(IDC_STOP);
    if (pWnd)
    {
        CRect rWnd;
        pWnd->GetWindowRect(rWnd); //In screen coordinates
        ScreenToClient(&rWnd);
        CPen pen(PS_SOLID, 1, MainFrame()->GetColors()->cPlotStopCursor);
        CPen * pOldPen = dc.SelectObject(&pen);
        CBrush brush(MainFrame()->GetColors()->cPlotStopCursor);
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
void CDlgEditor::OnCancel()
{
    if (SaView() && (!m_bCheck) && !IsDifferent(FALSE))
    {
        SaView()->SendMessage(WM_COMMAND, ID_EDIT_UNDO, 0);
    }
    CDialog::OnCancel();
}

// SDM 1.5Test8.2
/***************************************************************************/
// CDlgEditor::OnClose close TE don't undo current change
/***************************************************************************/
void CDlgEditor::OnClose()
{
    SendMessage(WM_COMMAND, IDOK, 0);
}

/***************************************************************************/
// CDlgEditor::OnHelpEdit Call Information help
/***************************************************************************/
void CDlgEditor::OnHelpEdit()
{
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/Edit/Transcription/Transcription_Editor.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CAnnotationEdit dialog


CAnnotationEdit::CAnnotationEdit(CWnd * pParent /*=NULL*/)
    : CDialog(CAnnotationEdit::IDD, pParent)
{
    //{{AFX_DATA_INIT(CAnnotationEdit)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_bClosing = FALSE;
    m_bChanged = FALSE;
}

void CAnnotationEdit::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAnnotationEdit)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAnnotationEdit, CDialog)
    //{{AFX_MSG_MAP(CAnnotationEdit)
    ON_WM_SIZE()
    ON_EN_UPDATE(IDC_INPUTSTRING, OnUpdateInputstring)
    ON_WM_ACTIVATE()
    ON_WM_CLOSE()
    ON_COMMAND(IDOK, OnOK)
    ON_COMMAND(IDCANCEL, OnCancel)
    ON_WM_PARENTNOTIFY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAnnotationEdit message handlers

/***************************************************************************/
// CAnnotationEdit::OnOK add checkpoint and save string
/***************************************************************************/
void CAnnotationEdit::OnOK()
{
    if (!m_bClosing)
    {
        m_bClosing = TRUE;
        if ((SaView()) && m_bChanged)
        {
            SaView()->ASelection().Update(SaView());
            CEdit * pEdit = (CEdit *)GetDlgItem(IDC_INPUTSTRING);
            CSaString szString;
            pEdit->GetWindowText(szString);
            SaView()->ASelection().SetSelectedAnnotationString(SaView(), szString, FALSE, TRUE);
        }
        DestroyWindow();
    }
}

// SDM 1.5Test8.2
/***************************************************************************/
// CDlgEditor::OnClose close TE don't undo current change
/***************************************************************************/
void CAnnotationEdit::OnClose()
{
    OnOK();
}

/***************************************************************************/
// CAnnotationEdit::OnCancel destroy window ignore string
/***************************************************************************/
void CAnnotationEdit::OnCancel()
{
    if (!m_bClosing)
    {
        m_bClosing = TRUE;
        DestroyWindow();
    }
}

/***************************************************************************/
// CAnnotationEdit::PostNcDestroy remove this
/***************************************************************************/
void CAnnotationEdit::PostNcDestroy()
{
    CDialog::PostNcDestroy();
    delete this;
}

/***************************************************************************/
// CAnnotationEdit::OnSize resize edit box to match dialog
/***************************************************************************/
void CAnnotationEdit::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);

    if (pWnd && pWnd->m_hWnd)
    {
        pWnd->SetWindowPos(NULL,0,0,cx,cy,SWP_NOACTIVATE);
    }
}

/***************************************************************************/
// CAnnotationEdit::SetText Set current editted text in edit box
/***************************************************************************/
CString CAnnotationEdit::SetText(const CString & szString)
{
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
// CAnnotationEdit::OnUpdateInputString filter string for invalid characters
/***************************************************************************/
void CAnnotationEdit::OnUpdateInputstring()
{
    if (!SaView())
    {
        OnCancel();// Should never happen
    }

    SaView()->ASelection().Update(SaView());
    if (SaView()->ASelection().GetSelection().nAnnotationIndex == -1)
    {
        OnCancel();// Should never happen
    }

    m_bChanged = TRUE;

    // Filter Inputstring if input filter specified
    if (SaView()->GetAnnotation(SaView()->ASelection().GetSelection().nAnnotationIndex)->GetInputFilter() != NULL)
    {
        CEdit * pEdit = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
        CSaString szString;
        pEdit->GetWindowText(szString);
        BOOL bChanged = (SaView()->GetAnnotation(SaView()->ASelection().GetSelection().nAnnotationIndex)->GetInputFilter())(szString);
        if (bChanged)
        {
            DWORD dwSelection = pEdit->GetSel();
            pEdit->SetWindowText(szString);
            pEdit->SetSel(dwSelection);
        }
    }
}

/***************************************************************************/
// CDlgEditor::SaView Get a pointer to the active CSaView if any
/***************************************************************************/
CSaView * CAnnotationEdit::SaView() const
{
    CSaView * pSaView = MainFrame()->GetCurrSaView();
    // Verify View is a CSaView type.
    if ((pSaView) && (!pSaView->IsKindOf(RUNTIME_CLASS(CSaView))))
    {
        pSaView = NULL;
    }
    return pSaView;
}

CSaDoc * CAnnotationEdit::SaDoc() const
{
    return SaView() ? SaView()->GetDocument() : NULL;
};

/***************************************************************************/
// CAnnotationEdit::OnActivate close window on deactivate
/***************************************************************************/
void CAnnotationEdit::OnActivate(UINT nState, CWnd * pWndOther, BOOL bMinimized)
{
    CDialog::OnActivate(nState, pWndOther, bMinimized);

    if (!m_bClosing && (nState == WA_INACTIVE))
    {
        OnOK();
    }
}

/***************************************************************************/
// CAnnotationEdit::OnInitDialog set font, text, selection, and style
/***************************************************************************/
BOOL CAnnotationEdit::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (!SaView())   // CSaView not active
    {
        OnCancel();
    }
    else
    {
        CFont * pFont = NULL;

        SaView()->ASelection().Update(SaView());
        if (SaView()->ASelection().GetSelection().nAnnotationIndex != -1)   //selection
        {
            int nAnnotation = SaView()->ASelection().GetSelection().nAnnotationIndex;
            pFont = SaDoc()->GetFont(nAnnotation);
            CString szText = SaView()->ASelection().GetSelectedAnnotationString(SaView());

            CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
            if (pWnd!=NULL)
            {
                if (pFont)
                {
                    pWnd->SetFont(pFont);
                }
                pWnd->SetWindowText(szText);
                pWnd->SetSel(0,-1); // Select all text
            }
        }
        else
        {
            OnCancel();
        }
    }

    return TRUE;  // return TRUE  unless you set the focus to a control

}


//SDM 1.5Test10.0
/***************************************************************************/
// CAnnotationEdit::PreTranslateMessage
// Translates accelerators BEFORE giving ALL characters to dialog
// disables VK_RETURN & VK_DELETE accelerators
/***************************************************************************/
BOOL CAnnotationEdit::PreTranslateMessage(MSG * pMsg)
{
    CMainFrame * pMain = MainFrame();
    // translate accelerators for frame and any children
    if (pMain && (pMain->GetCurrSaView()) && (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST))
    {
        switch (pMsg->wParam)
        {
        case VK_ESCAPE:
            if (pMsg->message == WM_KEYDOWN)
            {
                OnCancel();    //SDM 1.5Test8.6
            }
            return TRUE;
        case VK_LEFT:
        {
            int nFirst = 0, nLast = 0;
            CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
            if (pWnd && pWnd->IsWindowEnabled())
            {
                pWnd->GetSel(nFirst, nLast);
            }
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) && nFirst == 0 && nLast == nFirst)
            {
                OnOK();
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_PREVIOUS, 0);
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        case VK_RIGHT:
        {
            int nFirst = 0, nLast = 0, nLength = 0;
            CEdit * pWnd = (CEdit *) GetDlgItem(IDC_INPUTSTRING);
            if (pWnd && pWnd->IsWindowEnabled())
            {
                pWnd->GetSel(nFirst, nLast);
                nLength = pWnd->LineLength();
            }
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) && nFirst == nLength && nLast == nFirst)
            {
                OnOK();
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_NEXT, 0);
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        case VK_UP:
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN))
            {
                OnOK();
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_UP, 0);
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
            }
            return TRUE;
        case VK_DOWN:
            if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN))
            {
                OnOK();
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_DOWN, 0);
                SaView()->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
            }
            return TRUE;
        }
    }

    // Then call default handling
    return CDialog::PreTranslateMessage(pMsg);
}

