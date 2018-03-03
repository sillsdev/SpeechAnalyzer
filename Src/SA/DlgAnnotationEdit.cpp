/////////////////////////////////////////////////////////////////////////////
// DlgAnnotationEdit.cpp:
// Implementation of the CDlgAnnotationEdit (dialog) class.
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
#include "DlgAnnotationEdit.h"
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
            GetView()->SetSelectedAnnotationString(szString, false, true);
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
    pEdit->SetSel(nLength,nLength); 
	// Select all text
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

    if (!GetView()) { 
		// CSaView not active
        OnCancel();
    } else {
        CFont * pFont = NULL;

        GetView()->UpdateSelection();
        if (GetView()->GetSelectionIndex() != -1) { 
			//selection
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
	CSaView * pView = pMain->GetCurrSaView();

    // translate accelerators for frame and any children
    if ((pMain!=NULL) && (pView!=NULL) && ((pMsg->message >= WM_KEYFIRST) && (pMsg->message <= WM_KEYLAST))) {
        switch (pMsg->wParam) {
        case VK_ESCAPE:
            if (pMsg->message == WM_KEYDOWN) {
				//SDM 1.5Test8.6
                OnCancel();    
	            return TRUE;
            }
			break;
        case VK_LEFT: {
            int nFirst = 0;
			int nLast = 0;
            CEdit * pWnd = (CEdit *)GetDlgItem(IDC_INPUTSTRING);
            if ((pWnd!=NULL) && (pWnd->IsWindowEnabled())) {
                pWnd->GetSel(nFirst, nLast);
            }
            if (((pMsg->message == WM_KEYDOWN) || 
				(pMsg->message == WM_SYSKEYDOWN)) && 
				(nFirst == 0) && 
				(nLast == nFirst)) {
                OnOK();
                pView->SendMessage(WM_COMMAND, ID_EDIT_PREVIOUS, 0);
                pView->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
                return TRUE;
            }
			break;
        }
        case VK_RIGHT: {
            int nFirst = 0;
			int nLast = 0;
			int nLength = 0;
            CEdit * pWnd = (CEdit *)GetDlgItem(IDC_INPUTSTRING);
            if ((pWnd!=NULL) && (pWnd->IsWindowEnabled())) {
                pWnd->GetSel(nFirst, nLast);
                nLength = pWnd->LineLength();
            }
            if (((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN)) &&
                (nFirst == nLength) &&
                (nLast == nFirst)) {
                OnOK();
                pView->SendMessage(WM_COMMAND, ID_EDIT_NEXT, 0);
                pView->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
                return TRUE;
            }
			break;
        }
        case VK_UP:
            if (((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN)) &&
				(GetKeyState(VK_SHIFT)>=0)) {
                OnOK();
                pView->SendMessage(WM_COMMAND, ID_EDIT_UP, 0);
                pView->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
	            return TRUE;
            }
			break;
        case VK_DOWN:
            if (((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_SYSKEYDOWN)) &&
				(GetKeyState(VK_SHIFT)>=0)) {
                OnOK();
                pView->SendMessage(WM_COMMAND, ID_EDIT_DOWN, 0);
                pView->SendMessage(WM_COMMAND, ID_EDIT_INPLACE, 0);
	            return TRUE;
            }
			break;
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
