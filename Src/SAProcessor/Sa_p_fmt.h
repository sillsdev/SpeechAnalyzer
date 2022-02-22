/////////////////////////////////////////////////////////////////////////////
// sa_p_fmt.h:
// Interface of the CProcessFormants class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_FMT_H
#define _SA_P_FMT_H

#include "sa_p_spu.h"
#include "param.h"

#define MAX_FILTERS 5

class CProcessFormants : public CProcess {

public:
    CProcessFormants(Context & context);
    virtual ~CProcessFormants();
    void SmoothMedian(Model * pModel);
    SFormantFrame * LoadBuffer(DWORD dwFormantBlockStart);
    BOOL StoreFormantFrame(DWORD dwFormantFrame, SFormantFrame * pFormant, BOOL bWriteThru);
    BOOL WriteData(BPTR pData, UINT nDataSize, BOOL bAppend);
    BOOL SaveBuffer(UINT nDataSize);
    virtual long Exit(int nError); // exit processing on error
    void SetFormantParms(CFormantParm * pFormantParms);
    virtual void SetDataInvalid();
    CFormantParm * GetFormantParms();
    DWORD GetFormantFrameCount();
    unsigned short GetFormantFrameSize();
    SFormantFrame * GetFormantFrame(DWORD dwFrame);
    SStatistic GetFormantStats(int nFormant);
    void ResetTracking();
    long Process(void * pCaller, Model * pModel, BOOL bTrack, DWORD dwFrameStart, DWORD dwFrameSize, SSpectProcSelect SpectraSelected, int nProgress = 0, int nLevel = 1);
    int GetVowelCount(EGender nGender) const;
    SFormantFreq GetVowelFreq(int nIndex, EGender nGender, BOOL bMelScale = FALSE) const; // get vowel frequencies
    wstring GetVowel(int nIndex, EGender nGender) const; // returns the vowel for the graph
    wstring FindNearestVowel(SFormantFreq FormantFreq, Model * pModel) const; // returns nearest vowel for specified set of F1, F2, and F3 frequencies
    wstring FindNearestVowel(Model * pModel) const;  // returns nearest vowel from F1, F2, and F3 mean frequencies, as tracked internally by object

protected:

private:
    DWORD m_dwProcStart;
    DWORD m_dwProcEnd;
    DWORD m_dwFrameStart;
    DWORD m_dwFrameSize;
    SSpectProcSelect m_stSpectraProc;
    BOOL  m_bTrack;
    DWORD m_dwFormantFrame;
    DWORD m_dwFormantFrameCount;
    DWORD m_dwValidFrameCount;
    unsigned short m_nFormantFrameSize;
    CFormantParm m_FormantParms;
    SStatistic m_F[MAX_NUM_FORMANTS+1];
};

#endif
