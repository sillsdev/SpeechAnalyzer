/////////////////////////////////////////////////////////////////////////////
// ch_dlg.h:
// Interface of the CDlgChartVowelsPage (property page)
//                  CDlgChartConsonantsPage (property page)
//                  CDlgChartDiacriticsPage (property page)
//                  CDlgChartSuprasegsPage (property page)
//                  CDlgCharChart (property sheet)          classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "ch_dlwnd.h"
#include "ch_table.h"
#include "resource.h"

extern bool checkIPAHelp();

//###########################################################################
// CDlgChartVowelsPage property page

class CDlgChartVowelsPage : public CPropertyPage
{
    // Construction/destruction/creation
public:
    CDlgChartVowelsPage(); // standard constructor

    // Attributes
private:
    CChartText m_ChartText[8];
    CChartLine m_ChartLine[18];

    // Dialog Data
    //{{AFX_DATA(CDlgChartVowelsPage)
    enum { IDD = IDD_CHARTVOWELSPAGE };
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgChartVowelsPage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgChartConsonantsPage property page

class CDlgChartConsonantsPage : public CPropertyPage
{
    // Construction/destruction/creation
public:
    CDlgChartConsonantsPage(); // standard constructor

    // Attributes
private:
    CChartText m_ChartText[87];

    // Dialog Data
    //{{AFX_DATA(CDlgChartConsonantsPage)
    enum { IDD = IDD_CHARTCONSONANTSPAGE };
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgChartConsonantsPage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgChartDiacriticsPage property page

class CDlgChartDiacriticsPage : public CPropertyPage
{
    // Construction/destruction/creation
public:
    CDlgChartDiacriticsPage();    // standard constructor

    // Attributes
private:
    CChartText m_ChartText[50];

    // Dialog Data
    //{{AFX_DATA(CDlgChartDiacriticsPage)
    enum { IDD = IDD_CHARTDIACRITICSPAGE };
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgChartDiacriticsPage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgChartSuprasegsPage property page

class CDlgChartSuprasegsPage : public CPropertyPage
{

    // Construction/destruction/creation
public:
    CDlgChartSuprasegsPage();    // standard constructor

    // Attributes
private:
    CChartText m_ChartText[48];

    // Dialog Data
    //{{AFX_DATA(CDlgChartSuprasegsPage)
    enum { IDD = IDD_CHARTSUPRASEGSPAGE };
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgChartSuprasegsPage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgCharChart property sheet

// CDlgCharChart defines
#define EDIT_HEIGHT        30 // height of edit control
#define FONT_SIZE         -18 // character font size
#define MAX_NUMBER_TABLES   1 // number of character tables

// character tables
enum Table { IPA=0,};
// display modes
enum DisplayModes { ALL=0,};

//SDM 1.06.6U2 remove unused combobox move buttons
class CDlgCharChart : public CPropertySheet
{
    DECLARE_DYNAMIC(CDlgCharChart)

// Construction/destruction/creation
public:
    CDlgCharChart(LPCTSTR pszCaption, CWnd * pParent, UINT iSelectPage);
    virtual ~CDlgCharChart();

    // Attributes
private:
    CDlgChartVowelsPage     m_dlgVowelsPage;
    CDlgChartConsonantsPage m_dlgConsonantsPage;
    CDlgChartDiacriticsPage m_dlgDiacriticsPage;
    CDlgChartSuprasegsPage  m_dlgSuprasegsPage;

    CEdit       wndEdit;
    CStatic     wndEditText;
    CFont       m_TextFont;
    CFont       m_CharFont;
    CFont   *   m_pFont;
    int         m_nCharTable; // used character table
    int         m_nAnnotationFont; // used font comes from this annotation window
    CCharTable * m_apCharTables[MAX_NUMBER_TABLES]; // character table array
    bool        m_bIPAHelpInstalled;

public:
    CString   m_szString;
    enum { kPlaySound, kPlayWord, kPlaySegment, kPlayButtons, kPlayButtonIdFirst = 110 };
    static BOOL m_bPlay[kPlayButtons];

private:
    CButton   wndPlayButton[kPlayButtons];

    // Operations
public:
    void SetInitialState(CFont * pFont, int nCharTable = IPA, CString * pszText = NULL); // init the dialog
    CFont * GetTextFont()
    {
        return &m_TextFont;   // return pointer to text font
    }
    CCharTable * GetTable(int nIndex)
    {
        return m_apCharTables[nIndex];   // return pointer to character table
    }
    void OnHelpChart();
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void CleanUp();

private:
    void ChangeButtons(); // delete Apply button, move other buttons
    CButton m_cHelp;
    void SetupControls();

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgCharChart)
    virtual BOOL OnInitDialog();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnOK();
    afx_msg void OnCancel();
    afx_msg void OnClose();
    //}}AFX_MSG
    afx_msg void OnChecked(UINT nID);
    afx_msg LRESULT OnCharSelect(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()
};
