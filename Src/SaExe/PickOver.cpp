// pickover.cpp : implementation file
//

#include "stdafx.h"
#include "sa_minic.h"
#include "sa_graph.h"

#include "sa.h"
class CDataProcess;
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
// CPickOverlayDlg dialog


CPickOverlayDlg::CPickOverlayDlg(CWnd* pParent /*=NULL*/)
: CDialog(CPickOverlayDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPickOverlayDlg)
	//}}AFX_DATA_INIT

	for (int i=0;i<MAX_LIST_SIZE;i++)
	{
		m_bIsItemSelected[i] = FALSE;
	}
	m_NumItemsSelected = 0;
	m_numPlots = 0;
	for (int i=0;i<MAX_PLOTS;i++)
	{
		m_apPlot[i] = NULL;
	}

}

void CPickOverlayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPickOverlayDlg)
	DDX_Control(pDX, IDC_SELECT_GRAPHS2PRINT,  m_SelectItems);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPickOverlayDlg, CDialog)
	//{{AFX_MSG_MAP(CPickOverlayDlg)
	ON_BN_CLICKED(IDC_CLEARALLGRAPHS, OnClearallgraphs)
	ON_BN_CLICKED(IDC_SELECTALLGRAPHS, OnSelectallgraphs)
	ON_COMMAND(IDHELP, OnHelpPickOverlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPickOverlayDlg message handlers

void CPickOverlayDlg::OnClearallgraphs()
{
	for (int i=0;i< m_SelectItems.GetCount(); i++)
	{
		m_SelectItems.SetSel(i,FALSE);
	}
}

void CPickOverlayDlg::OnSelectallgraphs()
{
	for (int i=0;i< m_SelectItems.GetCount(); i++)
	{
		m_SelectItems.SetSel(i,TRUE);
	}
}

/***************************************************************************/
// CPickOverlayDlg::OnHelpPickOverlay Call Overlay help
/***************************************************************************/
void CPickOverlayDlg::OnHelpPickOverlay()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/Graphs/Overlay.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CPickOverlayDlg::SetupItemsList
/***************************************************************************/
void CPickOverlayDlg::SetupItemsList()
{
	if (m_Mode_AddTrueRemoveFalse)
	{
		for (int i=0; i < m_numItems; i++)
		{
			ASSERT(m_apGraphs[i] && CGraphWnd::IsMergeableGraph(m_apGraphs[i]));

			// get the title
			CString szText;
			m_apGraphs[i]->GetWindowText(szText.GetBuffer(32), 32);
			szText.ReleaseBuffer();

			CSaView * pView = (CSaView *)m_apGraphs[i]->GetParent();
			CSaDoc  * pDoc  = pView->GetDocument();

			CString szDocTitle(pDoc->GetTitle()); // load file name
			int nFind = szDocTitle.Find(':');
			if (nFind != -1) szDocTitle = szDocTitle.Left(nFind); // extract part left of :
			szText += " - " + szDocTitle;

			int index = m_SelectItems.AddString(szText);
			m_SelectItems.SetItemData(index, i);
		}
	}
	else
	{
		m_numItems = 0;
		for (UINT i=1; i < m_numPlots; i++)
		{
			if (m_apPlot[i])
			{
				m_numItems++;

				CString szText(m_apPlot[i]->GetPlotName());

				CSaApp * pApp = (CSaApp *)AfxGetApp();
				CDocument  * pDoc  = pApp->IsFileOpened(m_szDoc[i]);

				CString szDocTitle(pDoc->GetTitle()); // load file name
				int nFind = szDocTitle.Find(':');
				if (nFind != -1) szDocTitle = szDocTitle.Left(nFind); // extract part left of :
				szText += " - " + szDocTitle;

				int index = m_SelectItems.AddString(szText);
				m_SelectItems.SetItemData(index, i);
			}
		}
	}
}



/***************************************************************************/
// CPickOverlayDlg::OnOK
/***************************************************************************/
void CPickOverlayDlg::OnOK()
{
	UpdateData(TRUE);

	m_NumItemsSelected = m_SelectItems.GetSelCount();
	m_SelectItems.GetSelItems(m_NumItemsSelected, m_selections);

	for (int i=0;i<MAX_LIST_SIZE;i++)
	{
		m_bIsItemSelected[i] = FALSE;
	}
	for (int j=0;j<m_NumItemsSelected;j++)
	{
		long graphIndex = m_SelectItems.GetItemData(m_selections[j]);
		m_bIsItemSelected[graphIndex] = TRUE;
	}

	CDialog::OnOK();
}


/***************************************************************************/
// CPickOverlayDlg::OnInitDialog Dialog initialisation
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CPickOverlayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for (int i=0;i<MAX_LIST_SIZE;i++)
	{
		m_bIsItemSelected[i] = FALSE;
	}
	m_NumItemsSelected = 0;

	CenterWindow(); // center dialog on recorder window
	SetupItemsList();

	if (!m_Mode_AddTrueRemoveFalse)
	{
		GetDlgItem(IDC_HEADING)->SetWindowText(_T("Select Items to Remove"));
	}

	return TRUE;
}


