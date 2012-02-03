//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_DLGVOWELFORMANTS_H__72C28DC1_E58D_11D5_9FE4_00E0988A4887__INCLUDED_)
#define AFX_DLGVOWELFORMANTS_H__72C28DC1_E58D_11D5_9FE4_00E0988A4887__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "activex\flexeditgrid.h"
// DlgVowelFormants.h : header file
//

class CVowelFormants
{
public:
  CVowelFormants(const CSaString &szVowel, double inF1, double inF2, double inF3, double inF4 = UNDEFINED_DATA);
  CSaString m_szVowel;
  double F1;
  double F2;
  double F3;
  double F4;

  void WriteProperties(Object_ostream& obs) const;
  BOOL ReadProperties(Object_istream& obs);

  void Init(const CSaString &szVowel, double inF1, double inF2, double inF3, double inF4 = UNDEFINED_DATA);
};

typedef std::vector<CVowelFormants> CVowelFormantsVector;

class CVowelSetVersion
{
public:
  CVowelSetVersion(const CSaString szVersion = _T("3.0")) {m_szVersion = szVersion;};

  void WriteProperties(Object_ostream& obs) const;
  BOOL ReadProperties(Object_istream& obs);

	CSaString GetVersion() {return m_szVersion;};
	void SetVersion(const CSaString& szVersion) {m_szVersion = szVersion;};

private:
	CSaString m_szVersion;
};

class CVowelFormantSet
{
public:
  CVowelFormantSet(const CSaString &szSetName = _T(""), BOOL bUser = TRUE) : m_szSetName(szSetName) {m_bUser = bUser;};
  CVowelFormantSet(const CSaString &szSetName, const CVowelFormantsVector vowels[3], BOOL bUser = TRUE);

  void WriteProperties(Object_ostream& obs) const;
  BOOL ReadProperties(Object_istream& obs);

  const CSaString& GetName() const {return m_szSetName;};
  void SetName(const CSaString& szName) {m_szSetName = szName;};
  const CVowelFormantsVector& GetVowelFormants(int gender) const {return m_vowels[gender];};
  void SetVowelFormants(int gender, const CVowelFormantsVector &cVector) {m_vowels[gender] = cVector;};

  BOOL IsUser() const {return m_bUser;}

  enum gender {male = 0, female = 1, child = 2};

private:
  BOOL m_bUser;
  CSaString m_szSetName;
  CVowelFormantsVector m_vowels[3];
};

class CVowelFormantSets : private std::vector<CVowelFormantSet>
{
public:
  CVowelFormantSets() {Load();};
  CVowelFormantSets(const CSaString &szFilename) : m_szFilename(szFilename) { Load(szFilename);}
  virtual ~CVowelFormantSets() {};

  BOOL Load(const CSaString &szFilename);
  BOOL Save(const CSaString &szFilename) const;
  BOOL Load() { return Load(m_szFilename);}
  BOOL Save() const {return Save(m_szFilename);}
  CSaString SetFilename(const CSaString &szFilename) { CSaString szOld(m_szFilename); m_szFilename = szFilename; return szOld;}
  int SetDefaultSet(int nSet);  // Error checked
  int GetDefaultSet() const {return m_nDefaultSet;}
  static CVowelFormantSet None();
  static CVowelFormantSet HillenbrandEtAl95();
  static CVowelFormantSet Ladefoged93();
  static CVowelFormantSet PetersonBarney52();
  static CVowelFormantSet Whitley();
  static CVowelFormantSet DanielJones();
  static CVowelFormantSet SynthesisLadefoged();


  const CVowelFormantSet& operator[](size_type _P) const {return static_cast<const std::vector<CVowelFormantSet>*>(this)->operator[](_P);}
  CVowelFormantSet& operator[](size_type _P) {return static_cast<std::vector<CVowelFormantSet>*>(this)->operator[](_P);}
  size_type size() const {return static_cast<const std::vector<CVowelFormantSet>*>(this)->size();}

private:
  CSaString m_szFilename;
  int m_nDefaultSet;
};


/////////////////////////////////////////////////////////////////////////////
// CDlgVowelFormants dialog

class CDlgVowelFormants : public CDialog
{
// Construction
public:
	CDlgVowelFormants(CVowelFormantSet &cVowelSet, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    CVowelFormantSet &m_cVowelSetOK;
    CVowelFormantSet m_cSet;
	CFlexEditGrid	m_cGrid;
	//{{AFX_DATA(CDlgVowelFormants)
	enum { IDD = IDD_VOWEL_FORMANTS };
	int		m_nGender;
	CSaString	m_szSetName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgVowelFormants)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	BOOL OnValidateGenderChange();
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgVowelFormants)
	afx_msg void OnGenderChange();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

const CVowelFormantsVector& GetVowelVector(int nGender);
const CVowelFormantSet& GetDefaultVowelSet();
CVowelFormantSets& GetVowelSets();

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGVOWELFORMANTS_H__72C28DC1_E58D_11D5_9FE4_00E0988A4887__INCLUDED_)
