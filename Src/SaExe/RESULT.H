// result.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResult dialog

class CResult : public CDialog
{
  // Construction
public:
  CResult(CString result, CWnd* pParent = NULL);  // standard constructor

  // Dialog Data
  //{{AFX_DATA(CResult)
  enum { IDD = IDD_RESULT };
  CString m_szResult;
  //}}AFX_DATA

  // Implementation
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  // Generated message map functions
  //{{AFX_MSG(CResult)
  virtual void OnCancel();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
