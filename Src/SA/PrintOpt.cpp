/////////////////////////////////////////////////////////////////////////////
// printopt.cpp:
// Implementation of the CDlgPrintOptions class.
// This class manages the users page printing options using a dialog.
// Author: Doug Eberman
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "printopt.h"
#include "objectostream.h"
#include "objectistream.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/***************************************************************************/
// CDlgPrintOptions::CDlgPrintOptions constructor
/***************************************************************************/
CDlgPrintOptions::CDlgPrintOptions(CWnd * pParent /*=NULL*/)
    : CDialog(CDlgPrintOptions::IDD, pParent) {
    m_bFixedAspectR = TRUE;
    m_bUseSmallFonts = TRUE;
    m_IsLandscape = TRUE;
    m_NumColumnsLessOne = 0;
    m_strNumRows = "1";
    m_IsHiRes = 0;
    m_LayoutType = 0;
    m_IsExpanded = FALSE;
    m_strNumCols = "1";

    m_NumRows = 1;
    m_NumCols = 1;
    m_bUserClickedPrint = FALSE;
    m_bUserClickedPreview = FALSE;
    m_bIsRecGraphSelected = TRUE;  // 09/30/2000 - DDO
    for (int i=0; i<MAX_GRAPHS_NUMBER; i++) {
        m_bIsGraphSelected[i] = TRUE;
    }
    m_NumGraphsSelected = 0;
}



void CDlgPrintOptions::CopyFrom(const CDlgPrintOptions & from) {
    m_bFixedAspectR = from.m_bFixedAspectR;
    m_bUseSmallFonts = from.m_bUseSmallFonts;
    m_IsLandscape = from.m_IsLandscape;
    m_NumColumnsLessOne = from.m_NumColumnsLessOne;
    m_strNumRows = from.m_strNumRows;
    m_IsHiRes = from.m_IsHiRes;
    m_LayoutType = from.m_LayoutType;
    m_IsExpanded = from.m_IsExpanded;
    m_strNumCols = from.m_strNumCols;
    m_NumRows = from.m_NumRows;
    m_NumCols = from.m_NumCols;
    m_bUserClickedPrint = from.m_bUserClickedPrint;
    m_bUserClickedPreview = from.m_bUserClickedPreview;

    m_bIsRecGraphSelected = from.m_bIsRecGraphSelected;  // 09/30/2000 - DDO
    for (int i=0; i<MAX_GRAPHS_NUMBER; i++) {
        m_bIsGraphSelected[i] = from.m_bIsGraphSelected[i];
    }

    m_NumGraphsSelected = from.m_NumGraphsSelected;
}



/***************************************************************************/
// CDlgPrintOptions::CDlgPrintOptions copy constructor
/***************************************************************************/
CDlgPrintOptions::CDlgPrintOptions(const CDlgPrintOptions & from)
    : CDialog(CDlgPrintOptions::IDD, from.GetParent()) {
    CopyFrom(from);
}



/***************************************************************************/
// CDlgPrintOptions::CDlgPrintOptions assignment operator
/***************************************************************************/
CDlgPrintOptions & CDlgPrintOptions::operator=(const CDlgPrintOptions & from) {
    // RLJ 09/14/2000 Fails Debug assertion in VC++ 6.0 (presumably not in VC++ 1.52C)
    //  SetParent(from.GetParent());
    if (from.GetParentOwner()) {
        SetParent(from.GetParentOwner());
    }

    CopyFrom(from);
    return *this;
}





/***************************************************************************/
// CDlgPrintOptions::OnInitDialog Dialog initialisation
// The dialog is centered over the main frame window.
/***************************************************************************/
BOOL CDlgPrintOptions::OnInitDialog() {
    CDialog::OnInitDialog();

	m_bmpPort.SubclassDlgItem(IDC_PORTBMP, this);
    m_bmpLand.SubclassDlgItem(IDC_LANDBMP, this);
    m_bmpPort.LoadBitmaps(IDB_PORTU);
    m_bmpLand.LoadBitmaps(IDB_LANDU);

    // build and place the row spin control
    //m_SpinRow.Init(IDC_SPIN_ROW, this);

    // build and place the column spin control
    //m_SpinCol.Init(IDC_SPIN_COL, this);

    m_bUserClickedPrint = FALSE;
    m_bUserClickedPreview = FALSE;
    CenterWindow(); // center dialog on recorder window
    GetWindowRect(m_WndRect);

    if (m_IsExpanded) {
        GetDlgItem(IDC_MORE)->EnableWindow(TRUE);
        Expand();
    } else {
        Collapse();
        GetDlgItem(IDC_MORE)->EnableWindow(m_IsHiRes);
    }
    SetupGraphsList();
    //EnableCustomLayout(m_LayoutType == LAYOUT_CUSTOM());
    ShowLandscapeBitmap(m_IsLandscape);

    return TRUE;
}

