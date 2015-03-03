/////////////////////////////////////////////////////////////////////////////
// sa_wbch.h:
// Interface of the CSaWorkbenchView class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_WBCH_H
#define _SA_WBCH_H

#include "appdefs.h"
#include "Sa_dlWnd.h"

//###########################################################################
// CSaWorkbenchView frame

class CObjectOStream;
class CObjectIStream;
class CProcess;

class CSaWorkbenchView : public CFormView {
    DECLARE_DYNCREATE(CSaWorkbenchView)

// Construction/destruction/creation
public:
    CSaWorkbenchView();  // standard constructor

    // Attributes
private:
    CBrush       m_brBkg;
    CBitmapButton m_aRawButton[MAX_PROCESS_NUMBER];
    CBitmapButton m_aGraphButton[MAX_PROCESS_NUMBER];
    CBitmapButton m_aFilterButton[MAX_PROCESS_NUMBER][MAX_FILTER_NUMBER];
    CStaticText   m_aProcText[MAX_PROCESS_NUMBER];
    CFancyArrow   m_aArrow[MAX_PROCESS_NUMBER][MAX_FILTER_NUMBER + 1];

public:
    //{{AFX_DATA(CSaWorkbenchView)
    enum { IDD = IDD_WORKBENCH };
    //}}AFX_DATA

    // Operations
private:
    HBRUSH GetBkgBrush() {
        return (HBRUSH)m_brBkg.GetSafeHandle();   // return background brush
    }
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    virtual void OnInitialUpdate();
public:
    CProcess * CreateWbProcess(int nFilterNumber); // creates a workbench process
    LPCTSTR GetFilterResource(int nFilterNumber); // returns a pointer to a filter resource
    void SetupFilterProcesses(CObject * pDialog); // sets up workbench filter processes
    void CallPropertiesDialog(int nProcess, int nFilter); // calls the properties dialog of a workbench process
    void LoadAndSortFilter(int nProcess, int * pnFilter1, int * pnFilter2, int * pnFilter3, BOOL bLoad = TRUE); // loads and sorts (left align) filters
    virtual void WriteProperties(CObjectOStream & obs);
    virtual BOOL ReadProperties(CObjectIStream & obs);

    // Generated message map functions
protected:
    afx_msg HBRUSH OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor);
    afx_msg void OnActivateView(BOOL, CView *, CView *);
    afx_msg void OnDestroy();
    afx_msg void OnProcess0();
    afx_msg void OnProcess1();
    afx_msg void OnProcess2();
    afx_msg void OnProcess3();
    afx_msg void OnFilter10();
    afx_msg void OnFilter11();
    afx_msg void OnFilter12();
    afx_msg void OnFilter13();
    afx_msg void OnFilter20();
    afx_msg void OnFilter21();
    afx_msg void OnFilter22();
    afx_msg void OnFilter23();
    afx_msg void OnFilter30();
    afx_msg void OnFilter31();
    afx_msg void OnFilter32();
    afx_msg void OnFilter33();
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_WBCH_H
