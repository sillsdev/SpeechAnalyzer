//
// CSelfTest.cpp
//
// Implementation of the CSelfTestRunner class
//
// This class will run through a test routine when instantiated, logging results to an xml file
//
// 08/27/2001 TRE - original coding
//
#include "stdafx.h"
#include "SelfTest.h"
#include <windows.h>
//#include <mapiwin.h>
#include <process.h>
#include "sa_g_spg.h"
#include "sa_plot.h"
#include "SpectroParm.h"
#include "Process\Process.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_pit.h"
#include "Process\sa_p_cpi.h"
#include "Process\sa_p_spi.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_mel.h"
#include "Process\sa_p_cha.h"
#include "Process\sa_p_raw.h"
#include "Process\sa_p_spg.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_fmt.h"
#include "Process\sa_p_dur.h"
#include "Process\sa_p_glo.h"
#include "Process\sa_p_poa.h"
#include "Process\sa_p_rat.h"
#include "Process\sa_p_twc.h"
#include "Process\Butterworth.h"
#include "Process\formanttracker.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "resource.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "Process\Hilbert.h"

//
// file location of messages that get updated regularly
#define STATUS_MESSAGE 0x3A
#define N_TESTS_PASSED 0x95
#define N_TESTS_FAILED 0xB7

using std::exception;

//
// CSelfTestRunner constructor
//
CSelfTestRunner::CSelfTestRunner()
{
    TCHAR szString[ MAX_PATH] = _T("");

    m_pApp = (CSaApp *) AfxGetApp();

    TCHAR szExeFilename[ MAX_PATH];
    GetModuleFileName(AfxGetInstanceHandle(),szExeFilename,_MAX_PATH);

    ::GetTempPath( MAX_PATH,szString);
    m_szTempPath = szString;

    m_szExePath = szExeFilename;
    size_t lastBackslash = m_szExePath.find_last_of('\\');
	if (lastBackslash!=wstring::npos)
	{
		wstring temp;
		temp = m_szExePath.substr(0,lastBackslash+1);
		m_szExePath = temp;
	}
    
	m_szTestOutputPath = m_szExePath + L"SelfTest\\";
    m_szLogFileName = m_szTestOutputPath + L"SelfTest.xml";
    result.summary.testspassed=0;
	result.summary.testsfailed=0;

	CreateDirectory( m_szTestOutputPath.c_str(),NULL);
}


CSelfTestRunner::~CSelfTestRunner()
{
}

wstring NormalizePath( LPCTSTR path)
{
	wchar_t buffer[MAX_PATH];
	wmemset(buffer,0,_countof(buffer));
	// we need to normalize the path for this all to work correctly.
	wstring result = path;
	if (GetFullPathName( result.c_str(), _countof(buffer), buffer, NULL)!=0)
	{
		result = buffer;
	}
	return result;
}

static int CALLBACK MyBrowseCallbackProc( HWND hwnd,UINT uMsg, LPARAM /*lParam*/, LPARAM /*lpData*/)
{
    switch (uMsg)
    {
	case BFFM_INITIALIZED:
		{
			wchar_t buffer[MAX_PATH];
			wmemset(buffer,0,_countof(buffer));

		    wstring temp = AfxGetApp()->GetProfileString(L"SelfTest",L"DataPath",(LPCTSTR)L"");
			if (temp.length()==0)
			{
				// If the BFFM_INITIALIZED message is received
				// set the path to the start path.
				GetModuleFileName( AfxGetInstanceHandle(),buffer,_countof(buffer));

				temp = buffer;
				size_t lastBackslash = temp.find_last_of('\\');
				if (lastBackslash!=wstring::npos)
				{
					temp = temp.substr(0,lastBackslash+1);
				}
				lastBackslash = temp.find_last_of('..');
				if (lastBackslash!=wstring::npos)
				{
					temp = temp.substr(0,lastBackslash+1);
				}

				temp.append(L"SelfTest\\");

			}

			temp = NormalizePath(temp.c_str());

			SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)temp.c_str());
		}
		break;

	case BFFM_SELCHANGED:
		{
			LRESULT hPos = SendMessage( hwnd, TVM_GETNEXTITEM, TVGN_CARET, 0);
			SendMessage( hwnd, TVM_ENSUREVISIBLE, hPos, 0);
		}
	}
	return 0;
}

bool CSelfTestRunner::SelectTestDataDirectory()
{

    CoInitialize(NULL);

	CString msg;
	msg.LoadStringW(IDS_CHOOSE_SELFTEST_FOLDER);

    BROWSEINFO bi;
	ZeroMemory(&bi,sizeof(bi));
	bi.hwndOwner = AfxGetMainWnd()->m_hWnd;
	bi.lpszTitle = msg.GetBuffer(msg.GetLength());
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lpfn = MyBrowseCallbackProc;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl==NULL)
    {
        // they canceled...
		msg.ReleaseBuffer();
        CoUninitialize();
        return false;
    }
	msg.ReleaseBuffer();

    TCHAR szPath[MAX_PATH];
    memset(szPath,0,sizeof(szPath));
    BOOL retval = SHGetPathFromIDList(pidl, szPath);
    CoTaskMemFree(pidl);
    CoUninitialize();

    if (!retval) return false;

    wstring temp(szPath);
    if (temp[temp.length()-1]!='\\')
    {
        temp.append(L"\\");
    }
	m_szTestDataPath = temp.c_str();

	AfxGetApp()->WriteProfileString(L"SelfTest",L"DataPath",m_szTestDataPath.c_str());

    return true;
}

