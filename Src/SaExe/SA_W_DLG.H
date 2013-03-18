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
#define _SA_W_DLG_H

#include "CSaString.h"

class CWbDlgProcesses : public CDialog {

public:
    CWbDlgProcesses(CWnd * pParent = NULL); // standard constructor
	void OnHelpWorkbench();

	int m_nProcess; // active process
	CProcess * m_pWbProcessFilter1;
	CProcess * m_pWbProcessFilter2;
	CProcess * m_pWbProcessFilter3;
	enum { IDD = IDD_WB_PROCESSES };
	int m_nFilter1;
	int m_nFilter2;
	int m_nFilter3;

protected:
	virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnProperties1();
	afx_msg void OnProperties2();
	afx_msg void OnProperties3();
	afx_msg void OnSelchangeFilter1();
	afx_msg void OnSelchangeFilter2();
	afx_msg void OnSelchangeFilter3();
	virtual void OnCancel();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()

private:
    int m_nLocalFilter1;
    int m_nLocalFilter2;
    int m_nLocalFilter3;

};

class CWbDlgFilterPass : public CDialog {
public:
    CWbDlgFilterPass(CWnd * pParent = NULL); // standard constructor
	void OnHelpWorkbench();

	BOOL m_bBandPass; // TRUE if bandpass properties
	BOOL m_bLoPass;   // TRUE if lowpass properties
	enum { IDD = IDD_WB_FILTERPASS };
	UINT    m_nLowerFreq;
	UINT    m_nUpperFreq;
	UINT    m_nOrder;
	BOOL    m_bFilterFilter;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    virtual BOOL OnInitDialog();
    afx_msg void OnUpperFreqScroll();
    afx_msg void OnKillfocusUpperfreq();
    afx_msg void OnLowerFreqScroll();
    afx_msg void OnKillfocusLowerfreq();
    afx_msg void OnOrderScroll();
    afx_msg void OnKillfocusOrder();
    DECLARE_MESSAGE_MAP()

private:
	CSpinControl m_SpinUpperFreq;
	CSpinControl m_SpinLowerFreq;
	CSpinControl m_SpinOrder;

};

class CWbDlgFilterReverb : public CDialog {

public:
    CWbDlgFilterReverb(CWnd * pParent = NULL);  // standard constructor

	void OnHelpWorkbench();

	enum { IDD = IDD_WB_FILTERREVERB };
	UINT    m_nDelay;
	int     m_nGain;
	BOOL m_bEcho; // TRUE, if dialog is echo properties dialog

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    virtual BOOL OnInitDialog();
    afx_msg void OnDelayScroll();
    afx_msg void OnKillfocusDelay();
    afx_msg void OnGainScroll();
    afx_msg void OnKillfocusGain();
    DECLARE_MESSAGE_MAP()

private:
	CSpinControl m_SpinDelay;
	CSpinControl m_SpinGain;
};

class CWbDlgFilterEquation : public CDialog {

public:
    CWbDlgFilterEquation(CWnd * pParent = NULL);		// standard constructor
	void OnHelpWorkbench();

	enum { IDD = IDD_WB_FILTEREQUATION };
	CSaString m_szEquation;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual void OnOK();

	DECLARE_MESSAGE_MAP()

private:
	CFont m_Font;                        // special font for dialog controls

};

#endif
