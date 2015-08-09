/////////////////////////////////////////////////////////////////////////////
// sa_find.cpp:
// Implementation of the CDlgFind class.
// This class handles searches in the annotation window.
// Author: Doug Eberman
// copyright 1997 JAARS Inc. SIL
//
// Revision History
// 1.5Test11.0
//         SDM replaced reduntant function LeftEdgeOfAnnot() with GetOffset()
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgFind.h"
#include "Process\Process.h"
#include "Segment.h"
#include "sa_ipa.h"

#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "ch_dlg.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CDlgFind dialog

/////////////////////////////////////////////////////////////////////////////
// CDlgFind message map

BEGIN_MESSAGE_MAP(CDlgFind, CDialog)
    ON_BN_CLICKED(IDC_NEXT, OnNext)
    ON_BN_CLICKED(IDC_PREVIOUS, OnPrevious)
    ON_BN_CLICKED(IDC_CHARACTERCHART, OnCharacterChart)
    ON_CBN_SELCHANGE(IDC_CBOFIELD, OnSelchangeCbofield)
    ON_EN_CHANGE(IDC_INPUTSTRING, OnChangeInputstring)
    ON_EN_SETFOCUS(IDC_INPUTSTRING, OnSetFocusInputString)
    ON_EN_SETFOCUS(IDC_REPLACESTRING, OnSetFocusReplaceString)
    ON_BN_CLICKED(IDC_REPLACE, OnReplace)
    ON_BN_CLICKED(IDC_REPLACEALL, OnReplaceAll)
    ON_COMMAND(IDHELP, OnHelpFind)
    ON_CBN_SELCHANGE(IDC_CBOFIELD2, OnSelchangeCbofield2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFind construction/destruction/creation

/***************************************************************************/
// CDlgFind::CDlgFind Constructor
/***************************************************************************/
CDlgFind::CDlgFind(CWnd * pParent,
                   LPCTSTR pszFieldsToSearch,
                   const CString & strToFind,
                   bool bFindOnly,
                   const CString & strToReplace,
                   int idxDefaultField,
                   CMainFrame * pMainFrame) :
    m_sFieldsToSearch(pszFieldsToSearch),
    m_annotWndIndex(idxDefaultField),
    m_replaceStr(strToReplace),
    m_bFindOnly(bFindOnly),
    m_pMainFrame(pMainFrame),
    CDialog(CDlgFind::IDD, pParent) {

    m_strToFind = strToFind;
    m_breakOrBookMark = 0;

    if ((m_annotWndIndex < 0) || (m_annotWndIndex >= ANNOT_WND_NUMBER)) {
        m_annotWndIndex = 0;
    }

    if (m_annotWndIndex == GLOSS) {
        if (m_strToFind.GetAt(0) == TEXT_DELIMITER) {
            m_breakOrBookMark = 1;
        }
        m_strToFind = m_strToFind.Right(m_strToFind.GetLength() - 1); // delete delimiter
    }

    m_wrapped = false;
    m_beginFind = -1;
    m_curPos = -1;
    TRACE("bf:init\n");
    m_bCreated = Create(CDlgFind::IDD);
    if (m_bCreated) {
        ShowWindow(SW_SHOW);
    }
}

/***************************************************************************/
// CDlgFind::Completed - returns TRUE if the entire annotation window has
// been searched.
/***************************************************************************/
BOOL CDlgFind::Completed(BOOL isForward, int newPos) {
    BOOL ret = FALSE;

    if (isForward) {
        ret = ((m_wrapped) && (newPos >= m_beginFind));
    } else {
        ret = ((m_wrapped) && (newPos <= m_beginFind));
    }

    return ret;
}

/***************************************************************************/
// CDlgFind::ResetCompletionCheck - start a new search.
/***************************************************************************/
void CDlgFind::ResetCompletionCheck() {
    TRACE("bf:reset\n");
    m_beginFind = -1;
    m_wrapped = false;
}

/***************************************************************************/
// CDlgFind::GetFindString - get the string to search for.
/***************************************************************************/
CString CDlgFind::GetFindString() {
    UpdateData();
    return m_strToFind;
}

/***************************************************************************/
// CDlgFind::EnableDisable
/***************************************************************************/
void CDlgFind::EnableDisable() {
    BOOL glossMode = (AnnotationSetID() == GLOSS);
    GetDlgItem(IDC_CHARACTERCHART)->EnableWindow(!glossMode); // disable char. chart button
    GetDlgItem(IDC_INPUTGLOSS)->EnableWindow(glossMode); // disable selection
    GetDlgItem(IDC_INPUTBREAK)->EnableWindow(glossMode); // disable selection
}

/***************************************************************************/
// CDlgFind::SetEditFont
/***************************************************************************/
void CDlgFind::SetEditFont() {
    CSaView * pView = (CSaView *)m_pMainFrame->GetCurrSaView();
    CFont * pFont = pView->GetDocument()->GetFont(AnnotationSetID());

    if (pFont) {
        // change the font for the text controls
        GetDlgItem(IDC_INPUTSTRING)->SetFont(pFont);	// set the new font
        GetDlgItem(IDC_REPLACESTRING)->SetFont(pFont);	// set the new font
    }
}

/***************************************************************************/
// CDlgFind::DoDataExchange Data exchange
/***************************************************************************/
void CDlgFind::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_INPUTSTRING, m_strToFind);            // m_szFindWhat
    DDX_Control(pDX, IDC_CBOFIELD,  m_annotSetId);
    DDX_Radio(pDX, IDC_INPUTBREAK, m_breakOrBookMark);      // m_nSelect
    DDX_Text(pDX, IDC_REPLACESTRING, m_replaceStr);
    DDX_Control(pDX, IDC_CBOFIELD2, m_annotSetID2);
}

