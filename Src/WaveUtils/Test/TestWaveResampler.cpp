#include <SDKDDKVer.h>
#include "CppUnitTest.h"
#include <conio.h>
#include <string>
#include "..\WaveUtils.h"
#include "Test.h"

class ProgressUpdater : public IProgressUpdate {
	virtual void SetProgress(int value) {
	};
	virtual int GetProgress() {
		return 0;
	};
};

ProgressUpdater updater;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace WaveUtils_Test {
	TEST_CLASS(WaveResamplingTest) {
public:
	void test(LPCTSTR ifile) {

		wstring ifilename = buildSourcePath(ifile);
		wstring ofilename = buildResultPath(L"temp.wav");

		CWaveResampler resampler;
		CWaveResampler::ECONVERT result = resampler.Normalize(ifilename.c_str(), ofilename.c_str(), updater);
		Assert::IsTrue(result == CWaveResampler::EC_SUCCESS, L"expected success status");

		DWORD flags = MMIO_READ | MMIO_DENYWRITE;
		WORD bitsPerSample = 0;
		WORD formatTag = 0;
		WORD channels = 0;
		DWORD samplesPerSec = 0;
		WORD blockAlign = 0;
		vector<char> buffer;
		CWaveReader reader;
		reader.Read(ofilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);
		Assert::IsTrue(bitsPerSample == 16, L"expected 16 bits per sample");
	}

	TEST_METHOD(Test_01) {
		test(L"4ch.wav");
	}
	//TEST( Resampling, Test_02) { test(L"M1F1-Alaw-AFsp.wav");}
	//TEST( Resampling, Test_03) { test(L"M1F1-AlawWE-AFsp.wav");}
	TEST_METHOD(Test_04) {
		test(L"M1F1-float32-AFsp.wav");
	}
	TEST_METHOD(Test_05) {
		test(L"M1F1-float32WE-AFsp.wav");
	}
	//TEST( Resampling, Test_06) { test(L"M1F1-float64-AFsp.wav");}
	//TEST( Resampling, Test_07) { test(L"M1F1-float64WE-AFsp.wav");}
	//TEST( Resampling, Test_08) { test(L"M1F1-int12-AFsp.wav");}
	TEST_METHOD(Test_09) {
		test(L"M1F1-int12WE-AFsp.wav");
	}
	TEST_METHOD(Test_10) {
		test(L"M1F1-int16-AFsp.wav");
	}
	TEST_METHOD(Test_11) {
		test(L"M1F1-int16WE-AFsp.wav");
	}
	TEST_METHOD(Test_12) {
		test(L"96KHz-24bit Mono.wav");
	}
	TEST_METHOD(Test_13) {
		test(L"96KHz-24bit Original.WAV");
	}
	TEST_METHOD(Test_14) {
		test(L"96KHz-24bit Stereo.wav");
	}
	TEST_METHOD(Test_15) {
		test(L"M1F1-int24-AFsp.wav");
	}
	TEST_METHOD(Test_16) {
		test(L"M1F1-int24WE-AFsp.wav");
	}
	TEST_METHOD(Test_17) {
		test(L"M1F1-int32-AFsp.wav");
	}
	TEST_METHOD(Test_18) {
		test(L"M1F1-int32WE-AFsp.wav");
	}
	//TEST( Resampling, Test_19) { test(L"M1F1-mulaw-AFsp.wav");}
	//TEST( Resampling, Test_20) { test(L"M1F1-mulawWE-AFsp.wav");}
	//TEST( Resampling, Test_21) { test(L"M1F1-uint8-AFsp.wav");}
	//TEST( Resampling, Test_22) { test(L"M1F1-uint8WE-AFsp.wav");}
	};
}
