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

TEST(WaveWriteTest, InvalidParams_01) {

    wstring ofilename = buildSourcePath(L"silence.wav");
    DWORD flags = MMIO_CREATE|MMIO_WRITE;
    WORD bitsPerSample = 0;
    WORD formatTag = 0;
    WORD channels = 0;
    DWORD samplesPerSec = 0;
    vector<char> buffer;

    for (int i=0; i<22050; i++) {
        buffer.push_back(0);
        buffer.push_back(0);
    }

    CWaveWriter writer;
    EXPECT_THROW(writer.Write(ofilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, buffer),invalid_argument);
};

TEST(WaveWriteTest, SimpleWrite_01) {

    wstring ofilename = buildSourcePath(L"silence.wav");
    DWORD flags = MMIO_CREATE|MMIO_WRITE;
    WORD bitsPerSample = 16;
    WORD formatTag = 1;
    WORD channels = 2;
    DWORD samplesPerSec = 22050;
    WORD blockAlign = 4;
    vector<char> buffer;

    for (int i=0; i<22050; i++) {
        buffer.push_back(0);
        buffer.push_back(0);
    }

    CWaveWriter writer;
    writer.Write(ofilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, buffer);

    wstring ifilename = ofilename;
    CWaveReader reader;
    flags = MMIO_READ | MMIO_DENYWRITE;
    reader.Read(ifilename.c_str(), flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);

    ASSERT_TRUE(channels==2)  << "expected stereo.";
    ASSERT_TRUE(bitsPerSample==16) << "expected 16 bits per sample";
    ASSERT_TRUE(formatTag==1) << "expected wave format";
    ASSERT_TRUE(samplesPerSec==22050) << "expected 22khz";
    ASSERT_TRUE(blockAlign==4) << "expected block align of 4";
    ASSERT_TRUE(buffer.size()==44100) << "expected length of 44100";
    // do nothing for now
};

TEST(WaveWriteTest, ExtractChannel_01) {

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

    DWORD numSamples = buffer.size()/blockAlign;

    vector<char> newBuffer;
    WORD newBlockAlign = blockAlign/channels;
    WORD remainder = blockAlign-newBlockAlign;
    DWORD l = 0;
    for (DWORD i=0; i<numSamples; i++) {
        for (WORD j=0; j<newBlockAlign; j++) {
            newBuffer.push_back(buffer[l++]);
        }
        l += remainder;
    }

    wstring ofilename = buildResultPath(L"left_ch.wav");
    WORD newChannels = 1;
    CWaveWriter writer;
    writer.Write(ofilename.c_str(), MMIO_CREATE | MMIO_WRITE, bitsPerSample, formatTag, newChannels, samplesPerSec, newBuffer);
};

TEST(WaveWriteTest, ExtractChannel_02) {

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
    ExtractChannel(1,channels,blockAlign,buffer,newBuffer);

    wstring ofilename = buildResultPath(L"right_ch.wav");
    WORD newChannels = 1;
    CWaveWriter writer;
    writer.Write(ofilename.c_str(), MMIO_CREATE | MMIO_WRITE, bitsPerSample, formatTag, newChannels, samplesPerSec, newBuffer);
};
