// TaskBar.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "mainfrm.h"
#include "sa_view.h"
#include "TaskBar.h"
#include "graphsTypes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CTaskPage::~CTaskPage()
{
    delete m_pImageList[0];
    delete m_pImageList[1];
    delete m_pImageList[2];
}

/////////////////////////////////////////////////////////////////////////////
// CTaskBar dialog


IMPLEMENT_DYNAMIC(CTaskBar, CDialogBar)

CTaskBar::CTaskBar()
    : CDialogBar()
{
    m_nHotItem = -1;
    m_nSelectedPage = -1;
    //{{AFX_DATA_INIT(CTaskBar)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

CTaskBar::~CTaskBar()
{
    Clear();
}

void CTaskBar::Clear()
{
    m_nHotItem = -1;
    m_nSelectedPage = -1;
    while (m_pPages.size())
    {
        delete m_pPages.back();
        m_pPages.pop_back();
    }
    while (m_pPageButtons.size())
    {
        delete m_pPageButtons.back();
        m_pPageButtons.pop_back();
    }
}

void CTaskBar::DoDataExchange(CDataExchange * pDX)
{
    CDialogBar::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTaskBar)
    DDX_Control(pDX, IDC_TASK_LIST, m_cList);
    //}}AFX_DATA_MAP
}


// Adjust position of buttons & list based on current size and selected page
void CTaskBar::UpdateLayout()
{
    CRect rWnd;

    GetClientRect(rWnd);

    int nButtons = m_pPageButtons.size();
    CRect rList(rWnd);

    if (rList.Height() > nButtons*kButtonHeight)
    {
        rList.bottom = rList.bottom - nButtons*kButtonHeight;
    }
    else
    {
        rList.bottom = rList.top;
    }

    if (m_nSelectedPage >= 0)
    {
        rList.OffsetRect(0,(m_nSelectedPage+1)*kButtonHeight);
    }

    for (int i = 0; i < nButtons; i++)
    {
        // position top buttons
        CRect rButton(rWnd);

        rButton.top = i*kButtonHeight;
        rButton.top += i > m_nSelectedPage ? rList.Height() - 1 : 0;

        rButton.bottom = rButton.top + kButtonHeight;

        m_pPageButtons[i]->MoveWindow(rButton);
    }

    if (m_cList.m_hWnd)
    {
        m_cList.DeleteAllItems();

        CSize szSpacing;

        szSpacing.cy = 72;  // This spacing allows two lines of label with 32 pixel high icon
        szSpacing.cx = rList.Width();
        m_cList.SetIconSpacing(szSpacing);
        m_cList.MoveWindow(rList, TRUE);

        if (m_nSelectedPage >= 0)
        {
            CTaskPage * pPage = m_pPages[m_nSelectedPage];

            m_cList.SetImageList(pPage->m_pImageList[0], LVSIL_NORMAL);
            m_cList.SetImageList(pPage->m_pImageList[1], LVSIL_SMALL);
            m_cList.SetImageList(pPage->m_pImageList[2], LVSIL_STATE);

            // insert items
            for (int i=0; i < (int) pPage->m_cItemList.size(); i++)
            {
                CSaTaskItem & item = pPage->m_cItemList[i];

                item.pszText = item.szLabel.GetBuffer(0);

                int nResult = m_cList.InsertItem(&item);
                if (nResult != i)
                {
                    TRACE(_T("Item insert failed\n"));
                    break;
                }
            }

            // Shift items down
            for (int i=(int) pPage->m_cItemList.size()-1; i >= 0 ; i--)
            {
                POINT point;
                m_cList.GetItemPosition(i, &point);
                point.y += 6;
                m_cList.SetItemPosition(i, point);
            }
        }
    }
}

