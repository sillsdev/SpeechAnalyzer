/////////////////////////////////////////////////////////////////////////////
// ch_dlg.cpp:
// Implementation of the CDlgChartVowelsPage (property page)
//                       CDlgChartConsonantsPage (property page)
//                       CDlgChartDiacriticsPage (property page)
//                       CDlgChartSuprasegsPage (property page)
//                       CDlgCharChart (property sheet)          classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
//
// Revision History
//   1.06.6U2
//        SDM removed unused combobox from CDlgChart moved buttons and text box
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ch_dlg.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CDlgChartVowelsPage property page
// Displays information about the vowels in the character chart.

/////////////////////////////////////////////////////////////////////////////
// CDlgChartVowelsPage message map

BEGIN_MESSAGE_MAP(CDlgChartVowelsPage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgChartVowelsPage)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgChartVowelsPage construction/destruction/creation

/***************************************************************************/
// CDlgChartVowelsPage::CDlgChartVowelsPage Constructor
/***************************************************************************/
CDlgChartVowelsPage::CDlgChartVowelsPage() : CPropertyPage(CDlgChartVowelsPage::IDD)
{
    //{{AFX_DATA_INIT(CDlgChartVowelsPage)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
// CDlgChartVowelsPage helper functions

/***************************************************************************/
// CDlgChartVowelsPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgChartVowelsPage::DoDataExchange(CDataExchange * pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgChartVowelsPage)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDlgChartVowelsPage message handlers

/***************************************************************************/
// CDlgChartVowelsPage::OnInitDialog Dialog initialisation
// All the necessary informations are put into the dialog controls.
/***************************************************************************/
BOOL CDlgChartVowelsPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    // get the font for the text controls
    CFont * pFont = ((CDlgCharChart *)GetParent())->GetTextFont();
    // build and place the chart text controls
    m_ChartText[0].Init(IDC_CHART_T0, NO_LINES | TEXT_LEFT, pFont, this);
    m_ChartText[1].Init(IDC_CHART_T1, NO_LINES | TEXT_LEFT, pFont, this);
    m_ChartText[2].Init(IDC_CHART_T2, NO_LINES | TEXT_LEFT, pFont, this);
    m_ChartText[3].Init(IDC_CHART_T3, NO_LINES | TEXT_LEFT, pFont, this);
    m_ChartText[4].Init(IDC_CHART_T4, NO_LINES | TEXT_RIGHT, pFont, this);
    m_ChartText[5].Init(IDC_CHART_T100, NO_LINES | TEXT_LEFT, pFont, this);
    m_ChartText[6].Init(IDC_CHART_T200, NO_LINES | TEXT_LEFT, pFont, this);
    m_ChartText[7].Init(IDC_CHART_T300, NO_LINES | TEXT_LEFT, pFont, this);
    m_ChartLine[0].Init(IDC_CHART_L0, DIAG_LR | END_DOTS, this);
    m_ChartLine[1].Init(IDC_CHART_L1, DIAG_LR | END_DOTS, this);
    m_ChartLine[2].Init(IDC_CHART_L2, DIAG_LR | END_DOTS, this);
    m_ChartLine[3].Init(IDC_CHART_L100, DIAG_LR | END_DOTS, this);
    m_ChartLine[4].Init(IDC_CHART_L101, DIAG_LR, this);
    m_ChartLine[5].Init(IDC_CHART_L102, DIAG_LR, this);
    m_ChartLine[6].Init(IDC_CHART_L103, DIAG_LR | START_DOT, this);
    m_ChartLine[7].Init(IDC_CHART_L104, DIAG_LR, this);
    m_ChartLine[8].Init(IDC_CHART_L200, LINE_LEFT | END_DOTS, this);
    m_ChartLine[9].Init(IDC_CHART_L201, LINE_LEFT | END_DOTS, this);
    m_ChartLine[10].Init(IDC_CHART_L202, LINE_LEFT | END_DOTS, this);
    m_ChartLine[11].Init(IDC_CHART_L150, LINE_TOP, this);
    m_ChartLine[12].Init(IDC_CHART_L151, LINE_TOP, this);
    m_ChartLine[13].Init(IDC_CHART_L152, LINE_TOP, this);
    m_ChartLine[14].Init(IDC_CHART_L153, LINE_TOP, this);
    m_ChartLine[15].Init(IDC_CHART_L350, LINE_TOP, this);
    m_ChartLine[16].Init(IDC_CHART_L351, LINE_TOP, this);
    m_ChartLine[17].Init(IDC_CHART_L352, LINE_TOP, this);
    ((CDlgCharChart *)GetParent())->GetTable(IPA)->InitPage(VOWEL);
    return TRUE;
}

