/////////////////////////////////////////////////////////////////////////////
// sa_dlwnd.h:
// Interface of the CLEDDisplay (dialog child window)
//                  CSliderVertical (dialog child window)
//                  CSliderHorizontal (dialog child window)
//                  CVUBar (dialog child window)
//                  CProgressBar (window)
//                  CComboGridlines (combobox with gridlines)
//                  CSpinControl (micro scroll buttons)
//                  CToggleButton (toggle button)
//                  CTranscriptionDisp (dialog child window)
//                  CStaticText (dialog child window)
//                  CFancyArrow (dialog child window)
//                  CAboutTitle (dialog child window)         classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_DLWND_H
#define _SA_DLWND_H

//###########################################################################
// CLEDDisplay dialog child window

/////////////////////////////////////////////////////////////////////////
// CLEDDisplay defines

#define LED_DIGIT_WIDTH     8  // width of a digit
#define LED_DIGIT_HEIGHT    13 // height of a digit
#define LED_POINT_WIDTH     4  // width of a point or double point
#define LED_DIGIT_SPACE     2  // space between digits

class CLEDDisplay : public CWnd
{
    DECLARE_DYNCREATE(CLEDDisplay)

// Construction/destruction/creation
public:
    CLEDDisplay();
    ~CLEDDisplay();

    // Attributes
private:
    CBitmap * m_pBmLED; // LED bitmap
    char m_chMinMSD, m_chMinLSD;             // minutes display memory
    char m_chSecMSD, m_chSecLSD, m_chSecDec; // seconds display memory

    // Operations
public:
    void Init(UINT nID, CWnd * pParent);
    void SetTime(int nMin, int nSec); // min > 99 displays "--", sec > 599 displays "--.-"
    int GetMinutes()
    {
        return (m_chMinMSD * 10 + m_chMinLSD);   // return actually displayed minutes
    }
    int GetSeconds()
    {
        return (m_chSecMSD * 100 + m_chSecLSD * 10 + m_chSecDec);   // return actually displayed seconds
    }

    // Generated message map functions
protected:
    //{{AFX_MSG(CLEDDisplay)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CSliderVertical dialog child window

class CSliderVertical : public CWnd
{
    DECLARE_DYNCREATE(CSliderVertical)

// Construction/destruction/creation
public:
    CSliderVertical();
    ~CSliderVertical();

    // Attributes
private:
    CBitmap * m_pBmKnob;       // knob bitmap
    CRect  m_rPos;             // bitmap position and size
    char   m_chPos;            // position display memory
    BOOL   m_bKnobDrag;        // knob drag by mouse
    int    m_nID;              // ID

    // Operations
private:
    int GetPosFromMouse(CPoint point);
public:
    void Init(UINT nID, CWnd * pParent);
    void SetPosition(int nVal);
    int GetPosition()
    {
        return m_chPos;   // get actual slider position
    }

    // Generated message map functions
protected:
    //{{AFX_MSG(CSliderVertical)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CSliderHorizontal dialog child window

class CSliderHorizontal : public CWnd
{
    DECLARE_DYNCREATE(CSliderHorizontal)

// Construction/destruction/creation
public:
    CSliderHorizontal();
    ~CSliderHorizontal();

    // Attributes
private:
    CBitmap * m_pBmKnob;        // knob bitmap
    CRect   m_rPos;             // bitmap position and size
    char    m_chPos;            // position display memory
    BOOL    m_bKnobDrag;        // knob drag by mouse
    int     m_nID;              // ID

    // Operations
private:
    int GetPosFromMouse(CPoint point);
public:
    void Init(UINT nID, CWnd * pParent);
    void SetPosition(int nVal);
    int GetPosition()
    {
        return m_chPos;   // get actual slider position
    }

    // Generated message map functions
protected:
    //{{AFX_MSG(CSliderHorizontal)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CVUBar dialog child window

/////////////////////////////////////////////////////////////////////////////
// CVUBar defines

#define VU_BAR_WIDTH        10 // width of the VU bar
#define VU_CHANNEL_DISTANCE  4 // distance between channels
#define VU_BAR_DIVISIONS    20 // number of divisions
#define VU_BAR_GREEN        10 // first green point
#define VU_BAR_YELLOW       14 // first yellow point
#define VU_BAR_RED          17 // first red point
#define VU_BAR_HOLDCOUNT    10 // peak hold time

class CVUBar : public CWnd
{
    DECLARE_DYNCREATE(CVUBar)

// Construction/destruction/creation
public:
    CVUBar();
    ~CVUBar();

    // Attributes
private:
    CFont m_font;           // VU display character font
    int  m_nTextSpace;      // reserved space for text
    char m_chLVU;           // VU display left channel value
    char m_chLOldVU;        // VU display previous left channel value
    char m_chRVU;           // VU display left channel value
    char m_chROldVU;        // VU display previous left channel value
    TCHAR m_chLText;         // left channel text
    TCHAR m_chRText;         // right channel text
    char m_chLPeak;         // left channel peak value
    char m_chRPeak;         // right channel peak value
    int  m_nLPeakHoldCount; // left channel peak holding counter
    int  m_nRPeakHoldCount; // right channel peak holding counter
    int  m_nPeakHold;       // peak holding time (count)
    BOOL m_bPeak;           // TRUE, if peak holding enabled

    // Operations
public:
    void Init(UINT nID, CWnd * pParent);
    void InitVU(char chLText, char chRText, BOOL bPeak, int nPeakHold);
    void SetVU(int nLVal, int nRVal = -1); // set the level (left only for mono display)
    void Reset(); // resets the peak settings

