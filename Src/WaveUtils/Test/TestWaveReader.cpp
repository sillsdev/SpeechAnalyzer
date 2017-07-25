#include <SDKDDKVer.h>
#include "CppUnitTest.h"
#include <conio.h>
#include <string>
#include "..\WaveUtils.h"
#include <string>
#include "Test.h"

#pragma comment( lib, "winmm")
#pragma comment( lib, "waveutils")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace WaveUtils_Test {
	TEST_CLASS(WaveReadTest) {
public:
	TEST_METHOD(SimpleRead_01) {

		wstring ifilename = buildSourcePath(L"chfrench.wav");
		DWORD flags = MMIO_READ | MMIO_DENYWRITE;
		WORD bitsPerSample = 0;
		WORD formatTag = 0;
		WORD channels = 0;
		DWORD samplesPerSec = 0;
		WORD blockAlign = 0;
		vector<char> buffer;

		CWaveReader wave;
		wave.Read(ifilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);
		Assert::IsTrue(channels == 1, L"expected stereo.");
		Assert::IsTrue(bitsPerSample == 16, L"expected 16 bits per sample");
		Assert::IsTrue(samplesPerSec == 22050, L"expected 22khz");
		Assert::IsTrue(blockAlign == 2, L"expected block align of 4");
		Assert::IsTrue(buffer.size() == 114210, L"expected file length of 5924");
		// do nothing for now
	};

	TEST_METHOD(SimpleRead_02) {

		wstring ifilename = buildSourcePath(L"4ch.wav");
		DWORD flags = MMIO_READ | MMIO_DENYWRITE;
		WORD bitsPerSample = 0;
		WORD formatTag = 0;
		WORD channels = 0;
		DWORD samplesPerSec = 0;
		WORD blockAlign = 0;
		vector<char> buffer;

		CWaveReader wave;
		wave.Read(ifilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);
		Assert::IsTrue(channels == 4, L"expected stereo.");
		Assert::IsTrue(bitsPerSample == 16, L"expected 16 bits per sample");
		Assert::IsTrue(formatTag == 1, L"expected wave format");
		Assert::IsTrue(samplesPerSec == 44100, L"expected 22khz");
		Assert::IsTrue(blockAlign == 8, L"expected block align of 4");
		Assert::IsTrue(buffer.size() == 1352248, L"expected file length of 5924");
		// do nothing for now
	};

	TEST_METHOD(SimpleRead_03) {

		wstring ifilename = buildSourcePath(L"181.wav");
		DWORD flags = MMIO_READ | MMIO_DENYWRITE;
		WORD bitsPerSample = 0;
		WORD formatTag = 0;
		WORD channels = 0;
		DWORD samplesPerSec = 0;
		WORD blockAlign = 0;
		vector<char> buffer;

		CWaveReader wave;
		wave.Read(ifilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);
		Assert::IsTrue(channels == 2, L"expected stereo.");
		Assert::IsTrue(bitsPerSample == 24, L"expected 16 bits per sample");
		Assert::IsTrue(formatTag == 1, L"expected wave format");
		Assert::IsTrue(samplesPerSec == 96000, L"expected 22khz");
		Assert::IsTrue(blockAlign == 6, L"expected block align of 4");
		Assert::IsTrue(buffer.size() == 5840640, L"expected file length of 5924");
		// do nothing for now
	};

	TEST_METHOD(SimpleRead_04) {

		wstring ifilename = buildSourcePath(L"182.wav");
		DWORD flags = MMIO_READ | MMIO_DENYWRITE;
		WORD bitsPerSample = 0;
		WORD formatTag = 0;
		WORD channels = 0;
		DWORD samplesPerSec = 0;
		WORD blockAlign = 0;
		vector<char> buffer;

		CWaveReader wave;
		wave.Read(ifilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);
		Assert::IsTrue(channels == 2, L"expected stereo.");
		Assert::IsTrue(bitsPerSample == 24, L"expected 16 bits per sample");
		Assert::IsTrue(formatTag == 1, L"expected wave format");
		Assert::IsTrue(samplesPerSec == 96000, L"expected 22khz");
		Assert::IsTrue(blockAlign == 6, L"expected block align of 4");
		Assert::IsTrue(buffer.size() == 5637120, L"expected file length of 5924");
		// do nothing for now
	};
	};
}