/***************************************************************************/
// CDlgFind::ScrollIfNeeded - scroll the annotation window if neccessary.
/***************************************************************************/
void CDlgFind::ScrollIfNeeded() {
    CSegment * pAnnot = m_pMainFrame->GetAnnotation(AnnotationSetID());

    CSaView * pView = (CSaView *)m_pMainFrame->GetCurrSaView();
    ASSERT(pView);
    if (pAnnot->NeedToScroll(*pView,m_curPos)) {
        DWORD desiredPosition = pAnnot->GetOffset(m_curPos);
        pView->Scroll(desiredPosition);
    }
}

void CDlgFind::SetupDialogForFindOnly() {
    // change the replace button title
    GetDlgItem(IDC_REPLACE)->SetWindowText(_T("Replace..."));
    // hide replace controls
    CRect r4(m_rctReplaceString);
    r4.bottom += r4.Height() * 2;
    GetDlgItem(IDC_REPLACESTRING)->MoveWindow(r4);
    GetDlgItem(IDC_REPLACESTRING)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_REPLACEALL)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC2)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_CBOFIELD)->ShowWindow(SW_HIDE);

    // move controls below replace controls up to fill
    // the space left by the replace controls.
    CRect r,r2;

    GetDlgItem(IDC_STATIC2)->GetWindowRect(r);
    ScreenToClient(r);
    GetDlgItem(IDC_STATIC3)->MoveWindow(r);

    // move more controls up.
    CSize sz1, sz2;
    sz1 = m_rctGlossGroup.TopLeft() -
          m_rctWordbreakRadio.TopLeft();
    sz2 = m_rctGlossGroup.TopLeft() -
          m_rctBookmarkRadio.TopLeft();

    r2 = CRect(m_rctSearchInCombo.left,
               m_rctSearchInCombo.top,
               m_rctSearchInCombo.left +
               m_rctGlossGroup.Width(),
               m_rctSearchInCombo.top +
               m_rctGlossGroup.Height());
    GetDlgItem(IDC_STATICGLOSS)->MoveWindow(r2);

    r = CRect(r2.left - sz1.cx,r2.top  - sz1.cy,
              r2.left - sz1.cx + m_rctWordbreakRadio.Width(),
              r2.top  - sz1.cy + m_rctWordbreakRadio.Height());
    GetDlgItem(IDC_INPUTBREAK)->MoveWindow(r);

    r = CRect(r2.left - sz2.cx, r2.top  - sz2.cy,
              r2.left - sz2.cx + m_rctBookmarkRadio.Width(),
              r2.top  - sz2.cy  + m_rctBookmarkRadio.Height());
    GetDlgItem(IDC_INPUTGLOSS)->MoveWindow(r);

    // move the IPA Chart, Close and Cancel buttons
    // and resize the dialog.
    int nMoveDist = m_rctGlossGroup.top - r2.top;
    r2 = m_rctChartButton;
    r2.OffsetRect(0, -nMoveDist);
    GetDlgItem(IDC_CHARACTERCHART)->MoveWindow(r2);

    r2 = m_rctCancelButton;
    r2.OffsetRect(0, -nMoveDist);
    GetDlgItem(IDCANCEL)->MoveWindow(r2);

    r2 = m_rctHelpButton;
    r2.OffsetRect(0, -nMoveDist);
    GetDlgItem(IDHELP)->MoveWindow(r2);

    r = m_rctDialog;
    r.bottom -= nMoveDist;
    MoveWindow(r);

}


