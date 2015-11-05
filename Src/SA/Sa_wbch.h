/////////////////////////////////////////////////////////////////////////////
// sa_wbch.h:
// Interface of the CSaWorkbenchView class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_WBCH_H
#define _SA_WBCH_H

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
	// standard constructor
    CSaWorkbenchView();  

    // Attributes
private:
    CBrush m_brBkg;
    CBitmapButton m_aRawButton[MAX_PROCESS_NUMBER];
    CBitmapButton m_aGraphButton[MAX_PROCESS_NUMBER];
    CBitmapButton m_aFilterButton[MAX_PROCESS_NUMBER][MAX_FILTER_NUMBER];
    CStaticText m_aProcText[MAX_PROCESS_NUMBER];
    CFancyArrow m_aArrow[MAX_PROCESS_NUMBER][MAX_FILTER_NUMBER + 1];

public:
    enum { IDD = IDD_WORKBENCH };

public:
	// creates a workbench process
    CProcess * CreateWbProcess(int nFilterNumber);	
	// returns a pointer to a filter resource
    UINT GetFilterResource(int nFilterNumber);
	// sets up workbench filter processes
    void SetupFilterProcesses(CObject * pDialog);
	// calls the properties dialog of a workbench process
    void CallPropertiesDialog(int nProcess, int nFilter);
	// loads and sorts (left align) filters
    void LoadAndSortFilter(int nProcess, int * pnFilter1, int * pnFilter2, int * pnFilter3, BOOL bLoad = TRUE);
    virtual void WriteProperties(CObjectOStream & obs);
    virtual BOOL ReadProperties(CObjectIStream & obs);

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    virtual void OnInitialUpdate();

    // Generated message map functions
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

    // Operations
private:
    HBRUSH GetBkgBrush();

};

#endif //_SA_WBCH_H
