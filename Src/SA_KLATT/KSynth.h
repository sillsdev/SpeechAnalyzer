// Derived from Synth.h from Sensimetrics corporatation
//
// Author steve MacLean
// Copyright JAARS 2001
//
// Modification History
//   Copied source from Synth.h
//   Removed all #defines for array indices
//   Created KSynth class
//   Removed all extern definitions moved into class
//   Created TEMPORAL struct replaced int par array
//   Created SPKRDEF removed replaced int spkrdef

/*
*      SenSyn - Version 1.1
*
*      Copyright (c) 1991, 1992 by Sensimetrics Corporation
*      All Rights Reserved.
*
*/

/*  synth.h - synthesizer support
*
*  coded by Eric P. Carlson
*
*  Modification History:
*    23 Feb 1993    changed INT16 to short, same as unix, v1.1
*/

/* amplitude adjustment constants */
#define A_AV -20.5
#define A_AH -64.3
#define A_AF -43.7
#define A_AP -28
#define A_A1 -58
#define A_A2F -70
#define A_A3F -77
#define A_A4F -82.6
#define A_A5F -86.7
#define A_A6F -87.7
#define A_AB -73.5
#define A_ANV -68.7
#define A_A1V -69.1
#define A_A2V -77.5
#define A_A3V -84.9
#define A_A4V -88.5
#define A_ATV -81.5
// new
#define A_A5V -88.5
#define A_A6V -88.5
#define A_A7V -88.5
#define A_A8V -88.5



/* Coefficients structure */
typedef struct SCoefficients {
    Float asp_amp;
    Float fric_amp;
    Float f1p_amp;
    Float f2p_amp;
    Float f3p_amp;
    Float f4p_amp;
    Float f5p_amp;
    Float f6p_amp;
    Float npv_amp;
    Float f1v_amp;
    Float f2v_amp;
    Float f3v_amp;
    Float f4v_amp;
    Float f5v_amp;
    Float f6v_amp;
    Float f7v_amp;
    Float f8v_amp;
    Float tpv_amp;
    Float bypass_amp;
} Coefficients;

/* Synthesizer structure */
struct Synthesizer {
    /* static data */
    BOOL parallel_only_flag;
    int num_casc_formants;
    int num_samples;
    int output_select;

    /* dynamic state data */
    int pulse_freq;
    BOOL glottis_open;
    int period_ctr;
    BOOL voicing_state;
    BOOL pulse;
    int random;
    int voicing_time;
    long global_time;
    Float voicing_amp;
    Float glottal_state;
    Float asp_state;
    Float integrator;
    BOOL close_shortened;
    int close_time;

    /* voicing state */
    int F0;
    int FL;
    int OQ;
    int SQ;
    int DI;
    int AV;
    int TL;

};

class CSilVoicing {
public:
    CSilVoicing();
    void clear();

    /* static data */
    Float dHarmonics;

    /* dynamic state data */
    Float pulse_freq;
    Float period_ctr;
    Float voicing_time;
    long global_time; // flutter time
    Float voicing_amp;


    /* voicing state */
    Float F0;
    Float FL;
    Float AV;
    int TL;

};

enum EKSYNTH_LOC {
    O_NORMAL = 0,
    O_VOICING = 1,
    O_ASPIRATION = 2,
    O_FRICATION = 3,
    O_GLOTTAL = 4,
    O_TRACHEAL_CASC = 5,
    O_NASAL_ZERO_CASC = 6,
    O_NASAL_POLE_CASC = 7,
    O_FORMANT_5_CASC = 8,
    O_FORMANT_4_CASC = 9,
    O_FORMANT_3_CASC = 10,
    O_FORMANT_2_CASC = 11,
    O_FORMANT_1_CASC = 12,
    O_FORMANT_6_PARA = 13,
    O_FORMANT_5_PARA = 14,
    O_FORMANT_4_PARA = 15,
    O_FORMANT_3_PARA = 16,
    O_FORMANT_2_PARA = 17,
    O_FORMANT_1_PARA = 18,
    O_NASAL_PARA = 19,
    O_BYPASS_PARA = 20,
    NUM_OUTPUTS = 21
};

class CKSynth {
public:
    CKSynth(SPKRDEF * init) : spkrdef(*init) {
        init_synthesizer();
    }


    void senswave(TEMPORAL * parameters, INT16 * wave);
    Float synthesizedWaveformMaximum() {
        return sigmx;
    }

private:
    SPKRDEF spkrdef;

    TEMPORAL pars;
    Float sigmx;

    Coefficients coefs;
    Synthesizer synth;
    CSilVoicing m_silVoicing;

    // Glottal Sound Sources
    CResonator glottal_pulse;
    CResonator spectral_tilt;

    // Cascade Vocal Tract Model Laryngeal Sound Sources
    CResonator nasal_pole_cascade;
    CResonator nasal_zero_cascade;
    CResonator trach_pole_cascade;
    CResonator trach_zero_cascade; /* anti-resonators */
    CResonator formant_1_cascade;
    CResonator formant_2_cascade;
    CResonator formant_3_cascade;
    CResonator formant_4_cascade;
    CResonator formant_5_cascade;
    CResonator formant_6_cascade;
    CResonator formant_7_cascade;
    CResonator formant_8_cascade;

    // Frication Sound Sources
    CResonator formant_2_parallel;
    CResonator formant_3_parallel;
    CResonator formant_4_parallel;
    CResonator formant_5_parallel;
    CResonator formant_6_parallel;

    // Parallel Vocal Tract Model Laryngeal (Normally Not Used)
    CResonator nasal_pole_special;
    CResonator trach_pole_special;
    CResonator formant_1_special;
    CResonator formant_2_special;
    CResonator formant_3_special;
    CResonator formant_4_special;
    CResonator formant_5_special;
    CResonator formant_6_special;
    CResonator formant_7_special;
    CResonator formant_8_special;

    Float out[NUM_OUTPUTS];

private:
    void init_synthesizer();
    Float next_voice_sample(void);
    Float next_sil_voice_sample(void);
    Float next_sample();
};