void CPickOverlayDlg::ResetGraphsPtr()
{
	m_numItems = 0;
}

/***************************************************************************/
// CPickOverlayDlg::SetGraphsPtr
/***************************************************************************/
void CPickOverlayDlg::SetGraphsPtr(CGraphWnd * pGraphs[], CGraphWnd * pExceptGraph)
{
	ASSERT(pExceptGraph);
	int i = 0;
	while ((i < MAX_GRAPHS_NUMBER) && (m_numItems < MAX_LIST_SIZE))
	{
		if (!pGraphs[i])
		{
			; // skip null graphs
		}
		else if (pGraphs[i] == pExceptGraph)
		{
			// don't allow adding an overlay to itself.
			;
		}
		else if (pGraphs[i]->GetPlotID() == ID_GRAPHS_OVERLAY)
		{
			// don't allow adding an overlay to an overlay
			;
		}
		else if (pExceptGraph->IsIDincluded(pGraphs[i]->GetPlotID())
			&&
			(   (  (CSaView *)(pExceptGraph->GetParent())  )
			==
			(  (CSaView *)(pGraphs[i]->GetParent())    )
			)
			)
		{
			// don't allow merging into an overlay an item which is already in the overlay.
			;
		}
		else if (CGraphWnd::IsMergeableGraph(pGraphs[i]))
		{
			m_apGraphs[m_numItems++] = pGraphs[i];
		}
		i++;
	}
	m_Mode_AddTrueRemoveFalse = TRUE;
}

void CPickOverlayDlg::AddPlot( const CPlotWnd * pPlot, LPCTSTR szDoc)
{
	ASSERT(pPlot);
	ASSERT(m_numPlots < MAX_PLOTS);
	m_apPlot[m_numPlots] = pPlot;
	m_szDoc[m_numPlots] = szDoc;
	m_numPlots++;

	m_Mode_AddTrueRemoveFalse = FALSE;
}

UINT CPickOverlayDlg::GraphsCount(void)
{
	return m_numItems;
}

CGraphWnd * CPickOverlayDlg::GetNextSelectedGraph(int & pos)
{
	CGraphWnd * pGraphRet = NULL;

	while (!pGraphRet && (pos < m_numItems))
	{
		if (m_bIsItemSelected[pos])
		{
			pGraphRet = m_apGraphs[pos];
		}
		pos++;
	}

	return pGraphRet;
}

void CPickOverlayDlg::ResetPlots()
{
	for (int i=0;i<MAX_PLOTS;i++)
	{
		m_apPlot[i] = NULL;
		m_szDoc[i].Empty();
	}
	m_numPlots = 0;
}

const CPlotWnd * CPickOverlayDlg::GetPlot(int idx)
{
	return m_apPlot[idx];
}