BEGIN_MESSAGE_MAP(CTaskBar, CDialogBar)
    //{{AFX_MSG_MAP(CTaskBar)
    ON_WM_SIZE()
    ON_WM_SYSCOLORCHANGE()
    ON_NOTIFY(NM_SETFOCUS, IDC_TASK_LIST, OnSetfocusTaskList)
    //}}AFX_MSG_MAP
    ON_NOTIFY(LVN_GETINFOTIP, IDC_TASK_LIST, OnGetInfoTip)
    ON_MESSAGE(WM_INITDIALOG, HandleInitDialog)
    ON_NOTIFY(NM_CLICK, IDC_TASK_LIST, OnTaskItem)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_TASK_LIST, OnDrawTaskItem)
    ON_COMMAND_RANGE(kSelectPageFirst, kSelectPageLast, OnSelectPage)
    ON_UPDATE_COMMAND_UI_RANGE(kSelectPageFirst, kSelectPageLast, OnSelectPageUpdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTaskBar message handlers

// Initialize dialog, set list style, update layout
LRESULT CTaskBar::HandleInitDialog(WPARAM, LPARAM)
{
    if (CDialogBar::HandleInitDialog(0,0))
    {
        return FALSE;
    }

    UpdateData(FALSE); // Attach List control

    m_cList.SetExtendedStyle(LVS_EX_ONECLICKACTIVATE | LVS_EX_INFOTIP);
    OnSysColorChange(); // Set colors

    UpdateLayout();

    return TRUE;
}

// The baseline function does not route commands to self, we do not route
// outside of self.
void CTaskBar::OnUpdateCmdUI(CFrameWnd * /*pTarget*/, BOOL bDisableIfNoHndler)
{
    // We handle the update for our own messages
    UpdateDialogControls(this, bDisableIfNoHndler);
}

// Filter these messages to handle items in ListView and allow
// buttons with ID which match any used within the app.  Minimize
// ID conflict management
BOOL CTaskBar::SetStatusText(int nHit)
{
    if (nHit >= kSelectPageFirst && nHit <= kSelectPageLast)
    {
        nHit = IDS_TASK_PAGE_BUTTON;
    }
    else
    {
        int item = m_cList.GetHotItem();
        if (item != -1)
        {
            nHit = m_cList.GetItemData(item);
            if (nHit >= ID_GRAPHTYPES_SELECT_FIRST && nHit <= ID_GRAPHTYPES_SELECT_LAST)
            {
                nHit = ID_GRAPHTYPES_SELECT_FIRST;
            }
        }
        else
        {
            nHit = -1;
        }
    }

    return CDialogBar::SetStatusText(nHit);
}


// Handle dialog resizing by updating the layout
void CTaskBar::OnSize(UINT nType, int cx, int cy)
{
    CDialogBar::OnSize(nType, cx, cy);

    if (m_hWnd && cx > 0 && cy > 0)
    {
        UpdateLayout();
    }
}

// Task item was clicked fire command
void CTaskBar::OnTaskItem(NMHDR * pNMHDR, LRESULT * pResult)
{
    NMLISTVIEW * pNMListView = (NMLISTVIEW *)pNMHDR;

    if (pNMListView->iItem >= 0)
    {
        GetOwner()->SendMessage(WM_COMMAND, m_cList.GetItemData(pNMListView->iItem));
    }

    m_cList.SetItemState(pNMListView->iItem, 0, UINT(-1)); // Clear any state setting

    *pResult = 0;
}

// Select page, update List view contents to new page
// hide focus rect on button & remove default button rect
void CTaskBar::OnSelectPage(UINT wPageID)
{
    int nPage = wPageID - kSelectPageFirst;

    if (nPage < (int) m_pPages.size() && m_nSelectedPage != nPage)
    {
        m_nSelectedPage = nPage;
        m_nHotItem = -1;

        UpdateLayout();
    }
}


// provide a default handler to avoid potential problems with base message maps
void CTaskBar::OnSelectPageUpdate(CCmdUI * /*pCmdUI*/)
{
}

// Create a page for the task bar
// The pPage item must be allocated using operator new()
// ownership/responsibility is transferred to CTaskBar
void CTaskBar::AddPage(CTaskPage * pPage)
{
    ASSERT(m_hWnd != NULL);

    CTaskButton * pButton =
        new CTaskButton(pPage->m_szPageName, this, kSelectPageFirst + m_pPages.size());

    pButton->SetFont(m_cList.GetFont());

    m_pPages.push_back(pPage);
    m_pPageButtons.push_back(pButton);

    if (m_nSelectedPage == -1)
    {
        m_nSelectedPage = 0;
    }
}

// Update task bar colors
void CTaskBar::OnSysColorChange()
{
    CDialogBar::OnSysColorChange();

    if (m_cList.m_hWnd)
    {
        m_cList.SetBkColor(GetSysColor(COLOR_3DSHADOW));
        m_cList.SetTextColor(GetSysColor(COLOR_WINDOW));
        m_cList.SetTextBkColor(GetSysColor(COLOR_3DSHADOW));
    }
}

// Manually add button rect for hovering
void CTaskBar::OnDrawTaskItem(NMHDR * pNMHDR, LRESULT * pResult)
{
    NMLVCUSTOMDRAW * lplvcd = (NMLVCUSTOMDRAW *)pNMHDR;
    NMCUSTOMDRAW & cd = lplvcd->nmcd;

    if (cd.dwDrawStage == CDDS_PREPAINT)
    {
        *pResult = CDRF_NOTIFYITEMDRAW; // We want to be notified as each item is drawn
        return;
    }

    if (cd.dwDrawStage != CDDS_ITEMPREPAINT)
    {
        *pResult = CDRF_DODEFAULT;
        return;
    }

    int i = cd.dwItemSpec;
    {
        if (m_cList.GetHotItem() != m_nHotItem)
        {
            int nOldHotItem = m_nHotItem;
            m_nHotItem = m_cList.GetHotItem();
            // Clear any stuck state information
            if (m_cList.GetItemState(nOldHotItem, UINT(-1)))
            {
                m_cList.SetItemState(nOldHotItem, 0, UINT(-1));    // Clear any state setting
            }
            // hover item changed, we need to force a redraw to erase the old hover item icon
            m_cList.RedrawItems(nOldHotItem, nOldHotItem);
            SetStatusText(-1);  //update the status bar too
        }

        if (i == m_nHotItem)
        {
            CDC * pDC = m_cList.GetDC();

            CRect rWnd;
            m_cList.GetItemRect(i, rWnd, LVIR_ICON);

            CPen penHilight(PS_SOLID, 0, GetSysColor(COLOR_3DFACE));
            CPen penShadow(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW));

            CPen * pOldPen = pDC->SelectObject(&penHilight);
            pDC->MoveTo(rWnd.left,rWnd.bottom);
            pDC->LineTo(rWnd.left,rWnd.top);
            pDC->LineTo(rWnd.right, rWnd.top);

            pDC->SelectObject(&penShadow);
            pDC->LineTo(rWnd.right, rWnd.bottom);
            pDC->LineTo(rWnd.left,rWnd.bottom);
            pDC->SelectObject(&pOldPen);
        }
    }
    lplvcd->clrText = GetSysColor(COLOR_WINDOW);
    lplvcd->clrTextBk = GetSysColor(COLOR_3DSHADOW);

    *pResult = CDRF_NEWFONT;
}




