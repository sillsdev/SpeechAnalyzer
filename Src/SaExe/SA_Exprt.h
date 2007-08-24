/////////////////////////////////////////////////////////////////////////////
// sa_exprt.h:
// Interface of the      CExportSFM class.
//                       CExportTabbed class.
//                       CImport class.
//
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CExportBasicDialog

class CExportBasicDialog : public CDialog
{
  // Construction
public:
  CExportBasicDialog(const CSaString& szDocTitle, CWnd* pParent = NULL); // standard constructor

public:
  // Dialog Data
  //{{AFX_DATA(CExportBasicDialog)
  enum { IDD = IDD_EXPORT_SFM };
  BOOL  m_bAllSource;
  BOOL  m_bBandwidth;
  BOOL  m_bBits;
  BOOL  m_bComments;
  BOOL  m_bDialect;
  BOOL  m_bEthnologue;
  BOOL  m_bFamily;
  BOOL  m_bAllFile;
  BOOL  m_bFileSize;
  BOOL  m_bFileName;
  BOOL  m_bFree;
  BOOL  m_bGender;
  BOOL  m_bHighPass;
  BOOL  m_bInterlinear;
  BOOL  m_bLanguage;
  BOOL  m_bLastModified;
  BOOL  m_bLength;
  BOOL  m_bNotebookRef;
  BOOL  m_bNumberSamples;
  BOOL  m_bOriginalDate;
  BOOL  m_bOriginalFormat;
  BOOL  m_bPhones;
  BOOL  m_bSampleRate;
  BOOL  m_bAllParameters;
  BOOL  m_bRegion;
  BOOL  m_bSpeaker;
  BOOL  m_bTranscriber;
  BOOL  m_bWords;
  BOOL  m_bAllAnnotations;
  BOOL  m_bGloss;
  BOOL  m_bOrtho;
  BOOL  m_bPhonemic;
  BOOL  m_bPhonetic;
  BOOL  m_bPOS;
  BOOL  m_bReference;
  BOOL  m_bTone;
  BOOL  m_bCountry;
  BOOL  m_bQuantization;
  BOOL    m_bPhrase;
  //}}AFX_DATA
protected:
  CSaString m_szFileName;
  CSaString m_szDocTitle;

  // Implementation
public:
	void OnHelpExportBasic();
protected:
  void SetEnable(int nItem, BOOL bEnable);
  virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support
  virtual void OnOK() = 0; // this must be implented by the derived types

  // Generated message map functions
  //{{AFX_MSG(CExportBasicDialog)
  afx_msg void OnAllSource();
  afx_msg void OnAllFileInfo();
  afx_msg void OnAllParameters();
  virtual BOOL OnInitDialog();
  afx_msg void OnAllAnnotations();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CExportSFM dialog

class CExportSFM : public CExportBasicDialog
{
  // Construction
public:
  CExportSFM(const CSaString& szDocTitle, CWnd* pParent = NULL) : CExportBasicDialog(szDocTitle, pParent) {} // standard constructor

public:
  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(CExportSFM)
  virtual void OnOK();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CExportXML dialog

class CExportXML : public CExportBasicDialog
{
  // Construction
public:
  CExportXML(const CSaString& szDocTitle, CWnd* pParent = NULL) : CExportBasicDialog(szDocTitle, pParent) {} // standard constructor

public:
  // Implementation
  static const char XML_HEADER1[];
  static const char XML_HEADER2[];
  static const char XML_FOOTER[];
  static void CExportXML::OutputXMLField(CFile* pFile,const TCHAR *szFieldName,const CSaString &szContents);

protected:
  // Generated message map functions
  //{{AFX_MSG(CExportXML)
  virtual void OnOK();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CExportTable dialog

class CExportTable : public CDialog
{
  // Construction
public:
  CExportTable(const CSaString& szDocTitle, CWnd* pParent = NULL); // standard constructor

  // Dialog Data
  //{{AFX_DATA(CExportTable)
  enum { IDD = IDD_EXPORT_TABLE };
  BOOL  m_bAnnotations;
  BOOL  m_bCalculations;
  BOOL  m_bF1;
  BOOL  m_bF2;
  BOOL  m_bF3;
  BOOL  m_bF4;
  BOOL  m_bFormants;
  BOOL  m_bGloss;
  CSaString m_szIntervalTime;
  BOOL  m_bSegmentLength;
  BOOL  m_bMagnitude;
  BOOL  m_bOrtho;
  BOOL  m_bPhonemic;
  BOOL  m_bPhonetic;
  BOOL  m_bPitch;
  BOOL  m_bPOS;
  BOOL  m_bReference;
  BOOL  m_bSegmentStart;
  BOOL  m_bSampleTime;
  BOOL  m_bTone;
  BOOL  m_bZeroCrossings;
  int   m_nSampleRate;
  int   m_nCalculationMethod;
  BOOL  m_bPhonetic2;
  int     m_nRegion;
  BOOL	m_bMelogram;
  //}}AFX_DATA
private:
  CSaString m_szFileName;
  CSaString m_szDocTitle;

