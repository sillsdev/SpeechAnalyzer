#ifndef PRIVATECURSORWND_H
#define PRIVATECURSORWND_H

#include "SA_Cursr.h"

//###########################################################################
// CPrivateCursorWnd window

class CPrivateCursorWnd : public CCursorWnd
{
public:
    CPrivateCursorWnd();
    virtual ~CPrivateCursorWnd();
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
                        const RECT & rect, CWnd * pParentWnd, UINT nID, CCreateContext * pContext = NULL);
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);
    void ResetPosition();           // reset old cursor position
    virtual void OnDraw(CDC * pDC, const CRect & printRect);
    virtual void OnDrawHorizontalCursor(CDC * pDC, const CRect & printRect);
    void ChangeCursorPosition(CPoint point);
    void ChangeHorizontalCursorPosition(CPoint point);
    bool IsDragging();
	bool IsCreated();
	void Flash(bool on);

protected:
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);

    DECLARE_MESSAGE_MAP()

private:
    bool m_bCursorDrag;				// cursor drag by mouse
    CRect m_rWnd;					// cursor window coordinates
	bool created;
	bool hidden;
};

#endif