void CSelfTestRunner::GetSystemInfo()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

	char msg[512];
	sprintf_s(msg,512,"%2.2lu/%2.2lu/%4.4lu %2.2lu:%2.2lu.%3.3lu",st.wMonth,st.wDay,st.wYear,st.wHour,st.wMinute,st.wMilliseconds);
	result.summary.datetime = msg;

    SYSTEM_INFO si;
    ::GetSystemInfo(&si);

    ULONG UserNameLength = _MAX_PATH;
    ::GetUserNameA(msg,&UserNameLength);
	result.sysInfo.user = msg;

    UserNameLength = _MAX_PATH;
    ::GetComputerNameA(msg,&UserNameLength);
	result.sysInfo.computer = msg;

	OSVERSIONINFOA vi;
    vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    ::GetVersionExA(&vi);

    switch (vi.dwPlatformId)
    {
    case VER_PLATFORM_WIN32s:
        sprintf_s(msg,512,"Windows %lu.%lu (build %u) with Win32s",vi.dwMajorVersion,vi.dwMinorVersion,(UINT)LOWORD(vi.dwBuildNumber));
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        if (vi.dwMinorVersion==0)
        {
            sprintf_s(msg,512,"Windows 95 (%lu.%lu, build %u)",vi.dwMajorVersion,vi.dwMinorVersion,(UINT)LOWORD(vi.dwBuildNumber));
        }
        else
		{
            sprintf_s(msg,512,"Windows %s (%lu.%lu, build %lu)", vi.dwMinorVersion>50?_T("ME"):_T("98"),vi.dwMajorVersion,vi.dwMinorVersion,vi.dwBuildNumber);
		}
        break;
    case VER_PLATFORM_WIN32_NT:
    {
		/*
		Windows	8				6.2	6	2	OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
		Windows Server 2012		6.2	6	2	OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
		Windows	7				6.1	6	1	OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
		Windows Server 2008 R2	6.1	6	1	OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
		Windows Server 2008		6.0	6	0	OSVERSIONINFOEX.wProductType != VER_NT_WORKSTATION
		Windows Vista			6.0	6	0	OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
		Windows Server 2003 R2	5.2	5	2	GetSystemMetrics(SM_SERVERR2) != 0
		Windows Server 2003		5.2	5	2	GetSystemMetrics(SM_SERVERR2) == 0
		Windows XP				5.1	5	1	Not applicable
		Windows 2000			5.0	5	0	Not applicable		*/
        
		char * szType = "NT";
		switch (vi.dwMajorVersion)
		{
		case 5:
			switch (vi.dwMinorVersion)
			{
			case 0: szType = "2000"; break;
			case 1: szType = "XP"; break;
			case 2: szType = "Server 2003"; break;
			default: szType = "Unknown"; break;
			}
			break;
		case 6:
			switch (vi.dwMinorVersion)
			{
			case 0: szType = "Vista"; break;
			case 1: szType = "Windows 7"; break;
			case 2: szType = "Windows 8"; break;
			default: szType = "Unknown"; break;
			}
			break;
		default:
			szType = "Unknown";
		}
        sprintf_s(msg,512,"Windows %s (%lu.%lu, build %lu)",szType,vi.dwMajorVersion,vi.dwMinorVersion,vi.dwBuildNumber);
    }
    }

	string temp = msg;
    if ((vi.szCSDVersion!=NULL) && (vi.szCSDVersion[0]!=0))
    {
        temp.append(" - ");

		temp.append(vi.szCSDVersion);
    }
	result.sysInfo.os = temp.c_str();

    switch (si.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_INTEL:
        switch (si.dwProcessorType)
        {
        case PROCESSOR_INTEL_386:
			result.sysInfo.processor = "386";
            break;
        case PROCESSOR_INTEL_486:
            result.sysInfo.processor = "486";
            break;
        case PROCESSOR_INTEL_PENTIUM:
            result.sysInfo.processor = "Pentium";
            break;
        }
        break;
    case PROCESSOR_ARCHITECTURE_MIPS:
        result.sysInfo.processor = "MIPS";
        break;
    case PROCESSOR_ARCHITECTURE_ALPHA:
        result.sysInfo.processor = "ALPHA";
        break;
    case PROCESSOR_ARCHITECTURE_PPC:
        result.sysInfo.processor = "PPC";
        break;
    case PROCESSOR_ARCHITECTURE_UNKNOWN:
        result.sysInfo.processor = "UNKNOWN";
        break;
    }

    CSaString szSAVersion;
    szSAVersion.LoadString(VS_VERSION);
	result.sysInfo.version = szSAVersion.utf8();
}

void CSelfTestRunner::RunTest( LPCSTR szTestNumber, LPCSTR szDescription, void (*test)(CSelfTestRunner & runner, CSelfTest::Test & test))
{
	CSelfTest::Test testresult;
	testresult.id = szTestNumber;

	char msg[512];
	sprintf_s(msg,512,"TEST %3.3d IN PROGRESS", result.summary.testspassed + result.summary.testsfailed+1);
	result.summary.status = msg;

    if (strlen(szDescription)>0)
    {
		testresult.description = szDescription;
    }

	try 
	{
		test(*this,testresult);
		testresult.success = true;
	}
	catch (std::exception e)
	{
		testresult.details.push_back(e.what());
		testresult.success = false;
	}
	catch(...)
	{
		testresult.details.push_back("unexpected exception");
		testresult.success = false;
	}

    if (testresult.success)
    {
		++result.summary.testspassed;
    }
    else
    {
        ++result.summary.testsfailed;
    }

	sprintf_s(msg,_countof(msg),"Completed %3.3d tests.",result.summary.testspassed+result.summary.testsfailed);
	result.summary.status = msg;

	result.tests.push_back(testresult);
}

void CSelfTestRunner::Do()
{
	if (!SelectTestDataDirectory()) return;

	GetSystemInfo();
    RunTests();
	LogResults();
	DisplayResults();
}

void CSelfTestRunner::LogResults()
{
	FILE * file = NULL;
	errno_t err = fopen_s( &file, Utf8( m_szLogFileName.c_str()).c_str(), "w");
	if (err!=0)
	{
		return;
	}

	fprintf(file,"<?xml version=\"1.0\" ?>\r\n");
	fprintf(file,"<SelfTest>\r\n");
	
	fprintf(file,"\t<Summary>\r\n");
	fprintf(file,"\t\t<Status>%s</Status>\r\n",result.summary.status.c_str());
	fprintf(file,"\t\t<DateTime>%s</DateTime>\r\n",result.summary.datetime.c_str());
	fprintf(file,"\t\t<TestsPassed>%d</TestsPassed>\r\n",result.summary.testspassed);
	fprintf(file,"\t\t<TestsFailed>%d</TestsFailed>\r\n",result.summary.testsfailed);
	fprintf(file,"\t</Summary>\r\n");

	fprintf(file,"\t<SysInfo>\r\n");
	fprintf(file,"\t\t<User>%s</User>\r\n",result.sysInfo.user.c_str());
	fprintf(file,"\t\t<Computer>%s</Computer>\r\n",result.sysInfo.computer.c_str());
	fprintf(file,"\t\t<OS>%s</OS>\r\n",result.sysInfo.os.c_str());
	fprintf(file,"\t\t<Processor>%s</Processor>\r\n",result.sysInfo.processor.c_str());
	fprintf(file,"\t\t<SAVersion>%s</SAVersion>\r\n",result.sysInfo.version.c_str());
	fprintf(file,"\t</SysInfo>\r\n");

	fprintf(file,"\t<Tests>\r\n");
	for (size_t i=0;i<result.tests.size();i++)
	{
		CSelfTest::Test test = result.tests[i];
		fprintf(file,"\t\t<Test ID=\"%s\">\r\n",test.id.c_str());
		fprintf(file,"\t\t\t<Description>%s</Description>\r\n",test.description.c_str());
		for (size_t j=0;j<test.details.size();j++)
		{
			fprintf(file,"\t\t\t<Detail>%s</Detail>\r\n",test.details[j].c_str());
		}
		for (size_t j=0;j<test.differences.size();j++)
		{
			fprintf(file,"\t\t\t<Difference>\r\n");
			fprintf(file,"\t\t\t\t<ID1>%s</ID1>\r\n",test.differences[j].id1.c_str());
			fprintf(file,"\t\t\t\t<ID2>%s</ID2>\r\n",test.differences[j].id2.c_str());
			fprintf(file,"\t\t\t\t<Notice>%s</Notice>\r\n",test.differences[j].notice.c_str());
			fprintf(file,"\t\t\t\t<Data1>%s</Data1>\r\n",test.differences[j].data1.c_str());
			fprintf(file,"\t\t\t\t<Data2>%s</Data2>\r\n",test.differences[j].data2.c_str());
			fprintf(file,"\t\t\t</Difference>\r\n");
		}
		fprintf(file,"\t\t\t<Success>%s</Success>\r\n",((test.success)?"true":"false"));
		fprintf(file,"\t\t</Test>\r\n");
	}
	fprintf(file,"\t</Tests>\r\n");
	fprintf(file,"</SelfTest>\r\n");

	fflush(file);
	fclose(file);
}

