#ifndef HISTGRAM_H
#define HISTGRAM_H

#include "dspTypes.h"
#include "Error.h"
#include "Signal.h"

enum EHISTOGRAM_TYPE {COUNTS = 0, PDF = 1, CDF = 2};

struct SHistogramParms {
    uint16  nBins;          // Number of bins in histogram
    short * pBinDivs;       // Pointer to an array containing the bin divisions
};


class CHistogram {
public:
    CHistogram() = delete;
    CHistogram(const CHistogram&) = delete;

    static char * Copyright(void);
    static float Version(void);
    static dspError_t CreateObject(CHistogram ** ppoHistogram, SHistogramParms & stHistParms, SProcParms & stProcParms);
    dspError_t GenerateHistogram(void);
    dspError_t GetBin(int32 & nBinValue, uint16 nBinNum, double fCoeff, uint16 wGraphForm);
    dspError_t GetBinByData(int32 & nBinValue, int16 nData, double fCoeff, uint16 wGraphForm);
    dspError_t GetBinNum(int32 & nBinNum, int16 nData);
    dspError_t GetHistogram(short * pBins, double fCoeff, uint16 wGraphForm);
    ~CHistogram();
    virtual const SHistogramParms& GetHistogramParms() const;
    virtual short GetMaxValue(void* /*pCaller*/);
    virtual short GetMinValue(void* /*pCaller*/);
    virtual uint32 GetTotalCounts(void* /*pCaller*/);

private:
    CHistogram(SHistogramParms & stHistParms, SProcParms & stProcParms);
    dspError_t Process(uint8 * pBuffer);
    dspError_t Process(short * pBuffer);
    static void FreeHistMem(void);

    SProcParms m_stProcParms;
    SHistogramParms m_stHistParms;
    void * m_pSigBfr;
    int8 m_sbSmpFormat;
    uint32 m_dwBatchOffset;
    short * m_pHistogram;
    short m_nMaxValue;
    short m_nMinValue;
    uint32 m_dwTotalCounts;
};

#endif
