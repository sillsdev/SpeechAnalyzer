// DlgVowelFormants.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "sa_doc.h"
#include "mainfrm.h"
#include "DlgVowelFormants.h"
#include "objectistream.h"
#include "FileUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void PopulateGrid(CFlexEditGrid & cGrid, const CVowelFormantsVector & cVowels);
static CVowelFormantsVector ParseVowelGrid(CFlexEditGrid & cGrid, BOOL & bSuccess);

CDlgVowelFormants::CDlgVowelFormants(CVowelFormantSet & cVowelSet, CWnd * pParent) :
    CDialog(CDlgVowelFormants::IDD, pParent),
    m_cVowelSetOK(cVowelSet),
    m_cSet(cVowelSet) {
    m_szSetName = m_cSet.GetName();
    m_nGender = 0;
}


void CDlgVowelFormants::DoDataExchange(CDataExchange * pDX) {

    CDialog::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_MALE, m_nGender);
    DDX_Control(pDX, IDC_VOWEL_GRID, m_cGrid);
    DDX_Text(pDX, IDC_EDIT_SET_NAME, m_szSetName);
}


BEGIN_MESSAGE_MAP(CDlgVowelFormants, CDialog)
    ON_BN_CLICKED(IDC_CHILD, OnGenderChange)
    ON_BN_CLICKED(IDC_FEMALE, OnGenderChange)
    ON_BN_CLICKED(IDC_MALE, OnGenderChange)
END_MESSAGE_MAP()

void CDlgVowelFormants::OnGenderChange() {

    OnValidateGenderChange();
}

BOOL CDlgVowelFormants::OnValidateGenderChange() {

    BOOL bSuccess = FALSE;

    CVowelFormantsVector cVowelVector = ParseVowelGrid(m_cGrid, bSuccess);

    if (bSuccess) {
        m_cSet.SetVowelFormants(m_nGender, cVowelVector);
        UpdateData();
        PopulateGrid(m_cGrid, m_cSet.GetVowelFormants(m_nGender));
    } else {
        AfxMessageBox(IDS_VOWEL_FORMANTS_INVALID);
        UpdateData(FALSE); // Don't accept gender change
    }

    return bSuccess;
}

BOOL CDlgVowelFormants::OnInitDialog() {

    CDialog::OnInitDialog();

    PopulateGrid(m_cGrid, m_cSet.GetVowelFormants(m_nGender));
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVowelFormants::OnOK() {
    UpdateData();
    if (OnValidateGenderChange()) {
        m_cSet.SetName(m_szSetName);
        m_cVowelSetOK = m_cSet;
        pApp->GetVowelSets().Save();
        CDialog::OnOK();
    }
}

void CDlgVowelFormants::OnCancel() {
    CDialog::OnCancel();
}

static void PopulateGrid(CFlexEditGrid & cGrid, const CVowelFormantsVector & cVowels) {

    enum { columnIpa = 0, columnF1, columnF2, columnF3, columnF4};

    int nRow = 0;
    cGrid.Clear();
    cGrid.SetColWidth(columnIpa, 0, 800);
    cGrid.SetTextMatrix(nRow, columnIpa, _T("Vowel"));
    cGrid.SetTextMatrix(nRow, columnF1, _T("F1"));
    cGrid.SetTextMatrix(nRow, columnF2, _T("F2"));
    cGrid.SetTextMatrix(nRow, columnF3, _T("F3"));
    cGrid.SetTextMatrix(nRow, columnF4, _T("F4"));

    for (CVowelFormantsVector::const_iterator pVowel=cVowels.begin(); pVowel!=cVowels.end(); pVowel++) {
        const CVowelFormants & rVowel = *pVowel;

        nRow++;

        cGrid.SetTextMatrix(nRow, columnIpa, rVowel.m_szVowel);

        CSaString szFormant;

        szFormant.Format(_T("%g"), rVowel.F1);
        cGrid.SetTextMatrix(nRow, columnF1, szFormant);

        szFormant.Format(_T("%g"), rVowel.F2);
        cGrid.SetTextMatrix(nRow, columnF2, szFormant);

        szFormant.Format(_T("%g"), rVowel.F3);
        cGrid.SetTextMatrix(nRow, columnF3, szFormant);

        if (rVowel.F4 > 0) {
            szFormant.Format(_T("%g"), rVowel.F4);
            cGrid.SetTextMatrix(nRow, columnF4, szFormant);
        }
    }

    CMainFrame * pMain = static_cast<CMainFrame *>(AfxGetMainWnd());
    CSaString szFont;
    int nFontSize;
    szFont = pMain->GetFontFace(PHONETIC);
    nFontSize = pMain->GetFontSize(PHONETIC);

    cGrid.SetFont(szFont,(float)nFontSize,1,columnIpa,-1, 1);
}

static CVowelFormantsVector ParseVowelGrid(CFlexEditGrid & cGrid, BOOL & bSuccess) {

    bSuccess = TRUE;

    enum { columnIpa = 0, columnF1, columnF2, columnF3, columnF4};

    CVowelFormantsVector cVowels;
    CVowelFormants cVowel(_T(""),-1,-1,-1,-1);

    cVowels.reserve(cGrid.GetRows());
    for (int row = 1; row < cGrid.GetRows(); row++) {
        cVowel.Init(_T(""),-1,-1,-1,-1);
        BOOL bRowValid = TRUE;
        cVowel.m_szVowel = cGrid.GetTextMatrix(row,columnIpa);
        if (cVowel.m_szVowel.IsEmpty()) {
            if (!(cGrid.GetTextMatrix(row,columnF1).IsEmpty()
                    && cGrid.GetTextMatrix(row,columnF2).IsEmpty()
                    && cGrid.GetTextMatrix(row,columnF3).IsEmpty()
                    && cGrid.GetTextMatrix(row,columnF4).IsEmpty())) {
                bSuccess = FALSE;
            }
            continue;
        }

        CSaString value;

        value = cGrid.GetTextMatrix(row,columnF1);
        int scanned = swscanf_s(value, _T("%lf"), &cVowel.F1);
        if (scanned != 1 || cVowel.F1 < 200. || cVowel.F1 > 5000.) {
            bSuccess = FALSE;
            bRowValid = FALSE;
            continue;
        }
        value = cGrid.GetTextMatrix(row,columnF2);
        scanned = swscanf_s(value, _T("%lf"), &cVowel.F2);
        if (scanned != 1 || cVowel.F2 < 200. || cVowel.F2 > 5000.) {
            bSuccess = FALSE;
            bRowValid = FALSE;
            continue;
        }
        value = cGrid.GetTextMatrix(row,columnF3);
        scanned = swscanf_s(value, _T("%lf"), &cVowel.F3);
        if (scanned != 1 || cVowel.F3 < 200. || cVowel.F3 > 5000.) {
            bSuccess = FALSE;
            bRowValid = FALSE;
            continue;
        }
        value = cGrid.GetTextMatrix(row,columnF4);
        scanned = swscanf_s(value, _T("%lf"), &cVowel.F4);
        if ((cVowel.F4 != -1)&&(cVowel.F4 < 200. || cVowel.F4 > 5000.)) {
            bSuccess = FALSE;
            bRowValid = FALSE;
            continue;
        }

        if (bRowValid) {
            cVowels.push_back(cVowel); // add to end of list
        }
    }
    return cVowels;
}

