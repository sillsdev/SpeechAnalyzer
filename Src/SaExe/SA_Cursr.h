/////////////////////////////////////////////////////////////////////////////
// sa_cursr.h:
// Interface of the CStartCursorWnd
//                  CStopCursorWnd
//                  CPrivateCursorWnd classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
//
// Revisions
//  1.06.2
//       SDM  Added message handlers OnKey... to Start&Stop cursors
//  1.06.6U4
//       SDM  Added function ChangePosition
//  1.5Test10.4
//       SDM Made cursor windows dependent on CCursorWnd (new class)
//       SDM added bDrawn flag
/////////////////////////////////////////////////////////////////////////////

#ifndef SA_CURSR_DEF
#define SA_CURSR_DEF

class CCursorWnd : public CWnd 
{
	// Construction/destruction/creation
public:
	CCursorWnd();
	BOOL bDrawn;

	// Operations
public:
	BOOL IsDrawn();
	void SetDrawn(BOOL bNewDrawn);
};

//###########################################################################
// CStartCursorWnd window

class CStartCursorWnd : public CCursorWnd
{

	// Construction/destruction/creation
public:
	CStartCursorWnd();
	virtual ~CStartCursorWnd();
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnDraw(CDC * pDC, const CRect & printRect);


	// Attributes
private:
	CRect    m_rWnd;				// cursor window coordinates
	BOOL     m_bCursorDrag;			// cursor drag by mouse
	DWORD    m_dwStartDragPos;		// cursor position when starting drag
	DWORD    m_dwDragPos;			// current drag position
	int      m_nEditBoundaries;		// editing boundaries

	// Operations
public:
	void ResetPosition();			// reset old cursor position
	DWORD CalculateCursorPosition(CView*, int nPosition, int nWidth, DWORD* pStopCursor); // calculate the current cursor position (in data samples)
	BOOL IsDragging();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStartCursorWnd)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CStopCursorWnd window

class CStopCursorWnd : public CCursorWnd
{

	// Construction/destruction/creation
public:
	CStopCursorWnd();
	virtual ~CStopCursorWnd();
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// Attributes
private:
	CRect    m_rWnd;				// cursor window coordinates
	BOOL     m_bCursorDrag;			// cursor drag by mouse
	DWORD    m_dwDragPos;			// current drag position
	int      m_nEditBoundaries;		// editing boundaries


	// Operations
public:
	void ResetPosition();			// reset old cursor position
	DWORD CalculateCursorPosition(CView*, int nPosition, int nWidth, DWORD* pStartCursor); // calculate the current cursor position (in data samples)
	BOOL IsDragging();
	virtual void OnDraw(CDC * pDC, const CRect & printRect);

	// Generated message map functions
protected:
	//{{AFX_MSG(CStopCursorWnd)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CPrivateCursorWnd window

class CPrivateCursorWnd : public CCursorWnd
{

	// Construction/destruction/creation
public:
	CPrivateCursorWnd();
	virtual ~CPrivateCursorWnd();
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// Attributes
private:
	BOOL     m_bCursorDrag;			// cursor drag by mouse
	CRect    m_rWnd;				// cursor window coordinates

	// Operations
public:
	void ResetPosition();			// reset old cursor position
	virtual void OnDraw(CDC * pDC, const CRect & printRect);
	virtual void OnDrawHorizontalCursor(CDC * pDC, const CRect & printRect);
	void ChangeCursorPosition(CPoint point);
	void ChangeHorizontalCursorPosition(CPoint point);
	BOOL IsDragging();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPrivateCursorWnd)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

