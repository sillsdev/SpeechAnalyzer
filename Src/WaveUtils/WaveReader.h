#pragma once

#include <Windows.h>
#include <vector>
#include <MMSystem.h>

using std::vector;

/**
* reads a wave file
* @throws wave_error
*/
class CWaveReader {
public:
    CWaveReader();
    ~CWaveReader();

    void read(LPCTSTR filename, DWORD flags, WORD & bitsPerSample, WORD & formatTag, WORD & channels, DWORD & samplesPerSec, WORD & blockAlign, vector<char> & buffer);

private:
    HMMIO hmmio;
};