void CDlgFind::SetupDialogForFindAndReplace() {
    CRect r,r2;

    UpdateData();

    // restore the dialog size
    GetWindowRect(r);
    r2 = CRect(r.left,r.top,r.left + m_rctDialog.Width(),r.top  + m_rctDialog.Height());
    MoveWindow(r2);

    // restore positions of controls, making room for restore controls
    GetDlgItem(IDC_INPUTGLOSS)->MoveWindow(m_rctBookmarkRadio);
    GetDlgItem(IDC_INPUTBREAK)->MoveWindow(m_rctWordbreakRadio);
    GetDlgItem(IDC_STATICGLOSS)->MoveWindow(m_rctGlossGroup);
    GetDlgItem(IDC_CHARACTERCHART)->MoveWindow(m_rctChartButton);
    GetDlgItem(IDCANCEL)->MoveWindow(m_rctCancelButton);
    GetDlgItem(IDHELP)->MoveWindow(m_rctHelpButton);
    GetDlgItem(IDC_STATIC3)->MoveWindow(m_rctSearchInText);
    GetDlgItem(IDC_REPLACESTRING)->MoveWindow(m_rctReplaceString);

    // show replace controls
    GetDlgItem(IDC_STATIC2)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_REPLACESTRING)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_REPLACEALL)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_CBOFIELD2)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_CBOFIELD)->ShowWindow(SW_SHOW);

    // fix replace control title
    GetDlgItem(IDC_REPLACE)->SetWindowText(_T("Replace"));

    m_annotSetId.SetCurSel(m_annotSetID2.GetCurSel());

    UpdateData(FALSE);
}

void CDlgFind::SaveDialogLayout() {
    // save locations of controls
    GetDlgItem(IDC_STATIC3)->GetWindowRect(m_rctSearchInText);
    ScreenToClient(m_rctSearchInText);
    GetDlgItem(IDC_CBOFIELD)->GetWindowRect(m_rctSearchInCombo);
    ScreenToClient(m_rctSearchInCombo);
    GetDlgItem(IDC_STATICGLOSS)->GetWindowRect(m_rctGlossGroup);
    ScreenToClient(m_rctGlossGroup);
    GetDlgItem(IDC_INPUTBREAK)->GetWindowRect(m_rctWordbreakRadio);
    ScreenToClient(m_rctWordbreakRadio);
    GetDlgItem(IDC_INPUTGLOSS)->GetWindowRect(m_rctBookmarkRadio);
    ScreenToClient(m_rctBookmarkRadio);
    GetDlgItem(IDCANCEL)->GetWindowRect(m_rctCancelButton);
    ScreenToClient(m_rctCancelButton);
    GetDlgItem(IDHELP)->GetWindowRect(m_rctHelpButton);
    ScreenToClient(m_rctHelpButton);
    GetDlgItem(IDC_CHARACTERCHART)->GetWindowRect(m_rctChartButton);
    ScreenToClient(m_rctChartButton);
    GetDlgItem(IDC_REPLACESTRING)->GetWindowRect(m_rctReplaceString);
    ScreenToClient(m_rctReplaceString);
    GetWindowRect(m_rctDialog);
}

int CDlgFind::AnnotationSetID() {
    if (m_bFindOnly) {
        return m_annotSetID2.GetCurSel();
    }
    return m_annotSetId.GetCurSel();
}

