// Derived from Voice.c from Sensimetrics corporatation
//
// Author steve MacLean
// Copyright JAARS 2001
// 
// Modification History
//   2/14/2001 SDM Copied source from Voice.c
//                 Created KSynth class
//                 Created CResonator class
//                 Removed all extern definitions moved into class
//                 Created TEMPORAL struct replaced int par array
//                 Created SPKRDEF removed replaced int spkrdef


/*
*      SenSyn - Version 1.1
*
*      Copyright (c) 1991, 1992 by Sensimetrics Corporation
*      All Rights Reserved.
*
*/

/*
*  voice.c - synthesize one voice sample
*
*  coded by Eric P. Carlson
*
*  Modification History:
*     7 Nov 1991	removed from sample.c
*    21 Nov 1991	made spectral_tilt, formant_1s pitch-synchronous
*    10 Mar 1992	return spectral_tilt(0) when F0 = 0
*    23 Feb 1993	made lf tables same as unix version, v1.1
*/

#include "stdafx.h"
#include <math.h>		/* needed for power function */
#include "reson.h"		/* resonator support */
#include "synth.h"		/* synthesizer support */
#include "KSynth.h"

#pragma warning(push)  // these files generate a lot of warnings at level 4
#pragma warning(disable : 4275)
#include <complex>
#pragma warning(pop)

/* Spectral Tilt Bandwidth table */
static const int tilt[42] =
{
  5000, 4350, 3790, 3330, 2930, 2700, 2580, 2468, 2364, 2260,
  2157, 2045, 1925, 1806, 1687, 1568, 1449, 1350, 1272, 1199,
  1133, 1071, 1009,  947,  885,  833,  781,  729,  677,  625,
  599,  573,  547,  521,  495,  469,  442,  416,  390,  364,
  338,  312
};

/* LF Voicing Model tables */
static const Float bw_lf[41] =
{
  0.0,  -0.6,  -2.0,  -4.0,  -6.0,  -8.0, -10.4, -12.7, -15.3, -17.8,
  -20.1, -22.4, -24.7, -27.0, -29.2, -31.4, -33.6, -35.8, -37.9, -40.0,
  -42.1, -44.1, -46.2, -48.3, -50.4, -52.4, -54.5, -56.6, -57.8, -60.8,
  -62.7, -64.5, -66.3, -68.1, -69.9, -71.6, -73.3, -75.0, -76.6, -78.2,
  -79.6
};

static const Float e0_lf[41] =
{
  27.4, 26.3, 25.3, 24.3, 23.2, 22.1, 21.0, 20.0, 18.8, 17.6,
  16.1, 14.9, 13.8, 12.8, 11.7, 10.6, 9.81, 9.00, 8.12, 7.36,
  6.60, 6.05, 5.46, 4.92, 4.41, 3.94, 3.58, 3.14, 2.83, 2.49,
  2.24, 2.03, 1.83, 1.63, 1.48, 1.32, 1.19, 1.08, .982, .902,
  .832
};


/* ---------------------------------------------------------------------- */
static double whiteHarmonics(int nHarmonics, double time, double frequency, double phase, double dispersion);

