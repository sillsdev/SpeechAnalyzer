#ifndef DLGKLATTALL_H
#define DLGKLATTALL_H

// The debugger can't handle symbols more than 255 characters long.
// STL often creates symbols longer than that.
// When symbols are longer than 255 characters, the warning is disabled.
#pragma warning(disable:4786)

#include "flexeditgrid.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgKlattAll dialog

struct SIpaChar
{
    SIpaChar()
    {
        ;
    }

    SIpaChar(const CString & r_ipa, const TEMPORAL & r_parameters, double r_duration = -1)
    {
        ipa = r_ipa;
        duration = r_duration;
        parameters = r_parameters;
    }

    CString ipa;
    double duration;
    TEMPORAL parameters;
};

class CIpaCharVector : public std::vector<SIpaChar>
{
public:
    void Load(CString szPath);
    void Save(CString szPath);
};

class  CIpaCharMap : public std::map<CString, TEMPORAL>
{
public:
    CIpaCharMap(CIpaCharVector & vector);
};


typedef SPKRDEF CKlattConstants;

class CDlgKlattAll : public CFrameWnd
{
    // Construction
private:
    CDlgKlattAll(CWnd * pParent = NULL, int nSelectedView = -1);  // standard constructor
    ~CDlgKlattAll();
public:
    static void CreateSynthesizer(CWnd * pParent = NULL, int nSelectedView = -1);
    static void DestroySynthesizer();

    enum {kFragment = 0, kSegment, kIpaBlended, kDefaults, kConstants, kGrids};
    CFlexEditGrid m_cGrid[kGrids];

public:
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void * pExtra, AFX_CMDHANDLERINFO * pHandlerInfo);
protected:
    virtual void PostNcDestroy();

private:
    static CDlgKlattAll * m_pDlgSynthesisKlatt;

    enum {rowHeading = 0, rowFirst = 1, rowIpa = 1, rowDuration = 2, rowPitchIn =3, rowParameters = 5};
    enum {columnDescription = 0, columnDef = 1, columnSym = 2, columnFirst = 3};
    enum
    {
        rowF0 = rowParameters,
        rowAV,
        rowOQ,
        rowSQ,
        rowTL,
        rowFL,
        rowDI,
        rowAH,
        rowAF,
        rowF1,
        rowB1,
        rowDF1,
        rowDB1,
        rowF2,
        rowB2,
        rowF3,
        rowB3,
        rowF4,
        rowB4,
        rowF5,
        rowB5,
        rowF6,
        rowB6,
        rowFNP,
        rowBNP,
        rowFNZ,
        rowBNZ,
        rowFTP,
        rowBTP,
        rowFTZ,
        rowBTZ,
        rowA2F,
        rowA3F,
        rowA4F,
        rowA5F,
        rowA6F,
        rowAB,
        rowB2F,
        rowB3F,
        rowB4F,
        rowB5F,
        rowB6F,
        rowANV,
        rowA1V,
        rowA2V,
        rowA3V,
        rowA4V,
        rowATV,
        rowA5V,
        rowA6V,
        rowA7V,
        rowA8V
    };

    void LabelGrid(int nGrid);
    void NumberGrid(int nGrid);
    void PopulateParameterGrid(CFlexEditGrid & cGrid, const CIpaCharVector & cChars, BOOL bDuration = FALSE);
    void PopulateParameterGrid(CFlexEditGrid & cGrid, const SIpaChar & cChar, int nColumn, BOOL bDuration = TRUE);
    void PopulateParameterGrid(CFlexEditGrid & cGrid, const TEMPORAL * pParameters, int nColumn);
    void PopulateParameterGrid(int nGrid, const CIpaCharVector & cChars, BOOL bDuration = FALSE)
    {
        PopulateParameterGrid(m_cGrid[nGrid], cChars, bDuration);
    }
    void PopulateParameterGrid(int nGrid, const SIpaChar & cChar, int nColumn, BOOL bDuration = TRUE)
    {
        PopulateParameterGrid(m_cGrid[nGrid], cChar, nColumn, bDuration);
    }
    void PopulateParameterGrid(int nGrid, const TEMPORAL * pParameters, int nColumn)
    {
        PopulateParameterGrid(m_cGrid[nGrid], pParameters, nColumn);
    }
    void ParseConstantsGrid(int nGrid, CKlattConstants & cConstants);
    void ParseParameterGrid(int nGrid, CIpaCharVector & cChar);
    void ShowGrid(int nView);

    void ConvertCStringToCharVector(CString const & szString, CIpaCharVector & cChars);
    void OnUpdateSourceName();
    CString GetDefaultsPath();
    int m_nSelectedView;
    static int m_nSelectedMethod;
    CString m_szSynthesizedFilename;
    CString m_szShowFilename;
    CString m_szSourceFilename;
    CString m_szGrid[kGrids];
    CKlattConstants m_cConstants;
    CIpaCharVector m_cDefaults;
    double m_dTimeScale;

    BOOL  m_bPitch;
    BOOL  m_bDuration;
    BOOL  m_bIpa;

    BOOL m_bGetFragments;
    static BOOL m_bMinimize;
    static CSaDoc * m_pShowDoc;


    PCMWAVEFORMAT pcmWaveFormat();
    BOOL SynthesizeWave(LPCTSTR pszPathName, CIpaCharVector & cChars);
    void OnSynthesize();

    void OnKlattGetSegments(CFlexEditGrid & cGrid);
    void OnKlattGetFragments(CFlexEditGrid & cGrid);
    void OnKlattGetFrames(CFlexEditGrid & cGrid, int iFrameWidthInMs, int iFrameIntervalInMs, int iFrameWidthInFrags, int iFrameIntervalInFrags);
    void AdjustParallelAVs(double pFormAV[7], double pFormFreq[7]);
    void OnKlattApplyIpaDefaults(CFlexEditGrid & cGrid);
    void OnKlattBlendSegments(int nSrc, CFlexEditGrid & cGrid);
    void LabelDocument(CSaDoc * pDoc);
    void SilentColumn(CFlexEditGrid & cGrid, int column, CSaDoc * pDoc, DWORD dwDuration, WORD wSmpSize);
    BOOL GetFormants(CFlexEditGrid & cGrid, int column, CSaView * pView,
                     CProcessSpectrum * pSpectrum, CProcessGrappl * pAutoPitch, CProcessZCross * pZCross,
                     DWORD dwStart, DWORD dwLength, double * pFormFreq);
    BOOL GetFrame(CFlexEditGrid & cGrid, int & column, CSaView * pView,
                  CProcessSpectrum * pSpectrum, CProcessGrappl * pAutoPitch, CProcessZCross * pZCross,
                  DWORD dwStart, DWORD dwLength, DWORD dwInterval, double * pFormFreq);


