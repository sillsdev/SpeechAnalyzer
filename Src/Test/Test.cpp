#include <Windows.h>
#include <SDKDDKVer.h>
#include <gtest/gtest.h>
#include <conio.h>
#include <string>
#include <FileUtils.h>
#include "Test.h"

#pragma comment( lib, "winmm")
#pragma comment( lib, "waveutils")
#pragma comment( lib, "gtest")
#pragma comment( lib, "gtest_main")

int round(double value) {
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

int main(int argc, char ** argv) {
    int result = 0;
    try {
        ::testing::InitGoogleTest(&argc, argv);
        result = RUN_ALL_TESTS();
    } catch (...) {
        printf("exception occurred\n");
    }
    _getch();
    return result;
}
