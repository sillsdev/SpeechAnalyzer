#ifndef MSHFLEXGRID_H
#define MSHFLEXGRID_H

class COleFont;
class CPicture;
class CRecordset1;

class CMSHFlexGrid : public CWnd
{
protected:
    DECLARE_DYNCREATE(CMSHFlexGrid)
public:
    CLSID const & GetClsid()
    {
        static CLSID const clsid
            = { 0xecd9b64, 0x23aa, 0x11d0, { 0xb3, 0x51, 0x0, 0xa0, 0xc9, 0x5, 0x5d, 0x8e } };
        return clsid;
    }
    virtual BOOL Create(LPCTSTR /*lpszClassName*/,
                        LPCTSTR lpszWindowName, DWORD dwStyle,
                        const RECT & rect,
                        CWnd * pParentWnd, UINT nID,
                        CCreateContext * /*pContext*/ = NULL)
    {
        return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID);
    }

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle,
                const RECT & rect, CWnd * pParentWnd, UINT nID,
                CFile * pPersist = NULL, BOOL bStorage = FALSE,
                BSTR bstrLicKey = NULL)
    {
        return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
                             pPersist, bStorage, bstrLicKey);
    }

public:
    long GetRows();
    void SetRows(long nNewValue);
    long GetCols(long BandNumber);
    void SetCols(long BandNumber, long nNewValue);
    long GetFixedRows();
    void SetFixedRows(long nNewValue);
    long GetFixedCols();
    void SetFixedCols(long nNewValue);
    short GetVersion();
    CString GetFormatString();
    void SetFormatString(LPCTSTR lpszNewValue);
    long GetTopRow();
    void SetTopRow(long nNewValue);
    long GetLeftCol();
    void SetLeftCol(long nNewValue);
    long GetRow();
    void SetRow(long nNewValue);
    long GetCol();
    void SetCol(long nNewValue);
    long GetRowSel();
    void SetRowSel(long nNewValue);
    long GetColSel();
    void SetColSel(long nNewValue);
    CString GetText();
    void SetText(LPCTSTR lpszNewValue);
    unsigned long GetBackColor();
    void SetBackColor(unsigned long newValue);
    unsigned long GetForeColor();
    void SetForeColor(unsigned long newValue);
    unsigned long GetBackColorBand(long BandNumber);
    void SetBackColorBand(long BandNumber, unsigned long newValue);
    unsigned long GetForeColorBand(long BandNumber);
    void SetForeColorBand(long BandNumber, unsigned long newValue);
    unsigned long GetBackColorHeader(long BandNumber);
    void SetBackColorHeader(long BandNumber, unsigned long newValue);
    unsigned long GetForeColorHeader(long BandNumber);
    void SetForeColorHeader(long BandNumber, unsigned long newValue);
    unsigned long GetBackColorIndent(long BandNumber);
    void SetBackColorIndent(long BandNumber, unsigned long newValue);
    unsigned long GetBackColorFixed();
    void SetBackColorFixed(unsigned long newValue);
    unsigned long GetForeColorFixed();
    void SetForeColorFixed(unsigned long newValue);
    unsigned long GetBackColorSel();
    void SetBackColorSel(unsigned long newValue);
    unsigned long GetForeColorSel();
    void SetForeColorSel(unsigned long newValue);
    unsigned long GetBackColorBkg();
    void SetBackColorBkg(unsigned long newValue);
    unsigned long GetBackColorUnpopulated();
    void SetBackColorUnpopulated(unsigned long newValue);
    BOOL GetWordWrap();
    void SetWordWrap(BOOL bNewValue);
    COleFont GetFont();
    void SetRefFont(LPDISPATCH newValue);
    float GetFontWidth();
    void SetFontWidth(float newValue);
    COleFont GetFontFixed();
    void SetRefFontFixed(LPDISPATCH newValue);
    float GetFontWidthFixed();
    void SetFontWidthFixed(float newValue);
    COleFont GetFontBand(long BandNumber);
    void SetRefFontBand(long BandNumber, LPDISPATCH newValue);
    float GetFontWidthBand(long BandNumber);
    void SetFontWidthBand(long BandNumber, float newValue);
    COleFont GetFontHeader(long BandNumber);
    void SetRefFontHeader(long BandNumber, LPDISPATCH newValue);
    float GetFontWidthHeader(long BandNumber);
    void SetFontWidthHeader(long BandNumber, float newValue);
    CString GetCellFontName();
    void SetCellFontName(LPCTSTR lpszNewValue);
    float GetCellFontSize();
    void SetCellFontSize(float newValue);
    BOOL GetCellFontBold();
    void SetCellFontBold(BOOL bNewValue);
    BOOL GetCellFontItalic();
    void SetCellFontItalic(BOOL bNewValue);
    BOOL GetCellFontUnderline();
    void SetCellFontUnderline(BOOL bNewValue);
    BOOL GetCellFontStrikeThrough();
    void SetCellFontStrikeThrough(BOOL bNewValue);
    float GetCellFontWidth();
    void SetCellFontWidth(float newValue);
    long GetTextStyle();
    void SetTextStyle(long nNewValue);
    long GetTextStyleFixed();
    void SetTextStyleFixed(long nNewValue);
    long GetTextStyleBand(long BandNumber);
    void SetTextStyleBand(long BandNumber, long nNewValue);
    long GetTextStyleHeader(long BandNumber);
    void SetTextStyleHeader(long BandNumber, long nNewValue);
    BOOL GetScrollTrack();
    void SetScrollTrack(BOOL bNewValue);
    long GetFocusRect();
    void SetFocusRect(long nNewValue);
    long GetHighLight();
    void SetHighLight(long nNewValue);
    BOOL GetRedraw();
    void SetRedraw(BOOL bNewValue);
    long GetScrollBars();
    void SetScrollBars(long nNewValue);
    long GetMouseRow();
    long GetMouseCol();
    long GetCellLeft();
    long GetCellTop();
    long GetCellWidth();
    long GetCellHeight();
    long GetRowHeightMin();
    void SetRowHeightMin(long nNewValue);
    long GetFillStyle();
    void SetFillStyle(long nNewValue);
    long GetGridLines();
    void SetGridLines(long nNewValue);
    long GetGridLinesFixed();
    void SetGridLinesFixed(long nNewValue);
    unsigned long GetGridColor();
    void SetGridColor(unsigned long newValue);
    unsigned long GetGridColorFixed();
    void SetGridColorFixed(unsigned long newValue);
    unsigned long GetGridColorUnpopulated();
    void SetGridColorUnpopulated(unsigned long newValue);
    unsigned long GetGridColorBand(long BandNumber);
    void SetGridColorBand(long BandNumber, unsigned long newValue);
    unsigned long GetGridColorHeader(long BandNumber);
    void SetGridColorHeader(long BandNumber, unsigned long newValue);
    unsigned long GetGridColorIndent(long BandNumber);
    void SetGridColorIndent(long BandNumber, unsigned long newValue);
    unsigned long GetCellBackColor();
    void SetCellBackColor(unsigned long newValue);
    unsigned long GetCellForeColor();
    void SetCellForeColor(unsigned long newValue);
    short GetCellAlignment();
    void SetCellAlignment(short nNewValue);
    long GetCellTextStyle();
    void SetCellTextStyle(long nNewValue);
    short GetCellPictureAlignment();
    void SetCellPictureAlignment(short nNewValue);
    CString GetClip();
    void SetClip(LPCTSTR lpszNewValue);
    void SetSort(short nNewValue);
    long GetSelectionMode();
    void SetSelectionMode(long nNewValue);
    long GetMergeCells();
    void SetMergeCells(long nNewValue);
    BOOL GetAllowBigSelection();
    void SetAllowBigSelection(BOOL bNewValue);
    long GetAllowUserResizing();
    void SetAllowUserResizing(long nNewValue);
    long GetBorderStyle();
    void SetBorderStyle(long nNewValue);
    long GetHWnd();
    BOOL GetEnabled();
    void SetEnabled(BOOL bNewValue);
    long GetAppearance();
    void SetAppearance(long nNewValue);
    long GetMousePointer();
    void SetMousePointer(long nNewValue);
    CPicture GetMouseIcon();
    void SetRefMouseIcon(LPDISPATCH newValue);
    long GetPictureType();
    void SetPictureType(long nNewValue);
    CPicture GetPicture();
    CPicture GetCellPicture();
    void SetRefCellPicture(LPDISPATCH newValue);
    CString GetTextArray(long Index);
    void SetTextArray(long Index, LPCTSTR lpszNewValue);
    short GetColAlignment(long Index);
    void SetColAlignment(long Index, short nNewValue);
    short GetColAlignmentFixed(long Index);
    void SetColAlignmentFixed(long Index, short nNewValue);
    short GetColAlignmentBand(long BandNumber, long BandColIndex);
    void SetColAlignmentBand(long BandNumber, long BandColIndex, short nNewValue);
    short GetColAlignmentHeader(long BandNumber, long BandColIndex);
    void SetColAlignmentHeader(long BandNumber, long BandColIndex, short nNewValue);
    long GetColWidth(long Index, long BandNumber);
    void SetColWidth(long Index, long BandNumber, long nNewValue);
    long GetRowHeight(long Index);
    void SetRowHeight(long Index, long nNewValue);
    BOOL GetMergeRow(long Index);
    void SetMergeRow(long Index, BOOL bNewValue);
    BOOL GetMergeCol(long Index);
    void SetMergeCol(long Index, BOOL bNewValue);
    void SetRowPosition(long Index, long nNewValue);
    void SetColPosition(long Index, long BandNumber, long nNewValue);
    long GetRowData(long Index);
    void SetRowData(long Index, long nNewValue);
    long GetColData(long Index);
    void SetColData(long Index, long nNewValue);
    CString GetTextMatrix(long Row, long Col);
    void SetTextMatrix(long Row, long Col, LPCTSTR lpszNewValue);
    void AddItem(LPCTSTR Item, const VARIANT & Index);
    void RemoveItem(long Index);
    void Clear();
    void Refresh();
    void ClearStructure();
    LPUNKNOWN GetDataSource();
    void SetRefDataSource(LPUNKNOWN newValue);
    CString GetDataMember();
    void SetDataMember(LPCTSTR lpszNewValue);
    BOOL GetRowIsVisible(long Index);
    BOOL GetColIsVisible(long Index);
    long GetRowPos(long Index);
    long GetColPos(long Index);
    short GetGridLineWidth();
    void SetGridLineWidth(short nNewValue);
    short GetGridLineWidthFixed();
    void SetGridLineWidthFixed(short nNewValue);
    short GetGridLineWidthUnpopulated();
    void SetGridLineWidthUnpopulated(short nNewValue);
    short GetGridLineWidthBand(long BandNumber);
    void SetGridLineWidthBand(long BandNumber, short nNewValue);
    short GetGridLineWidthHeader(long BandNumber);
    void SetGridLineWidthHeader(long BandNumber, short nNewValue);
    short GetGridLineWidthIndent(long BandNumber);
    void SetGridLineWidthIndent(long BandNumber, short nNewValue);
    BOOL GetRightToLeft();
    void SetRightToLeft(BOOL bNewValue);
    CRecordset1 GetRecordset();
    void SetRefRecordset(LPDISPATCH newValue);
    VARIANT GetRowset();
    void SetRefRowset(const VARIANT & newValue);
    long GetColHeader(long BandNumber);
    void SetColHeader(long BandNumber, long nNewValue);
    CString GetColHeaderCaption(long BandNumber, long BandColIndex);
    void SetColHeaderCaption(long BandNumber, long BandColIndex, LPCTSTR lpszNewValue);
    long BandColIndex();
    long GetBandData(long BandData);
    void SetBandData(long BandData, long nNewValue);
    long GetBandDisplay();
    void SetBandDisplay(long nNewValue);
    BOOL GetBandExpandable(long BandNumber);
    void SetBandExpandable(long BandNumber, BOOL bNewValue);
    long GetGridLinesBand(long BandNumber);
    void SetGridLinesBand(long BandNumber, long nNewValue);
    long GetGridLinesHeader(long BandNumber);
    void SetGridLinesHeader(long BandNumber, long nNewValue);
    long GetGridLinesIndent(long BandNumber);
    void SetGridLinesIndent(long BandNumber, long nNewValue);
    long GetGridLinesUnpopulated();
    void SetGridLinesUnpopulated(long nNewValue);
    long GetBandIndent(long BandNumber);
    void SetBandIndent(long BandNumber, long nNewValue);
    long GetBandLevel();
    long GetBands();
    long GetCellType();
    long GetRowSizingMode();
    void SetRowSizingMode(long nNewValue);
    CString GetDataField(long BandNumber, long BandColIndex);
    BOOL GetRowExpandable();
    BOOL GetRowExpanded();
    void SetRowExpanded(BOOL bNewValue);
    void CollapseAll(long BandNumber);
    void ExpandAll(long BandNumber);
    short GetColWordWrapOption(long Index);
    void SetColWordWrapOption(long Index, short nNewValue);
    short GetColWordWrapOptionBand(long BandNumber, long BandColIndex);
    void SetColWordWrapOptionBand(long BandNumber, long BandColIndex, short nNewValue);
    short GetColWordWrapOptionHeader(long BandNumber, long BandColIndex);
    void SetColWordWrapOptionHeader(long BandNumber, long BandColIndex, short nNewValue);
    short GetColWordWrapOptionFixed(long Index);
    void SetColWordWrapOptionFixed(long Index, short nNewValue);
    long GetOLEDropMode();
    void SetOLEDropMode(long nNewValue);
    void OLEDrag();
};

#endif
