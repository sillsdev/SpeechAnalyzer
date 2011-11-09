/////////////////////////////////////////////////////////////////////////////
// advancedParameters.h:
// Interface of the CDlgAdvancedParsePage (property page)
//                  CDlgAdvancedSegmentPage (property page)
//                  CDlgAdvanced (property sheet)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg.h
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _ADVANCED_PARAMETERS_H

#include "sa_dlwnd.h"
#include "resource.h"

#define _ADVANCED_PARAMETERS_H


//###########################################################################
// CDlgAdvancedParsePage property page

class CDlgAdvancedParsePage : public CPropertyPage
{

	// Construction/destruction/creation
public:
	CDlgAdvancedParsePage(); // standard constructor

	// Attributes
private:
	CSpinControl m_SpinBreak;
	CSpinControl m_SpinMaxThreshold;
	CSpinControl m_SpinMinThreshold;
	BOOL         m_bModified; // TRUE if page member data modified

	// Dialog Data
	//{{AFX_DATA(CDlgAdvancedParsePage)
	enum { IDD = IDD_ADVANCEDPARSEPAGE };
	int     m_nBreakWidth;
	int     m_nMaxThreshold;
	int     m_nMinThreshold;
	//}}AFX_DATA

	// Operations
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
public:
	void Apply();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDlgAdvancedParsePage)
	virtual BOOL OnInitDialog();
	afx_msg void OnBreakWidthScroll();
	afx_msg void OnKillfocusBreakWidth();
	afx_msg void OnMaxThresholdScroll();
	afx_msg void OnKillfocusMaxThreshold();
	afx_msg void OnMinThresholdScroll();
	afx_msg void OnKillfocusMinThreshold();
	afx_msg void OnChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgAdvancedSegmentPage property page

class CDlgAdvancedSegmentPage : public CPropertyPage
{

	// Construction/destruction/creation
public:
	CDlgAdvancedSegmentPage(); // standard constructor

	// Attributes
private:
	// CLW 1.07a
	CSpinControl m_SpinSegment;
	CSpinControl m_SpinChangeMin;
	CSpinControl m_SpinZeroCrossingMin;
	BOOL         m_bModified; // TRUE if page member data modified

	// Dialog Data
	//{{AFX_DATA(CDlgAdvancedSegmentPage)
	enum { IDD = IDD_ADVANCEDSEGMENTPAGE };
	int     m_nSegmentWidth;
	int     m_nChMinThreshold;
	int     m_nZCMinThreshold;
	//}}AFX_DATA

	// Operations
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
public:
	void Apply();

	// Generated message map functions
protected:
	// CLW 1.07a
	//{{AFX_MSG(CDlgAdvancedSegmentPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSegmentWidthScroll();
	afx_msg void OnKillfocusSegmentWidth();
	afx_msg void OnChangeMinScroll();
	afx_msg void OnKillfocusChangeMin();
	afx_msg void OnZeroCrossMinScroll();
	afx_msg void OnKillfocusZeroCrossMin();
	afx_msg void OnChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgAdvanced property sheet

class CDlgAdvanced : public CPropertySheet
{   
	DECLARE_DYNAMIC(CDlgAdvanced)

// Construction/destruction/creation
public:
	enum DisplayMode{ SHOW_SEGMENT, SHOW_PARSE, SHOW_BOTH};

	CDlgAdvanced( LPCTSTR pszCaption, CWnd* pParent, UINT iSelectPage, DisplayMode mode);

	// Attributes
public:
	CDlgAdvancedParsePage m_dlgParsePage;
	CDlgAdvancedSegmentPage m_dlgSegmentPage;

	// Operations
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

	// Generated message map functions
protected:
	//{{AFX_MSG(CDlgAdvanced)
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnApply();
	afx_msg void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	DisplayMode m_mode;
};

#endif //_ADVANCED_PARAMETERS_H
