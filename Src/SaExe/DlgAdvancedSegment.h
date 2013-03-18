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
#ifndef DLGADVANCEDPARSESEGMENT_H
#define DLGADVANCEDPARSESEGMENT_H

#include "sa_dlwnd.h"
#include "resource.h"

class CSaDoc;

//###########################################################################
// CDlgAdvancedSegment property page

class CDlgAdvancedSegment : public CDialog {
    DECLARE_DYNAMIC(CDlgAdvancedSegment)
public:
    CDlgAdvancedSegment(CSaDoc * pDoc); // standard constructor
    virtual ~CDlgAdvancedSegment();

    BOOL Create();
    void Show(LPCTSTR title);

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
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
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
    afx_msg void OnBnClickedApply();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    virtual void OnOK();
    virtual void OnCancel();

public:
    CButton m_OKButton;
    CButton m_ApplyButton;

    CSaDoc * m_pDoc;
};

#endif