void CDlgPrintOptions::ShowLandscapeBitmap(BOOL show) {
    GetDlgItem(IDC_PORTBMP)->ShowWindow(!show);
    GetDlgItem(IDC_LANDBMP)->ShowWindow(show);
}

/***************************************************************************/
// CDlgPrintOptions::SetGraphsPtr
/***************************************************************************/
void CDlgPrintOptions::SetGraphsPtr(CGraphWnd * pGraphs[]) {
    for (int i=0; i < MAX_GRAPHS_NUMBER; i++) {
        m_apGraphs[i] = pGraphs[i];
    }
}

/***************************************************************************/
// CDlgPrintOptions::SetupGraphsList
/***************************************************************************/
void CDlgPrintOptions::SetupGraphsList() {
    m_numGraphs = 0;

    for (int i=0; i < MAX_GRAPHS_NUMBER; i++) {
        if (m_apGraphs[i]) {
            m_numGraphs++;

            // get the title
            CString szText;
            m_apGraphs[i]->GetWindowText(szText.GetBuffer(32), 32);
            szText.ReleaseBuffer();

            int index = m_listBoxGraphsToPrint.AddString(szText);
            m_listBoxGraphsToPrint.SetItemData(index, i);
            if (m_bIsGraphSelected[i]) {
                m_listBoxGraphsToPrint.SetSel(index, TRUE);
            }
        }
    }
}



/***************************************************************************/
// CDlgPrintOptions::DoDataExchange
/***************************************************************************/
void CDlgPrintOptions::DoDataExchange(CDataExchange * pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_SCREEN_SHOT, m_IsHiRes);
    DDX_Check(pDX, IDC_FIXED_PRINT_ASPECTR, m_bFixedAspectR);
    DDX_Check(pDX, IDC_SMALL_FONTS, m_bUseSmallFonts);
    DDX_Radio(pDX, IDC_PORTRAIT, m_IsLandscape);
    DDX_Text(pDX, IDC_PRINT_ROWS, m_strNumRows);
    DDX_Radio(pDX, IDC_USESCREENLAYOUT, m_LayoutType);
    DDX_Text(pDX, IDC_PRINT_COLS, m_strNumCols);
    DDX_Control(pDX, IDC_SELECT_GRAPHS2PRINT,  m_listBoxGraphsToPrint);
}

/***************************************************************************/
// CDlgPrintOptions::OnOK
/***************************************************************************/
void CDlgPrintOptions::OnOK() {
    UpdateData(TRUE);

    m_NumRows = _ttoi(m_strNumRows);
    m_NumCols = _ttoi(m_strNumCols);
    m_NumGraphsSelected = m_listBoxGraphsToPrint.GetSelCount();
    m_listBoxGraphsToPrint.GetSelItems(m_NumGraphsSelected, m_selections);

    int i;
    for (i = 0; i < MAX_GRAPHS_NUMBER; i++) {
        m_bIsGraphSelected[i] = FALSE;
    }

    //*******************************************************
    // 09/30/2000 - DDO
    //*******************************************************
    i = 0;
    m_bIsRecGraphSelected = FALSE;
    if (m_listBoxGraphsToPrint.GetItemData(m_selections[0]) == -1) {
        m_bIsRecGraphSelected = TRUE;
        i = 1;
    }
    //*******************************************************

    for (; i < m_NumGraphsSelected; i++) {
        long graphIndex = m_listBoxGraphsToPrint.GetItemData(m_selections[i]);
        m_bIsGraphSelected[graphIndex] = TRUE;
    }
    BOOL ok = TRUE;

    if (m_IsHiRes && m_LayoutType == LAYOUT_CUSTOM()) {
        int remainder = m_NumGraphsSelected % (m_NumRows * m_NumCols);

        if (remainder) {
            AfxMessageBox(IDS_ERROR_BAD_PRINTLAYOUT, MB_OK | MB_ICONEXCLAMATION, 0);
            ok = FALSE;
        }
    }
    if (ok) {
        CDialog::OnOK();
    }
}



/***************************************************************************/
// CDlgPrintOptions::EnableCustomLayout
/***************************************************************************/
void CDlgPrintOptions::EnableCustomLayout(BOOL enable) {
    GetDlgItem(IDC_PRINT_ROWS)->EnableWindow(enable);
    GetDlgItem(IDC_PRINT_COLS)->EnableWindow(enable);
    m_SpinRow.EnableWindow(enable);
    m_SpinCol.EnableWindow(enable);
}



