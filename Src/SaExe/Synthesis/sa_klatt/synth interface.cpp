// Author Steve MacLean
// Copyright JAARS 2001
// 
// Modification History
//   2/14/2001 SDM Created to provide a clean interface to the Klatt synthesizer.
//                    Designed to hide class details.

#include "stdafx.h"
#include "synth.h"
#include "reson.h"
#include "KSynth.h"

CKSynth* CreateKSynth(SPKRDEF* spkrdef)
{
	return new CKSynth(spkrdef);
}

void DestroyKSynth(CKSynth *synth)
{
	if(synth)
		delete synth;
}

void SynthesizeFrame(CKSynth *synth, TEMPORAL *par, INT16 *wave)
{
	synth->senswave(par,wave);
}

Float WaveformMaximum(CKSynth *synth)
{
	return synth->synthesizedWaveformMaximum();
}

