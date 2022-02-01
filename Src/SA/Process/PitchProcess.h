#pragma once

#include <atomic>
#include "grappl.h"
#include "dspTypes.h"

class CPitchProcess {

    struct Parameters {
        int32 sampFreq;         /* acoustic data sampling frequency */
        int16 minPitch;         /* min permissible pitch frequency */
        int16 maxPitch;         /* max permissible pitch frequency */
        int16 minVoiced16;      /* min smoothed amplitude for voicing (x16) */
        int16 minGroup;         /* min points in significant pitch contour */
        int maxChange;
        int smpSize;
        DWORD dataSize;
        DWORD bufferSize;
        WORD blockAlign;
    };

private:
    enum State { BufferTooSmall,NoData,Error, Cancelled,NoPitch,DataReady};
    int16 eightbit;         /* true for 8-bit data */
    int16 mode;             /* calculation mode */
    int32 smoothfreq;       /* moving-average filter equivalent freq */
    int16 calcint;          /* result calculation interval in data samples */
    int16 maxchange_pc10;   /* max % change (x10) between successive pitch res */
    int16 minmeanweight;    /* min mean weight for significant pitch contour */
    int16 maxinterp_pc10;   /* max % change (x10) to join contours across 1-pt gap */
    // byte offset into waveform data'
    DWORD dataPos;
    int maxValue;
    int minValue;
    std::atomic_int progress;
    std::atomic_bool cancelled;

public:
    State process(Parameters parameters, HPSTR (Read)(DWORD dwOffset, BOOL bBlockBegin), bool (Write)(const void* lpBuf, UINT nCount));
};