//###########################################################################
// CDlgChartConsonantsPage property page
// Displays information about the consonants in the character chart.

/////////////////////////////////////////////////////////////////////////////
// CDlgChartConsonantsPage message map

BEGIN_MESSAGE_MAP(CDlgChartConsonantsPage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgChartConsonantsPage)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgChartConsonantsPage construction/destruction/creation

/***************************************************************************/
// CDlgChartConsonantsPage::CDlgChartConsonantsPage Constructor
/***************************************************************************/
CDlgChartConsonantsPage::CDlgChartConsonantsPage() : CPropertyPage(CDlgChartConsonantsPage::IDD)
{
    //{{AFX_DATA_INIT(CDlgChartConsonantsPage)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
// CDlgChartConsonantsPage helper functions

/***************************************************************************/
// CDlgChartConsonantsPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgChartConsonantsPage::DoDataExchange(CDataExchange * pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgChartConsonantsPage)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDlgChartConsonantsPage message handlers

/***************************************************************************/
// CDlgChartConsonantsPage::OnInitDialog Dialog initialisation
// All the necessary informations are put into the dialog controls.
/***************************************************************************/
BOOL CDlgChartConsonantsPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    // get the font for the text controls
    CFont * pFont = ((CDlgCharChart *)GetParent())->GetTextFont();
    // build and place the chart text controls
    m_ChartText[25].Init(IDC_CHART_T106, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[27].Init(IDC_CHART_T108, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[40].Init(IDC_CHART_T902, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[41].Init(IDC_CHART_T903, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[44].Init(IDC_CHART_T1103, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[45].Init(IDC_CHART_T1203, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[46].Init(IDC_CHART_T1204, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[47].Init(IDC_CHART_T1205, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[48].Init(IDC_CHART_T1104, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[49].Init(IDC_CHART_T1105, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[0].Init(IDC_CHART_T0, HORZBOTTOM_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD | VERTRIGHT_BOLD | TEXT_LEFT, pFont, this);
    m_ChartText[1].Init(IDC_CHART_T1, HORZBOTTOM_LINE | VERTRIGHT_LINE | VERTRIGHT_BOLD | TEXT_LEFT, pFont, this);
    m_ChartText[2].Init(IDC_CHART_T2, HORZBOTTOM_LINE | VERTRIGHT_LINE | VERTRIGHT_BOLD | TEXT_LEFT, pFont, this);
    m_ChartText[4].Init(IDC_CHART_T4, HORZBOTTOM_LINE | VERTRIGHT_LINE | VERTRIGHT_BOLD | TEXT_LEFT, pFont, this);
    m_ChartText[3].Init(IDC_CHART_T3, HORZBOTTOM_LINE | VERTRIGHT_LINE | VERTRIGHT_BOLD | TEXT_LEFT, pFont, this);
    m_ChartText[5].Init(IDC_CHART_T5, HORZBOTTOM_LINE | VERTRIGHT_LINE | VERTRIGHT_BOLD | TEXT_LEFT, pFont, this);
    m_ChartText[6].Init(IDC_CHART_T6, HORZBOTTOM_LINE | VERTRIGHT_LINE | VERTRIGHT_BOLD | TEXT_LEFT, pFont, this);
    m_ChartText[8].Init(IDC_CHART_T8, HORZBOTTOM_LINE | VERTRIGHT_LINE | VERTRIGHT_BOLD | TEXT_LEFT, pFont, this);
    m_ChartText[7].Init(IDC_CHART_T7, HORZBOTTOM_LINE | VERTRIGHT_LINE | VERTRIGHT_BOLD | TEXT_LEFT, pFont, this);
    m_ChartText[9].Init(IDC_CHART_T100, HORZBOTTOM_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[10].Init(IDC_CHART_T200, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[11].Init(IDC_CHART_T400, HORZBOTTOM_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[12].Init(IDC_CHART_T500, HORZBOTTOM_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[13].Init(IDC_CHART_T600, HORZBOTTOM_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[14].Init(IDC_CHART_T700, HORZBOTTOM_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[15].Init(IDC_CHART_T800, HORZBOTTOM_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[16].Init(IDC_CHART_T900, HORZBOTTOM_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[17].Init(IDC_CHART_T1000, HORZBOTTOM_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[18].Init(IDC_CHART_T1100, HORZBOTTOM_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[19].Init(IDC_CHART_T1200, HORZBOTTOM_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD, pFont, this);
    m_ChartText[20].Init(IDC_CHART_T101, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[21].Init(IDC_CHART_T102, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[22].Init(IDC_CHART_T103, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[23].Init(IDC_CHART_T104, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[24].Init(IDC_CHART_T105, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[26].Init(IDC_CHART_T107, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[28].Init(IDC_CHART_T201, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[29].Init(IDC_CHART_T401, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[30].Init(IDC_CHART_T501, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[31].Init(IDC_CHART_T601, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[32].Init(IDC_CHART_T701, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[33].Init(IDC_CHART_T801, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[34].Init(IDC_CHART_T901, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[35].Init(IDC_CHART_T1001, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[36].Init(IDC_CHART_T1101, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[37].Init(IDC_CHART_T1201, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[38].Init(IDC_CHART_T402, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[39].Init(IDC_CHART_T403, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[42].Init(IDC_CHART_T1102, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    m_ChartText[43].Init(IDC_CHART_T1202, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_DARK, pFont, this);
    // non-pulmonic
    m_ChartText[50].Init(IDC_CHART_T50, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[51].Init(IDC_CHART_T51, OUTLINE, pFont, this);
    m_ChartText[52].Init(IDC_CHART_T52, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[53].Init(IDC_CHART_T53, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[54].Init(IDC_CHART_T54, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[55].Init(IDC_CHART_T55, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[56].Init(IDC_CHART_T56, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[57].Init(IDC_CHART_T57, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[58].Init(IDC_CHART_T150, HORZBOTTOM_LINE | HORZTOP_LINE | VERTRIGHT_LINE, pFont, this);
    m_ChartText[59].Init(IDC_CHART_T151, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[60].Init(IDC_CHART_T152, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[61].Init(IDC_CHART_T153, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[62].Init(IDC_CHART_T154, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[63].Init(IDC_CHART_T155, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[64].Init(IDC_CHART_T156, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[65].Init(IDC_CHART_T250, HORZBOTTOM_LINE | HORZTOP_LINE | VERTRIGHT_LINE, pFont, this);
    m_ChartText[66].Init(IDC_CHART_T251, HORZBOTTOM_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[67].Init(IDC_CHART_T252, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[68].Init(IDC_CHART_T253, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[69].Init(IDC_CHART_T254, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[70].Init(IDC_CHART_T255, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[71].Init(IDC_CHART_T256, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    // other symbols
    m_ChartText[72].Init(IDC_CHART_T350, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[73].Init(IDC_CHART_T351, OUTLINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[74].Init(IDC_CHART_T352, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[75].Init(IDC_CHART_T353, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[76].Init(IDC_CHART_T354, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[77].Init(IDC_CHART_T355, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[78].Init(IDC_CHART_T356, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[79].Init(IDC_CHART_T357, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[80].Init(IDC_CHART_T450, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[81].Init(IDC_CHART_T451, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[82].Init(IDC_CHART_T452, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[83].Init(IDC_CHART_T453, BCK_TRANSPARENT, pFont, this);
    m_ChartText[84].Init(IDC_CHART_T454, BCK_TRANSPARENT, pFont, this);
    m_ChartText[85].Init(IDC_CHART_T455, BCK_TRANSPARENT, pFont, this);
    m_ChartText[86].Init(IDC_CHART_T456, BCK_TRANSPARENT, pFont, this);
    ((CDlgCharChart *)GetParent())->GetTable(IPA)->InitPage(CONSONANT);
    return TRUE;
}

//###########################################################################
// CDlgChartDiacriticsPage property page
// Displays information about the diacritics in the character chart.

/////////////////////////////////////////////////////////////////////////////
// CDlgChartDiacriticsPage message map

BEGIN_MESSAGE_MAP(CDlgChartDiacriticsPage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgChartDiacriticsPage)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgChartDiacriticsPage construction/destruction/creation

/***************************************************************************/
// CDlgChartDiacriticsPage::CDlgChartDiacriticsPage Constructor
/***************************************************************************/
CDlgChartDiacriticsPage::CDlgChartDiacriticsPage() : CPropertyPage(CDlgChartDiacriticsPage::IDD)
{
    //{{AFX_DATA_INIT(CDlgChartDiacriticsPage)
    //}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
// CDlgChartDiacriticsPage helper functions

/***************************************************************************/
// CDlgChartDiacriticsPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgChartDiacriticsPage::DoDataExchange(CDataExchange * pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgChartDiacriticsPage)
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDlgChartDiacriticsPage message handlers

/***************************************************************************/
// CDlgChartDiacriticsPage::OnInitDialog Dialog initialisation
/***************************************************************************/
BOOL CDlgChartDiacriticsPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    // get the font for the text controls
    CFont * pFont = ((CDlgCharChart *)GetParent())->GetTextFont();
    // build and place the chart text controls
    m_ChartText[0].Init(IDC_CHART_T0, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[1].Init(IDC_CHART_T1, OUTLINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[2].Init(IDC_CHART_T2, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[3].Init(IDC_CHART_T3, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[4].Init(IDC_CHART_T4, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[5].Init(IDC_CHART_T5, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[6].Init(IDC_CHART_T6, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[7].Init(IDC_CHART_T7, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[8].Init(IDC_CHART_T8, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[9].Init(IDC_CHART_T9, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[10].Init(IDC_CHART_T10, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[11].Init(IDC_CHART_T11, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[12].Init(IDC_CHART_T12, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[13].Init(IDC_CHART_T100, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[14].Init(IDC_CHART_T101, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[15].Init(IDC_CHART_T102, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[16].Init(IDC_CHART_T103, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[17].Init(IDC_CHART_T104, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[18].Init(IDC_CHART_T105, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[19].Init(IDC_CHART_T106, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[20].Init(IDC_CHART_T107, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[21].Init(IDC_CHART_T108, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[22].Init(IDC_CHART_T109, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[23].Init(IDC_CHART_T110, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[24].Init(IDC_CHART_T111, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[25].Init(IDC_CHART_T300, VERTLEFT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[26].Init(IDC_CHART_T200, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[27].Init(IDC_CHART_T201, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[28].Init(IDC_CHART_T202, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[29].Init(IDC_CHART_T203, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[30].Init(IDC_CHART_T204, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[31].Init(IDC_CHART_T205, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[32].Init(IDC_CHART_T206, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[33].Init(IDC_CHART_T207, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[34].Init(IDC_CHART_T208, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[35].Init(IDC_CHART_T209, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[36].Init(IDC_CHART_T210, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[37].Init(IDC_CHART_T211, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[38].Init(IDC_CHART_T250, TEXT_RIGHT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[39].Init(IDC_CHART_T251, TEXT_RIGHT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[40].Init(IDC_CHART_T260, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[41].Init(IDC_CHART_T261, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[42].Init(IDC_CHART_T500, VERTLEFT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[43].Init(IDC_CHART_T400, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[44].Init(IDC_CHART_T401, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[45].Init(IDC_CHART_T402, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[46].Init(IDC_CHART_T403, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[47].Init(IDC_CHART_T404, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[48].Init(IDC_CHART_T405, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[49].Init(IDC_CHART_T406, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    ((CDlgCharChart *)GetParent())->GetTable(IPA)->InitPage(DIACRITIC);
    return TRUE;
}

//###########################################################################
// CDlgChartSuprasegsPage property page
// Displays information about the suprasegmentals in the character chart.

/////////////////////////////////////////////////////////////////////////////
// CDlgChartSuprasegsPage message map

BEGIN_MESSAGE_MAP(CDlgChartSuprasegsPage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgChartSuprasegsPage)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgChartSuprasegsPage construction/destruction/creation

/***************************************************************************/
// CDlgChartSuprasegsPage::CDlgChartSuprasegsPage Constructor
/***************************************************************************/
CDlgChartSuprasegsPage::CDlgChartSuprasegsPage() : CPropertyPage(CDlgChartSuprasegsPage::IDD)
{
    //{{AFX_DATA_INIT(CDlgChartSuprasegsPage)
    //}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
// CDlgChartSuprasegsPage helper functions

/***************************************************************************/
// CDlgChartSuprasegsPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgChartSuprasegsPage::DoDataExchange(CDataExchange * pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgChartSuprasegsPage)
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDlgChartSuprasegsPage message handlers

/***************************************************************************/
// CDlgChartSuprasegsPage::OnInitDialog Dialog initialisation
/***************************************************************************/
BOOL CDlgChartSuprasegsPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    // get the font for the text controls
    CFont * pFont = ((CDlgCharChart *)GetParent())->GetTextFont();
    // build and place the chart text controls
    m_ChartText[0].Init(IDC_CHART_T0, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[37].Init(IDC_CHART_T1, HORZTOP_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[1].Init(IDC_CHART_T2, HORZBOTTOM_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[2].Init(IDC_CHART_T3, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[3].Init(IDC_CHART_T4, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[4].Init(IDC_CHART_T5, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[5].Init(IDC_CHART_T6, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[6].Init(IDC_CHART_T7, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[7].Init(IDC_CHART_T8, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[8].Init(IDC_CHART_T9, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[9].Init(IDC_CHART_T10, HORZBOTTOM_LINE | VERTLEFT_LINE | VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[10].Init(IDC_CHART_T100, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[11].Init(IDC_CHART_T101, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[12].Init(IDC_CHART_T102, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[13].Init(IDC_CHART_T103, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[14].Init(IDC_CHART_T104, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[15].Init(IDC_CHART_T105, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[16].Init(IDC_CHART_T106, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[17].Init(IDC_CHART_T107, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[18].Init(IDC_CHART_T108, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[19].Init(IDC_CHART_T300, VERTLEFT_LINE | VERTRIGHT_LINE | VERTRIGHT_BOLD | BCK_TRANSPARENT, pFont, this);
    m_ChartText[20].Init(IDC_CHART_T200, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[21].Init(IDC_CHART_T201, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[22].Init(IDC_CHART_T202, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[23].Init(IDC_CHART_T203, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[24].Init(IDC_CHART_T204, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[25].Init(IDC_CHART_T205, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[26].Init(IDC_CHART_T206, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[27].Init(IDC_CHART_T500, VERTRIGHT_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[28].Init(IDC_CHART_T400, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[29].Init(IDC_CHART_T401, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[30].Init(IDC_CHART_T402, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[31].Init(IDC_CHART_T403, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[32].Init(IDC_CHART_T404, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[33].Init(IDC_CHART_T405, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[34].Init(IDC_CHART_T406, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[35].Init(IDC_CHART_T153, BCK_TRANSPARENT, pFont, this);
    m_ChartText[36].Init(IDC_CHART_T154, BCK_TRANSPARENT, pFont, this);
    m_ChartText[38].Init(IDC_CHART_T150, TEXT_LEFT | BCK_TRANSPARENT, pFont, this);
    m_ChartText[39].Init(IDC_CHART_T151, OUTLINE | HORZBOTTOM_BOLD | VERTRIGHT_BOLD | BCK_TRANSPARENT, pFont, this);
    m_ChartText[40].Init(IDC_CHART_T152, HORZBOTTOM_LINE | HORZTOP_LINE | VERTRIGHT_LINE | HORZBOTTOM_BOLD | BCK_TRANSPARENT, pFont, this);
    m_ChartText[41].Init(IDC_CHART_T1200, HORZBOTTOM_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[42].Init(IDC_CHART_T1202, HORZBOTTOM_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[43].Init(IDC_CHART_T1203, HORZBOTTOM_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[44].Init(IDC_CHART_T1204, HORZBOTTOM_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[45].Init(IDC_CHART_T1205, HORZBOTTOM_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[46].Init(IDC_CHART_T1206, HORZBOTTOM_LINE | BCK_TRANSPARENT, pFont, this);
    m_ChartText[47].Init(IDC_CHART_T1201, HORZBOTTOM_LINE | BCK_TRANSPARENT, pFont, this);
    ((CDlgCharChart *)GetParent())->GetTable(IPA)->InitPage(SUPRASEG);
    return TRUE;
}

//###########################################################################
// CDlgCharChart property sheet
// Displays all the important information about the wave file of the actually
// opened document.

IMPLEMENT_DYNAMIC(CDlgCharChart, CPropertySheet)

BOOL CDlgCharChart::m_bPlay[kPlayButtons] = { TRUE, FALSE, FALSE };

/////////////////////////////////////////////////////////////////////////////
// CDlgCharChart message map

BEGIN_MESSAGE_MAP(CDlgCharChart, CPropertySheet)
    //{{AFX_MSG_MAP(CDlgCharChart)
    ON_WM_CREATE()
    ON_COMMAND(IDOK, OnOK)
    ON_COMMAND(IDCANCEL, OnCancel)
    ON_WM_CLOSE()
    //}}AFX_MSG_MAP
    // Custom messages
    ON_COMMAND_RANGE(kPlayButtonIdFirst, kPlayButtonIdFirst+kPlayButtons, OnChecked)
    ON_MESSAGE(WM_USER_CHARSELECT, OnCharSelect)
    ON_COMMAND(IDHELP, OnHelpChart)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCharChart construction/destruction/creation

/***************************************************************************/
// CDlgCharChart::CDlgCharChart Constructor
/***************************************************************************/
CDlgCharChart::CDlgCharChart(LPCTSTR pszCaption, CWnd * pParent, UINT iSelectPage)
    : CPropertySheet(pszCaption, pParent, iSelectPage)
{
    //{{AFX_DATA_INIT(CDlgCharChart)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    // add the property sheet pages
    AddPage(&m_dlgVowelsPage);
    AddPage(&m_dlgConsonantsPage);
    AddPage(&m_dlgDiacriticsPage);
    AddPage(&m_dlgSuprasegsPage);
    m_pFont = NULL;
    m_nCharTable = IPA;
    //SDM 1.06.6U2
    //    m_nDisplayMode = ALL;
    m_nAnnotationFont = 0;
    m_szString.Empty();
    // construct the character tables
    m_apCharTables[IPA] = new CIPATable();
}

CDlgCharChart::~CDlgCharChart()
{
    CleanUp();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCharChart helper functions

/***************************************************************************/
// CDlgCharChart::DoDataExchange Data exchange
/***************************************************************************/
void CDlgCharChart::DoDataExchange(CDataExchange * pDX)
{
    CPropertySheet::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgCharChart)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

/***************************************************************************/
// CDlgCharChart::CleanUp
/***************************************************************************/
void CDlgCharChart::CleanUp()
{
    for (int nLoop = 0; nLoop < MAX_NUMBER_TABLES; nLoop++)
    {
        if (m_apCharTables[nLoop])
        {
            delete m_apCharTables[nLoop];
            m_apCharTables[nLoop] = NULL;
        }
    }
}

/***************************************************************************/
// CDlgCharChart::ChangeButtons
// This property sheet never uses an Apply button. The Apply button
// is deleted and a Help button put in its place.
/***************************************************************************/
void CDlgCharChart::ChangeButtons()
{
    CWnd * pWndApply = GetDlgItem(ID_APPLY_NOW); // get pointers to the button objects
    CWnd * pWndCancel = GetDlgItem(IDCANCEL);
    CWnd * pWndOK = GetDlgItem(IDOK);
    CRect rButton;
    pWndApply->GetWindowRect(rButton); // get coordinates of apply button
    ScreenToClient(rButton);
    rButton.OffsetRect(0, rButton.Height());
    m_cHelp.Create(_T("&Help"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, rButton, this, IDHELP);
    m_cHelp.SetFont(pWndOK->GetFont());
    pWndApply->DestroyWindow(); // delete apply button (not needed)
    pWndCancel->GetWindowRect(rButton);
    ScreenToClient(rButton);
    rButton.OffsetRect(0, rButton.Height());
    pWndCancel->MoveWindow(rButton);
    pWndOK->GetWindowRect(rButton);
    ScreenToClient(rButton);
    rButton.OffsetRect(0, rButton.Height());
    pWndOK->MoveWindow(rButton);

    // Resize dialog box
    CRect rWnd;
    GetWindowRect(rWnd);
    SetWindowPos(&wndTop, rWnd.left, rWnd.top, rWnd.Width(), rWnd.Height() + rButton.Height(), SWP_SHOWWINDOW);
}

/***************************************************************************/
// CDlgCharChart::SetInitialState Init the dialog
// There are four parameters possible. pFont contains a pointer to a font,
// which will be used to display the characters. If the pointer is NULL, the
// windows standard font will be used (default NULL). nCharTable sets up the
// right character table to display (default IPA).  pszText points to a CString
// object, which contains a string to put into the edit control. If this
// pointer is NULL (default), no string will be put.
/***************************************************************************/
void CDlgCharChart::SetInitialState(CFont * pFont, int nCharTable, CString * pszText)
{
    m_pFont = pFont;
    m_nCharTable = nCharTable;
    if (pszText)
    {
        m_szString = *pszText;
    }
    // prepare the font for the characters
    LOGFONT logFont;
    if (!m_pFont)
    {
        CFont * pFont = CFont::FromHandle((HFONT)::GetStockObject(SYSTEM_FONT));
        if (pFont)
        {
            pFont->GetObject(sizeof(LOGFONT), (void *)&logFont);    // fill up logFont
        }
    }
    else
    {
        m_pFont->GetObject(sizeof(LOGFONT), (void *)&logFont);    // fill up logFont
    }
    logFont.lfHeight = FONT_SIZE; // set constant character size
    m_CharFont.CreateFontIndirect(&logFont); // create the character font
    // setup the initial character table
    m_apCharTables[IPA]->SetupTable(&m_CharFont, this, &m_dlgVowelsPage, &m_dlgConsonantsPage, &m_dlgDiacriticsPage, &m_dlgSuprasegsPage);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCharChart message handlers

//SDM 1.06.6U2 Remove unused combobox & move buttons and edit box
/***************************************************************************/
// CDlgCharChart::OnCreate Dialog creation
/***************************************************************************/
int CDlgCharChart::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }
    return 0;
}

/***************************************************************************/
// CDlgCharChart::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgCharChart::OnInitDialog()
{
    CPropertySheet::OnInitDialog();
    // RLJ 08/10/2000
    // delete apply button and move cancel and ok buttons
    ChangeButtons();
    SetupControls();
    return TRUE;
}

/***************************************************************************/
// CDlgCharChart::OnCharSelect A character has been selected
// lParam contains a pointer to the CString, which contains the selected
// character.
//***************************************************************************/
LRESULT CDlgCharChart::OnCharSelect(WPARAM /*wParam*/, LPARAM lParam)
{
    wndEdit.GetWindowText(m_szString);
    m_szString += *((CString *)lParam);
    wndEdit.SetWindowText(m_szString);
    wndEdit.SetSel(0, -1); // select all the text and scroll caret into view
    return 0;
}

/***************************************************************************/
// CDlgCharChart::OnOK OK button hit
/***************************************************************************/
void CDlgCharChart::OnOK()
{
    // get the character string
    TCHAR szBuffer[128];
    wndEdit.GetWindowText(szBuffer, 127);
    m_szString = szBuffer;
    EndDialog(IDOK);
}

/***************************************************************************/
// CDlgCharChart::OnCancel Button cancel hit
/***************************************************************************/
void CDlgCharChart::OnCancel()
{
    EndDialog(IDCANCEL);
}

/***************************************************************************/
// CDlgCharChart::OnClose Close the dialog
/***************************************************************************/
void CDlgCharChart::OnClose()
{
    EndDialog(ID_CLOSE);
}

void CDlgCharChart::SetupControls()
{
    // add the edit control, the combobox and the text
    CRect rOkButton, rCancelButton;

    GetDlgItem(IDCANCEL)->GetWindowRect(rCancelButton);
    ScreenToClient(rCancelButton);
    GetDlgItem(IDOK)->GetWindowRect(rOkButton);
    ScreenToClient(rOkButton);

    // get the font for the text controls
    CFont * pFont = GetDlgItem(IDOK)->GetFont(); // get the standard font

    LOGFONT logFont;
    pFont->GetObject(sizeof(LOGFONT), (void *)&logFont); // fill up logFont

    // modify the logFont
    logFont.lfWeight = FW_NORMAL; // not bold
    m_TextFont.CreateFontIndirect(&logFont); // create the modified font

    m_bIPAHelpInstalled = checkIPAHelp();

    if (!m_bIPAHelpInstalled)
    {
        AfxMessageBox(IDS_ERROR_NO_IPAHELP, MB_OK|MB_ICONEXCLAMATION, 0);
        m_bPlay[kPlaySound] = FALSE;
    }

    // move checkboxes over buttons
    CRect rCntrl, rDlg;
    int nTextWidth = 75; // Approximate size of segment checkbox in pixels
    int nBorder = rCancelButton.left - rOkButton.right;
    GetWindowRect(rDlg);
    ScreenToClient(rDlg);
    int nLeft = rOkButton.left + (rOkButton.Width() - nTextWidth) * 3 / 2;
    int nTop = rOkButton.top - nBorder - rOkButton.Height();
    for (int i = 0; i < kPlayButtons; i++)
    {
        LPCTSTR szText[kPlayButtons] = { _T("IPA Sounds"), _T("Word"), _T("Segment") };
        rCntrl.SetRect(nLeft, nTop, nLeft + nTextWidth, nTop + rOkButton.Height());
        wndPlayButton[i].Create(szText[i], BS_AUTOCHECKBOX | WS_CHILD | WS_VISIBLE | WS_TABSTOP, rCntrl, this, kPlayButtonIdFirst + i);
        wndPlayButton[i].SetFont(&m_TextFont);
        wndPlayButton[i].SetCheck(m_bPlay[i]);
        nLeft += nTextWidth + nBorder;
    }
    wndPlayButton[kPlaySound].EnableWindow(m_bIPAHelpInstalled);
    wndPlayButton[kPlayWord].EnableWindow(m_bPlay[kPlaySound]);
    wndPlayButton[kPlaySegment].EnableWindow(m_bPlay[kPlaySound]);

    // move edit box caption
    CString szText;
    szText.LoadString(IDS_CHART_EDITTEXT);

    // get textmetrics
    CDC * pDC = GetDC(); // device context
    CFont * oldFont = (CFont *) pDC->SelectObject(&m_TextFont);
    int nLineBreak = szText.Find('\n');
    CSize size;
    if (nLineBreak != -1)
    {
        CSize sizeLeft = pDC->GetTextExtent(szText.Left(nLineBreak));
        CSize sizeRight = pDC->GetTextExtent(szText.Mid(nLineBreak + 1));

        size = sizeRight.cx > sizeLeft.cx ? sizeRight : sizeLeft;
    }
    else
    {
        size = pDC->GetTextExtent(szText);
    }
    pDC->SelectObject(oldFont);  // set back old font
    ReleaseDC(pDC);
    pDC = NULL;
    nTextWidth = size.cx;

    nBorder = rCancelButton.left - rOkButton.right;
    nLeft = nBorder + 1;
    nTop += 3; // add some white-space
    rCntrl.SetRect(nLeft, nTop, nLeft + nTextWidth, nTop + size.cy * 3 / 2);
    wndEditText.Create(szText, WS_CHILD | WS_VISIBLE, rCntrl, this, 104);
    wndEditText.SetFont(&m_TextFont);

    // move edit box
    nTop = rCntrl.bottom;
    nTextWidth = rOkButton.left - 3 * nBorder;

    // get font height
    TEXTMETRIC tm;
    pDC = GetDC(); // get device context
    oldFont = pDC->SelectObject(m_pFont); // select x-scale font
    pDC->GetTextMetrics(&tm); // get text metrics
    pDC->SelectObject(oldFont);  // set back old font
    ReleaseDC(pDC);
    int nTextHeight = 11 * tm.tmHeight / 10 + 4; // return window height
    nTextHeight = nTextHeight < rOkButton.Height() ? rOkButton.Height() : nTextHeight;

    // adjust dialog or edit box size, if needed
    int nBottom = nTop + nTextHeight;
    if (nBottom > rOkButton.bottom)
    {
        MoveWindow(rDlg + CRect(0, 0, 0, nBottom - rOkButton.bottom));
    }
    else
    {
        nTextHeight += rOkButton.bottom - nBottom;    // line up bottom of edit box with buttons
    }

    rCntrl.SetRect(nLeft, nTop, nLeft + nTextWidth, nTop + nTextHeight);
    wndEdit.Create(WS_CHILD | WS_BORDER | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, rCntrl, this, 101);

    // fill the edit control and set the font
    wndEdit.SetWindowText(m_szString);
    if (m_pFont)
    {
        wndEdit.SetFont(m_pFont);
    }

    // delete apply button and move cancel and ok buttons
    CenterWindow();
}

void CDlgCharChart::OnChecked(UINT nID)
{
    int i = nID - kPlayButtonIdFirst;

    m_bPlay[i] = wndPlayButton[i].GetCheck();

    if (i == kPlaySound)
    {
        wndPlayButton[kPlayWord].EnableWindow(m_bPlay[kPlaySound]);
        wndPlayButton[kPlaySegment].EnableWindow(m_bPlay[kPlaySound]);
    }
}

/***************************************************************************/
// CDlgCharChart::OnHelpChart Call Character Chart help
/***************************************************************************/
void CDlgCharChart::OnHelpChart()
{
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/Edit/Character_Chart.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}