Float CKSynth::next_voice_sample(void)
{
  register Float output = 0.0;
  register Float freq, bw, temp, time, open;
  register BOOL pulse;


  if(spkrdef.SS > 3)
    return next_sil_voice_sample();

  synth.global_time++;	/* flutter time runs continuously */

  /* check for F0 = zero */
  if (!pars.F0)
  {
    synth.period_ctr = 0;	/* make sure we open glottis when F0 != 0 */
    synth.glottis_open = FALSE;
    synth.voicing_state = FALSE;
    return spectral_tilt.AdvanceResonator(0.);
  }

  /* glottal opening/closing transition */
  if ((--synth.period_ctr) <= 0)
  {
    /* check for glottal transition */
    if (!synth.glottis_open)
    {
      /* glottis opening */
      synth.glottis_open = TRUE;
      synth.pulse = TRUE;
      synth.voicing_time = 0;
      synth.F0 = round(pars.F0*10.);
      synth.FL = round(pars.FL);
      synth.OQ = round(pars.OQ);
      synth.SQ = pars.SQ;
      synth.DI = round(pars.DI);
      synth.AV = round(pars.AV);
      synth.TL = pars.TL;
      synth.voicing_amp = synth.AV ? dB2amp(spkrdef.GV+synth.AV+A_AV) : 0;
      if (synth.FL)
      {
        Float seconds = (Float) synth.global_time / spkrdef.SR;
        freq = synth.F0 / 10. + (synth.FL / 50. * synth.F0 / 600. *
          (cos(TWO_PI * 12.7 * seconds) +
          cos(TWO_PI * 7.1 * seconds) +
          cos(TWO_PI * 4.7 * seconds)));
      }
      else
        freq = synth.F0 / 10.;
      synth.pulse_freq = round(spkrdef.SR / freq);
      synth.period_ctr = round(synth.pulse_freq * synth.OQ / 100.);
      synth.close_time = synth.pulse_freq - synth.period_ctr;
      if (synth.DI)
        if (!synth.close_shortened)
        {
          synth.close_shortened = TRUE;
          synth.close_time -= round(synth.close_time * synth.DI / 100.);
          synth.voicing_amp *= 1. - synth.DI / 100.;
          synth.TL += round(synth.DI / 4.25);	/** not quite right **/
        }
        else
        {
          /* previous close shortened */
          synth.close_shortened = FALSE;
          synth.close_time += round(synth.close_time * synth.DI / 100.);
        }
        else  /* no diplophonia */
          synth.close_shortened = FALSE;

        /* setup spectral tilt */
        if (spkrdef.SS == 3)	/* LF source corner rounding */
          synth.TL += 2;
        spectral_tilt.InterPolePair(round(0.375 * tilt[synth.TL]),
          tilt[synth.TL], spkrdef.SR);
        if (synth.TL > 10)
          spectral_tilt.InterAdjustGain(1.0 + (synth.TL-10)*(synth.TL-10) / 1000.);

        /* setup pitch-synchronous formant_1_cascade and formant_1_special */
        if(pars.DF1 || pars.DB1)
        {
          formant_1_cascade.InterPolePair(pars.F1 + pars.DF1,
            pars.B1 + pars.DB1, spkrdef.SR);
          formant_1_special.InterPolePair(pars.F1 + pars.DF1,
            pars.B1 + pars.DB1, spkrdef.SR);
        }

        switch (spkrdef.SS)
        {
        case 1:  /* impulsive source */
          synth.voicing_state = FALSE;
          temp = (Float) synth.pulse_freq * synth.OQ / 100.;
          glottal_pulse.InterPolePair(0, round(10000. / temp), spkrdef.SR);
          glottal_pulse.InterAdjustGain(0.002675 * temp * temp); /* -51.3 dB */
          break;

        case 2:  /* natural source */
          /** nothing to do **/
          break;

        case 3:  /* LF source */
          freq = synth.F0 / (22. * synth.OQ / 100.) *
            (synth.SQ + 100.) / synth.SQ;
          temp = (bw_lf[synth.SQ / 10 - 9] -
            bw_lf[synth.SQ / 10 - 10]) / 10.;
          bw = (bw_lf[synth.SQ / 10 - 10] + (synth.SQ % 10) * temp) *
            200. / (synth.pulse_freq * synth.OQ / 100.);
          glottal_pulse.InterPolePair(round(freq), round(bw), spkrdef.SR);
          temp = (e0_lf[synth.SQ / 10 - 9] -
            e0_lf[synth.SQ / 10 - 10]) / 10.;
          glottal_pulse.InterAdjustGain((e0_lf[synth.SQ / 10 - 10] +
            (synth.SQ % 10) * temp) *
            (synth.pulse_freq * synth.OQ / 100.) / 200.);
          break;
        }
    }
    else
    {
      /* glottis closing */
      synth.glottis_open = FALSE;
      synth.voicing_time++;
      synth.pulse = FALSE;
      synth.period_ctr = synth.close_time;
      if(pars.DF1 || pars.DB1)
      {
        formant_1_cascade.InterPolePair(pars.F1, pars.B1, spkrdef.SR);
        formant_1_special.InterPolePair(pars.F1, pars.B1, spkrdef.SR);
      }
    }
  }
  else
  {
    /* during open or closed phase */
    synth.voicing_time++;
    synth.pulse = FALSE;
  }


  /* ---------------------------------------------------------------------- */
  /* voicing source */

  switch (spkrdef.SS)
  {
  case 1:	/* impulse source */
    pulse = (BOOL)(synth.pulse - synth.voicing_state);
    synth.voicing_state = synth.pulse;
    output = spectral_tilt.AdvanceResonator(
               glottal_pulse.AdvanceResonator(pulse * synth.voicing_amp));
    break;

  case 2:	/* natural KLGLOT88 source */
    if (synth.glottis_open)
    {
      time = (Float) synth.voicing_time;
      open = (Float) synth.pulse_freq * synth.OQ / 100.;
      temp = synth.voicing_amp * .00055 *
        (synth.pulse_freq ? 100. / synth.pulse_freq : 0) *
        (50. / synth.OQ) *
        (2 * time - 3 * time * time / open);
      output = spectral_tilt.AdvanceResonator(temp);
    }
    else
      output = spectral_tilt.AdvanceResonator(0.);
    break;

  case 3:	/* LF model source */
    if (synth.glottis_open)
      output = spectral_tilt.AdvanceResonator(
        glottal_pulse.AdvanceResonator(synth.pulse ? (synth.voicing_amp * 0.0795) : 0.));
    else
    {
      glottal_pulse.ClearResonator();
      output = spectral_tilt.AdvanceResonator(0.); // sdm bug fix this makes sense, used to no braces
    }
    break;
  }

  return output;
}


