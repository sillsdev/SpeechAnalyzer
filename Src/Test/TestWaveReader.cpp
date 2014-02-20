#include <SDKDDKVer.h>
#include <gtest/gtest.h>
#include <conio.h>
#include <string>
#include "WaveUtils.h"
#include "Test.h"

#pragma comment( lib, "winmm")
#pragma comment( lib, "waveutils")
#pragma comment( lib, "gtest")
#pragma comment( lib, "gtest_main")

using std::string;

TEST( WaveReadTest, SimpleRead_01) {

	wstring ifilename = buildSourcePath(L"chfrench.wav"); 
	DWORD flags = MMIO_READ | MMIO_DENYWRITE;
	WORD bitsPerSample = 0;
	WORD formatTag = 0;
	WORD channels = 0;
	DWORD samplesPerSec = 0;
	WORD blockAlign = 0;
	vector<char> buffer;

	CWaveReader wave;
	wave.read( ifilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);
	ASSERT_TRUE(channels==1)  << "expected stereo.";
	ASSERT_TRUE(bitsPerSample==16) << "expected 16 bits per sample";
	ASSERT_TRUE(formatTag==1) << "expected wave format";
	ASSERT_TRUE(samplesPerSec==22050) << "expected 22khz";
	ASSERT_TRUE(blockAlign==2) << "expected block align of 4";
	ASSERT_TRUE(buffer.size()==114210) << "expected file length of 5924";
	// do nothing for now
};

TEST( WaveReadTest, SimpleRead_02) {

	wstring ifilename = buildSourcePath(L"4ch.wav"); 
	DWORD flags = MMIO_READ | MMIO_DENYWRITE;
	WORD bitsPerSample = 0;
	WORD formatTag = 0;
	WORD channels = 0;
	DWORD samplesPerSec = 0;
	WORD blockAlign = 0;
	vector<char> buffer;

	CWaveReader wave;
	wave.read( ifilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);
	ASSERT_TRUE(channels==4)  << "expected stereo.";
	ASSERT_TRUE(bitsPerSample==16) << "expected 16 bits per sample";
	ASSERT_TRUE(formatTag==1) << "expected wave format";
	ASSERT_TRUE(samplesPerSec==44100) << "expected 22khz";
	ASSERT_TRUE(blockAlign==8) << "expected block align of 4";
	ASSERT_TRUE(buffer.size()==1352248) << "expected file length of 5924";
	// do nothing for now
};

TEST( WaveReadTest, SimpleRead_03) {

	wstring ifilename = buildSourcePath(L"181.wav"); 
	DWORD flags = MMIO_READ | MMIO_DENYWRITE;
	WORD bitsPerSample = 0;
	WORD formatTag = 0;
	WORD channels = 0;
	DWORD samplesPerSec = 0;
	WORD blockAlign = 0;
	vector<char> buffer;

	CWaveReader wave;
	wave.read( ifilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);
	ASSERT_TRUE(channels==2)  << "expected stereo.";
	ASSERT_TRUE(bitsPerSample==24) << "expected 16 bits per sample";
	ASSERT_TRUE(formatTag==1) << "expected wave format";
	ASSERT_TRUE(samplesPerSec==96000) << "expected 22khz";
	ASSERT_TRUE(blockAlign==6) << "expected block align of 4";
	ASSERT_TRUE(buffer.size()==5840640) << "expected file length of 5924";
	// do nothing for now
};

TEST( WaveReadTest, SimpleRead_04) {

	wstring ifilename = buildSourcePath(L"182.wav"); 
	DWORD flags = MMIO_READ | MMIO_DENYWRITE;
	WORD bitsPerSample = 0;
	WORD formatTag = 0;
	WORD channels = 0;
	DWORD samplesPerSec = 0;
	WORD blockAlign = 0;
	vector<char> buffer;

	CWaveReader wave;
	wave.read( ifilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);
	ASSERT_TRUE(channels==2)  << "expected stereo.";
	ASSERT_TRUE(bitsPerSample==24) << "expected 16 bits per sample";
	ASSERT_TRUE(formatTag==1) << "expected wave format";
	ASSERT_TRUE(samplesPerSec==96000) << "expected 22khz";
	ASSERT_TRUE(blockAlign==6) << "expected block align of 4";
	ASSERT_TRUE(buffer.size()==5637120) << "expected file length of 5924";
	// do nothing for now
};

