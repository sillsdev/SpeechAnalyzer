#include "Stdafx.h"
#include "FmtParm.h"

CFmtParm::CFmtParm()
{
    wTag = 0;
    wChannels = 0;
    dwSamplesPerSec = 0;
    dwAvgBytesPerSec = 0;
    wBitsPerSample = 0;
    wBlockAlign = 0;
}

CFmtParm::~CFmtParm()
{
}

// get the sample size in bytes for a single channel
DWORD CFmtParm::GetSampleSize() const
{
    return wBlockAlign / wChannels;
}

// create a WAVEFORMATEX structure from this object
WAVEFORMATEX CFmtParm::GetWaveFormatEX()
{
    WAVEFORMATEX formatEx;
    memset(&formatEx,0,sizeof(WAVEFORMATEX));
    formatEx.wFormatTag = wTag;
    formatEx.nChannels = wChannels;
    formatEx.nSamplesPerSec = dwSamplesPerSec;
    formatEx.nAvgBytesPerSec = dwAvgBytesPerSec;
    formatEx.nBlockAlign = wBlockAlign;
    formatEx.wBitsPerSample = wBitsPerSample;
    formatEx.cbSize = 0;
    return formatEx;
}
