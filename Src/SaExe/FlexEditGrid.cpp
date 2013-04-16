#include "stdafx.h"
#include "flexEditGrid.h"

class CFlexEditWnd : public CEdit
{
    // Construction
public:
    CFlexEditWnd(CFlexEditGrid * const parent);

    // Attributes
public:
    CFlexEditGrid * const m_pGrid;
private:
    BOOL m_bVisible;

    // Operations
public:

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CFlexEditWnd)
    //}}AFX_VIRTUAL

    // Implementation
public:
    virtual BOOL IsVisible() const;
    virtual BOOL SetVisible(BOOL bState);
    virtual ~CFlexEditWnd();

    // Generated message map functions
protected:
    //{{AFX_MSG(CFlexEditWnd)
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKillfocus();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/*
1F3D5522-3F42-11d1-B2FA-00A0C908FB55
*/

WCHAR pwchLicenseKeyMSHFlxGd[] =
{
    0x0031, 0x0046, 0x0033, 0x0044, 0x0035, 0x0035,
    0x0032, 0x0032, 0x002D, 0x0033, 0x0046, 0x0034,
    0x0032, 0x002D, 0x0031, 0x0031, 0x0064, 0x0031,
    0x002D, 0x0042, 0x0032, 0x0046, 0x0041, 0x002D,
    0x0030, 0x0030, 0x0041, 0x0030, 0x0043, 0x0039,
    0x0030, 0x0038, 0x0046, 0x0042, 0x0035, 0x0035
};

/////////////////////////////////////////////////////////////////////////////
// CFlexEditGrid

CFlexEditGrid::CFlexEditGrid()
{
    m_pEdit = new CFlexEditWnd(this);
    m_nUndoRow = -1;
    m_nUndoCol = -1;
    m_bFakeArrowKeys = FALSE;
}

CFlexEditGrid::~CFlexEditGrid()
{
    if (m_pEdit)
    {
        delete m_pEdit;
    }
}


BOOL CFlexEditGrid::Create(LPCTSTR , LPCTSTR , DWORD dwStyle, const RECT & rect,
                           CWnd * pParentWnd, UINT nID)
{
    BSTR bstrLicense = ::SysAllocStringLen(pwchLicenseKeyMSHFlxGd, sizeof(pwchLicenseKeyMSHFlxGd)/sizeof(WCHAR));

    BOOL bResult = CMSHFlexGrid::Create(NULL, dwStyle, rect, pParentWnd, nID, NULL, FALSE, bstrLicense);

    ::SysFreeString(bstrLicense);

    return bResult;
}

BEGIN_MESSAGE_MAP(CFlexEditGrid, CMSHFlexGrid)
    ON_WM_SETFOCUS()
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
    ON_COMMAND(ID_EDIT_CUT, OnEditCut)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
END_MESSAGE_MAP()



BEGIN_EVENTSINK_MAP(CFlexEditGrid, CMSHFlexGrid)
// {{AFX_EVENTSINK_MAP(CFlexEditGrid)
ON_EVENT_REFLECT(CFlexEditGrid, -603 /* KeyPress */, OnKeyPressGrid, VTS_PI2)
ON_EVENT_REFLECT(CFlexEditGrid, -601 /* DblClick */, OnDblClickGrid, VTS_NONE)
ON_EVENT_REFLECT(CFlexEditGrid, 72 /* LeaveCell */, OnUpdateGrid, VTS_NONE)
ON_EVENT_REFLECT(CFlexEditGrid, -602 /* KeyDown */, OnKeyDownGrid, VTS_PI2 VTS_I2)
ON_EVENT_REFLECT(CFlexEditGrid, 73 /* Scroll */, OnUpdateGrid, VTS_NONE)
// }}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CFlexEditGrid::OnDblClickGrid()
{
    ShowEditBox(GetText(),0,-1);

    return TRUE;
}

BOOL CFlexEditGrid::OnKeyPressGrid(short FAR * KeyAscii)
{

    if (!handleSpecialKeys(KeyAscii, 0, FALSE))
    {
        CString buf;
        buf.Format(_T("%c"), *KeyAscii);
        ShowEditBox(buf,1,-1);
    }

    return TRUE;
}

