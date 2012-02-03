/////////////////////////////////////////////////////////////////////////////
// graphsTypes.cpp:
// Implementation of the CDlgGraphsTypesCustom (dialog)
//                       CDlgGraphsTypes (property sheet)
//
// Authors: Urs Ruchti, Steve MacLean
// copyright 1997-2002 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "graphsTypes.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "sa_view.h"
#include "sa_mplot.h"
#include "mainfrm.h"
#include "settings\obstream.h"

//###########################################################################
// CDlgGraphsTypesCustom dialog
// Dialog to choose, which graphs the user wants to display.

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsTypesCustom message map

BEGIN_MESSAGE_MAP(CDlgGraphsTypesCustom, CDialog)
	//{{AFX_MSG_MAP(CDlgGraphsTypesCustom)
	ON_LBN_SELCHANGE(IDC_LAYOUT, OnSelchangeLayout)
	ON_BN_CLICKED(IDC_ORDER, OnOrder)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(BN_CLICKED, IDD_SPECTROGRAM, IDD_SPECTROGRAM, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_SNAPSHOT, IDD_SNAPSHOT, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_SPECTRUM, IDD_SPECTRUM, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_CHANGE, IDD_CHANGE, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_CHPITCH, IDD_CHPITCH, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_DURATION, IDD_DURATION, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_GRAPITCH, IDD_GRAPITCH, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_LOUDNESS, IDD_LOUDNESS, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_PITCH, IDD_PITCH, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_RAWDATA, IDD_RAWDATA, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_SMPITCH, IDD_SMPITCH, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_ZCROSS, IDD_ZCROSS, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_POA, IDD_POA, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_3D, IDD_3D, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_F1F2, IDD_F1F2, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_F2F1F1, IDD_F2F1F1, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_F2F1, IDD_F2F1, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_RATIO, IDD_RATIO, OnGraphCheck)
	ON_BN_CLICKED(IDD_MELOGRAM, OnMelogram)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_TWC, IDD_TWC, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_MAGNITUDE, IDD_MAGNITUDE, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_RECORDING, IDD_RECORDING, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_GLOTWAVE, IDD_GLOTWAVE, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_STAFF, IDD_STAFF, OnGraphCheck)
	ON_CONTROL_RANGE(BN_CLICKED, IDD_STATIC, IDD_STATIC, OnGraphCheck)
	ON_COMMAND(IDHELP, OnHelpGraphsTypes)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsTypesCustom construction/destruction/creation

/***************************************************************************/
// CDlgGraphsTypesCustom::CDlgGraphsTypesCustom Constructor
/***************************************************************************/
CDlgGraphsTypesCustom::CDlgGraphsTypesCustom(CWnd* pParent) : CDialog(CDlgGraphsTypesCustom::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgGraphsTypesCustom)
	m_nLayout = -1;
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsTypesCustom helper functions

/***************************************************************************/
// CDlgGraphsTypesCustom::DoDataExchange Data exchange
/***************************************************************************/
void CDlgGraphsTypesCustom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgGraphsTypesCustom)
	DDX_LBIndex(pDX, IDC_LAYOUT, m_nLayout);
	//}}AFX_DATA_MAP
}

/***************************************************************************/
// 10/12/2000 - DODO
/***************************************************************************/
static UINT anGraphIDs[] =
{
	IDD_RAWDATA,          IDD_LOUDNESS,
	IDD_DURATION,         IDD_CHANGE,
	IDD_GLOTWAVE,         IDD_PITCH,
	IDD_CHPITCH,          IDD_SMPITCH,
	IDD_GRAPITCH,         IDD_SPECTROGRAM,
	IDD_SNAPSHOT,         IDD_ZCROSS,
	IDD_SPECTRUM,         IDD_RATIO,
	IDD_F1F2,             IDD_F2F1,
	IDD_F2F1F1,           IDD_3D,
	IDD_MELOGRAM,         IDD_TWC,
	IDD_MAGNITUDE,        IDD_STAFF,
	IDD_POA,              IDD_RECORDING,
	IDD_CEPPITCH,         IDD_SDP_A,
	IDD_SDP_B,            IDD_INVSDP,
	IDD_WAVELET,															//7/30/01 ARH - Added for the wavelet graph
	NULL
};

/***************************************************************************/
// CDlgGraphsTypesCustom::GetCheckedGraphs Return selected graphs
// The IDs of the graphs, selected by the user in the dialog, and stored in
// the dialogs private graph ID array will be copied in a graph ID array,
// which address has been given as parameter.
/***************************************************************************/
void CDlgGraphsTypesCustom::GetCheckedGraphs(UINT* pGraphIDs, int *nLayout)
{
	m_cGraphs.GetGraphs(pGraphIDs);
	if(m_nLayout >= 0)
		*nLayout = m_Layout.GetLayoutFromSelection(m_nLayout);
	else
		*nLayout = -1;
}

