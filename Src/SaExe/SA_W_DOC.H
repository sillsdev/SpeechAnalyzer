/////////////////////////////////////////////////////////////////////////////
// sa_w_doc.h:
// Interface of the CWorkbenchDoc class
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_DOC_H
#define _SA_W_DOC_H

//###########################################################################
// CWorkbenchDoc document

class CWorkbenchDoc : public CDocument {
    DECLARE_SERIAL(CWorkbenchDoc)

// Construction/destruction/creation
protected:
    CWorkbenchDoc();

    // Attributes
protected:
    BOOL m_bDifferentFile; // TRUE, if file has different path than the one before

    // Operations
public:
    BOOL IsDifferentFile() {
        return m_bDifferentFile;   // returns TRUE, if file has different path
    }
protected:
    virtual ~CWorkbenchDoc();
    virtual void Serialize(CArchive & ar); // overridden for document i/o
    virtual BOOL OnOpenDocument(const TCHAR * pszPathName);
    virtual BOOL OnSaveDocument(const TCHAR * pszPathName);
    virtual BOOL SaveModified(); // return TRUE if ok to continue

    // Generated message map functions
protected:
    //{{AFX_MSG(CWorkbenchDoc)
    afx_msg void OnFileSave();
    afx_msg void OnUpdateFileSave(CCmdUI * pCmdUI);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif //_SA_W_DOC_H