void CSelfTestRunner::DisplayResults()
{
	bool success = (result.summary.testspassed == (result.summary.testspassed+result.summary.testsfailed));
    CString szMessage;
    szMessage.Format(_T("%u tests passed of %u."),result.summary.testspassed,result.summary.testspassed+result.summary.testsfailed);
	UINT uType = ((success)?MB_OK:(MB_OK|MB_ICONEXCLAMATION));
    ::MessageBox( NULL, szMessage, L"Speech Analyzer Self Test", uType);

    // open selftest.xml if some tests failed
    if (!result.summary.testsfailed) return;

    wstring szCommand = L"c:\\windows\\notepad.exe";
    wstring szRestOfCommand = L"";

    HKEY hXML = NULL;
	LSTATUS status = RegOpenKeyEx( HKEY_CLASSES_ROOT, L".xml\\OpenWithProgids", 0, KEY_QUERY_VALUE, &hXML);
	if (status==ERROR_SUCCESS)
	{
		DWORD dwIndex = 0;
		wchar_t name[16384];
		wmemset( name, 0, _countof(name));
		BYTE buffer[16384];
		memset( buffer, 0, _countof(buffer));
		DWORD nameSize = _countof(name);
		DWORD valueSize = _countof(buffer);
		DWORD type = 0;
		status = RegEnumValueW( hXML, dwIndex, name, &nameSize, NULL, &type, buffer, &valueSize);
		if (status==ERROR_SUCCESS)
		{
			if (type==REG_SZ)
			{
				wchar_t subKey[16384];
				swprintf_s(subKey,_countof(subKey),L"%s\\shell\\open\\command",name);
				HKEY hOpen = NULL;
				status = RegOpenKeyW( HKEY_CLASSES_ROOT, subKey, &hOpen);
				if (status==ERROR_SUCCESS)
				{
					dwIndex = 0;
					wmemset( name, 0, _countof(name));
					nameSize = _countof(name);
					DWORD bufSize = _countof(buffer);
					type = 0;
					status = RegEnumValueW( hOpen, dwIndex, name, &nameSize, NULL, &type, buffer, &bufSize);
					if (status==ERROR_SUCCESS)
					{
						wchar_t * pCmd = (wchar_t*)buffer;
						if (wcslen(pCmd)!=0)
						{
							szCommand = pCmd;
						}
					}
				}
			}
		}
	}
	if (szCommand[0]=='\"')
	{
		szCommand = szCommand.substr(1);
		size_t nEnd = szCommand.find(_T("\""));
		if (nEnd!= wstring::npos)
		{
			szRestOfCommand = szCommand.substr(nEnd+1);
			szCommand = szCommand.substr(0,nEnd);

			nEnd = szRestOfCommand.find(L"\"%1\"");
			if (nEnd!=wstring::npos)
			{
				wstring a = szRestOfCommand.substr(0,nEnd);
				wstring b = szRestOfCommand.substr(nEnd+4);
				szRestOfCommand = a.append(b);
			}
			while (szRestOfCommand[0]==' ')
			{
				szRestOfCommand = szRestOfCommand.substr(1);
			}
		}
	}
	else
	{
		size_t nEnd = szCommand.find(_T(" /"));
		if (nEnd!= wstring::npos)
		{
			szRestOfCommand = szCommand.substr(nEnd);
			szCommand = szCommand.substr(0,nEnd);
		}
	}

	if (m_szLogFileName[0]!='\"')
	{
		wstring temp;
		temp.append(L"\"");
		temp.append(m_szLogFileName);
		temp.append(L"\"");
		m_szLogFileName = temp;
	}

	if (szRestOfCommand.length()==0)
	{
		_wspawnl(_P_NOWAIT, szCommand.c_str(), szCommand.c_str(), m_szLogFileName.c_str(), NULL);
	}
	else
	{
		_wspawnl(_P_NOWAIT, szCommand.c_str(), szCommand.c_str(), szRestOfCommand.c_str(), m_szLogFileName.c_str(), NULL);
	}
}

//
// FileCompare()
//
// Compares two files. If they are different, logs the difference
// Returns -1 if one of the files doesn't exist or if the file sizes are different.
// Returns 0 if number of bytes differing is <= nTolerableDifferences
// Otherwise returns the number of bytes in the file which are different.
//
// If file2 is NULL, uses the "SelfTest\*.tmp" file, where * corresponds to the test name
//
#define BUFFERSIZE 1024
int CSelfTestRunner::FileCompare( CSelfTest::Test & testresult, LPCTSTR szFileOne, long nTolerableDifferences)
{
	CSaString temp;
	temp.setUtf8(testresult.id.c_str());

	wstring szFileTwo;
	szFileTwo = m_szTestDataPath.c_str();
	szFileTwo.append(temp);
	szFileTwo.append(L".tmp");
	szFileTwo = NormalizePath(szFileTwo.c_str());

	return FileCompare( testresult, szFileOne, nTolerableDifferences, szFileTwo.c_str());
}

int CSelfTestRunner::FileCompare( CSelfTest::Test & testresult, LPCTSTR szFileOne, long nTolerableDifferences, LPCTSTR szFileTwo)
{
    long nFileDifferences = 0;
    long nFirstDifference = -1;
    
	if (!FileExists(szFileOne))
    {
        char msg[512];
		sprintf_s(msg,512,"Unable to open file %s", szFileOne);
		testresult.details.push_back(msg);
    }
	
	if (!FileExists(szFileTwo))
    {
		char msg[512];
		sprintf_s(msg,512,"Unable to open file ");

		CSaString szMessage;
		szMessage.setUtf8(msg);
		szMessage.Append(szFileTwo);

        wstring szBenchMessage = szFileTwo;
		szBenchMessage.append(L" does not exist.\r\n\nAre you intending to create the benchmark at this time?");

		if (::MessageBox( NULL, szBenchMessage.c_str(), szMessage, MB_YESNO)==IDYES)
        {
            CFile File1(szFileOne,CFile::modeRead|CFile::typeBinary);
            CFile File2(szFileTwo,CFile::modeWrite|CFile::modeCreate|CFile::typeBinary);
            char Buffer[BUFFERSIZE];

            for (;;)
            {
                UINT nBytesRead = File1.Read(Buffer,BUFFERSIZE);
                if (!nBytesRead)
                {
                    File1.Close();
                    File2.Flush();
                    File2.Close();
                    return 0;
                }
                File2.Write(Buffer,nBytesRead);
            }
        }
        else
        {
			testresult.details.push_back(Utf8(szMessage));
        }
    }
    if ((!FileExists(szFileOne))||(!FileExists(szFileTwo)))
    {
		testresult.details.push_back("Neither file exists.");
        return -1;
    }

    CFile File1(szFileOne,CFile::modeRead|CFile::typeBinary);
    CFile File2(szFileTwo,CFile::modeRead|CFile::typeBinary);
    char Buffer1[BUFFERSIZE], Buffer2[BUFFERSIZE];


    ULONGLONG dwFile1Len=File1.GetLength();
    ULONGLONG dwFile2Len=File2.GetLength();
    if (dwFile1Len!=dwFile2Len)   // Files are different length
    {
        char msg[512];
		sprintf_s(msg,512,"%s has a length of %lu",Utf8(szFileOne).c_str(),dwFile1Len);
		testresult.details.push_back(msg);
        sprintf_s(msg,512,"%s has a length of %lu",Utf8(szFileTwo).c_str(),dwFile2Len);
		testresult.details.push_back(msg);

        File1.Close();
        File2.Close();
        return -1;
    }


    for (UINT nChunkNumber = 0;; ++nChunkNumber)
    {
        UINT nRead = File1.Read((void *)Buffer1,BUFFERSIZE);
        nRead = File2.Read((void *)Buffer2,BUFFERSIZE);

        for (register unsigned i=1; i<nRead; ++i)
        {
            if (Buffer1[i-1]!=Buffer2[i-1])   // Files have different content
            {
                if (nFirstDifference == -1)
                {
                    nFirstDifference = (((DWORD)nChunkNumber)*BUFFERSIZE)+i;
                }
                ++nFileDifferences;
            }
        }

        if (nRead!=BUFFERSIZE)
        {
            break;
        }
    }
    if ((nFirstDifference != -1) && (nFileDifferences > nTolerableDifferences))
    {
		char buffer[512];
		sprintf_s( buffer, 512, "Files differ in %ld of %lu bytes",nFileDifferences,File1.GetLength());
        testresult.details.push_back( buffer);
        HexDataCompare( testresult, File1, File2, nFirstDifference);

        File1.Close();
        File2.Close();
        return nFileDifferences;
    }
    File1.Close();
    File2.Close();
    return 0;
}

