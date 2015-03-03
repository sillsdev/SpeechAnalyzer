//
// CSelfTest.h
//
// Definition of the CSelfTestRunner class
//
// 08/27/2001 TRE - original coding
//

#ifndef SELFTEST_H
#define SELFTEST_H

#include <io.h> // for _access()
#include "sa.h"
#include "mainfrm.h"
#include <vector>
using std::vector;

class CSelfTest {
public:
    class Summary {
    public:
        string status;
        string datetime;
        int testspassed;
        int testsfailed;
    } summary;
    class SysInfo {
    public:
        string user;
        string computer;
        string os;
        string processor;
        string version;
    } sysInfo;
    class Test {
    public:
        string id;
        string description;
        vector<string> details;
        bool success;
        class FileDifference {
        public:
            string id1;
            string id2;
            string notice;
            string data1;
            string data2;
        };
        vector<FileDifference> differences;
    };
    vector<Test> tests;
};

class CSelfTestRunner {
public:
    CSelfTestRunner();
    ~CSelfTestRunner();
    void Do();

    LPCTSTR GetTestDataFolder();
    LPCTSTR GetTestOutputFolder();
    LPCTSTR GetTempPath();
    int FileCompare(CSelfTest::Test & testresult, LPCTSTR szFileOne, long nTolerableDifferences=0);
    int FileCompare(CSelfTest::Test & testresult, LPCTSTR szFileOne, long nTolerableDifferences, LPCTSTR szFileTwo);
    void MessageLoop(DWORD dwMilliSeconds=0);
    bool CheckClipboard(UINT nFormat);

protected:
    bool SelectTestDataDirectory();
    void GetSystemInfo();
    void RunTests();
    void RunTest(LPCSTR szTestNumber, LPCSTR szDescription, void (*test)(CSelfTestRunner & runner, CSelfTest::Test & test));
    void LogResults();
    void DisplayResults();

    bool HexDataCompare(CSelfTest::Test & testresult, CFile & FileOne,CFile & FileTwo,UINT HighlightPosition);
    void EmptyClipboard();

    CSaApp * m_pApp;

    CSelfTest result;

    wstring m_szTempPath;
    wstring m_szExePath;
    wstring m_szTestOutputPath;
    wstring m_szTestDataPath;
    wstring m_szLogFileName;
};

#endif