BOOL CFlexEditGrid::OnUpdateGrid()
{
    // Check to see if edit is visible.
    BOOL bVisible = m_pEdit->IsVisible();
    if (bVisible)
    {
        m_cUndoString = GetText();
        m_nUndoRow = GetRow();
        m_nUndoCol = GetCol();
        CString cStr;
        m_pEdit->GetWindowText(cStr);
        SetText(cStr);
        m_pEdit->SetWindowText(_T(""));
        m_pEdit->SetVisible(FALSE);
        m_pEdit->SetFont(NULL);
        m_cFont.DeleteObject();
        SetFocus();
    }
    return FALSE; // allow this message to be fired in the container.
}

BOOL CFlexEditGrid::OnKeyDownGrid(short FAR * KeyCode, short Shift)
{
    return handleSpecialKeys(KeyCode, Shift);
}

BOOL CFlexEditGrid::handleEditBoxSpecialKeys(short FAR * KeyCode, short /*Shift*/, BOOL bHandleIt)
{
    if (!bHandleIt)
    {
        switch (*KeyCode)
        {
        case VK_UP:
        case VK_DOWN:
        case VK_LEFT:
        case VK_RIGHT:
        case VK_RETURN:
        case VK_ESCAPE:
        case VK_CANCEL:
            return TRUE;
        default:
            return FALSE;
        }
    }

    switch (*KeyCode)
    {
    case VK_UP:
        if (GetRow() > GetFixedRows())
        {
            OnUpdateGrid();
            SetRow(GetRow() - 1);
            return TRUE;
        }
        break;
    case VK_DOWN:
        if (GetRow() < (GetRows() - 1))
        {
            OnUpdateGrid();
            SetRow(GetRow() + 1);
            return TRUE;
        }
        break;
    case VK_LEFT:
        if (GetCol() > GetFixedCols())
        {
            OnUpdateGrid();
            SetCol(GetCol() - 1);
            return TRUE;
        }
        break;
    case VK_RIGHT:
        if (GetCol() < GetCols(0 /* Band */))
        {
            OnUpdateGrid();
            SetCol(GetCol() + 1);
            return TRUE;
        }
        break;
    case VK_RETURN:
        return OnUpdateGrid();
    case VK_ESCAPE:
    case VK_CANCEL:
    {
        // Check to see if edit is visible.
        BOOL bVisible = m_pEdit->IsVisible();
        if (bVisible)
        {
            m_pEdit->SetWindowText(_T(""));
            m_pEdit->SetVisible(FALSE);
            return TRUE;
        }
    }
    }
    return FALSE; // event not consumed here
}

BOOL CFlexEditGrid::handleSpecialKeys(short FAR * KeyCode, short /*Shift*/, BOOL bHandleIt)
{
    if (!bHandleIt)
    {
        switch (*KeyCode)
        {
        case VK_UP:
        case VK_DOWN:
        case VK_LEFT:
        case VK_RIGHT:
            return m_bFakeArrowKeys;
        case VK_RETURN:
        case VK_ESCAPE:
        case VK_CANCEL:
            return TRUE;
        default:
            return FALSE;
        }
    }

    switch (*KeyCode)
    {
    case VK_RETURN:
        return OnUpdateGrid();
    case VK_ESCAPE:
    case VK_CANCEL:
    {
        // Check to see if edit is visible.
        BOOL bVisible = m_pEdit->IsVisible();
        if (bVisible)
        {
            m_pEdit->SetWindowText(_T(""));
            m_pEdit->SetVisible(FALSE);
            return TRUE;
        }
    }
    }

    if (!m_bFakeArrowKeys)
    {
        return FALSE;
    }

    try
    {
        long row = GetRow();
        long col = GetCol();
        long rows = GetRows();
        long cols = GetCols(0);
        long fixedRows = GetFixedRows();
        long fixedCols = GetFixedCols();

        switch (*KeyCode)
        {
        case VK_UP:
            if (row > fixedRows)
            {
                OnUpdateGrid();
                SetRow(--row);
                while (!GetRowIsVisible(row))
                {
                    SetTopRow(GetTopRow() - 1);
                }
                return TRUE;
            }
            break;
        case VK_DOWN:
            if (row < (rows - 1))
            {
                OnUpdateGrid();
                SetRow(++row);

                long rowPartial = row + 1;
                if (rowPartial == rows)
                {
                    rowPartial--;
                }
                while (!GetRowIsVisible(rowPartial))
                {
                    SetTopRow(GetTopRow() + 1);
                }
                return TRUE;
            }
            break;
        case VK_LEFT:
            if (col > fixedCols)
            {
                OnUpdateGrid();
                SetCol(--col);
                while (!GetColIsVisible(col))
                {
                    SetLeftCol(GetLeftCol() - 1);
                }
                return TRUE;
            }
            break;
        case VK_RIGHT:
            if (col < (cols - 1))
            {
                OnUpdateGrid();
                SetCol(++col);

                long colPartial = col + 1;
                if (colPartial == cols)
                {
                    colPartial--;
                }
                while (!GetColIsVisible(colPartial))
                {
                    SetLeftCol(GetLeftCol() + 1);
                }
                return TRUE;
            }
            break;
        }
    }
    catch (COleException)
    {
        return TRUE;
    }
    return FALSE; // event not consumed here
}

