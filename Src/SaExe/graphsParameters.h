/////////////////////////////////////////////////////////////////////////////
// graphsParameters.h:
// Interface of the CDlgParametersRawdataPage (property page)
//                  CDlgParametersPitchPage (property page)
//                  CDlgParametersSpectroPage (property page)
//                  CDlgParametersSpectrumPage (property page)
//                  CDlgParametersFormantsPage (property page)
//                  CDlgParametersSDPPage (property page)
//                  CDlgParametersMusicPage (property page)
//                  CDlgGraphsParameters (property sheet)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg.h
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHS_PARAMETERS_H
#define _GRAPHS_PARAMETERS_H

#include "sa_dlwnd.h"
#include "param.h"
#include "resource.h"
#include "SpectroParm.h"

//###########################################################################
// CDlgParametersRawdataPage property page

class CDlgParametersRawdataPage : public CPropertyPage
{

    // Construction/destruction/creation
public:
    CDlgParametersRawdataPage(); // standard constructor

    // Attributes
public:
    // Dialog Data
    //{{AFX_DATA(CDlgParametersRawdataPage)
    enum { IDD = IDD_PARAMETERSRAWDATAPAGE };
    int     m_nProcess;
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
public:

    // Generated message map functions
protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnProcess();
    DECLARE_MESSAGE_MAP()
};

class CDlgParametersPitchPage : public CPropertyPage
{

public:
    CDlgParametersPitchPage(); // standard constructor

private:
    CSpinControl    m_SpinMinFreq;
    CSpinControl    m_SpinMaxFreq;
    CSpinControl    m_SpinVoicing;
    CSpinControl    m_SpinChange;
    CSpinControl    m_SpinGroup;
    CSpinControl    m_SpinInterpol;
    CSpinControl    m_SpinUpperBound;
    CSpinControl    m_SpinLowerBound;
    CSpinControl    m_SpinMedianFilterSize;
    int             m_nFreqLimit;
    BOOL m_bModified; // TRUE if page member data modified

    // RLJ, 09/27/2000: Bug GPI-01
    int     m_nManualLowerBound;
    int     m_nManualUpperBound;

    enum { IDD = IDD_PARAMETERSPITCHPAGE };
    int     m_nChange;
    int     m_nGroup;
    int     m_nInterpol;
    double  m_fVoicing;
    int     m_nMaxFreq;
    int     m_nMinFreq;
    int     m_nLowerBound;
    int     m_nUpperBound;
    int     m_nPitchRange;
    int     m_nPitchScaling;
    int     m_nCepMedianFilterSize;
    BOOL    m_bUseCepMedianFilter;

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void EnableCepstralSettings(BOOL enable);
    void EnableCepMedianSize(BOOL enable);

public:
    void Apply();

protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    afx_msg void OnChangeScroll();
    afx_msg void OnKillfocusChange();
    afx_msg void OnGroupScroll();
    afx_msg void OnKillfocusGroup();
    afx_msg void OnInterpolScroll();
    afx_msg void OnKillfocusInterpol();
    afx_msg void OnVoicingScroll();
    afx_msg void OnKillfocusVoicing();
    afx_msg void OnMaxFreqScroll();
    afx_msg void OnKillfocusMaxfreq();
    afx_msg void OnMinFreqScroll();
    afx_msg void OnKillfocusMinfreq();
    afx_msg void OnUpperBoundScroll();
    afx_msg void OnKillfocusUpperBound();
    afx_msg void OnLowerBoundScroll();
    afx_msg void OnKillfocusLowerBound();
    afx_msg void OnRange();
    afx_msg void OnScaling();
    afx_msg void OnMedianFilterSizeSpin();
    afx_msg void OnUsemedianfilter();
    afx_msg void OnKillfocusMedianFilter();
    afx_msg void OnChange();
    afx_msg void OnPitchManualDefaultFile();
    afx_msg void OnPitchManualDefaultApp();
    afx_msg void OnPitchManualDefault(BOOL bAppDefaults);
    DECLARE_MESSAGE_MAP()
};

class CDlgParametersSpectroPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CDlgParametersSpectroPage)

public:
    // graphId is the IDD_SPECTROGRAMA or B.
    CDlgParametersSpectroPage(UINT dialogID, UINT graphId, const CSpectroParm * defaults);

    void Apply();

protected:

    // Which graph to work on
    UINT m_GraphId;

    CSpinControl    m_SpinFrequency;
    int             m_nFreqLimit;
    int     m_nFrequency;
    int     m_nMaxThreshold;
    int     m_nMinThreshold;
    BOOL m_bModified; // TRUE if page member data modified
    enum { IDD = IDD_PARAMETERSSPECTROPAGE };
    CStatic m_cMinStatic;
    CStatic m_cMaxStatic;
    CStatic m_cFreqStatic;
    CSliderCtrl m_cSliderMin;
    CSliderCtrl m_cSliderMax;
    CSliderCtrl m_cSliderFreq;
    int     m_nResolution;
    int     m_nColor;
    BOOL    m_bF1;
    BOOL    m_bF2;
    BOOL    m_bF3;
    BOOL    m_bF4;
    BOOL    m_bSmoothFormantTracks;
    int     m_nOverlay;
    BOOL    m_bSmoothSpectra;
    BOOL    m_bFormants;
    BOOL    m_bFormantColor;
    BOOL    m_bShowPitch;

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

public:
    int SetMaxThreshold(int nData);
    int SetMinThreshold(int nData);
    int SetFrequency(int nData);

protected:
    afx_msg void OnModifiedSmoothFormantTracks();
    afx_msg void OnModifiedColor();
    afx_msg void OnModifiedResolution();
    afx_msg void OnModifiedOverlay();
    virtual BOOL OnInitDialog();
    afx_msg void OnChange();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar);
    afx_msg void OnModifiedFormants();
    afx_msg void OnMoreInfo();
    afx_msg void OnModifiedFormants(BOOL bMessage);

    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgParametersSpectrumPage property page

class CDlgParametersSpectrumPage : public CPropertyPage
{

    // Construction/destruction/creation
public:
    CDlgParametersSpectrumPage(); // standard constructor

    // Attributes
private:
    CSpinControl    m_SpinSmoothLevel;
    CSpinControl    m_SpinSharpening;
    CSpinControl    m_SpinPwrUpper;
    CSpinControl    m_SpinPwrLower;
    CSpinControl    m_SpinFreqUpper;
    CSpinControl    m_SpinFreqLower;
    int             m_nFreqLimit;
    int           m_nFreqScaleRange; // Changed name meaning changed
    BOOL m_bModified; // TRUE if page member data modified
    // Dialog Data
    //{{AFX_DATA(CDlgParametersSpectrumPage)
    enum { IDD = IDD_PARAMETERSSPECTRUMPAGE };
    int     m_nPwrLower;
    int     m_nPeak;
    int     m_nSmooth;
    int     m_nPwrUpper;
    int     m_nSpectrumScaling;
    int     m_nFreqLower;
    int     m_nFreqUpper;
    BOOL    m_bShowLpcSpectrum;
    BOOL    m_bShowCepSpectrum;
    BOOL    m_bShowFormantFreq;
    BOOL    m_bShowFormantBandwidth;
    BOOL    m_bShowFormantPower;
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
public:
    void Apply();

    // Generated message map functions
protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnPwrUpperScroll();
    afx_msg void OnKillfocusPwrUpper();
    afx_msg void OnPwrLowerScroll();
    afx_msg void OnKillfocusPwrLower();
    afx_msg void OnFreqUpperScroll();
    afx_msg void OnKillfocusFreqUpper();
    afx_msg void OnFreqLowerScroll();
    afx_msg void OnKillfocusFreqLower();
    afx_msg void OnModifiedFreqRange();
    afx_msg void OnSmoothScroll();
    afx_msg void OnKillfocusSmooth();
    afx_msg void OnPeakScroll();
    afx_msg void OnKillfocusPeak();
    afx_msg void OnScaling();
    afx_msg void OnChange();
    afx_msg void OnModifiedShowSmoothedPlots();
    afx_msg void OnModifiedShowFormantFreq();
    afx_msg void OnModifiedShowFormantBandwidth();
    afx_msg void OnModifiedShowFormantPower();
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgParametersFormantsPage property page

class CDlgParametersFormantsPage : public CPropertyPage
{

