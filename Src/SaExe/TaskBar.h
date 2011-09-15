#if !defined(AFX_TASKBAR_H__C90DA083_8812_11D5_9FE4_00E0988A4887__INCLUDED_)

#include "resource.h"

#define AFX_TASKBAR_H__C90DA083_8812_11D5_9FE4_00E0988A4887__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#pragma warning(disable : 4097) // typedef name used as synonym
#endif // _MSC_VER > 1000
// TaskBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTaskBar dialog

class CSaTaskItem : public LVITEM
{
public:
  CSaTaskItem(const LVITEM item) : LVITEM(item) {;}
  CString szLabel;
  CString szTip;
};

class CTaskPage 
{
public:
  CTaskPage() { m_pImageList[0] = m_pImageList[1] = m_pImageList[2] = NULL; }
  virtual ~CTaskPage();


  CString m_szPageName;
  std::vector<CSaTaskItem> m_cItemList;
  CImageList *m_pImageList[3]; // normal, small, state
};

/////////////////////////////////////////////////////////////////////////////
// CTaskButton window

class CTaskButton : public CButton
{
// Construction
public:
	CTaskButton(LPCTSTR szCaption, CWnd *pParent, UINT nID);

// Attributes
public:
  BOOL m_bSelected;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTaskButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTaskButton)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

class CTaskBar : public CDialogBar
{
	DECLARE_DYNAMIC(CTaskBar)

// Construction
public:
  CTaskBar();   
  virtual ~CTaskBar();  
  
  void AddPage(CTaskPage *pPage);
  void Clear();

  int SelectPage(int nPage);
  void UpdateLayout();

  enum {kButtonHeight = 23, kSelectPageFirst = 1000, kSelectPageLast = 1100};

  // Dialog Data
protected:
  std::vector<CTaskPage *> m_pPages;
  int m_nSelectedPage;
  int m_nHotItem;
  CSize m_cAvailableSize;
  std::vector<CButton *> m_pPageButtons;
  //{{AFX_DATA(CTaskBar)
	enum { IDD = IDD_TASKBAR };
	CListCtrl	m_cList;
	//}}AFX_DATA
  
  // Overrides
  virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
  virtual BOOL SetStatusText(int nHit);
  
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTaskBar)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  
  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CTaskBar)
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnSysColorChange();
  afx_msg void OnSetfocusTaskList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetInfoTip(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
  afx_msg void OnTaskItem(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDrawTaskItem(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnSelectPage(UINT wPageID);
  afx_msg void OnSelectPageUpdate(CCmdUI* pCmdUI);
  afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
  afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
  DECLARE_MESSAGE_MAP()
};

void SetupTaskBar(CTaskBar &m_wndTaskBar);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TASKBAR_H__C90DA083_8812_11D5_9FE4_00E0988A4887__INCLUDED_)
