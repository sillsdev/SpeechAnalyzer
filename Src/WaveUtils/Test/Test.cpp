#include <SDKDDKVer.h>
#include "CppUnitTest.h"
#include <Windows.h>
#include <SDKDDKVer.h>
#include <conio.h>
#include <string>
#include <FileUtils.h>
#include "Test.h"

#pragma comment( lib, "winmm")
#pragma comment( lib, "waveutils")
#pragma comment( lib, "fileutils")

int round2Int(double value) {
    return (int)floor(value + 0.5);
}

wstring buildResultPath(LPCTSTR filename) {
    FileUtils::CreateFolder(L"\\Working\\SIL\\MSEA\\test_samples\\out\\");
    wstring result;
    result.append(L"\\Working\\SIL\\MSEA\\test_samples\\out\\");
    result.append(filename);
    return result;
}

wstring buildSourcePath(LPCTSTR filename) {
    wstring result;
    result.append(L"\\Working\\SIL\\MSEA\\test_samples\\");
    result.append(filename);
    return result;
}
