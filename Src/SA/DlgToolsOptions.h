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
#define _TOOLS_OPTIONS_H

#include "sa_dlwnd.h"
#include "colors.h"
#include "ToolSettings.h"

typedef struct SColorComboInfo {
    TCHAR   *  pszColorItem;
    COLORREF  cColor;
} ColorComboInfo;

//###########################################################################
// CComboColor combobox with colors
// DDO - 08/11/00
/////////////////////////////////////////////////////////////////////////
// CComboColor defines

class CComboColor : public CComboBox {

private:
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

};

//###########################################################################
// CDlgOptionsViewPage property page

class CDlgOptionsViewPage : public CPropertyPage {

public:
    CDlgOptionsViewPage(); // standard constructor

    void EnableDynamicUpdate(BOOL bState = TRUE);
    void SetGridStyle(int XStyle, int YStyle);
    void GetGridStyle(int * XStyle, int * YStyle);

    enum { IDD = IDD_OPTIONSVIEWPAGE };
    BOOL m_bStatusbar;
    BOOL m_bToolbar;
    BOOL m_bScrollZoom;
    int m_nCaptionStyle;
    BOOL m_bXGrid;
    BOOL m_bYGrid;
    int m_nXStyleIndex;
    int m_nYStyleIndex;
    int m_nCursorAlignment;
    int m_nPitchMode;
    int m_nPosMode;
    BOOL m_bToneAbove;
    int m_nGraphUpdateMode;
    BOOL m_bAnimate;
    int m_nAnimationRate;
    BOOL m_bTaskbar;

    static const int IndexToStyle[];

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    virtual BOOL OnInitDialog();
    afx_msg void OnModifiedStatusbar();
    afx_msg void OnModified();
    afx_msg void OnModifiedRange();
    afx_msg void OnModifiedAnimate();
    afx_msg void OnAnimationRateScroll();
    afx_msg void OnChange();

    DECLARE_MESSAGE_MAP()

private:
    CComboGridlines  m_xGridlines;
    CComboGridlines  m_yGridlines;
    CSpinControl     m_SpinAnimationRate;
    BOOL m_bModified; // TRUE if page member data modified

};

//###########################################################################
// CDlgOptionsColorPage property page

class CDlgOptionsColorPage : public CPropertyPage {

public:
    CDlgOptionsColorPage(); // standard constructor

    BOOL m_bColorsChanged;  // TRUE, if colors changed by user
    Colors m_cColors;       // internal color structure

    enum { IDD = IDD_OPTIONSCOLORSPAGE };
    int m_nGraphSelect;
    int m_nAnnotationSelect;
    int m_nScaleSelect;
    int m_nOverlaySelect;

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void FillColorComboBoxInfo(BOOL bAddStrings);
    BOOL ChangeColor(COLORREF * pColor);
    virtual BOOL OnInitDialog();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDefault();
    afx_msg void OnSystem();
    afx_msg void OnChgColorAnnot();
    afx_msg void OnChgColorGraph();
    afx_msg void OnChgColorOvrly();
    afx_msg void OnChgColorScale();

    CComboColor m_GraphItemColors;
    CComboColor m_AnnotItemColors;
    CComboColor m_ScaleItemColors;
    CComboColor m_OvrlyItemColors;
    CFont m_Font;

    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgOptionsFontPage property page

class CDlgOptionsFontPage : public CPropertyPage {

public:
    CDlgOptionsFontPage();  // standard constructor
    ~CDlgOptionsFontPage(); // standard destructor

    BOOL m_bFontChanged;            // TRUE, if fonts changed by user
    CStringArray m_GraphFonts;      // array of graph font face strings
    CUIntArray m_GraphFontSizes;    // array of graph font sizes
    BOOL m_bUseUnicodeEncoding;     // Experimental....

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    virtual BOOL OnInitDialog();
    afx_msg void OnFont();
    afx_msg void OnSelChangeAnnotList();

    DECLARE_MESSAGE_MAP()

private:
    enum { IDD = IDD_OPTIONSFONTPAGE };
    CListBox  m_lbAnnotList;
    CString m_szAnnotFontInfo;
    int   m_nAnnotListIndex;
    CString m_szSampleText;
    CFont m_Font;
    CFont * m_pSampleFont;
};

//###########################################################################
// CDlgOptionsSavePage property page

class CDlgOptionsSavePage : public CPropertyPage {

public:
    CDlgOptionsSavePage(); // standard constructor

    enum { IDD = IDD_OPTIONSSAVEPAGE };
    CStatic m_TempIcon;
    CStatic m_PermIcon;
    BOOL  m_saveOpenFiles;
    BOOL  m_showStartupDlg;
    CString m_szPermGraphs;
    CString m_szTempGraphs;
    CString m_szPermCurrLabel;
    CString m_szTempCurrLabel;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);
    void ShowPermanentView();
    void ShowTemporaryView();
    void SetStartDlgCheckHelp(void);
    virtual BOOL OnInitDialog();
    afx_msg void OnSaveTempDefaultTemplate();
    afx_msg void OnSavePermDefaultTemplate();
    afx_msg void OnReopenFiles();
    afx_msg void OnShowsStartupDlg();

    DECLARE_MESSAGE_MAP()

private:
    CFont m_Font;
};

class CDlgOptionsAudioPage : public CPropertyPage {

public:
    CDlgOptionsAudioPage(); // standard constructor

    enum { IDD = IDD_OPTIONSAUDIOPAGE };
    BOOL m_bShowAdvancedAudio;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
};

class CDlgToolsOptions : public CPropertySheet {

    DECLARE_DYNAMIC(CDlgToolsOptions)

public:
    CDlgToolsOptions(LPCTSTR pszCaption, CWnd * pParent, bool fullView);
    void ApplyNow();
    void OnHelpToolsOptions();

    CDlgOptionsViewPage m_dlgViewPage;
    CDlgOptionsColorPage m_dlgColorPage;
    CDlgOptionsFontPage m_dlgFontPage;
    CDlgOptionsSavePage m_dlgSavePage;
    CDlgOptionsAudioPage m_dlgAudioPage;

    CToolSettings GetSettings(bool fullView);

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    virtual BOOL OnInitDialog();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnApplyNow();
    void ChangeButtons();                               // delete Apply button, move other buttons

    CButton m_cHelp;
    bool fullView;

    DECLARE_MESSAGE_MAP()
};

#endif
