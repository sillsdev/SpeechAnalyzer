// Derived from Init_syn.c from Sensimetrics corporatation
//
// Author steve MacLean
// Copyright JAARS 2001
// 
// Modification History
//   2/14/2001 SDM Copied source from Init_syn.c
//                 Created KSynth class
//                 Created CResonator class
//                 Removed all extern definitions moved into class
//                 Created TEMPORAL struct replaced int par array
//                 Created SPKRDEF removed replaced int spkrdef
//                 Removed static function members and placed in class

/*
*      SenSyn - Version 1.0
*
*      Copyright (c) 1991, 1992 by Sensimetrics Corporation
*      All Rights Reserved.
*
*/

/*
*  init_syn.c - synthesizer data structures and initialization
*
*  coded by Eric P. Carlson
*
*  Modification History:
*     5 Nov 1991	removed from spyn.c
*/

#include "stdafx.h"
#include "reson.h"		/* resonator support */
#include "synth.h"		/* synthesizer support */
#include "KSynth.h"


/* ---------------------------------------------------------------------- */
/* initialize synthesizer */

void CKSynth::init_synthesizer()
{
  synth.parallel_only_flag = spkrdef.CP;
  synth.num_casc_formants = spkrdef.NF;
  synth.num_samples = spkrdef.UI;
  synth.output_select = spkrdef.OS;

  synth.pulse_freq = 0;
  synth.glottis_open = FALSE;
  synth.period_ctr = 0;
  synth.voicing_state = FALSE;
  synth.pulse = FALSE;
  synth.random = spkrdef.RS;
  synth.voicing_time = 0;
  synth.global_time = 0;
  synth.voicing_amp = 0;
  synth.glottal_state = 0;
  synth.asp_state = 0;
  synth.integrator = 0;
  synth.close_shortened = FALSE;
  synth.close_time = 0;

  synth.F0 = 0;
  synth.FL = 0;
  synth.OQ = 0;
  synth.SQ = 0;
  synth.DI = 0;
  synth.AV = 0;
  synth.TL = 0;

  m_silVoicing.clear();

  glottal_pulse.ClearResonator();
  spectral_tilt.ClearResonator();
  nasal_pole_cascade.ClearResonator();
  formant_1_cascade.ClearResonator();
  formant_2_cascade.ClearResonator();
  formant_3_cascade.ClearResonator();
  formant_4_cascade.ClearResonator();
  formant_5_cascade.ClearResonator();
  formant_6_cascade.ClearResonator();
  formant_7_cascade.ClearResonator();
  formant_8_cascade.ClearResonator();
  formant_2_parallel.ClearResonator();
  formant_3_parallel.ClearResonator();
  formant_4_parallel.ClearResonator();
  formant_5_parallel.ClearResonator();
  formant_6_parallel.ClearResonator();
  nasal_zero_cascade.ClearResonator();
  trach_pole_cascade.ClearResonator();
  trach_zero_cascade.ClearResonator();
  formant_1_special.ClearResonator();
  formant_2_special.ClearResonator();
  formant_3_special.ClearResonator();
  formant_4_special.ClearResonator();
  formant_5_special.ClearResonator();
  formant_6_special.ClearResonator();
  formant_7_special.ClearResonator();
  formant_8_special.ClearResonator();
  nasal_pole_special.ClearResonator();
  trach_pole_special.ClearResonator();
}
