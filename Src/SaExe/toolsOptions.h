/////////////////////////////////////////////////////////////////////////////
// toolsOptions.h:
// Interface of the CDlgOptionsViewPage (property page)
//                  CDlgOptionsColorPage (property page)
//                  CDlgOptionsFontPage (property page)
//                  CDlgOptionsMemoryPage (property page)
//                  CDlgOptionsSavePage (property page)
//                  CDlgToolsOptions (property sheet)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg2.h
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _TOOLS_OPTIONS_H

#include "sa_dlwnd.h"
#include "colors.h"
#include "resource.h"

#define _TOOLS_OPTIONS_H


typedef struct {
  TCHAR     *pszColorItem;
  COLORREF  cColor;
} ColorComboInfo;

//###########################################################################
// CComboColor combobox with colors
// DDO - 08/11/00
/////////////////////////////////////////////////////////////////////////
// CComboColor defines

class CComboColor : public CComboBox
{
  // Construction/destruction/creation
public:
  // Attributes
private:

  // Operations
private:
  virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
public:

};

//###########################################################################
// CDlgOptionsViewPage property page

class CDlgOptionsViewPage : public CPropertyPage
{

  // Construction/destruction/creation
public:
  CDlgOptionsViewPage(); // standard constructor

  // Attributes
private:
  CComboGridlines  m_xGridlines;
  CComboGridlines  m_yGridlines;
  CSpinControl     m_SpinAnimationRate;
  BOOL m_bModified; // TRUE if page member data modified
  static const int IndexToStyle[];

  // Dialog Data
public:
  //{{AFX_DATA(CDlgOptionsViewPage)
	enum { IDD = IDD_OPTIONSVIEWPAGE };
  BOOL              m_bStatusbar;
  BOOL              m_bToolbar;
  BOOL              m_bScrollZoom;
  int               m_nCaptionStyle;
  BOOL              m_bXGrid;
  BOOL              m_bYGrid;
  int               m_nXStyleIndex;
  int               m_nYStyleIndex;
  int               m_nCursorAlignment;
  int               m_nPitchMode;
  int               m_nPosMode;
  BOOL              m_bToneAbove;
  int               m_nGraphUpdateMode;
  BOOL              m_bAnimate;
  int               m_nAnimationRate;
  BOOL	m_bTaskbar;
  //}}AFX_DATA

  // Operations
public:
  void EnableDynamicUpdate(BOOL bState = TRUE);
  void SetGridStyle(int XStyle, int YStyle);
  void GetGridStyle(int *XStyle, int *YStyle);

protected:
  virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

  // Generated message map functions
protected:
  //{{AFX_MSG(CDlgOptionsViewPage)
  virtual BOOL OnInitDialog();
  afx_msg void OnModifiedStatusbar();
  afx_msg void OnModified();
  afx_msg void OnModifiedRange();
  afx_msg void OnModifiedAnimate();
  afx_msg void OnAnimationRateScroll();
  afx_msg void OnChange();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgOptionsColorPage property page

class CDlgOptionsColorPage : public CPropertyPage
{

  // Construction/destruction/creation
public:
  CDlgOptionsColorPage(); // standard constructor

  // Attributes
public:
  BOOL    m_bColorsChanged; // TRUE, if colors changed by user
  Colors  m_cColors;        // internal color structure

  // Dialog Data
  //{{AFX_DATA(CDlgOptionsColorPage)
  enum { IDD = IDD_OPTIONSCOLORSPAGE };
  int     m_nGraphSelect;
  int     m_nAnnotationSelect;
  int     m_nScaleSelect;
  int     m_nOverlaySelect;
  //}}AFX_DATA

  // Operations
protected:
  virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

  CComboColor m_GraphItemColors;
  CComboColor m_AnnotItemColors;
  CComboColor m_ScaleItemColors;
  CComboColor m_OvrlyItemColors;

  void FillColorComboBoxInfo(BOOL bAddStrings);
  BOOL ChangeColor(COLORREF* pColor);

