// Butterworth.h: interface for the CProcessButterworth class.
//
//////////////////////////////////////////////////////////////////////
#ifndef IIRFILTER_H
#define IIRFILTER_H

#include "IProcess.h"
#include "Process.h"

class CProcessIIRFilter : public CProcess {
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
    BOOL m_bReverse;
    bool m_bFilterFilter;
    int m_nFilterFilterSilence;
    BOOL m_bSrcWBenchProcess;
    BOOL m_bDstWBenchProcess;
    IProcess * m_pSourceProcess;

private:
    enum {
        DEFAULT_FILTER_FILTER_SILENCE_SAMPLES = 4096
    };

    void SetFilterFilter(bool bSet);
    static int round2Int(double value);
    int ReadSourceData(DWORD dwDataPos, int wSmpSize, ISaDoc * pDoc);
    long ProcessForward(ISaDoc * pDoc, IProcess * pLowerProcess, int & nProgress, int & nLevel);
    long ProcessReverse(void * pCaller, ISaDoc * pDoc, int & nProgress, int & nLevel);
};

#endif
