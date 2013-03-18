#ifndef PRIVATECURSORWND_H
#define PRIVATECURSORWND_H

#include "SA_Cursr.h"

//###########################################################################
// CPrivateCursorWnd window

class CPrivateCursorWnd : public CCursorWnd {
    // Construction/destruction/creation
public:
    CPrivateCursorWnd();
    virtual ~CPrivateCursorWnd();
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
                        const RECT & rect, CWnd * pParentWnd, UINT nID, CCreateContext * pContext = NULL);
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);

    // Attributes
private:
    BOOL     m_bCursorDrag;         // cursor drag by mouse
    CRect    m_rWnd;                // cursor window coordinates

    // Operations
public:
    void ResetPosition();           // reset old cursor position
    virtual void OnDraw(CDC * pDC, const CRect & printRect);
    virtual void OnDrawHorizontalCursor(CDC * pDC, const CRect & printRect);
    void ChangeCursorPosition(CPoint point);
    void ChangeHorizontalCursorPosition(CPoint point);
    BOOL IsDragging();

    // Generated message map functions
protected:
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    DECLARE_MESSAGE_MAP()
};

#endif

