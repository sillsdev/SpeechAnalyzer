#ifndef STOPCURSORWND_H
#define STOPCURSORWND_H

#include "SA_Cursr.h"

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
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()
};

#endif

