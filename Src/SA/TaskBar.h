#ifndef TASKBAR_H
#define TASKBAR_H

/////////////////////////////////////////////////////////////////////////////
// CTaskBar dialog
class CSaTaskItem : public LVITEM {
public:
    CSaTaskItem(const LVITEM item);
    CString szLabel;
    CString szTip;
};

class CTaskPage {
public:
    CTaskPage( LPCTSTR szCaption);
    virtual ~CTaskPage();
    
	CString m_szPageName;
    std::vector<CSaTaskItem> itemList;
    CImageList imageList;
};

/////////////////////////////////////////////////////////////////////////////
// CTaskButton window
class CTaskButton : public CButton {
public:
    CTaskButton(LPCTSTR szCaption, CWnd * pParent, UINT nID);
    virtual ~CTaskButton();

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    BOOL m_bSelected;

protected:
    DECLARE_MESSAGE_MAP()
};

class CTaskBar : public CDialogBar {
    DECLARE_DYNAMIC(CTaskBar)

public:
    CTaskBar();
    virtual ~CTaskBar();

    void AddPage(CTaskPage * pPage);
    void Clear();
	void Setup();

    int SelectPage(int nPage);
    void UpdateLayout();

    enum {kButtonHeight = 23, kSelectPageFirst = 1000, kSelectPageLast = 1100};

protected:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSetfocusTaskList(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnGetInfoTip(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnTaskItem(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnDrawTaskItem(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnSelectPage(UINT wPageID);
    afx_msg void OnSelectPageUpdate(CCmdUI * pCmdUI);
    afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
    afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
    virtual void OnUpdateCmdUI(CFrameWnd * pTarget, BOOL bDisableIfNoHndler);
    virtual BOOL SetStatusText(int nHit);
    virtual void DoDataExchange(CDataExchange * pDX);

    std::vector<CTaskPage *> m_pPages;
    int m_nSelectedPage;
    int m_nHotItem;
    std::vector<CButton *> m_pPageButtons;
    CListCtrl m_cList;

    enum { IDD = IDD_TASKBAR };

    DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};

#endif
