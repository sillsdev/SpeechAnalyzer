#ifndef FLEXEDITGRID_H
#define FLEXEDITGRID_H

#include "MSHFlexGrid.h"

class CFlexEditWnd;

class CFlexEditGrid : public CMSHFlexGrid
{
public:
    CFlexEditGrid();
    virtual BOOL Create(LPCTSTR lpszClassName,
                        LPCTSTR lpszWindowName,
                        DWORD dwStyle,
                        const RECT & rect,
                        CWnd * pParentWnd,
                        UINT nID);

private:
    CFlexEditWnd * m_pEdit;
    long m_lBorderWidth;
    long m_lBorderHeight;

    int m_nLogX;
    int m_nLogY;

    long m_nUndoRow;
    long m_nUndoCol;
    CString m_cUndoString;
    BOOL m_bFakeArrowKeys;
    CFont m_cFont;

public:
    virtual void PreSubclassWindow();
    void ShowEditBox(CString const & boxContents, long selectionStart = -1, long selectionEnd = -1);
    BOOL Undo(CString * redoText);
    void InvalidData();
    virtual ~CFlexEditGrid();
    BOOL handleEditBoxSpecialKeys(short * KeyCode, short Shift, BOOL bHandleIt = TRUE);
    void FakeArrowKeys(BOOL bSetting)
    {
        m_bFakeArrowKeys = bSetting;
    }
    void SetFont(LPCTSTR face, float nPointSize, long startRow, long startCol = 0, long rows = 1,  long cols = 1);

    // Clipboard Utilities
    CString SaveRange(int top, int left, int bottom, int right, BOOL bRotate);
    void LoadRange(int top, int left, CString szData, BOOL bRotate);
    void ClearRange(int top, int left, int bottom, int right);

private:
    BOOL handleSpecialKeys(short * KeyCode, short Shift, BOOL bHandleIt = TRUE);

protected:
    afx_msg void OnSetFocus(CWnd * pOldWnd);
    DECLARE_EVENTSINK_MAP()

    // manually entered
    afx_msg BOOL OnUpdateGrid();
    afx_msg BOOL OnKeyPressGrid(short FAR * KeyAscii);
    afx_msg BOOL OnDblClickGrid();
    afx_msg BOOL OnKeyDownGrid(short FAR * KeyCode, short Shift);
public:
    afx_msg void OnEditCopy();
    afx_msg void OnUpdateEditCopy(CCmdUI * pCmdUI);
    afx_msg void OnEditClear();
    afx_msg void OnUpdateEditClear(CCmdUI * pCmdUI);
    afx_msg void OnEditCut();
    afx_msg void OnUpdateEditCut(CCmdUI * pCmdUI);
    afx_msg void OnEditPaste();
    afx_msg void OnUpdateEditPaste(CCmdUI * pCmdUI);
    DECLARE_MESSAGE_MAP()
};

#endif
