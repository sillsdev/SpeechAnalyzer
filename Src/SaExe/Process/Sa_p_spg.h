/////////////////////////////////////////////////////////////////////////////
// sa_p_spg.h:
// Interface of the CProcessSpectrogram class
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   10.5Test11.1A
//      RLJ Add SetSpectroAB parameter to GetIndex call so we that we know  
//             which set (A or B) of spectrogram parameters to save.
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_SPG_H

#include "dsp\spectgrm.h"
#include "Appdefs.h"

#define _SA_P_SPG_H

//###########################################################################
// CProcessSpectrogram data processing

class SpectroParm;
class CProcessSpectroFormants;

class CProcessSpectrogram : public CAreaDataProcess
{   
// Construction/destruction/creation
public:
    CProcessSpectrogram(const SpectroParm &cParm, ISaDoc *pDoc, BOOL bRealTime = TRUE);
    virtual ~CProcessSpectrogram();

// Attributes
private:
  int m_nWindowWidth;
  int m_nWindowHeight;
  SpectroParm   m_SpectroParm;
  CProcessSpectroFormants *m_pSpectroFormants;
  const BOOL m_bRealTime;
  ISaDoc* m_pDoc;
  
  // Operations
private:
  int SpectraBandwidth();
  int NyquistSpectraInterval(double dSourceSamplingRate);
  virtual ISaDoc* GetDocument() const {return m_pDoc;}
protected:
  virtual long Exit(int nError); // exit processing on error
public:
  long Process(void* pCaller, ISaDoc* pDoc, CSaView* pView, int nWidth, int nHeight, int nProgress = 0, int nLevel = 1);
  void* GetSpectroSlice(DWORD dwIndex); // return spectrogram slice data

  const SpectroParm& GetSpectroParm() const { return m_SpectroParm;} // return reference to spectrogram parameters structure
  void SetSpectroParm(const SpectroParm& cValue); // set spectrogram parameters

  int GetWindowWidth() const {return m_nWindowWidth;} // return processed window width
  int GetWindowHeight() const {return m_nWindowHeight;} // return processed window height

  CProcessSpectroFormants* GetFormantProcess() { return m_pSpectroFormants;}
};

#endif //_SA_P_SPG_H
