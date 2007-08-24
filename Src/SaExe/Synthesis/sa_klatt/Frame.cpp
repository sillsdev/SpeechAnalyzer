// Derived from Frame.c from Sensimetrics corporatation
//
// Author steve MacLean
// Copyright JAARS 2001
// 
// Modification History
//   2/14/2001 SDM Copied source from Frame.c
//                 Created KSynth class
//                 Created CResonator class
//                 Removed all extern definitions moved into class
//                 Created TEMPORAL struct replaced int par array
//                 Created SPKRDEF removed replaced int spkrdef
//                 Added parameters to function senswave()

/*
*      SenSyn - Version 1.1
*
*      Copyright (c) 1991, 1992 by Sensimetrics Corporation
*      All Rights Reserved.
*
*/

/*
*  frame.c - synthesize one frame
*
*  coded by Eric P. Carlson
*
*  Modification History:
*     5 Nov 1991	removed sample.c and init_syn.c, renamed to frame.c
*    21 Nov 1991	moved spectral_tilt, formant_1s to be pitch-synchronous
*    27 Feb 1992	update formant_1s here, but DF1/DB1 pitch-synchronously
*    23 Feb 1993	changed casts in sigmx calculation, version 1.1
*/

#include "stdafx.h"
#include <math.h>		/* needed for power and fabs function */
#include <limits.h>		/* for LONG_MAX and LONG_MIN */
#include "reson.h"		/* resonator support */
#include "synth.h"		/* synthesizer support */
#include "KSynth.h"

/* ---------------------------------------------------------------------- */

void CKSynth::senswave(TEMPORAL *parameters, INT16 *wave)
{
  int index;
  Float sample;

  // copy new parameters
  pars = *parameters;

  /* setup amplitudes */
  coefs.asp_amp = pars.AH?dB2amp(spkrdef.GH + pars.AH + A_AH):0;
  coefs.fric_amp = pars.AF?dB2amp(spkrdef.GF + pars.AF + A_AF):0;
  coefs.f1p_amp = dB2amp(A_A1);
  coefs.f2p_amp = dB2amp(pars.A2F + A_A2F);
  coefs.f3p_amp = dB2amp(pars.A3F + A_A3F);
  coefs.f4p_amp = dB2amp(pars.A4F + A_A4F);
  coefs.f5p_amp = dB2amp(pars.A5F + A_A5F);
  coefs.f6p_amp = dB2amp(pars.A6F + A_A6F);
  coefs.bypass_amp = dB2amp(pars.AB + A_AB);
  coefs.npv_amp = dB2amp(pars.ANV + A_ANV);
  coefs.f1v_amp = dB2amp(pars.A1V + A_A1V);
  coefs.f2v_amp = dB2amp(pars.A2V + A_A2V);
  coefs.f3v_amp = dB2amp(pars.A3V + A_A3V);
  coefs.f4v_amp = dB2amp(pars.A4V + A_A4V);
  coefs.tpv_amp = dB2amp(pars.ATV + A_ATV);
  coefs.f5v_amp = dB2amp(pars.A5V + A_A5V);
  coefs.f6v_amp = dB2amp(pars.A6V + A_A6V);
  coefs.f7v_amp = dB2amp(pars.A7V + A_A7V);
  coefs.f8v_amp = dB2amp(pars.A8V + A_A8V);

  /* setup resonators */
  /* formant_1_cascade is also setup pitch-synchronously in voice.c */
  formant_1_cascade.InterPolePair(
    pars.F1 + (synth.glottis_open ? pars.DF1 : 0),
    pars.B1 + (synth.glottis_open ? pars.DB1 : 0),
    spkrdef.SR);
  formant_2_cascade.InterPolePair(pars.F2, pars.B2, spkrdef.SR);
  formant_3_cascade.InterPolePair(pars.F3, pars.B3, spkrdef.SR);
  formant_4_cascade.InterPolePair(pars.F4, pars.B4, spkrdef.SR);
  formant_5_cascade.InterPolePair(pars.F5, pars.B5, spkrdef.SR);
  formant_6_cascade.InterPolePair(pars.F6, pars.B6, spkrdef.SR);
  formant_7_cascade.InterPolePair(6500, 500, spkrdef.SR);
  formant_8_cascade.InterPolePair(7500, 600, spkrdef.SR);
  nasal_pole_cascade.InterPolePair(pars.FNP, pars.BNP, spkrdef.SR);
  nasal_zero_cascade.InterZeroPair(pars.FNZ, pars.BNZ, spkrdef.SR);
  trach_pole_cascade.InterPolePair(pars.FTP, pars.BTP, spkrdef.SR);
  trach_zero_cascade.InterZeroPair(pars.FTZ, pars.BTZ, spkrdef.SR);
  formant_2_parallel.InterPolePair(pars.F2, pars.B2F, spkrdef.SR);
  formant_3_parallel.InterPolePair(pars.F3, pars.B3F, spkrdef.SR);
  formant_4_parallel.InterPolePair(pars.F4, pars.B4F, spkrdef.SR);
  formant_5_parallel.InterPolePair(pars.F5, pars.B5F, spkrdef.SR);
  formant_6_parallel.InterPolePair(pars.F6, pars.B6F, spkrdef.SR);
  nasal_pole_special.InterPolePair(pars.FNP, pars.BNP, spkrdef.SR);
  /* formant_1_special is also setup pitch-synchronously in voice.c */
  formant_1_special.InterPolePair(
    pars.F1 + (synth.glottis_open ? pars.DF1 : 0),
    pars.B1 + (synth.glottis_open ? pars.DB1 : 0),
    spkrdef.SR);
  formant_2_special.InterPolePair(pars.F2, pars.B2, spkrdef.SR);
  formant_3_special.InterPolePair(pars.F3, pars.B3, spkrdef.SR);
  formant_4_special.InterPolePair(pars.F4, pars.B4, spkrdef.SR);
  formant_5_special.InterPolePair(pars.F5, pars.B5, spkrdef.SR);
  formant_6_special.InterPolePair(pars.F6, pars.B6, spkrdef.SR);
  formant_7_special.InterPolePair(6500, 500, spkrdef.SR);
  formant_8_special.InterPolePair(7500, 600, spkrdef.SR);
  trach_pole_special.InterPolePair(pars.FTP, pars.BTP, spkrdef.SR);

  /* setup noise source */
  if (spkrdef.SB && !pars.AF && !pars.AH)
    synth.random = spkrdef.RS;

  /* synthesize some samples */
  for (index = 0; index < spkrdef.UI; index++)
  {
    sample = next_sample();
    if (sample <= 32767)
	{
      if (sample >= -32768)
        wave[index] = (INT16) round(sample);	/* round, not truncate */
      else
        wave[index] = -32768;
	}
    else
      wave[index] = 32767;

    if (fabs(sample) > sigmx)
      sigmx = fabs(sample);
  }
}