/* ---------------------------------------------------------------------- */

Float CKSynth::next_sil_voice_sample(void)
{
  Float output = 0.0;

  m_silVoicing.global_time++;	/* flutter time runs continuously */

  /* check for F0 = zero */
  if (!pars.F0 && !m_silVoicing.F0)
  {
    m_silVoicing.period_ctr = 0;	/* make sure we open glottis when F0 != 0 */
    return spectral_tilt.AdvanceResonator(0.);
  }

  if ((--m_silVoicing.period_ctr) <= 0.5)
  {
    // pitch synchronus parameter changes
    m_silVoicing.F0 = pars.F0;
    if(!m_silVoicing.F0)
    {
      m_silVoicing.period_ctr = 0;	/* make sure we open glottis when F0 != 0 */
      return spectral_tilt.AdvanceResonator(0.);
    }
    m_silVoicing.FL = pars.FL;
    m_silVoicing.AV = pars.AV;
    m_silVoicing.TL = pars.TL;
    m_silVoicing.voicing_amp = m_silVoicing.AV ? dB2amp(spkrdef.GV+m_silVoicing.AV+-40) : 0;
    m_silVoicing.voicing_time = m_silVoicing.period_ctr + 1;

    double freq;
    if (m_silVoicing.FL)
    {
      Float seconds = (Float) m_silVoicing.global_time / spkrdef.SR;
      freq = m_silVoicing.F0 + (m_silVoicing.FL / 50. * m_silVoicing.F0 / 600. *
        (cos(TWO_PI * 12.7 * seconds) +
        cos(TWO_PI * 7.1 * seconds) +
        cos(TWO_PI * 4.7 * seconds)));
    }
    else
      freq = m_silVoicing.F0;
    
    m_silVoicing.pulse_freq = freq / spkrdef.SR;
    m_silVoicing.period_ctr += 1/m_silVoicing.pulse_freq;
    
    /* setup spectral tilt */
    spectral_tilt.InterPolePair(0.375 * tilt[m_silVoicing.TL],
      tilt[m_silVoicing.TL], spkrdef.SR);
    if (m_silVoicing.TL > 10)
      spectral_tilt.InterAdjustGain(1.0 + (m_silVoicing.TL-10)*(m_silVoicing.TL-10) / 1000.);
    
    /* setup pitch-synchronous formant_1_cascade and formant_1_special */
    formant_1_cascade.InterPolePair(pars.F1, pars.B1, spkrdef.SR);
    formant_1_special.InterPolePair(pars.F1, pars.B1, spkrdef.SR);

    switch (spkrdef.SS)
    {
    case 7:   // 20dB decaying 
    default:
      m_silVoicing.dHarmonics = int(0.25/m_silVoicing.pulse_freq)*2; // always even
      break;
    }
    
  }

  
  /* ---------------------------------------------------------------------- */
  /* voicing source */
  
  double whiteRadiated = 0;
  
  switch (spkrdef.SS)
  {
  case 4:	// Heartbeat rising, falling discontinuity, rising
    whiteRadiated = whiteHarmonics((int)m_silVoicing.dHarmonics, m_silVoicing.voicing_time, m_silVoicing.pulse_freq, -90, 180);
    break;
  case 5:	// Positive impulse
    whiteRadiated = whiteHarmonics((int)m_silVoicing.dHarmonics, m_silVoicing.voicing_time, m_silVoicing.pulse_freq, 180, 180);
    break;
  case 6:	// Mixture Topheavy Heartbeat
    whiteRadiated = whiteHarmonics((int)m_silVoicing.dHarmonics, m_silVoicing.voicing_time, m_silVoicing.pulse_freq, -135, 180);
    break;
  case 7:   // 20dB decaying 
  default:
    ;
  }
  output = spectral_tilt.AdvanceResonator(m_silVoicing.voicing_amp*whiteRadiated/m_silVoicing.dHarmonics);
    
  m_silVoicing.voicing_time++;

  return output;
}


