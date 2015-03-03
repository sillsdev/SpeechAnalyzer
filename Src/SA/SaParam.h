#pragma once
#include <afx.h>
#include "SaString.h"
#include "atltime.h"

class CSaParam {
public:
    CSaParam(void);
    CSaParam operator=(const CSaParam & right);
    void Serialize(CArchive & ar);

    CSaString   szDescription;      // file description
    WORD        wFlags;             // 16 bits of flags for chunks
    BYTE        byRecordFileFormat; // wav, utt, mac, etc
    CTime       RecordTimeStamp;    // recording time stamp of wave file
    DWORD       dwRecordBandWidth;  // used by AudioCon (def =SamplesPerSec/2)
    BYTE        byRecordSmpSize;    // bits per sample when originally recorded
    DWORD       dwNumberOfSamples;  // number of samples in wave data
    long        lSignalMax;         // max signal value
    long        lSignalMin;         // min signal value
    DWORD       dwSignalBandWidth;  // minimum bandwidth ever found
    BYTE        byQuantization;     // sample quantization in bits
};
