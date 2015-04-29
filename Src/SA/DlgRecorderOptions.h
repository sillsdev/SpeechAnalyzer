#ifndef DLGRECORDEROPTIONS_H
#define DLGRECORDEROPTIONS_H

#include "sa_dlwnd.h"
#include "sa_wave.h"
#include "fnkeys.h"
#include "DlgRecorder.h"

class CSaDoc;

class CDlgRecorderOptions : public CDialog {

public:
    CDlgRecorderOptions(CDlgRecorder * pParent);

    void SetSamplingRate(int nRate);
    unsigned int GetSamplingRate();
    void SetBitDepth(int nBits);
    short GetBitDepth();
    void SetHighpass(BOOL bHighpass);
    BOOL GetHighpass();
    void SetChannels(int nChannels);
    short GetChannels();
    CDlgRecorder & GetRecorder();
    void OnHelpRecorderOptions();

protected:
    // Dialog Data
    enum { IDD = IDD_RECORDEROPTIONS };
    int m_nRate;
    int m_nBits;
    BOOL m_bHighpass;
    int m_nMode;
    BOOL m_bSourceAuto;
    CDlgRecorder * const m_pRecorder;

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    virtual BOOL OnInitDialog();
    afx_msg void OnDefault();
    afx_msg void OnSource();
    afx_msg void OnSourceAuto();

    DECLARE_MESSAGE_MAP()
};

#endif
