// pickover.cpp : implementation file
//

#include "stdafx.h"
#include "sa_minic.h"
#include "sa_graph.h"

#include "sa.h"
class CProcess;
#include "sa_plot.h"
#include "sa_annot.h"
#include "sa_mplot.h"
#include "pickover.h"
#include "sa_doc.h"
#include "sa_view.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgPickOver dialog


CDlgPickOver::CDlgPickOver(CWnd * pParent /*=NULL*/)
    : CDialog(CDlgPickOver::IDD, pParent) {
    for (int i=0; i<MAX_LIST_SIZE; i++) {
        m_bIsItemSelected[i] = FALSE;
    }
    m_NumItemsSelected = 0;
    m_numPlots = 0;
    for (int i=0; i<MAX_PLOTS; i++) {
        m_apPlot[i] = NULL;
    }

}

void CDlgPickOver::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SELECT_GRAPHS2PRINT,  m_SelectItems);
}

BEGIN_MESSAGE_MAP(CDlgPickOver, CDialog)
    ON_BN_CLICKED(IDC_CLEARALLGRAPHS, OnClearallgraphs)
    ON_BN_CLICKED(IDC_SELECTALLGRAPHS, OnSelectallgraphs)
    ON_COMMAND(IDHELP, OnHelpPickOverlay)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDlgPickOver message handlers

void CDlgPickOver::OnClearallgraphs() {
    for (int i=0; i< m_SelectItems.GetCount(); i++) {
        m_SelectItems.SetSel(i,FALSE);
    }
}

void CDlgPickOver::OnSelectallgraphs() {
    for (int i=0; i< m_SelectItems.GetCount(); i++) {
        m_SelectItems.SetSel(i,TRUE);
    }
}

/***************************************************************************/
// CDlgPickOver::OnHelpPickOverlay Call Overlay help
/***************************************************************************/
void CDlgPickOver::OnHelpPickOverlay() {
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Graphs/Overlay.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CDlgPickOver::SetupItemsList
/***************************************************************************/
void CDlgPickOver::SetupItemsList() {
    if (m_Mode_AddTrueRemoveFalse) {
        for (int i=0; i < m_numItems; i++) {
            ASSERT(m_apGraphs[i] && CGraphWnd::IsMergeableGraph(m_apGraphs[i]));

            // get the title
            CString szText;
            m_apGraphs[i]->GetWindowText(szText.GetBuffer(32), 32);
            szText.ReleaseBuffer();

            CSaView * pView = (CSaView *)m_apGraphs[i]->GetParent();
            CSaDoc  * pDoc  = pView->GetDocument();
            CString szDocTitle(pDoc->GetFilenameFromTitle().c_str());    // load file name
            szText += " - " + szDocTitle;

            int index = m_SelectItems.AddString(szText);
            m_SelectItems.SetItemData(index, i);
        }
    } else {
        m_numItems = 0;
        for (UINT i=1; i < m_numPlots; i++) {
            if (m_apPlot[i]) {
                m_numItems++;

                CString szText(m_apPlot[i]->GetPlotName());

                CSaApp * pApp = (CSaApp *)AfxGetApp();
                CDocument  * pDoc  = pApp->IsFileOpened(m_szDoc[i]);
                CSaDoc * pSaDoc = (CSaDoc *)pDoc;
                CString szDocTitle(pSaDoc->GetFilenameFromTitle().c_str()); // load file name
                szText += " - " + szDocTitle;

                int index = m_SelectItems.AddString(szText);
                m_SelectItems.SetItemData(index, i);
            }
        }
    }
}



/***************************************************************************/
// CDlgPickOver::OnOK
/***************************************************************************/
void CDlgPickOver::OnOK() {
    UpdateData(TRUE);

    m_NumItemsSelected = m_SelectItems.GetSelCount();
    m_SelectItems.GetSelItems(m_NumItemsSelected, m_selections);

    for (int i=0; i<MAX_LIST_SIZE; i++) {
        m_bIsItemSelected[i] = FALSE;
    }
    for (int j=0; j<m_NumItemsSelected; j++) {
        long graphIndex = m_SelectItems.GetItemData(m_selections[j]);
        m_bIsItemSelected[graphIndex] = TRUE;
    }

    CDialog::OnOK();
}


/***************************************************************************/
// CDlgPickOver::OnInitDialog Dialog initialisation
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CDlgPickOver::OnInitDialog() {
    CDialog::OnInitDialog();

    for (int i=0; i<MAX_LIST_SIZE; i++) {
        m_bIsItemSelected[i] = FALSE;
    }
    m_NumItemsSelected = 0;

    CenterWindow(); // center dialog on recorder window
    SetupItemsList();

    if (!m_Mode_AddTrueRemoveFalse) {
        GetDlgItem(IDC_HEADING)->SetWindowText(_T("Select Items to Remove"));
    }

    return TRUE;
}


void CDlgPickOver::ResetGraphsPtr() {
    m_numItems = 0;
}

/***************************************************************************/
// CDlgPickOver::SetGraphsPtr
/***************************************************************************/
void CDlgPickOver::SetGraphsPtr(CGraphWnd * pGraphs[], CGraphWnd * pExceptGraph) {
    ASSERT(pExceptGraph);
    int i = 0;
    while ((i < MAX_GRAPHS_NUMBER) && (m_numItems < MAX_LIST_SIZE)) {
        if (!pGraphs[i]) {
            ; // skip null graphs
        } else if (pGraphs[i] == pExceptGraph) {
            // don't allow adding an overlay to itself.
            ;
        } else if (pGraphs[i]->GetPlotID() == ID_GRAPHS_OVERLAY) {
            // don't allow adding an overlay to an overlay
            ;
        } else if (pExceptGraph->IsIDincluded(pGraphs[i]->GetPlotID())
                   &&
                   (((CSaView *)(pExceptGraph->GetParent()))
                    ==
                    ((CSaView *)(pGraphs[i]->GetParent()))
                   )
                  ) {
            // don't allow merging into an overlay an item which is already in the overlay.
            ;
        } else if (CGraphWnd::IsMergeableGraph(pGraphs[i])) {
            m_apGraphs[m_numItems++] = pGraphs[i];
        }
        i++;
    }
    m_Mode_AddTrueRemoveFalse = TRUE;
}

void CDlgPickOver::AddPlot(const CPlotWnd * pPlot, LPCTSTR szDoc) {
    ASSERT(pPlot);
    ASSERT(m_numPlots < MAX_PLOTS);
    m_apPlot[m_numPlots] = pPlot;
    m_szDoc[m_numPlots] = szDoc;
    m_numPlots++;

    m_Mode_AddTrueRemoveFalse = FALSE;
}

UINT CDlgPickOver::GraphsCount(void) {
    return m_numItems;
}

CGraphWnd * CDlgPickOver::GetNextSelectedGraph(int & pos) {
    CGraphWnd * pGraphRet = NULL;
    while ((!pGraphRet) && (pos < m_numItems)) {
        if (m_bIsItemSelected[pos]) {
            pGraphRet = m_apGraphs[pos];
        }
        pos++;
    }

    return pGraphRet;
}

void CDlgPickOver::ResetPlots() {
    for (int i=0; i<MAX_PLOTS; i++) {
        m_apPlot[i] = NULL;
        m_szDoc[i].Empty();
    }
    m_numPlots = 0;
}

const CPlotWnd * CDlgPickOver::GetPlot(int idx) {
    return m_apPlot[idx];
}