/***************************************************************************/
// CSelfTestRunner::MessageLoop Do windows message loop
// This function enables key down message to come through while
// processing.
/***************************************************************************/
void CSelfTestRunner::MessageLoop(DWORD dwMilliSeconds)
{
    DWORD dwStartTime=::GetTickCount();
    BOOL bDoingBackgroundProcessing = TRUE;

    while (bDoingBackgroundProcessing && (dwStartTime+dwMilliSeconds > ::GetTickCount()))
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (!AfxGetApp()->PumpMessage())
            {
                bDoingBackgroundProcessing = FALSE;
                //::PostQuitMessage( );
                break;
            }
        }
        // let MFC do its idle processing
        LONG lIdle = 0;
        while (AfxGetApp()->OnIdle(lIdle++))
		{
		}
        // Perform some background processing here
        // using another call to OnIdle
    }
}

void CSelfTestRunner::EmptyClipboard()
{
    if (::OpenClipboard(*AfxGetMainWnd()))
    {
        ::EmptyClipboard();
        ::CloseClipboard();
    }
}

bool CSelfTestRunner::CheckClipboard(UINT nFormat)   //CF_BITMAP
{
	BOOL bReturn = FALSE;
    if (::OpenClipboard(*AfxGetMainWnd()))
    {
        bReturn = ::IsClipboardFormatAvailable(nFormat);
        ::CloseClipboard();
    }
    return (bReturn==TRUE);
}

string ToHex( unsigned char byte)
{
    char szHexValue[3];
	memset(szHexValue,0,_countof(szHexValue));
    szHexValue[0] = (char)(byte/16);
    if (szHexValue[0]<=9)
    {
        szHexValue[0] += '0';
    }
    else
    {
        szHexValue[0]+=('A'-10);
    }
    szHexValue[1] = (char)(byte%16);
    if (szHexValue[1]<=9)
    {
        szHexValue[1] += '0';
    }
    else
    {
        szHexValue[1]+=('A'-10);
    }

	string result = szHexValue;
	return result;
}

bool CSelfTestRunner::HexDataCompare( CSelfTest::Test & testresult, CFile & file1, CFile & file2, UINT highlightPosition)
{
	CString szOutString;
    UINT beginPosition=(highlightPosition>7)?highlightPosition-8:0;
    unsigned char buffer[16] = "";
    register UINT i=0;

	CSelfTest::Test::FileDifference difference;

	difference.id1 = Utf8(file1.GetFileName());
	difference.id2 = Utf8(file2.GetFileName());

    CString szNotice;
    szNotice.Format(_T("Files diverge at byte %lu."),highlightPosition);
	difference.notice = Utf8(szNotice);

	string data1;
    file1.Seek(beginPosition,SEEK_SET);
    UINT nRead = file1.Read(buffer,16);
    for (i=0; i<nRead; ++i)
    {
        if ((i+beginPosition)==(highlightPosition-1))
        {
            data1.append("*");
        }
		data1.append( ToHex(buffer[i]).c_str());
        if ((i+beginPosition)==(highlightPosition-1))
        {
            data1.append("*");
        }
        if (i<(nRead-1))
        {
            data1.append(" ");
        }
    }
	difference.data1 = data1;

	string data2;
    file2.Seek(beginPosition,SEEK_SET);
    nRead = file2.Read(buffer,16);
    for (i=0; i<nRead; ++i)
    {
        if ((i+beginPosition)==highlightPosition-1)
        {
            data2.append("*");
        }
		data2.append(ToHex(buffer[i]).c_str());
        if ((i+beginPosition)==highlightPosition-1)
        {
            data2.append("*");
        }
        if (i<nRead-1)
        {
            data2.append(" ");
        }
    }
	difference.data2 = data2;

	testresult.differences.push_back(difference);

    return TRUE;
}

LPCTSTR CSelfTestRunner::GetTestDataFolder()
{
	return m_szTestDataPath.c_str();
}

LPCTSTR CSelfTestRunner::GetTestOutputFolder()
{
	return m_szTestOutputPath.c_str();
}

LPCTSTR CSelfTestRunner::GetTempPathW()
{
	return m_szTempPath.c_str();
}

void Fail( LPCSTR msg)
{
	throw exception(msg);
}

void Assert( bool cond, LPCSTR msg)
{
	if (!cond) throw exception(msg);
}

class CTestDoc
{
public:
	CTestDoc( CSelfTestRunner & runner, LPCTSTR filename)
	{
		CString szTestFileName;
		szTestFileName.Append( runner.GetTestDataFolder());
		szTestFileName.Append(filename);
		if (!FileExists(szTestFileName))
		{
			throw std::exception("test file not open");
		}

		CWinApp * pApp = AfxGetApp();
		pDoc = (CSaDoc *)pApp->OpenDocumentFile(szTestFileName);
		pDoc->EnableBackgroundProcessing(FALSE);
		pMain = (CMainFrame*)AfxGetMainWnd();
		pView = (CSaView *)pMain->MDIGetActive()->GetActiveView();
		pView->ChangeCursorAlignment(ALIGN_AT_SAMPLE);

	    EmptyClipboard();
	}

	~CTestDoc()
	{
        BOOL success = pMain->SendMessage(WM_COMMAND,ID_FILE_CLOSE,0);
		Assert(success==TRUE,"Failed to close document");
        pDoc = NULL;
		pView = NULL;
		pMain = NULL;
	}

	CMainFrame * pMain;
	CSaDoc * pDoc;
	CSaView * pView;
};

