/////////////////////////////////////////////////////////////////////////////
// sa_w_dlg.h:
// Interface of the CWbDlgProcesses (dialog)
//                  CWbDlgFilterPass (dialog)
//                  CWbDlgFilterReverb (dialog)
//                  CWbDlgFilterEquation (dialog) classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_DLG_H

#include "CSaString.h"

#define _SA_W_DLG_H

//###########################################################################
// CWbDlgProcesses dialog

class CWbDlgProcesses : public CDialog
{

  // Construction/destruction/creation
public:
  CWbDlgProcesses(CWnd* pParent = NULL); // standard constructor

  // Attributes
private:
  int      m_nLocalFilter1;
  int      m_nLocalFilter2;
  int      m_nLocalFilter3;
public:
  int m_nProcess; // active process
  CDataProcess* m_pWbProcessFilter1;
  CDataProcess* m_pWbProcessFilter2;
  CDataProcess* m_pWbProcessFilter3;
	void OnHelpWorkbench();

  // Dialog Data
  //{{AFX_DATA(CWbDlgProcesses)
  enum { IDD = IDD_WB_PROCESSES };
  int     m_nFilter1;
  int     m_nFilter2;
  int     m_nFilter3;
  //}}AFX_DATA

  // Operations
protected:
  virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

  // Generated message map functions
  //{{AFX_MSG(CWbDlgProcesses)
  virtual BOOL OnInitDialog();
  afx_msg void OnProperties1();
  afx_msg void OnProperties2();
  afx_msg void OnProperties3();
  afx_msg void OnSelchangeFilter1();
  afx_msg void OnSelchangeFilter2();
  afx_msg void OnSelchangeFilter3();
  virtual void OnCancel();
  afx_msg void OnClose();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CWbDlgFilterPass dialog

class CWbDlgFilterPass : public CDialog
{

  // Construction/destruction/creation
public:
  CWbDlgFilterPass(CWnd* pParent = NULL); // standard constructor

  // Attributes
private:
  CSpinControl m_SpinUpperFreq;
  CSpinControl m_SpinLowerFreq;
  CSpinControl m_SpinOrder;
public:
  BOOL m_bBandPass; // TRUE if bandpass properties
  BOOL m_bLoPass;   // TRUE if lowpass properties

  // Dialog Data
  //{{AFX_DATA(CWbDlgFilterPass)
	enum { IDD = IDD_WB_FILTERPASS };
  UINT    m_nLowerFreq;
  UINT    m_nUpperFreq;
	UINT	m_nOrder;
	BOOL	m_bFilterFilter;
	//}}AFX_DATA

  // Operations
public:
	void OnHelpWorkbench();
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  // Generated message map functions
  //{{AFX_MSG(CWbDlgFilterPass)
  virtual BOOL OnInitDialog();
  afx_msg void OnUpperFreqScroll();
  afx_msg void OnKillfocusUpperfreq();
  afx_msg void OnLowerFreqScroll();
  afx_msg void OnKillfocusLowerfreq();
  afx_msg void OnOrderScroll();
  afx_msg void OnKillfocusOrder();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CWbDlgFilterReverb dialog

class CWbDlgFilterReverb : public CDialog
{

  // Construction/destruction/creation
public:
  CWbDlgFilterReverb(CWnd* pParent = NULL);   // standard constructor

  // Attributes
private:
  CSpinControl m_SpinDelay;
  CSpinControl m_SpinGain;
public:
  // Dialog Data
  //{{AFX_DATA(CWbDlgFilterReverb)
  enum { IDD = IDD_WB_FILTERREVERB };
  UINT    m_nDelay;
  int     m_nGain;
  //}}AFX_DATA
  BOOL m_bEcho; // TRUE, if dialog is echo properties dialog

  // Operations
public:
	void OnHelpWorkbench();
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  // Generated message map functions
  //{{AFX_MSG(CWbDlgFilterReverb)
  virtual BOOL OnInitDialog();
  afx_msg void OnDelayScroll();
  afx_msg void OnKillfocusDelay();
  afx_msg void OnGainScroll();
  afx_msg void OnKillfocusGain();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CWbDlgFilterEquation dialog

class CWbDlgFilterEquation : public CDialog
{

  // Construction/destruction/creation
public:
  CWbDlgFilterEquation(CWnd* pParent = NULL); // standard constructor

  // Attributes
private:
  CFont m_Font;                        // special font for dialog controls
public:
  // Dialog Data
  //{{AFX_DATA(CWbDlgFilterEquation)
  enum { IDD = IDD_WB_FILTEREQUATION };
  CSaString m_szEquation;
  //}}AFX_DATA

  // Operations
public:
	void OnHelpWorkbench();
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  // Generated message map functions
  //{{AFX_MSG(CWbDlgFilterEquation)
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#endif //_SA_W_DLG_H
