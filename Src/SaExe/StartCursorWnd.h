#ifndef STARTCURSORWND_H
#define STARTCURSORWND_H

#include "SA_Cursr.h"

//###########################################################################
// CStartCursorWnd window

class CStartCursorWnd : public CCursorWnd
{
    // Construction/destruction/creation
public:
    CStartCursorWnd();
    virtual ~CStartCursorWnd();
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
                        const RECT & rect, CWnd * pParentWnd, UINT nID, 
						CCreateContext * pContext = NULL);
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);
    virtual void OnDraw(CDC * pDC, const CRect & printRect);
    void ResetPosition();																	// reset old cursor position
    DWORD CalculateCursorPosition(CView *, int nPosition, int nWidth, DWORD * pStopCursor); // calculate the current cursor position (in data samples)
    BOOL IsDragging();

protected:
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

private:
    CRect m_rWnd;				// cursor window coordinates
    BOOL m_bCursorDrag;         // cursor drag by mouse
    DWORD m_dwStartDragPos;     // cursor position when starting drag
    DWORD m_dwDragPos;          // current drag position
    int m_nEditBoundaries;		// editing boundaries
	bool m_bSelectDrag;			// true if the user pressed control before or while dragging
};

#endif