/***************************************************************************/
// CDlgGraphsTypesCustom::OnGraphCheck, 10/12/2000 - DDO
/***************************************************************************/
void CDlgGraphsTypesCustom::OnGraphCheck(UINT nID)
{
	if(nID)
		m_cGraphs.SelectGraph(nID, IsDlgButtonChecked(nID));

	//**********************************************************
	// If more than six graph types were checked tell the user
	// that's a no-no.
	//**********************************************************
	if (nID && m_cGraphs.GetCount(TRUE) > 6)
	{
		TCHAR *szMsg = _T("With the exception of the graphs TWC, music's magnitude and\nposition view, you may only choose six graph types to display.");
		AfxMessageBox(szMsg, MB_OK | MB_ICONEXCLAMATION);

		CheckDlgButton(nID, FALSE);
		if(nID == IDD_MELOGRAM)
			OnMelogram();
		return;
	}

	m_Layout.SelectLayoutWithCount(m_cGraphs.GetCount(TRUE));

	CWnd *pOrder = GetDlgItem(IDC_ORDER);
	if(pOrder)
		pOrder->EnableWindow(m_cGraphs.GetCount(TRUE) > 1);
}

void CDlgGraphsTypesCustom::CheckDlgButton(int nIDButton, UINT nCheck, BOOL bDefaultOrder)
{
	CDialog::CheckDlgButton(nIDButton, nCheck);

	m_cGraphs.SelectGraph(nIDButton, nCheck, bDefaultOrder);
}

/***************************************************************************/
// CDlgGraphsTypesCustom::OnMelogram - TCJ 4/12/2000
/***************************************************************************/
void CDlgGraphsTypesCustom::OnMelogram()
{
	BOOL bMelChecked = (BOOL)IsDlgButtonChecked(IDD_MELOGRAM);
	GetDlgItem(IDD_TWC)->EnableWindow(bMelChecked);
	GetDlgItem(IDD_MAGNITUDE)->EnableWindow(bMelChecked);
	CheckDlgButton(IDD_TWC, bMelChecked);
	CheckDlgButton(IDD_MAGNITUDE, bMelChecked);
	if (bMelChecked)
		OnGraphCheck(IDD_MELOGRAM);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsTypesCustom message handlers

/***************************************************************************/
// CDlgGraphsTypesCustom::OnInitDialog Dialog initialization
// The dialog is centered on the main frame and then the graphs from the
// graph ID array are checked on the dialog.
/***************************************************************************/
BOOL CDlgGraphsTypesCustom::OnInitDialog()
{
	CDialog::OnInitDialog();

	// subclass the control
	VERIFY(m_Layout.SubclassDlgItem(IDC_LAYOUT, this));

	CenterWindow(); // center dialog on frame window


	//************************************************
	// get pointer to view and document
	//************************************************
	CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	CSaView* pView = (CSaView*)pMDIFrameWnd->GetCurrSaView();
	m_pDoc = (CSaDoc*)pView->GetDocument();       // get pointer to doc - TCJ 4/12/00
	UINT* pGraphID = pView->GetGraphIDs();        // get graph ID array view member data

	// check the used graph checkboxes
	UINT nID;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		nID = pGraphID[nLoop];
		if (nID == ID_GRAPHS_OVERLAY)
		{
			// If overlay present check the base graph type...
			CMultiPlotWnd *pPlot = (CMultiPlotWnd *)pView->GetGraph(nLoop)->GetPlot();
			nID = pPlot->GetBasePlotID();
		}
		if (nID) CheckDlgButton(nID, TRUE, FALSE);
	}

	//**************************************************
	// If the melogram graph is not initially checked
	// then disable the TWC and magnitude check boxes.
	//**************************************************
	if (!(BOOL)IsDlgButtonChecked(IDD_MELOGRAM))
	{
		GetDlgItem(IDD_TWC)->EnableWindow(FALSE);
		GetDlgItem(IDD_MAGNITUDE)->EnableWindow(FALSE);
	}

	OnGraphCheck(0);  // Handle graph list change related updates

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgGraphsTypesCustom::OnSelchangeLayout()
{
	UpdateData(TRUE);
}

void CDlgGraphsTypesCustom::OnOrder() 
{
	CDlgGraphTypesOrder cOrder(m_cGraphs, m_Layout.GetLayoutFromSelection(m_nLayout), this);
	if (cOrder.DoModal() == IDOK)
	{
		EndDialog(IDOK);
	}
}

/***************************************************************************/
// CDlgGraphsTypesCustom::OnOK OK button pressed
//   Ensure at least one graph type is selected before closing
/***************************************************************************/
void CDlgGraphsTypesCustom::OnOK()
{
	if (m_cGraphs.GetCount(TRUE) == 0)  m_cGraphs.SelectGraph(IDD_RAWDATA); // no graph selected: set raw data
	EndDialog(IDOK);
}

/***************************************************************************/
// CDlgGraphsTypesCustom::OnHelpGraphsTypes Call Graphs Types help
/***************************************************************************/
void CDlgGraphsTypesCustom::OnHelpGraphsTypes()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/Graphs/Graph_Types/Custom_Graph_Types.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CGraphList

//**********************************************************
// CGraphList::CGraphList
//**********************************************************
CGraphList::CGraphList(const UINT *pGraphIDs)
{
	if(pGraphIDs)
		SetGraphs(pGraphIDs);
	else
		ClearGraphs();
}

//**********************************************************
// CGraphList::CGraphList
//**********************************************************
CGraphList::CGraphList(const CGraphList &src)
{
	SetGraphs(src.m_nGraphID);
}

//**********************************************************
// CGraphList::GetCount(BOOL bLayoutOnly) const
// Count all checked graph types optionally excluding
// graphs which do not consume layout positions.
//**********************************************************
int CGraphList::GetCount(BOOL bLayoutOnly) const
{
	int nGraphCount = 0;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		UINT nID = m_nGraphID[nLoop];
		if (nID)
			if(!bLayoutOnly || IsLayoutGraph(nID))
				nGraphCount++;
	}

	return nGraphCount;
}

