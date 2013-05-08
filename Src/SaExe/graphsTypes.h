/////////////////////////////////////////////////////////////////////////////
// graphsTypes.h:
// Implementation of the CDlgGraphsTypesCustom (dialog)
//                       CDlgGraphsTypes (property sheet)
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
#ifndef _GRAPHS_TYPES_H

#include "sa_dlwnd.h"
#include "resource.h"
#include "appdefs.h"

#define _GRAPHS_TYPES_H

//###########################################################################
// CDlgGraphsTypesCustom dialog

class CObjectOStream;
class CObjectIStream;
class CSaDoc;
class CSaString;

class CGraphList
{
public:
    CGraphList(const UINT * pGraphIDs = NULL);
    CGraphList(const CGraphList & src);

public:
    int GetCount(BOOL bLayoutOnly = FALSE) const;
    void SelectGraph(UINT nID, BOOL bSelected=TRUE, BOOL bDefaultOrder = TRUE);
    int IsSelected(UINT nID) const;
    void ClearGraphs(BOOL bFilter = FALSE, BOOL bLayout = FALSE);
    void GetGraphs(UINT * pGraphIDs) const; // return checked graphs
    const UINT * GetGraphs() const
    {
        return m_nGraphID;   // return checked graphs
    }
    void SetGraphs(const UINT * pGraphIDs, BOOL bClear=TRUE);
    static bool IsLayoutGraph(UINT nID)
    {
        return nID != IDD_RECORDING && nID != IDD_TWC && nID != IDD_MAGNITUDE;
    }
    void InsertAt(UINT nID, int nIndex);
    bool operator==(const CGraphList & compare) const;
    UINT operator[](int nIndex) const
    {
        return (nIndex >= 0 && nIndex  < MAX_GRAPHS_NUMBER) ? m_nGraphID[nIndex] : 0;
    }
    CSaString GetDescription() const;
private:
    static int GraphDefaultOrder(UINT nID);
    void CompactList();
private:
    UINT  m_nGraphID[MAX_GRAPHS_NUMBER];  // array of graph IDs
};

class CDlgGraphsTypesCustom : public CDialog
{

    // Construction/destruction/creation
public:
    CDlgGraphsTypesCustom(CWnd * pParent = NULL); // standard constructor

    // Attributes

    // Dialog Data
    //{{AFX_DATA(CDlgGraphsTypesCustom)
    enum { IDD = IDD_GRAPHSTYPES_CUSTOM };
    int     m_nLayout;
    //}}AFX_DATA

    // Operations
public:
    void GetCheckedGraphs(UINT *, int * nLayout); // return checked graphs
    void OnHelpGraphsTypes();

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

private:
    CGraphList m_cGraphs;
    CFont m_Font;             // special font for dialog controls
    CSaDoc * m_pDoc;            // holds a SA document
    CLayoutListBox m_Layout;

    void CheckDlgButton(int nIDButton, UINT nCheck, BOOL bDefaultOrder = TRUE);

    // Generated message map functions
protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnSelchangeLayout();
    afx_msg void OnOrder();
    afx_msg void OnGraphCheck(UINT nID);
    afx_msg void OnMelogram();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsTypes dialog

class CGraphConfiguration : public CGraphList
{
public:
    CGraphConfiguration(const UINT * pGraphIDs = NULL, int nLayout = -1) : CGraphList(pGraphIDs), m_nLayout(nLayout)
    {
        ;
    }
    CGraphConfiguration(const CGraphList & src, int nLayout = -1) : CGraphList(src), m_nLayout(nLayout)
    {
        ;
    }
    CGraphConfiguration(const CGraphConfiguration & src) : CGraphList(src), m_nLayout(src.m_nLayout)
    {
        ;
    }

    bool operator==(const CGraphConfiguration & compare) const
    {
        return compare.m_nLayout == m_nLayout ? CGraphList::operator ==(compare) : FALSE;
    }

    void WriteProperties(CObjectOStream & obs, BOOL bPredefined) const;
    BOOL ReadProperties(CObjectIStream & obs, BOOL & bPredefined);

