#ifndef DLGEXPORTTABLE_H
#define DLGEXPORTTABLE_H

#include "CSaString.h"
#include "resource.h"

class CDlgExportTable : public CDialog
{
	// Construction
public:
	CDlgExportTable(const CSaString& szDocTitle, CWnd* pParent = NULL); // standard constructor

	// Dialog Data
	enum { IDD = IDD_EXPORT_TABLE };
	BOOL  m_bAnnotations;
	BOOL  m_bCalculations;
	BOOL  m_bF1;
	BOOL  m_bF2;
	BOOL  m_bF3;
	BOOL  m_bF4;
	BOOL  m_bFormants;
	BOOL  m_bGloss;
	CSaString m_szIntervalTime;
	BOOL  m_bSegmentLength;
	BOOL  m_bMagnitude;
	BOOL  m_bOrtho;
	BOOL  m_bPhonemic;
	BOOL  m_bPhonetic;
	BOOL  m_bPitch;
	BOOL  m_bPOS;
	BOOL  m_bReference;
	BOOL  m_bSegmentStart;
	BOOL  m_bSampleTime;
	BOOL  m_bTone;
	BOOL  m_bZeroCrossings;
	int   m_nSampleRate;
	int   m_nCalculationMethod;
	BOOL  m_bPhonetic2;
	int   m_nRegion;
	BOOL  m_bMelogram;
private:
	CSaString m_szFileName;
	CSaString m_szDocTitle;

	// Implementation
public:
	void OnHelpExportTable();

protected:
	void SetEnable(int nItem, BOOL bEnable);
	void SetVisible(int nItem, BOOL bVisible, BOOL bEnable = TRUE);
	virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support

	// Generated message map functions
	afx_msg void OnAllAnnotations();
	afx_msg void OnAllFormants();
	afx_msg void OnAllCalculations();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateIntervalTime();
	afx_msg void OnSample();
	afx_msg void OnPhonetic();
	DECLARE_MESSAGE_MAP()
};

#endif