void ValidateProcess( short int nResult)
{
	Assert(nResult!=PROCESS_ERROR,"processing failed");
	Assert(nResult!=PROCESS_CANCELED,"process was cancelled");
}

void ValidateFileCompare( int result)
{
	Assert(result==0,"FileCompare failed");
}

bool Confirm( LPCTSTR msg)
{
	int result = ::MessageBox(NULL,msg,L"Speech Analyzer Self Test",MB_YESNO|MB_ICONQUESTION);
	return (result==IDYES);
}

/******************************************************************************
* TEST IMPLEMENTATIONS
******************************************************************************/
void Test001( CSelfTestRunner & runner, CSelfTest::Test & )
{
	wstring szTestFileName;
	szTestFileName.append(runner.GetTestDataFolder());
	szTestFileName.append(L"chfrench.wav");
	Assert(FileExists( szTestFileName.c_str()),"test file not open");
	CWinApp * pApp = AfxGetApp();
	CSaDoc * pDoc = (CSaDoc *)pApp->OpenDocumentFile(szTestFileName.c_str());
	Assert(pDoc!=NULL,"failed to open document");
    pDoc->EnableBackgroundProcessing(FALSE);
	CMainFrame * pMain = (CMainFrame*)AfxGetMainWnd();
    CSaView * pView = (CSaView *)pMain->MDIGetActive()->GetActiveView();
	Assert(pView!=NULL,"failed to open view");
}

// manually set the cursor alignment to sample alignment
void Test002( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
	Assert(doc.pView->GetCursorAlignment()==ALIGN_AT_SAMPLE,"failed to align at sample");
}

void Test003( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc( runner, L"chfrench.wav");

	bool passed = true;
    const CUttParm * pUP = doc.pDoc->GetUttParm();
	if (pUP->nCritLoud != 6912)
	{
		passed = false;
		test.details.push_back("Critical loudness incorrectly set.");
	}
	if (pUP->nMaxChange != 11)
	{
		passed = false;
		test.details.push_back("Maximum change incorrectly set.");
	}
	if (pUP->nMaxFreq != 260)
	{
		passed = false;
		test.details.push_back("Maximum frequency incorrectly set.");
	}
	if (pUP->nMaxInterp != 7)
	{
		passed = false;
		test.details.push_back("Maximum interpolation incorrectly set.");
	}
	if (pUP->nMinFreq != 70)
	{
		passed = false;
		test.details.push_back("Minimum frequency incorrectly set.");
	}
	if (pUP->nMinGroup != 6)
	{
		passed = false;
		test.details.push_back("Minimum group incorrectly set.");
	}
	Assert(passed,"Utterance failed");
}

//
// Test Utterance Parameters of CHFRENCH.WAV file
//
void Test004( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");

    bool success = true;
	CFmtParm fmtParm;
    doc.pDoc->GetFmtParm(fmtParm,false);
    if (fmtParm.dwAvgBytesPerSec != 44100)
    {
        success=false;
		test.details.push_back("Average bytes per second incorrectly set.");
    }
    if (fmtParm.dwSamplesPerSec != 22050)
    {
        success=false;
        test.details.push_back("Samples per second incorrectly set.");
    }
    if (fmtParm.wBitsPerSample != 16)
    {
        success=false;
        test.details.push_back("Bits per sample incorrectly set.");
    }
    if (fmtParm.wBlockAlign != 2)
    {
        success=false;
        test.details.push_back( "BlockAlign incorrectly set.");
    }
    if (fmtParm.wChannels != 1)
    {
        success=false;
        test.details.push_back( "Channels incorrectly set.");
    }
    if (fmtParm.wTag != 1)
    {
        success=false;
        test.details.push_back( "Tag incorrectly set.");
    }
	Assert(success,"FmtParm failed");
}

void Test005( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    doc.pView->SetStartCursorPosition(30714,SNAP_LEFT);
    doc.pView->SetStopCursorPosition(35046,SNAP_RIGHT);
    DWORD dwStart = doc.pView->GetStartCursorPosition();
    DWORD dwStop  = doc.pView->GetStopCursorPosition();
    Assert(dwStart==30714,"Failed to set start cursor position");
	Assert(dwStop!=35046,"Failed to set stop cursor position");
}

void Test006( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessZCross * pZCross = doc.pDoc->GetZCross();
	// attempt to process data
    ValidateProcess(LOWORD(pZCross->Process( &runner, doc.pDoc))); 
	ValidateFileCompare(runner.FileCompare( test, pZCross->GetProcessFileName()));
}

void Test007( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessPitch * pPitch = doc.pDoc->GetPitch();
	// attempt to process data
    ValidateProcess(LOWORD(pPitch->Process( &runner, doc.pDoc))); 
	ValidateFileCompare(runner.FileCompare( test, pPitch->GetProcessFileName()));
}

void Test008( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");

    CProcessCustomPitch * pCustomPitch = doc.pDoc->GetCustomPitch();
    ValidateProcess(LOWORD(pCustomPitch->Process( &runner, doc.pDoc))); // process data
    ValidateFileCompare(runner.FileCompare( test,  pCustomPitch->GetProcessFileName()));
}

void Test009( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessSmoothedPitch * pSmoothedPitch = doc.pDoc->GetSmoothedPitch();
    ValidateProcess(LOWORD(pSmoothedPitch->Process( &runner, doc.pDoc))); // process data
    ValidateFileCompare(runner.FileCompare(test,pSmoothedPitch->GetProcessFileName()));
}

void Test010( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessGrappl * pAutoPitch = doc.pDoc->GetGrappl();
    ValidateProcess(LOWORD(pAutoPitch->Process( &runner, doc.pDoc))); // process data
	ValidateFileCompare(runner.FileCompare( test,  pAutoPitch->GetProcessFileName()));

	UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_GRAPITCH,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
    runner.MessageLoop(3000);
}

void Test011( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessFragments * pFragment = doc.pDoc->GetFragments();
    pFragment->SetDataInvalid();
    doc.pDoc->EnableBackgroundProcessing(FALSE);
    pFragment->RestartProcess();
    pFragment->Process( &runner, (CSaDoc *)doc.pDoc); // process data
    // wait for idle loop to finish
    while (!pFragment->IsDataReady())
    {
        runner.MessageLoop(10);
    }
	ValidateFileCompare(runner.FileCompare( test,  pFragment->GetProcessFileName()));
}

void Test012( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessLoudness * pLoudness = doc.pDoc->GetLoudness();
    ValidateProcess(LOWORD(pLoudness->Process(&runner, doc.pDoc))); // process data
	ValidateFileCompare(runner.FileCompare( test,  pLoudness->GetProcessFileName()));
}

void Test013( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessMelogram * pMelogram = doc.pDoc->GetMelogram();
    ValidateProcess(LOWORD(pMelogram->Process(&runner, doc.pDoc))); // process data
    ValidateFileCompare(runner.FileCompare( test, pMelogram->GetProcessFileName()));
}

void Test014( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessChange * pChange = doc.pDoc->GetChange();
    ValidateProcess(LOWORD(pChange->Process(&runner,doc.pDoc))); // process data
    ValidateFileCompare(runner.FileCompare( test, pChange->GetProcessFileName()));
}

