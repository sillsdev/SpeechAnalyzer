// Author Steve MacLean
// Copyright JAARS 2001
//
// Modification History
//   2/22/2001 SDM Created SParameterDescription concept to allow programatic access to Klatt defaults and desriptions
//                  copied text from sensimetrics parameter file (text from Klatt)


#include "stdafx.h"
#include <stddef.h>
#include "synth.h"

#define kp(sym, type, min, val, max, description, units) \
{ offsetof(TEMPORAL, sym), _T(#sym), _T("%") _T(#type), min, val, max, _T(description) , _T(units)}

static const SParameterDescription temporalDesc[] = {
    kp(F0,  lgdddd,     0,  1000,  5000  ,"Fundamental frequency","Hz"),
    kp(AV,  lg,     0,    60,    80  ,"Amplitude of voicing","dB"),
    kp(OQ,  lg,    10,    50,    99  ,"Open quotient (voicing open-time/period)","%"),
    kp(SQ,   d,   100,   200,   500  ,"Speed quotient (rise/fall time, LF model)","%"),
    kp(TL,   d,     0,     0,    41  ,"Extra tilt of voicing spectrum","dB down @ 3 kHz"),
    kp(FL,  lg,     0,     0,   100  ,"Flutter (random fluct in f0)","% of maximum"),
    kp(DI,  lg,     0,     0,   100  ,"Diplophonia (alt periods closer)","% of max"),
    kp(AH,  lg,     0,     0,    80  ,"Amplitude of aspiration","dB"),
    kp(AF,  lg,     0,     0,    80  ,"Amplitude of frication","dB"),
    kp(F1,  lg,   180,   500,  1300  ,"Frequency of 1st formant","Hz"),
    kp(B1,  lg,    30,    60,  1000  ,"Bandwidth of 1st formant","Hz"),
    kp(DF1, lg,     0,     0,   100  ,"Change in F1 during open portion of period","Hz"),
    kp(DB1, lg,     0,     0,   400  ,"Change in B1 during open portion of period","Hz"),
    kp(F2,  lg,   550,  1500,  3000  ,"Frequency of 2nd formant","Hz"),
    kp(B2,  lg,    40,    90,  1000  ,"Bandwidth of 2nd formant","Hz"),
    kp(F3,  lg,  1200,  2500,  4800  ,"Frequency of 3rd formant","Hz"),
    kp(B3,  lg,    60,   150,  1000  ,"Bandwidth of 3rd formant","Hz"),
    kp(F4,  lg,  2400,  3250,  4990  ,"Frequency of 4th formant","Hz"),
    kp(B4,  lg,   100,   200,  1000  ,"Bandwidth of 4th formant","Hz"),
    kp(F5,  lg,  3000,  3700,  4990  ,"Frequency of 5th formant","Hz"),
    kp(B5,  lg,   100,   200,  1500  ,"Bandwidth of 5th formant","Hz"),
    kp(F6,  lg,  3000,  4990,  4990  ,"Frequency of 6th formant","Hz"),
    kp(B6,  lg,   100,   500,  4000  ,"Bandwidth of 6th formant","Hz"),
    kp(FNP, lg,   180,   500,  2000  ,"Frequency of nasal pole","Hz"),
    kp(BNP, lg,    40,    90,  1000  ,"Bandwidth of nasal pole","Hz"),
    kp(FNZ, lg,   180,   500,  2000  ,"Frequency of nasal zero","Hz"),
    kp(BNZ, lg,    40,    90,  1000  ,"Bandwidth of nasal zero","Hz"),
    kp(FTP, lg,   300,  2150,  3000  ,"Frequency of tracheal pole","Hz"),
    kp(BTP, lg,    40,   180,  1000  ,"Bandwidth of tracheal pole","Hz"),
    kp(FTZ, lg,   300,  2150,  3000  ,"Frequency of tracheal zero","Hz"),
    kp(BTZ, lg,    40,   180,  2000  ,"Bandwidth of tracheal zero","Hz"),
    kp(A2F, lg,     0,     0,    80  ,"Amp of fric-excited parallel 2nd formant","dB"),
    kp(A3F, lg,     0,     0,    80  ,"Amp of fric-excited parallel 3rd formant","dB"),
    kp(A4F, lg,     0,     0,    80  ,"Amp of fric-excited parallel 4th formant","dB"),
    kp(A5F, lg,     0,     0,    80  ,"Amp of fric-excited parallel 5th formant","dB"),
    kp(A6F, lg,     0,     0,    80  ,"Amp of fric-excited parallel 6th formant","dB"),
    kp(AB,  lg,     0,     0,    80  ,"Amp of fric-excited parallel bypass path","dB"),
    kp(B2F, lg,    40,   250,  1000  ,"Bw of fric-excited parallel 2nd formant","Hz"),
    kp(B3F, lg,    60,   300,  1000  ,"Bw of fric-excited parallel 3rd formant","Hz"),
    kp(B4F, lg,   100,   320,  1000  ,"Bw of fric-excited parallel 4th formant","Hz"),
    kp(B5F, lg,   100,   360,  1500  ,"Bw of fric-excited parallel 5th formant","Hz"),
    kp(B6F, lg,   100,  1500,  4000  ,"Bw of fric-excited parallel 6th formant","Hz"),
    kp(ANV, lg,     0,     0,    80  ,"Amp of voice-excited parallel nasal form.","dB"),
    kp(A1V, lg,     0,    60,    80  ,"Amp of voice-excited parallel 1st formant","dB"),
    kp(A2V, lg,     0,    60,    80  ,"Amp of voice-excited parallel 2nd formant","dB"),
    kp(A3V, lg,     0,    60,    80  ,"Amp of voice-excited parallel 3rd formant","dB"),
    kp(A4V, lg,     0,    60,    80  ,"Amp of voice-excited parallel 4th formant","dB"),
    kp(ATV, lg,     0,     0,    80  ,"Amp of voice-excited par tracheal formant","dB"),
    kp(A5V, lg,     0,     0,    80  ,"Amp of voice-excited parallel 5th formant","dB"),
    kp(A6V, lg,     0,     0,    80  ,"Amp of voice-excited parallel 6th formant","dB"),
    kp(A7V, lg,     0,     0,    80  ,"Amp of voice-excited parallel 7th formant","dB"),
    kp(A8V, lg,     0,     0,    80  ,"Amp of voice-excited parallel 8th formant","dB"),
    -1
};

#undef kp
#define kp(sym, type, min, val, max, description, units) \
{ offsetof(SPKRDEF, sym), _T(#sym), _T("%") _T(#type), min, val, max, _T(description) , _T(units)}

static const SParameterDescription constDesc[] = {
    kp(DU,   d,    30,   500,  5000,  "Duration of the utterance", "msec"),
    kp(UI,   d,     1,     5,    20,  "Update interval for parameter reset", "msec"),
    kp(SR,   d,  5000, 10000, 20000,  "Output sampling rate", "samples/sec"),
    kp(NF,   d,     1,     5,     6,  "Number of formants in cascade branch", ""),
    kp(SS,   d,     1,     2,     3,  "Source switch (1=impulse, 2=natural, 3=LF model)", ""),
    kp(RS,   d,     1,     8,  8191,  "Random seed (initial value of random # generator)", ""),
    kp(SB,   d,     0,     1,     1,  "Same noise burst, reset RS if AF=AH=0, 0=no,1=yes", ""),
    kp(CP,   d,     0,     0,     1,  "0=Cascade, 1=Parallel tract excitation by AV", ""),
    kp(OS,   d,     0,     0,    20,  "Output selector (0=normal,1=voicing source,...)", ""),
    kp(GV,  lg,     0,    60,    80,  "Overall gain scale factor for AV", "dB"),
    kp(GH,  lg,     0,    60,    80,  "Overall gain scale factor for AH", "dB"),
    kp(GF,  lg,     0,    60,    80,  "Overall gain scale factor for AF", "dB"),
    -1
};

static const TEMPORAL temporalDefaults = {
    100,
    60,
    50,
    200,
    0,
    0,
    0,
    0,
    0,
    500,
    60,
    0,
    0,
    1500,
    90,
    2500,
    150,
    3250,
    200,
    3700,
    200,
    4990,
    500,
    500,
    90,
    500,
    90,
    2150,
    180,
    2150,
    180,
    0,
    0,
    0,
    0,
    0,
    0,
    250,
    300,
    320,
    360,
    1500,
    0,
    60,
    60,
    60,
    60,
    0,
    0,
    0,
    0,
    0
};

static SPKRDEF globalDefaults = {
    500,
    5,
    10000,
    8,
    2,
    8,
    TRUE,
    FALSE,
    0,
    60,
    60,
    60,
};

const SParameterDescription * GetTemporalKlattDesc() {
    return temporalDesc;
}

const SParameterDescription * GetGlobalKlattDesc() {
    return constDesc;
}


const TEMPORAL & GetTemporalKlattDefaults() {
    return temporalDefaults;
}

const SPKRDEF & GetGlobalKlattDefaults() {
    return globalDefaults;
}