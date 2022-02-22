#ifndef DSPWINS_H
#define DSPWINS_H

#include "dspTypes.h"
#include <vector>
#include "WindowSettings.h"

using std::vector;

class CDspWin {
public:
    CDspWin(int32 nLength, uint32 smpRate, int32 windowType);
    CDspWin(const CDspWin & from);
    CDspWin()=delete;
    CDspWin & operator=(const CDspWin & from);
    static CDspWin FromBandwidth(double bandwidth, uint32 smpRate, int32 windowType);
    const double * WindowDouble();
    const float * WindowFloat();
    static int32 CalcLength(double bandwidth, uint32 smpRate, int32 windowType);
    static int32 CalcEquivalentLength(int32 length, int32 dstWindowType, int32 srcWindowType = kRect);
    int32 Length() const;
    double Bandwidth() const;
    int32 Type() const;

public:
    enum { kRect=0, kHanning=1, kHann=1, kHamming=2, kBlackman=3, kBlackmanHarris=4, kGaussian=5 };

    class CWindowParms {
    public:
        CWindowParms(double bandwidthProduct, int32 terms, const double * coefficients) : m_bandwidthProduct(bandwidthProduct) , m_terms(terms) , m_coefficients(coefficients) {
        }

        const double m_bandwidthProduct;
        const int32 m_terms;
        const double * const m_coefficients;
    private:
        // Not used
        const CWindowParms & operator=(const CWindowParms &) {
        }
    };

private:

    int32 m_windowType;
    uint32 m_smpRate;
    int32 m_nLength;
    bool m_bWindowBuilt;
    vector<double> m_cDWindow;
    vector<float> m_cFWindow;

    void Build();
    static CWindowParms WindowParms(int32 windowType);
};

class CHanningWin {
public:
    static dspError_t FromLength(CHanningWin ** Hanning, int32 Length, uint32 smpRate);
    static dspError_t FromBandwidth(CHanningWin ** Hanning, float Bandwidth, uint32 smpRate);
    static int32 CalcLength(float Bandwidth, uint32 smpRate);
    int32 Length();
    float Bandwidth();
    float Data(uint32 i);
    ~CHanningWin();

private:
    CHanningWin(float * Data, int32 Length, uint32 SmpRate);
    float * m_Data;
    int32 m_Length;
    float m_Bandwidth;
};

class CKaiserWin {
public:
    static dspError_t Setup(CKaiserWin ** Kaiser, float Bandwidth, float Gain, float Atten, float SmpRate);
    ~CKaiserWin();
    float CalcCoeff(int32 i);
    float Coeff(uint32 i);
    void Fill(void);

private:
    CKaiserWin(float * Coeff, bool * CoeffSet, float Bandwidth, float Gain, float Atten,
               float Transition, int32 GroupDelay, float SmpRate);
    float m_Bandwidth;
    float m_Gain;
    float m_Atten;
    float m_Transition;
    float m_TradeOff;
    int32  m_GroupDelay;
    int32  m_Length;
    float m_SmpRate;
    float * m_Coeff;
    bool * m_CoeffSet;
};

#endif