// Populate TaskBar with Icons, Labels, and Commands
void SetupTaskBar(CTaskBar & cTaskBar)
{
    static const LVITEM itemTemplate =
    {
        LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM | LVIF_STATE | LVIF_TEXT, // UINT mask
        0, // int iItem
        0, // int iSubItem
        0, // UINT state
        0xffffffff, // UINT stateMask
        NULL, // Label
        0, // int cchTextMax; (ignored)
        0, // iImage;
        0, // lParam
#if (_WIN32_IE >= 0x0300)
        0, // int iIndent;
#endif
#if (_WIN32_IE >= 0x560)
        0, // int iGroupId;
        0, // UINT cColumns; // tile view columns
        0, // PUINT puColumns;
#endif
    };

    cTaskBar.Clear();

    // Create page 1
    CTaskPage * pPage = new CTaskPage;

    pPage->m_szPageName = "Phonetic";
    pPage->m_pImageList[0] = new CImageList();

    pPage->m_pImageList[0]->Create(IDR_TASKBAR,30,32,RGB(192,192,192));

    for (int i = 0; i < 6; i++)
    {
        CSaTaskItem lvitem = itemTemplate;
        int nConfiguration = i;
        LPARAM nCommand = nConfiguration + ID_GRAPHTYPES_SELECT_FIRST;
        UINT anNewGraphID[MAX_GRAPHS_NUMBER];
        int nLayout = -1;

        CDlgGraphsTypes::GetStandardCheckedGraphs(nConfiguration, &anNewGraphID[0], &nLayout, &lvitem.szLabel, &lvitem.szTip);
        lvitem.iItem = i;
        lvitem.lParam = nCommand;
        lvitem.iImage = nLayout - ID_LAYOUT_FIRST;;

        pPage->m_cItemList.push_back(lvitem);
    }


    cTaskBar.AddPage(pPage);

    // Create page 1
    pPage = new CTaskPage;

    pPage->m_szPageName = "Music";
    pPage->m_pImageList[0] = new CImageList();

    pPage->m_pImageList[0]->Create(IDR_TASKBAR,30,32,RGB(192,192,192));

    for (int i = 0; i < 6; i++)
    {
        CSaTaskItem lvitem = itemTemplate;
        int nConfiguration = i + 6;
        LPARAM nCommand = nConfiguration + ID_GRAPHTYPES_SELECT_FIRST;
        UINT anNewGraphID[MAX_GRAPHS_NUMBER];
        int nLayout = -1;

        CDlgGraphsTypes::GetStandardCheckedGraphs(nConfiguration, &anNewGraphID[0], &nLayout, &lvitem.szLabel, &lvitem.szTip);
        lvitem.iItem = i;
        lvitem.lParam = nCommand;
        lvitem.iImage = nLayout - ID_LAYOUT_FIRST;;

        pPage->m_cItemList.push_back(lvitem);
    }

    cTaskBar.AddPage(pPage);
    cTaskBar.UpdateLayout();
}

