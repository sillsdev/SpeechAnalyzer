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

    // Operations
public:
    BOOL IsDifferentFile();

protected:
    CWorkbenchDoc();
    virtual ~CWorkbenchDoc();

    virtual void Serialize(CArchive & ar); // overridden for document i/o
    virtual BOOL OnOpenDocument(LPCTSTR pszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR pszPathName);
    virtual BOOL SaveModified(); // return TRUE if ok to continue
    afx_msg void OnFileSave();
    afx_msg void OnUpdateFileSave(CCmdUI * pCmdUI);

    DECLARE_MESSAGE_MAP()

    // Attributes
    BOOL m_bDifferentFile; // TRUE, if file has different path than the one before

};

#endif //_SA_W_DOC_H