SilVoicing::SilVoicing()
{
  clear();
}

void SilVoicing::clear()
{
  SilVoicing &synth = *this;
  
  synth.pulse_freq = 0;
  synth.period_ctr = 0;
  synth.voicing_time = 0;
  synth.global_time = 0;
  synth.voicing_amp = 0;

  synth.F0 = 0;
  synth.FL = 0;
  synth.AV = 0;
  synth.TL = 0;

  // transient data (should not matter)
  synth.dHarmonics = 1;
}


// whiteHarmonics() is designed to calculate a harmonic series of sinusoids each with the same 
// amplitude.  
//
//               sum<n = 1..nHamonics>( cos(n*f*t + phase(n)) );
//
//               phase(n) = phase + (n - 1)*dispersion;
//
// using eulers relation eITheta = cos(Theta) + i*sin(Theta)
// this becomes the real part of a geometric series
static double whiteHarmonics(int nHarmonics, double time, double freq, double phase, double dispersion)
{
  // sum of the first n terms of a geometric series
  // sn = a1 * (1 - r^n)/(1 - r)  r!=1
  double radiansR = TWO_PI*(time*freq+dispersion/360);
  std::complex<double> whiteHarmonics = 0.;
    
  // assume a1 = 1
  if(cos(radiansR) != 1.)
  {
    std::complex<double> r(cos(radiansR),sin(radiansR));
    std::complex<double> rN(cos(nHarmonics * radiansR),sin(nHarmonics * radiansR));
    
    whiteHarmonics = (1.- rN)/(1. - r);
  }
  else
  {
    // For this case the geometric series formulas do not work
    // but n*1 == n....
    whiteHarmonics = nHarmonics;
  }
  
  //  angle a1 = f*t + phase
  double radiansA1 = TWO_PI*(time*freq+phase/360);
  std::complex<double> a1(cos(radiansA1),sin(radiansA1));

  whiteHarmonics *= a1;

  return whiteHarmonics.real(); // the cos() part
}