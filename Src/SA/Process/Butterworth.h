// Butterworth.h: interface for the CProcessButterworth class.
//
//////////////////////////////////////////////////////////////////////
#ifndef BUTTERWORTH_H
#define BUTTERWORTH_H

#include "IIRFilter.h"
#include "Process.h"

class CProcessButterworth : public CProcessIIRFilter {
public:
    CProcessButterworth(BOOL bWorkBenchOutput = TRUE);
    virtual ~CProcessButterworth();

    void LowPass(int nOrder, double dFrequency, double dScale=1.);
    void HighPass(int nOrder, double dFrequency, double dScale=1.);
    void BandPass(int nOrder, double dFrequency, double dBandwidth, double dScale=1.);
    void SetFilterFilter(bool bSet);
    virtual long Process(void * pCaller, ISaDoc *, int nProgress = 0, int nLevel = 1);
    void ConfigureProcess(double dSampling);
    double ForwardTick(double data);
    void CascadeLowPass(CZTransform & zTransform, int nOrder, double dFrequency, double dSampling, double & tau);
    void CascadeHighPass(CZTransform & zTransform, int nOrder, double dFrequency, double dSampling, double & tau);
    void CascadeScale(CZTransform & zTransform, double dScale);
    void ClearFilter();
    void SetReverse(BOOL bSet);
    double FilterFilterNorm(int nOrder) const;
    static int round2Int(double value);

private:
    enum FilterType {
        kftUndefined,
        kftHighPass,
        kftLowPass,
        kftBandPass
    };
    bool m_bFilterFilter;
    BOOL m_bReverse;
    int m_nOrder;
    FilterType m_ftFilterType;
    double m_dSampling;
    double m_dFrequency;
    double m_dBandwidth;
    double m_dScale;
};

#endif
