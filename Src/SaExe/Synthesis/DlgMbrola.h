//{{AFX_INCLUDES()
//}}AFX_INCLUDES

#if !defined(AFX_DLGMBROLA_H__19EAED4A_02F4_11D5_9FE4_C4B78D7E672E__INCLUDED_)
#define AFX_DLGMBROLA_H__19EAED4A_02F4_11D5_9FE4_C4B78D7E672E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgMbrola.h : header file
//

#include "activex\flexEditGrid.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgMbrola dialog

class CDlgMbrola : public CPropertyPage
{
  DECLARE_DYNCREATE(CDlgMbrola)

    // Construction
public:
  CDlgMbrola();   // standard constructor
  ~CDlgMbrola();   // standard constructor

  CString m_szMBRolaName;

  double m_fPitchUpdateInterval;
  CFlexEditGrid	m_cGrid;
  CFlexEditGrid	m_cOutputGrid;
  BOOL m_bGetComplete;
  BOOL m_bConvertComplete;
  // Dialog Data
  //{{AFX_DATA(CDlgMbrola)
  enum { IDD = IDD_MBROLA };
  CComboBox	m_cSource;
  CComboBox	m_cDictionary;
  BOOL	m_bGetDuration;
  BOOL	m_bGetIPA;
  BOOL	m_bGetPitch;
  //}}AFX_DATA


  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CDlgMbrola)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

private:
  enum 
  {
    rowHeading = 0,
    rowIpa = 1, rowSampa = 1, 
    rowDuration = 2, 
    rowPitchAvg = 3,
    rowPitchMs = 5
  };
  enum {columnDescription = 0,columnFirst = 1};
  // Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CDlgMbrola)
  afx_msg void OnMbrolaGet();
  afx_msg void OnMbrolaSynthesize();
  virtual BOOL OnInitDialog();
  afx_msg void OnLeaveCellMbrolaGrid();
  afx_msg void OnMbrolaPlaySynth();
  afx_msg void OnMbrolaPlay();
  afx_msg void OnMbrolaPlaySource();
	afx_msg void OnMbrolaConvert();
	afx_msg void OnMbrolaDisplay();
	afx_msg void OnDropdownSource();
  DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGMBROLA_H__19EAED4A_02F4_11D5_9FE4_C4B78D7E672E__INCLUDED_)
