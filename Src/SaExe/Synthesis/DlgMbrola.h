#ifndef DLGMBROLA_H
#define DLGMBROLA_H

#include "flexEditGrid.h"

class CDlgMbrola : public CPropertyPage {
    DECLARE_DYNCREATE(CDlgMbrola)

public:
    CDlgMbrola();   // standard constructor
    ~CDlgMbrola();   // standard constructor

    CString m_szMBRolaName;

    double m_fPitchUpdateInterval;
    CFlexEditGrid m_cGrid;
    CFlexEditGrid m_cOutputGrid;
    BOOL m_bGetComplete;
    BOOL m_bConvertComplete;
    enum { IDD = IDD_MBROLA };
    CComboBox m_cSource;
    CComboBox m_cDictionary;
    BOOL  m_bGetDuration;
    BOOL  m_bGetIPA;
    BOOL  m_bGetPitch;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);

private:
    enum {
        rowHeading = 0,
        rowIpa = 1, rowSampa = 1,
        rowDuration = 2,
        rowPitchAvg = 3,
        rowPitchMs = 5
    };
    enum {columnDescription = 0,columnFirst = 1};

protected:
    afx_msg void OnMbrolaGet();
    afx_msg void OnMbrolaSynthesize();
    virtual BOOL OnInitDialog();
    afx_msg void OnLeaveCellMbrolaGrid();
    afx_msg void OnMbrolaPlaySynth();
    afx_msg void OnMbrolaPlay();
    afx_msg void OnMbrolaPlaySource();
    afx_msg void OnMbrolaConvert();
    afx_msg void OnMbrolaDisplay();
    afx_msg void OnDropdownSource();
    DECLARE_EVENTSINK_MAP()
    DECLARE_MESSAGE_MAP()
};

#endif
