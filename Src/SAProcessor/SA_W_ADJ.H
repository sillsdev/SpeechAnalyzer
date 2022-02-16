/////////////////////////////////////////////////////////////////////////////
// sa_w_adj.h:
// Interface of the CProcessAdjust
//
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_ADJ_H
#define _SA_W_ADJ_H

#include "Butterworth.h"

//###########################################################################
// CProcessAdjust data processing

class CProcessAdjust : public CProcessButterworth {
public:
    CProcessAdjust(Context * pContext);
    virtual ~CProcessAdjust();

    BOOL GetInvert() const;
    void SetInvert(BOOL bNew);
    BOOL GetZero() const;
    void SetZero(BOOL bNew);
    BOOL GetNormalize() const;
    void SetNormalize(BOOL bNew);
    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
    virtual void * GetProcessedDataBlock(DWORD dwOffset, size_t sObjectSize, BOOL bReverse=FALSE);  // return processed data pointer to object staring at dwOffset
    virtual HPSTR GetProcessedWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE);                   // return pointer to block of processed wave source
    virtual DWORD GetProcessBufferIndex(size_t nSize = 1);
    bool NeedsAdjustment() const;

private:
    BOOL m_bInvert;
    BOOL m_bZero;
    BOOL m_bNormalize;
};

#endif