/***************************************************************************/
// CDlgFind::OnInitDialog Dialog initialisation
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CDlgFind::OnInitDialog() {
    CDialog::OnInitDialog();

    CenterWindow();

    // setup the annotation combo box (m_annotSetId)
    CString sRemaining(m_sFieldsToSearch);
    while (!sRemaining.IsEmpty()) {
        int idxNL = sRemaining.Find('\n');
        CString sTmp = sRemaining.Left((idxNL<0) ? sRemaining.GetLength() : idxNL);
        m_annotSetId.AddString(sTmp);
        if (idxNL < 0) {
            break;
        }
        sRemaining = sRemaining.Mid(idxNL+1);
    }
    m_annotSetId.SetCurSel(m_annotWndIndex);

    // setup the annotation combo box (m_annotSetId2)
    sRemaining = m_sFieldsToSearch;
    while (!sRemaining.IsEmpty()) {
        int idxNL = sRemaining.Find('\n');
        CString sTmp = sRemaining.Left((idxNL<0) ? sRemaining.GetLength() : idxNL);
        m_annotSetID2.AddString(sTmp);
        if (idxNL < 0) {
            break;
        }
        sRemaining = sRemaining.Mid(idxNL+1);
    }
    m_annotSetID2.SetCurSel(m_annotWndIndex);

    EnableDisable();
    SetEditFont();
    SaveDialogLayout();

    if (m_bFindOnly) {
        SetupDialogForFindOnly();
    } else {
        SetupDialogForFindAndReplace();
    }
	// set focus to edit control
    GotoDlgCtrl(GetDlgItem(IDC_INPUTSTRING)); 
    m_nTextBoxInFocus = IDC_INPUTSTRING;

    return FALSE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgFind::OnCharacterChart Input from character chart wanted
/***************************************************************************/
void CDlgFind::OnCharacterChart() {
    // save the edit box selection, just to be sure
    int nStartChar, nEndChar;
    DWORD dwSel = ((CEdit *)GetDlgItem(m_nTextBoxInFocus))->GetSel();
    nStartChar = LOWORD(dwSel);
    nEndChar = HIWORD(dwSel);

    // create the character chart property sheet
    CString szCaption;
    szCaption.LoadString(IDS_DLGTITLE_CHARCHART); // load caption string
    CDlgCharChart * pDlgCharChart;
    pDlgCharChart = new CDlgCharChart(szCaption, NULL, 0); // create the property sheet
    pDlgCharChart->SetInitialState(GetDlgItem(m_nTextBoxInFocus)->GetFont());
    if (pDlgCharChart->DoModal() == IDOK) {

        // insert the new string
        ((CEdit *)GetDlgItem(m_nTextBoxInFocus))->SetSel(nStartChar, nEndChar);
        ((CEdit *)GetDlgItem(m_nTextBoxInFocus))->ReplaceSel(pDlgCharChart->m_szString);
        UpdateData(TRUE);
    }
    if (pDlgCharChart) {
        delete pDlgCharChart;
    }
    GotoDlgCtrl(GetDlgItem(m_nTextBoxInFocus)); // set focus to edit control
}

/***************************************************************************/
// CDlgFind::OnSelchangeCbofield - adjust dialog and start a new search if
// the annotation window to search in changes.
/***************************************************************************/
void CDlgFind::OnSelchangeCbofield() {
    EnableDisable();
    SetEditFont();
    ResetCompletionCheck();
}

void CDlgFind::OnSelchangeCbofield2() {
    OnSelchangeCbofield();
}

/***************************************************************************/
// CDlgFind::OnChangeInputString - start a new search if find string changes
/***************************************************************************/
void CDlgFind::OnChangeInputstring() {
    ResetCompletionCheck();
    EnableSearch(!GetFindString().IsEmpty());
}

/***************************************************************************/
// CDlgFind::OnSetFocusInputString - track focus changes
/***************************************************************************/
void CDlgFind::OnSetFocusInputString() {
    m_nTextBoxInFocus = IDC_INPUTSTRING;
}

/***************************************************************************/
// CDlgFind::OnSetFocusReplaceString - track focus changes
/***************************************************************************/
void CDlgFind::OnSetFocusReplaceString() {
    m_nTextBoxInFocus = IDC_REPLACESTRING;
}

/***************************************************************************/
// CDlgFind::OnNext - find the next match of the find string
/***************************************************************************/
void CDlgFind::OnNext() {
    
	TRACE(">>>OnNext %d %d\n",m_curPos,m_beginFind);
    if (m_pMainFrame==NULL) {
        return;
    }

    CSaDoc * pDoc = m_pMainFrame->GetCurrDoc();
    if (pDoc==NULL) {
        return;
    }

    CString findme = GetFindString();
    if (findme.IsEmpty()) {
        return;
    }

    CSegment * pAnnot = m_pMainFrame->GetAnnotation(AnnotationSetID());
    if ((pAnnot->IsEmpty()) ||
            ((!m_bFindOnly) && (m_curPos==-1) && (pAnnot->FindNext(-1,findme)==-1))) {
        AfxMessageBox(IDS_FIND_NONE,MB_ICONINFORMATION,0);
        return;
    }

    // curSel will be selection index or
    // -1 if nothing is selected
    int curSel = pAnnot->GetSelection();

    TRACE("curSel=%d\n",curSel);
    TRACE("m_beginFind=%d\n",m_beginFind);
    TRACE("m_curPos=%d\n",m_curPos);

    // setup the beginning point, because we don't have one yet.
    // in this case we don't want to do a replace yet
    if (curSel==-1) {
        // there's nothing selected, and we haven't started yet
        if (m_beginFind==-1) {
            TRACE("bf:begin\n");
            m_wrapped = false;
            m_beginFind = pAnnot->FirstVisibleIndex(*pDoc);
            m_curPos = m_beginFind;
            curSel = m_curPos;
            TRACE("bf:begin2\n");
            if (pAnnot->Match(curSel, findme)) {
                ScrollIfNeeded();
                pDoc->SelectSegment(pAnnot,m_curPos);
            } else {
                TRACE("onnext1\n");
                OnNext();
            }
            TRACE("1\n");
            return;
        }
    } else {
        // next special case - initial selection has already been done
        if (m_beginFind==-1) {
            TRACE("bf:begin\n");
            m_wrapped = false;
            m_beginFind = curSel;
            m_curPos = m_beginFind;
            TRACE("onnext2\n");
            OnNext();
            TRACE("2\n");
            return;
        }
    }

    if ((m_curPos != -1) && (curSel != m_curPos)) {
        m_beginFind = pAnnot->GetSelection();
        TRACE("bf:test\n");
        if (m_beginFind == -1) {
            m_beginFind = pAnnot->FirstVisibleIndex(*pDoc);
            TRACE("bf:first visible %d\n",m_beginFind);
        }
        m_curPos = m_beginFind;
    }

    // find the next string
    int newPos = pAnnot->FindNext(m_curPos,findme);
    if (newPos >= 0) {
        if (Completed(TRUE, newPos)) {
            AfxMessageBox(IDS_FIND_FINISHED,MB_ICONINFORMATION,0);
            m_curPos = m_beginFind;
            ScrollIfNeeded();
            pDoc->SelectSegment(pAnnot,m_curPos);
            ResetCompletionCheck();
        } else {
            m_curPos = newPos;
            ScrollIfNeeded();
            pDoc->SelectSegment(pAnnot,m_curPos);
        }
    } else {
        if (AfxMessageBox(IDS_FIND_WRAP_PAST_END, MB_YESNO|MB_ICONQUESTION,0)==IDYES) {
            m_curPos = -1;
            m_wrapped = true;
            OnNext();
        } else {
            ResetCompletionCheck();
        }
    }
    TRACE("3\n");
}

/***************************************************************************/
// CDlgFind::OnPrevious - find the next match of the find string, searching
// backwards.
/***************************************************************************/
void CDlgFind::OnPrevious() {
    
	if (m_pMainFrame==NULL) {
        return;
    }
    CSaDoc * pDoc = m_pMainFrame->GetCurrDoc();
    if (pDoc==NULL) {
        return;
    }

    CString findme = GetFindString();
    if (findme.IsEmpty()) {
        return;
    }

    CSegment * pAnnot = m_pMainFrame->GetAnnotation(AnnotationSetID());
    if ((pAnnot->IsEmpty()) || (pAnnot->FindPrev(-1,findme)==-1)) {
        AfxMessageBox(IDS_FIND_NONE,MB_ICONINFORMATION,0);
        return;
    }

    int curSel = pAnnot->GetSelection();
    if ((m_beginFind == -1) || ((m_curPos != -1) && (curSel != m_curPos))) {
        m_beginFind = pAnnot->GetSelection();
        TRACE("bf:do\n");
        if (m_beginFind == -1) {
            m_beginFind = pAnnot->LastVisibleIndex(*pDoc);
            TRACE("bf:do2\n");
        }
        m_curPos = m_beginFind;
    }

    int newPos = pAnnot->FindPrev(m_curPos,findme);
    if (newPos >= 0) {
        if (Completed(FALSE, newPos)) {
            AfxMessageBox(IDS_FIND_FINISHED,MB_ICONINFORMATION,0);
            m_curPos = m_beginFind;
            ScrollIfNeeded();
            pDoc->SelectSegment(pAnnot,m_curPos);
            ResetCompletionCheck();
        } else {
            m_curPos = newPos;
            ScrollIfNeeded();
            pDoc->SelectSegment(pAnnot,m_curPos);
        }
    } else {
        if (AfxMessageBox(IDS_FIND_WRAP_PAST_START, MB_YESNO|MB_ICONQUESTION,0)==IDYES) {
            m_curPos = -1;
            m_wrapped = true;
            OnPrevious();
        } else {
            pDoc->SelectSegment(pAnnot,m_curPos);
            ResetCompletionCheck();
        }
    }
}

void CDlgFind::OnReplace() {
    if (m_bFindOnly) {
        m_bFindOnly = false;
        SetupDialogForFindAndReplace();
    } else {
        Replace();
    }
}

void CDlgFind::Replace() {
    if (m_pMainFrame==NULL) {
        return;
    }

    CSaDoc * pDoc = m_pMainFrame->GetCurrDoc();
    if (pDoc==NULL) {
        return;
    }

    CString findme = GetFindString();
    if (findme.IsEmpty()) {
        return;
    }

    CString replaceme = GetReplaceString();
    if (replaceme.IsEmpty()) {
        AfxMessageBox(IDS_EMPTY_REPLACE,MB_ICONINFORMATION,0);
        return;
    }

    CSegment * pAnnot = m_pMainFrame->GetAnnotation(AnnotationSetID());
    if (pAnnot->IsEmpty()) {
        AfxMessageBox(IDS_FIND_NONE,MB_ICONINFORMATION,0);
        return;
    }

    int curSel = pAnnot->GetSelection();
    if ((curSel >= 0) && (pAnnot->Match(curSel, findme))) {
        pDoc->CheckPoint();
        pAnnot->Replace(pDoc, curSel, findme, replaceme);
    }

    ResetCompletionCheck();
    OnNext();
}

/***************************************************************************/
// CDlgFind::GetReplaceString - get the string to use for replacment
/***************************************************************************/
CString CDlgFind::GetReplaceString() {
    UpdateData();
    return m_replaceStr;
};

void CDlgFind::OnReplaceAll() {

    if (m_pMainFrame==NULL) {
        return;
    }

    CSaDoc * pDoc = m_pMainFrame->GetCurrDoc();
    if (pDoc==NULL) {
        return;
    }

    CString findme = GetFindString();
    if (findme.IsEmpty()) {
        return;
    }

    CString replaceme = GetReplaceString();
    if (replaceme.IsEmpty()) {
        AfxMessageBox(IDS_EMPTY_REPLACE,MB_ICONINFORMATION,0);
        return;
    }

    CSegment * pAnnot = m_pMainFrame->GetAnnotation(AnnotationSetID());
    if (pAnnot->IsEmpty() || (pAnnot->FindNext(-1,findme)==-1)) {
        AfxMessageBox(IDS_FIND_NONE,MB_ICONINFORMATION,0);
        return;
    }

    int newPos = pAnnot->FindNext(-1,findme);
    pDoc->CheckPoint();
    while (newPos >= 0) {
        if (newPos != pAnnot->GetSelection()) {
            pDoc->SelectSegment(pAnnot,newPos);
        }
        pAnnot->ReplaceSelectedSegment(pDoc,replaceme);
        newPos = pAnnot->FindNext(newPos,findme);
    }
}


/***************************************************************************/
// CDlgFind::EnableSearch
/***************************************************************************/
void CDlgFind::EnableSearch(BOOL enable) {
    GetDlgItem(IDC_NEXT)->EnableWindow(enable);
    GetDlgItem(IDC_PREVIOUS)->EnableWindow(enable);
    GetDlgItem(IDC_REPLACE)->EnableWindow(enable);
}

/***************************************************************************/
// CDlgFind::OnHelpFind Call Find help
/***************************************************************************/
void CDlgFind::OnHelpFind() {
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/Edit/Find.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

BOOL CDlgFind::Created() const {
    return m_bCreated;
}

void CDlgFind::ChangeView() {
    OnSelchangeCbofield();
}
