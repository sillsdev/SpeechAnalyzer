// Author Steve MacLean
// Copyright JAARS 2001
//
// Modification History
//   2/14/2001 SDM Created to provide a clean interface to the Klatt synthesizer.
//                    Designed to hide class details.
//   2/22/2001 SDM Created PARAMETER_DESC concept to allow programatic access to Klatt defaults and desriptions

typedef double Float;
typedef short INT16;

#include <tchar.h>

struct PARAMETER_DESC {
    size_t parameterOffset;
    TCHAR const * symbolDescription;
    TCHAR const * typeScanf;
    Float min;
    Float val;
    Float max;
    TCHAR const * description;
    TCHAR const * units;
};

struct SPKRDEF {
    int DU;//UtteranceDuration;
    int UI;//UpdateInterval;
    int SR;//SamplingRate;
    int NF;//NumberCascadedFormants;
    int SS;//SourceSwitch;
    int RS;//RandomSeed;
    BOOL SB;//SameBurst;
    BOOL CP;//CascadeParallelSwitch;
    int OS;//OutputSwitch;
    Float GV;//VoicingGain;
    Float GH;//AspirationGain;
    Float GF;//FricationGain;
} ;

struct TEMPORAL {
    Float F0;
    Float AV;
    Float OQ;
    int SQ;
    int TL;
    Float FL;
    Float DI;
    Float AH;
    Float AF;
    Float F1;
    Float B1;
    Float DF1;
    Float DB1;
    Float F2;
    Float B2;
    Float F3;
    Float B3;
    Float F4;
    Float B4;
    Float F5;
    Float B5;
    Float F6;
    Float B6;
    Float FNP;
    Float BNP;
    Float FNZ;
    Float BNZ;
    Float FTP;
    Float BTP;
    Float FTZ;
    Float BTZ;
    Float A2F;
    Float A3F;
    Float A4F;
    Float A5F;
    Float A6F;
    Float AB;
    Float B2F;
    Float B3F;
    Float B4F;
    Float B5F;
    Float B6F;
    Float ANV;
    Float A1V;
    Float A2V;
    Float A3V;
    Float A4V;
    Float ATV;
    Float A5V;
    Float A6V;
    Float A7V;
    Float A8V;
};

class CKSynth;

CKSynth * CreateKSynth(SPKRDEF * spkrdef);

void DestroyKSynth(CKSynth * synth);

void SynthesizeFrame(CKSynth * synth, TEMPORAL * par, INT16 * wave);

Float WaveformMaximum(CKSynth * synth);

const PARAMETER_DESC * GetTemporalKlattDesc();
const PARAMETER_DESC * GetGlobalKlattDesc();

const TEMPORAL & GetTemporalKlattDefaults();
const SPKRDEF & GetGlobalKlattDefaults();