void CFlexEditGrid::PreSubclassWindow()
{
    // Calculate border size.
    long row = GetRow();
    long col = GetCol();
    SetRow(0);
    SetCol(0);
    m_lBorderWidth = GetCellLeft();
    m_lBorderHeight = GetCellTop();
    SetRow(row);
    SetCol(col);

    // To convert grid rect from twips to DC units you need
    // pixels per inch.
    CDC * pDC = GetDC();
    m_nLogX = pDC->GetDeviceCaps(LOGPIXELSX);
    m_nLogY = pDC->GetDeviceCaps(LOGPIXELSY);
    ReleaseDC(pDC);

    // Create invisible edit control.
    m_pEdit->Create(//WS_CHILD|
        ES_NOHIDESEL|
        ES_WANTRETURN|
        ES_MULTILINE|
        ES_AUTOHSCROLL,
        CRect(0,0,0,0), this, GetDlgCtrlID());
}


void CFlexEditGrid::InvalidData()
{
    CString redo;
    Undo(&redo);

    ShowEditBox(redo);
}

BOOL CFlexEditGrid::Undo(CString * redoText)
{
    if (m_nUndoRow != -1)
    {
        SetRow(m_nUndoRow);
        SetCol(m_nUndoCol);
        if (redoText)
        {
            *redoText = GetText();
        }
        SetText(m_cUndoString);
        return TRUE;
    }
    return FALSE;
}

void CFlexEditGrid::ShowEditBox(const CString & boxContents, long selectionStart, long selectionEnd)
{

    m_pEdit->SetWindowText(boxContents);
    m_cFont.CreatePointFont(int(GetCellFontSize()*10 +0.5), GetCellFontName());
    m_pEdit->SetFont(&m_cFont);
    m_pEdit->SetSel(selectionStart, selectionEnd);

    // Adjust for border height and convert from twips to screen
    // units.
    m_pEdit->MoveWindow(
        ((GetCellLeft() - m_lBorderWidth) * m_nLogX + 720)/1440,
        ((GetCellTop() - m_lBorderHeight)  * m_nLogY + 720)/1440,
        (GetCellWidth() * m_nLogX)/1440,
        (GetCellHeight() * m_nLogY)/1440, FALSE);

    m_pEdit->SetVisible(TRUE);
    m_pEdit->SetFocus();
}

void CFlexEditGrid::SetFont(LPCTSTR face, float pointSize, long startRow, long startCol, long rows,  long cols)
{
    SetRedraw(FALSE);
    long initRow = GetRow();
    long initCol = GetCol();

    long lastCol = (cols > 0) ? startCol + cols : GetCols(0);
    long lastRow = (rows > 0) ? startRow + rows : GetRows();

    for (long row = startRow; row < lastRow; row++)
    {
        for (long col = startCol; col < lastCol; col++)
        {
            SetRow(row);
            SetCol(col);
            SetCellFontName(face);
            SetCellFontSize(pointSize);
        }
    }

    SetRow(initRow);
    SetCol(initCol);
    SetRedraw(TRUE);
    Refresh();
}

/////////////////////////////////////////////////////////////////////////////
// CFlexEditGrid message handlers
void CFlexEditGrid::OnSetFocus(CWnd * pOldWnd)
{
    CMSHFlexGrid::OnSetFocus(pOldWnd);

    OnUpdateGrid();
}

/////////////////////////////////////////////////////////////////////////////
// CFlexEditWnd

CFlexEditWnd::CFlexEditWnd(CFlexEditGrid * const parent) : m_pGrid(parent)
{
    m_bVisible = FALSE;
}

