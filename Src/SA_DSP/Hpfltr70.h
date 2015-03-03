#ifndef HPFLTR70_H
#define HPFLTR70_H

#include "dspTypes.h"
#include "Error.h"
#include "Signal.h"

// #define TEST_HPF        // define to activate sine wave test driver

#define HIGHPASS_CUTIN_FREQ   70
// Highpass filter class definition.
class CHighPassFilter70 {
public:
    static char * Copyright(void);
    static float Version(void);
    static dspError_t CreateObject(CHighPassFilter70 ** ppoHPFilter, uint32 wSmpRate /* in Hz */);
    dspError_t Filter(uint8 * pProcessBuffer, uint32 dwProcessLength /* in samples */);
    dspError_t Filter(short * pProcessBuffer, uint32 dwProcessLength /* in samples */);
    dspError_t ForwardPass(uint8 * pProcessBuffer, uint32 dwProcessLength /* in samples */);
    dspError_t ForwardPass(short * pProcessBuffer, uint32 dwProcessLength /* in samples */);
    dspError_t BackwardPass(uint8 * pProcessBuffer, uint32 dwProcessLength /* in samples */);
    dspError_t BackwardPass(short * pProcessBuffer, uint32 dwProcessLength /* in samples */);

#ifdef TEST_HPF
    dspError_t DoBackwardPass(uint8 * pProcessBuffer, uint32 dwProcessLength /* in samples */);
    dspError_t DoBackwardPass(short * pProcessBuffer, uint32 dwProcessLength /* in samples */);
    void GenTestSuite(void * pProcessBuffer, uint32 dwProcessLength, uint16 wSmpSize);
#endif
    short GetBlockMin();
    short GetBlockMax();
    ~CHighPassFilter70();

private:
    static dspError_t ValidateSignalParms(uint16 wSmpRate);
    void SetFilterParms(uint32 wSmpRate);
    CHighPassFilter70(uint32 wSmpRate);
    bool m_bFirstForwardPass;
    bool m_bFirstBackwardPass;
#ifdef TEST_HPF
    bool m_bFirstTestPass;
#endif
    int32 m_f11;
    int32 m_b11;
    int32 m_b12;
    int32 m_g;
    int32 m_r11L;
    int32 m_r11H;
    int32 m_r12L;
    int32 m_r12H;
    short m_nBlockMin;
    short m_nBlockMax;
};

// Filter parameter workspace structure.
struct SHPFWorkspace {
    int32 f11;
    int32 b11;
    int32 b12;
    int32 g;
    int32 r10;
    int32 r11L;
    int32 r11H;
    int32 r12L;
    int32 r12H;
    int32 Sig;
    uint8 * pSig;
    uint8 * pStart;
    uint8 * pEnd;
    short nMin;
    short nMax;
};


#endif
