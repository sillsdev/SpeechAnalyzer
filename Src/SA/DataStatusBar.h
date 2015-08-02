/////////////////////////////////////////////////////////////////////////////
// DataStatusBar.h:
// Interface of the CDataStatusBar classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef DATASTATUSBAR_H
#define DATASTATUSBAR_H

#include "sa_dlWnd.h"

//###########################################################################
// CDataStatusBar window

/////////////////////////////////////////////////////////////////////////////
// CDataStatusBar defines

class CDataStatusBar : public CStatusBar {

    // Construction/destruction/creation
public:
    CDataStatusBar();
    virtual ~CDataStatusBar();

    // Attributes
private:
    CFont * m_pFont;                  // status bar font
    BOOL m_bPositionSymbol;           // TRUE, if the graphic symbol for the position pane is visible
    BOOL m_bLengthSymbol;             // TRUE, if the graphic symbol for the length pane is visible
    BOOL m_bAmplitudeSymbol;          // TRUE, if the graphic symbol for the amplitude pane is visible
    BOOL m_bScaleSymbol;              // TRUE, if the graphic symbol for the scale pane is visible
    BOOL m_bPitchSymbol;              // TRUE, if the graphic symbol for the pitch pane is visible
    BOOL m_bFrequencySymbol;          // TRUE, if the graphic symbol for the frequency pane is visible
    BOOL m_bNoteSymbol;               // TRUE, if the graphic symbol for the note pane is visible
    BOOL m_bRawSpectrumSymbol;        // TRUE, if the graphic symbol for the raw spectrum pane is visible
    BOOL m_bCepstralSpectrumSymbol;   // TRUE, if the graphic symbol for the cepstrally-smoothed spectrum pane is visible
    BOOL m_bLpcSpectrumSymbol;        // TRUE, if the graphic symbol for the LPC spectrum pane is visible

public:
    void Init(); // initialisation
    void DoPaint(CDC * pdc); // overridden for sa statusbar
    void SetPaneSymbol(int nPaneID, BOOL bShow = TRUE); // show or hide pane symbols
    BOOL SetPaneText(int nPaneID, LPCTSTR lpszText, BOOL bUpdate = TRUE);
    void GetItemRect(int nPaneID, LPRECT lpRect);

    // Generated message map functions
protected:
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};

#endif
