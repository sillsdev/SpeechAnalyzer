#if !defined(AFX_DLGVOCALTRACT_H__071AED78_78BD_4BC6_8377_23FED0020735__INCLUDED_)
#define AFX_DLGVOCALTRACT_H__071AED78_78BD_4BC6_8377_23FED0020735__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// The debugger can't handle symbols more than 255 characters long.
// STL often creates symbols longer than that.
// When symbols are longer than 255 characters, the warning is disabled.
#pragma warning(disable:4786)

#include "activex\flexeditgrid.h"
// dlgvocaltract.h : header file
//

class CIpaVTChar
{
public:
  typedef std::vector<double> VocalTract;

  struct CStimulus
  {
    CStimulus() {Pitch=AV=AF=AH=VHX=0;}
    double Pitch;
    double AV;
    double AH;
    double AF;
    double VHX;
  };

  CIpaVTChar() {m_duration=m_dVTGain=m_dFrameEnergy=0;}
  CIpaVTChar(const CString& ipa, double duration, double FEnergy, double VTGain, const CStimulus& stimulus, VocalTract& areas, VocalTract& reflection, VocalTract& pred)
  {
    m_ipa = ipa;
    m_duration = duration;
    m_dFrameEnergy = FEnergy;
    m_stimulus = stimulus;
    m_dVTGain = VTGain;
    m_areas = areas;
    m_reflection = reflection;
    m_pred = pred;
  }

  CString m_ipa;
  double m_duration;
  double m_dFrameEnergy;

  CStimulus m_stimulus;

  double m_dVTGain;

  VocalTract m_areas;
  VocalTract m_reflection;
  VocalTract m_pred;
};

class CIpaVTCharVector : public std::vector<CIpaVTChar>
{
public:
  void Load(CString szPath);
  void Save(CString szPath);
};

class  CIpaVTCharMap : public std::map<CString, CIpaVTChar>
{
public:
  CIpaVTCharMap(CIpaVTCharVector &vector);
};


/////////////////////////////////////////////////////////////////////////////
// CDlgVocalTract frame

class CDlgVocalTract : public CFrameWnd
{
  // Construction
private:
	CDlgVocalTract(CWnd* pParent = NULL);           // protected constructor used by dynamic creation
	virtual ~CDlgVocalTract();           // protected constructor used by dynamic creation
public:
  static void CreateSynthesizer(CWnd* pParent = NULL);
  static void DestroySynthesizer();
  
