/////////////////////////////////////////////////////////////////////////////
// sa_p_fra.h:
// Interface of the CProcessFragments class.
// Author: Alec Epting
// copyright 1998 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_FRA_H
#define _SA_P_FRA_H

#include "appdefs.h"
#include "Process\Process.h"

//###########################################################################
// CProcessFragments data processing

class CProcessFragments : public CProcess {
// Construction/destruction/creation
public:
    CProcessFragments();
    virtual ~CProcessFragments();

// Attributes
private:
    CFragment * m_pFragmenter;
    ULONG m_dwFragmentCount;
    ULONG m_dwWaveIndex;
    ULONG m_dwPitchIndex;
    ULONG m_dwFragmentIndex;

// Operations
public:
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    ULONG GetBufferLength();
    SFragParms * GetFragmentBlock(ULONG dwFragmentIndex);
    const SFragParms & GetFragmentParms(ULONG dwFragmentIndex);
    ULONG GetFragmentCount();
    ULONG GetFragmentIndex(ULONG dwWaveIndex);
};


#endif //_SA_P_FRA_H
