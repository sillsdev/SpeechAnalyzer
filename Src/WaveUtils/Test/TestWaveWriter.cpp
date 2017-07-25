#include <SDKDDKVer.h>
#include "CppUnitTest.h"
#include <conio.h>
#include <string>
#include "..\WaveUtils.h"
#include "Test.h"

#pragma comment( lib, "winmm")
#pragma comment( lib, "waveutils")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace WaveUtils_Test {

	TEST_CLASS(WaveWriteTest) {

public:
	TEST_METHOD( InvalidParams_01) {

		try {
			wstring ofilename = buildSourcePath(L"silence.wav");
			DWORD flags = MMIO_CREATE | MMIO_WRITE;
			WORD bitsPerSample = 0;
			WORD formatTag = 0;
			WORD channels = 0;
			DWORD samplesPerSec = 0;
			vector<char> buffer;

			for (int i = 0; i < 22050; i++) {
				buffer.push_back(0);
				buffer.push_back(0);
			}

			CWaveWriter writer;
			writer.Write(ofilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, buffer);
			Assert::IsTrue(false,L"shouldn't get here");
		} catch (...) {
		}
	};

	TEST_METHOD(SimpleWrite_01) {

		wstring ofilename = buildSourcePath(L"silence.wav");
		DWORD flags = MMIO_CREATE | MMIO_WRITE;
		WORD bitsPerSample = 16;
		WORD formatTag = 1;
		WORD channels = 2;
		DWORD samplesPerSec = 22050;
		WORD blockAlign = 4;
		vector<char> buffer;

		for (int i = 0; i < 22050; i++) {
			buffer.push_back(0);
			buffer.push_back(0);
		}

		CWaveWriter writer;
		writer.Write(ofilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, buffer);

		wstring ifilename = ofilename;
		CWaveReader reader;
		flags = MMIO_READ | MMIO_DENYWRITE;
		reader.Read(ifilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);

		Assert::IsTrue(channels == 2,L"expected stereo.");
		Assert::IsTrue(bitsPerSample == 16, L"expected 16 bits per sample");
		Assert::IsTrue(formatTag == 1, L"expected wave format");
		Assert::IsTrue(samplesPerSec == 22050, L"expected 22khz");
		Assert::IsTrue(blockAlign == 4, L"expected block align of 4");
		Assert::IsTrue(buffer.size() == 44100, L"expected length of 44100");
		// do nothing for now
	};

	TEST_METHOD(ExtractChannel_01) {

		wstring ifilename = buildSourcePath(L"4ch.wav");
		WORD bitsPerSample = 0;
		WORD formatTag = 0;
		WORD channels = 0;
		DWORD samplesPerSec = 0;
		WORD blockAlign = 0;

		//TODO handle memory during exceptions
		vector<char> buffer;
		CWaveReader reader;
		reader.Read(ifilename.c_str(), MMIO_ALLOCBUF | MMIO_READ, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);

		DWORD numSamples = buffer.size() / blockAlign;

		vector<char> newBuffer;
		WORD newBlockAlign = blockAlign / channels;
		WORD remainder = blockAlign - newBlockAlign;
		DWORD l = 0;
		for (DWORD i = 0; i < numSamples; i++) {
			for (WORD j = 0; j < newBlockAlign; j++) {
				newBuffer.push_back(buffer[l++]);
			}
			l += remainder;
		}

		wstring ofilename = buildResultPath(L"left_ch.wav");
		WORD newChannels = 1;
		CWaveWriter writer;
		writer.Write(ofilename.c_str(), MMIO_CREATE | MMIO_WRITE, bitsPerSample, formatTag, newChannels, samplesPerSec, newBuffer);
	};

	TEST_METHOD(ExtractChannel_02) {

		wstring ifilename = buildSourcePath(L"4ch.wav");
		WORD bitsPerSample = 0;
		WORD formatTag = 0;
		WORD channels = 0;
		DWORD samplesPerSec = 0;
		WORD blockAlign = 0;

		//TODO handle memory during exceptions
		vector<char> buffer;
		CWaveReader reader;
		reader.Read(ifilename.c_str(), MMIO_ALLOCBUF | MMIO_READ, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);

		vector<char> newBuffer;
		ExtractChannel(1, channels, blockAlign, buffer, newBuffer);

		wstring ofilename = buildResultPath(L"right_ch.wav");
		WORD newChannels = 1;
		CWaveWriter writer;
		writer.Write(ofilename.c_str(), MMIO_CREATE | MMIO_WRITE, bitsPerSample, formatTag, newChannels, samplesPerSec, newBuffer);
	};
	};
}
