// targview.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "sa_wbch.h"
#include "targview.h"
#include "mainfrm.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "doclist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTargViewDlg dialog

CDlgTargView::CDlgTargView(CMainFrame * pMainFrame)
    : CDialog(CDlgTargView::IDD, (CWnd *)pMainFrame) ,
      m_pMainFrame(pMainFrame),
      m_numTargs(0),
      m_selectionIndex(0)
{
    CSaDoc * pdoc = (CSaDoc *)m_pMainFrame->GetCurrSaView()->GetDocument();
    CFileStatus * pDocFileStat = pdoc->GetFileStatus();
    TCHAR * pDocFileName = pDocFileStat->m_szFullName;

    m_SrcView = pDocFileName;
    TrimPath(&m_SrcView);

    for (int i=0; i<MAX_TARGETS; i++)
    {
        m_targets[i] = NULL;
    }
}

void CDlgTargView::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SELECT_TARGVIEW, m_TargView);
    DDX_Text(pDX, IDC_SRCVIEW, m_SrcView);
}

BEGIN_MESSAGE_MAP(CDlgTargView, CDialog)
    ON_COMMAND(IDHELP, OnHelpEqualize)
END_MESSAGE_MAP()

/***************************************************************************/
// CTargViewDlg::SetupTargViewList
/***************************************************************************/
void CDlgTargView::SetupTargViewList()
{
    CSaDoc * pSrcDoc = (CSaDoc *)m_pMainFrame->GetCurrSaView()->GetDocument();
    CDocList doclst; // list of currently open docs

    m_numTargs = 0;

    for (CSaDoc * pdoc = doclst.pdocFirst(); pdoc; pdoc = doclst.pdocNext())
    {
        if ((pdoc != NULL) && (pdoc != pSrcDoc))
        {
            m_onlyTarg = pdoc->GetFileStatus()->m_szFullName;
            TrimPath(&m_onlyTarg);
            int index = m_TargView.AddString(m_onlyTarg);
            m_TargView.SetItemData(index, m_numTargs);
            POSITION pos = pdoc->GetFirstViewPosition();
            m_targets[m_numTargs] = (CSaView *)pdoc->GetNextView(pos);
            m_numTargs++;
        }
    }

    if (m_numTargs!=1)
    {
        m_onlyTarg.Empty();
    }
}

CSaView * CDlgTargView::Targ()
{
    CSaView * target = NULL;

    if (m_selectionIndex >= 0)
    {
        if (m_selectionIndex < m_numTargs)
        {
            target = m_targets[m_selectionIndex];
        }
    }

    return target;
}

/***************************************************************************/
// CTargViewDlg::OnOK() Button OK hit
/***************************************************************************/
void CDlgTargView::OnOK()
{
    UpdateData(TRUE);
    m_selectionIndex = (int)m_TargView.GetItemData(m_TargView.GetCurSel());
    CDialog::OnOK();
}

BOOL CDlgTargView::OnInitDialog()
{
    CDialog::OnInitDialog();

    CenterWindow(); // center dialog on recorder window
    SetupTargViewList();

    // change the font for the text controls
    CWnd * pWnd = GetDlgItem(IDC_DLG_EXPLANATION);
    CFont * pFont = pWnd->GetFont(); // get the standard font
    LOGFONT logFont;
    pFont->GetObject(sizeof(LOGFONT), (void *)&logFont); // fill up logFont
    // modify the logFont
    logFont.lfWeight = FW_NORMAL; // not bold
    m_Font.CreateFontIndirect(&logFont); // create the modified font
    pWnd->SetFont(&m_Font); // set the modified font

    m_TargView.SetCurSel(0);

    return TRUE;
}

void CDlgTargView::TrimPath(CString * pCStr)
{
    // calculate number of characters possible to display and limit the string
    const int nChars = 30;
    if (pCStr->GetLength() > nChars)   // file path is too long
    {
        *pCStr = pCStr->Left(3) + "..." + pCStr->Right(nChars - 6); // drive...rest
    }
}

/***************************************************************************/
// CTargViewDlg::OnHelpEqualize Call Export Table help
/***************************************************************************/
void CDlgTargView::OnHelpEqualize()
{
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Graphs/Equalize_Length.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

CString CDlgTargView::OnlyTarg()
{
    return m_onlyTarg;
}

int CDlgTargView::NumTargs()
{
    return m_numTargs;
}