  // Implementation
public:
	void OnHelpExportTable();
protected:
  void SetEnable(int nItem, BOOL bEnable);
  void SetVisible(int nItem, BOOL bVisible, BOOL bEnable = TRUE);
  virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support

  // Generated message map functions
  //{{AFX_MSG(CExportTable)
  afx_msg void OnAllAnnotations();
  afx_msg void OnAllFormants();
  afx_msg void OnAllCalculations();
  virtual void OnOK();
  virtual BOOL OnInitDialog();
  afx_msg void OnUpdateIntervalTime();
  afx_msg void OnSample();
  afx_msg void OnPhonetic();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

class CImport
{
public:
  CImport(const CSaString& szFileName, BOOL batch=FALSE) {m_szPath = szFileName;m_bBatch = batch;}; // standard constructor

  CSaString m_szPath;
  BOOL m_bBatch;

  enum { KEEP = 0, AUTO = 1, MANUAL = 2, QUERY = 3 };

  BOOL Import(int nMode = QUERY);

private:
  BOOL ReadTable(Object_istream &ios, int nMode = QUERY);
};

/////////////////////////////////////////////////////////////////////////////
// CImportDlg dialog

class CImportDlg : public CDialog
{
  // Construction
public:
  CImportDlg(CWnd* pParent = NULL); // standard constructor

  // Dialog Data
  //{{AFX_DATA(CImportDlg)
  enum { IDD = IDD_IMPORT_DIALOG };
  int   m_nMode;
  //}}AFX_DATA

  // Implementation
protected:
  virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV support

  // Generated message map functions
  //{{AFX_MSG(CImportDlg)
  virtual void OnOK();
  virtual BOOL OnInitDialog();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDlgAnnotationWizard dialog

class CSaDoc;

class CDlgAnnotationWizard : public CDialog
{
  // Construction
public:
  enum MODE { IMPORT, ALIGN };
  CDlgAnnotationWizard(CWnd* pParent, MODE mode, CSaDoc* pSaDoc);
  ~CDlgAnnotationWizard();

  // Attributes
protected:
  MODE    m_mode;
  int     m_nState;
  BOOL    m_bPhoneticModified;
  BOOL    m_bPhonemicModified;
  BOOL    m_bOrthographicModified;
  BOOL    m_bGlossModified;
  int     m_nAlignBy;
  int     m_nSegmentBy;
  CSaString m_szPhonetic;
  CSaString m_szPhonemic;
  CSaString m_szOrthographic;
  CSaString m_szGloss;
  CFont*  m_pFont;
  CSaDoc* m_pSaDoc;

  const char* m_szPhonemicNullSegment;
  const char* m_szOrthographicNullSegment;

  enum STATE { BACK,NEXT,INIT,PHONETICTEXT,PHONEMICTEXT,ORTHOGRAPHICTEXT,GLOSSTEXT,ALIGNBY,SEGMENTBY,FINISH};

public:
  // Dialog Data
  //{{AFX_DATA(CDlgAnnotationWizard)
  enum { IDD = IDD_ANNOTATIONWIZARD };
  BOOL    m_bOrthographic;
  BOOL    m_bPhonemic;
  BOOL    m_bPhonetic;
  BOOL    m_bGloss;
  //}}AFX_DATA

public:
  void AutoAlign(const CSaString& Phonetic, const CSaString& Phonemic,
    const CSaString& Ortho, const CSaString& Gloss,
    CSaString& Alignment, CSaString& Segmentation);

  void AutoAlign(const CSaString* Phonetic, const CSaString* Phonemic, const CSaString* Ortho, const CSaString* Gloss);
  // Operations
protected:
  void SetVisible(int nItem, BOOL bVisible);
  void SetEnable(int nItem, BOOL bEnable);
  void SetText(int nItem, CSaString szText);
  void SetText(int nItem, UINT nIDS);
  void SetChecked(int nItem);
  void SetState(int nState);
  void SetAnnotation(void);
  void SaveAnnotation(void);
  const CSaString BuildString(int nSegment);
  void OK();
  const CSaString ReadFile(const CSaString& Pathname);


  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  // Generated message map functions
  //{{AFX_MSG(CDlgAnnotationWizard)
  virtual BOOL OnInitDialog();
  afx_msg void OnBack();
  afx_msg void OnNext();
  afx_msg void OnRadio();
  afx_msg void OnImport();
  afx_msg void OnRevert();
  afx_msg void OnUpdateAnnotation();
  virtual void OnOK();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};


