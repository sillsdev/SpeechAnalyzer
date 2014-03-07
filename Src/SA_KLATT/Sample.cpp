// Derived from Sample.c from Sensimetrics corporatation
//
// Author steve MacLean
// Copyright JAARS 2001
//
// Modification History
//   2/14/2001 SDM Copied source from Sample.c
//                 Created KSynth class
//                 Created CResonator class
//                 Removed all extern definitions moved into class
//                 Created TEMPORAL struct replaced int par array
//                 Created SPKRDEF removed replaced int spkrdef

/*
*      SenSyn - Version 1.0
*
*      Copyright (c) 1991, 1992 by Sensimetrics Corporation
*      All Rights Reserved.
*
*/

/*
*  sample.c - synthesize one sample
*
*  coded by Eric P. Carlson
*
*  Modification History:
*    14 Jan 1993       sign-extend modulo in noise calculation
*     8 Jan 1993    corrected modulo arithmetic in noise calculation
*    10 Mar 1992    removed aspiration modulation when F0 = 0
*     5 Nov 1991    removed from spyn.c
*     7 Nov 1991    moved voicing source to voice.c
*/

#include "stdafx.h"
#include <math.h>       /* needed for power function */
#include "reson.h"      /* resonator support */
#include "synth.h"      /* synthesizer support */
#include "KSynth.h"

/* ---------------------------------------------------------------------- */

Float CKSynth::next_sample()
{
    Float noise, first_diff, output, special;
    register Float casc;


    out[O_VOICING] = next_voice_sample();


    /* ---------------------------------------------------------------------- */
    /* aspiration/frication noise source */
    noise = (int)(short)(synth.random = ((synth.random * 20077L + 12345L)
                                         % 65536L)) / 65536.;
    if (pars.F0 && !synth.glottis_open && synth.AV)
    {
        noise /= 2;
    }
    out[O_ASPIRATION] = coefs.asp_amp * noise - synth.asp_state;
    synth.asp_state = coefs.asp_amp * noise;
    out[O_GLOTTAL] = 12. * out[O_VOICING] + 5. * out[O_ASPIRATION];
    out[O_FRICATION] = -(coefs.fric_amp * noise);


    /* ---------------------------------------------------------------------- */
    /* cascade synthesis */

    if (!synth.parallel_only_flag)
    {
        casc = out[O_NASAL_POLE_CASC] = nasal_pole_cascade.AdvanceResonator(
                                            out[O_NASAL_ZERO_CASC] = nasal_zero_cascade.AdvanceAntiResonator(
                                                    out[O_TRACHEAL_CASC] = trach_pole_cascade.AdvanceResonator(
                                                            trach_zero_cascade.AdvanceAntiResonator(out[O_GLOTTAL] / 4.))));
        switch (spkrdef.NF)
        {
        case 8:
            casc = formant_8_cascade.AdvanceResonator(casc);
        case 7:
            casc = formant_7_cascade.AdvanceResonator(casc);
        case 6:
            casc = formant_6_cascade.AdvanceResonator(casc);
        case 5:
            casc = out[O_FORMANT_5_CASC] =
                       formant_5_cascade.AdvanceResonator(casc);
        case 4:
            casc = out[O_FORMANT_4_CASC] =
                       formant_4_cascade.AdvanceResonator(casc);
        case 3:
            casc = out[O_FORMANT_3_CASC] =
                       formant_3_cascade.AdvanceResonator(casc);
        case 2:
            casc = out[O_FORMANT_2_CASC] =
                       formant_2_cascade.AdvanceResonator(casc);
        case 1:
        default:
            out[O_FORMANT_1_CASC] =
                formant_1_cascade.AdvanceResonator(casc);
        }

        /* if using cascade, clear state of special parallel branch */
        synth.glottal_state = first_diff = special = 0;
    }


    /* ---------------------------------------------------------------------- */
    /* special parallel branch */

    else
    {

        first_diff = out[O_GLOTTAL] - synth.glottal_state;
        synth.glottal_state = out[O_GLOTTAL];

        special = (out[O_NASAL_PARA] = nasal_pole_special.AdvanceResonator(
                                           out[O_GLOTTAL] * coefs.npv_amp))
                  + (out[O_FORMANT_1_PARA] = formant_1_special.AdvanceResonator(
                          out[O_GLOTTAL] * coefs.f1v_amp))
                  - formant_2_special.AdvanceResonator(first_diff * coefs.f2v_amp)
                  + formant_3_special.AdvanceResonator(first_diff * coefs.f3v_amp)
                  - formant_4_special.AdvanceResonator(first_diff * coefs.f4v_amp)
                  + formant_5_special.AdvanceResonator(first_diff * coefs.f5v_amp)
                  - formant_6_special.AdvanceResonator(first_diff * coefs.f6v_amp)
                  + formant_7_special.AdvanceResonator(first_diff * coefs.f7v_amp)
                  - formant_8_special.AdvanceResonator(first_diff * coefs.f8v_amp)
                  + trach_pole_special.AdvanceResonator(first_diff * coefs.tpv_amp);

        /* if using parallel, clear state of special cascade branch */
        out[O_FORMANT_1_CASC] = 0.;
    }


    /* ---------------------------------------------------------------------- */
    /* parallel synthesis */

    out[O_FORMANT_2_PARA] = formant_2_parallel.AdvanceResonator(out[O_FRICATION] * coefs.f2p_amp);
    out[O_FORMANT_3_PARA] = formant_3_parallel.AdvanceResonator(out[O_FRICATION] * coefs.f3p_amp);
    out[O_FORMANT_4_PARA] = formant_4_parallel.AdvanceResonator(out[O_FRICATION] * coefs.f4p_amp);
    out[O_FORMANT_5_PARA] = formant_5_parallel.AdvanceResonator(out[O_FRICATION] * coefs.f5p_amp);
    out[O_FORMANT_6_PARA] = formant_6_parallel.AdvanceResonator(out[O_FRICATION] * coefs.f6p_amp);
    out[O_BYPASS_PARA] = out[O_FRICATION] * coefs.bypass_amp;


    /* ---------------------------------------------------------------------- */
    /* normal output */

    out[O_NORMAL] = out[O_FORMANT_1_CASC] +
                    out[O_FORMANT_2_PARA] - out[O_FORMANT_3_PARA] +
                    out[O_FORMANT_4_PARA] - out[O_FORMANT_5_PARA] +
                    out[O_FORMANT_6_PARA] - out[O_BYPASS_PARA] + special;

    switch (spkrdef.OS)
    {
    case 1:
    case 2:
    case 3:
        output = out[spkrdef.OS] + 0.99 * synth.integrator;
        synth.integrator = output;
        break;

    default:
        output = out[spkrdef.OS];
        break;
    }
    return output;
}
