#ifndef WAVEFORMGENERATORPROCESS_H
#define WAVEFORMGENERATORPROCESS_H

#include "WaveformGeneratorSettings.h"
#include "process\Process.h"

class CProcessWaveformGenerator : public CProcess {
public:
    CProcessWaveformGenerator();
    virtual ~CProcessWaveformGenerator();
    long Process(CWaveformGeneratorSettings & parms, void * pCaller = NULL, int nProgress = 0, int nLevel = 1);
};

#endif
