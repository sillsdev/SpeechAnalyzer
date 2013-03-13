#ifndef WAVEFORMGENERATORPROCESS_H
#define WAVEFORMGENERATORPROCESS_H

#include "WaveformGeneratorSettings.h"
#include "process\sa_proc.h"

class CWaveformGeneratorProcess : public CDataProcess {
public:
	CWaveformGeneratorProcess();
	virtual ~CWaveformGeneratorProcess();
	long Process( CWaveformGeneratorSettings & parms, void * pCaller = NULL, int nProgress = 0, int nLevel = 1);
};

#endif