    // Construction/destruction/creation
public:
    CDlgParametersFormantsPage();   // standard constructor
    BOOL        m_bAccess;

    // Attributes
private:
    BOOL m_bModified; // TRUE if page member data modified
    // Dialog Data
    //{{AFX_DATA(CDlgParametersFormantsPage)
    enum { IDD = IDD_PARAMETERSFORMANTSPAGE };
    CButton m_cFormantVowelEdit;
    CComboBox   m_cVowelSet;
    BOOL    m_bCepstralFormants;
    BOOL    m_bTrackFormants;
    BOOL    m_bSmoothFormants;
    BOOL    m_bMelScale;
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
public:
    void Apply();
    static void PopulateVowelSetCombo(CComboBox & cBox);

    // Generated message map functions
protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnModifiedSource();
    afx_msg void OnModifiedTrack();
    afx_msg void OnModifiedSmooth();
    afx_msg void OnModifiedScale();
    afx_msg void OnEditChangeFormantVowels();
    afx_msg void OnFormantVowelsEdit();
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgParametersSDPPage property page

class CDlgParametersSDPPage : public CPropertyPage
{

    // Construction/destruction/creation
public:
    CDlgParametersSDPPage();    // standard constructor

    // Attributes
private:
    CSpinControl    m_SpinPanes;
    CSpinControl    m_SpinSteps;
    CSpinControl    m_SpinUpper;
    BOOL m_bModified; // TRUE if page member data modified
    // Dialog Data
    //{{AFX_DATA(CDlgParametersSDPPage)
    enum { IDD = IDD_PARAMETERSSDPPAGE };
    UINT    m_nPanes;
    UINT    m_nSteps;
    UINT    m_nUpperBound;
    BOOL    m_bAverage;
    int     m_nStepMode;
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
public:
    void Apply();

    // Generated message map functions
protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnPanesScroll();
    afx_msg void OnKillfocusPanes();
    afx_msg void OnStepsScroll();
    afx_msg void OnKillfocusSteps();
    afx_msg void OnUpperScroll();
    afx_msg void OnKillfocusUpper();
    afx_msg void OnStepMode();
    afx_msg void OnModified();
    afx_msg void OnChange();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersMusicPage dialog

class CDlgParametersMusicPage : public CPropertyPage
{
    // Construction
public:
    CDlgParametersMusicPage();  // standard constructor

    void Apply();

private:
    CSpinControl    m_SpinUpperBound;
    CSpinControl    m_SpinLowerBound;
    CSpinControl    m_SpinCalcUpperBound;
    CSpinControl    m_SpinCalcLowerBound;
    BOOL m_bModified; // TRUE if page member data modified


    // Dialog Data
    int     m_nManualLowerBound;
    int     m_nManualUpperBound;
    int     m_nManualCalcLowerBound;
    int     m_nManualCalcUpperBound;

    //{{AFX_DATA(CDlgParametersMusicPage)
    enum { IDD = IDD_PARAMETERSMUSICPAGE };
    BOOL  m_nDynamic;
    BOOL  m_nWeighted;
    int     m_nLowerBound;
    int     m_nUpperBound;
    int     m_nRange;
    int     m_nCalcLowerBound;
    int     m_nCalcUpperBound;
    int     m_nCalcRange;
    //}}AFX_DATA

