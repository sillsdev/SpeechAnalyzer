#include "pch.h"
#include <string>
#include <Windows.h>
#include "CppUnitTest.h"
#include <process.h>
#include "TestHelper.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using std::string;

LPCSTR SA_DEV_HOME = "SA_DEV_HOME";

string TestHelper::GetTempDir() {

	char buffer[MAX_PATH+1];
	memset(buffer, 0, sizeof(buffer));
	DWORD count = ::GetTempPathA(sizeof(buffer), buffer);
	Assert::IsTrue(count != 0);
	return string(buffer);
}

string TestHelper::GetDevHome() {
	
	char buffer[512];
	memset(buffer, 0, sizeof(buffer));
	DWORD count = ::GetEnvironmentVariableA(SA_DEV_HOME, buffer, sizeof(buffer));
	Assert::IsTrue(count != 0);
	return string(buffer);
}

string TestHelper::MakePath(LPCSTR partA, LPCSTR partB) {
	string result;
	result.append(partA);
	result.append("\\");
	result.append(partB);
	return result;
}

string TestHelper::CopyWaveFile(LPCSTR srcDir, LPCSTR destDir, LPCSTR filename) {

	// copy the wave file
	string src;
	src.append(srcDir);
	src.append("\\");
	src.append(filename);
	
	string dest;
	dest.append(destDir);
	dest.append("\\");
	dest.append(filename);

		printf("copying file from %s to %s\n", src.c_str(), dest.c_str());

	BOOL bResult = DeleteFileA(dest.c_str());

	bResult = CopyFileA(src.c_str(), dest.c_str(), TRUE);
	Assert::IsTrue(bResult != 0);
	return dest;
}

string TestHelper::CreateListFile(LPCSTR destDir, LPCSTR waveFile, LPCSTR showCmd) {
	return CreateListFile(destDir, waveFile, showCmd, 50, 50);
}

string TestHelper::CreateListFile(LPCSTR destDir, LPCSTR waveFile, LPCSTR showCmd, int speed, int volume) {

	string file;
	file = MakePath(destDir, "demo.script");

	// write the file
	FILE * pFile = NULL;
	errno_t err = fopen_s(&pFile, file.c_str(), "w");
	Assert::IsTrue(err==0);
	fprintf(pFile, "[Settings]\n");
	fprintf(pFile, "CallingApp = SpeechAnalyzer\n");
	fprintf(pFile, "ShowWindow = %s\n", showCmd);
	fprintf(pFile, "[Commands]\n");
	fprintf(pFile, "Command0 = SelectFile(0)\n");
	fprintf(pFile, "Command1 = Play(%d, %d, 0, 37000)\n",speed, volume);
	fprintf(pFile, "Command2 = Return(1)\n");
	fprintf(pFile, "[AudioFiles]\n");
	fprintf(pFile, "File0 = %s\n", waveFile);
	fclose(pFile);
	
	return file;
}

void TestHelper::LaunchSA( LPCSTR devHome, LPCSTR listFile) {

	// prepare the app and file
	// create saserver.exe in the debug directory.
	char app_path[256];
	char server_path[256];
	memset(app_path, 0, sizeof(app_path));
	memset(server_path, 0, sizeof(server_path));
	sprintf_s(app_path, "%s\\output\\debug\\sa.exe", devHome);
	sprintf_s(server_path, "%s\\output\\debug\\saserver.exe", devHome);
	BOOL bResult = DeleteFileA(server_path);
	bResult = CopyFileA(app_path, server_path, TRUE);
	Assert::IsTrue(bResult!=0);
	intptr_t iresult = _spawnl(_P_WAIT, server_path, server_path, "-L", listFile, NULL);
	Assert::IsTrue(iresult == 0);
}