void CTaskBar::OnSetfocusTaskList(NMHDR * pNMHDR, LRESULT * pResult)
{
    UNUSED_ALWAYS(pNMHDR);
    CMainFrame * pWnd = (CMainFrame *)AfxGetMainWnd();

    CSaView * pView = pWnd->GetCurrSaView();

    if (pView)
    {
        if (pView->GetFocusedGraphWnd())
        {
            ((CWnd *)pView->GetFocusedGraphWnd())->SetFocus();
        }
        else
        {
            pView->SetFocus();
        }
    }
    else
    {
        pWnd->SetFocus();
    }

    *pResult = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CTaskButton

CTaskButton::CTaskButton(LPCTSTR szCaption, CWnd * pParent, UINT nID)
{
    m_bSelected = FALSE;
    Create(szCaption, WS_CHILD|WS_VISIBLE|BS_OWNERDRAW|BS_NOTIFY, CRect(0,0,0,0), pParent, nID);
}

CTaskButton::~CTaskButton()
{
}


BEGIN_MESSAGE_MAP(CTaskButton, CButton)
    //{{AFX_MSG_MAP(CTaskButton)
    ON_WM_SETFOCUS()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTaskButton message handlers

void CTaskButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CDC cDC;

    cDC.Attach(lpDrawItemStruct->hDC);

    BOOL bUp = (lpDrawItemStruct->itemState & ODS_SELECTED) == 0;

    if (bUp && m_bSelected && (lpDrawItemStruct->itemState & ODS_FOCUS))
    {
        GetParent()->SetFocus();
    }

    m_bSelected = !bUp;

    CRect r(lpDrawItemStruct->rcItem);

    // Draw outer rect
    cDC.Draw3dRect(&r,
                   GetSysColor(bUp ? COLOR_3DHILIGHT : COLOR_3DDKSHADOW),
                   GetSysColor(!bUp ? COLOR_3DHILIGHT : COLOR_3DDKSHADOW));

    r.DeflateRect(1,1);

    cDC.Draw3dRect(&r,
                   GetSysColor(bUp ? COLOR_3DLIGHT : COLOR_3DSHADOW),
                   GetSysColor(!bUp ? COLOR_3DLIGHT : COLOR_3DSHADOW));

    CString szCaption;

    GetWindowText(szCaption);
    cDC.SetTextColor(GetSysColor(COLOR_BTNTEXT));
    cDC.SetBkColor(GetSysColor(COLOR_BTNFACE));
    cDC.DrawText(szCaption, &r, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}


void CTaskBar::OnGetInfoTip(NMHDR * pNMHDR, LRESULT * pResult)
{
    NMLVGETINFOTIP * pInfo = (NMLVGETINFOTIP *)pNMHDR;
    CTaskPage * pPage = m_pPages[m_nSelectedPage];
    if (pPage->m_cItemList.size() >= (unsigned) pInfo->iItem)
    {
        wcsncpy_s(pInfo->pszText, pInfo->cchTextMax, pPage->m_cItemList[pInfo->iItem].szTip, pInfo->cchTextMax);
    }

    *pResult = 0;
}
