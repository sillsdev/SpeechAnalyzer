#pragma once

#include <Windows.h>
#include <vector>
#include <MMSystem.h>

using std::vector;

/**
* writes a wave file
* @throws invalid_argument
* @throws wave_error
*/
class CWaveWriter {
public:
    CWaveWriter();
    ~CWaveWriter();

    void Write(LPCTSTR filename, DWORD flags, WORD bitsPerSample, WORD formatTag, WORD channels, DWORD samplesPerSec, vector<char> & buffer);

private:
    HMMIO hmmio;
};

