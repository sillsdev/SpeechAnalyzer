/////////////////////////////////////////////////////////////////////////////
// DlgAdvancedSegment.h:
// Interface of the CDlgAdvancedSegment
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
#pragma once

#include "sa_dlwnd.h"
#include "resource.h"

//###########################################################################
// CDlgAdvancedSegment property page

class CDlgAdvancedSegment : public CDialog
{
	DECLARE_DYNAMIC(CDlgAdvancedSegment)
public:
	CDlgAdvancedSegment(CWnd* pParent = NULL); // standard constructor
	virtual ~CDlgAdvancedSegment() {};
private:
	CSpinControl m_SpinSegment;
	CSpinControl m_SpinChangeMin;
	CSpinControl m_SpinZeroCrossingMin;

	// Dialog Data
	//{{AFX_DATA(CDlgAdvancedSegment)
	enum { IDD = IDD_ADVANCEDSEGMENT};
	int m_nSegmentWidth;
	int m_nChMinThreshold;
	int m_nZCMinThreshold;
	//}}AFX_DATA

	// Operations
protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	void EnableTestButton(bool enable);
	void Apply();
	void Undo();

	// Generated message map functions
	// CLW 1.07a
	//{{AFX_MSG(CDlgAdvancedSegment)
	virtual BOOL OnInitDialog();
	afx_msg void OnSegmentWidthScroll();
	afx_msg void OnChangeMinScroll();
	afx_msg void OnZeroCrossMinScroll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
public:
	CButton m_OKButton;
	CButton m_ApplyButton;
	afx_msg void OnBnClickedApply();
};