//**********************************************************
// CGraphList::ClearGraphs
//**********************************************************
void CGraphList::ClearGraphs(BOOL bFilter, BOOL bLayout)
{
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		BOOL bLayoutGraph = IsLayoutGraph(m_nGraphID[nLoop]);
		BOOL bClear = !bFilter || (bLayout ? bLayoutGraph : !bLayoutGraph);
		if(bClear)
			m_nGraphID[nLoop] = 0;
	}
	CompactList();
}

void CGraphList::CompactList()
{
	int nFound = 0;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if(m_nGraphID[nLoop] != 0)
		{
			if(nLoop != nFound)
			{
				m_nGraphID[nFound] = m_nGraphID[nLoop];
				m_nGraphID[nLoop] = 0;
			}      
			nFound++;
		}
	}
}
//**********************************************************
// CGraphList::SetGraphs
//**********************************************************
void CGraphList::SetGraphs(const UINT *pGraphIDs, BOOL bClear)
{
	if(bClear)
		ClearGraphs();

	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		SelectGraph(pGraphIDs[nLoop], TRUE, FALSE); // Copy order intact
	}
}

//**********************************************************
// CGraphList::GetGraphs
//**********************************************************
void CGraphList::GetGraphs(UINT *pGraphIDs) const
{
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		*(pGraphIDs + nLoop) = m_nGraphID[nLoop]; // return own graph ID array
	}
}

//**********************************************************
// CGraphList::SelectGraph
//**********************************************************
void CGraphList::SelectGraph(UINT nID, BOOL bSelect, BOOL bDefaultOrder)
{
	if(bSelect)
	{
		int nIndex = IsSelected(nID);

		if(nIndex >= 0)
			return;

		if(!IsLayoutGraph(nID))
		{
			// non-layout graphs are grouped before layout so as not to 
			// interfere with layout order comparison
			InsertAt(nID, 0);
			return;
		}

		int nOrder = GraphDefaultOrder(nID);
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
		{
			if(m_nGraphID[nLoop] == 0)
			{
				m_nGraphID[nLoop] = nID;
				return;
			}
			if(bDefaultOrder && nOrder < GraphDefaultOrder(m_nGraphID[nLoop]))
			{
				InsertAt(nID, nLoop);
				return;
			}
		}
	}
	else
	{
		int nIndex = IsSelected(nID);

		if(nIndex < 0)
			return;

		for (int nLoop = nIndex + 1; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
			m_nGraphID[nLoop - 1] = m_nGraphID[nLoop];
	}
}

void CGraphList::InsertAt(UINT nID, int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < MAX_GRAPHS_NUMBER);

	for (int nLoop = MAX_GRAPHS_NUMBER - 1; nLoop > nIndex; nLoop--)
	{
		m_nGraphID[nLoop] = m_nGraphID[nLoop - 1];
	}
	m_nGraphID[nIndex] = nID;
}

int CGraphList::IsSelected(UINT nID) const 
{
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if(m_nGraphID[nLoop] == nID)
			return nLoop;
	}
	return -1;
}

bool CGraphList::operator==(const CGraphList &compare) const 
{
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		UINT nID = m_nGraphID[nLoop];
		if(IsLayoutGraph(nID))
		{
			if(m_nGraphID[nLoop] != compare.m_nGraphID[nLoop])
				return FALSE;
		}
		else
		{
			// order of non-layout graphs does not effect equality
			// non-layout graphs are grouped before layout so as not to 
			// interfere with layout order comparison
			if(compare.IsSelected(nID) == -1)
				return FALSE;
		}
	}
	return TRUE;
}


int CGraphList::GraphDefaultOrder(UINT nID)
{
	if(!IsLayoutGraph(nID))
		return -1;

	switch(nID)
	{
	case IDD_RAWDATA:
		return 0;
	case IDD_MELOGRAM:
		return 1;
	case IDD_STAFF:
		return 2;
	}
	return 999;  // large number
}