  enum {kFragment = 0, kSegment, kIpaBlended, kDefaults, kGrids};
  CFlexEditGrid	m_cGrid[kGrids];

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgVocalTract)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
private:
  static CDlgVocalTract* m_pDlgSynthesis;

  enum 
  {
    rowHeading = 0, 
    rowFirst = 1, 
    rowIpa = 1, 
    rowDuration = 2, 
    rowPitch =3, 
    rowFEnergy = 4,
    rowAV = 6,
    rowAH = 7, 
    rowAF = 8, 
    rowVHX = 9,
    rowVTGain =11,
  };
  enum
  {
    rAreaFirst,
    rAreaLast,
    rReflectionFirst,
    rReflectionLast,
    rPredFirst,
    rPredLast,
    rLast,
  };
  int getRow(int rRow)
  {
    int nSections = m_nCurrentOrder;
    switch (rRow)
    {
    case rAreaFirst:
      return 14;
    case rAreaLast:
      return getRow(rAreaFirst) + nSections + 1;
    case rReflectionFirst:
      return getRow(rAreaLast) + 2;
    case rReflectionLast:
      return getRow(rReflectionFirst) + nSections;
    case rPredFirst:
      return getRow(rReflectionLast) + 2;
    case rPredLast:
      return getRow(rPredFirst) + nSections;
    case rLast:
    default:
      return getRow(rPredLast) + 2;
    }
  }
  enum {columnDescription = 0,columnSym = 1,columnFirst = 2};
  protected:

  void LabelGrid(int nGrid);
  void NumberGrid(int nGrid);
  void PopulateParameterGrid(CFlexEditGrid &cGrid, const CIpaVTCharVector &cChars);
  void PopulateParameterGrid(CFlexEditGrid &cGrid, const CIpaVTChar &cChar, int nColumn);
  void PopulateParameterGrid(int nGrid, const CIpaVTCharVector &cChars)
  {
    PopulateParameterGrid(m_cGrid[nGrid], cChars);
  }
  void PopulateParameterGrid(int nGrid, const CIpaVTChar &cChar, int nColumn)
  {
    PopulateParameterGrid(m_cGrid[nGrid], cChar, nColumn);
  }
  void ParseParameterGrid(CFlexEditGrid &cGrid, int column, CIpaVTChar &columnChar);
  void ParseParameterGrid(int nGrid, CIpaVTCharVector &cChar);
  void ParseParameterGrid(CFlexEditGrid &cGrid, CIpaVTCharVector &cChars);
  void SilentColumn(CFlexEditGrid &cGrid, int column, CSaDoc *pDoc, DWORD dwDuration, WORD wSmpSize);
  void ShowGrid(int nView);

  void ConvertCStringToCharVector(CString const &szString, CIpaVTCharVector &cChars);
  void OnUpdateSourceName();
  CString GetDefaultsPath(int nOrder);
  int m_nSelectedView;
  static int m_nSelectedMethod;
  CString m_szSynthesizedFilename;
  CString m_szShowFilename;
  CString m_szSourceFilename;
  CString m_szGrid[kGrids];
  CIpaVTCharVector m_cDefaults;
  double m_dTimeScale;

  BOOL	m_bPitch;
  BOOL	m_bDuration;
  BOOL	m_bIpa;

  BOOL m_bArtificialStimulus;
  BOOL m_bWindowLPC;
  BOOL m_bPreEmphasize;
  BOOL m_bClosedPhase;
  BOOL m_bMirror;
  enum { RESIDUAL_ERROR = 0, LOSSLESS_TUBE = 1, LATTICE = 2, DIRECT = 3, VTGAIN = 4, SQ_ERROR = 5 };
  enum { DB0 = 0, DB12 = 12, DB6 = 6 };
  int m_nStructure;
  int m_nTilt;
  int m_nMethod;
  int m_nRequestedOrder;
  int m_nCurrentOrder;
  DWORD m_dwSampleRate;
  static BOOL m_bMinimize;
  static CSaDoc* m_pShowDoc;

  std::vector<double> residual;


  PCMWAVEFORMAT pcmWaveFormat();
  BOOL SynthesizeWave(const TCHAR* pszPathName, CIpaVTCharVector &cChars);
  BOOL SynthesizeDataChunk(HMMIO hmmioFile, PCMWAVEFORMAT pcm, CIpaVTCharVector &cChars);
  void OnSynthesize();

  void OnGetSegments(CFlexEditGrid &cGrid);
  void OnGetFragments(CFlexEditGrid &cGrid);
  void OnApplyIpaDefaults(CFlexEditGrid &cGrid);
  void OnBlendSegments(int nSrc, CFlexEditGrid &cGrid);
  void LabelDocument(CSaDoc* pDoc);



	// Generated message map functions
	//{{AFX_MSG(CDlgVocalTract)
  afx_msg void OnGetAll();
  afx_msg void OnClose();
	afx_msg void OnUpdateClose(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnEditClear();
	afx_msg void OnUpdateEditClear(CCmdUI* pCmdUI);
  afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
  afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
  afx_msg void OnEditPaste();
  afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
  afx_msg void OnFileOpen();
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
  afx_msg void OnFileSaveAs();
  afx_msg void OnPlayBoth();
  afx_msg void OnPlaySynth();
  afx_msg void OnPlaySource();
  afx_msg void OnSynthDisplay();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
  afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnArtificial();
	afx_msg void OnUpdateArtificial(CCmdUI* pCmdUI);
	afx_msg void OnResidual();
	afx_msg void OnUpdateResidual(CCmdUI* pCmdUI);
	afx_msg void OnWindow();
	afx_msg void OnUpdateWindow(CCmdUI* pCmdUI);
	afx_msg void OnLosslessTube();
	afx_msg void OnUpdateLosslessTube(CCmdUI* pCmdUI);
	afx_msg void OnLattice();
	afx_msg void OnUpdateLattice(CCmdUI* pCmdUI);
	afx_msg void OnDirect();
	afx_msg void OnUpdateDirect(CCmdUI* pCmdUI);
	afx_msg void OnCovariance();
	afx_msg void OnUpdateCovariance(CCmdUI* pCmdUI);
	afx_msg void OnAutocor();
	afx_msg void OnUpdateAutocor(CCmdUI* pCmdUI);
	afx_msg void OnOrder();
	afx_msg void OnVTGain();
	afx_msg void OnUpdateVTGain(CCmdUI* pCmdUI);
	afx_msg void On12dB();
	afx_msg void OnUpdate12dB(CCmdUI* pCmdUI);
	afx_msg void On6dB();
	afx_msg void OnUpdate6dB(CCmdUI* pCmdUI);
	afx_msg void On0dB();
	afx_msg void OnUpdate0dB(CCmdUI* pCmdUI);
	afx_msg void OnMirror();
	afx_msg void OnUpdateMirror(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSqError(CCmdUI* pCmdUI);
	afx_msg void OnSqError();
	afx_msg void OnClosedPhase();
	afx_msg void OnUpdateClosedPhase(CCmdUI* pCmdUI);
	afx_msg void OnAdjustCells();
  afx_msg void OnFragments();
  afx_msg void OnUpdateFragments(CCmdUI* pCmdUI);
  afx_msg void OnIpa();
  afx_msg void OnUpdateIpa(CCmdUI* pCmdUI);
  afx_msg void OnIpaDefaults();
  afx_msg void OnUpdateIpaDefaults(CCmdUI* pCmdUI);
	afx_msg void OnIpaBlend();
	afx_msg void OnUpdateIpaBlend(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnSynthHide();
	afx_msg void OnUpdateSynthHide(CCmdUI* pCmdUI);
	afx_msg void OnSynthShow();
	afx_msg void OnPreemphasize();
	afx_msg void OnUpdatePreemphasize(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CDlgVTOrder dialog

class CDlgVTOrder : public CDialog
{
// Construction
public:
	CDlgVTOrder(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgVTOrder)
	enum { IDD = IDD_ORDER };
	UINT	m_nOrder;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgVTOrder)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgVTOrder)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGVOCALTRACT_H__071AED78_78BD_4BC6_8377_23FED0020735__INCLUDED_)
