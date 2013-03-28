// pickover.h : header file
//
#ifndef _PICKOVER_H
#define _PICKOVER_H

class CGraphWnd;
class CPlotWnd;
class CSaView;

#define MAX_PLOTS MAX_LIST_SIZE

/////////////////////////////////////////////////////////////////////////////
// CPickOverlayDlg dialog

class CPickOverlayDlg : public CDialog
{
    // Construction
public:
    const CPlotWnd * GetPlot(int idx);
    void ResetPlots();
    CPickOverlayDlg(CWnd * pParent = NULL); // standard constructor

    enum { MAX_LIST_SIZE = 64 };

    // getters
    CGraphWnd * GetNextSelectedGraph(int & pos);
    BOOL IsItemSelected(int i)
    {
        return m_bIsItemSelected[i];
    };

    // setters
    void ResetGraphsPtr();
    void SetGraphsPtr(CGraphWnd * pGraphs[], CGraphWnd * pExceptGraph);
    void AddPlot(const CPlotWnd *, LPCTSTR pszDoc);

    UINT GraphsCount(void);


    // Implementation
public:
    void OnHelpPickOverlay();
protected:

    // Dialog Data
    //{{AFX_DATA(CPickOverlayDlg)
    enum { IDD = IDD_PICKOVERLAY };
    CListBox    m_SelectItems;
    //}}AFX_DATA

    BOOL    m_Mode_AddTrueRemoveFalse;
    void    SetupItemsList(void);
    const CPlotWnd * m_apPlot[MAX_PLOTS];
    UINT      m_numPlots;
    CString m_szDoc[MAX_PLOTS];
    CGraphWnd * m_apGraphs[MAX_LIST_SIZE];
    int     m_numItems;
    int     m_NumItemsSelected;
    int     m_selections[MAX_LIST_SIZE+1];
    BOOL    m_bIsItemSelected[MAX_LIST_SIZE];
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CPickOverlayDlg)
    afx_msg void OnClearallgraphs();
    afx_msg void OnSelectallgraphs();
    //}}AFX_MSG
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    DECLARE_MESSAGE_MAP()
};

#endif //_PICKOVER_H
