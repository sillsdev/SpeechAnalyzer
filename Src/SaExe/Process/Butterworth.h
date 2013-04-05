// Butterworth.h: interface for the CButterworth class.
//
//////////////////////////////////////////////////////////////////////
#ifndef BUTTERWORTH_H
#define BUTTERWORTH_H

#include "IProcess.h"
#include "Process.h"
#include "dsp\ZTransform.h"

class CProcessIIRFilter : public CProcess
{
public:
    CProcessIIRFilter(BOOL bDstWBench = TRUE);
    virtual ~CProcessIIRFilter();

    void SetSourceProcess(IProcess * pSourceProcess, BOOL bWBenchProcess = TRUE);
    void SetOutputType(BOOL bWBenchProcess = TRUE);
    virtual long Process(void * pCaller, ISaDoc *, int nProgress = 0, int nLevel = 1);
    void SetFilterFilterSilenceSamples(int forwardSamples);
    int FilterFilterSilenceSamples();
    CZTransform GetForward();

protected:
    CZTransform m_zForwardTransform;
    CZTransform m_zReverseTransform;

private:
    enum
    {
        DEFAULT_FILTER_FILTER_SILENCE_SAMPLES = 4096
    };

    void SetFilterFilter(BOOL bSet);
    static int round(double value);
    int ReadSourceData(DWORD dwDataPos, int wSmpSize, ISaDoc * pDoc);
    // write a block into the temporary file
    BOOL WriteWaveDataBlock(DWORD dwPosition, HPSTR lpData, DWORD dwDataLength);
    BOOL m_bReverse;
    BOOL m_bFilterFilter;
    int m_nFilterFilterSilence;
    BOOL m_bSrcWBenchProcess;
    BOOL m_bDstWBenchProcess;
    IProcess * m_pSourceProcess;
};

class CButterworth : public CProcessIIRFilter
{
public:
    CButterworth(BOOL bWorkBenchOutput = TRUE);
    virtual ~CButterworth();

    void LowPass(int nOrder, double dFrequency, double dScale=1.);
    void HighPass(int nOrder, double dFrequency, double dScale=1.);
    void BandPass(int nOrder, double dFrequency, double dBandwidth, double dScale=1.);
    void SetFilterFilter(BOOL bSet);
    virtual long Process(void * pCaller, ISaDoc *, int nProgress = 0, int nLevel = 1);
    void ConfigureProcess(double dSampling);
    double ForwardTick(double data);
    void CascadeLowPass(CZTransform & zTransform, int nOrder, double dFrequency, double dSampling, double & tau);
    void CascadeHighPass(CZTransform & zTransform, int nOrder, double dFrequency, double dSampling, double & tau);
    void CascadeScale(CZTransform & zTransform, double dScale);
    void ClearFilter();
    void SetReverse(BOOL bSet);
    double FilterFilterNorm(int nOrder) const;
    static int round(double value);

private:
    enum FilterType
    {
        kftUndefined,
        kftHighPass,
        kftLowPass,
        kftBandPass
    };
    BOOL m_bFilterFilter;
    BOOL m_bReverse;
    int m_nOrder;
    FilterType m_ftFilterType;
    double m_dSampling;
    double m_dFrequency;
    double m_dBandwidth;
    double m_dScale;
};

class CHilbert : public CProcessIIRFilter
{
public:
    CHilbert(CProcess * pSourceProcess = NULL, BOOL bWBenchProcess = FALSE);

private:
    static const double Pole1000x96dB[];

    CZTransform AllPass(double pole);
    CZTransform DelayHalf();
};

#endif