    // Generated message map functions
protected:
    //{{AFX_MSG(CVUBar)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CProgressBar window

/////////////////////////////////////////////////////////////////////////////
// CProgressBar defines

#define PROGRESS_BAR_DIVISIONS  10 // number of divisions

class CProgressBar : public CWnd
{
    DECLARE_DYNCREATE(CProgressBar)

// Construction/destruction/creation
public:
    CProgressBar();
    virtual ~CProgressBar();

    // Attributes
private:
    char m_chProgress;         // progress display memory
    int  m_nProgress;

    // Operations
public:
    void Init(UINT nID, CWnd * pParent);
    void SetProgress(int nVal);
    int  GetProgress()
    {
        return m_nProgress;
    };

    // Generated message map functions
protected:
    //{{AFX_MSG(CProgressBar)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CComboGridlines combobox with gridlines

/////////////////////////////////////////////////////////////////////////
// CComboGridlines defines

#define COLOR_ITEM_HEIGHT   16 // height of each item in list

class CComboGridlines : public CComboBox
{
    // Construction/destruction/creation
public:

    // Attributes
private:

    // Operations
private:
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
public:
    void AddLineItem(int nPenStyle);
};

//###########################################################################
// CSpinControl micro scroll buttons

class CSpinControl : public CWnd
{
    DECLARE_DYNCREATE(CSpinControl)

public:
    CSpinControl();
    ~CSpinControl();

private:
    BOOL m_bUpButtonDown;  // button status
    BOOL m_bLowButtonDown;
    int  m_nID;            // ID
    BOOL m_bTimerStart;    // TRUE indicates a new mouse repeat timer start

private:
    void DrawArrowUp(CDC * pDC, CRect rect);
    void DrawArrowDown(CDC * pDC, CRect rect);
    void DrawButtonUp(CDC * pDC, CRect rect);
    void DrawButtonDown(CDC * pDC, CRect rect);
    void MouseClick(UINT nFlags, CPoint point);
public:
    void Init(UINT nID, CWnd * pParent);
    int GetID()
    {
        return m_nID;   // get spin control ID
    }
    BOOL UpperButtonClicked()
    {
        return m_bUpButtonDown;   // return TRUE if upper button hit
    }

    // Generated message map functions
protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnEnable(BOOL bEnable);
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CToggleButton

class CToggleButton : public CWnd
{
    DECLARE_DYNCREATE(CToggleButton)

public:
    CToggleButton();
    ~CToggleButton();

private:
    CRect m_rSymbol;       // symbol rectangle
    int   m_nID;           // ID
    BOOL  m_bButtonDown;   // button status
    BOOL  m_bSymbolFlash;  // button flash status
    BOOL  m_bSymbolOff;    // button symbol draw status
    CString m_szText;      // button text

private:
    void DrawButtonUp(CDC * pDC, CRect rWnd);
    void DrawButtonDown(CDC * pDC, CRect rWnd);
    void DrawText(CDC * pDC, CRect rWnd);
    void DrawSymbol(CDC * pDC);

public:
    void Init(UINT nID, CString szText, CWnd * pParent);
    int GetID()
    {
        return m_nID;   // get control ID
    }
    void Flash(BOOL bFlash);           // starts/stops symbol flashing
    void Push();                       // push released button
    void Release();                    // release pressed button

    // Generated message map functions
protected:
    //{{AFX_MSG(CToggleButton)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnEnable(BOOL bEnable);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CTranscriptionDisp dialog child window

class CTranscriptionDisp : public CWnd
{
    // SDM 1.06.6r1 uses underlying static text control
    // Operations
public:
    void Init(UINT nID, CWnd * pParent);

};

//###########################################################################
// CAboutTitle dialog child window

class CAboutTitle : public CWnd
{
    DECLARE_DYNCREATE(CAboutTitle)

// Construction/destruction/creation
public:
    CAboutTitle();
    ~CAboutTitle();

    // Attributes
private:

    // Operations
public:
    void Init(UINT nID, CWnd * pParent);

    // Generated message map functions
protected:
    //{{AFX_MSG(CAboutTitle)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CStaticText dialog child window

class CStaticText : public CWnd
{
    DECLARE_DYNCREATE(CStaticText)

// Construction/destruction/creation
public:
    CStaticText();
    ~CStaticText();

    // Attributes
private:
    UINT m_nID;

    // Operations
public:
    void Init(UINT nID, CWnd * pParent);

    // Generated message map functions
protected:
    //{{AFX_MSG(CStaticText)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CFancyArrow dialog child window

class CFancyArrow : public CWnd
{
    DECLARE_DYNCREATE(CFancyArrow)

// Construction/destruction/creation
public:
    CFancyArrow();
    ~CFancyArrow();

    // Attributes
private:

    // Operations
public:
    void Init(UINT nID, CWnd * pParent);

    // Generated message map functions
protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    DECLARE_MESSAGE_MAP()
};


//###########################################################################
// CLayoutListBox custom menu

class CLayoutListBox : public CListBox
{

    // Construction/destruction/creation
public:
    CLayoutListBox()
    {
        m_number = -1;
    };
    virtual ~CLayoutListBox();
    int GetLayoutFromSelection(int nIndex);
    int GetPreferredLayout() const
    {
        return GetPreferredLayout(m_number);
    }
    static int GetPreferredLayout(int number);

    // Attributes
private:
    int m_number;

    // Operations
public:
    void AppendLayoutItem(int nNumber);
    void SelectLayoutWithCount(int nNumber = -1);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
};

#endif //_SA_DLWND_H
