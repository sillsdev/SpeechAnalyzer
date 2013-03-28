//
// SelfTest.h
//
// Definition of the CSASelfTest class
//
// 08/27/2001 TRE - original coding
//

#ifndef SELFTEST_H

#include <io.h> // for _access()
#include "stdafx.h"
#include "sa.h"
#include "mainfrm.h"

class CSASelfTest
{
    CSaApp * m_pApp;
    CMainFrame * m_pMain;
    CString m_szTempPath;
    CString m_szExeFolderPath;
    CString m_szTestFolderPath;
    CString m_szLogFileName;
    CString m_szTestNumber;
    unsigned m_nTestsPassed, m_nTestsFailed;

    void SelfTest();

    BOOL StartTest(const CString szTestNumber, const CString szDescription);
    void EndTest(BOOL bSuccess=TRUE);
    void LogEntry(const CString szMessage);
    BOOL LogHexDataCompare(CFile & FileOne,CFile & FileTwo,UINT HighlightPosition);
    int FileCompare(const CString szFileOne, long nTolerableDifferences=0, CString szFileTwo="");
    BOOL FileExists(const CString szFile)
    {
        return !_taccess(szFile,0);
    }
    void MessageLoop(DWORD dwMilliSeconds=0);
    void EmptyClipboard();
    BOOL CheckClipboard(UINT nFormat);
public:
    CSASelfTest();
    ~CSASelfTest();
};//CSASelfTest

#endif //SELFTEST_H
