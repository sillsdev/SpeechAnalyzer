/////////////////////////////////////////////////////////////////////////////
// sa_dlwnd.cpp:
// Implementation of the CLEDDisplay (dialog child window)
//                       CSliderVertical (dialog child window)
//                       CSliderHorizontal (dialog child window)
//                       CVUBar (dialog child window)
//                       CProgressBar (window)
//                       CComboGridlines (combobox with gridlines)
//                       CSpinControl (micro scroll buttons)
//                       CToggleButton (toggle button)
//                       CTranscriptionDisp (dialog child window)
//                       CStaticText (dialog child window)
//                       CFancyArrow (dialog child window)
//                       CAboutTitle (dialog child window)        classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
//
// Revision History
//    1.06.6U6
//         SDM Change CSliderVertical to respond to clicks outside the bitmap
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process\Process.h"
#include "Segment.h"

#include "sa_doc.h"
#include "sa_view.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CLEDDisplay
// LED display with light blue bitmap numbers in a black 3D box. It displays
// time in minutes and seconds in the format "XX:YY.Y". Seconds are in 100ms
// units.

IMPLEMENT_DYNCREATE(CLEDDisplay, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CLEDDisplay message map

BEGIN_MESSAGE_MAP(CLEDDisplay, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLEDDisplay construction/destruction/creation

/***************************************************************************/
// CLEDDisplay::CLEDDisplay Constructor
/***************************************************************************/
CLEDDisplay::CLEDDisplay() {
    // construct and load LED bitmap
    m_pBmLED = new CBitmap;
    m_pBmLED->LoadBitmap(IDB_LED);
    m_chMinMSD = m_chMinLSD = 10;             // display all dashs
    m_chSecMSD = m_chSecLSD = m_chSecDec = 10;
}

/***************************************************************************/
// CLEDDisplay::CLEDDisplay Destructor
/***************************************************************************/
CLEDDisplay::~CLEDDisplay() {
    // destruct LED bitmap
    if (m_pBmLED) {
        delete m_pBmLED;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CLEDDisplay helper functions

/***************************************************************************/
// CLEDDisplay::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CLEDDisplay::Init(UINT nID, CWnd * pParent) {
    if (nID && pParent) {
        CWnd * pWnd = pParent->GetDlgItem(nID);
        CRect rWnd(0, 0, 0, 0);
        Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
        pWnd->GetWindowRect(rWnd);
        pParent->ScreenToClient(rWnd);
        MoveWindow(rWnd, TRUE);
    }
}

/***************************************************************************/
// CLEDDisplay::SetTime Set time in display
// Puts the right parts of the LED-bitmap into the window to display the
// time given as parameter (seconds are in 100ms units). If nMin is larger
// than 99, minutes will display "--". If nSec is larger than 599, seconds
// will display "--.-".
/***************************************************************************/
void CLEDDisplay::SetTime(int nMin, int nSec) {
    char chOldMinMSD, chOldMinLSD, chOldSecMSD, chOldSecLSD, chOldSecDec;
    chOldMinMSD = m_chMinMSD; // save old values
    chOldMinLSD = m_chMinLSD;
    chOldSecMSD = m_chSecMSD;
    chOldSecLSD = m_chSecLSD;
    chOldSecDec = m_chSecDec;
    if (nMin > 99) { // dashs to display
        m_chMinMSD = 10;
        m_chMinLSD = 10;
    } else {
        m_chMinMSD = char(nMin / 10);
        m_chMinLSD = char(nMin % 10);
    }
    if ((int)nSec > 599) { // dashs to display
        m_chSecMSD = 10;
        m_chSecLSD = 10;
        m_chSecDec = 10;
    } else {
        m_chSecMSD = char((int)nSec / 100);
        m_chSecLSD = char(((int)nSec % 100) / 10);
        m_chSecDec = char((int)nSec % 10);
    }
    CRect rWnd; // prepare invalid rectangle
    GetClientRect(rWnd);
    rWnd.top = (rWnd.bottom - LED_DIGIT_HEIGHT) / 2;
    rWnd.bottom = rWnd.top + LED_DIGIT_HEIGHT;
    rWnd.left = (rWnd.right - 5 * LED_DIGIT_WIDTH - 2 * LED_POINT_WIDTH - (7 * LED_DIGIT_SPACE - 1)) / 2;
    rWnd.right = rWnd.left + 5 * LED_DIGIT_WIDTH + 2 * LED_POINT_WIDTH + (7 * LED_DIGIT_SPACE - 1);
    if (chOldMinMSD != m_chMinMSD) {
        InvalidateRect(rWnd, FALSE);    // value changed
    } else {
        rWnd.left += LED_DIGIT_WIDTH + LED_DIGIT_SPACE;
        if (chOldMinLSD != m_chMinLSD) {
            InvalidateRect(rWnd, FALSE);    // value changed
        } else {
            rWnd.left += LED_DIGIT_WIDTH + LED_POINT_WIDTH + 2 * LED_DIGIT_SPACE;
            if (chOldSecMSD != m_chSecMSD) {
                InvalidateRect(rWnd, FALSE);    // value changed
            } else {
                rWnd.left += LED_DIGIT_WIDTH + LED_DIGIT_SPACE;
                if (chOldSecLSD != m_chSecLSD) {
                    InvalidateRect(rWnd, FALSE);    // value changed
                } else {
                    rWnd.left += LED_DIGIT_WIDTH + LED_POINT_WIDTH + 2 * LED_DIGIT_SPACE;
                    if (chOldSecDec != m_chSecDec) {
                        InvalidateRect(rWnd, FALSE);    // value changed
                    } else {
                        return;
                    }
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// CLEDDisplay message handlers

/***************************************************************************/
// CLEDDisplay::OnPaint Painting
/***************************************************************************/
void CLEDDisplay::OnPaint() {
    CPaintDC dc(this); // device context for painting
    CBitmap * pBmOld = NULL;
    CDC dcMem;
    // get invalid region
    CRect rClip;
    dc.GetClipBox(&rClip);
    dc.LPtoDP(&rClip);
    dcMem.CreateCompatibleDC(&dc);
    pBmOld = dcMem.SelectObject(m_pBmLED);
    CRect rWnd;
    GetClientRect(rWnd);
    rWnd.top = (rWnd.bottom - LED_DIGIT_HEIGHT) / 2; // center display
    rWnd.left = (rWnd.right - 5 * LED_DIGIT_WIDTH - 2 * LED_POINT_WIDTH - (7 * LED_DIGIT_SPACE - 1)) / 2;
    // put bitmap onto screen
    if (rClip.left < (rWnd.left + LED_DIGIT_WIDTH))
        dc.BitBlt(rWnd.left, rWnd.top, LED_DIGIT_WIDTH, LED_DIGIT_HEIGHT,
                  &dcMem, m_chMinMSD * LED_DIGIT_WIDTH, 0, SRCCOPY); // minutes MSD
    rWnd.left += LED_DIGIT_WIDTH + LED_DIGIT_SPACE;
    if (rClip.left < (rWnd.left + LED_DIGIT_WIDTH))
        dc.BitBlt(rWnd.left, rWnd.top, LED_DIGIT_WIDTH, LED_DIGIT_HEIGHT,
                  &dcMem, m_chMinLSD * LED_DIGIT_WIDTH, 0, SRCCOPY); // minutes LSD
    rWnd.left += LED_DIGIT_WIDTH + LED_DIGIT_SPACE;
    if (rClip.left < (rWnd.left + LED_POINT_WIDTH))
        dc.BitBlt(rWnd.left, rWnd.top, LED_POINT_WIDTH, LED_DIGIT_HEIGHT,
                  &dcMem, 11 * LED_DIGIT_WIDTH, 0, SRCCOPY); // double point
    rWnd.left += LED_POINT_WIDTH + LED_DIGIT_SPACE;
    if (rClip.left < (rWnd.left + LED_DIGIT_WIDTH))
        dc.BitBlt(rWnd.left, rWnd.top, LED_DIGIT_WIDTH, LED_DIGIT_HEIGHT,
                  &dcMem, m_chSecMSD * LED_DIGIT_WIDTH, 0, SRCCOPY); // seconds MSD
    rWnd.left += LED_DIGIT_WIDTH + LED_DIGIT_SPACE;
    if (rClip.left < (rWnd.left + LED_DIGIT_WIDTH))
        dc.BitBlt(rWnd.left, rWnd.top, LED_DIGIT_WIDTH, LED_DIGIT_HEIGHT,
                  &dcMem, m_chSecLSD * LED_DIGIT_WIDTH, 0, SRCCOPY); // seconds LSD
    rWnd.left += LED_DIGIT_WIDTH + LED_DIGIT_SPACE;
    if (rClip.left < (rWnd.left + LED_POINT_WIDTH))
        dc.BitBlt(rWnd.left, rWnd.top, LED_POINT_WIDTH, LED_DIGIT_HEIGHT,
                  &dcMem, 11 * LED_DIGIT_WIDTH + LED_POINT_WIDTH, 0, SRCCOPY); // double point
    rWnd.left += LED_POINT_WIDTH + LED_DIGIT_SPACE;
    if (rClip.left < (rWnd.left + LED_DIGIT_WIDTH))
        dc.BitBlt(rWnd.left, rWnd.top, LED_DIGIT_WIDTH, LED_DIGIT_HEIGHT,
                  &dcMem, m_chSecDec * LED_DIGIT_WIDTH, 0, SRCCOPY); // seconds decimal
    dcMem.SelectObject(pBmOld);
    dcMem.DeleteDC();
}

/***************************************************************************/
// CLEDDisplay::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CLEDDisplay::OnEraseBkgnd(CDC * pDC) {
    CRect rWnd;
    GetClientRect(rWnd);
    // get invalid region
    CRect rClip;
    pDC->GetClipBox(&rClip);
    pDC->LPtoDP(&rClip);
    CBrush brush;
    brush.CreateStockObject(BLACK_BRUSH);
    pDC->FillRect(&rClip, &brush);
    // paint 3D-frame
    CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    CPen * pOldPen = pDC->SelectObject(&penDkgray);
    pDC->MoveTo(0, rWnd.bottom); // paint 3D-frame
    pDC->LineTo(0, 0);
    pDC->LineTo(rWnd.right - 1, 0);
    pDC->SelectObject(&penLtgray);
    pDC->LineTo(rWnd.right - 1, rWnd.bottom - 1);
    pDC->LineTo(0, rWnd.bottom - 1);
    pDC->SelectObject(pOldPen);
    return TRUE;
}

//###########################################################################
// CSliderVertical
// Vertical 3D slider, whichs knob can be draged with the mouse, or set by
// function call. Its range is 0...100. The parent will be notified by
// message and will receive a slider control ID with the message. The ID has
// to be delivered to the control after creating it.

IMPLEMENT_DYNCREATE(CSliderVertical, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CSliderVertical message map

BEGIN_MESSAGE_MAP(CSliderVertical, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSliderVertical construction/destruction/creation

/***************************************************************************/
// CSliderVertical::CSliderVertical Constructor
/***************************************************************************/
CSliderVertical::CSliderVertical() {
    // construct and load knob bitmap
    m_pBmKnob = new CBitmap;
    m_pBmKnob->LoadBitmap(IDB_KNOBHORIZ); // load bitmap
    BITMAP bm;
    m_pBmKnob->GetObject(sizeof(BITMAP), &bm); // get bitmap size
    m_rPos.SetRect(0, 0, bm.bmWidth, bm.bmHeight);
    m_chPos = 100;
    m_bKnobDrag = FALSE;
    m_nID = NULL;
}

/***************************************************************************/
// CSliderVertical::~CSliderVertical Destructor
/***************************************************************************/
CSliderVertical::~CSliderVertical() {
    // destruct knob bitmap
    if (m_pBmKnob) {
        delete m_pBmKnob;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CSliderVertical helper functions

/***************************************************************************/
// CSliderVertical::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CSliderVertical::Init(UINT nID, CWnd * pParent) {
    m_nID = nID;
    if (m_nID && pParent) {
        CWnd * pWnd = pParent->GetDlgItem(m_nID);
        CRect rWnd(0, 0, 0, 0);
        Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
        pWnd->GetWindowRect(rWnd);
        pParent->ScreenToClient(rWnd);
        MoveWindow(rWnd, TRUE);
    }
}

/***************************************************************************/
// CSliderVertical::SetPosition Set slider position
// The slider position range goes from 0 to 100. This function sets the
// slider to a position given by parameter (limitet to 100).
/***************************************************************************/
void CSliderVertical::SetPosition(int nVal) {
    CRect rWnd;
    GetClientRect(rWnd);
    int nOldPos = m_chPos;
    m_chPos = char(nVal);
    if (m_chPos > 100) {
        m_chPos = 100;
    }
    {
        CRect rOld = m_rPos; // save old knob position
        int nPos = (rWnd.bottom - m_rPos.Height()) * (100 - m_chPos) / 100;
        m_rPos.SetRect((rWnd.right - m_rPos.Width()) / 2, nPos,
                       ((rWnd.right - m_rPos.Width()) / 2) + m_rPos.Width(),
                       nPos + m_rPos.Height());
        if (rOld.left == 0) {
            InvalidateRect(NULL, TRUE);    // first paint, paint all new
        } else {
            if (nOldPos != m_chPos) { // change
                rOld.UnionRect(rOld, m_rPos); // create invalid rectangle
                InvalidateRect(rOld, TRUE); // paint
            } else {
                return;    // no change
            }
        }
        UpdateWindow();
    }
}

/***************************************************************************/
// CSliderVertical::GetPosFromMouse Get Position from Mouse
// Calculates the slider position from the mouse pointer position draging
// the slider knob.
/***************************************************************************/
int CSliderVertical::GetPosFromMouse(CPoint point) {
    CRect rWnd;
    GetClientRect(rWnd);
    rWnd.top = 100 - (100 * (point.y - m_rPos.Height() / 2) / (rWnd.bottom - m_rPos.Height()));
    if (rWnd.top > 100) {
        rWnd.top = 100;
    }
    if (rWnd.top < 0) {
        rWnd.top = 0;
    }
    return rWnd.top;
}

/////////////////////////////////////////////////////////////////////////////
// CSliderVertical message handlers

/***************************************************************************/
// CSliderVertical::OnPaint Painting
/***************************************************************************/
void CSliderVertical::OnPaint() {
    CPaintDC dc(this); // device context for painting
    CBitmap * pBmOld = NULL;
    CDC dcMem;
    dcMem.CreateCompatibleDC(&dc);
    pBmOld = dcMem.SelectObject(m_pBmKnob);
    // put bitmap onto screen
    dc.BitBlt(m_rPos.left, m_rPos.top, m_rPos.Width(), m_rPos.Height(), &dcMem, 0, 0, SRCCOPY); // knob
    dcMem.SelectObject(pBmOld);
    dcMem.DeleteDC();
}

/***************************************************************************/
// CSliderVertical::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CSliderVertical::OnEraseBkgnd(CDC * pDC) {
    // use same background as dialog
    CRect rWnd;
    GetClientRect(rWnd);

    // TCJ 7/00: 32-bit conversion - Make CSliderVertical system gray
    CBrush brGray(GetSysColor(COLOR_3DFACE));
    pDC->FillRect(rWnd,&brGray);

    // prepare painting of 3D slider track
    CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    CPen * pOldPen = pDC->SelectObject(&penDkgray);
    CRect rTrack;
    rTrack.SetRect(m_rPos.left + m_rPos.Width() / 2 - 1, m_rPos.Height() / 2,
                   m_rPos.left + m_rPos.Width() / 2 + 2, rWnd.bottom - m_rPos.Height() / 2);
    // paint track
    pDC->MoveTo(rTrack.left, rTrack.bottom - 1);
    pDC->LineTo(rTrack.left, rTrack.top);
    pDC->LineTo(rTrack.right + 1, rTrack.top);
    pDC->SelectObject(&penLtgray);
    pDC->MoveTo(rTrack.right, rTrack.top + 1);
    pDC->LineTo(rTrack.right, rTrack.bottom);
    pDC->LineTo(rTrack.left - 1, rTrack.bottom);
    pDC->MoveTo(rTrack.right - 1, rTrack.top + 1);
    pDC->LineTo(rTrack.right - 1, rTrack.bottom);
    pDC->SelectStockObject(BLACK_PEN);
    pDC->MoveTo(rTrack.left + 1, rTrack.top + 1);
    pDC->LineTo(rTrack.left + 1, rTrack.bottom);
    // prepare painting of position markers
    rTrack.SetRect(m_rPos.left - 5, m_rPos.Height() / 2,
                   m_rPos.right + 4, rWnd.bottom - m_rPos.Height() / 2);
    float fPos = (float)rTrack.top;
    for (int i = 0; i < 11; i++) {
        if ((i == 0) || (i == 5) || (i == 10)) { // big markers
            pDC->MoveTo(rTrack.left, (int)fPos);
            pDC->LineTo(rTrack.left + 4, (int)fPos);
            pDC->MoveTo(rTrack.right, (int)fPos);
            pDC->LineTo(rTrack.right - 4, (int)fPos);
        } else { // small markers
            pDC->MoveTo(rTrack.left + 2, (int)fPos);
            pDC->LineTo(rTrack.left + 4, (int)fPos);
            pDC->MoveTo(rTrack.right - 2, (int)fPos);
            pDC->LineTo(rTrack.right - 4, (int)fPos);
        }
        fPos += (float)rTrack.Height() / 10;
    }
    pDC->SelectObject(pOldPen);
    return TRUE;
}

/***************************************************************************/
// CSliderVertical::OnLButtonDown Left mouse button down
// If the user hits the slider knob with the mouse pointer, then the slider
// window captures all mouse input, until the user releases the left mouse
// button, and drags the slider knob.
/***************************************************************************/
void CSliderVertical::OnLButtonDown(UINT nFlags, CPoint point) {
    // SDM 1.06.6U6 if (m_rPos.PtInRect(point)) // mouseclick on knob
    {
        m_bKnobDrag = TRUE;
        SetCapture(); // receive all mouse input
        SetPosition(GetPosFromMouse(point)); // calculate and paint new position
    }
    CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CSliderVertical::OnLButtonUp Left mouse button up
// If the user releases the left mouse button and he was dragging the slider
// knob, then the parent (dialog) is notified to synchronize other controls
// with the new slider position. The mouse input is freed to all windows.
/***************************************************************************/
void CSliderVertical::OnLButtonUp(UINT nFlags, CPoint point) {
    m_bKnobDrag = FALSE;
    ReleaseCapture(); // mouse input also to other windows
    GetParent()->SendMessage(WM_COMMAND, m_nID, 0); // notify parent
    CWnd::OnLButtonUp(nFlags, point);
}

/***************************************************************************/
// CSliderVertical::OnMouseMove Mouse moved
/***************************************************************************/
void CSliderVertical::OnMouseMove(UINT nFlags, CPoint point) {
    if (m_bKnobDrag) {
        SetPosition(GetPosFromMouse(point)); // calculate and paint new position
        GetParent()->SendMessage(WM_COMMAND, m_nID, 0); // notify parent
    }
    CWnd::OnMouseMove(nFlags, point);
}

//###########################################################################
// CSliderHorizontal
// Horizontal 3D slider, whichs knob can be draged with the mouse, or set by
// function call. Its range is 0...100. The parent will be notified by
// message and will receive a slider control ID with the message. The ID has
// to be delivered to the control after creating it.

IMPLEMENT_DYNCREATE(CSliderHorizontal, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CSliderHorizontal message map

BEGIN_MESSAGE_MAP(CSliderHorizontal, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSliderHorizontal construction/destruction/creation

/***************************************************************************/
// CSliderHorizontal::CSliderHorizontal Constructor
/***************************************************************************/
CSliderHorizontal::CSliderHorizontal() {
    // construct and load knob bitmap
    m_pBmKnob = new CBitmap;
    m_pBmKnob->LoadBitmap(IDB_KNOBVERT); // load bitmap
    BITMAP bm;
    m_pBmKnob->GetObject(sizeof(BITMAP), &bm); // get bitmap size
    m_rPos.SetRect(0, 0, bm.bmWidth, bm.bmHeight);
    m_chPos = 100;
    m_bKnobDrag = FALSE;
    m_nID = NULL;
}

/***************************************************************************/
// CSliderHorizontal::~CSliderHorizontal Destructor
/***************************************************************************/
CSliderHorizontal::~CSliderHorizontal() {
    // destruct knob bitmap
    if (m_pBmKnob) {
        delete m_pBmKnob;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CSliderHorizontal helper functions

/***************************************************************************/
// CSliderHorizontal::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CSliderHorizontal::Init(UINT nID, CWnd * pParent) {
    m_nID = nID;
    if (m_nID && pParent) {
        CWnd * pWnd = pParent->GetDlgItem(m_nID);
        CRect rWnd(0, 0, 0, 0);
        Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
        pWnd->GetWindowRect(rWnd);
        pParent->ScreenToClient(rWnd);
        MoveWindow(rWnd, TRUE);
    }
}

/***************************************************************************/
// CSliderHorizontal::SetPosition Set slider position
// The slider position range goes from 0 to 100. This function sets the
// slider to a position given by parameter (limitet to 100).
/***************************************************************************/
void CSliderHorizontal::SetPosition(int nVal) {
    CRect rWnd;
    GetClientRect(rWnd);
    int nOldPos = m_chPos;
    m_chPos = char(nVal);
    if (m_chPos > 100) {
        m_chPos = 100;
    }
    {
        CRect rOld = m_rPos; // save old knob position
        int nPos = (rWnd.right - m_rPos.Width()) * m_chPos / 100;
        m_rPos.SetRect(nPos, (rWnd.bottom - m_rPos.Height()) / 2,
                       nPos + m_rPos.Width(),
                       ((rWnd.bottom - m_rPos.Height()) / 2) + m_rPos.Height());
        if (rOld.left == 0) {
            InvalidateRect(NULL, TRUE);    // first paint, paint all new
        } else {
            if (nOldPos != m_chPos) { // change
                rOld.UnionRect(rOld, m_rPos); // create invalid rectangle
                InvalidateRect(rOld, TRUE); // paint
            } else {
                return;    // no change
            }
        }
        UpdateWindow();
    }
}

/***************************************************************************/
// CSliderHorizontal::GetPosFromMouse Get Position from Mouse
// Calculates the slider position from the mouse pointer position draging
// the slider knob.
/***************************************************************************/
int CSliderHorizontal::GetPosFromMouse(CPoint point) {
    CRect rWnd;
    GetClientRect(rWnd);
    rWnd.left = (100 * (point.x - m_rPos.Width() / 2) / (rWnd.right - m_rPos.Width()));
    if (rWnd.left > 100) {
        rWnd.left = 100;
    }
    if (rWnd.left < 0) {
        rWnd.left = 0;
    }
    return rWnd.left;
}

/////////////////////////////////////////////////////////////////////////////
// CSliderHorizontal message handlers

/***************************************************************************/
// CSliderHorizontal::OnPaint Painting
/***************************************************************************/
void CSliderHorizontal::OnPaint() {
    CPaintDC dc(this); // device context for painting
    CBitmap * pBmOld = NULL;
    CDC dcMem;
    dcMem.CreateCompatibleDC(&dc);
    pBmOld = dcMem.SelectObject(m_pBmKnob);
    // put bitmap onto screen
    dc.BitBlt(m_rPos.left, m_rPos.top, m_rPos.Width(), m_rPos.Height(), &dcMem, 0, 0, SRCCOPY); // knob
    dcMem.SelectObject(pBmOld);
    dcMem.DeleteDC();
}

/***************************************************************************/
// CSliderHorizontal::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CSliderHorizontal::OnEraseBkgnd(CDC * pDC) {
    // use same background as dialog
    CRect rWnd;
    GetClientRect(rWnd);

    // TCJ 7/00: 32-bit conversion - Make CSliderHorizontal system gray
    CBrush brGray(GetSysColor(COLOR_3DFACE));
    pDC->FillRect(rWnd,&brGray);

    // prepare painting of 3D slider track
    CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    CPen * pOldPen = pDC->SelectObject(&penDkgray);
    CRect rTrack;
    rTrack.SetRect(m_rPos.Width() / 2 , m_rPos.top + m_rPos.Height() / 2 - 1,
                   rWnd.right - m_rPos.Width() / 2, m_rPos.top + m_rPos.Height() / 2 + 2);
    // paint track
    pDC->MoveTo(rTrack.left, rTrack.bottom - 1);
    pDC->LineTo(rTrack.left, rTrack.top);
    pDC->LineTo(rTrack.right + 1, rTrack.top);
    pDC->SelectObject(&penLtgray);
    pDC->MoveTo(rTrack.right, rTrack.top + 1);
    pDC->LineTo(rTrack.right, rTrack.bottom);
    pDC->LineTo(rTrack.left - 1, rTrack.bottom);
    pDC->MoveTo(rTrack.right - 1, rTrack.top + 1);
    pDC->LineTo(rTrack.right - 1, rTrack.bottom);
    pDC->SelectStockObject(BLACK_PEN);
    pDC->MoveTo(rTrack.left + 1, rTrack.top + 1);
    pDC->LineTo(rTrack.right, rTrack.top + 1);
    // prepare painting of position markers
    rTrack.SetRect(m_rPos.Width() / 2, m_rPos.top - 5,
                   rWnd.right - m_rPos.Width() / 2, m_rPos.bottom + 4);
    float fPos = (float)rTrack.left;
    for (int i = 0; i < 11; i++) {
        if ((i == 0) || (i == 5) || (i == 10)) { // big markers
            pDC->MoveTo((int)fPos, rTrack.top);
            pDC->LineTo((int)fPos, rTrack.top + 4);
            pDC->MoveTo((int)fPos, rTrack.bottom);
            pDC->LineTo((int)fPos, rTrack.bottom - 4);
        } else { // small markers
            pDC->MoveTo((int)fPos, rTrack.top + 2);
            pDC->LineTo((int)fPos, rTrack.top + 4);
            pDC->MoveTo((int)fPos, rTrack.bottom - 2);
            pDC->LineTo((int)fPos, rTrack.bottom - 4);
        }
        fPos += (float)rTrack.Width() / 10;
    }
    pDC->SelectObject(pOldPen);
    return TRUE;
}

/***************************************************************************/
// CSliderHorizontal::OnLButtonDown Left mouse button down
// If the user hits the slider knob with the mouse pointer, then the slider
// window captures all mouse input, until the user releases the left mouse
// button, and drags the slider knob.
/***************************************************************************/
void CSliderHorizontal::OnLButtonDown(UINT nFlags, CPoint point) {
    if (m_rPos.PtInRect(point)) { // mouseclick on knob
        m_bKnobDrag = TRUE;
        SetCapture(); // receive all mouse input
        SetPosition(GetPosFromMouse(point)); // calculate and paint new position
    }
    CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CSliderHorizontal::OnLButtonUp Left mouse button up
// If the user releases the left mouse button and he was dragging the slider
// knob, then the parent (dialog) is notified to synchronize other controls
// with the new slider position. The mouse input is freed to all windows.
/***************************************************************************/
void CSliderHorizontal::OnLButtonUp(UINT nFlags, CPoint point) {
    m_bKnobDrag = FALSE;
    ReleaseCapture(); // mouse input also to other windows
    GetParent()->SendMessage(WM_COMMAND, m_nID, 0); // notify parent
    CWnd::OnLButtonUp(nFlags, point);
}

/***************************************************************************/
// CSliderHorizontal::OnMouseMove Mouse moved
/***************************************************************************/
void CSliderHorizontal::OnMouseMove(UINT nFlags, CPoint point) {
    if (m_bKnobDrag) {
        SetPosition(GetPosFromMouse(point)); // calculate and paint new position
        GetParent()->SendMessage(WM_COMMAND, m_nID, 0); // notify parent
    }
    CWnd::OnMouseMove(nFlags, point);
}

//###########################################################################
// CVUBar
// VU bar with light blue and red bar blocks (a presetable number of blocks
// per bar) in a black 3D box. The block size and the 0 dB power point are also
// presetable. The bar works horizontally or vertically, dependent on which side
// of the window is bigger.

IMPLEMENT_DYNCREATE(CVUBar, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CVUBar message map

BEGIN_MESSAGE_MAP(CVUBar, CWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVUBar construction/destruction/creation

/***************************************************************************/
// CVUBar::CVUBar Constructor
/***************************************************************************/
CVUBar::CVUBar() {
    m_chLVU = m_chRVU = 0;
    m_chLText = 'L'; // left channel text
    m_chRText = 'R'; // right channel text
    m_chLPeak = m_chRPeak = 0;
    m_nLPeakHoldCount = m_nRPeakHoldCount = 0;
    m_nPeakHold = VU_BAR_HOLDCOUNT;
    m_bPeak = TRUE;
}

/***************************************************************************/
// CVUBar::~CVUBar Destructor
/***************************************************************************/
CVUBar::~CVUBar() {
}

/////////////////////////////////////////////////////////////////////////////
// CVUBar helper functions

/***************************************************************************/
// CVUBar::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CVUBar::Init(UINT nID, CWnd * pParent) {
    if (nID && pParent) {
        CWnd * pWnd = pParent->GetDlgItem(nID);
        CRect rWnd(0, 0, 0, 0);
        Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
        pWnd->GetWindowRect(rWnd);
        pParent->ScreenToClient(rWnd);
        MoveWindow(rWnd, TRUE);
    }
}

/***************************************************************************/
// CVUBar::InitVU Initialize VU bar
/***************************************************************************/
void CVUBar::InitVU(char chLText, char chRText, BOOL bPeak, int nPeakHold) {
    m_chLText = chLText;
    m_chRText = chRText;
    m_nPeakHold = nPeakHold;
    m_bPeak = bPeak;
}

/***************************************************************************/
// CVUBar::Reset Resets the peak settings
/***************************************************************************/
void CVUBar::Reset() {
    m_nLPeakHoldCount = m_nPeakHold;
    m_nRPeakHoldCount = m_nPeakHold;
    SetVU(0);
}

/***************************************************************************/
// CVUBar::SetPosition Set VU bar
// The bar range goes from 0 to 100. This function sets the VU bar to a
// display given by parameter (limitet to 100).
/***************************************************************************/
void CVUBar::SetVU(int nLVal, int nRVal) {
    m_chLOldVU = m_chLVU; // save old left value
    m_chROldVU = m_chRVU; // save old right value
    if (nLVal > 100) {
        nLVal = 100;    // limit new left value
    }
    // change new left value to number of displayed divisions
    m_chLVU = char((nLVal + (50 / VU_BAR_DIVISIONS)) / (100 / VU_BAR_DIVISIONS));
    if (nRVal == -1) {
        m_chRVU = m_chLVU;    // get new right value from left for mono
    } else {
        if (nRVal > 100) {
            nRVal = 100;
        }
        // change new right value to number of displayed divisions
        m_chRVU = char((nRVal + (50 / VU_BAR_DIVISIONS)) / (100 / VU_BAR_DIVISIONS));
    }
    BOOL bLPeakReset = FALSE;
    BOOL bRPeakReset = FALSE;
    char chLOldPeak=0, chROldPeak=0;
    if (m_bPeak) {
        // check left peak
        if (m_chLVU > m_chLPeak) {
            m_chLPeak = m_chLVU; // save left peak value
            m_nLPeakHoldCount = 0; // reset left peak count
        } else {
            m_nLPeakHoldCount++;
            if (m_nLPeakHoldCount > m_nPeakHold) {
                // reset left peak
                chLOldPeak = m_chLPeak; // save old left peak for invalidation
                bLPeakReset = TRUE;
                m_chLPeak = m_chLVU; // save left peak value
                m_nLPeakHoldCount = 0; // reset left peak count
            }
        }
        // check right peak
        if (m_chRVU > m_chRPeak) {
            m_chRPeak = m_chRVU; // save right peak value
            m_nRPeakHoldCount = 0; // reset right peak count
        } else {
            m_nRPeakHoldCount++;
            if (m_nRPeakHoldCount > m_nPeakHold) {
                // reset left peak
                chROldPeak = m_chRPeak; // save old right peak for invalidation
                bRPeakReset = TRUE;
                m_chRPeak = m_chRVU; // save right peak value
                m_nRPeakHoldCount = 0; // reset right peak count
            }
        }
    }
    CRect rWnd, rUpdate;
    GetClientRect(rWnd);
    if (rWnd.right < rWnd.bottom) { // vertical bar
        int nPixelPerDivision = (rWnd.Height() - 4 - m_nTextSpace) / VU_BAR_DIVISIONS;
        // set invalid left rectangle
        int nXLPos = (rWnd.Width() - VU_CHANNEL_DISTANCE - 2 * VU_BAR_WIDTH) / 2;
        // set invalid right rectangle
        int nXRPos = nXLPos + VU_CHANNEL_DISTANCE + VU_BAR_WIDTH;
        int nYPos = rWnd.bottom - 2 - m_nTextSpace;
        if (bLPeakReset) {
            // reset left peak bar
            rUpdate.left   = nXLPos;
            rUpdate.top    = nYPos - nPixelPerDivision * chLOldPeak;
            rUpdate.right  = nXLPos + VU_BAR_WIDTH;
            rUpdate.bottom = rUpdate.top + 3;
            InvalidateRect(rUpdate, TRUE);
        }
        if (bRPeakReset) {
            // reset right peak bar
            rUpdate.left   = nXRPos;
            rUpdate.top    = nYPos - nPixelPerDivision * chROldPeak;
            rUpdate.right  = nXRPos + VU_BAR_WIDTH;
            rUpdate.bottom = rUpdate.top + 3;
            InvalidateRect(rUpdate, TRUE);
        }
        if ((m_chLOldVU == m_chLVU) && (m_chROldVU == m_chRVU)) {
            return;    // no change
        }
        // invalidate left channel
        if (m_chLOldVU > m_chLVU) { // old left value was higher
            rUpdate.SetRect(nXLPos, nYPos - nPixelPerDivision * m_chLOldVU, nXLPos + VU_BAR_WIDTH, nYPos - nPixelPerDivision * m_chLVU);
        } else { // old left value was lower
            rUpdate.SetRect(nXLPos, nYPos - nPixelPerDivision * m_chLVU, nXLPos + VU_BAR_WIDTH, nYPos - nPixelPerDivision * m_chLOldVU);
        }
        InvalidateRect(rUpdate, TRUE);
        // invalidate right channel
        if (m_chROldVU > m_chRVU) { // old right value was higher
            rUpdate.SetRect(nXRPos, nYPos - nPixelPerDivision * m_chROldVU, nXRPos + VU_BAR_WIDTH, nYPos - nPixelPerDivision * m_chRVU);
        } else { // old right value was lower
            rUpdate.SetRect(nXRPos, nYPos - nPixelPerDivision * m_chRVU, nXRPos + VU_BAR_WIDTH, nYPos - nPixelPerDivision * m_chROldVU);
        }
        InvalidateRect(rUpdate, TRUE);
    } else { // horizontal bar
        int nPixelPerDivision = (rWnd.Width() - 4 - m_nTextSpace) / VU_BAR_DIVISIONS;
        // set invalid left rectangle
        int nYLPos = (rWnd.Height() - VU_CHANNEL_DISTANCE - 2 * VU_BAR_WIDTH) / 2;
        // set invalid right rectangle
        int nYRPos = nYLPos + VU_CHANNEL_DISTANCE + VU_BAR_WIDTH;
        int nXPos = 2 + m_nTextSpace;
        if (bLPeakReset) {
            // reset left peak bar
            rUpdate.right  = nXPos + nPixelPerDivision * chLOldPeak;
            rUpdate.left   = rUpdate.right - 3;
            rUpdate.top    = nYLPos;
            rUpdate.bottom = nYLPos + VU_BAR_WIDTH;
            InvalidateRect(rUpdate, TRUE);
        }
        if (bRPeakReset) {
            // reset right peak bar
            rUpdate.right  = nXPos + nPixelPerDivision * chROldPeak;
            rUpdate.left   = rUpdate.right - 3;
            rUpdate.top    = nYRPos;
            rUpdate.bottom = nYRPos + VU_BAR_WIDTH;
            InvalidateRect(rUpdate, TRUE);
        }
        if ((m_chLOldVU == m_chLVU) && (m_chROldVU == m_chRVU)) {
            return;    // no change
        }
        // invalidate left channel
        if (m_chLOldVU > m_chLVU) { // old left value was higher
            rUpdate.SetRect(nXPos + nPixelPerDivision * m_chLVU, nYLPos, nXPos + nPixelPerDivision * m_chLOldVU, nYLPos + VU_BAR_WIDTH);
        } else { // old left value was lower
            rUpdate.SetRect(nXPos + nPixelPerDivision * m_chLOldVU, nYLPos, nXPos + nPixelPerDivision * m_chLVU, nYLPos + VU_BAR_WIDTH);
        }
        InvalidateRect(rUpdate, TRUE);
        // invalidate right channel
        if (m_chROldVU > m_chRVU) { // old right value was higher
            rUpdate.SetRect(nXPos + nPixelPerDivision * m_chRVU, nYRPos, nXPos + nPixelPerDivision * m_chROldVU, nYRPos + VU_BAR_WIDTH);
        } else { // old right value was lower
            rUpdate.SetRect(nXPos + nPixelPerDivision * m_chROldVU, nYRPos, nXPos + nPixelPerDivision * m_chRVU, nYRPos + VU_BAR_WIDTH);
        }
        InvalidateRect(rUpdate, TRUE);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CVUBar message handlers

/***************************************************************************/
// CVUBar::OnCreate Window creation
// Creation of the font, used in the window.
/***************************************************************************/
int CVUBar::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }
    LPCTSTR pszName = _T("MS Sans Serif");
    m_font.CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                      DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), pszName);
    // get height of the font
    TEXTMETRIC tm;
    CDC * pDC = GetDC(); // get device context
    CFont * pOldFont = pDC->SelectObject(&m_font); // select font
    pDC->GetTextMetrics(&tm); // get text metrics
    pDC->SelectObject(pOldFont);  // set back old font
    ReleaseDC(pDC);
    m_nTextSpace = tm.tmHeight;
    return 0;
}

/***************************************************************************/
// CVUBar::OnPaint Painting
/***************************************************************************/
void CVUBar::OnPaint() {
    // create brushes and pens
    LOGBRUSH logBrushDarkGreen, logBrushGreen, logBrushRed, logBrushYellow;
    logBrushDarkGreen.lbStyle = BS_SOLID;
    logBrushDarkGreen.lbColor = RGB(0, 196, 0);
    logBrushGreen.lbStyle = BS_SOLID;
    logBrushGreen.lbColor = RGB(0, 255, 0);
    logBrushRed.lbStyle = BS_SOLID;
    logBrushRed.lbColor = RGB(255, 0, 0);
    logBrushYellow.lbStyle = BS_SOLID;
    logBrushYellow.lbColor = RGB(255, 255, 0);
    CBrush brushDarkGreen, brushGreen, brushRed, brushYellow;
    brushDarkGreen.CreateBrushIndirect(&logBrushDarkGreen);
    brushGreen.CreateBrushIndirect(&logBrushGreen);
    brushRed.CreateBrushIndirect(&logBrushRed);
    brushYellow.CreateBrushIndirect(&logBrushYellow);
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
    // prepare painting
    CPaintDC dc(this); // device context for painting
    CRect rWnd;
    GetClientRect(rWnd);
    // get invalid region
    CRect rClip, rDraw;
    dc.GetClipBox(&rClip);
    dc.LPtoDP(&rClip);
    CBrush * pOldBrush = dc.SelectObject(&brushDarkGreen);
    if (rWnd.right < rWnd.bottom) {
        // vertical bar
        int nPixelPerDivision = (rWnd.Height() - 4 - m_nTextSpace) / VU_BAR_DIVISIONS;
        // set invalid left rectangle
        int nXLPos = (rWnd.Width() - VU_CHANNEL_DISTANCE - 2 * VU_BAR_WIDTH) / 2;
        // set invalid right rectangle
        int nXRPos = nXLPos + VU_CHANNEL_DISTANCE + VU_BAR_WIDTH;
        int nYPos = rWnd.bottom - 2 - m_nTextSpace;
        if (rClip.bottom > nYPos) {
            // draw the text
            dc.SetTextColor(GetSysColor(COLOR_BTNFACE)); // set font color
            dc.SetBkMode(TRANSPARENT);
            rDraw.SetRect(nXLPos, nYPos, nXLPos + VU_BAR_WIDTH, rWnd.bottom - 2);
            CFont * pOldFont = dc.SelectObject(&m_font);
            dc.DrawText(&m_chLText, 1, rDraw, DT_SINGLELINE | DT_BOTTOM | DT_CENTER | DT_NOCLIP);
            rDraw.OffsetRect(VU_CHANNEL_DISTANCE + VU_BAR_WIDTH, 0);
            dc.DrawText(&m_chRText, 1, rDraw, DT_SINGLELINE | DT_BOTTOM | DT_CENTER | DT_NOCLIP);
            dc.SelectObject(pOldFont);
        }
        if ((rClip.left < nXLPos) || (rClip.right > (nXRPos + VU_BAR_WIDTH))) {
            // draw the 0 dB lines for power, 3 dB for peak
            CPen * pOldPen = dc.SelectObject(&penLtgray);
            rDraw.SetRect(nXLPos, nYPos - VU_BAR_YELLOW * nPixelPerDivision, nXRPos + VU_BAR_WIDTH, 0);
            dc.MoveTo(rWnd.left + 2, rDraw.top);
            dc.LineTo(rDraw.left, rDraw.top);
            dc.MoveTo(rWnd.right - 2, rDraw.top);
            dc.LineTo(rDraw.right, rDraw.top);
            dc.SelectObject(pOldPen);
        }
        // draw the divisions
        int nLoop = VU_BAR_DIVISIONS - (rClip.bottom - 4 + m_nTextSpace + nPixelPerDivision - 1) / nPixelPerDivision;
        if (nLoop < 0) {
            nLoop = 0;
        }
        rDraw.SetRect(nXLPos, nYPos - (nLoop + 1) * nPixelPerDivision,
                      nXLPos + VU_BAR_WIDTH, nYPos - nLoop * nPixelPerDivision);
        if (nLoop >= VU_BAR_GREEN) {
            dc.SelectObject(&brushGreen);    // select green brush
        }
        if (nLoop >= VU_BAR_YELLOW) {
            dc.SelectObject(&brushYellow);    // select yellow brush
        }
        if (nLoop >= VU_BAR_RED) {
            dc.SelectObject(&brushRed);    // select red brush
        }
        int nLimit = m_chLVU;
        if (nLimit < m_chRVU) {
            nLimit = m_chRVU;
        }
        for (nLoop; nLoop < nLimit; nLoop++) {
            // draw bar
            if (nLoop == VU_BAR_GREEN) {
                dc.SelectObject(&brushGreen);    // select green brush
            }
            if (nLoop == VU_BAR_YELLOW) {
                dc.SelectObject(&brushYellow);    // select yellow brush
            }
            if (nLoop == VU_BAR_RED) {
                dc.SelectObject(&brushRed);    // select red brush
            }
            if (nLoop < m_chLVU) {
                dc.Rectangle(rDraw);    // draw one of the left divisions
            }
            rDraw.OffsetRect(VU_CHANNEL_DISTANCE + VU_BAR_WIDTH, 0); // move draw rectangle to right channel
            if (nLoop < m_chRVU) {
                dc.Rectangle(rDraw);    // draw one of the right divisions
            }
            rDraw.OffsetRect(-VU_CHANNEL_DISTANCE - VU_BAR_WIDTH, -nPixelPerDivision); // move draw rectangle
        }
        if (m_bPeak) {
            if (m_chLPeak > (m_chLVU + 1)) {
                // draw the left peak
                dc.SelectObject(&brushDarkGreen);
                rDraw.top = nYPos - (int)m_chLPeak * nPixelPerDivision;
                rDraw.bottom = rDraw.top + 3;
                if (m_chLPeak >= VU_BAR_GREEN) {
                    dc.SelectObject(&brushGreen);    // select green brush
                }
                if (m_chLPeak > VU_BAR_YELLOW) {
                    dc.SelectObject(&brushYellow);    // select yellow brush
                }
                if (m_chLPeak > VU_BAR_RED) {
                    dc.SelectObject(&brushRed);    // select red brush
                }
                dc.Rectangle(rDraw);
            }
            if (m_chRPeak > (m_chRVU + 1)) {
                // draw the right peak
                dc.SelectObject(&brushDarkGreen);
                rDraw.OffsetRect(VU_CHANNEL_DISTANCE + VU_BAR_WIDTH, 0); // move draw rectangle to right channel
                rDraw.top = nYPos - (int)m_chRPeak * nPixelPerDivision;
                rDraw.bottom = rDraw.top + 3;
                if (m_chRPeak > VU_BAR_GREEN) {
                    dc.SelectObject(&brushGreen);    // select green brush
                }
                if (m_chRPeak > VU_BAR_YELLOW) {
                    dc.SelectObject(&brushYellow);    // select yellow brush
                }
                if (m_chRPeak > VU_BAR_RED) {
                    dc.SelectObject(&brushRed);    // select red brush
                }
                dc.Rectangle(rDraw);
            }
        }
    } else { // horizontal bar
        int nPixelPerDivision = (rWnd.Width() - 4 - m_nTextSpace) / VU_BAR_DIVISIONS;
        // set invalid left rectangle
        int nYLPos = (rWnd.Height() - VU_CHANNEL_DISTANCE - 2 * VU_BAR_WIDTH) / 2;
        // set invalid right rectangle
        int nYRPos = nYLPos + VU_CHANNEL_DISTANCE + VU_BAR_WIDTH;
        int nXPos = 2 + m_nTextSpace;
        if (rClip.left < (rWnd.left + nXPos)) {
            // draw the text
            dc.SetTextColor(GetSysColor(COLOR_BTNFACE)); // set font color
            dc.SetBkMode(TRANSPARENT);
            rDraw.SetRect(2, nYLPos, nXPos, nYLPos + VU_BAR_WIDTH);
            CFont * pOldFont = dc.SelectObject(&m_font);
            dc.DrawText(&m_chLText, 1, rDraw, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP);
            rDraw.OffsetRect(0, VU_CHANNEL_DISTANCE + VU_BAR_WIDTH);
            dc.DrawText(&m_chRText, 1, rDraw, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOCLIP);
            dc.SelectObject(pOldFont);
        }
        if ((rClip.left < nYLPos) || (rClip.right > (nYRPos + VU_BAR_WIDTH))) {
            // draw the 0 dB lines for power, 3 dB for peak
            CPen * pOldPen = dc.SelectObject(&penLtgray);
            rDraw.SetRect(nXPos + VU_BAR_YELLOW * nPixelPerDivision, nYLPos, 0, nYRPos + VU_BAR_WIDTH);
            dc.MoveTo(rDraw.left, rWnd.top + 2);
            dc.LineTo(rDraw.left, rDraw.top);
            dc.MoveTo(rDraw.left, rDraw.bottom);
            dc.LineTo(rDraw.left, rWnd.bottom - 2);
            dc.SelectObject(pOldPen);
        }
        int nLoop = (rClip.left - 2 - m_nTextSpace - nPixelPerDivision + 1) / nPixelPerDivision;
        if (nLoop < 0) {
            nLoop = 0;
        }
        rDraw.SetRect(nXPos + nLoop * nPixelPerDivision, nYLPos, nXPos + (nLoop + 1) * nPixelPerDivision, nYLPos + VU_BAR_WIDTH);
        if (nLoop >= VU_BAR_GREEN) {
            dc.SelectObject(&brushGreen);    // select green brush
        }
        if (nLoop >= VU_BAR_YELLOW) {
            dc.SelectObject(&brushYellow);    // select yellow brush
        }
        if (nLoop >= VU_BAR_RED) {
            dc.SelectObject(&brushRed);    // select red brush
        }
        int nLimit = m_chLVU;
        if (nLimit < m_chRVU) {
            nLimit = m_chRVU;
        }
        for (nLoop; nLoop < nLimit; nLoop++) {
            // draw bar
            if (nLoop == VU_BAR_GREEN) {
                dc.SelectObject(&brushGreen);    // select green brush
            }
            if (nLoop == VU_BAR_YELLOW) {
                dc.SelectObject(&brushYellow);    // select yellow brush
            }
            if (nLoop == VU_BAR_RED) {
                dc.SelectObject(&brushRed);    // select red brush
            }
            if (nLoop < m_chLVU) {
                dc.Rectangle(rDraw);    // draw one of the left divisions
            }
            rDraw.OffsetRect(0, VU_CHANNEL_DISTANCE + VU_BAR_WIDTH); // move draw rectangle to right channel
            if (nLoop < m_chRVU) {
                dc.Rectangle(rDraw);    // draw one of the right divisions
            }
            rDraw.OffsetRect(nPixelPerDivision, -VU_CHANNEL_DISTANCE - VU_BAR_WIDTH); // move draw rectangle
        }
        if (m_bPeak) {
            if (m_chLPeak > (m_chLVU + 1)) {
                // draw the left peak
                dc.SelectObject(&brushDarkGreen);
                rDraw.right = nXPos + (int)m_chLPeak * nPixelPerDivision;
                rDraw.left = rDraw.right - 3;
                if (m_chLPeak >= VU_BAR_GREEN) {
                    dc.SelectObject(&brushGreen);    // select green brush
                }
                if (m_chLPeak > VU_BAR_YELLOW) {
                    dc.SelectObject(&brushYellow);    // select yellow brush
                }
                if (m_chLPeak > VU_BAR_RED) {
                    dc.SelectObject(&brushRed);    // select red brush
                }
                dc.Rectangle(rDraw);
            }
            if (m_chRPeak > (m_chRVU + 1)) {
                // draw the right peak
                dc.SelectObject(&brushDarkGreen);
                rDraw.OffsetRect(0, VU_CHANNEL_DISTANCE + VU_BAR_WIDTH); // move draw rectangle to right channel
                rDraw.right = nXPos + (int)m_chRPeak * nPixelPerDivision;
                rDraw.left = rDraw.right - 3;
                if (m_chRPeak > VU_BAR_GREEN) {
                    dc.SelectObject(&brushGreen);    // select green brush
                }
                if (m_chRPeak > VU_BAR_YELLOW) {
                    dc.SelectObject(&brushYellow);    // select yellow brush
                }
                if (m_chRPeak > VU_BAR_RED) {
                    dc.SelectObject(&brushRed);    // select red brush
                }
                dc.Rectangle(rDraw);
            }
        }
    }
    dc.SelectObject(pOldBrush);
}

/***************************************************************************/
// CVUBar::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CVUBar::OnEraseBkgnd(CDC * pDC) {
    CRect rWnd;
    GetClientRect(rWnd);
    // get invalid region
    CRect rClip;
    pDC->GetClipBox(&rClip);
    pDC->LPtoDP(&rClip);

    // fill background
    CBrush brush;
    brush.CreateStockObject(BLACK_BRUSH);
    pDC->FillRect(&rClip, &brush);

    BOOL bRedraw = FALSE;
    // check invalid region to redraw 3D-frame
    if (rWnd.right < rWnd.bottom) {
        // vertical bar
        if ((rClip.left < (rWnd.left + (rWnd.right - VU_BAR_WIDTH) / 2))
                || (rClip.right > (rWnd.left + VU_BAR_WIDTH + (rWnd.right - VU_BAR_WIDTH) / 2))) {
            bRedraw = TRUE;
        }
    } else { // horizontal bar
        if ((rClip.top < (rWnd.top + (rWnd.bottom - VU_BAR_WIDTH) / 2))
                || (rClip.bottom > (rWnd.top + VU_BAR_WIDTH + (rWnd.bottom - VU_BAR_WIDTH) / 2))) {
            bRedraw = TRUE;
        }
    }
    if (bRedraw) {
        // paint 3D-frame
        CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
        CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
        CPen * pOldPen = pDC->SelectObject(&penDkgray);
        pDC->MoveTo(0, rWnd.bottom); // paint 3D-frame
        pDC->LineTo(0, 0);
        pDC->LineTo(rWnd.right - 1, 0);
        pDC->SelectObject(&penLtgray);
        pDC->LineTo(rWnd.right - 1, rWnd.bottom - 1);
        pDC->LineTo(0, rWnd.bottom - 1);
        pDC->SelectObject(pOldPen);
    }
    return TRUE;
}

//###########################################################################
// CProgressBar
// Progress bar windows 95 style with presetable number of blocks. The bar
// works only horizontally, its range is 0...100.

IMPLEMENT_DYNCREATE(CProgressBar, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CProgressBar message map

BEGIN_MESSAGE_MAP(CProgressBar, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressBar construction/destruction/creation

/***************************************************************************/
// CProgressBar::CProgressBar Constructor
/***************************************************************************/
CProgressBar::CProgressBar() {
    m_chProgress = -1;
    m_nProgress = 0;
}

/***************************************************************************/
// CProgressBar::~CProgressBar Destructor
/***************************************************************************/
CProgressBar::~CProgressBar() {
}

/////////////////////////////////////////////////////////////////////////////
// CProgressBar helper functions

/***************************************************************************/
// CProgressBar::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CProgressBar::Init(UINT nID, CWnd * pParent) {
    if (nID && pParent) {
        CWnd * pWnd = pParent->GetDlgItem(nID);
        CRect rWnd(0, 0, 0, 0);
        Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
        pWnd->GetWindowRect(rWnd);
        pParent->ScreenToClient(rWnd);
        MoveWindow(rWnd, TRUE);
    }
}

/***************************************************************************/
// CProgressBar::SetProgress Set progress bar
// The bar range goes from 0 to 100. This function sets the progress bar to a
// display given by parameter (limitet to 100).
/***************************************************************************/
void CProgressBar::SetProgress(int nVal) {
    char chOldProgress = m_chProgress; // save old value
    if (nVal > 100) {
        nVal = 100;    // limit new value
    }
    m_nProgress = nVal; // save new progress
    // change new value to number of displayed divisions
    m_chProgress = char((nVal + (50 / PROGRESS_BAR_DIVISIONS)) / (100 / PROGRESS_BAR_DIVISIONS));
    if (chOldProgress == m_chProgress) {
        return;    // no change
    }
    CRect rWnd, rUpdate;
    GetClientRect(rWnd);
    // calculate pixels per bar division
    int nPixelPerDivision = (rWnd.Width() - 2) / PROGRESS_BAR_DIVISIONS;
    // set invalid rectangle
    rUpdate.SetRect(2 + nPixelPerDivision * chOldProgress, 1,
                    2 + nPixelPerDivision * m_chProgress, rWnd.bottom - 1);
    InvalidateRect(rUpdate, TRUE);
    UpdateWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CProgressBar message handlers

/***************************************************************************/
// CProgressBar::OnPaint Painting
/***************************************************************************/
void CProgressBar::OnPaint() {
    // create brush
    LOGBRUSH logBrushBlue;
    logBrushBlue.lbStyle = BS_SOLID;
    logBrushBlue.lbColor = RGB(0, 0, 128);
    CBrush brushBlue;
    brushBlue.CreateBrushIndirect(&logBrushBlue);
    // create pen
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
    // prepare painting
    CPaintDC dc(this); // device context for painting
    CRect rWnd;
    GetClientRect(rWnd);
    // get invalid region
    CRect rClip, rDraw;
    dc.GetClipBox(&rClip);
    dc.LPtoDP(&rClip);
    CBrush * pOldBrush = dc.SelectObject(&brushBlue);
    CPen * pOldPen = dc.SelectObject(&penLtgray);
    // calculate pixels per bar division and first division to draw
    int nPixelPerDivision = (rWnd.Width() - 2) / PROGRESS_BAR_DIVISIONS;
    int nLoop = (rClip.left - 1 - nPixelPerDivision + 1) / nPixelPerDivision;
    if (nLoop < 0) {
        nLoop = 0;
    }
    rDraw.SetRect(1 + nLoop * nPixelPerDivision, 1,
                  2 + (nLoop + 1) * nPixelPerDivision, rWnd.bottom - 1);
    for (nLoop; nLoop < m_chProgress; nLoop++) {
        // draw bar
        dc.Rectangle(rDraw); // draw one division
        rDraw.OffsetRect(nPixelPerDivision, 0); // move draw rectangle
    }
    dc.SelectObject(pOldPen);
    dc.SelectObject(pOldBrush);
}

/***************************************************************************/
// CProgressBar::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CProgressBar::OnEraseBkgnd(CDC * pDC) {
    CRect rWnd;
    GetClientRect(rWnd);

    // TCJ 7/00: 32-bit conversion - Make CProgressBar system gray
    CBrush brGray(GetSysColor(COLOR_3DFACE));
    pDC->FillRect(rWnd,&brGray);

    // paint 3D-frame
    CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    CPen * pOldPen = pDC->SelectObject(&penDkgray);
    pDC->MoveTo(0, rWnd.bottom); // paint 3D-frame
    pDC->LineTo(0, 0);
    pDC->LineTo(rWnd.right - 1, 0);
    pDC->SelectObject(&penLtgray);
    pDC->LineTo(rWnd.right - 1, rWnd.bottom - 1);
    pDC->LineTo(0, rWnd.bottom - 1);
    pDC->SelectObject(pOldPen);
    return TRUE;
}

//###########################################################################
// CComboGridlines
// Customized Combobox with gridlines in it. Base class is the standard Combo-
// box.

/////////////////////////////////////////////////////////////////////////////
// CComboGridlines helper functions

/***************************************************************************/
// CComboGridlines::AddLineItem Add item to list
/***************************************************************************/
void CComboGridlines::AddLineItem(int nPenStyle) {
    // add a combobox item
    AddString((LPCTSTR)nPenStyle); // the normal combobox string is used to store an pen style
}

/***************************************************************************/
// CComboGridlines::MeasureItem Item measuring
/***************************************************************************/
void CComboGridlines::MeasureItem(LPMEASUREITEMSTRUCT lpMIS) {
    // all items are of fixed size must use CBS_OWNERDRAWVARIABLE for this to work
    lpMIS->itemHeight = COLOR_ITEM_HEIGHT;
}

/***************************************************************************/
// CComboGridlines::DrawItem Item drawing
/***************************************************************************/
void CComboGridlines::DrawItem(LPDRAWITEMSTRUCT lpDIS) {
    CDC * pDC = CDC::FromHandle(lpDIS->hDC);
    int nPenStyle = (int)lpDIS->itemData; // pen style in item data
    if (nPenStyle >= 0) { // if no valid pen style, don't draw
        BOOL bDraw = FALSE;
        BOOL bHighlight = FALSE;
        if (lpDIS->itemAction & ODA_DRAWENTIRE) {
            bDraw = TRUE;
        }
        if ((lpDIS->itemState & ODS_SELECTED) && (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE))) {
            bDraw = TRUE;
            bHighlight = TRUE;
        }
        if (!(lpDIS->itemState & ODS_SELECTED) && (lpDIS->itemAction & ODA_SELECT)) {
            bDraw = TRUE;
        }
        if (bDraw) {
            COLORREF colorText = bHighlight ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_WINDOWTEXT);
            COLORREF colorBack = bHighlight ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_WINDOW);
            CPen pen(nPenStyle, 1, colorText); // black line
            CPen * pOldPen = pDC->SelectObject(&pen);
            LOGBRUSH logBrush;
            logBrush.lbStyle = BS_SOLID;
            logBrush.lbColor = colorBack;
            CBrush brush;
            brush.CreateBrushIndirect(&logBrush);
            int nPos = lpDIS->rcItem.top + (lpDIS->rcItem.bottom - lpDIS->rcItem.top) / 2;
            COLORREF color = pDC->SetBkColor(colorBack);

            // Item has been de-selected
            pDC->FillRect(&lpDIS->rcItem, &brush);
            const int nRepeat = 32;
            if (nPenStyle > 4) {
                // special dot style
                for (int nLoop = 10 + ((lpDIS->rcItem.right - 2*10) % nRepeat)/2; nLoop < lpDIS->rcItem.right - 10; nLoop += nRepeat) {
                    pDC->SetPixel(nLoop, nPos, colorText);
                }
            } else if (nPenStyle == 2) {
                // special plus style
                for (int nLoop = 10 + ((lpDIS->rcItem.right - 2*10) % nRepeat)/2; nLoop < lpDIS->rcItem.right - 10; nLoop += nRepeat) {
                    pDC->SetPixel(nLoop, nPos, colorText);
                    pDC->SetPixel(nLoop - 1, nPos, colorText);
                    pDC->SetPixel(nLoop + 1, nPos, colorText);
                    pDC->SetPixel(nLoop, nPos + 1, colorText);
                    pDC->SetPixel(nLoop, nPos - 1, colorText);
                }
            } else {
                pDC->MoveTo(10, nPos);
                pDC->LineTo(lpDIS->rcItem.right - 10, nPos);
            }

            pDC->SetBkColor(color); // set back old background
            pDC->SelectObject(pOldPen);
        }
    }
}

//###########################################################################
// CSpinControl dialog
// Little windows 95 style spin controls. Buttons go up and down. The parent
// will be notified by message and will receive a spin control ID with the
// message. The ID has to be delivered to the control after creating it.

IMPLEMENT_DYNCREATE(CSpinControl, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CSpinControl message map

BEGIN_MESSAGE_MAP(CSpinControl, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_TIMER()
    ON_WM_ENABLE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpinControl construction/destruction/creation

/***************************************************************************/
// CSpinControl::CSpinControl Constructor
/***************************************************************************/
CSpinControl::CSpinControl() {
    m_nID = NULL;
    m_bUpButtonDown = FALSE;
    m_bLowButtonDown = FALSE;
    m_bTimerStart = FALSE;
}

/***************************************************************************/
// CSpinControl::~CSpinControl Destructor
/***************************************************************************/
CSpinControl::~CSpinControl() {
}

/////////////////////////////////////////////////////////////////////////////
// CSpinControl helper functions

/***************************************************************************/
// CSpinControl::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CSpinControl::Init(UINT nID, CWnd * pParent) {
    m_nID = nID;
    if (m_nID && pParent) {
        CWnd * pWnd = pParent->GetDlgItem(m_nID);
        CRect rWnd(0, 0, 0, 0);
        Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
        pWnd->GetWindowRect(rWnd);
        pParent->ScreenToClient(rWnd);
        MoveWindow(rWnd, TRUE);
    }
}

/***************************************************************************/
// CSpinControl::DrawArrowUp Drawing up arrow
/***************************************************************************/
void CSpinControl::DrawArrowUp(CDC * pDC, CRect rWnd) {
    CPen penDkgray;
    CPen * pOldPen=NULL;
    if (!IsWindowEnabled()) {
        // window is disabled
        penDkgray.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
        pOldPen = pDC->SelectObject(&penDkgray);
    }
    int nLeft = rWnd.right / 2 - 2;
    int nRight = nLeft + 2;
    pDC->MoveTo(nLeft + 1, rWnd.top + 3); // draw vertical center line
    pDC->LineTo(nLeft + 1, rWnd.bottom - 4);
    for (int nLoop = rWnd.top + 4; nLoop < rWnd.bottom - 3; nLoop++) {
        pDC->MoveTo(nLeft, nLoop); // draw vertical line left from center
        pDC->LineTo(nLeft, rWnd.bottom - 4);
        pDC->MoveTo(nRight, nLoop); // draw vertical line right from center
        pDC->LineTo(nRight, rWnd.bottom - 4);
        nLeft--;
        nRight++;
    }
    if (!IsWindowEnabled()) {
        pDC->SelectObject(pOldPen);
    }
}

/***************************************************************************/
// CSpinControl::DrawArrowDown Drawing down arrow
/***************************************************************************/
void CSpinControl::DrawArrowDown(CDC * pDC, CRect rWnd) {
    CPen penDkgray;
    CPen * pOldPen=NULL;
    if (!IsWindowEnabled()) {
        // window is disabled
        penDkgray.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
        pOldPen = pDC->SelectObject(&penDkgray);
    }
    int nLeft = rWnd.right / 2 - 2;
    int nRight = nLeft + 2;
    pDC->MoveTo(nLeft + 1, rWnd.top + 3); // draw vertical center line
    pDC->LineTo(nLeft + 1, rWnd.bottom - 4);
    for (int nLoop = rWnd.bottom - 6; nLoop > rWnd.top + 2; nLoop--) {
        pDC->MoveTo(nLeft, nLoop); // draw vertical line left from center
        pDC->LineTo(nLeft, rWnd.top + 2);
        pDC->MoveTo(nRight, nLoop); // draw vertical line right from center
        pDC->LineTo(nRight, rWnd.top + 2);
        nLeft--;
        nRight++;
    }
    if (!IsWindowEnabled()) {
        pDC->SelectObject(pOldPen);
    }
}

/***************************************************************************/
// CSpinControl::DrawButtonUp Drawing button up state
/***************************************************************************/
void CSpinControl::DrawButtonUp(CDC * pDC, CRect rWnd) {
    CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    CPen * pOldPen = pDC->SelectObject(&penLtgray);
    pDC->MoveTo(rWnd.left, rWnd.bottom - 1); // paint 3D-frame
    pDC->LineTo(rWnd.left, rWnd.top);
    pDC->LineTo(rWnd.right - 1, rWnd.top);
    pDC->SelectStockObject(BLACK_PEN);
    pDC->LineTo(rWnd.right - 1, rWnd.bottom - 1);
    pDC->LineTo(rWnd.left, rWnd.bottom - 1);
    pDC->SelectObject(&penDkgray);
    pDC->MoveTo(rWnd.left + 1, rWnd.bottom - 2);
    pDC->LineTo(rWnd.right - 2, rWnd.bottom - 2);
    pDC->LineTo(rWnd.right - 2, rWnd.top - 1);
    pDC->SelectObject(pOldPen);
}

/***************************************************************************/
// CSpinControl::DrawButtonDown Drawing button down state
/***************************************************************************/
void CSpinControl::DrawButtonDown(CDC * pDC, CRect rWnd) {
    CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    pDC->SelectStockObject(BLACK_PEN);
    pDC->MoveTo(rWnd.left, rWnd.bottom - 1); // paint 3D-frame
    pDC->LineTo(rWnd.left, rWnd.top);
    pDC->LineTo(rWnd.right - 1, rWnd.top);
    CPen * pOldPen = pDC->SelectObject(&penLtgray);
    pDC->LineTo(rWnd.right - 1, rWnd.bottom - 1);
    pDC->LineTo(rWnd.left, rWnd.bottom - 1);
    pDC->SelectObject(&penDkgray);
    pDC->MoveTo(rWnd.left + 1, rWnd.bottom - 1);
    pDC->LineTo(rWnd.left + 1, rWnd.top + 1);
    pDC->LineTo(rWnd.right - 2, rWnd.top + 1);
    pDC->SelectObject(pOldPen);
}

/***************************************************************************/
// CSpinControl::MouseClick Mouse click test
// This function evaluates which of the two button of the spin control has
// been hit by the mouse pointer and notifies the parent to set actions.
/***************************************************************************/
void CSpinControl::MouseClick(UINT , CPoint point) {
    CWnd * pWnd = GetParent();
    CRect rWnd, rUpper;
    GetClientRect(rWnd);
    rUpper.SetRect(rWnd.left, rWnd.top, rWnd.right, rWnd.bottom / 2);
    if (rUpper.PtInRect(point)) { // mouseclick on upper button
        m_bUpButtonDown = TRUE;
        m_bLowButtonDown = FALSE;
        pWnd->SendMessage(WM_COMMAND, m_nID, 0); // notify parent
    } else {
        m_bUpButtonDown = FALSE;
        m_bLowButtonDown = TRUE;
        pWnd->SendMessage(WM_COMMAND, m_nID, 0); // notify parent
    }
    SetCapture(); // receive all mouse input
    InvalidateRect(NULL, TRUE); // draw buttons, erase background
}

/////////////////////////////////////////////////////////////////////////////
// CSpinControl message handlers

/***************************************************************************/
// CSpinControl::OnPaint Painting
/***************************************************************************/
void CSpinControl::OnPaint() {
    CPaintDC dc(this); // device context for painting
    CRect rWnd, rButton;
    GetClientRect(rWnd);
    // paint upper button
    rButton.SetRect(rWnd.left, rWnd.top, rWnd.right, rWnd.bottom / 2);
    if (m_bUpButtonDown) {
        DrawButtonDown(&dc, rButton);
    } else {
        DrawButtonUp(&dc, rButton);
    }
    DrawArrowUp(&dc, rButton);
    // paint lower button up
    rButton.SetRect(rWnd.left, rWnd.bottom / 2, rWnd.right, rWnd.bottom);
    if (m_bLowButtonDown) {
        DrawButtonDown(&dc, rButton);
    } else {
        DrawButtonUp(&dc, rButton);
    }
    DrawArrowDown(&dc, rButton);
}

/***************************************************************************/
// CSpinControl::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CSpinControl::OnEraseBkgnd(CDC * pDC) {
    CRect rWnd;
    GetClientRect(rWnd);

    // use same background as dialog
    // TCJ 7/00: 32-bit conversion - Make spin control system gray
    CBrush brGray(GetSysColor(COLOR_3DFACE));
    pDC->FillRect(rWnd,&brGray);

    return TRUE;
}

/***************************************************************************/
// CSpinControl::OnLButtonDown Mouse left button down
// Starts a timer to start repeating until the left mouse button is up.
/***************************************************************************/
void CSpinControl::OnLButtonDown(UINT nFlags, CPoint point) {
    MouseClick(nFlags, point);
    WORD nTime;
    SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &nTime, 0);
    nTime = WORD((nTime + 1) * 250); // calculate delay time in ms
    m_bTimerStart = TRUE;
    SetTimer(ID_TIMER_DELAY, nTime, NULL);  // start the mouse repeat timer with the keyboard delay
    CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CSpinControl::OnLButtonDblClk Mouse left button doubleclick
// When the user clicks very fast onto the control, this will be treated as
// doubleclick and the second mouse click will not cause a left button down
// message. Instead this message will arrive and allow the user to click
// fast. Starts a timer to start repeating until the left mouse button is up.
/***************************************************************************/
void CSpinControl::OnLButtonDblClk(UINT nFlags, CPoint point) {
    MouseClick(nFlags, point);
    int nTime;
    SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &nTime, 0);
    nTime = (nTime + 1) * 250; // calculate delay time in ms
    m_bTimerStart = TRUE;
    SetTimer(ID_TIMER_DELAY, nTime, NULL); // start the mouse repeat timer with the keyboard delay
    CWnd::OnLButtonDblClk(nFlags, point);
}

/***************************************************************************/
// CSpinControl::OnLButtonUp Mouse left button up
/***************************************************************************/
void CSpinControl::OnLButtonUp(UINT nFlags, CPoint point) {
    KillTimer(ID_TIMER_DELAY); // kill the mouse repeat timer
    m_bUpButtonDown = FALSE;
    m_bLowButtonDown = FALSE;
    ReleaseCapture();
    InvalidateRect(NULL, TRUE); // draw buttons, erase background
    CWnd::OnLButtonUp(nFlags, point);
}

/***************************************************************************/
// CSpinControl::OnTimer Timer event
// The last left button down mouse hit has to be repeated. If the timer event
// occurs the first time, the time has to be changed from keyboard delay to
// keyboard repeat speed.
/***************************************************************************/
void CSpinControl::OnTimer(UINT nIDEvent) {
    if (m_bTimerStart) {
        KillTimer(ID_TIMER_DELAY);
        m_bTimerStart = FALSE;
        WORD nTime;
        SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &nTime, 0);
        // calculate delay time in ms (formula is near the truth, a table)
        nTime = (WORD)(33.3 + ((float)(31 - nTime) * 0.69688) * ((float)(31 - nTime) * 0.69688));
        SetTimer(ID_TIMER_DELAY, nTime, NULL); // go on with the mouse repeat timer with the keyboard repeat speed
    }
    // repeat the last action if a button pressed
    GetParent()->SendMessage(WM_COMMAND, m_nID, 0); // notify parent
    CWnd::OnTimer(nIDEvent);
}

/***************************************************************************/
// CSpinControl::OnEnable Enable/disable window
/***************************************************************************/
void CSpinControl::OnEnable(BOOL bEnable) {
    Invalidate(); // redraw whole window
    CWnd::OnEnable(bEnable);
}

//###########################################################################
// CToggleButton dialog
// Toggle button, stays down if clicked. Goes up by changing its mode.
// flash mode (if it has LED, it flashes). The parent will be notified by
// message about a left mouse down event and will receive a toggle button
// control ID with the message. The ID has to be delivered to the control
// after creating it.

IMPLEMENT_DYNCREATE(CToggleButton, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CToggleButton message map

BEGIN_MESSAGE_MAP(CToggleButton, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
    ON_WM_ENABLE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToggleButton construction/destruction/creation

/***************************************************************************/
// CToggleButton::CToggleButton Constructor
/***************************************************************************/
CToggleButton::CToggleButton() {
    m_nID = NULL;
    m_bButtonDown = FALSE;
    m_bSymbolFlash = FALSE;
    m_bSymbolOff = FALSE;
    m_szText = "empty";
}

/***************************************************************************/
// CToggleButton::~CToggleButton Destructor
/***************************************************************************/
CToggleButton::~CToggleButton() {
}

/////////////////////////////////////////////////////////////////////////////
// CToggleButton helper functions

/***************************************************************************/
// CToggleButton::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CToggleButton::Init(UINT nID, CString szText, CWnd * pParent) {
    m_nID = nID;
    if (m_nID && pParent) {
        CWnd * pWnd = pParent->GetDlgItem(m_nID);
        CRect rWnd(0, 0, 0, 0);
        Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
        pWnd->GetWindowRect(rWnd);
        pParent->ScreenToClient(rWnd);
        MoveWindow(rWnd, TRUE);
        // set up symbol rectangle
        GetClientRect(m_rSymbol);
        m_rSymbol.left = m_rSymbol.left + (m_rSymbol.Width() - 10) / 2;
        m_rSymbol.right = m_rSymbol.left + 10;
        m_rSymbol.top = m_rSymbol.top + (2 * m_rSymbol.Height() / 3 - 10) / 2 - 1;
        m_rSymbol.bottom = m_rSymbol.top + 10;
        m_szText = szText;
    }
}

/***************************************************************************/
// CToggleButton::DrawButtonUp Drawing button up state
/***************************************************************************/
void CToggleButton::DrawButtonUp(CDC * pDC, CRect rWnd) {
    CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    CPen * pOldPen = pDC->SelectObject(&penLtgray);
    pDC->MoveTo(rWnd.left + 1, rWnd.bottom - 2); // paint 3D-frame
    pDC->LineTo(rWnd.left + 1, rWnd.top + 1);
    pDC->LineTo(rWnd.right - 2, rWnd.top + 1);
    pDC->MoveTo(rWnd.left, rWnd.bottom - 1);
    pDC->LineTo(rWnd.left, rWnd.top);
    pDC->LineTo(rWnd.right - 1, rWnd.top);
    pDC->SelectStockObject(BLACK_PEN);
    pDC->LineTo(rWnd.right - 1, rWnd.bottom - 1);
    pDC->LineTo(rWnd.left - 1, rWnd.bottom - 1);
    pDC->SelectObject(&penDkgray);
    pDC->MoveTo(rWnd.left + 1, rWnd.bottom - 2);
    pDC->LineTo(rWnd.right - 2, rWnd.bottom - 2);
    pDC->LineTo(rWnd.right - 2, rWnd.top + 1);
    pDC->SelectObject(pOldPen);
    DrawSymbol(pDC);
    DrawText(pDC, rWnd);
}

/***************************************************************************/
// CToggleButton::DrawButtonDown Drawing button down state
/***************************************************************************/
void CToggleButton::DrawButtonDown(CDC * pDC, CRect rWnd) {
    CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    pDC->SelectStockObject(BLACK_PEN);
    pDC->MoveTo(rWnd.left, rWnd.bottom - 1); // paint 3D-frame
    pDC->LineTo(rWnd.left, rWnd.top);
    pDC->LineTo(rWnd.right, rWnd.top);
    CPen * pOldPen = pDC->SelectObject(&penLtgray);
    pDC->MoveTo(rWnd.right - 1, rWnd.top + 1);
    pDC->LineTo(rWnd.right - 1, rWnd.bottom - 1);
    pDC->LineTo(rWnd.left, rWnd.bottom - 1);
    pDC->MoveTo(rWnd.right - 2, rWnd.top + 2);
    pDC->LineTo(rWnd.right - 2, rWnd.bottom - 2);
    pDC->LineTo(rWnd.left + 1, rWnd.bottom - 2);
    pDC->SelectObject(&penDkgray);
    pDC->MoveTo(rWnd.left + 1, rWnd.bottom - 2);
    pDC->LineTo(rWnd.left + 1, rWnd.top + 1);
    pDC->LineTo(rWnd.right - 1, rWnd.top + 1);
    pDC->SelectObject(pOldPen);
    DrawSymbol(pDC);
    DrawText(pDC, rWnd);
}

/***************************************************************************/
// CToggleButton::DrawText Drawing button text
/***************************************************************************/
void CToggleButton::DrawText(CDC * pDC, CRect rWnd) {
    // create the font
    CFont font;
    LPCTSTR pszName = _T("MS Sans Serif");
    font.CreateFont(15, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), pszName);
    CFont * pOldFont = pDC->SelectObject(&font);
    // set font color
    if (IsWindowEnabled()) {
        pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
    } else {
        pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
    }
    pDC->SetBkMode(TRANSPARENT);
    rWnd.top = rWnd.Height() / 2 - 1;
    // draw the button text
    pDC->DrawText(m_szText, m_szText.GetLength(), rWnd, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP);
    pDC->SelectObject(pOldFont);  // set back old font
}

/***************************************************************************/
// CToggleButton::DrawSymbol Drawing button symbol
/***************************************************************************/
void CToggleButton::DrawSymbol(CDC * pDC) {
    CBrush brush;
    CRect rWnd;
    if (IsWindowEnabled()) {
        // button is enabled
        CPen * pOldPen;
        CBrush * pOldBrush;
        CPen penDkRed(PS_SOLID, 1, RGB(128, 0, 0));
        CPen penDkGreen(PS_SOLID, 1, RGB(0, 128, 0));
        POINT ptPoly[3];
        switch (m_nID) {
        case IDC_RECORD:
            if (m_bSymbolOff || !m_bButtonDown) {
                brush.CreateSolidBrush(RGB(128, 0, 0)); // dark red
                pOldBrush = pDC->SelectObject(&brush);
            } else {
                brush.CreateSolidBrush(RGB(255, 0, 0)); // light red
                pOldBrush = pDC->SelectObject(&brush);
            }
            pOldPen = pDC->SelectObject(&penDkRed);
            rWnd.SetRect(m_rSymbol.left, m_rSymbol.top, m_rSymbol.right + 1, m_rSymbol.bottom + 1);
            pDC->Ellipse(rWnd);
            pDC->SelectStockObject(WHITE_PEN);
            pDC->MoveTo(rWnd.left + 3, rWnd.top + 2); // paint cross
            pDC->LineTo(rWnd.left + 3, rWnd.top + 5);
            pDC->MoveTo(rWnd.left + 2, rWnd.top + 3);
            pDC->LineTo(rWnd.left + 5, rWnd.top + 3);
            pDC->SelectObject(pOldPen);
            pDC->SelectObject(pOldBrush);
            break;

        case IDC_PLAY:
            if (m_bSymbolOff || !m_bButtonDown) {
                brush.CreateSolidBrush(RGB(0, 128, 0)); // dark green
                pOldBrush = pDC->SelectObject(&brush);
            } else {
                brush.CreateSolidBrush(RGB(0, 255, 0)); // light green
                pOldBrush = pDC->SelectObject(&brush);
            }
            pOldPen = pDC->SelectObject(&penDkGreen);
            ptPoly[0].x = m_rSymbol.left;
            ptPoly[0].y = m_rSymbol.top;
            ptPoly[1].x = m_rSymbol.right;
            ptPoly[1].y = m_rSymbol.top + m_rSymbol.Height() / 2;
            ptPoly[2].x = m_rSymbol.left;
            ptPoly[2].y = m_rSymbol.bottom;
            pDC->Polygon(&ptPoly[0], 3);
            pDC->SelectObject(pOldPen);
            pDC->SelectObject(pOldBrush);
            break;

        case IDC_PAUSE:
            brush.CreateStockObject(BLACK_BRUSH);
            rWnd.SetRect(m_rSymbol.left, m_rSymbol.top, m_rSymbol.left + 4, m_rSymbol.bottom);
            pDC->FillRect(&rWnd, &brush);
            rWnd.left = m_rSymbol.right - 4;
            rWnd.right = rWnd.left + 4;
            pDC->FillRect(&rWnd, &brush);
            break;

        case IDC_STOP:
            brush.CreateStockObject(BLACK_BRUSH);
            pDC->FillRect(&m_rSymbol, &brush);
            break;

        default:
            break;
        }
    } else {
        // button is disabled
        CPen * pOldPen;
        POINT ptPoly[2];
        CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
        CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
        switch (m_nID) {
        case IDC_RECORD:
            ptPoly[0].x = m_rSymbol.left;
            ptPoly[0].y = m_rSymbol.bottom;
            ptPoly[1].x = m_rSymbol.right;
            ptPoly[1].y = m_rSymbol.top;
            pOldPen = pDC->SelectObject(&penLtgray);
            pDC->Arc(m_rSymbol, ptPoly[0], ptPoly[1]);
            pDC->SelectObject(&penDkgray);
            pDC->Arc(m_rSymbol, ptPoly[1], ptPoly[0]);
            pDC->SelectObject(pOldPen);
            break;
        case IDC_PLAY:
            pOldPen = pDC->SelectObject(&penDkgray);
            pDC->MoveTo(m_rSymbol.left, m_rSymbol.bottom);
            pDC->LineTo(m_rSymbol.left, m_rSymbol.top);
            pDC->LineTo(m_rSymbol.right, m_rSymbol.top + m_rSymbol.Height() / 2);
            pDC->SelectObject(&penLtgray);
            pDC->LineTo(m_rSymbol.left, m_rSymbol.bottom);
            pDC->SelectObject(pOldPen);
            break;
        case IDC_PAUSE:
            rWnd.SetRect(m_rSymbol.left, m_rSymbol.top, m_rSymbol.left + 4, m_rSymbol.bottom);
            pOldPen = pDC->SelectObject(&penLtgray);
            pDC->MoveTo(rWnd.right, rWnd.top);
            pDC->LineTo(rWnd.right, rWnd.bottom);
            pDC->LineTo(rWnd.left, rWnd.bottom);
            pDC->SelectObject(&penDkgray);
            pDC->LineTo(rWnd.left, rWnd.top);
            pDC->LineTo(rWnd.right, rWnd.top);
            rWnd.left = m_rSymbol.right - 4;
            rWnd.right = rWnd.left + 4;
            pDC->SelectObject(&penLtgray);
            pDC->MoveTo(rWnd.right, rWnd.top);
            pDC->LineTo(rWnd.right, rWnd.bottom);
            pDC->LineTo(rWnd.left, rWnd.bottom);
            pDC->SelectObject(&penDkgray);
            pDC->LineTo(rWnd.left, rWnd.top);
            pDC->LineTo(rWnd.right, rWnd.top);
            pDC->SelectObject(pOldPen);
            break;
        case IDC_STOP:
            pOldPen = pDC->SelectObject(&penLtgray);
            pDC->MoveTo(m_rSymbol.right, m_rSymbol.top);
            pDC->LineTo(m_rSymbol.right, m_rSymbol.bottom);
            pDC->LineTo(m_rSymbol.left, m_rSymbol.bottom);
            pDC->SelectObject(&penDkgray);
            pDC->LineTo(m_rSymbol.left, m_rSymbol.top);
            pDC->LineTo(m_rSymbol.right, m_rSymbol.top);
            pDC->SelectObject(pOldPen);
            break;
        default:
            break;
        }
    }

}

/***************************************************************************/
// CToggleButton::Flash Starts/stops symbol flashing
/***************************************************************************/
void CToggleButton::Flash(BOOL bFlash) {

    if (!m_bSymbolFlash) {
        if (bFlash) {
            m_bSymbolFlash = TRUE;
            SetTimer(ID_TIMER_DELAY, 500, NULL);             // start the mouse repeat timer with the keyboard delay
        }
    } else {
        if (!bFlash) {
            m_bSymbolFlash = FALSE;
            m_bSymbolOff = FALSE;
            KillTimer(ID_TIMER_DELAY);
            InvalidateRect(m_rSymbol, FALSE);   // redraw the symbol, don't erase the background
        }
    }
}

/***************************************************************************/
// CToggleButton::Push Release pressed button
/***************************************************************************/
void CToggleButton::Push() {
    if (!m_bButtonDown) {
        m_bButtonDown = TRUE;
        Flash(FALSE);
        InvalidateRect(NULL, TRUE); // draw button, erase background
    }
}

/***************************************************************************/
// CToggleButton::Release Release pressed button
/***************************************************************************/
void CToggleButton::Release() {
    if (m_bButtonDown) {
        m_bButtonDown = FALSE;
        Flash(FALSE);
        InvalidateRect(NULL, TRUE); // draw button, erase background
    }
}

/////////////////////////////////////////////////////////////////////////////
// CToggleButton message handlers

/***************************************************************************/
// CToggleButton::OnPaint Painting
/***************************************************************************/
void CToggleButton::OnPaint() {
    CPaintDC dc(this); // device context for painting
    CRect rWnd, rButton;
    GetClientRect(rWnd);
    // paint button
    if (m_bButtonDown) {
        DrawButtonDown(&dc, rWnd);
    } else {
        DrawButtonUp(&dc, rWnd);
    }
}

/***************************************************************************/
// CToggleButton::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CToggleButton::OnEraseBkgnd(CDC * pDC) {
    CRect rWnd;
    GetClientRect(rWnd);
    // use same background as dialog
    // TCJ 7/00: 32-bit conversion - Make buttons system gray
    CBrush brGray(GetSysColor(COLOR_3DFACE));
    pDC->FillRect(rWnd,&brGray);

    return TRUE;
}

/***************************************************************************/
// CToggleButton::OnLButtonDown Mouse left button down
/***************************************************************************/
void CToggleButton::OnLButtonDown(UINT nFlags, CPoint point) {
    if (IsWindowEnabled()) {
        if (!m_bButtonDown) {
            m_bButtonDown = TRUE;
            InvalidateRect(NULL, TRUE); // draw button, erase background
            SetCapture(); // receive all mouse input
        }
    }
    CWnd::OnLButtonDown(nFlags, point);
}

/***************************************************************************/
// CToggleButton::OnLButtonDblClk Mouse left button doubleclick
// When the user clicks very fast onto the control, this will be treated as
// doubleclick and the second mouse click will not cause a left button down
// message. Instead this message will arrive and allow the user to click
// fast.
/***************************************************************************/
void CToggleButton::OnLButtonDblClk(UINT nFlags, CPoint point) {
    if (IsWindowEnabled()) {
        if (!m_bButtonDown) {
            m_bButtonDown = TRUE;
            InvalidateRect(NULL, TRUE); // draw button, erase background
            SetCapture(); // receive all mouse input
        }
    }
    CWnd::OnLButtonDblClk(nFlags, point);
}

/***************************************************************************/
// CToggleButton::OnLButtonUp Mouse left button up
/***************************************************************************/
void CToggleButton::OnLButtonUp(UINT nFlags, CPoint point) {
    if (IsWindowEnabled()) {
        // inform parent
        CWnd * pWnd = GetParent();
        pWnd->SendMessage(WM_COMMAND, m_nID, 0); // notify parent
    }
    ReleaseCapture();
    CWnd::OnLButtonUp(nFlags, point);
}

/***************************************************************************/
// CToggleButton::OnTimer Timer event, redraw the button symbol
/***************************************************************************/
void CToggleButton::OnTimer(UINT nIDEvent) {
    m_bSymbolOff = !m_bSymbolOff;
    InvalidateRect(m_rSymbol, FALSE); // redraw the symbol, don't erase the background
    CWnd::OnTimer(nIDEvent);
}

/***************************************************************************/
// CToggleButton::OnEnable Enable/disable window
/***************************************************************************/
void CToggleButton::OnEnable(BOOL bEnable) {
    Invalidate(); // redraw whole window
    CWnd::OnEnable(bEnable);
}

//###########################################################################
// CTranscriptionDisp
// Special window with 3D border to display wave file transcription.

// SDM 1.06.6r1 changed to use underlying static text control
/////////////////////////////////////////////////////////////////////////////
// CTranscriptionDisp helper functions

/***************************************************************************/
// CTranscriptionDisp::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CTranscriptionDisp::Init(UINT nID, CWnd * pParent) {
    if (nID && pParent) {
        //SDM 1.06.6r1
        CWnd * pWnd = pParent->GetDlgItem(nID);

        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
        ASSERT(pView->IsKindOf(RUNTIME_CLASS(CSaView)));
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        CString content = pDoc->GetSegment(PHONETIC)->GetContent();
        pWnd->SetWindowText(content);
        CFont * pFont = pDoc->GetFont(PHONETIC);
        pWnd->SetFont(pFont);
    }
}

//###########################################################################
// CAboutTitle
// Special window painting the programs title.

IMPLEMENT_DYNCREATE(CAboutTitle, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CAboutTitle message map

BEGIN_MESSAGE_MAP(CAboutTitle, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutTitle construction/destruction/creation

/***************************************************************************/
// CAboutTitle::CAboutTitle Constructor
/***************************************************************************/
CAboutTitle::CAboutTitle() {
}

/***************************************************************************/
// CAboutTitle::~CAboutTitle Destructor
/***************************************************************************/
CAboutTitle::~CAboutTitle() {
}

/////////////////////////////////////////////////////////////////////////////
// CAboutTitle helper functions

/***************************************************************************/
// CAboutTitle::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CAboutTitle::Init(UINT nID, CWnd * pParent) {
    if (nID && pParent) {
        CWnd * pWnd = pParent->GetDlgItem(nID);
        CRect rWnd(0, 0, 0, 0);
        Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
        pWnd->GetWindowRect(rWnd);
        pParent->ScreenToClient(rWnd);
        MoveWindow(rWnd, TRUE);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CAboutTitle message handlers

/***************************************************************************/
// CAboutTitle::OnPaint Painting
/***************************************************************************/
void CAboutTitle::OnPaint() {
    CPaintDC dc(this); // device context for painting
    // create and select title font
    CFont font;
    font.CreateFont(48, 0, 0, 0, FW_BOLD, 1, 0, 0,
                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), _T("Times New Roman"));
    CFont * pOldFont = dc.SelectObject(&font);
    dc.SetBkMode(TRANSPARENT); // letters overlap, so they must be transparent
    // get the title text
    CString szText;
    szText.LoadString(IDR_MAINFRAME);
    // get window coordinates
    CRect rWnd;
    GetClientRect(rWnd);
    rWnd.left +=4; // to adjust to center
    // draw the text white
    dc.SetTextColor(RGB(255, 255, 255)); // set first color to white
    dc.DrawText(szText, szText.GetLength(), rWnd, DT_SINGLELINE | DT_CENTER | DT_TOP | DT_NOCLIP);
    rWnd.right -= 4;
    // draw the text dark gray
    dc.SetTextColor(RGB(128, 128, 128)); // set first color to white
    dc.DrawText(szText, szText.GetLength(), rWnd, DT_SINGLELINE | DT_CENTER | DT_TOP | DT_NOCLIP);
    rWnd.right -= 4;
    // draw the text blue
    dc.SetTextColor(RGB(0, 0, 128)); // set first color to white
    dc.DrawText(szText, szText.GetLength(), rWnd, DT_SINGLELINE | DT_CENTER | DT_TOP | DT_NOCLIP);
    dc.SelectObject(pOldFont);  // allows for proper restoration
}

/***************************************************************************/
// CAboutTitle::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CAboutTitle::OnEraseBkgnd(CDC * pDC) {
    CRect rWnd;
    GetClientRect(rWnd);

    // use same background as dialog
    // TCJ 7/00: 32-bit conversion - Make CSliderVertical system gray
    CBrush brGray(GetSysColor(COLOR_3DFACE));
    pDC->FillRect(rWnd,&brGray);

    return CWnd::OnEraseBkgnd(pDC);
}

//###########################################################################
// CStaticText
// Special window for drawing static text with dialog background color.

IMPLEMENT_DYNCREATE(CStaticText, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CStaticText message map

BEGIN_MESSAGE_MAP(CStaticText, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticText construction/destruction/creation

/***************************************************************************/
// CStaticText::CStaticText Constructor
/***************************************************************************/
CStaticText::CStaticText() {
    m_nID = 0;
}

/***************************************************************************/
// CStaticText::~CStaticText Destructor
/***************************************************************************/
CStaticText::~CStaticText() {
}

/////////////////////////////////////////////////////////////////////////////
// CStaticText helper functions

/***************************************************************************/
// CStaticText::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CStaticText::Init(UINT nID, CWnd * pParent) {
    m_nID = nID;
    if (m_nID && pParent) {
        CWnd * pWnd = pParent->GetDlgItem(m_nID);
        CRect rWnd(0, 0, 0, 0);
        Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
        pWnd->GetWindowRect(rWnd);
        pParent->ScreenToClient(rWnd);
        MoveWindow(rWnd, TRUE);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CStaticText message handlers

/***************************************************************************/
// CStaticText::OnPaint Painting
/***************************************************************************/
void CStaticText::OnPaint() {
    CPaintDC dc(this); // device context for painting
    // create and select title font
    if (m_nID) {
        CWnd * pWnd = GetParent()->GetDlgItem(m_nID);
        CFont * pOldFont = dc.SelectObject(pWnd->GetFont()); // select actual font
        dc.SetBkMode(TRANSPARENT);
        // get the static text
        CString szText;
        pWnd->GetWindowText(szText);
        // get window coordinates
        CRect rWnd;
        GetClientRect(rWnd);
        // draw the text
        dc.DrawText(szText, szText.GetLength(), rWnd, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOCLIP);
        dc.SelectObject(pOldFont); // allows for proper restoration
    }
}

/***************************************************************************/
// CStaticText::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CStaticText::OnEraseBkgnd(CDC * pDC) {
    // use same background as dialog
    CRect rWnd;
    GetClientRect(rWnd);

    // TCJ 7/00: 32-bit conversion - Make CStaticText system gray
    CBrush brGray(GetSysColor(COLOR_3DFACE));
    pDC->FillRect(rWnd,&brGray);

    return CWnd::OnEraseBkgnd(pDC);
}

//###########################################################################
// CFancyArrow
// Special window painting the programs title.

IMPLEMENT_DYNCREATE(CFancyArrow, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CFancyArrow message map

BEGIN_MESSAGE_MAP(CFancyArrow, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFancyArrow construction/destruction/creation

/***************************************************************************/
// CFancyArrow::CFancyArrow Constructor
/***************************************************************************/
CFancyArrow::CFancyArrow() {
}

/***************************************************************************/
// CFancyArrow::~CFancyArrow Destructor
/***************************************************************************/
CFancyArrow::~CFancyArrow() {
}

/////////////////////////////////////////////////////////////////////////////
// CStaticText helper functions

/***************************************************************************/
// CFancyArrow::Init Initialisation
// Creates the window and places it over the dialog control.
/***************************************************************************/
void CFancyArrow::Init(UINT nID, CWnd * pParent) {
    if (nID && pParent) {
        CWnd * pWnd = pParent->GetDlgItem(nID);
        CRect rWnd(0, 0, 0, 0);
        Create(NULL, NULL, WS_CHILD|WS_VISIBLE, rWnd, pParent, 1);
        pWnd->GetWindowRect(rWnd);
        pParent->ScreenToClient(rWnd);
        MoveWindow(rWnd, TRUE);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CFancyArrow message handlers

/***************************************************************************/
// CFancyArrow::OnPaint Painting
/***************************************************************************/
void CFancyArrow::OnPaint() {
    CPaintDC dc(this); // device context for painting
    // get window coordinates
    CRect rWnd;
    GetClientRect(rWnd);
    // paint 3D-arrow
    CPen penDkgray(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    CPen penLtgray(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    CPen * pOldPen = dc.SelectObject(&penDkgray);
    dc.MoveTo(rWnd.right, rWnd.bottom / 2);
    dc.LineTo(rWnd.right - rWnd.Height(), rWnd.bottom);
    dc.LineTo(rWnd.right - rWnd.Height(), rWnd.bottom - rWnd.Height() / 4);
    dc.LineTo(rWnd.left, rWnd.bottom - rWnd.Height() / 4);
    dc.SelectObject(&penLtgray);
    dc.LineTo(rWnd.left, rWnd.top + rWnd.Height() / 4);
    dc.LineTo(rWnd.right - rWnd.Height(), rWnd.top + rWnd.Height() / 4);
    dc.LineTo(rWnd.right - rWnd.Height(), rWnd.top);
    dc.LineTo(rWnd.right, rWnd.bottom / 2);
    dc.SelectObject(pOldPen);
}

/***************************************************************************/
// CFancyArrow::OnEraseBkgnd Erasing background
/***************************************************************************/
BOOL CFancyArrow::OnEraseBkgnd(CDC * pDC) {
    // use same background as dialog
    CRect rWnd;
    GetClientRect(rWnd);

    // TCJ 7/00: 32-bit conversion - Make CFancyArrow system gray
    CBrush brGray(GetSysColor(COLOR_3DFACE));
    pDC->FillRect(rWnd,&brGray);

    return CWnd::OnEraseBkgnd(pDC);
}

int CSpinControl::GetID() {
    return m_nID;   // get spin control ID
}

BOOL CSpinControl::UpperButtonClicked() {
    return m_bUpButtonDown;   // return TRUE if upper button hit
}
