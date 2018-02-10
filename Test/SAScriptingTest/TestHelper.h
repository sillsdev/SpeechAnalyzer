#pragma once
#include <string>

using std::string;

class TestHelper {

public:
	static string GetTempDir();
	static string GetDevHome();
	static string MakePath(LPCSTR partA, LPCSTR partB);
	static string CopyWaveFile(LPCSTR srcDir, LPCSTR destDir, LPCSTR filename);
	static string CreateListFile(LPCSTR destDir, LPCSTR waveFile, LPCSTR showCmd);
	static string CreateListFile(LPCSTR destDir, LPCSTR waveFile, LPCSTR showCmd, int speed, int volume);
	static void LaunchSA(LPCSTR devHome, LPCSTR listFile);
};

