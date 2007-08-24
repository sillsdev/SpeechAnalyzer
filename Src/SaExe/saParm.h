#ifndef _SOURCE_PARM_H__

#include "CSaString.h"

#define _SOURCE_PARM_H__

typedef struct SaParm
{
  float       fVersion;           // version of sa/utt/ipa chunks (eg 2.1)
  CSaString     szDescription;      // file description
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
  int         nWordCount;         // number of words in gloss
} SaParm;                         // RIFF file header sa parameters

#endif