CFlexEditWnd::~CFlexEditWnd()
{
}


BEGIN_MESSAGE_MAP(CFlexEditWnd, CEdit)
    //{{AFX_MSG_MAP(CFlexEditWnd)
    ON_WM_CHAR()
    ON_WM_KEYDOWN()
    ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillfocus)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFlexEditWnd message handlers

void CFlexEditWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar != VK_RETURN)   // Ignore ENTER key.  we only really want a single line edit control
    {
        CEdit::OnChar(nChar, nRepCnt, nFlags);
    }
}

void CFlexEditWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    short KeyCode = short(nChar);
    BOOL result = FALSE;
    int selStart, selEnd;

    GetSel(selStart,selEnd);
    if (((KeyCode != VK_LEFT)  || selEnd <= 0) &&
            ((KeyCode != VK_RIGHT) || selStart >= (GetWindowTextLength() - 1)))
    {
        result = m_pGrid->handleEditBoxSpecialKeys(&KeyCode, short(nFlags));
    }

    if (!result)
    {
        CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
    }
}


BOOL CFlexEditWnd::SetVisible(BOOL bState)
{
    BOOL bOldState = m_bVisible;

    m_bVisible = bState;

    if (bState)
    {
        ShowWindow(SW_SHOW);
    }
    else
    {
        ShowWindow(SW_HIDE);
    }

    return bOldState;
}


BOOL CFlexEditWnd::IsVisible() const
{
    return m_bVisible;
}

void CFlexEditWnd::OnKillfocus()
{
    short KeyCode = VK_RETURN;
    m_pGrid->handleEditBoxSpecialKeys(&KeyCode, 0);
}


void CFlexEditGrid::OnEditCopy()
{
    long row = GetRow();
    long rowSel = GetRowSel();
    long top = (row < rowSel) ? row : rowSel;
    long bottom = (row > rowSel) ? row : rowSel;

    long col = GetCol();
    long colSel = GetColSel();
    long left = (col < colSel) ? col : colSel;
    long right = (col > colSel) ? col : colSel;

    CString szCopy = SaveRange(top, left, bottom + 1, right + 1, TRUE);

    HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, szCopy.GetLength() + 1);
    ASSERT(hData);
    if (hData)
    {
        LPTSTR lpData = (TCHAR *)GlobalLock(hData);
        ASSERT(lpData);
        if (lpData)
        {
            int len = szCopy.GetLength();
            TCHAR * str = szCopy.GetBuffer(len+1);
            ASSERT(str);
            memcpy(lpData, str, len);
            lpData[len] = 0;
            szCopy.ReleaseBuffer();
            GlobalUnlock(hData);

            // Clear the current contents of the clipboard, and set
            // the data handle to the new string.
            if (OpenClipboard())
            {
                EmptyClipboard();
                SetClipboardData(CF_TEXT, hData);
                CloseClipboard();
            }
        }
    }
}

void CFlexEditGrid::OnUpdateEditCopy(CCmdUI *)
{
}

void CFlexEditGrid::OnEditClear()
{
    long row = GetRow();
    long rowSel = GetRowSel();
    long top = (row < rowSel) ? row : rowSel;
    long bottom = (row > rowSel) ? row : rowSel;

    long col = GetCol();
    long colSel = GetColSel();
    long left = (col < colSel) ? col : colSel;
    long right = (col > colSel) ? col : colSel;

    ClearRange(top, left, bottom, right);
}

void CFlexEditGrid::OnUpdateEditClear(CCmdUI *)
{
}

void CFlexEditGrid::OnEditCut()
{
    OnEditCopy();
    OnEditClear();
}

void CFlexEditGrid::OnUpdateEditCut(CCmdUI *)
{
}

void CFlexEditGrid::OnEditPaste()
{
    if (OpenClipboard())
    {
        HGLOBAL hClipData = NULL;
        char * lpClipData = NULL;
        // get text from the clipboard
        if (NULL!=(hClipData = GetClipboardData(CF_TEXT)))
        {
            if (NULL!=(lpClipData = (char *)GlobalLock(hClipData)))
            {
                CString data(lpClipData);
                LoadRange(GetRow(),GetCol(),data, TRUE);
                GlobalUnlock(hClipData);
            }
        }

        CloseClipboard();
    }
}