    int     m_nLayout;
};

class CGraphConfigurationVector : private std::vector<CGraphConfiguration>
{
public:
    CGraphConfigurationVector(LPCTSTR szFilename = NULL, int nTaskType = -1)
    {
        SetFilename(szFilename);
        Load(szFilename, nTaskType);
    }
    virtual ~CGraphConfigurationVector() {};

private:
    typedef std::vector<CGraphConfiguration> BaseClass;

public:
    BOOL Load(LPCTSTR szFilename, int nTaskType);
    BOOL Save(const LPCTSTR szFilename) const;
    BOOL Save() const
    {
        return Save(m_szFilename);
    }
    void Remove(int nIndex)
    {
        erase(begin()+nIndex);
    }
    CString SetFilename(const LPCTSTR szFilename)
    {
        CString szOld(m_szFilename);
        m_szFilename = szFilename;
        return szOld;
    }
    int GetCountPredefinedSets() const
    {
        return m_nPredefinedSets;
    }

    // publicize some vector member functions
    const CGraphConfiguration & operator[](size_type _P) const
    {
        return static_cast<const BaseClass *>(this)->operator[](_P);
    }
    CGraphConfiguration & operator[](size_type _P)
    {
        return static_cast<BaseClass *>(this)->operator[](_P);
    }
    size_type size() const
    {
        return static_cast<const BaseClass *>(this)->size();
    }
    void push_back(const CGraphConfiguration & entry)
    {
        static_cast<BaseClass *>(this)->push_back(entry);
    }
    inline bool IsEqual(const CGraphConfigurationVector & operand2) const
    {
        return *this == operand2;
    }

    int find(const CGraphConfiguration & data) const;

private:
    CString m_szFilename;
    int m_nPredefinedSets;
};


class CDlgGraphsTypes : public CDialog
{
    // Construction
public:
    CDlgGraphsTypes(CWnd * pParent, const UINT * pGraphIDs, int nLayout); // standard constructor

    // Dialog Data
    int m_nTaskType;
    //{{AFX_DATA(CDlgGraphsTypes)
    enum { IDD = IDD_GRAPHSTYPES };
    CListBox  m_cList;
    CTabCtrl  m_cTaskType;
    //}}AFX_DATA

public:
    void GetCheckedGraphs(UINT * pGraphIDs, int * nLayout); // return checked graphs
    static void GetStandardCheckedGraphs(int nConfiguration, UINT * pGraphIDs, int * nLayout, CString * label = NULL, CString * tip = NULL);
    static CGraphConfiguration GetStandardCheckedGraphs(int nConfiguration);
    static int & RecentTaskType();
    void OnHelpGraphsTypes();

    // Implementation
protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    BOOL bCustom;
    int m_nConfiguration;
    const CGraphConfiguration cCurrentConfig;
    virtual BOOL PreTranslateMessage(MSG * pMsg);
    CGraphConfigurationVector & GetGraphConfigurationVector(int nTaskType);


private:
    static void PopulateList(CListBox & cList, const CGraphConfigurationVector & cVector, BOOL bAlphaHotKey = FALSE);
    CDlgGraphsTypesCustom m_cCustom;
    CGraphConfigurationVector m_cSpeech;
    CGraphConfigurationVector m_cMusic;

    // Generated message map functions
    afx_msg void OnCustom();
    virtual void OnOK();
    afx_msg void OnAdd();
    afx_msg void OnRemove();
    virtual BOOL OnInitDialog();
    afx_msg void OnTaskTypeChange(NMHDR * pNMHDR = NULL , LRESULT * pResult = NULL);
    afx_msg void OnSelchangeGraphTypes();
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CDlgGraphTypesOrder dialog

class CDlgGraphTypesOrder : public CDialog
{
    // Construction
public:
    CDlgGraphTypesOrder(CGraphList & cOrder, int nLayout, CWnd * pParent = NULL); // standard constructor

    // Dialog Data
    CGraphList & m_cResultOrder;
    CGraphList m_cLayoutOrder;
    const int m_nLayout;

    //{{AFX_DATA(CDlgGraphTypesOrder)
    enum { IDD = IDD_GRAPHTYPES_ORDER };
    CListBox  m_cList;
    //}}AFX_DATA


    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDlgGraphTypesOrder)
protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    //}}AFX_VIRTUAL

private:
    void OnGraphListChanged();
    void OnDrawLayout();
    void OnPopulateList();
    void ApplyLayoutOrder();
    void SwapLayoutGraphs(int nFirst, int nSecond = -1);
    static CGraphList GetLayoutOrder(const CGraphList & src);

    // Implementation
public:
    void OnHelpGraphTypesOrder();
protected:

    // Generated message map functions
    afx_msg void OnDown();
    afx_msg void OnUp();
    afx_msg void OnSelchangeList();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    DECLARE_MESSAGE_MAP()
};

#endif //_GRAPHS_TYPES_H