void Test015( CSelfTestRunner & runner, CSelfTest::Test & test)
{

	CTestDoc doc(runner,L"chfrench.wav");
    CProcessRaw * pRaw = doc.pDoc->GetRaw();
    ValidateProcess(LOWORD(pRaw->Process(&runner, doc.pDoc))); // process data
	ValidateFileCompare(runner.FileCompare( test, pRaw->GetProcessFileName()));
}

void Test016( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessSpectrogram * pSpectrogram = (CProcessSpectrogram *)doc.pDoc->GetSpectrogram(TRUE); // get pointer to spectrogram object
    pSpectrogram->SetDataInvalid();
    // set spectrogram parameters
    CSpectroParm cSpectroParm = pSpectrogram->GetSpectroParm();
    CSpectroParm * pSpectroParm = &cSpectroParm;
    pSpectroParm->nResolution = 2;
    pSpectroParm->nColor = 0;
    pSpectroParm->nOverlay = 0;
    pSpectroParm->bShowPitch = 1;
    pSpectroParm->bShowF1 = 1;
    pSpectroParm->bShowF2 = 1;
    pSpectroParm->bShowF3 = 1;
    pSpectroParm->bShowF4 = 1;
    pSpectroParm->bShowF5andUp = 0;
    pSpectroParm->bSmoothFormantTracks = 1;
    pSpectroParm->nFrequency = 8000;
    pSpectroParm->nMinThreshold = 1;
    pSpectroParm->nMaxThreshold = 233;
    pSpectroParm->bSmoothSpectra = 1;
    pSpectrogram->SetSpectroParm(*pSpectroParm);
    // process spectrogram
	// some arbitrary plot width
	// some arbitrary plot height (ignored)
    ValidateProcess(pSpectrogram->Process( &runner, doc.pDoc, doc.pView, 752, 500, 0, 1));
	ValidateFileCompare(runner.FileCompare( test, pSpectrogram->GetProcessFileName()/*,7*/ ));
}

//
// this process does not create a temp file, so we have to create one
//
void Test017( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");

	// run the fragmenter
	CProcessFragments * pFragmenter = doc.pDoc->GetFragments();
    pFragmenter->SetDataInvalid();
    doc.pDoc->EnableBackgroundProcessing(FALSE);
    pFragmenter->RestartProcess();
    pFragmenter->Process( &runner, (CSaDoc *)doc.pDoc); // process data
    // wait for idle loop to finish
    while (!pFragmenter->IsDataReady())
    {
        runner.MessageLoop(10);
    }

    CProcessSpectrum * pSpectrum = doc.pDoc->GetSpectrum();

    CSpectrumParm * stParmSpec = pSpectrum->GetSpectrumParms();
    stParmSpec->nSmoothLevel = 3;
    stParmSpec->nPeakSharpFac = 3;
    pSpectrum->SetSpectrumParms(stParmSpec);

    SSpectProcSelect SpectraSelected;
    SpectraSelected.bCepstralSpectrum = TRUE;    // turn off to reduce processing time
    SpectraSelected.bLpcSpectrum = TRUE;          // use Lpc method for estimating formants

    ValidateProcess(LOWORD( pSpectrum->Process( &runner, doc.pDoc, 30870, 4410, SpectraSelected))); // process data

	DWORD dwDataSize = pSpectrum->GetDataSize();
    void * pData = (char *) pSpectrum->GetProcessedData(0);

    CString szFileName;
	szFileName.Append(runner.GetTempPath());
	szFileName.Append(L"Spectrum.tmp");

    CFile OutputFile(szFileName,CFile::modeCreate|CFile::modeWrite);
    OutputFile.Write(pData,dwDataSize);
    OutputFile.Flush();
    OutputFile.Close();

	ValidateFileCompare(runner.FileCompare( test, szFileName/*,1037*/ ));
	// delete the file now
    _tunlink(szFileName); 
}

void Test018( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");

	// run the fragmenter
	CProcessFragments * pFragmenter = doc.pDoc->GetFragments();
    pFragmenter->SetDataInvalid();
    doc.pDoc->EnableBackgroundProcessing(FALSE);
    pFragmenter->RestartProcess();
    pFragmenter->Process( &runner, (CSaDoc *)doc.pDoc); // process data
    // wait for idle loop to finish
    while (!pFragmenter->IsDataReady())
    {
        runner.MessageLoop(10);
    }

    CProcessFormants * pFormants = doc.pDoc->GetFormants();

    SSpectProcSelect SpectraSelected;
    SpectraSelected.bCepstralSpectrum = TRUE;    // turn off to reduce processing time
    SpectraSelected.bLpcSpectrum = TRUE;          // use Lpc method for estimating formants

    ValidateProcess(LOWORD(pFormants->Process( &runner, doc.pDoc,TRUE,30870,4410,SpectraSelected)));
	ValidateFileCompare(runner.FileCompare( test, pFormants->GetProcessFileName()/*,20*/ ));
}

//
// this process does not create a temp file, so we have to create one
//
void Test019( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessPOA * pPOA = doc.pDoc->GetPOA();
    ValidateProcess(LOWORD(pPOA->Process( &runner, doc.pDoc,(DWORD)30870,(DWORD)35280))); // process data

    SVocalTractModel * pData = (SVocalTractModel *) pPOA->GetProcessedData(0);
	CString szFileName;
	szFileName.Append(runner.GetTempPath());
	szFileName.Append(L"PointOfArticulation.tmp");

    CFile OutputFile(szFileName,CFile::modeCreate|CFile::modeWrite);
    OutputFile.Write(&pData->dErrorRatio,sizeof(pData->dErrorRatio));
    OutputFile.Write(&pData->nNormCrossSectAreas,sizeof(pData->nNormCrossSectAreas));
    for (register int i=0; i<pData->nNormCrossSectAreas; ++i)
    {
        OutputFile.Write(&pData->dNormCrossSectArea[i],sizeof(double));
    }
    OutputFile.Flush();
    OutputFile.Close();

    ValidateFileCompare(runner.FileCompare( test, szFileName/*,33*/ ));
    _tunlink(szFileName);
}

void Test020( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench.wav");
    CProcessTonalWeightChart * pTWC = doc.pDoc->GetTonalWeightChart();
    CProcessMelogram * pMelogram = (CProcessMelogram *)doc.pDoc->GetMelogram(); // get pointer to melogram object
    int nLevel = 0, nProgress = 0;
    ValidateProcess(LOWORD(pMelogram->Process( &runner, doc.pDoc, nProgress, ++nLevel)));
    DWORD dwMelDataSize = pMelogram->GetDataSize() * 2; // size of melogram data
    DWORD wSmpSize = doc.pDoc->GetSampleSize();
    DWORD dwRawDataSize = doc.pDoc->GetDataSize(); // size of raw data
    double fScaleFactor = (double)dwRawDataSize / (double)dwMelDataSize;
    DWORD dwFrameStart = (DWORD)((double)doc.pView->GetStartCursorPosition() / fScaleFactor) & ~1; // must be multiple of two
    DWORD dwFrameSize  = ((DWORD)((double)doc.pView->GetStopCursorPosition() / fScaleFactor) & ~1) - dwFrameStart + wSmpSize;
    if (doc.pView->GetStaticTWC())
    {
        dwFrameStart = 0;
        dwFrameSize  = dwMelDataSize;
    }
    ValidateProcess(LOWORD(pTWC->Process( &runner,doc.pDoc,dwFrameStart,dwFrameSize, 37,59)));  // process data
    ValidateFileCompare(runner.FileCompare( test, pTWC->GetProcessFileName()));
}

