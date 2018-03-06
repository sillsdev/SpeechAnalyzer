#include "pch.h"
#include "CppUnitTest.h"
#include <process.h>
#include <string>
#include <Windows.h>
#include "TestHelper.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using std::wstring;

namespace SAScriptingTest {
	TEST_CLASS(ScriptingTest) {
		string tempDir;
		string devHome;

public:
	TEST_METHOD_INITIALIZE(initialize) {
		tempDir = TestHelper::GetTempDir();
		devHome = TestHelper::GetDevHome();
	}

	TEST_METHOD(TestSimpleShowGarbage) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Garbage");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimpleShowHide) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Hide");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimpleShowShow) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimpleShowRestore) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Restore");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimpleShowMaximize) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Maximize");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimpleShowMinimize) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Minimize");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimpleShowSized) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Size(0, 0, 400, 200)");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimpleShowNone) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "None");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlaybackNormal) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlaybackZero) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 0, 0);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlayback100) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlayback50) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 50, 100);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlaybackMax) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 333, 100);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlaybackBelowRange) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", -1, -1);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlaybackAboveRange) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 500, 500);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlayback22kFullSpeedSecondHalf) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 19796, 39592);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlayback22kHalfSpeedSecondHalf) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 50, 100, 19796, 39592);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlayback44k) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test44k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlayback44kHalfSpeed) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test44k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 50, 100, 0, 79184);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlayback44kFullSpeedSecondHalf) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test44k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 39592, 79184);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSimplePlayback44kHalfSpeedSecondHalf) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test44k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 50, 100, 39592, 79184);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}
	};
}