CSaString CGraphList::GetDescription() const
{
	const CGraphList &cLayout = *this;
	CSaString szDescription;

	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++)
	{
		UINT nID = cLayout[i];

		if(!nID)
			continue;

		if(IsLayoutGraph(nID) || nID == IDD_RECORDING)
		{
			if(!szDescription.IsEmpty())
				szDescription += _T(", ");

			szDescription += CSaView::GetGraphTitle(nID);

			if(nID == IDD_MELOGRAM)
			{
				if(cLayout.IsSelected(IDD_TWC) >= 0)
				{
					szDescription += _T(", ");
					szDescription += CSaView::GetGraphTitle(IDD_TWC);
				}
				if(cLayout.IsSelected(IDD_MAGNITUDE) >= 0)
				{
					szDescription += _T(", ");
					szDescription += CSaView::GetGraphTitle(IDD_MAGNITUDE);
				}
			}
		}
	}

	return szDescription;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphTypesOrder dialog


CDlgGraphTypesOrder::CDlgGraphTypesOrder(CGraphList &cOrder, int nLayout, CWnd* pParent)
: CDialog(CDlgGraphTypesOrder::IDD, pParent), m_cResultOrder(cOrder), m_nLayout(nLayout)
{
	m_cLayoutOrder = GetLayoutOrder(cOrder);
	//{{AFX_DATA_INIT(CDlgGraphTypesOrder)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgGraphTypesOrder::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgGraphTypesOrder)
	DDX_Control(pDX, IDC_LIST, m_cList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgGraphTypesOrder, CDialog)
	//{{AFX_MSG_MAP(CDlgGraphTypesOrder)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	ON_WM_DRAWITEM()
	ON_COMMAND(IDHELP, OnHelpGraphTypesOrder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphTypesOrder message handlers

void CDlgGraphTypesOrder::OnDown() 
{
	int nSelection = m_cList.GetCurSel();

	if(nSelection >= m_cList.GetCount() - 1)
		return; // can not move down

	SwapLayoutGraphs(nSelection);

	OnGraphListChanged();

	m_cList.SetCurSel(nSelection + 1);
	OnSelchangeList();  // Update buttons enables
}

void CDlgGraphTypesOrder::OnUp() 
{
	int nSelection = m_cList.GetCurSel();

	if(nSelection <= 0)
		return;  // Can not move up

	SwapLayoutGraphs(nSelection - 1);

	OnGraphListChanged();

	m_cList.SetCurSel(nSelection - 1);
	OnSelchangeList();  // Update buttons enables
}

void CDlgGraphTypesOrder::OnSelchangeList() 
{
	int nSelection = m_cList.GetCurSel();

	CWnd *pUp = GetDlgItem(IDC_UP);
	if(pUp)
		pUp->EnableWindow(nSelection > 0);
	CWnd *pDown = GetDlgItem(IDC_DOWN);
	if(pDown)
		pDown->EnableWindow(nSelection < m_cList.GetCount() - 1);	
}

BOOL CDlgGraphTypesOrder::OnInitDialog() 
{
	CDialog::OnInitDialog();

	OnSelchangeList();
	OnGraphListChanged();
	OnDrawLayout();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDlgGraphTypesOrder::OnOK() 
{
	ApplyLayoutOrder();  // Apply changes

	CDialog::OnOK();
}

/***************************************************************************/
// CDlgGraphTypesOrder::OnHelpGraphTypesOrder Call Graph Types Order help
/***************************************************************************/
void CDlgGraphTypesOrder::OnHelpGraphTypesOrder()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/Graphs/Graph_Types/Graph_Types_Order.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

void CDlgGraphTypesOrder::OnGraphListChanged()
{
	OnPopulateList();
	// The shape of the layout could be affected by the graph order
	OnDrawLayout();  
}

void CDlgGraphTypesOrder::OnDrawLayout() 
{
	CWnd *pWnd = GetDlgItem(IDC_PICTURE);

	ASSERT(pWnd);
	CClientDC dc(pWnd);

	CDC *pDC = &dc;

	CRect rDraw;

	pWnd->GetClientRect(rDraw);

	// Erase
	pDC->FillSolidRect(rDraw, pDC->GetBkColor());

	for(int nGraph = 0; nGraph < CSaView::GetNumberOfGraphsInLayout(m_nLayout); nGraph++)
	{
		CRect rGraph;

		CSaView::GetGraphSubRect(m_nLayout, &rDraw, &rGraph, nGraph, m_cLayoutOrder.GetGraphs());

		pDC->MoveTo(rGraph.left, rGraph.bottom);
		pDC->LineTo(rGraph.left, rGraph.top);
		pDC->LineTo(rGraph.right, rGraph.top);
		pDC->LineTo(rGraph.right, rGraph.bottom);
		pDC->LineTo(rGraph.left, rGraph.bottom);

		CString szNumber;
		szNumber.Format(_T("%d"), nGraph + 1);
		pDC->DrawText(szNumber, rGraph, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
}

void CDlgGraphTypesOrder::OnPopulateList() 
{
	int nCount = m_cLayoutOrder.GetCount();

	m_cList.ResetContent();

	for (int i = 0; i < nCount; i++)
	{
		CString szDescription;
		UINT nID = m_cLayoutOrder[i];

		szDescription.Format(_T("%d. "), i+1);
		szDescription += CSaView::GetGraphTitle(nID);

		if(nID == IDD_MELOGRAM)
		{
			if(m_cResultOrder.IsSelected(IDD_TWC) >= 0)
			{
				szDescription += _T(", ");
				szDescription += CSaView::GetGraphTitle(IDD_TWC);
			}
			if(m_cResultOrder.IsSelected(IDD_MAGNITUDE) >= 0)
			{
				szDescription += _T(", ");
				szDescription += CSaView::GetGraphTitle(IDD_MAGNITUDE);
			}
		}

		m_cList.AddString(szDescription);
	}
}

void CDlgGraphTypesOrder::ApplyLayoutOrder()
{
	// remove layout graphs from result list
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		int nID = m_cResultOrder[nLoop];

		if(nID && CGraphList::IsLayoutGraph(nID))
		{
			m_cResultOrder.SelectGraph(nID, FALSE);  // remove graph
			nLoop--; // We removed a graph
		}
	}

	// insert the layout graphs in order
	m_cResultOrder.SetGraphs(m_cLayoutOrder.GetGraphs(), FALSE);
}

CGraphList CDlgGraphTypesOrder::GetLayoutOrder(const CGraphList &src)
{
	CGraphList cResult;

	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		UINT nID = src[nLoop];

		if(nID && CGraphList::IsLayoutGraph(nID))
			cResult.SelectGraph(nID, TRUE, FALSE); // Copy Order intact.
	}

	return cResult;
}

void CDlgGraphTypesOrder::SwapLayoutGraphs(int nFirst, int nSecond)
{
	ASSERT(nFirst >= 0 && nFirst < MAX_GRAPHS_NUMBER);

	if (nSecond < 0)
		nSecond = nFirst +  1;

	ASSERT(nSecond >= 0 && nSecond < MAX_GRAPHS_NUMBER);

	UINT  nGraphs[MAX_GRAPHS_NUMBER];  // array of graph IDs

	m_cLayoutOrder.GetGraphs(nGraphs);

	std::swap(nGraphs[nFirst], nGraphs[nSecond]);

	m_cLayoutOrder.SetGraphs(nGraphs);
}

void CDlgGraphTypesOrder::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if(nIDCtl == IDC_PICTURE)
		OnDrawLayout();
	else	
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

static const char* psz_Configuration = "GraphConfiguration";
static const char* psz_Layout      = "Layout";
static const char* psz_LayoutGraph = "LayoutGraph";
static const char* psz_OtherGraph = "OtherGraph";
static const char* psz_Predefined      = "Predefined";

// Write spectrumParm properties to stream
void CGraphConfiguration::WriteProperties(Object_ostream& obs, BOOL bPredefined) const
{
	UNUSED_ALWAYS(bPredefined);

	obs.WriteBeginMarker(psz_Configuration, GetDescription());

	// write out properties
	obs.WriteInteger(psz_Layout, m_nLayout);
	// obs.WriteBool(psz_Predefined, bPredefined);
	for(int i = 0; i < MAX_GRAPHS_NUMBER; i++)
	{
		UINT nID = operator[](i);

		if(nID)
			obs.WriteUInt(IsLayoutGraph(nID) ? psz_LayoutGraph : psz_OtherGraph, nID, CSaView::GetGraphTitle(nID));
	}

	obs.WriteEndMarker(psz_Configuration);
}

// Read spectrumParm properties from *.psa file.
BOOL CGraphConfiguration::ReadProperties(Object_istream& obs, BOOL &bPredefined)
{
	if ( !obs.bAtBackslash() || !obs.bReadBeginMarker(psz_Configuration) )
	{
		return FALSE;
	}

	UINT nID = 0;
	int nIndex = 0;

	bPredefined = false;

	while ( !obs.bAtEnd() )
	{
		if ( obs.bReadInteger(psz_Layout, m_nLayout) );
		else if ( obs.bReadBool(psz_Predefined, bPredefined)); // Read a boolean. The presence of the marker means TRUE
		else if ( obs.bReadUInt(psz_LayoutGraph, nID) || obs.bReadUInt(psz_LayoutGraph, nID))
			InsertAt((UINT)nID, nIndex++);
		else if ( obs.bEnd(psz_Configuration) )
			break;
	}
	return TRUE;
}

BOOL CGraphConfigurationVector::Load(LPCTSTR szFilename, int nTaskType)
{
	reserve(16); // reserve room for 16 sets to minimize copying

	m_nPredefinedSets = 0; // Consider all graph layouts user preferences
	int nSkip = size();

	if(szFilename  != NULL)
	{ 
		try
		{
			CSaString filename(szFilename);
			Object_istream obs(filename);
			while ( !obs.bAtEnd() )
			{
				CGraphConfiguration newSet;
				BOOL bPredefined;
				if(newSet.ReadProperties(obs, bPredefined))
				{
					if(!bPredefined && nSkip == 0)
						push_back(newSet);
					else if(bPredefined) // early files did not mark predefined configurations so we had to count 
						nSkip = 0; // This is not an early file.
					else
						nSkip--;
				}
				else
					obs.ReadMarkedString();  // Skip unexpected field
			}
		}
		catch(...)
		{
		}
	}

	BOOL bSuccess = (size() > 0);  // Not an empty list.

	if(!bSuccess && nTaskType == 0) // Speech
	{
		for(int i=0; i < 6; i++)
		{
			push_back(CDlgGraphsTypes::GetStandardCheckedGraphs(i));
		}
	}

	if(!bSuccess && nTaskType == 1) // Music
	{
		for(int i=0; i < 6; i++)
		{
			push_back(CDlgGraphsTypes::GetStandardCheckedGraphs(i + 6));
		}
	}

	return TRUE;
}

BOOL CGraphConfigurationVector::Save(LPCTSTR szFilename) const
{
	if(szFilename == NULL)
		return FALSE;

	try
	{
		CSaString filename(szFilename);
		Object_ostream obs(filename);
		for(int i = 0; i < (int)size(); i++)
		{
			operator[](i).WriteProperties(obs, i < GetCountPredefinedSets());
		}
	}
	catch(...)
	{
		return FALSE;
	}

	return TRUE;
}

int CGraphConfigurationVector::find(const CGraphConfiguration &data) const
{
	iterator pFound;
	CGraphConfigurationVector &unconstThis = *const_cast<CGraphConfigurationVector *>(this);

	pFound = std::find(unconstThis.begin(), unconstThis.end(), data);

	if(pFound != end())
		return pFound - begin();
	else return -1;
}

CGraphConfigurationVector& GetGraphConfigurationVector(int nTaskType)
{
	CSaString szPath(AfxGetApp()->GetProfileString(_T(""), _T("DataLocation")));

	switch(nTaskType)
	{
	case 0: // speech
		{
			CString szSpeechPath = szPath + _T("\\g-speech.psa");

			static CGraphConfigurationVector theSpeechConfiguration(szSpeechPath, 0);

			return theSpeechConfiguration;
		}
	case 1:
		{
			CString szMusicPath = szPath + _T("\\g-music.psa");

			static CGraphConfigurationVector theMusicConfiguration(szMusicPath, 1);
			return theMusicConfiguration;
		}
	default:
		{
			static CGraphConfigurationVector theEmptyConfiguration;      
			return theEmptyConfiguration;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsTypes dialog


CDlgGraphsTypes::CDlgGraphsTypes(CWnd* pParent, const UINT *pGraphIDs ,int nLayout)
: CDialog(CDlgGraphsTypes::IDD, pParent), cCurrentConfig(pGraphIDs, nLayout)
{
	//{{AFX_DATA_INIT(CDlgGraphsTypes)
	//}}AFX_DATA_INIT
	bCustom = FALSE;
	m_nConfiguration = -1;

	for(int nTaskType = 0; nTaskType < 2; nTaskType++)
		GetGraphConfigurationVector(nTaskType) = ::GetGraphConfigurationVector(nTaskType);

	if(pGraphIDs)
	{
		for(int i = 0; i < 2; i++)
		{
			int nTaskType = (RecentTaskType() + i) % 2;  // Start with MRU first

			CGraphConfigurationVector &cGraphs = GetGraphConfigurationVector(nTaskType);

			m_nConfiguration = cGraphs.find(cCurrentConfig);

			if(m_nConfiguration != -1)
			{
				RecentTaskType() = m_nTaskType = nTaskType;
				break;
			}
		}
	}
}

void CDlgGraphsTypes::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgGraphsTypes)
	DDX_Control(pDX, IDC_LIST, m_cList);
	DDX_Control(pDX, IDC_TASKTYPE, m_cTaskType);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgGraphsTypes, CDialog)
	//{{AFX_MSG_MAP(CDlgGraphsTypes)
	ON_BN_CLICKED(IDC_CUSTOM, OnCustom)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TASKTYPE, OnTaskTypeChange)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeGraphTypes)
	ON_LBN_DBLCLK(IDC_LIST, OnOK)
	ON_COMMAND(IDHELP, OnHelpGraphsTypes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsTypes message handlers

void CDlgGraphsTypes::OnCustom()
{
	if (m_cCustom.DoModal() == IDOK)
	{
		bCustom = TRUE;
		OnOK();
	}
}

/***************************************************************************/
// CDlgGraphsTypes::GetCheckedGraphs Return selected graphs
// The IDs of the graphs, selected by the user in the dialog, and stored in
// the dialogs private graph ID array will be copied in a graph ID array,
// which address has been given as parameter.
/***************************************************************************/
void CDlgGraphsTypes::GetCheckedGraphs(UINT* pGraphIDs, int *nLayout)
{
	if(bCustom)
	{
		m_cCustom.GetCheckedGraphs(pGraphIDs, nLayout);
		return;
	}
	else
	{
		CGraphConfigurationVector &cGraphs = ::GetGraphConfigurationVector(m_nTaskType);

		CGraphConfiguration &cConfig = cGraphs[m_nConfiguration];

		cConfig.GetGraphs(pGraphIDs);
		*nLayout = cConfig.m_nLayout;

		return;
	}
}


CGraphConfiguration CDlgGraphsTypes::GetStandardCheckedGraphs(int nConfiguration)
{
	int nLayout;
	UINT pGraphIDs[MAX_GRAPHS_NUMBER];

	GetStandardCheckedGraphs(nConfiguration, pGraphIDs, &nLayout);

	return CGraphConfiguration(pGraphIDs, nLayout);
}

void CDlgGraphsTypes::GetStandardCheckedGraphs(int nConfiguration, UINT* pGraphIDs, int *nLayout, CString *pLabelOut, CString *pTipOut)
{
	*nLayout = -1;

	LPCSTR pLabel = "No label";

	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		*(pGraphIDs + nLoop) = 0; // return own graph ID array
	}

	int nIndex = 0;

	switch(nConfiguration)
	{
	case 0: // Waveform
		pLabel = "Waveform";
		pGraphIDs[nIndex++] = IDD_RAWDATA;
		break;
	case 1: // Waveform, Auto Pitch
		pLabel = "Waveform, Auto Pitch";
		pGraphIDs[nIndex++] = IDD_RAWDATA;
		pGraphIDs[nIndex++] = IDD_GRAPITCH;
		break;
	case 2: // Waveform, Raw Pitch, Auto Pitch
		pLabel = "Wave, Raw, Intens, Auto\nWaveform, Raw Pitch, Intensity, Auto Pitch";
		pGraphIDs[nIndex++] = IDD_RAWDATA;
		pGraphIDs[nIndex++] = IDD_PITCH;
		pGraphIDs[nIndex++] = IDD_LOUDNESS;
		pGraphIDs[nIndex++] = IDD_GRAPITCH;
		break;
	case 3: // Waveform, Spect. A
		pLabel = "Waveform, Spectrogram";
		pGraphIDs[nIndex++] = IDD_RAWDATA;
		pGraphIDs[nIndex++] = IDD_SPECTROGRAM;
		break;
	case 4: // Waveform, Spect. A. Spectrum, 
		pLabel = "Waveform, Spgm, Sptrm\nWaveform, Spectrogram, Spectrum";
		pGraphIDs[nIndex++] = IDD_RAWDATA;
		pGraphIDs[nIndex++] = IDD_SPECTROGRAM;
		pGraphIDs[nIndex++] = IDD_SPECTRUM;
		break;
	case 5: // Waveform, Spectrum, Spect. A, F1-F2
		pLabel = "Wave,Spgm, Sptrm,F2-F1\nWaveform, Spectrogram, Spectrum, F2-F1";
		pGraphIDs[nIndex++] = IDD_RAWDATA;
		pGraphIDs[nIndex++] = IDD_SPECTROGRAM;
		pGraphIDs[nIndex++] = IDD_SPECTRUM;
		pGraphIDs[nIndex++] = IDD_F2F1;
		*nLayout = ID_LAYOUT_4C;
		break;
	case 6: // A: Position View, Melogram, TWC, Magnitude, Staff
		pLabel = "Pos,Mel,TW Mag, Staff\nPosition, Melogram, TWC, Magnitude, Staff";
		pGraphIDs[nIndex++] = IDD_RECORDING;
		pGraphIDs[nIndex++] = IDD_MELOGRAM;
		pGraphIDs[nIndex++] = IDD_TWC;
		pGraphIDs[nIndex++] = IDD_MAGNITUDE;
		pGraphIDs[nIndex++] = IDD_STAFF;
		break;
	case 7: // B: Waveform, Melogram, TWC, Magnitude, Staff
		pLabel = "Wav,Mel,TW Mag, Staff\nWaveform, Melogram, TWC, Magnitude, Staff";
		pGraphIDs[nIndex++] = IDD_RAWDATA;
		pGraphIDs[nIndex++] = IDD_MELOGRAM;
		pGraphIDs[nIndex++] = IDD_TWC;
		pGraphIDs[nIndex++] = IDD_MAGNITUDE;
		pGraphIDs[nIndex++] = IDD_STAFF;
		break;
	case 8: // C. Position View, Melogram, Staff
		pLabel = "Pos,Mel, Staff\nPosition, Melogram, Staff";
		pGraphIDs[nIndex++] = IDD_RECORDING;
		pGraphIDs[nIndex++] = IDD_MELOGRAM;
		pGraphIDs[nIndex++] = IDD_STAFF;
		break;
	case 9: // D: Position View, Melogram
		pLabel = "Position, Melogram";
		pGraphIDs[nIndex++] = IDD_RECORDING;
		pGraphIDs[nIndex++] = IDD_MELOGRAM;
		break;
	case 10: // E. Melogram, TWC
		pLabel = "Melogram, TWC";
		pGraphIDs[nIndex++] = IDD_MELOGRAM;
		pGraphIDs[nIndex++] = IDD_TWC;
		break;
	case 11: // F: Waveform
		pLabel = "Waveform";
		pGraphIDs[nIndex++] = IDD_RAWDATA;
		break;
	default:
		pGraphIDs[0] = IDD_RAWDATA;
	}

	if(*nLayout == -1)
	{
		// nLayout not set use default value
		*nLayout = CSaView::SetLayout(pGraphIDs);
	}

	CString szLabel = pLabel;
	CString szTip;

	int nCount = szLabel.Find('\n');

	if(nCount != -1)
	{
		szTip = szLabel.Mid(nCount + 1);
		szLabel = szLabel.Left(nCount);
	}
	else
	{
		szTip = szLabel;
	}

	if(pLabelOut)
		*pLabelOut = szLabel;
	if(pTipOut)
		*pTipOut = szTip;
}


void CDlgGraphsTypes::OnOK()
{
	UpdateData(TRUE);

	for(int nTaskType = 0; nTaskType < 2; nTaskType++)
	{
		CGraphConfigurationVector &cGlobal = ::GetGraphConfigurationVector(nTaskType);
		if(!cGlobal.IsEqual(GetGraphConfigurationVector(nTaskType)))
		{
			cGlobal = GetGraphConfigurationVector(nTaskType);
			cGlobal.Save();
		}
	}

	if(bCustom || m_nConfiguration != -1)
		CDialog::OnOK();
	else
		CDialog::OnCancel();
}


void CDlgGraphsTypes::OnAdd() 
{
	CGraphConfigurationVector& cGraphs = GetGraphConfigurationVector(m_nTaskType);
	cGraphs.push_back(cCurrentConfig);
	OnTaskTypeChange();
}

void CDlgGraphsTypes::OnRemove() 
{
	CGraphConfigurationVector& cGraphs = GetGraphConfigurationVector(m_nTaskType);
	cGraphs.Remove(m_nConfiguration);
	OnTaskTypeChange();
}

void CDlgGraphsTypes::OnSelchangeGraphTypes() 
{
	UpdateData();


	m_nConfiguration = m_cList.GetCurSel();
	CWnd *pWnd = GetDlgItem(IDC_REMOVE);
	const CGraphConfigurationVector& cGraphs = GetGraphConfigurationVector(m_nTaskType);
	if(pWnd) pWnd->EnableWindow(m_cList.GetCurSel()>= cGraphs.GetCountPredefinedSets());
}

BOOL CDlgGraphsTypes::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString szTab;

	szTab.LoadString(IDS_GRAPHTYPES_TAB_SPEECH);
	m_cTaskType.InsertItem(0, szTab);

	szTab.LoadString(IDS_GRAPHTYPES_TAB_MUSIC);
	m_cTaskType.InsertItem(1, szTab);

	m_cTaskType.SetCurSel(RecentTaskType());

	OnTaskTypeChange();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDlgGraphsTypes::OnTaskTypeChange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UNUSED_ALWAYS(pNMHDR);
	UNUSED_ALWAYS(pResult);

	UpdateData();

	RecentTaskType() = m_nTaskType = m_cTaskType.GetCurSel();

	const CGraphConfigurationVector& cGraphs = GetGraphConfigurationVector(m_nTaskType);
	PopulateList(m_cList, cGraphs, m_nTaskType == 1);

	m_cList.SetCurSel(cGraphs.find(cCurrentConfig));

	OnSelchangeGraphTypes();

	CWnd *pWnd = GetDlgItem(IDC_ADD);
	if(pWnd) pWnd->EnableWindow(m_cList.GetCurSel()==-1);

	pWnd = GetDlgItem(IDC_REMOVE);
	if(pWnd) pWnd->EnableWindow(m_cList.GetCurSel()>= cGraphs.GetCountPredefinedSets());
}

/***************************************************************************/
// CDlgGraphsTypes::OnHelpGraphsTypes Call Graphs Types help
/***************************************************************************/
void CDlgGraphsTypes::OnHelpGraphsTypes()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/Graphs/Graph_Types/Graph_Types.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

void CDlgGraphsTypes::PopulateList(CListBox &cList, const CGraphConfigurationVector &cVector, BOOL bAlphaHotKey)
{
	cList.SetCurSel(-1);

	cList.ResetContent();

	for(int i = 0; i < (int) cVector.size(); i++)
	{
		CString szDescription;
		CString szHotKey;

		if(i < 10)
			szHotKey.Format(_T("%hc: "), i + (bAlphaHotKey ? 'A' : '1'));

		const CGraphConfiguration &cConfig = cVector[i];

		szDescription = szHotKey + cConfig.GetDescription();

		cList.AddString(szDescription);
	}
}

int& CDlgGraphsTypes::RecentTaskType()
{
	static int nTaskType = 0; // Speech by default

	return nTaskType;
}

BOOL CDlgGraphsTypes::PreTranslateMessage( MSG* pMsg )
{
	return CDialog::PreTranslateMessage( pMsg );
}

CGraphConfigurationVector& CDlgGraphsTypes::GetGraphConfigurationVector(int nTaskType)
{
	switch(nTaskType)
	{
	case 0: // speech
		return m_cSpeech;
	case 1:
		return m_cMusic;
	default:
		{
			static CGraphConfigurationVector theEmptyConfiguration;      
			return theEmptyConfiguration;
		}
	}
}

