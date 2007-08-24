/////////////////////////////////////////////////////////////////////////////
// waveformGenerator.h:
//
// Author: Steve MacLean
// copyright 2000-2001 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_WAVEFORM_GENERATOR_H

#include "resource.h"

#define _SA_WAVEFORM_GENERATOR_H



//###########################################################################
// CDlgWaveformGenerator dialog

class CDlgWaveformGenerator : public CDialog
{

  // Construction/destruction/creation
public:
  CDlgWaveformGenerator(CWnd* pParent = NULL);   // standard constructor

  // Attributes
private:
  class settings
  {
  public:
    settings();

  class process;

  public:
    // enables for seven signal generators in order
    BOOL    m_bSinusoid1;
    BOOL    m_bSinusoid2;
    BOOL    m_bSinusoid3;
    BOOL    m_bComb;
    BOOL    m_bSquareWave;
    BOOL    m_bTriangle;
    BOOL    m_bSawtooth;
    
    double  m_dAmplitude[7];
    double  m_dFrequency[7];
    double  m_dPhase[7];
    
    // special cases for discontinutities
    int     m_nHandleDiscontinuities;
    int     m_nHarmonics;
    
    PCMWAVEFORMAT pcm;
    double  m_fFileLength;

  public:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void WriteProperties(Object_ostream& obs);
    virtual BOOL bReadProperties(Object_istream& obs);
    BOOL Synthesize(TCHAR* szFileName);
    void SynthesizeSamples(HPSTR pTargetData, DWORD dwDataPos, DWORD dwBufferSize);
  };
  

private:
  settings working;
public:
  settings current;
    
  //{{AFX_DATA(CDlgWaveformGenerator)
	enum { IDD = IDD_WAVEFORM_GENERATOR };
	double	m_fFileLength;
	CString	m_szSamplingRate;
	CString	m_szBits;
	//}}AFX_DATA

  // Operations
public:
	void OnHelpWaveformGenerator();
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  // Generated message map functions
  //{{AFX_MSG(CDlgWaveformGenerator)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnGeneratorBandwith();
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};


extern CDlgWaveformGenerator dlgWaveformGenerator;

#endif //_SA_WAVEFORM_GENERATOR_H