void Test022( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    // It would be nice if we could send the keys to the menu (Alt+F to pull down file menu, etc.)
    // bTestSuccess = m_pMain->SendMessage(WM_CHAR, 'F', 0x20000000);
    // ...for now we can just send the message
    EmptyClipboard();
    doc.pMain->SendMessage(WM_COMMAND,ID_GRAPHS_SCREENCOPY,0);
	Assert(runner.CheckClipboard(CF_BITMAP),"Screen copy to clipboard failed");
    EmptyClipboard();
}

void Test023( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    EmptyClipboard();
    doc.pMain->SendMessage(WM_COMMAND,ID_GRAPHS_GRAPHSCOPY,0);
    Assert(runner.CheckClipboard(CF_BITMAP),"Graph copy to clipboard failed");
    EmptyClipboard();
}

void Test024( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    EmptyClipboard();
    doc.pMain->SendMessage(WM_COMMAND,ID_GRAPHS_WINDOWCOPY,0);
    Assert(runner.CheckClipboard(CF_BITMAP),"Window copy to clipboard failed");
    EmptyClipboard();
}

void Test025( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    runner.MessageLoop(1);
    doc.pMain->SendMessage(WM_COMMAND,ID_EDIT_SELECTWAVEFORM,0);
    runner.MessageLoop(1);
    doc.pMain->SendMessage(WM_COMMAND,ID_EDIT_CUT,0);
    runner.MessageLoop(1);
    doc.pMain->SendMessage(WM_COMMAND,ID_EDIT_PASTE,0);
    runner.MessageLoop(1);
    doc.pMain->SendMessage(WM_COMMAND,ID_EDIT_UNDO,0);
    runner.MessageLoop(1);
}

void Test026( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    doc.pView->SetStartCursorPosition(0,SNAP_LEFT);
    doc.pView->SetStopCursorPosition(doc.pDoc->GetDataSize(),SNAP_RIGHT);

    DWORD dwStart = doc.pView->GetStartCursorPosition();
    DWORD dwStop  = doc.pView->GetStopCursorPosition();
    DWORD dwSize  = doc.pDoc->GetDataSize();

	CFmtParm fmtParm;
    doc.pDoc->GetFmtParm(fmtParm,false);

	Assert(dwStart!=0,"Expected start position to be non-zero");
	Assert(dwStop==(dwSize-(fmtParm.wBitsPerSample/8)),"stop position is unexpected");
}

