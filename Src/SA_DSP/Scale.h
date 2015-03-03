#ifndef SCALE_H
#define SCALE_H

enum ESCALE_OPTIONS {DB = 0 , LINEAR = 1};         //scale options

/* Minimum log power is derived from the theoretical signal-to-quantization noise level given by  */
/* 6*b - 1.24, when b is the number of quantization bits per sample.  For 16-bit data, this works */
/* out to be 94.76 dB, so minimum power should be at least 94.76 dB down from full range, which   */
/* is defined to be 3 dB.                                                                         */
#define MIN_LOG_PWR   -110.F    // this matches the default for the spectrum power scale

/* Power in dB is defined as 20*log(magnitude).  Therefore, divide by 20 to get log magnitude.    */
#define MIN_LOG_MAG   (MIN_LOG_PWR / 20.F)

class Scale {
public:
    //static char *Copyright(void);
    //static float Version(void);
    //static dspError_t CreateObject()
    Scale() {
        ;
    }
    // From Parsons, Voice_and_Speech_Processing, McGraw-Hill, c 1987, ISBN 0-07-048541-0, p 73
    // static double ToMels(double dFrequency) {return (1000.*log10(1 + dFrequency/1000.)/log10(2.));}
    // The following formula seems to be more common in the auditory literature
    static double ToMels(double dFrequency) {
        return 2595.*log10(1 + dFrequency/700.);
    }
    // From Traunmueller (1990)
    static double ToBarks(double dFrequency) {
        return 26.81/(1 + 1960./dFrequency) - 0.53;
    }
    // From B.C.J. More and B.R. Glasberg (1983) "Suggested formulae for calculating auditory-filter
    // bandwidths and excitation patterns" J. Acoust. Soc. Am. 74: 750-753.
    static double ToErbs(double dFrequency) {
        return 11.17*log((dFrequency + 312.)/(dFrequency + 14675.)) + 43.;
    }
    ~Scale() {
        ;
    };
private:
protected:
};


#endif

