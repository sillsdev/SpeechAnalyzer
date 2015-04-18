#ifndef STOPCURSORWND_H
#define STOPCURSORWND_H

#include "SA_Cursr.h"

//###########################################################################
// CStopCursorWnd window

class CStopCursorWnd : public CCursorWnd {

public:
    CStopCursorWnd();
    virtual ~CStopCursorWnd();
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT & rect, CWnd * pParentWnd, UINT nID, CCreateContext * pContext = NULL);
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);
	// reset old cursor position
    void ResetPosition();
	// calculate the current cursor position (in data samples)
    DWORD CalculateCursorPosition(CView *, int nPosition, int nWidth, DWORD * pStartCursor);
    BOOL IsDragging();
    virtual void OnDraw(CDC * pDC, const CRect & printRect);

protected:
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    DECLARE_MESSAGE_MAP()

private:
	// cursor window coordinates
    CRect m_rWnd;
	// cursor drag by mouse
    BOOL m_bCursorDrag;
	// current drag position
    DWORD m_dwDragPos;
	// editing boundaries
    int m_nEditBoundaries;

};

#endif

