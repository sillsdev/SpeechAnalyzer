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
	// test the SHOW command
	TEST_METHOD(TestShowGarbage) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Garbage");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestShowHide) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Hide");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestShowShow) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestShowRestore) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Restore");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestShowMaximize) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Maximize");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestShowMinimize) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Minimize");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestShowSized) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Size(0, 0, 400, 200)");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestShowNone) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "None");
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	// RANGE TESTS
	TEST_METHOD(TestRangeBelow) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", -1, -1);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestRangeAbove) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 1000, 1000);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	// test the VOLUME parameter
	TEST_METHOD(TestVolumeZero) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show",100, 0);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestVolumeHalf) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 50);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestVolumeFull) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	// test the SPEED parameter
	TEST_METHOD(TestSpeedZero) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 0, 100);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSpeedHalf) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 50, 100);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestSpeedFull) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	// test the POSITION parameters
	TEST_METHOD(TestPosition22kCompleteMono) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 0, 39592);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition22kFirstHalfMono) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 0, 19796);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition22kSecondHalfMono) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 19796, 39592);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition22kCompleteStereo) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "teststereo22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 0, 39592);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition22kFirstHalfStereo) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "teststereo22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 0, 19796);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition22kSecondHalfStereo) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "teststereo22k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 19796, 39592);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition44kCompleteMono) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test44k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 0, 79184);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition44kFirstHalfMono) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test44k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 0, 39592);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition44kSecondHalfMono) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "test44k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 39592, 79184);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition44kCompleteStereo) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "teststereo44k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 0, 79184);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition44kFirstHalfStereo) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "teststereo44k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 0, 39592);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestPosition44kSecondHalfStereo) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "teststereo44k.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 39592, 79184);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	TEST_METHOD(TestChFrenchStereo) {
		string waveFile = TestHelper::CopyWaveFile(TestHelper::MakePath(devHome.c_str(), "test\\samples\\scripting").c_str(), tempDir.c_str(), "chfrenchstereo.wav");
		string listFile = TestHelper::CreateListFile(tempDir.c_str(), waveFile.c_str(), "Show", 100, 100, 6900, 14800);
		TestHelper::LaunchSA(devHome.c_str(), listFile.c_str());
	}

	};
}