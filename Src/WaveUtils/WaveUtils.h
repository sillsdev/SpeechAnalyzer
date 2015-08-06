#pragma once

#include "WaveReader.h"
#include "WaveWriter.h"
#include "WaveError.h"
#include "WaveResampler.h"
#include "FmtParm.h"

#include <stdexcept>

using std::invalid_argument;

/**
* extracts a single channel from wave file audio data vector
*
* @param channel the zero based channel to select
* @param numChannels the number of channels in the input vector
* @param blockAlign the block alignment of the input vector
* @param numSamples the number of samples in the input vector
* @param in the input data array
* @param out the output data array
*/
extern void ExtractChannel(WORD channel, WORD numChannels, WORD blockAlign, vector<char> & in, vector<char> & out);
extern bool IsStandardWaveFormat(LPCTSTR pszPathName, bool allow8Bit);

