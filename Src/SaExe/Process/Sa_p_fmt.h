/////////////////////////////////////////////////////////////////////////////
// sa_p_fmt.h:
// Interface of the CProcessFormants class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_FMT_H
#define _SA_P_FMT_H

#include "sa_p_spu.h"
#include "dsp\mathx.h"
#include "dsp\formants.h"
#include "CSaString.h"

#define MAX_FILTERS 5


//###########################################################################
// CProcessFormants data processing


class CProcessFormants : public CDataProcess
{   
  
  // Construction/destruction/creation
public:
  CProcessFormants();
  virtual ~CProcessFormants();
  void SmoothMedian(ISaDoc *pDoc);
  
  
  // Attributes
private:
  DWORD m_dwProcStart;
  DWORD m_dwProcEnd;
  DWORD m_dwFrameStart;
  DWORD m_dwFrameSize;
  SPECT_PROC_SELECT m_stSpectraProc;
  BOOL  m_bTrack;
  DWORD m_dwFormantFrame;
  DWORD m_dwFormantFrameCount;
  DWORD m_dwValidFrameCount;
  unsigned short m_nFormantFrameSize;
  FormantParm m_FormantParms;
  STATISTIC m_F[MAX_NUM_FORMANTS+1];
protected:
  //  CFilterProcess * m_pFilters[MAX_FILTERS];
  //  WORD m_filterCount;
  
  // Operations
protected:
  FORMANT_FRAME *LoadBuffer(DWORD dwFormantBlockStart);
  BOOL StoreFormantFrame(DWORD dwFormantFrame, FORMANT_FRAME* pFormant, BOOL bWriteThru);
  BOOL WriteData(HPSTR pData, UINT nDataSize, BOOL bAppend);
  BOOL SaveBuffer(UINT nDataSize);
  virtual long Exit(int nError); // exit processing on error
  //  void ProcessFilters(void * pCaller, BOOL * pRes, DWORD start, DWORD end);
  //  void ResetFilters(void);
  
  //  BOOL LoadBuf();
  //  BOOL OpenBuf();
  //  BOOL ReadBuf();
  //  BOOL WriteBuf();
  //  BOOL m_bufChanged;    
  //  DWORD m_dwLastOffset;
public:           
  //  BOOL UnloadBuf();
  
  //  void SetFilteredIntData(void * pThePlotWnd, DWORD dwOffset, FILTER_ID filterID,
  //          BOOL * pbRes, int valToWrite);
  void SetFormantParms(FormantParm* pFormantParms) {m_FormantParms = *pFormantParms;}
  virtual void SetDataInvalid() {SetDataReady(FALSE);}
  FormantParm* GetFormantParms() {return &m_FormantParms;}
  DWORD GetFormantFrameCount() {return m_dwFormantFrameCount;} 
  unsigned short GetFormantFrameSize() {return m_nFormantFrameSize;}
  FORMANT_FRAME *GetFormantFrame(DWORD dwFrame);
  STATISTIC GetFormantStats(int nFormant) {return m_F[nFormant];}
  void ResetTracking() {m_bTrack = FALSE; m_dwFormantFrameCount = 0;SetDataReady(FALSE);}
  long Process(void* pCaller, ISaDoc* pDoc, BOOL bTrack, DWORD dwFrameStart, DWORD dwFrameSize, 
    SPECT_PROC_SELECT SpectraSelected, int nProgress = 0, int nLevel = 1);
  int GetVowelCount(int nGender) const;
  FORMANT_FREQ GetVowelFreq(int nIndex, int nGender, BOOL bMelScale = FALSE) const; // get vowel frequencies
  CSaString GetVowel(int nIndex, int nGender) const; // returns the vowel for the graph
  CSaString FindNearestVowel(FORMANT_FREQ FormantFreq, ISaDoc *pDoc) const;  // returns nearest vowel for specified set of F1, F2, and F3 frequencies
  CSaString FindNearestVowel(ISaDoc *pDoc) const;   // returns nearest vowel from F1, F2, and F3 mean frequencies, as tracked internally by object
};

#endif //_SA_P_FMT_H
