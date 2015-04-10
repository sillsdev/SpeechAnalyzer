/////////////////////////////////////////////////////////////////////////////
// printopt.h
// Interface of the CPrintOptions class
// This class manages the users page printing options using a dialog.
// Author: Doug Eberman
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#ifndef _PRINTOPT_
#define _PRINTOPT_

class CDlgPrintOptions : public CDialog {
public:
    // Construction & initialization
    CDlgPrintOptions(CWnd * pParent = NULL); // standard constructor
    CDlgPrintOptions(const CDlgPrintOptions & from);  // copy constructor
    CDlgPrintOptions & operator=(const CDlgPrintOptions &); // assignment operator
    void SetupGraphsList(void);
    void CopyFrom(const CDlgPrintOptions & from);

    // getters
    BOOL IsGraphSelected(int nGraph) {
        return m_bIsGraphSelected[nGraph];
    };
    BOOL IsRecGraphSelected() {
        return m_bIsRecGraphSelected;
    };
    int NumGraphsSelected(void) {
        return m_NumGraphsSelected;
    };
    int NumRows(void) {
        return m_NumRows;
    };
    int NumCols(void) {
        return m_NumCols;
    };
    int LayoutType(void) {
        return m_LayoutType;
    };
    BOOL bUseSmallFonts(void) {
        return m_bUseSmallFonts;
    };
    BOOL bFixedAspectR(void) {
        return m_bFixedAspectR;
    };
    BOOL bIsLandscape(void) {
        return (m_IsLandscape != 0);
    };
    BOOL bIsHiRes(void) {
        return (m_IsHiRes != 0);
    };
    BOOL UserClickedPrint(void) {
        return m_bUserClickedPrint;
    };
    BOOL UserClickedPreview(void) {
        return m_bUserClickedPreview;
    };
    static const int LAYOUT_AS_SCREEN() {
        return 0;
    };
    static const int LAYOUT_1GRAPH_PER_PAGE() {
        return 1;
    };
    static const int LAYOUT_CUSTOM() {
        return 2;
    };

    // setters
    void SetGraphsPtr(CGraphWnd * pGraphs[]);
    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);


// Implementation
public:
    void OnHelpPrintOptions();
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    void Collapse(void);
    void Expand(void);
    void EnableCustomLayout(BOOL enable);
    void ShowLandscapeBitmap(BOOL show);

    // Generated message map functions
    afx_msg void OnMore();
    afx_msg void OnUseScreenLayout() {
        EnableCustomLayout(FALSE);
    };
    afx_msg void OnPrintOnePerPage() {
        EnableCustomLayout(FALSE);
    }
    afx_msg void OnPrintManyPerPage() {
        EnableCustomLayout(TRUE);
    }
    afx_msg void OnSelectallgraphs();
    afx_msg void OnPrint();
    afx_msg void OnClearallgraphs();
    afx_msg void OnHires();
    afx_msg void OnPreview();
    afx_msg void OnScreenShot();
    afx_msg void OnSpinRow();
    afx_msg void OnSpinCol();
    afx_msg void OnPortrait();
    afx_msg void OnLandscape();
    DECLARE_MESSAGE_MAP()

    // Dialog Data
    enum { IDD = IDD_PRINTOPTIONS };
    BOOL m_bFixedAspectR;
    BOOL m_bUseSmallFonts;
    int m_IsLandscape;
    int m_NumColumnsLessOne;
    CString m_strNumRows;
    int m_IsHiRes;
    int m_LayoutType;
    CListBox m_listBoxGraphsToPrint;
    CString m_strNumCols;

    // internal data
    CBitmapButton m_bmpPort;
    CBitmapButton m_bmpLand;
    CSpinControl  m_SpinRow;
    CSpinControl  m_SpinCol;
    BOOL          m_bUserClickedPreview;
    BOOL          m_bUserClickedPrint;
    int           m_NumRows;
    int           m_NumCols;
    int           m_numGraphs;
    int           m_NumGraphsSelected;
    BOOL          m_IsExpanded;
    BOOL          m_bIsGraphSelected[MAX_GRAPHS_NUMBER];
    BOOL          m_bIsRecGraphSelected;
    CRect         m_WndRect;
    CGraphWnd  *  m_apGraphs[MAX_GRAPHS_NUMBER];
    int           m_selections[MAX_GRAPHS_NUMBER+1];

};


#endif
