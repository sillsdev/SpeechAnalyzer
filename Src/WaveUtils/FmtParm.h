#ifndef FMTPARM_H
#define FMTPARM_H

#include <Windows.h>
#include <MMSystem.h>

class CFmtParm {

public:
    CFmtParm();
    virtual ~CFmtParm();
    // get the sample size in bytes
    DWORD GetSampleSize() const;
    // create a WAVEFORMATEX structure from this object
    WAVEFORMATEX GetWaveFormatEX();

    WORD wTag;                      // format tag
    WORD wChannels;                 // number of channels
    DWORD dwSamplesPerSec;          // sampling rate
    DWORD dwAvgBytesPerSec;         // bytes per second for all channels
    WORD wBitsPerSample;            // data width
    WORD wBlockAlign;               // block align for all channels
};

#endif
