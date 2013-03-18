// FileOpen.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "FileOpen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgFileOpen

IMPLEMENT_DYNAMIC(CDlgFileOpen, CFileDialog)

CDlgFileOpen::CDlgFileOpen(LPCTSTR lpszDefExt,
                           LPCTSTR lpszFileName,
                           DWORD dwFlags,
                           LPCTSTR lpszFilter,
                           CWnd * pParentWnd) :
    CFileDialog(TRUE, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd, 0, FALSE) {
    SetTemplate(IDD, IDD);
    m_ofn.hInstance = AfxFindResourceHandle(MAKEINTRESOURCE(IDD),RT_DIALOG);
}


BEGIN_MESSAGE_MAP(CDlgFileOpen, CFileDialog)
    //{{AFX_MSG_MAP(CDlgFileOpen)
    ON_BN_CLICKED(IDC_PLAY, OnPlay)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CDlgFileOpen::OnPlay() {
    PlaySound(GetPathName(), 0, SND_ASYNC | SND_NODEFAULT | SND_FILENAME);
}
