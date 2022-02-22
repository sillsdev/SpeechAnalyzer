#ifndef WAVEFORMGENERATORPROCESS_H
#define WAVEFORMGENERATORPROCESS_H

#include "WaveformGeneratorSettings.h"

class CProcessWaveformGenerator : public CProcess {
public:
    CProcessWaveformGenerator(Context& context) : CProcess(context) {};
    CProcessWaveformGenerator() = delete ;

    long Process(CWaveformGeneratorSettings & parms, void * pCaller = NULL, int nProgress = 0, int nLevel = 1);
};

#endif