protected:

    // Generated message map functions
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnEditCopy();
    afx_msg void OnFileOpen();
    afx_msg void OnFileSaveAs();
    afx_msg void OnKlattConst();
    afx_msg void OnUpdateKlattConst(CCmdUI * pCmdUI);
    afx_msg void OnKlattDisplay();
    afx_msg void OnKlattFragments();
    afx_msg void OnUpdateKlattFragments(CCmdUI * pCmdUI);
    afx_msg void OnKlattIpa();
    afx_msg void OnUpdateKlattIpa(CCmdUI * pCmdUI);
    afx_msg void OnKlattIpaDefaults();
    afx_msg void OnUpdateKlattIpaDefaults(CCmdUI * pCmdUI);
    afx_msg void OnKlattGetAll();
    afx_msg void OnPlayBoth();
    afx_msg void OnPlaySynth();
    afx_msg void OnPlaySource();
    afx_msg void OnEditClear();
    afx_msg void OnEditCut();
    afx_msg void OnEditPaste();
    afx_msg void OnUpdateEditPaste(CCmdUI * pCmdUI);
    afx_msg void OnClose();
    afx_msg void OnSmoothe();
    afx_msg void OnUpdateEditCopy(CCmdUI * pCmdUI);
    afx_msg void OnUpdateEditCut(CCmdUI * pCmdUI);
    afx_msg void OnUpdateEditClear(CCmdUI * pCmdUI);
    afx_msg void OnUpdateEditSmoothe(CCmdUI * pCmdUI)
    {
        pCmdUI->Enable();
    }
    afx_msg void OnUpdateClose(CCmdUI * pCmdUI);
    afx_msg void OnUpdateFileOpen(CCmdUI * pCmdUI);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnSetFocus(CWnd * pOldWnd);
    afx_msg void OnKlattElongate();
    afx_msg void OnUpdateKlattElongate(CCmdUI * pCmdUI);
    afx_msg void OnKlattHelp();
    afx_msg void OnKlattIpaBlend();
    afx_msg void OnUpdateKlattIpaBlend(CCmdUI * pCmdUI);
    afx_msg void OnAdjustCells();
    afx_msg void OnSynthHide();
    afx_msg void OnUpdateSynthHide(CCmdUI * pCmdUI);
    afx_msg void OnSynthShow();
    afx_msg void OnIntervalNFrag(UINT nID);
    afx_msg void OnIntervalNMs(UINT nID);
    afx_msg void OnWindowNFrag(UINT nID);
    afx_msg void OnWindowNMs(UINT nID);
    afx_msg void OnUpdateIntervalNMs(CCmdUI * pCmdUI);
    afx_msg void OnUpdateIntervalNFrag(CCmdUI * pCmdUI);
    afx_msg void OnUpdateWindowNFrag(CCmdUI * pCmdUI);
    afx_msg void OnUpdateWindowNMs(CCmdUI * pCmdUI);
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDlgSynthesisAdjustCells dialog

class CDlgSynthesisAdjustCells : public CDialog
{
public:
    CDlgSynthesisAdjustCells(CWnd * pParent = NULL, double dScale = 1.0, double dOffset = 0);  // standard constructor

    enum { IDD = IDD_SYNTHESIS_ADJUST_CELLS };
    double  m_dScale;
    double  m_dOffset;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support

protected:
    DECLARE_MESSAGE_MAP()
};

#endif
