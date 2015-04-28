/////////////////////////////////////////////////////////////////////////////
// ch_dlwnd.h:
// Interface of the CChartText (dialog child window)
//           CChartLine (dialog child window)
//          CChartChar (dialog child window) classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _CH_DLWND_H_
#define _CH_DLWND_H_

//###########################################################################
// CChartText dialog child window

// CChartText defines

// display mode, default is 0
#define NO_LINES    0x0000  // dialog background, no lines, centered
#define HORZTOP_LINE   0x0001  // displays a horizontal line on top of window
#define HORZBOTTOM_LINE  0x0002  // displays a horizontal line on bottom of window
#define VERTLEFT_LINE  0x0004  // displays a vertical line on left side of window
#define VERTRIGHT_LINE  0x0008  // displays a vertical line on right side of window
#define OUTLINE      0x000F  // displays window borders
#define HORZTOP_BOLD   0x0010  // displays the horizontal line on top of window bold (if there)
#define HORZBOTTOM_BOLD  0x0020  // displays the horizontal line on bottom of window bold (if there)
#define VERTLEFT_BOLD  0x0040  // displays the vertical line on left side of window bold (if there)
#define VERTRIGHT_BOLD  0x0080  // displays the vertical line on right side of window bold (if there)
#define OUTLINE_BOLD  0x00F0  // displays window borders bold
#define BCK_TRANSPARENT  0x0100  // displays no background (transparent)
#define BCK_DARK    0x0200  // displays a dark background
#define TEXT_TOP       0x1000  // displays text on window top
#define TEXT_BOTTOM     0x2000  // displays text on window bottom
#define TEXT_LEFT     0x4000  // displays text left aligned
#define TEXT_RIGHT      0x8000  // displays text right aligned

class CChartText : public CWnd {
    DECLARE_DYNCREATE(CChartText)

// Construction/destruction/creation
public:
    CChartText();
    ~CChartText();

    // Attributes
private:
    UINT    m_nID;    // window ID
    int     m_nMode;  // display mode
    CString m_szText; // text to display
    CFont * m_pFont;  // pointer to font for text

    // Operations
public:
    void Init(UINT nID, int nMode, CFont * pFont, CWnd * pParent);

    // Generated message map functions
protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CChartLine dialog child window

// CChartLine defines
#define DOT_SIZE 6 // dot size in pixel

// display mode, default is 0
#define NORMAL      0x0000  // line DIAG_LR, no dots
#define DIAG_LR     0x0001  // displays a diagonal line from left top to right bot.
#define DIAG_RL     0x0002  // displays a diagonal line from left bottom to right top
#define LINE_TOP     0x0004  // displays a horizontal line on top of window
#define LINE_LEFT     0x0008  // displays a vertical line on left side of window
#define END_DOTS     0x0010  // displays dots on ends of line
#define START_DOT    0x0020  // displays a dot at the beginning of the line

class CChartLine : public CWnd {
    DECLARE_DYNCREATE(CChartLine)

// Construction/destruction/creation
public:
    CChartLine();
    ~CChartLine();

    // Attributes
private:
    int  m_nMode;

    // Operations
public:
    void Init(UINT nID, int nMode, CWnd * pParent);

    // Generated message map functions
protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CChartChar dialog child window

// CChartChar defines
#define ZOOM_DELAY       0         // zoom delay in milliseconds
#define SOUND_DELAY      250       // sound delay in milliseconds
#define WM_USER_CHARSELECT  WM_USER + 31 // wide range message sent

enum ECharType {         // character types
    VOWEL=0,
    CONSONANT,
    DIACRITIC,
    SUPRASEG,
};

// display mode, default is 0
#define ICH_NORMAL      0x0000  // no example character, enabled, with zoom
#define ICH_GRAYED       0x0001  // displays the character grayed (no selection, no zoom)
#define ICH_SELECTED     0x0002  // displays the character selected (invers)
#define ICH_NO_ZOOM     0x0010  // disables zoom on mouse move
#define ICH_DISABLED    0x0020  // cannot be selected
#define ICH_BUBBLE       0x8000  // this window is a bubble window (zoom)

class CDlgCharChart;
class CChartChar : public CWnd {
    DECLARE_DYNCREATE(CChartChar)

// Construction/destruction/creation
public:
    CChartChar();
    ~CChartChar();

    // Attributes
private:
    CString  m_szChar;   // character(s) code to display
    CString  m_szSoundFile; // part of name of WAV file to play
    UINT   m_nID;      // window ID
    int    m_nType;    // character type
    int    m_nMode;    // display mode
    CFont  * m_pFont;    // pointer to character font
    CChartChar * m_pCreator; // creator window (only needed for bubble (zoom), normally NULL))
    CWnd * m_pParent;  // parent window
    CDlgCharChart * m_pCaller;  // window that wants to receive the messages
    BOOL  m_bReadyToSelect;
    CChartChar * m_pBubble;  // pointer to bubble (zoom) window
    enum {kStateIdle, kStateBegin, kStateWord, kStateSegment, kStateSound, kStateDone };
    int m_nPlayState;


    // Operations
public:
    void Setup(CString * pszChar, CString * pszSoundFile, UINT nID, int nType, int nMode, CFont * pFont, CWnd * pParent, CDlgCharChart * pCaller, CChartChar * pCreator = NULL);
    void Init();
    void ChangeMode(int nMode);
    CString * GetChar() {
        return &m_szChar;   // return pointer to character string
    }
    UINT GetID() {
        return m_nID;   // return character ID
    }
    int  GetType() {
        return m_nType;   // return character type
    }
    int  GetMode() {
        return m_nMode;   // return character mode
    }

    // Generated message map functions
protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    DECLARE_MESSAGE_MAP()
};

#endif //_CH_DLWND_H_