/***************************************************************************/
// CDlgPrintOptions::Message Map
/***************************************************************************/
BEGIN_MESSAGE_MAP(CDlgPrintOptions, CDialog)
    ON_BN_CLICKED(IDC_MORE, OnMore)
    ON_BN_CLICKED(IDC_USESCREENLAYOUT  , OnUseScreenLayout)
    ON_BN_CLICKED(IDC_ONEPERPAGE , OnPrintOnePerPage)
    ON_BN_CLICKED(IDC_PRINT_MANYPERPAGE, OnPrintManyPerPage)
    ON_BN_CLICKED(IDC_SELECTALLGRAPHS, OnSelectallgraphs)
    ON_BN_CLICKED(ID_PRINT, OnPrint)
    ON_BN_CLICKED(IDC_CLEARALLGRAPHS, OnClearallgraphs)
    ON_BN_CLICKED(IDC_HIRES, OnHires)
    ON_BN_CLICKED(ID_PREVIEW, OnPreview)
    ON_BN_CLICKED(IDC_SCREEN_SHOT, OnScreenShot)
    ON_COMMAND(IDC_SPIN_ROW, OnSpinRow)
    ON_COMMAND(IDC_SPIN_COL, OnSpinCol)
    ON_BN_CLICKED(IDC_PORTRAIT, OnPortrait)
    ON_BN_CLICKED(IDC_LANDSCAPE, OnLandscape)
    ON_COMMAND(IDHELP, OnHelpPrintOptions)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CDlgPrintOptions message handlers


/***************************************************************************/
// CDlgPrintOptions::OnMore - user clicked more/less button so
//  expand/collapse window
/***************************************************************************/
void CDlgPrintOptions::OnMore() {
    if (m_IsExpanded) {
        Collapse();
    } else {
        Expand();
    }
}



/***************************************************************************/
// CDlgPrintOptions::Expand() - helper for OnMore()
/***************************************************************************/
void CDlgPrintOptions::Expand() {
    CWnd * pWnd = GetDlgItem(IDC_MORE);
    ASSERT(pWnd);
    pWnd->SetWindowText(_T("LESS &<<"));

    CRect rct;
    GetWindowRect(rct);
    rct.bottom = rct.top + m_WndRect.Height();
    MoveWindow(rct);

    m_IsExpanded = TRUE;
}



/***************************************************************************/
// CDlgPrintOptions::Collapse - helper for OnMore()
/***************************************************************************/
void CDlgPrintOptions::Collapse() {
    CWnd * pWnd = GetDlgItem(IDC_MORE);
    ASSERT(pWnd);
    pWnd->SetWindowText(_T("MORE &>>"));

    //pWnd = GetDlgItem(IDC_PRINT_TYPE_GROUP_BOX);
    pWnd = GetDlgItem(ID_PRINT);
    ASSERT(pWnd);
    CRect bottomGroup;
    pWnd->GetWindowRect(bottomGroup);

    CRect rct(m_WndRect);
    rct.bottom = bottomGroup.bottom + 12;

    MoveWindow(rct);

    m_IsExpanded = FALSE;
}



/***************************************************************************/
// CDlgPrintOptions::OnSelectallgraphs - user clicked the ALL GRAPHS button,
// so set them all to be selected.
/***************************************************************************/
void CDlgPrintOptions::OnSelectallgraphs() {
    for (int i=0; i< m_listBoxGraphsToPrint.GetCount(); i++) {
        m_listBoxGraphsToPrint.SetSel(i,TRUE);
    }
}



/***************************************************************************/
// CDlgPrintOptions::OnPrint - user clicked print.  Set flag and exit -
// caller will check flag and do the actual print.
/***************************************************************************/
void CDlgPrintOptions::OnPrint() {
    m_bUserClickedPrint = TRUE;
    OnOK();
}




/***************************************************************************/
// CDlgPrintOptions::OnClearallgraphs - user selected clear all graphs from
// the list of possible graphs to print - clear all selections.
/***************************************************************************/
void CDlgPrintOptions::OnClearallgraphs() {
    // TODO: Add your control notification handler code here
    for (int i=0; i< m_listBoxGraphsToPrint.GetCount(); i++) {
        m_listBoxGraphsToPrint.SetSel(i,FALSE);
    }
}



/***************************************************************************/
// CDlgPrintOptions::OnHiRes - user selected the hi-res type of printing,
// enable the ability to expand dialog and reveal more hi-res printing options.
/***************************************************************************/
void CDlgPrintOptions::OnHires() {
    GetDlgItem(IDC_MORE)->EnableWindow(TRUE);
}