    // Implementation
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnClickWeighted();
    afx_msg void OnClickDynamic();
    afx_msg void OnUpperBoundScroll();
    afx_msg void OnKillfocusUpperBound();
    afx_msg void OnLowerBoundScroll();
    afx_msg void OnKillfocusLowerBound();
    afx_msg void OnRange();
    afx_msg void OnCalcUpperBoundScroll();
    afx_msg void OnKillfocusCalcUpperBound();
    afx_msg void OnCalcLowerBoundScroll();
    afx_msg void OnKillfocusCalcLowerBound();
    afx_msg void OnCalcRange();
    afx_msg void OnChange();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersIntensityPage dialog

class CDlgParametersIntensityPage : public CPropertyPage
{
    // Construction
public:
    CDlgParametersIntensityPage();
    ~CDlgParametersIntensityPage();

    void Apply();

private:
    BOOL m_bModified; // TRUE if page member data modified
    // Dialog Data
    //{{AFX_DATA(CDlgParametersIntensityPage)
    enum { IDD = IDD_PARAMETERS_INTENSITY_PAGE };
    CComboBox   m_cDisplayScaleMode;
    int     m_nScaleMode;
    //}}AFX_DATA


    // Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CDlgParametersIntensityPage)
protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation
protected:
    // Generated message map functions
    afx_msg void OnChange();
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersResearchPage dialog

class CDlgParametersResearchPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CDlgParametersResearchPage)

    // Construction
public:
    CDlgParametersResearchPage();
    ~CDlgParametersResearchPage();


    void Apply();

private:
    BOOL m_bModified; // TRUE if page member data modified
    // Dialog Data
    //{{AFX_DATA(CDlgParametersResearchPage)
    enum { IDD = IDD_PARAMETERS_RESEARCH };
    CComboBox   m_cWindowReplication;
    CComboBox   m_cWindowType;
    CComboBox   m_cSmooth;
    //}}AFX_DATA

    CResearchSettings m_workingSettings;

    // Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CDlgParametersResearchPage)
protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation
protected:
    // Generated message map functions
    afx_msg void OnModified();
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersFormantTracker dialog

class CDlgParametersFormantTracker : public CPropertyPage
{
    DECLARE_DYNCREATE(CDlgParametersFormantTracker)

    // Construction
public:
    CDlgParametersFormantTracker();
    ~CDlgParametersFormantTracker();

    void Apply();

private:
    BOOL m_bModified; // TRUE if page member data modified
    // Dialog Data
    //{{AFX_DATA(CDlgParametersFormantTracker)
    enum { IDD = IDD_PARAMETERS_FORMANT_TRACKER };
    //}}AFX_DATA

    CFormantTrackerOptions m_workingSettings;

    // Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CDlgParametersFormantTracker)
protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation
protected:
    // Generated message map functions
    afx_msg void OnModified();
    DECLARE_MESSAGE_MAP()

};

//###########################################################################
// CDlgGraphsParameters property sheet

class CDlgGraphsParameters : public CPropertySheet
{
    DECLARE_DYNAMIC(CDlgGraphsParameters)

// Construction/destruction/creation
public:
    CDlgGraphsParameters(LPCTSTR pszCaption, CWnd * pParent);
    ~CDlgGraphsParameters();// SDM 1.5Test10.3

    // Attributes
public:
    CDlgParametersRawdataPage m_dlgRawdataPage;
    CDlgParametersPitchPage m_dlgPitchPage;
    CDlgParametersSpectroPage * m_pDlgSpectrogramPage;
    CDlgParametersSpectroPage * m_pDlgSnapshotPage;
    CDlgParametersSpectrumPage m_dlgSpectrumPage;
    CDlgParametersFormantsPage m_dlgFormantsPage;
    CDlgParametersSDPPage m_dlgSDPPage;
    CDlgParametersMusicPage m_dlgMusicPage;
    CDlgParametersIntensityPage m_dlgIntensityPage;
    CDlgParametersResearchPage * m_pDlgResearchPage;
    CDlgParametersFormantTracker * m_pDlgFTrackerPage;

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void ChangeButtons(); // delete Apply button, move other buttons
    CButton m_cHelp;
public:
    void OnHelpParameters();

    // Generated message map functions
protected:
    virtual BOOL OnInitDialog();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnApply();
    afx_msg void OnOK();
    DECLARE_MESSAGE_MAP()
};

#endif //_GRAPHS_PARAMETERS_H