void VisualTest100( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_ZCROSS,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void VisualTest101( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_PITCH,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void VisualTest102( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_CHPITCH,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void VisualTest103( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_SMPITCH,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void VisualTest104( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_GRAPITCH,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void VisualTest105( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_LOUDNESS,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void VisualTest106( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_MELOGRAM,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void VisualTest107( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_CHANGE,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void VisualTest108( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_SPECTROGRAM,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void VisualTest109( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
	UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_SPECTRUM,0,0,0,0,0,0,0,0};
	doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void VisualTest110( CSelfTestRunner & runner, CSelfTest::Test & )
{
	CTestDoc doc(runner,L"chfrench.wav");
    UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_POA,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
	Assert(Confirm(L"Does this graph seem correct?"),"confirmation failure");
}

void Test00a( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench_frag_test_22khz.wav");
	doc.pView->ChangeCursorAlignment(ALIGN_AT_ZERO_CROSSING);

	DWORD start = doc.pDoc->GetBytesFromTime(0.012);
	DWORD stop = doc.pDoc->GetBytesFromTime(0.054);
	doc.pView->SetStartStopCursorPosition(start,stop);

    CProcessGrappl * pAutoPitch = doc.pDoc->GetGrappl();
	DWORD dataSize = doc.pDoc->GetDataSize();
	Assert(dataSize==2962,"unexpected data size");
	DWORD numSamples = doc.pDoc->GetNumSamples();
	Assert(numSamples==1481,"wrong number of samples");

    ValidateProcess(LOWORD(pAutoPitch->Process( &runner, doc.pDoc)));

	string path;
	path.append(Utf8(runner.GetTestOutputFolder()).c_str());
	path.append("\\");
	path.append(test.id);
	path.append(".txt");
	pAutoPitch->Dump(path.c_str());

	UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_GRAPITCH,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
    runner.MessageLoop(3000);

	ValidateFileCompare( runner.FileCompare( test,  pAutoPitch->GetProcessFileName()));
	Assert(Confirm(L"Is this graph correct?"),"confirmation failure");
}

void Test00b( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench_frag_test_96khz.wav");
	doc.pView->ChangeCursorAlignment(ALIGN_AT_ZERO_CROSSING);

	DWORD start = doc.pDoc->GetBytesFromTime(0.012);
	DWORD stop = doc.pDoc->GetBytesFromTime(0.054);
	doc.pView->SetStartStopCursorPosition(start,stop);

    CProcessGrappl * pAutoPitch = doc.pDoc->GetGrappl();
	ValidateProcess(LOWORD(pAutoPitch->Process( &runner, doc.pDoc)));

	string path;
	path.append(Utf8(runner.GetTestOutputFolder()).c_str());
	path.append("\\");
	path.append(test.id);
	path.append(".txt");
	pAutoPitch->Dump(path.c_str());

	UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_GRAPITCH,0,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
    runner.MessageLoop(3000);

	ValidateFileCompare( runner.FileCompare( test,  pAutoPitch->GetProcessFileName()));
	Assert(Confirm(L"Is this graph correct?"),"confirmation failure");
}

void Test00c( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench_frag_test_22khz.wav");
	doc.pView->ChangeCursorAlignment(ALIGN_AT_ZERO_CROSSING);

	DWORD start = doc.pDoc->GetBytesFromTime(0.012);
	DWORD stop = doc.pDoc->GetBytesFromTime(0.054);
	doc.pView->SetStartStopCursorPosition(start,stop);

	CProcessHilbert * pHilbert = doc.pDoc->GetHilbert();
	DWORD dataSize = doc.pDoc->GetDataSize();
	Assert(dataSize==2962,"unexpected data size");
	DWORD numSamples = doc.pDoc->GetNumSamples();
	Assert(numSamples==1481,"wrong number of samples");

    ValidateProcess(LOWORD(pHilbert->Process( &runner, doc.pDoc)));

	string path;
	path.append(Utf8(runner.GetTestOutputFolder()).c_str());
	path.append("\\");
	path.append(test.id);
	path.append(".txt");
	pHilbert->Dump(path.c_str());

	UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_GRAPITCH,IDD_SPECTROGRAM,0,0,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
    runner.MessageLoop(3000);

	//ValidateFileCompare( runner.FileCompare( test,  pHilbert->GetProcessFileName()));
	Assert(Confirm(L"Is this graph correct?"),"confirmation failure");
}

void Test00d( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench_frag_test_96khz.wav");
	doc.pView->ChangeCursorAlignment(ALIGN_AT_ZERO_CROSSING);

	DWORD start = doc.pDoc->GetBytesFromTime(0.012);
	DWORD stop = doc.pDoc->GetBytesFromTime(0.054);
	doc.pView->SetStartStopCursorPosition(start,stop);

	CProcessHilbert * pHilbert = doc.pDoc->GetHilbert();
	ValidateProcess(LOWORD(pHilbert->Process( &runner, doc.pDoc)));

	string path;
	path.append(Utf8(runner.GetTestOutputFolder()).c_str());
	path.append("\\");
	path.append(test.id);
	path.append(".txt");
	pHilbert->Dump(path.c_str());

	UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_GRAPITCH,IDD_SPECTROGRAM,IDD_SPECTRUM,IDD_F2F1,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
    runner.MessageLoop(3000);
	
	Assert(Confirm(L"Is this graph correct?"),"confirmation failure");
}

void Test00e( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench_frag_test_22khz.wav");
	doc.pView->ChangeCursorAlignment(ALIGN_AT_ZERO_CROSSING);

	DWORD start = doc.pDoc->GetBytesFromTime(0.012);
	DWORD stop = doc.pDoc->GetBytesFromTime(0.054);
	doc.pView->SetStartStopCursorPosition(start,stop);

	CProcessFormantTracker * pProcess = doc.pDoc->GetFormantTracker();
	ValidateProcess(LOWORD(pProcess->Process( &runner, doc.pDoc)));

	string path;
	path.append(Utf8(runner.GetTestOutputFolder()).c_str());
	path.append("\\");
	path.append(test.id);
	path.append(".txt");
	pProcess->Dump(path.c_str());

	UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_GRAPITCH,IDD_SPECTROGRAM,IDD_SPECTRUM,IDD_F2F1,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
    runner.MessageLoop(3000);

	Assert(Confirm(L"Is this graph correct?"),"confirmation failure");
}

void Test00f( CSelfTestRunner & runner, CSelfTest::Test & test)
{
	CTestDoc doc(runner,L"chfrench_frag_test_96khz.wav");
	doc.pView->ChangeCursorAlignment(ALIGN_AT_ZERO_CROSSING);

	DWORD start = doc.pDoc->GetBytesFromTime(0.012);
	DWORD stop = doc.pDoc->GetBytesFromTime(0.054);
	doc.pView->SetStartStopCursorPosition(start,stop);

	CProcessFormantTracker * pProcess = doc.pDoc->GetFormantTracker();
	ValidateProcess(LOWORD(pProcess->Process( &runner, doc.pDoc)));

	string path;
	path.append(Utf8(runner.GetTestOutputFolder()).c_str());
	path.append("\\");
	path.append(test.id);
	path.append(".txt");
	pProcess->Dump(path.c_str());

	UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_GRAPITCH,IDD_SPECTROGRAM,IDD_SPECTRUM,IDD_F2F1,0,0,0,0,0};
    doc.pView->OnGraphsTypesPostProcess(GrphIDs,-1);
    runner.MessageLoop(3000);
	
	Assert(Confirm(L"Is this graph correct?"),"confirmation failure");
}

//
// Main testing routine
//
// Each test should begin with a call to StartTest() and end with a call to EndTest()
// The test should be executed only if StartTest() returns TRUE.
//
void CSelfTestRunner::RunTests( )
{
	RunTest( "CHFrench Open", "Open file CHFRENCH.WAV", &Test001);
	RunTest( "CHFrench Set Cursor Alignment", "Set cursor alignment to sample.", &Test002);
	RunTest( "CHFrench UttParm","Test utterance parameters", &Test003);
	RunTest( "CHFrench FmtParm","Test format parameters", &Test004);
	RunTest( "CHFrench CursorPositioning","Position the cursors in voiced region", &Test005);
	RunTest( "CHFrench ZCrossing","Test zero crossing process.", &Test006);
	RunTest( "CHFrench RawPitch","Test raw pitch process.", &Test007);
	RunTest( "CHFrench CustomPitch","Test custom pitch process.", &Test008);
	RunTest( "CHFrench SmoothedPitch","Test smoothed pitch process.", &Test009);
	RunTest( "CHFrench AutoPitch","Test auto pitch process.", &Test010);
	RunTest( "CHFrench Fragment","Test fragment process.", &Test011);
	RunTest( "CHFrench Loudness","Test loudness process.", &Test012);
	RunTest( "CHFrench Melogram","Test melogram process.", &Test013);
	RunTest( "CHFrench Change","Test change process.", &Test014);
	RunTest( "CHFrench Raw","Test raw data process.", &Test015);
	RunTest( "CHFrench Spectrogram","Test spectrogram process.", &Test016);
	RunTest( "CHFrench Spectrum","Test spectrum process.", &Test017);
	RunTest( "CHFrench Formants","Test formants process.", &Test018);
	RunTest( "CHFrench PointOfArticulation","Test point of articulation process.", &Test019);
	RunTest( "CHFrench TWC","Test tonal weighting chart process.", &Test020);

	RunTest( "CHFrench Screen Capture","Test File|Screen Capture|Copy Screen with CHFRENCH.WAV", &Test022);
	RunTest( "CHFrench Graphs Capture","Test File|Screen Capture|Copy Graphs with CHFRENCH.WAV", &Test023);
	RunTest( "CHFrench Window Capture","Test File|Screen Capture|Copy Window with CHFRENCH.WAV", &Test024);
	RunTest( "CHFrench CutPaste","Cut and paste portions of the file", &Test025);
	RunTest( "CHFrench CursorPositioning","Position the cursors at ends of file", &Test026);

	if (Confirm(L"Run visual tests?"))
	{
		RunTest( "CHFrench visual ZCrossing", "Visual check of zero crossing graph", &VisualTest100);
		RunTest( "CHFrench visual RawPitch", "Visual check of raw pitch graph", &VisualTest101);
		RunTest( "CHFrench visual CustomPitch","Visual check of custom pitch graph", &VisualTest102);
		RunTest( "CHFrench visual SmoothedPitch","Visual check of smoothed pitch graph", &VisualTest103);
		RunTest( "CHFrench visual AutoPitch","Visual check of autopitch graph", &VisualTest104);
		RunTest( "CHFrench visual Loudness","Visual check of loudness graph.", &VisualTest105);
		RunTest( "CHFrench visual Melogram","Visual check of melogram graph.", &VisualTest106);
		RunTest( "CHFrench visual Change","Visual check of change graph.", &VisualTest107);
		RunTest( "CHFrench visual Spectrogram","Visual check of spectrogram graph.", &VisualTest108);
		RunTest( "CHFrench visual Spectrum","Visual check of spectrum graph.", &VisualTest109);
		RunTest( "CHFrench visual POA","Visual check of point of articulation graph.", &VisualTest110);
		RunTest( "22khz_grappl", "22khz", &Test00a);
		RunTest( "96khz_grappl", "96khz", &Test00b);
		RunTest( "22khz_hillbert", "22khz", &Test00c);
		RunTest( "96khz_hillbert", "96khz", &Test00d);
		RunTest( "22khz_formanttracker", "22khz", &Test00e);
		RunTest( "96khz_formanttracker", "96khz", &Test00f);
	}
}