void CFlexEditGrid::OnUpdateEditPaste(CCmdUI * pCmdUI)
{
    BOOL enable = FALSE;
    if (OpenClipboard())
    {
        if (IsClipboardFormatAvailable(CF_TEXT))
		{
			enable = TRUE;
		}

        CloseClipboard();
    }
    pCmdUI->Enable(enable);
}

// Converts a range of grid cells to a string
// separates columns by "\t" (tab)
// separates rows by "\r\n" (crlf)
// allows option of rotating grid contents in strings
// bottom, right refer to first cell location NOT included in string (+1)
CString CFlexEditGrid::SaveRange(int top, int left, int bottom, int right, BOOL bRotate)
{
    CString szGrid;
    CString szEmptyCols;

    if (top > GetRows())
    {
        top = GetRows();
    }
    if (bottom > GetRows())
    {
        bottom = GetRows();
    }

    int xMin,xMax;
    int yMin,yMax;

    if (bRotate)
    {
        xMin = left;
        xMax = right;
        yMin = top;
        yMax = bottom;
    }
    else
    {
        yMin = left;
        yMax = right;
        xMin = top;
        xMax = bottom;
    }

    for (int x = xMin; x < xMax; x++)
    {
        CString szLine;
        CString szField;
        CString szEmptyFields;

        szLine.Empty();
        szEmptyFields.Empty();

        for (int y = yMin; y < yMax; y++)
        {
            if (bRotate)
            {
                szField = GetTextMatrix(y,x);
            }
            else
            {
                szField = GetTextMatrix(x,y);
            }
            if (!szField.IsEmpty())
            {
                szLine += szEmptyFields;
                szLine += szField;
                szEmptyFields.Empty();
            }

            szEmptyFields += _T("\t");
        }

        if (!szLine.IsEmpty())
        {
            szGrid += szEmptyCols;
            szGrid += szLine;
            szEmptyCols.Empty();
        }
        szEmptyCols += _T("\r\n");
    }

    return szGrid;
}


// Converts a string to a range of grid cells
// separates columns by "\t" (tab)
// separates rows by "\r\n" (crlf)
// allows option of rotating string contents into grid
void CFlexEditGrid::LoadRange(int top, int left, CString szGrid, BOOL bRotate)
{
    int nLineStart = 0;
    int nLength = szGrid.GetLength();

    int xMin;
    int yMin;

    if (bRotate)
    {
        xMin = left;
        yMin = top;
    }
    else
    {
        yMin = left;
        xMin = top;
    }

    for (int x = xMin; nLineStart < nLength; x++)
    {
        int nLineEnd = szGrid.Find(_T("\n"),nLineStart);

        if (nLineEnd == -1)
        {
            nLineEnd = nLength;
        }

        CString szLine = szGrid.Mid(nLineStart,nLineEnd-nLineStart);

        int nLineLength = szLine.GetLength();

        if (szLine[nLineLength-1] == '\r')
        {
            // usually lines should be terminated by CrLf ("\r\n")
            szLine = szLine.Left(nLineLength - 1);
            nLineLength--;
        }

        if (!bRotate && x >= GetRows())
        {
            SetRows(x + 1);
        }
        if (bRotate && x >= GetCols(0))
        {
            SetCols(0, x + 1);
        }

        int nFieldStart = 0;
        for (int y = yMin; nFieldStart < nLineLength; y++)
        {
            int nFieldEnd = szLine.Find(_T("\t"), nFieldStart);

            if (nFieldEnd == -1)
                // clipboard may not have a trailing \t
            {
                nFieldEnd = nLineLength;
            }

            CString szField = szLine.Mid(nFieldStart,nFieldEnd-nFieldStart);

            if (!bRotate && y >= GetCols(0))
            {
                SetCols(0, y + 1);
            }
            if (bRotate && y >= GetRows())
            {
                SetRows(y + 1);
            }

            if (bRotate)
            {
                SetTextMatrix(y,x,szField);
            }
            else
            {
                SetTextMatrix(x,y,szField);
            }

            nFieldStart = nFieldEnd+1;
        }
        nLineStart = nLineEnd + 1;
    }
}

void CFlexEditGrid::ClearRange(int top, int left, int bottom, int right)
{
    for (int x = left; x <= right; x++)
    {
        for (int y = top; y <= bottom; y++)
        {
            SetTextMatrix(y,x,_T(""));
        }
    }
}