/***************************************************************************/
// CDlgPrintOptions::OnScreenShot - user selected the screen shot type of
// printing.  Disable the ability to expand the dialog as it would only
// reveal hi-res options which don't apply to screen shot.
/***************************************************************************/
void CDlgPrintOptions::OnScreenShot() {
    if (m_IsExpanded) {
        Collapse();
    }
    GetDlgItem(IDC_MORE)->EnableWindow(FALSE);
}



/***************************************************************************/
// CDlgPrintOptions::OnPreview - user clicked preview.  Set flag and exit -
// caller will check flag and do a preview.
/***************************************************************************/
void CDlgPrintOptions::OnPreview() {
    m_bUserClickedPreview = TRUE;
    OnOK();
}



/***************************************************************************/
// CDlgPrintOptions::OnSpinRow  - number of rows to print per page spiner hit.
/***************************************************************************/
void CDlgPrintOptions::OnSpinRow() {
    UpdateData(TRUE);
    m_NumRows = _ttoi(m_strNumRows);

    if (m_SpinRow.UpperButtonClicked()) {
        m_NumRows++;
    } else {
        m_NumRows--;
    }

    if (m_NumRows > m_numGraphs) {
        m_NumRows = m_numGraphs;
    }
    if (m_NumRows < 1) {
        m_NumRows = 1;
    }

    swprintf_s(m_strNumRows.GetBuffer(100),100,_T("%d"),m_NumRows);
    m_strNumRows.ReleaseBuffer();
    UpdateData(FALSE);
}



/***************************************************************************/
// CDlgPrintOptions::OnSpinCol  - number of cols to print per page spiner hit.
/***************************************************************************/
void CDlgPrintOptions::OnSpinCol() {
    UpdateData(TRUE);
    m_NumCols = _ttoi(m_strNumCols);

    if (m_SpinCol.UpperButtonClicked()) {
        m_NumCols++;
    } else {
        m_NumCols--;
    }

    if (m_NumCols > m_numGraphs) {
        m_NumCols = m_numGraphs;
    }
    if (m_NumCols < 1) {
        m_NumCols = 1;
    }

    swprintf_s(m_strNumCols.GetBuffer(100),100,_T("%d"),m_NumCols);
    m_strNumCols.ReleaseBuffer();
    UpdateData(FALSE);
}


/***************************************************************************/
// CDlgPrintOptions::OnPortrait - portrait radio button pushed.  Show
// correct bitmap.
/***************************************************************************/
void CDlgPrintOptions::OnPortrait() {
    UpdateData(TRUE);
    ShowLandscapeBitmap(m_IsLandscape);
}



/***************************************************************************/
// CDlgPrintOptions::OnLandscape - landscape radio button pushed.  Show
// correct bitmap.
/***************************************************************************/
void CDlgPrintOptions::OnLandscape() {
    UpdateData(TRUE);
    ShowLandscapeBitmap(m_IsLandscape);
}

static LPCSTR psz_printoptions = "printoptions";
static LPCSTR psz_fixedaspectr = "fixedaspectr";
static LPCSTR psz_usesmallfonts= "usesmallfonts";
static LPCSTR psz_islandscape  = "islandscape";
static LPCSTR psz_ishires      = "ishires";

void CDlgPrintOptions::WriteProperties(CObjectOStream & obs) {

	obs.WriteNewline();
    obs.WriteBeginMarker(psz_printoptions);
    obs.WriteBool(psz_fixedaspectr, m_bFixedAspectR);
    obs.WriteBool(psz_usesmallfonts, m_bUseSmallFonts);
    obs.WriteBool(psz_islandscape, m_IsLandscape);
    obs.WriteBool(psz_ishires, m_IsHiRes);
    obs.WriteEndMarker(psz_printoptions);
}


BOOL CDlgPrintOptions::ReadProperties(CObjectIStream & obs) {

	if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_printoptions)) {
        return FALSE;
    }

    while (!obs.bAtEnd()) {
        if (obs.bReadBool(psz_fixedaspectr, m_bFixedAspectR))  ;
        else if (obs.bReadBool(psz_usesmallfonts, m_bUseSmallFonts)) ;
        else if (obs.bReadBool(psz_islandscape, m_IsLandscape)) ;
        else if (obs.bReadBool(psz_ishires, m_IsHiRes)) ;
        else if (obs.bEnd(psz_printoptions)) {
            break;
        }
    }

    return TRUE;
}

/***************************************************************************/
// CDlgPrintOptions::OnHelpPrintOptions Call Page Setup dialog help
/***************************************************************************/
void CDlgPrintOptions::OnHelpPrintOptions() {
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/File/Page_Setup.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}
