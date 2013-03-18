/////////////////////////////////////////////////////////////////////////////
// waveformGenerator.h:
//
// Author: Steve MacLean
// copyright 2000-2001 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef DLGWAVEFORM_GENERATOR_H
#define DLGWAVEFORM_GENERATOR_H

#include "resource.h"
#include "process\Process.h"
#include "WaveformGeneratorSettings.h"

//###########################################################################
// CDlgWaveformGenerator dialog

class CDlgWaveformGenerator : public CDialog {

public:
    CDlgWaveformGenerator(CWnd * pParent = NULL);

    double  m_fFileLength;
    CString m_szSamplingRate;
    CString m_szBits;
    CWaveformGeneratorSettings working;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    virtual BOOL OnInitDialog();
    afx_msg void OnGeneratorBandwith();
    afx_msg void OnHelpWaveformGenerator();

    enum { IDD = IDD_WAVEFORM_GENERATOR };

    DECLARE_MESSAGE_MAP()

};

#endif
