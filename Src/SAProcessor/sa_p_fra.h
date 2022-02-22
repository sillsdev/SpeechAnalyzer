/////////////////////////////////////////////////////////////////////////////
// sa_p_fra.h:
// Interface of the CProcessFragments class.
// Author: Alec Epting
// copyright 1998 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_FRA_H
#define _SA_P_FRA_H

#include "sa_process.h"

//###########################################################################
// CProcessFragments data processing

class CProcessFragments : public CProcess {
public:
    CProcessFragments(Context & context);
    virtual ~CProcessFragments();

    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
    ULONG GetBufferLength();
    SFragParms * GetFragmentBlock(ULONG dwFragmentIndex);
    const SFragParms & GetFragmentParms(ULONG dwFragmentIndex);
    ULONG GetFragmentCount();
    ULONG GetFragmentIndex(ULONG dwWaveIndex);

private:
    long SubProcess(bool background, void* pCaller, Model* pModel, int nProgress, int nLevel);

    CFragment* m_pFragmenter;
    ULONG m_dwFragmentCount;
    ULONG m_dwWaveIndex;
    ULONG m_dwPitchIndex;
    ULONG m_dwFragmentIndex;

};


#endif //_SA_P_FRA_H
