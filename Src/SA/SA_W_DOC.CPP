/////////////////////////////////////////////////////////////////////////////
// sa_w_doc.cpp:
// Implementation of the CWorkbenchDoc class.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa.h"
#include "sa_wbch.h"
#include "sa_w_dlg.h"
#include "mainfrm.h"
#include "sa_w_doc.h"
#include "objectostream.h"
using std::ios;
using std::ifstream;
using std::ofstream;


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CWorkbenchDoc
// Workbench document class. Handles the data to setup the workbench, stores
// and restores it from a special workbench file.

IMPLEMENT_SERIAL(CWorkbenchDoc, CDocument, 0 /* schema number*/)

/////////////////////////////////////////////////////////////////////////////
// CWorkbenchDoc message map

BEGIN_MESSAGE_MAP(CWorkbenchDoc, CDocument)
    ON_COMMAND(ID_FILE_SAVE, OnFileSave)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkbenchDoc construction/destruction/creation

/***************************************************************************/
// CWorkbenchDoc::CWorkbenchDoc Constructor
/***************************************************************************/
CWorkbenchDoc::CWorkbenchDoc() {
    m_bDifferentFile = TRUE;
}

/***************************************************************************/
// CWorkbenchDoc::~CWorkbenchDoc Destructor
/***************************************************************************/
CWorkbenchDoc::~CWorkbenchDoc() {
}

/////////////////////////////////////////////////////////////////////////////
// CWorkbenchDoc serialization
void CWorkbenchDoc::Serialize(CArchive & ar) {
    if (ar.IsStoring()) {
        // TODO: add storing code here
    } else {
        // TODO: add loading code here
    }
}

/////////////////////////////////////////////////////////////////////////////
// CWorkbenchDoc helper functions

/***************************************************************************/
// CWorkbenchDoc::OnOpenDocument Opening a new document
// Reads all available information from the workbench file and sets up the
// workbench processes and all there properties.
/***************************************************************************/
BOOL CWorkbenchDoc::OnOpenDocument(LPCTSTR pszPathName) {

    if (!((CSaApp *)AfxGetApp())->CloseWorkbench(this)) {
		// close already opened workbench
        return FALSE;    
    }
    if (!CDocument::OnOpenDocument(pszPathName)) {
        return FALSE;
    }
    CString szPath = *((CSaApp *)AfxGetApp())->GetWorkbenchPath();
    CString szNewPath = pszPathName;
    m_bDifferentFile = (szNewPath != szPath);

    // wait cursor
    CScopedCursor cursor(*this);
    BOOL bRet = FALSE;

    try {
        CSaString path(pszPathName);
        CObjectIStream obs(path.utf8().c_str());
        POSITION pos = GetFirstViewPosition();
        bRet = ((CSaWorkbenchView *)GetNextView(pos))->ReadProperties(obs);
    } catch (...) {
    }
    return bRet;
}

/***************************************************************************/
// CWorkbenchDoc::OnSaveDocument Saving a document
// Stores all available information from workbench into the workbench
// settings file.
/***************************************************************************/
BOOL CWorkbenchDoc::OnSaveDocument(LPCTSTR pszPathName) {
    
    CScopedCursor cursor(*this);
    BOOL bRet = FALSE;

    try {
        CSaString path(pszPathName);
        CObjectOStream obs(path.utf8().c_str());
        POSITION pos = GetFirstViewPosition();
        ((CSaWorkbenchView *)GetNextView(pos))->WriteProperties(obs);
        bRet = TRUE;
    } catch (...) {
    }
    SetModifiedFlag(FALSE);
    return bRet;
}

/***************************************************************************/
// CWorkbenchDoc::SaveModified Saving a modified document
// Before using the framework original save functions we have to change the
// documents title. We have to clean up the title and after the closing reset
// it with the workbench title (because the user may cancel the closing).
/***************************************************************************/
BOOL CWorkbenchDoc::SaveModified() {
    CString szCaption, szGraphTitle;
    if (!IsModified()) {
        return TRUE;    // ok to continue
    }
    LPCTSTR pszPathName = GetPathName();
    if (_tcslen(pszPathName) == 0) {
        // the pathname is empty, reset title string
		// get the current caption string
        szCaption = GetTitle(); 
        int nFind = szCaption.Find(':');
        if (nFind != -1) {
			// extract part right of and with :
            szGraphTitle = szCaption.Right(szCaption.GetLength() - nFind); 
			// extract part left of :
            szCaption = szCaption.Left(nFind); 
            szCaption = szCaption.Trim();
        }
		// write the new caption string
        SetTitle(szCaption); 
    }
    // call the base class saving
    BOOL bResult = CDocument::SaveModified();
    if (!szCaption.IsEmpty()) {
        // set back the title string
		// get the current view caption string
        szCaption = GetTitle(); 
		// add the graph title
        szCaption += szGraphTitle; 
		// write the new caption string
        SetTitle(szCaption); 
    }
    return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// CWorkbenchDoc message handlers

/***************************************************************************/
// CWorkbenchDoc::OnFileSave Save file
// Before using the framework original save functions we have to change the
// title string. CDocument will call save as routine to save the document.
// So we have to be modify the title string and reset to its original state
// (the view has added the workbench title), because this title is used to
// generate the new default filename. After that, the title has again to be
// modified with the workbench title.
/***************************************************************************/
void CWorkbenchDoc::OnFileSave() {

    CString szCaption;
	CString szGraphTitle;
    LPCTSTR pszPathName = GetPathName();
    if (_tcslen(pszPathName) == 0) {
        // the pathname is empty, reset view title string
		// get the current view caption string
        szCaption = GetTitle(); 
        int nFind = szCaption.Find(':');
        if (nFind != -1) {
			// extract part right of and with :
            szGraphTitle = szCaption.Right(szCaption.GetLength() - nFind); 
			// extract part left of :
            szCaption = szCaption.Left(nFind); 
            szCaption = szCaption.Trim();
        }
		// write the new caption string
        SetTitle(szCaption); 
    }

    // call the base class saving
    CDocument::OnFileSave();
    if (!szCaption.IsEmpty()) {
        // set back the title string
		// get the current view caption string
        szCaption = GetTitle(); 
		// add the graph title
        szCaption += szGraphTitle;
		// write the new caption string
        SetTitle(szCaption); 
    }
}

/***************************************************************************/
// CWorkbenchDoc::OnUpdateFileSave Menu Update
/***************************************************************************/
void CWorkbenchDoc::OnUpdateFileSave(CCmdUI * pCmdUI) {
	// enable always to be able to save window placement
    pCmdUI->Enable(TRUE); 
}

BOOL CWorkbenchDoc::IsDifferentFile() {
	// returns TRUE, if file has different path
    return m_bDifferentFile;   
}
