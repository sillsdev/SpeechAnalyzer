/////////////////////////////////////////////////////////////////////////////
// sa_sbar.h:
// Interface of the CSaDataStatusBar
//           CSaProgressStatusBar classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_SBAR_H

#include "sa_dlWnd.h"

#define _SA_SBAR_H

//###########################################################################
// CDataStatusBar window

/////////////////////////////////////////////////////////////////////////////
// CDataStatusBar defines

#define SB_FONT_SIZE  -10 // status bar font size

class CDataStatusBar : public CStatusBar
{

  // Construction/destruction/creation
public:
  CDataStatusBar();
  virtual ~CDataStatusBar();

  // Attributes
private:
  CFont* m_pFont;                   // status bar font
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

  // Operations
public:
  void Init(); // initialisation
  void DoPaint(CDC* pdc); // overridden for sa statusbar
  void SetPaneSymbol(int nPaneID, BOOL bShow = TRUE); // show or hide pane symbols
  BOOL SetPaneText(int nPaneID, LPCTSTR lpszText, BOOL bUpdate = TRUE);
  void GetItemRect(int nPaneID, LPRECT lpRect);

  // Generated message map functions
protected:
  //{{AFX_MSG(CDataStatusBar)
  afx_msg void OnPaint();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CProgressStatusBar window

/////////////////////////////////////////////////////////////////////////////
// CProgressStatusBar defines

#define SB_FONT_SIZE  -10 // status bar font size

class CProgressStatusBar : public CStatusBar
{
  // Construction/destruction/creation
public:
  CProgressStatusBar();
  virtual ~CProgressStatusBar();

  // Attributes
private:
  CFont* m_pFont;             // status bar font
  void* m_pProcessOwner;    // process owner
  void* m_pProcessCaller;    // process caller
  CProgressBar m_ProgressBar; // progress bar object
  //CProgressCancelDlg* m_pdlgCancel;
  BOOL m_bIsPrinting;
  int m_nInMessageLoop;

  // Operations
protected:
  void MessageLoop(); // do windows message loop
public:
  void Init(); // initialisation
  void SetProcessOwner(void* pProcess, void* pCaller, int nProcessID = -1); // save the process owner
  void* GetProcessOwner() {return m_pProcessOwner;} // return the process owner
  void* GetProcessCaller() {return m_pProcessCaller;} // return the process caller
  void InitProgress(); // initialisation of progress bar
  void SetProgress(int nVal); // set progress bar
  int  GetProgress() {return m_ProgressBar.GetProgress();} // get progress
  BOOL SetPaneText(int nPaneID, LPCTSTR lpszText, BOOL bUpdate = TRUE);
  void GetItemRect(int nPaneID, LPRECT lpRect);
  void SetIsPrintingFlag(BOOL isPrinting) { m_bIsPrinting = isPrinting; };
  virtual void DelayShow();
  BOOL InProcessMessageLoop() const {return m_nInMessageLoop != 0;}


  // Related to Cancel button.


  // Generated message map functions
protected:
  //{{AFX_MSG(CProgressStatusBar)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#endif //_SA_SBAR_H