  // Generated message map functions
protected:
  CFont m_Font;

  //{{AFX_MSG(CDlgOptionsColorPage)
  virtual BOOL OnInitDialog();
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnDefault();
  afx_msg void OnSystem();
  afx_msg void OnChgColorAnnot();
  afx_msg void OnChgColorGraph();
  afx_msg void OnChgColorOvrly();
  afx_msg void OnChgColorScale();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgOptionsFontPage property page

class CDlgOptionsFontPage : public CPropertyPage
{

  // Construction/destruction/creation
public:
  CDlgOptionsFontPage();  // standard constructor
  ~CDlgOptionsFontPage(); // standard destructor

  // Attributes
public:
  BOOL         m_bFontChanged;   // TRUE, if fonts changed by user
  CStringArray m_GraphFonts;     // array of graph font face strings
  CUIntArray   m_GraphFontSizes; // array of graph font sizes

  BOOL	m_bUseUnicodeEncoding; // Experimental....

private:
  // Dialog Data
  //{{AFX_DATA(CDlgOptionsFontPage)
	enum { IDD = IDD_OPTIONSFONTPAGE };
  CListBox  m_lbAnnotList;
  CString m_szAnnotFontInfo;
  int   m_nAnnotListIndex;
  CString m_szSampleText;
  //}}AFX_DATA

  CFont m_Font;
  CFont *m_pSampleFont;

  // Operations
protected:
  virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

  // Generated message map functions
protected:
  //{{AFX_MSG(CDlgOptionsFontPage)
  virtual BOOL OnInitDialog();
  afx_msg void OnFont();
  afx_msg void OnSelChangeAnnotList();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgOptionsSavePage property page

class CDlgOptionsSavePage : public CPropertyPage
{

  // Construction/destruction/creation
public:
  CDlgOptionsSavePage(); // standard constructor

  // Attributes

  // Dialog Data
  //{{AFX_DATA(CDlgOptionsSavePage)
  enum { IDD = IDD_OPTIONSSAVEPAGE };
  CStatic m_TempIcon;
  CStatic m_PermIcon;
  BOOL  m_saveOpenFiles;         // tdg - 09/03/97
  BOOL  m_showStartupDlg;        // DDO - 08/03/00
  CString m_szPermGraphs;
  CString m_szTempGraphs;
  CString m_szPermCurrLabel;
  CString m_szTempCurrLabel;
  //}}AFX_DATA
private:
  CFont m_Font;
  // Operations
protected:
  virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support
  void ShowCurrentDefaultViews(BOOL bPermanent);  // DDO - 08/07/00
  void SetStartDlgCheckHelp(void);                // DDO - 08/09/00

  // Generated message map functions
protected:
  //{{AFX_MSG(CDlgOptionsSavePage)
  virtual BOOL OnInitDialog();
  afx_msg void OnSaveTempDefaultTemplate();
  afx_msg void OnSavePermDefaultTemplate();
  afx_msg void OnReopenFiles();
  afx_msg void OnShowsStartupDlg();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgToolsOptions property sheet

class CDlgToolsOptions : public CPropertySheet
{   DECLARE_DYNAMIC(CDlgToolsOptions)

// Construction/destruction/creation
public:
  CDlgToolsOptions(LPCTSTR pszCaption, CWnd* pParent);

  // Attributes
public:
  CDlgOptionsViewPage m_dlgViewPage;
  CDlgOptionsColorPage m_dlgColorPage;
  CDlgOptionsFontPage m_dlgFontPage;
  CDlgOptionsSavePage m_dlgSavePage;
  void ApplyNow() { OnApplyNow(); };

  // Operations
public:
	void OnHelpToolsOptions();
protected:
  virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	void ChangeButtons(); // delete Apply button, move other buttons
	CButton	m_cHelp;

  // Generated message map functions
protected:
  //{{AFX_MSG(CDlgToolsOptions)
  virtual BOOL OnInitDialog();
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnApplyNow();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#endif //_TOOLS_OPTIONS_H
