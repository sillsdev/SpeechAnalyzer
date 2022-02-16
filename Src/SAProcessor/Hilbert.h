// Hilbert.h: interface for the CProcessButterworth class.
//
//////////////////////////////////////////////////////////////////////
#ifndef HILBERT_H
#define HILBERT_H

#include "IIRFilter.h"

class CProcessHilbert : public CProcessIIRFilter {
public:
    CProcessHilbert(Context * pContext, CProcess * pSourceProcess = NULL, BOOL bWBenchProcess = FALSE);
    void Dump(LPCSTR ofilename);

private:
    static const double Pole1000x96dB[];

    CZTransform AllPass(double pole);
    CZTransform DelayHalf();
};

#endif
