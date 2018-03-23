/////////////////////////////////////////////////////////////////////////////
// sa_minic.h:
// Interface of the CMiniCaptionWnd class.
// Author: Rod Early, changes: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_MINIC_H
#define _SA_MINIC_H

//###########################################################################
// CMiniCaptionWnd window

/////////////////////////////////////////////////////////////////////////////
// CMiniCaptionWnd defines

class CMiniCaptionWnd : public CWnd {
    DECLARE_DYNCREATE(CMiniCaptionWnd)

    // Construction/destruction/creation
public:
    CMiniCaptionWnd();
    virtual ~CMiniCaptionWnd();
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT & rect, CWnd * pParentWnd, UINT nID, CCreateContext * pContext = NULL);
    virtual BOOL PreCreateWindow(CREATESTRUCT & cs);

    // Attributes
protected:
    int m_iNCHeight;        // height of the mini-caption. not used for anything else.
    BOOL m_bFocus;          // TRUE if this window has focus
    int m_nCaption;         // graph caption style

    // Operations
public:
    virtual void SetGraphFocus(BOOL); // sets or resets the focus from this graph window and redraws the caption bar
    virtual void RedrawCaption();     // redraws the caption of this graph
    void OnNcDraw(CDC * pDC, const CRect & printRect);
private:
    // calculate the non-client areas (buttons, etc)
    void CalcNCAreas(BOOL isPrinting, const CRect & printRect, LPRECT prcNC, LPRECT prcBar, LPRECT prcSysMenu, LPRECT prcCaption,
                     LPRECT prcRightmost, LPRECT prcRightTwo, LPRECT prcRightOne);
	void Darken(COLORREF & c);
	void DrawButton(CDC * pDC, UINT nIDResource, CRect * prButton); // draw transparent caption button
	void DrawCaption(CDC * pDC, LONG top, LONG left, LONG height, LONG width);

    // Generated message map functions
protected:
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS * lpncsp);
    afx_msg void OnNcPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
    afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
    afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO *);
    afx_msg void OnInitMenu(CMenu *);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    DECLARE_MESSAGE_MAP()

    void DrawCaptionText(CDC * pDC, const CRect & rcWnd);
};

#endif
