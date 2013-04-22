// DlgVocalTract.cpp : implementation file
//

#include "stdafx.h"
#include "sa.h"
#include "sa_doc.h"
#include "Segment.h"
#include "sa_view.h"
#include "Segment.h"
#include "DlgKlattAll.h"
#include "FileUtils.h"
#include "dlgvocaltract.h"
#include "mainfrm.h"
#include "MusicPhraseSegment.h"
#include "PhoneticSegment.h"
#include "SFMHelper.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_spi.h"
#include "Process\sa_p_poa.h"
#include "Process\sa_p_fra.h"
#include "Process\Butterworth.h"
#include "dsp\AcousticTube.h"
#include <iterator>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgVTOrder dialog


CDlgVTOrder::CDlgVTOrder(CWnd * pParent /*=NULL*/)
    : CDialog(CDlgVTOrder::IDD, pParent)
{
    m_nOrder = 0;
}

void CDlgVTOrder::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_ORDER, m_nOrder);
    DDV_MinMaxUInt(pDX, m_nOrder, 1, 889);
}

BEGIN_MESSAGE_MAP(CDlgVTOrder, CDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgVTOrder message handlers

void CDlgVTOrder::OnOK()
{
    UpdateData();

    CDialog::OnOK();
}

void CIpaVTCharVector::Load(CString szPath)
{
    // is there a saved map???
    CFileStatus status; // file status

    reserve(1024);  // growth is exponential save some time here

    if ((!CFile::GetStatus(szPath, status)) || (status.m_size==0))
    {
        return;
    }

    CStdioFile file;

    file.Open(szPath,CFile::modeRead | CFile::shareDenyWrite);

    CString line;
    while (file.ReadString(line))
    {
        if (line[0] == '#')   // skip comments
        {
            continue;
        }


        int field=0;
        CString value;
        CIpaVTChar columnChar;

        value = CSFMHelper::ExtractTabField(line, field++);
        columnChar.m_ipa = value;
        if (columnChar.m_ipa.IsEmpty())
        {
            continue;
        }


        columnChar.m_duration = 0;
        value = CSFMHelper::ExtractTabField(line, field++);
        int nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_duration);

        columnChar.m_dFrameEnergy = 0;
        value = CSFMHelper::ExtractTabField(line, field++);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_dFrameEnergy);

        columnChar.m_stimulus.Pitch = 0;
        value = CSFMHelper::ExtractTabField(line, field++);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_stimulus.Pitch);

        columnChar.m_stimulus.AV = 0;
        value = CSFMHelper::ExtractTabField(line, field++);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_stimulus.AV);

        columnChar.m_stimulus.AF = 0;
        value = CSFMHelper::ExtractTabField(line, field++);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_stimulus.AF);

        columnChar.m_stimulus.AH = 0;
        value = CSFMHelper::ExtractTabField(line, field++);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_stimulus.AH);

        columnChar.m_stimulus.VHX = 0;
        value = CSFMHelper::ExtractTabField(line, field++);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_stimulus.VHX);

        columnChar.m_dVTGain = 0;
        value = CSFMHelper::ExtractTabField(line, field++);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_dVTGain);

        value = CSFMHelper::ExtractTabField(line, field++);
        int i = 0;
        swscanf_s(value,_T("%d"),&i);
        const int VTAreas = i;
        columnChar.m_areas.reserve(VTAreas);
        for (i = 0; i < VTAreas; i++)
        {
            CString szValue;
            double dValue = 1.;

            szValue = CSFMHelper::ExtractTabField(line, field++);
            swscanf_s(szValue, _T("%lf"), &dValue);
            columnChar.m_areas.push_back(dValue);
        }

        value = CSFMHelper::ExtractTabField(line, field++);
        i = 0;
        swscanf_s(value,_T("%d"),&i);
        const int VTReflections = i;
        columnChar.m_reflection.reserve(VTReflections);
        for (i = 0; i < VTReflections; i++)
        {
            CString szValue;
            double dValue = 0.;

            szValue = CSFMHelper::ExtractTabField(line, field++);
            swscanf_s(szValue, _T("%lf"), &dValue);
            columnChar.m_reflection.push_back(dValue);
        }

        value = CSFMHelper::ExtractTabField(line, field++);
        i = 0;
        swscanf_s(value,_T("%d"),&i);
        const int VTPreds = i;
        columnChar.m_pred.reserve(VTPreds);
        for (i = 0; i < VTPreds; i++)
        {
            CString szValue;
            double dValue = 0.;

            szValue = CSFMHelper::ExtractTabField(line, field++);
            swscanf_s(szValue, _T("%lf"), &dValue);
            columnChar.m_pred.push_back(dValue);
        }

        this->push_back(columnChar); // add to end of list
    }

    CIpaVTCharVector(*this).swap(*this); // shrink size using swap trick from Effective STL #18

    file.Close();
}

void CIpaVTCharVector::Save(CString szPath)
{
    CFile file;

    UINT nSuccess = file.Open(szPath,CFile::modeWrite | CFile::modeCreate | CFile::shareDenyWrite);
    if (!nSuccess)
    {
        //MsgBox(GetDesktopWindow(), "Could not open IpaDefaults.txt. Check Read-Only status.", "IPADefaults", MB_OK);
        ::MessageBoxA(::GetDesktopWindow(),"Could not open IpaVTDefaults#.txt.\n    Check Read-Only status.",
                      "IPAVTDefaults#",MB_OK|MB_ICONEXCLAMATION);
        return;
    }

    const_iterator pParm;

    for (pParm = begin(); pParm != end(); pParm++)
    {
        const CIpaVTChar & vtChar = *pParm;
        CString entry;
        CString field;

        entry = pParm->m_ipa + "\t";

        field.Format(_T("%f\t"),vtChar.m_duration);
        entry = entry + field;

        field.Format(_T("%f\t"),vtChar.m_dFrameEnergy);
        entry = entry + field;

        field.Format(_T("%f\t%f\t%f\t%f\t%f\t"),
                     vtChar.m_stimulus.Pitch,
                     vtChar.m_stimulus.AV,
                     vtChar.m_stimulus.AF,
                     vtChar.m_stimulus.AH,
                     vtChar.m_stimulus.VHX);
        entry = entry + field;

        field.Format(_T("%f\t"),vtChar.m_dVTGain);
        entry = entry + field;

        unsigned int i;

        field.Format(_T("%d\t"),vtChar.m_areas.size());
        entry = entry + field;

        for (i=0; i<vtChar.m_areas.size(); i++)
        {
            field.Format(_T("%f\t"),vtChar.m_areas[i]);
            entry = entry + field;
        }

        field.Format(_T("%d\t"),vtChar.m_reflection.size());
        entry = entry + field;

        for (i=0; i<vtChar.m_reflection.size(); i++)
        {
            field.Format(_T("%f\t"),vtChar.m_reflection[i]);
            entry = entry + field;
        }

        field.Format(_T("%d\t"),vtChar.m_pred.size());
        entry = entry + field;

        for (i=0; i<vtChar.m_pred.size(); i++)
        {
            field.Format(_T("%f\t"),vtChar.m_pred[i]);
            entry = entry + field;
        }

        entry += "\r\n";

        file.Write(entry, entry.GetLength());
    }
}

CIpaVTCharMap::CIpaVTCharMap(CIpaVTCharVector & vect)
{
    CIpaVTCharVector::const_iterator pChar = vect.begin();
    for (pChar = vect.begin(); pChar != vect.end(); pChar++)
    {
        (*this)[pChar->m_ipa] = *pChar;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgVocalTract

int CDlgVocalTract::m_nSelectedMethod = CDlgVocalTract::kFragment;
BOOL CDlgVocalTract::m_bMinimize = TRUE;
CSaDoc * CDlgVocalTract::m_pShowDoc = NULL;

CDlgVocalTract * CDlgVocalTract::m_pDlgSynthesis = NULL;


CDlgVocalTract::CDlgVocalTract(CWnd * pParent /*=NULL*/)
    : CFrameWnd()
{
    m_nSelectedView = m_nSelectedMethod;
    m_szGrid[kFragment] = _T("Fragments.txt");

    DWORD dwStyle = 0;

    dwStyle |= WS_POPUPWINDOW;
    dwStyle |= WS_CAPTION;
    dwStyle |= WS_MINIMIZEBOX;
    dwStyle |= WS_MAXIMIZEBOX;
    dwStyle |= WS_THICKFRAME;
    //dwStyle |= WS_MAXIMIZE;

    m_bArtificialStimulus = FALSE;
    m_bWindowLPC = FALSE;
    m_bPreEmphasize = FALSE;
    m_bClosedPhase = FALSE;
    m_bMirror = FALSE;
    m_nMethod = LPC_COVAR_LATTICE;
    m_nStructure = LOSSLESS_TUBE;
    m_nTilt = DB0;
    m_nRequestedOrder = 0;
    m_nCurrentOrder = 0;
    m_dwSampleRate = 22050;

    // Here we go all initialization should have taken place
    BOOL bResult = LoadFrame(IDR_SYNTHESIS_VOCAL_TRACT, dwStyle, pParent);

    ASSERT(bResult);

    CRect rc(20,20,400,400); // arbitrary rect
    if (pParent)
    {
        pParent->GetWindowRect(rc);
    }
    rc.DeflateRect(20,20);
    MoveWindow(rc);
    ShowWindow(SW_SHOWMAXIMIZED);
}

CDlgVocalTract::~CDlgVocalTract()
{
    if (m_nCurrentOrder)
    {
        m_cDefaults.Save(GetDefaultsPath(m_nCurrentOrder));
    }
}


void CDlgVocalTract::CreateSynthesizer(CWnd * pParent)
{
    DestroySynthesizer();
    ASSERT(m_pDlgSynthesis == NULL);
    m_pDlgSynthesis = new CDlgVocalTract(pParent);
}

void CDlgVocalTract::DestroySynthesizer()
{
    if (m_pDlgSynthesis)
    {
        m_pDlgSynthesis->DestroyWindow();
    }

    if (m_pDlgSynthesis)
    {
        delete m_pDlgSynthesis;
        m_pDlgSynthesis = NULL;
    }
}

BEGIN_MESSAGE_MAP(CDlgVocalTract, CFrameWnd)
    ON_COMMAND(ID_KLATT_GET_ALL, OnGetAll)
    ON_COMMAND(ID_CLOSE, OnClose)
    ON_UPDATE_COMMAND_UI(ID_CLOSE, OnUpdateClose)
    ON_WM_CREATE()
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_COMMAND(ID_EDIT_CUT, OnEditCut)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
    ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
    ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
    ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
    ON_COMMAND(ID_PLAY_BOTH, OnPlayBoth)
    ON_COMMAND(ID_PLAY_SYNTH, OnPlaySynth)
    ON_COMMAND(ID_PLAY_ORIGINAL, OnPlaySource)
    ON_COMMAND(ID_SYNTH_DISPLAY, OnSynthDisplay)
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
    ON_COMMAND(ID_SYNTHESIS_VTRACT_ARTIFICIAL, OnArtificial)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_ARTIFICIAL, OnUpdateArtificial)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_RESIDUAL, OnResidual)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_RESIDUAL, OnUpdateResidual)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_WINDOW, OnWindow)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_WINDOW, OnUpdateWindow)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_LOSSLESS_TUBE, OnLosslessTube)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_LOSSLESS_TUBE, OnUpdateLosslessTube)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_LATTICE, OnLattice)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_LATTICE, OnUpdateLattice)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_DIRECT, OnDirect)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_DIRECT, OnUpdateDirect)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_COVARIANCE, OnCovariance)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_COVARIANCE, OnUpdateCovariance)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_AUTOCOR, OnAutocor)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_AUTOCOR, OnUpdateAutocor)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_ORDER, OnOrder)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_VTGAIN, OnVTGain)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_VTGAIN, OnUpdateVTGain)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_12DB, On12dB)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_12DB, OnUpdate12dB)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_6DB, On6dB)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_6DB, OnUpdate6dB)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_0DB, On0dB)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_0DB, OnUpdate0dB)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_MIRROR, OnMirror)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_MIRROR, OnUpdateMirror)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_SQ_ERROR, OnUpdateSqError)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_SQ_ERROR, OnSqError)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_CLOSED_PHASE, OnClosedPhase)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_CLOSED_PHASE, OnUpdateClosedPhase)
    ON_COMMAND(ID_SYNTHESIS_ADJUST_CELLS, OnAdjustCells)
    ON_COMMAND(ID_KLATT_FRAGMENTS, OnFragments)
    ON_UPDATE_COMMAND_UI(ID_KLATT_FRAGMENTS, OnUpdateFragments)
    ON_COMMAND(ID_KLATT_IPA, OnIpa)
    ON_UPDATE_COMMAND_UI(ID_KLATT_IPA, OnUpdateIpa)
    ON_COMMAND(ID_KLATT_IPA_DEFAULTS, OnIpaDefaults)
    ON_UPDATE_COMMAND_UI(ID_KLATT_IPA_DEFAULTS, OnUpdateIpaDefaults)
    ON_COMMAND(ID_KLATT_IPA_BLEND, OnIpaBlend)
    ON_UPDATE_COMMAND_UI(ID_KLATT_IPA_BLEND, OnUpdateIpaBlend)
    ON_WM_DESTROY()
    ON_COMMAND(ID_SYNTH_HIDE, OnSynthHide)
    ON_UPDATE_COMMAND_UI(ID_SYNTH_HIDE, OnUpdateSynthHide)
    ON_COMMAND(ID_SYNTH_SHOW, OnSynthShow)
    ON_COMMAND(ID_SYNTHESIS_VTRACT_PREEMPHASIZE, OnPreemphasize)
    ON_UPDATE_COMMAND_UI(ID_SYNTHESIS_VTRACT_PREEMPHASIZE, OnUpdatePreemphasize)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgVocalTract message handlers

void CDlgVocalTract::PostNcDestroy()
{
    if (m_pDlgSynthesis)
    {
        delete m_pDlgSynthesis;
        m_pDlgSynthesis = NULL;
    }

    CWnd::PostNcDestroy();
}

int CDlgVocalTract::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    CRect rc(0,0,0,0);
    for (int i=0; i < kGrids; i++)
    {
        m_cGrid[i].Create(NULL, _T(""), WS_VISIBLE,rc,this,i);
        m_cGrid[i].SetBorderStyle(FALSE);
        m_cGrid[i].PreSubclassWindow();
        m_cGrid[i].SetAllowUserResizing(3 /*flexResizeBoth*/);
        m_cGrid[i].FakeArrowKeys(TRUE); // for some reason the arrow keys do not seem to work unless on a dialog ??
    }

    this->LabelGrid(kFragment);
    this->LabelGrid(kSegment);
    this->LabelGrid(kIpaBlended);
    this->LabelGrid(kDefaults);

    ShowGrid(m_nSelectedMethod);
    PopulateParameterGrid(kDefaults, m_cDefaults);
    OnGetAll();

    return 0;
}

void CDlgVocalTract::OnClose()
{
    DestroySynthesizer();
}

void CDlgVocalTract::OnUpdateClose(CCmdUI *)
{
}

void CDlgVocalTract::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);

    CRect rc;

    this->GetClientRect(rc);
    if (m_cGrid[m_nSelectedView].GetSafeHwnd())
    {
        m_cGrid[m_nSelectedView].MoveWindow(rc);
    }
}

BOOL CDlgVocalTract::OnCmdMsg(UINT nID, int nCode, void * pExtra, AFX_CMDHANDLERINFO * pHandlerInfo)
{
    return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CDlgVocalTract::ShowGrid(int nGrid)
{

    if (nGrid < 0 || nGrid >= kGrids)
    {
        ASSERT(FALSE);
        return;
    }

    // show the new grid first
    CRect rc;
    m_nSelectedView = nGrid;

    this->GetClientRect(rc);
    if (m_cGrid[nGrid].GetSafeHwnd())
    {
        m_cGrid[nGrid].MoveWindow(rc);
        m_cGrid[nGrid].SetEnabled(TRUE);
        m_cGrid[nGrid].SetFocus();
    }

    // hide all others
    for (int i=0; i < kGrids; i++)
    {
        if (i != nGrid)
        {
            m_cGrid[i].MoveWindow(0,0,0,0);
            m_cGrid[i].SetEnabled(FALSE);
        }
    }

    CString szText;
    GetWindowText(szText);
    int nStart = szText.Find(_T(" - "));
    if (nStart != -1)
    {
        szText = szText.Left(nStart);
    }
    szText += _T(" - ") + m_szGrid[nGrid];
    SetWindowText(szText);
}

void CDlgVocalTract::OnSynthDisplay()
{
    OnSynthesize();
    // open synthesized wavefile in SA
    CFileStatus status; // file status
    if (CFile::GetStatus(m_szSynthesizedFilename, status))
    {
        if (status.m_size!=0)
        {
            // file created open in SA
            CSaApp * pApp = (CSaApp *)(AfxGetApp());

            CSaDoc * pDoc = pApp->OpenWavFileAsNew(m_szSynthesizedFilename);
            m_szSynthesizedFilename.Empty();

            LabelDocument(pDoc);

            if (m_bMinimize)
            {
                ShowWindow(SW_MINIMIZE);
            }
        }
    }
}

void CDlgVocalTract::OnSynthShow()
{
    CString szSave(m_szSynthesizedFilename);

    m_szSynthesizedFilename  = m_szShowFilename;

    OnSynthesize();
    // open synthesized wavefile in SA
    CFileStatus status; // file status
    if (CFile::GetStatus(m_szSynthesizedFilename, status))
    {
        if (status.m_size!=0)
        {
            // file created open in SA
            CSaApp * pApp = (CSaApp *)(AfxGetApp());

            CSaDoc * pDoc = pApp->IsDocumentOpened(m_pShowDoc) ? m_pShowDoc : NULL;

            if (pDoc && pDoc->GetFileStatus()->m_szFullName != m_szSynthesizedFilename)
            {
                pDoc = NULL;
            }

            // Load temporarary file into document
            if (pDoc)
            {
                pDoc->ApplyWaveFile(m_szSynthesizedFilename, pDoc->GetUnprocessedDataSize(), FALSE);
            }

            if (!pDoc)
            {
                pDoc = pApp->OpenWavFileAsNew(m_szSynthesizedFilename);
            }
            m_szShowFilename = m_szSynthesizedFilename;

            LabelDocument(pDoc);

            if (m_bMinimize)
            {
                ShowWindow(SW_MINIMIZE);
            }

            m_szSynthesizedFilename = szSave;
            m_pShowDoc = pDoc;
        }
    }
}

void CDlgVocalTract::OnSetFocus(CWnd * pOldWnd)
{
    CFrameWnd::OnSetFocus(pOldWnd);

    m_cGrid[m_nSelectedView].SetFocus();
}

void CDlgVocalTract::PopulateParameterGrid(CFlexEditGrid & cGrid, const CIpaVTCharVector & cChars)
{
    CIpaVTCharVector::const_iterator pParm;
    int column = columnFirst;

    for (pParm = cChars.begin(); pParm != cChars.end(); pParm++)
    {
        PopulateParameterGrid(cGrid, *pParm, column);
        column++;
    }
}

void CDlgVocalTract::PopulateParameterGrid(CFlexEditGrid & cGrid, const CIpaVTChar & cChar, int column)
{
    if (column >= cGrid.GetCols(0))
    {
        cGrid.SetCols(0, column + 1);
        cGrid.SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,column,1,-1);
    }

    CString szString;


    // clear column
    szString.Empty();
    for (int row = rowFirst; row < cGrid.GetRows(); row++)
    {
        cGrid.SetTextMatrix(row,column,szString);
    }

    cGrid.SetTextMatrix(rowIpa,column,cChar.m_ipa);

    szString.Format(_T("%.2f"),cChar.m_duration);
    cGrid.SetTextMatrix(rowDuration,column,szString);

    if (cChar.m_stimulus.Pitch)
    {
        szString.Format(_T("%.5g"),cChar.m_stimulus.Pitch);
    }
    else
    {
        szString.Empty();
    }
    cGrid.SetTextMatrix(rowPitch,column,szString);

    szString.Format(_T("%.3g"), cChar.m_dFrameEnergy);
    cGrid.SetTextMatrix(rowFEnergy, column, szString);

    szString.Format(_T("%.1f"), cChar.m_stimulus.VHX);
    cGrid.SetTextMatrix(rowVHX, column, szString);

    szString.Format(_T("%.3g"), cChar.m_dVTGain);
    cGrid.SetTextMatrix(rowVTGain, column, szString);

    szString.Format(_T("%.3g"), cChar.m_stimulus.AV);
    cGrid.SetTextMatrix(rowAV,column,szString);
    szString.Format(_T("%.3g"), cChar.m_stimulus.AH);
    cGrid.SetTextMatrix(rowAH,column,szString);
    szString.Format(_T("%.3g"), cChar.m_stimulus.AF);
    cGrid.SetTextMatrix(rowAF,column,szString);

    unsigned int i;
    for (i=0; i<cChar.m_areas.size(); i++)
    {
        szString.Format(_T("%.5g"), cChar.m_areas[i]);
        cGrid.SetTextMatrix(i+getRow(rAreaFirst),column,szString);
    }

    for (i=0; i<cChar.m_reflection.size(); i++)
    {
        szString.Format(_T("%.5g"), cChar.m_reflection[i]);
        cGrid.SetTextMatrix(i+getRow(rReflectionFirst),column,szString);
    }

    for (i=0; i<cChar.m_pred.size(); i++)
    {
        szString.Format(_T("%.5g"), cChar.m_pred[i]);
        cGrid.SetTextMatrix(i+getRow(rPredFirst),column,szString);
    }
}

void CDlgVocalTract::LabelGrid(int nGrid)
{
    if (nGrid == -1)
    {
        nGrid = m_nSelectedView;
    }

    if (m_cGrid[nGrid].GetCols(0) < 3)
    {
        m_cGrid[nGrid].SetCols(0,3);
    }
    m_cGrid[nGrid].SetFixedCols(2);
    m_cGrid[nGrid].SetTextMatrix(rowHeading,columnDescription, _T("Description"));
    m_cGrid[nGrid].SetTextMatrix(rowHeading,columnSym, _T("Sym"));
    m_cGrid[nGrid].SetColWidth(columnDescription,0, 2500);
    m_cGrid[nGrid].SetColWidth(columnSym,0, 600);
    {
        // Label Grid
        if (m_cGrid[nGrid].GetCols(0) < 100)
        {
            m_cGrid[nGrid].SetCols(0,100);
        }
        if (m_cGrid[nGrid].GetRows() != getRow(rLast))
        {
            m_cGrid[nGrid].SetRows(getRow(rLast));
        }
        m_cGrid[nGrid].SetTextMatrix(rowIpa,columnDescription, _T("IPA"));
        m_cGrid[nGrid].SetTextMatrix(rowIpa,columnSym, _T("IPA"));
        m_cGrid[nGrid].SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,columnFirst,1, -1);
        m_cGrid[nGrid].SetRowHeight(rowIpa, 500);

        m_cGrid[nGrid].SetTextMatrix(rowDuration,columnDescription, _T("Duration"));
        m_cGrid[nGrid].SetTextMatrix(rowDuration,columnSym, _T("ms"));
        m_cGrid[nGrid].SetTextMatrix(rowPitch,columnDescription, _T("Pitch"));
        m_cGrid[nGrid].SetTextMatrix(rowPitch,columnSym, _T("Hz"));
        m_cGrid[nGrid].SetTextMatrix(rowFEnergy,columnDescription, _T("Frame Energy"));
        m_cGrid[nGrid].SetTextMatrix(rowFEnergy,columnSym, _T("dB"));

        m_cGrid[nGrid].SetTextMatrix(rowAV,columnDescription, _T("Voicing"));
        m_cGrid[nGrid].SetTextMatrix(rowAV,columnSym, _T("dB"));
        m_cGrid[nGrid].SetTextMatrix(rowAH,columnDescription, _T("Aspiration"));
        m_cGrid[nGrid].SetTextMatrix(rowAH,columnSym, _T("dB"));
        m_cGrid[nGrid].SetTextMatrix(rowAF,columnDescription, _T("Frication"));
        m_cGrid[nGrid].SetTextMatrix(rowAF,columnSym, _T("dB"));
        m_cGrid[nGrid].SetTextMatrix(rowVHX,columnDescription, _T("VH Crossover"));
        m_cGrid[nGrid].SetTextMatrix(rowVHX,columnSym, _T("Hz"));

        m_cGrid[nGrid].SetTextMatrix(rowVTGain, columnDescription, _T("VT Gain"));
        m_cGrid[nGrid].SetTextMatrix(rowVTGain,columnSym, _T("dB"));

        for (int i = getRow(rAreaFirst); i < getRow(rLast); i++)
        {
            m_cGrid[nGrid].SetTextMatrix(i, columnSym, _T(""));
            m_cGrid[nGrid].SetTextMatrix(i, columnDescription, _T(""));
        }

        for (int i = getRow(rAreaFirst); i < getRow(rAreaLast); i++)
        {
            CString szArea;

            szArea.Format(_T("%d"), i-getRow(rAreaFirst)+1);
            m_cGrid[nGrid].SetTextMatrix(i, columnSym, szArea);

            szArea.Format(_T("Tube #%d area"), i-getRow(rAreaFirst)+1);
            m_cGrid[nGrid].SetTextMatrix(i, columnDescription, szArea);
        }
        m_cGrid[nGrid].SetTextMatrix(getRow(rAreaFirst), columnDescription, _T("Tube #1 area (source)"));

        for (int i = getRow(rPredFirst); i < getRow(rPredLast); i++)
        {
            CString szPred;

            szPred.Format(_T("%d"), i-getRow(rPredFirst)+1);
            m_cGrid[nGrid].SetTextMatrix(i, columnSym, szPred);

            szPred.Format(_T("Pred #%d"), i-getRow(rPredFirst)+1);
            m_cGrid[nGrid].SetTextMatrix(i, columnDescription, szPred);
        }

        for (int i = getRow(rReflectionFirst); i < getRow(rReflectionLast); i++)
        {
            CString szReflection;

            szReflection.Format(_T("%d"), i-getRow(rReflectionFirst)+1);
            m_cGrid[nGrid].SetTextMatrix(i, columnSym, szReflection);

            szReflection.Format(_T("Reflection #%d"), i-getRow(rReflectionFirst)+1);
            m_cGrid[nGrid].SetTextMatrix(i, columnDescription, szReflection);
        }
        m_cGrid[nGrid].SetTextMatrix(getRow(rAreaFirst), columnDescription, _T("Tube #1 area (source)"));

        NumberGrid(nGrid);
    }
}

void CDlgVocalTract::NumberGrid(int nGrid)
{
    for (int i=columnFirst; i<m_cGrid[nGrid].GetCols(0); i++)
    {
        CString number;
        number.Format(_T("%d"), i-columnFirst+1);
        m_cGrid[nGrid].SetTextMatrix(rowHeading,i, number);
    }
}

CString CDlgVocalTract::GetDefaultsPath(int nOrder)
{
    CSaString szPath = AfxGetApp()->GetProfileString(_T(""), _T("DataLocation"));
    szPath += "\\";

    CString szPathOrder;
    szPathOrder.Format(_T("%sVTDefaults%d.txt"), szPath, nOrder);

    return szPathOrder;
}


void CDlgVocalTract::OnGetAll()
{
    OnUpdateSourceName();
    if (m_nCurrentOrder != m_nRequestedOrder)
    {
        m_cDefaults.Save(GetDefaultsPath(m_nCurrentOrder));
        m_cDefaults.Load(GetDefaultsPath(m_nRequestedOrder));
        m_nCurrentOrder = m_nRequestedOrder;
        LabelGrid(kDefaults);
        PopulateParameterGrid(kDefaults, m_cDefaults);
    }

    LabelGrid(kSegment);
    LabelGrid(kIpaBlended);
    LabelGrid(kFragment);

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pDoc = (CSaDoc *)pApp->IsFileOpened(m_szSourceFilename);
    {

        CUttParm myUttParm;
        CUttParm * pUttParm = &myUttParm;
        pDoc->GetUttParm(pUttParm); // get sa parameters utterance member data
        CUttParm * pSavedUttParm = new CUttParm;

        CProcessSmoothedPitch * pPitch = pDoc->GetSmoothedPitch(); // SDM 1.5 Test 11.0
        pDoc->GetUttParm(pSavedUttParm); // save current smoothed pitch parameters
        pUttParm->nMinFreq = 40;
        pUttParm->nMaxFreq = 500;
        pUttParm->nCritLoud = 6;
        pUttParm->nMaxChange = 10;
        pUttParm->nMinGroup = 6;
        pUttParm->nMaxInterp = 7;
        if (pUttParm->nMinFreq != pSavedUttParm->nMinFreq
                || pUttParm->nMaxFreq != pSavedUttParm->nMaxFreq
                || pUttParm->nCritLoud != pSavedUttParm->nCritLoud
                || pUttParm->nMaxChange != pSavedUttParm->nMaxChange
                || pUttParm->nMinGroup != pSavedUttParm->nMinGroup
                || pUttParm->nMaxInterp != pSavedUttParm->nMaxInterp)
        {
            pPitch->SetDataInvalid();
        }
        pDoc->SetUttParm(pUttParm);
        short int nResult = LOWORD(pPitch->Process(this, pDoc)); // process data
        pDoc->SetUttParm(pSavedUttParm); // restore smoothed pitch parameters
        if (nResult == PROCESS_ERROR || nResult == PROCESS_CANCELED)
        {
            return;
        }

        delete pSavedUttParm;
    }

    OnGetFragments(m_cGrid[kFragment]);
    OnGetSegments(m_cGrid[kSegment]);
    ParseParameterGrid(kDefaults, m_cDefaults);
    OnApplyIpaDefaults(m_cGrid[kSegment]);
    OnBlendSegments(kSegment,m_cGrid[kIpaBlended]);

    {
        // Correct Pitch & Intensity
        // Get rid of smoothed pitch data so it doesn't interfere with an existing graph
        pDoc->GetSmoothedPitch()->SetDataInvalid();
    }
}

void CDlgVocalTract::OnUpdateSourceName()
{
    if (m_szSourceFilename.IsEmpty() || !static_cast<CSaApp *>(AfxGetApp())->IsFileOpened(m_szSourceFilename))
    {
        m_szSourceFilename.Empty();
        m_szShowFilename.Empty();
        m_pShowDoc = 0;

        // Populate Source
        CMDIChildWnd * pChild = static_cast<CMainFrame *>(AfxGetMainWnd())->MDIGetActive();
        while (pChild)
        {
            CDocument * pDoc = pChild->GetActiveDocument(); // get pointer to document
            if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CSaDoc)))
            {
                m_szSourceFilename = pDoc->GetPathName();
            }
            if (!m_szSourceFilename.IsEmpty())
            {
                if (m_nRequestedOrder == 0)
                {
                    DWORD samplesPerSec = reinterpret_cast<CSaDoc *>(pDoc)->GetSamplesPerSec();
                    m_nRequestedOrder = (samplesPerSec + 500)/1000;
                }
                break;
            }
            pChild = (CMDIChildWnd *) pChild->GetNextWindow();
        }
    }
}

void CDlgVocalTract::OnApplyIpaDefaults(CFlexEditGrid & cGrid)
{
    CIpaVTCharMap map(m_cDefaults);
    for (int column=columnFirst; column<cGrid.GetCols(0); column++)
    {
        CIpaVTChar cCurrent;
        ParseParameterGrid(cGrid, column, cCurrent);
        CString ipa = cCurrent.m_ipa;
        if (ipa.GetLength())
        {
            CIpaVTCharMap::const_iterator pParm = map.find(ipa);

            if (pParm == map.end())
            {
                pParm = map.find("undefined");
            }

            if (pParm != map.end())
            {
                CIpaVTChar cMixed = pParm->second;

                double adjust = cMixed.m_dFrameEnergy - cCurrent.m_dFrameEnergy;

                cMixed.m_dFrameEnergy = cCurrent.m_dFrameEnergy;
                cMixed.m_duration = cCurrent.m_duration;
                cMixed.m_stimulus.Pitch = cCurrent.m_stimulus.Pitch;

                cMixed.m_stimulus.AV += adjust;
                cMixed.m_stimulus.AH += adjust;
                cMixed.m_stimulus.AF += adjust;

                PopulateParameterGrid(cGrid, cMixed, column);
            }
        }
    }
}


static void CurveFitPitch(CSaDoc * pDoc, double fSizeFactor, double dBeginWAV, double dEndWAV, double * offset, double * slope)
{
    DWORD dwIndex;
    DWORD dwBegin = (DWORD)(dBeginWAV/fSizeFactor);
    DWORD dwEnd = (DWORD)(dEndWAV/fSizeFactor);

    int n = 0;
    double sumX = 0;
    double sumY = 0;
    double sumXX = 0;
    double sumYY = 0;
    double sumXY = 0;

    BOOL bRes = TRUE;
    for (dwIndex = dwBegin; dwIndex <= dwEnd; dwIndex++)
    {
        // get data for this pixel
        int nHere = pDoc->GetSmoothedPitch()->GetProcessedData(dwIndex, &bRes); // SDM 1.5Test11.0
        if (nHere > 0)
        {
            double Y = double(nHere)/PRECISION_MULTIPLIER;
            double X = double(dwIndex-dwBegin)*fSizeFactor;

            sumX += X;
            sumXX += X*X;
            sumY += Y;
            sumXY += X*Y;
            sumYY += Y*Y;

            n++;
        }
    }
    if (n>0)
    {
        double localSlope;
        if (sumX)
        {
            localSlope = (n*sumXY - sumX*sumY)/(n*sumXX - sumX*sumX);
        }
        else
        {
            localSlope = 0;    // if no change in x, assume 0 slope
        }
        double localOffset = sumY/n - localSlope*sumX/n;

        if (offset)
        {
            *offset = localOffset;
        }
        if (slope)
        {
            *slope = localSlope;
        }
    }
    else
    {
        if (offset)
        {
            *offset = - 1.;
        }
        if (slope)
        {
            *slope = 0;
        }
    }
}

// The following formula attempts to calculate the VH crossover using higher order statistics
// to distinguish between harmonics and noise.
// See derivation in mathcad...  You would never believe
static double VHX(double meanSqError, double meanQuadError, double /*analysisLength*/, double samplingRate, double pitch)
{
    double nyquistRate = samplingRate/2;
    double fr2 = meanSqError;
    double fr4 = meanQuadError;
    double a = -3*pitch;
    double r = (fr4/(fr2*fr2) - 3)*(1.5*pitch*nyquistRate*nyquistRate);

    double square = (27*r-4*a*a*a)*3*r;
    double squareRoot = square > 0 ? sqrt(square) : 0;
    double cube = 0.5*r - 1./27.*a*a*a + 1./18.* squareRoot;
    double cubeRoot = cube > 0 ? pow(cube, 1./3.) : -pow(-cube, 1./3.);

    double vhx = square > 0 ? (cubeRoot + 1/9.*a*a/cubeRoot - 1/3.*a) : 0;

    // The model generating the VHX assumes block harmonic to noise crossover
    // This results in an over estimate of the VHX parameter.
    // The correction factor is approximate... It might be better to update the model.
    vhx *= 0.5;

    if (vhx < 3 * pitch)
    {
        vhx = 3*pitch;
    }

    return vhx > samplingRate/2 ? samplingRate/2 - pitch : vhx;
}

struct SAnalyzer
{

    SAnalyzer(CSaDoc * ipDoc, WORD iwSmpSize, SIG_PARMS & iSignal, const LPC_SETTINGS & iLpcSetting, BOOL ibClosedPhase = FALSE)
        : Signal(iSignal), LpcSetting(iLpcSetting)
    {
        pDoc = ipDoc;
        wSmpSize = iwSmpSize;
        bClosedPhase = ibClosedPhase;
    }

    CSaDoc * pDoc;
    WORD wSmpSize;
    BOOL bClosedPhase;
    SIG_PARMS & Signal;
    const LPC_SETTINGS & LpcSetting;
};

static void Analyze(SAnalyzer & a, CString szIpa, DWORD dwStart, DWORD dwEnd, CIpaVTChar & cChar, DWORD dwDuration=0, DWORD dwOffset=0, std::vector<double> * pResidual=NULL)
{
    double pitch = 0;

    cChar.m_ipa = szIpa;

    cChar.m_duration = a.pDoc->GetTimeFromBytes((dwEnd - dwStart + 2) * 1000);

    {
        // Pitch
        double offset;
        double slope;
        CProcessSmoothedPitch * pPitch = a.pDoc->GetSmoothedPitch(); // SDM 1.5 Test 11.0
        double fSizeFactorPitch = (double)a.pDoc->GetDataSize() / (double)(pPitch->GetDataSize() - 1);

        CurveFitPitch(a.pDoc, fSizeFactorPitch, dwStart, dwEnd, &offset, &slope);
        if (offset > 0)
        {
            pitch = offset + slope*(dwEnd - dwStart + 1)/2.;
        }
        else
        {
            pitch = 0;
        }
        cChar.m_stimulus.Pitch = pitch;
    }

    {
        // Vocal Tract
        int nSections = a.LpcSetting.nOrder;
        int nSamplesPerSection = 5;
        int nNewStart = dwStart/a.wSmpSize;
        int nNewEnd = dwEnd/a.wSmpSize;
        int nMinLength = nSections*nSamplesPerSection;
        int nDesiredLength = nMinLength;

        if (pitch)
        {
            int nPitchLength = int(a.Signal.SmpRate/pitch);

            if (nDesiredLength < 3*nPitchLength)
            {
                nDesiredLength = 3*nPitchLength;
            }
        }

        if (DWORD(nDesiredLength)*a.wSmpSize > dwEnd - dwStart)
        {
            if (DWORD(nDesiredLength)*a.wSmpSize <= dwDuration)
            {
                int nCenter = (dwStart + dwEnd)/(2*a.wSmpSize);
                nNewStart = nCenter - nDesiredLength/2;
                nNewEnd = nNewStart + nDesiredLength;

                if (DWORD(nNewEnd*a.wSmpSize) > dwOffset + dwDuration)
                {
                    nNewEnd = (dwOffset + dwDuration)/a.wSmpSize;
                    nNewStart = nNewEnd - nDesiredLength;
                }

                if (DWORD(nNewStart*a.wSmpSize) < dwOffset)
                {
                    nNewStart = dwOffset/a.wSmpSize;
                    nNewEnd = nNewStart + nDesiredLength;
                }
            }
            else if ((dwEnd - dwStart) <= dwDuration)
            {
                nNewStart = dwOffset/a.wSmpSize;
                nNewEnd = (dwOffset + dwDuration)/a.wSmpSize;
            }
            else
            {
                if (nNewStart < nSections)
                {
                    nNewEnd += nSections - nNewStart;
                    nNewStart = nSections;
                }
            }
        }

        nNewStart -= nSections;

        if (nNewStart < 0)
        {
            nNewStart = 0;
        }

        // Get waveform and buffer parameters.
        DWORD    dwFrameSize = nNewEnd - nNewStart + 1;

        a.Signal.Start = (void *)a.pDoc->GetWaveData(nNewStart*a.wSmpSize, TRUE); //load sample
        a.Signal.Length = dwFrameSize;

        LPC_SETTINGS LpcSetting = a.LpcSetting;
        LpcSetting.nOrder = (unsigned short)nSections;
        LpcSetting.nFrameLen = (unsigned short)(dwFrameSize);

        if (a.bClosedPhase && pitch != 0)
        {
            // determine pitch phase
            // Construct an LPC object for vocal tract modeling.
            CLinPredCoding * pLpcObject = NULL;
            dspError_t Err;

            Err = CLinPredCoding::CreateObject(&pLpcObject, LpcSetting, a.Signal);

            // Perform LPC analysis.
            LPC_MODEL * pLpc;
            Err = pLpcObject->GetLpcModel(&pLpc, a.Signal.Start);

            double dSin = 0;
            double dCos = 0;

            for (int i = 0; i < pLpc->nResiduals; i++)
            {
                double dPhase;
                dPhase= (i+LpcSetting.nOrder)*(pitch/a.Signal.SmpRate)*2*PI;
                dSin += sin(dPhase)*pLpc->pResidual[i]*pLpc->pResidual[i];
                dCos += cos(dPhase)*pLpc->pResidual[i]*pLpc->pResidual[i];
            }

            double errorPhase = atan2(dSin, dCos);  // -PI to PI
            double phaseShift = -PI;  // Use shift -PI to PI
            LpcSetting.dPitch = pitch;
            LpcSetting.dClosurePhase = errorPhase + phaseShift >= 0 ? phaseShift + errorPhase : phaseShift + 2*PI + errorPhase;  // 0 to 2*PI

            delete pLpcObject; // delete the Lpc object
        }

        // Construct an LPC object for vocal tract modeling.
        CLinPredCoding * pLpcObject = NULL;
        dspError_t Err;

        Err = CLinPredCoding::CreateObject(&pLpcObject, LpcSetting, a.Signal);

        // Perform LPC analysis.
        LPC_MODEL * pLpc;
        Err = pLpcObject->GetLpcModel(&pLpc, a.Signal.Start);

        cChar.m_dFrameEnergy = 10*log10(pLpc->dMeanEnergy);

        cChar.m_dVTGain = -10*log10(pLpc->dMeanSqPredError / (pLpc->dMeanEnergy+0.25));

        if (pitch > 0)
        {
            cChar.m_stimulus.AV =
                cChar.m_stimulus.AH = 10*log10(pLpc->dMeanSqPredError) - 3;
            cChar.m_stimulus.AF = 0;

            // This is a place holder...
            // Needs to be replaced by an analysis of the Voicing Aspiration Crossover frequency
            cChar.m_stimulus.VHX = VHX(pLpc->dMeanSqPredError, pLpc->dMeanQuadPredError, pLpc->nResiduals, a.Signal.SmpRate, pitch);
        }
        else
        {
            cChar.m_stimulus.AV = 0;
            cChar.m_stimulus.AH = 0;
            cChar.m_stimulus.AF = 10*log10(pLpc->dMeanSqPredError) - 3;
            cChar.m_stimulus.VHX = 0;
        }

        cChar.m_areas.clear();
        cChar.m_areas.reserve(nSections+1);
        std::copy(&pLpc->pNormCrossSectArea[0], &pLpc->pNormCrossSectArea[nSections+1], std::back_inserter(cChar.m_areas));

        cChar.m_reflection.clear();
        cChar.m_reflection.reserve(nSections);
        std::copy(&pLpc->pReflCoeff[0], &pLpc->pReflCoeff[nSections], std::back_inserter(cChar.m_reflection));

        cChar.m_pred.clear();
        cChar.m_pred.reserve(nSections);
        std::copy(&pLpc->pPredCoeff[1], &pLpc->pPredCoeff[nSections+1], std::back_inserter(cChar.m_pred));

        if (pResidual)
        {
            std::vector<double> & residual = *pResidual;

            int nFirst = dwStart/a.wSmpSize - (nNewStart + nSections);
            int nLast = dwEnd/a.wSmpSize - (nNewStart + nSections);
            if (nFirst < 0)
            {
                residual.insert(residual.end(), -nFirst, 0);
                nFirst = 0;
            }
            if (pLpc->nResiduals <= nLast)
            {
                std::copy(&pLpc->pResidual[nFirst], &pLpc->pResidual[pLpc->nResiduals], std::back_inserter(residual));
                residual.insert(residual.end(), nLast + 1 - pLpc->nResiduals, 0);
            }
            else
            {
                std::copy(&pLpc->pResidual[nFirst], &pLpc->pResidual[nLast+1], std::back_inserter(residual));
            }
        }

        delete pLpcObject; // delete the Lpc object
        pLpcObject = NULL;
    }
}

void CDlgVocalTract::SilentColumn(CFlexEditGrid & cGrid, int column, CSaDoc * pDoc, DWORD dwDuration, WORD wSmpSize)
{
    CString szString;

    // clear parameters
    szString.Empty();
    for (int row = rowFirst; row < cGrid.GetRows(); row++)
    {
        cGrid.SetTextMatrix(row,column,szString);
    }

    szString.Format(_T("silence"));
    cGrid.SetTextMatrix(rowIpa,column,szString);

    if (dwDuration)
    {
        szString.Format(_T("%.2f"),pDoc->GetTimeFromBytes(dwDuration * wSmpSize)*1000.);
        cGrid.SetTextMatrix(rowDuration,column,szString);
    }

    cGrid.SetTextMatrix(rowAV,column,_T("0"));
    cGrid.SetTextMatrix(rowAH,column,_T("0"));
    cGrid.SetTextMatrix(rowAF,column,_T("0"));
}

void CDlgVocalTract::OnGetSegments(CFlexEditGrid & cGrid)
{
    CString szFilename = m_szSourceFilename;

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pDoc = (CSaDoc *)pApp->IsFileOpened(szFilename);
    if (!pDoc)
    {
        return;
    }
    CSegment * pPhonetic = pDoc->GetSegment(PHONETIC);

    if (pPhonetic->IsEmpty())   // no annotations
    {
        return;
    }

    enum {PITCH, CALCULATIONS};
    double fSizeFactor[CALCULATIONS];

    if (m_bPitch)   // formants need pitch info
    {
        CProcessSmoothedPitch * pPitch = pDoc->GetSmoothedPitch(); // SDM 1.5 Test 11.0
        fSizeFactor[PITCH] = (double)pDoc->GetDataSize() / (double)(pPitch->GetDataSize() - 1);
    }

    // process all flags
    CString szString;

    DWORD dwOffset = 0;
    DWORD dwPrevOffset;
    DWORD dwDuration = 0;
    int nIndex = 0;
    int column = columnFirst;
    const DWORD dwMinSilence = pDoc->GetBytesFromTime(0.0001);

    cGrid.SetCols(0, pPhonetic->GetOffsetSize());

    DWORD wSmpSize = pDoc->GetSampleSize();

    SIG_PARMS Signal;
    if (wSmpSize == 1)
    {
        Signal.SmpDataFmt = PCM_UBYTE;    //samples are unsigned 8 bit
    }
    else
    {
        Signal.SmpDataFmt = PCM_2SSHORT;    //samples are 2's complement 16 bit
    }
    Signal.SmpRate = pDoc->GetSamplesPerSec();  //set sample rate

    CIpaVTChar cChar;

    LPC_SETTINGS LpcSetting;
    LpcSetting.Process.Flags = NORM_CROSS_SECT | MEAN_SQ_ERR | ENERGY | RESIDUAL;
    if (m_bWindowLPC)
    {
        LpcSetting.Process.Flags |= WINDOW_SIGNAL;
    }
    if (m_bPreEmphasize)
    {
        LpcSetting.Process.Flags |= PRE_EMPHASIS;
    }
    LpcSetting.nMethod = (unsigned short)m_nMethod;
    LpcSetting.nOrder = (unsigned short)m_nCurrentOrder;
    LpcSetting.nFrameLen = 0;

    SAnalyzer analyze(pDoc, wSmpSize, Signal, LpcSetting, m_bClosedPhase);

    // construct table entries
    while (nIndex != -1)
    {
        dwPrevOffset = dwOffset;
        dwOffset = pPhonetic->GetOffset(nIndex);
        if (dwPrevOffset + dwDuration + dwMinSilence < dwOffset)
        {
            dwDuration = dwOffset - (dwPrevOffset + dwDuration);
            SilentColumn(cGrid, column, pDoc, dwDuration, 1);
            column++;
        }
        dwDuration = pPhonetic->GetDuration(nIndex);

        szString = pPhonetic->GetSegmentString(nIndex);

        Analyze(analyze, szString, dwOffset, dwOffset + dwDuration, cChar, dwDuration, dwOffset, NULL);
        PopulateParameterGrid(cGrid, cChar, column);

        nIndex = pPhonetic->GetNext(nIndex);

        column++;

        if (column >= cGrid.GetCols(0))
        {
            cGrid.SetCols(0, column+10);
            cGrid.SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,column,1,-1);
        }
    }

    dwPrevOffset = dwOffset;
    dwOffset = pDoc->GetDataSize();
    if (dwPrevOffset + dwDuration + dwMinSilence < dwOffset)
    {
        dwDuration = dwOffset - (dwPrevOffset + dwDuration);
        SilentColumn(cGrid, column, pDoc, dwDuration, 1);
        column++;
    }

    // clear residual columns
    cGrid.SetCols(0, column);
}

void CDlgVocalTract::OnGetFragments(CFlexEditGrid & cGrid)
{
    CString szFilename = m_szSourceFilename;

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pDoc = (CSaDoc *)pApp->IsFileOpened(szFilename);
    if (!pDoc)
    {
        return;
    }
    CSegment * pPhonetic = pDoc->GetSegment(PHONETIC);
    m_dwSampleRate = pDoc->GetSamplesPerSec();
    if (pPhonetic->IsEmpty())   // no annotations
    {
        return;
    }

    enum {PITCH, FORMANTS, CALCULATIONS};
    double fSizeFactor[CALCULATIONS];

    BOOL bPitch = TRUE;
    BOOL bVocalTract = TRUE;

    if (bPitch || bVocalTract)   // formants need pitch info
    {
        CProcessSmoothedPitch * pPitch = pDoc->GetSmoothedPitch(); // SDM 1.5 Test 11.0
        fSizeFactor[PITCH] = (double)pDoc->GetDataSize() / (double)(pPitch->GetDataSize() - 1);
    }

    // process all flags
    CString szString;

    CProcessFragments * pFragment = pDoc->GetFragments();
    DWORD wSmpSize = pDoc->GetSampleSize();
    SIG_PARMS Signal;
    if (bVocalTract)
    {
        residual.clear();
        residual.reserve(pDoc->GetDataSize()/wSmpSize);
        if (wSmpSize == 1)
        {
            Signal.SmpDataFmt = PCM_UBYTE;      //samples are unsigned 8 bit
        }
        else
        {
            Signal.SmpDataFmt = PCM_2SSHORT;    //samples are 2's complement 16 bit
        }
        Signal.SmpRate = pDoc->GetSamplesPerSec();  //set sample rate
    }

    DWORD dwOffset = 0;
    DWORD dwDuration = 0;
    DWORD dwPrevOffset = 0;

    DWORD dwFragmentIndex = 0;
    DWORD dwFragmentStart = 0;
    DWORD dwFragmentEnd = 0;
    FRAG_PARMS stFragment;

    DWORD dwLastFragmentIndex = pFragment->GetFragmentIndex((pDoc->GetDataSize() - 1) / wSmpSize);

    // resize grid to number of fragments in file
    m_cGrid[kFragment].SetCols(0,columnFirst + dwLastFragmentIndex);
    NumberGrid(kFragment);
    m_cGrid[kFragment].SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,columnFirst,1, -1);

    int nIndex = 0;
    int column = columnFirst;
    const DWORD dwMinSilence = pDoc->GetBytesFromTime(0.0001);

    CIpaVTChar cChar;

    LPC_SETTINGS LpcSetting;
    LpcSetting.Process.Flags = NORM_CROSS_SECT | MEAN_SQ_ERR | ENERGY | RESIDUAL;
    if (m_bWindowLPC)
    {
        LpcSetting.Process.Flags |= WINDOW_SIGNAL;
    }
    if (m_bPreEmphasize)
    {
        LpcSetting.Process.Flags |= PRE_EMPHASIS;
    }
    LpcSetting.nMethod = (unsigned short)m_nMethod;
    LpcSetting.nOrder = (unsigned short)m_nCurrentOrder;
    LpcSetting.nFrameLen = 0;

    SAnalyzer analyze(pDoc, wSmpSize, Signal, LpcSetting, m_bClosedPhase);

    // construct table entries
    while (nIndex != -1)
    {
        dwPrevOffset = dwOffset;
        dwOffset = pPhonetic->GetOffset(nIndex);
        if (dwPrevOffset + dwDuration + dwMinSilence < dwOffset)
        {
            dwDuration = dwOffset - (dwPrevOffset + dwDuration);
            SilentColumn(cGrid, column, pDoc, dwDuration, 1);
            // clear parameters
            residual.insert(residual.end(), dwDuration/wSmpSize, 0);

            column++;
        }

        dwDuration = pPhonetic->GetDuration(nIndex);
        dwFragmentIndex = pFragment->GetFragmentIndex(dwOffset / wSmpSize);
        dwLastFragmentIndex = pFragment->GetFragmentIndex((dwOffset + dwDuration - 1) / wSmpSize);


        while (dwFragmentIndex <= dwLastFragmentIndex)
        {
            szString = pPhonetic->GetSegmentString(nIndex);

            stFragment = pFragment->GetFragmentParms(dwFragmentIndex);
            dwFragmentStart = stFragment.dwOffset * wSmpSize;
            if (dwFragmentStart < dwOffset)
            {
                dwFragmentStart = dwOffset;
            }
            dwFragmentEnd = (stFragment.dwOffset + stFragment.wLength - 1) * wSmpSize;
            if (dwFragmentEnd > dwOffset + dwDuration)
            {
                dwFragmentEnd = dwOffset + dwDuration;
            }

            Analyze(analyze, szString, dwFragmentStart, dwFragmentEnd, cChar, dwDuration, dwOffset, &residual);
            PopulateParameterGrid(cGrid, cChar, column);

            column++;

            if (column >= cGrid.GetCols(0))
            {
                cGrid.SetCols(0, column+10);
                cGrid.SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,column,1,-1);
            }

            dwFragmentIndex++;
        }
        nIndex = pPhonetic->GetNext(nIndex);
    }

    dwPrevOffset = dwOffset;
    dwOffset = pDoc->GetDataSize();
    if (dwPrevOffset + dwDuration + dwMinSilence < dwOffset)
    {
        dwDuration = dwOffset - (dwPrevOffset + dwDuration);
        SilentColumn(cGrid, column, pDoc, dwDuration, 1);

        column++;
    }

    // clear residual columns
    cGrid.SetCols(0, column);

    // m_bGetComplete = TRUE;
    if (bVocalTract)
    {
        int nDuration = pDoc->GetDataSize()/wSmpSize - residual.size();
        if (nDuration > 0)
        {
            residual.insert(residual.end(), nDuration, 0);
        }
    }
}

static double InterpolateWeight(double dLocation, double dBreakPoint, double dEndPoint)
{
    // second order
    if (dLocation < dBreakPoint)
    {
        return 1.0 - 0.5*(dLocation / dBreakPoint)*(dLocation / dBreakPoint);
    }
    else
    {
        return 0.5* (dEndPoint - dLocation) / (dEndPoint - dBreakPoint) * (dEndPoint - dLocation) / (dEndPoint - dBreakPoint);
    }

    // simple linear for now
    if (dLocation < dBreakPoint)
    {
        return 1.0 - 0.5*(dLocation / dBreakPoint);
    }
    else
    {
        return 0.5* (dEndPoint - dLocation) / (dEndPoint - dBreakPoint);
    }
}

static void WeightChars(CIpaVTChar & cPrev, CIpaVTChar & cNext, double dWeight, CIpaVTChar & cThis)
{
#define WeightParameter(P) ((cThis. ## P) = ((cPrev. ## P)*dWeight + (cNext. ## P)*(1 - dWeight)))

    unsigned int i;

    for (i=0; i<cThis.m_areas.size(); i++)
    {
        WeightParameter(m_areas[i]);
    }

    for (i=0; i<cThis.m_reflection.size(); i++)
    {
        WeightParameter(m_reflection[i]);
    }

    for (i=0; i<cThis.m_pred.size(); i++)
    {
        WeightParameter(m_pred[i]);
    }

    // These parameters will need work
    WeightParameter(m_dFrameEnergy);
    WeightParameter(m_dVTGain);
    WeightParameter(m_stimulus.AV);
    WeightParameter(m_stimulus.AH);
    WeightParameter(m_stimulus.AF);
    WeightParameter(m_stimulus.VHX);

#undef WeightParameter
}

static void Interpolate(CIpaVTCharVector & cInterpolated, int nPrevious, double dBreakPoint, int nNext)
{
    double breakTime = 0;
    for (int nLocation = nPrevious; nLocation < dBreakPoint; nLocation++)
    {
        breakTime += cInterpolated[nLocation].m_duration;
    }

    double endTime = 0;
    for (int nLocation = nPrevious; nLocation < nNext; nLocation++)
    {
        endTime += cInterpolated[nLocation].m_duration;
    }

    double time = 0;
    for (int nLocation = nPrevious + 1; nLocation < nNext; nLocation++)
    {
        time += cInterpolated[nLocation].m_duration/2.;
        double dWeight = InterpolateWeight(time, breakTime, endTime);

        WeightChars(cInterpolated[nPrevious],cInterpolated[nNext], dWeight, cInterpolated[nLocation]);
        time += cInterpolated[nLocation].m_duration/2.;
    }
}

void CDlgVocalTract::OnBlendSegments(int nSrc, CFlexEditGrid & cGrid)
{
    CIpaVTCharVector cSegments;
    CIpaVTCharVector cInterpolated;
    ParseParameterGrid(nSrc, cSegments);

    cInterpolated.reserve(cSegments.size()*8); // estimate 8x growth from unblended

    for (unsigned int nIndex = 0; nIndex < cSegments.size(); nIndex++)
    {
        if (cSegments[nIndex].m_stimulus.AH == 0. &&
                cSegments[nIndex].m_stimulus.AF == 0. &&
                (cSegments[nIndex].m_stimulus.AV == 0. || cSegments[nIndex].m_stimulus.Pitch == 0.))
        {
            // Silence
            CIpaVTChar value = cSegments[nIndex > 0 ? nIndex - 1 : nIndex];
            // Insert end marker to prevent blending amlitudes
            value.m_ipa = cSegments[nIndex].m_ipa;
            value.m_duration = 0;
            cInterpolated.push_back(value);
            // Sudden transition to silence
            value.m_stimulus = cSegments[nIndex].m_stimulus;
            cInterpolated.push_back(value);
            value.m_duration = cSegments[nIndex].m_duration/2;
            cInterpolated.push_back(value);
            value.m_duration = 0;
            cInterpolated.push_back(value);

            // sudden transition to new formants during silence midpoint
            value = cSegments[nIndex + 1 < cSegments.size() ? nIndex + 1 : nIndex];
            value.m_ipa = cSegments[nIndex].m_ipa;
            value.m_duration = 0;
            value.m_stimulus = cSegments[nIndex].m_stimulus;
            cInterpolated.push_back(value);
            value.m_duration = cSegments[nIndex].m_duration/2;
            cInterpolated.push_back(value);
            value.m_duration = 0;
            cInterpolated.push_back(value);
            // sudden transition to new amplitude
            value.m_stimulus = cSegments[nIndex + 1 < cSegments.size() ? nIndex + 1 : nIndex].m_stimulus;
            cInterpolated.push_back(value);
        }
        else
        {
            CIpaVTChar newValue(cSegments[nIndex]);

            double dLength = cSegments[nIndex].m_duration/2;
            double desiredLength = 5; // aim for 5ms segments

            newValue.m_duration = dLength/int(dLength/desiredLength);
            for (int replicas = int(dLength/desiredLength); replicas > 0; replicas--)
            {
                cInterpolated.push_back(newValue);
            }

            newValue.m_duration = 0;
            cInterpolated.push_back(newValue);

            newValue.m_duration = dLength/int(dLength/desiredLength);
            for (int replicas = int(dLength/desiredLength); replicas > 0; replicas--)
            {
                cInterpolated.push_back(newValue);
            }
        }
    }


    int nLastMarker = 0;
    double dBreakPoint= 0;
    CString szPrevious;
    for (unsigned int nIndex = 0; nIndex < cInterpolated.size(); nIndex++)
    {
        if (szPrevious != cInterpolated[nIndex].m_ipa)
        {
            dBreakPoint = nIndex - 0.5;
            szPrevious = cInterpolated[nIndex].m_ipa;
        }

        if (cInterpolated[nIndex].m_duration == 0)
        {
            if (dBreakPoint >= nLastMarker && nIndex)
            {
                // Interpolate between markers
                Interpolate(cInterpolated, nLastMarker, dBreakPoint, nIndex);
            }
            nLastMarker = nIndex;
        }
    }

    PopulateParameterGrid(cGrid, cInterpolated);
}

void CDlgVocalTract::ParseParameterGrid(CFlexEditGrid & cGrid, int column, CIpaVTChar & columnChar)
{
    columnChar.m_ipa.Empty();
    columnChar.m_ipa = cGrid.GetTextMatrix(rowIpa,column);

    if (columnChar.m_ipa.IsEmpty())
    {
        return;
    }

    const int VTAreas = getRow(rAreaLast) - getRow(rAreaFirst);
    columnChar.m_areas.reserve(VTAreas);
    for (int i = 0; i < VTAreas; i++)
    {
        CString szValue;
        double dValue = 1.;

        szValue = cGrid.GetTextMatrix(i + getRow(rAreaFirst),column);
        swscanf_s(szValue, _T("%lf"), &dValue);
        columnChar.m_areas.push_back(dValue);
    }

    const int VTReflections = getRow(rReflectionLast) - getRow(rReflectionFirst);
    columnChar.m_reflection.reserve(VTReflections);
    for (int i = 0; i < VTReflections; i++)
    {
        CString szValue;
        double dValue = 0.;

        szValue = cGrid.GetTextMatrix(i + getRow(rReflectionFirst),column);
        swscanf_s(szValue, _T("%lf"), &dValue);
        columnChar.m_reflection.push_back(dValue);
    }

    const int VTPreds = getRow(rPredLast) - getRow(rPredFirst);
    columnChar.m_pred.reserve(VTPreds);
    for (int i = 0; i < VTPreds; i++)
    {
        CString szValue;
        double dValue = 0.;

        szValue = cGrid.GetTextMatrix(i + getRow(rPredFirst),column);
        swscanf_s(szValue, _T("%lf"), &dValue);
        columnChar.m_pred.push_back(dValue);
    }
    if (TRUE)
    {
        CString value;

        columnChar.m_duration = 0;
        value = cGrid.GetTextMatrix(rowDuration,column);
        int nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_duration);

        columnChar.m_stimulus.Pitch = 0;
        value = cGrid.GetTextMatrix(rowPitch,column);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_stimulus.Pitch);

        columnChar.m_dFrameEnergy = 0;
        value = cGrid.GetTextMatrix(rowFEnergy,column);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_dFrameEnergy);

        columnChar.m_stimulus.AV = 0;
        value = cGrid.GetTextMatrix(rowAV,column);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_stimulus.AV);

        columnChar.m_stimulus.AH = 0;
        value = cGrid.GetTextMatrix(rowAH,column);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_stimulus.AH);

        columnChar.m_stimulus.AF = 0;
        value = cGrid.GetTextMatrix(rowAF,column);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_stimulus.AF);

        columnChar.m_stimulus.VHX = 0;
        value = cGrid.GetTextMatrix(rowVHX,column);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_stimulus.VHX);

        columnChar.m_dVTGain = 0;
        value = cGrid.GetTextMatrix(rowVTGain,column);
        nScanned = swscanf_s(value, _T("%lf"), &columnChar.m_dVTGain);
    }
}

void CDlgVocalTract::ParseParameterGrid(int nGrid, CIpaVTCharVector & cChars)
{
    ParseParameterGrid(m_cGrid[nGrid], cChars);
}

void CDlgVocalTract::ParseParameterGrid(CFlexEditGrid & cGrid, CIpaVTCharVector & cChars)
{
    cChars.clear();

    cChars.reserve(cGrid.GetCols(0));
    for (int column = columnFirst; column < cGrid.GetCols(0); column++)
    {
        CIpaVTChar columnChar;

        ParseParameterGrid(cGrid, column, columnChar);

        if (columnChar.m_ipa.IsEmpty())
        {
            continue;
        }

        cChars.push_back(columnChar); // add to end of list
    }
}

void CDlgVocalTract::OnSynthesize()
{
    CIpaVTCharVector cChars;

    //  ParseConstantsGrid(kConstants, m_cConstants);
    ParseParameterGrid(m_nSelectedMethod, cChars);

    if (cChars.begin() == cChars.end())
    {
        return;
    }

    if (m_szSynthesizedFilename.IsEmpty())
    {
        // create temp filename for synthesized waveform
        GetTempFileName(_T("lvt"), m_szSynthesizedFilename.GetBuffer(_MAX_PATH), _MAX_PATH);
        m_szSynthesizedFilename.ReleaseBuffer();
    }

    SynthesizeWave(m_szSynthesizedFilename, cChars);
}


PCMWAVEFORMAT CDlgVocalTract::pcmWaveFormat()
{
    PCMWAVEFORMAT pcm;

    pcm.wBitsPerSample = 16;
    pcm.wf.wFormatTag = 1;  // PCM
    pcm.wf.nChannels = 1; // mono

    pcm.wf.nSamplesPerSec = m_dwSampleRate;

    pcm.wf.nBlockAlign = (unsigned short)(pcm.wf.nChannels*(pcm.wBitsPerSample/8));
    pcm.wf.nAvgBytesPerSec = pcm.wf.nSamplesPerSec * pcm.wf.nBlockAlign;

    return pcm;
}

static double RandOneRms()
{
    double randOnePeakPeak = double(rand()*2 - RAND_MAX)/RAND_MAX;

    return 1.5*randOnePeakPeak*rand()/RAND_MAX;
}

static double Impulse(double dImpulsePhase, int nHarmonics)
{
    double pulse = 0;
    double alt = 1;

    for (int i = 1; i < nHarmonics; i++)
    {
        pulse += alt*sin(i*dImpulsePhase);
    }

    return pulse;
}

// Preemphasis delay as defined in the LPC code
#define DECAY  0.95
BOOL CDlgVocalTract::SynthesizeDataChunk(HMMIO hmmioFile, PCMWAVEFORMAT pcm, CIpaVTCharVector & cChars)
{
    const double pi = 3.14159265358979323846264338327950288419716939937511;

    if (cChars.size() == 0)
    {
        return TRUE;
    }

    BOOL artificialStimulus = m_bArtificialStimulus;

    // synthesize wavefile
    std::vector<double> data;
    CAcousticTube  vt(cChars[0].m_areas, cChars[0].m_reflection);
    double PreEmphasisDenominator[] = { 1, -DECAY };
    CZTransform inversePreEmphasis(1,NULL, PreEmphasisDenominator);

    srand(1); // Initialize to a constant so synthesis is reproducible

    data.reserve(residual.size());
    double nSamples = 0;
    double dVoice = 0;
    double dImpulsePhase = -pi;
    double dAspOpen = 10;
    double dAspClose = 0;
    for (unsigned int i = 0; i < cChars.size(); i++)
    {
        int nSections = cChars[i].m_areas.size();
        vt.SetTransform(cChars[i].m_areas, cChars[i].m_reflection);
        double AH = cChars[i].m_stimulus.AH > 0 ? pow(10, cChars[i].m_stimulus.AH/20) : 0;
        double AF = cChars[i].m_stimulus.AF > 0 ? pow(10, cChars[i].m_stimulus.AF/20) : 0;
        double AV = cChars[i].m_stimulus.AV > 0 ? pow(10, cChars[i].m_stimulus.AV/20) : 0;
        double dVoicePitch = cChars[i].m_stimulus.Pitch;
        double dImpulsePhaseChange = 2*pi*dVoicePitch/pcm.wf.nSamplesPerSec;
        int nHarmonics = int(pcm.wf.nSamplesPerSec/cChars[i].m_stimulus.Pitch/2) - 2;


        // Change from amplitude of impulse to amplitude of single harmonic of impulse train
        double AVSH = AV/pow(nHarmonics*0.5, 0.5);

        CButterworth artificialStimulusAspiration;
        CButterworth artificialStimulusVoice;

        if (artificialStimulus && cChars[i].m_stimulus.VHX)
        {
            artificialStimulusVoice.LowPass(2, cChars[i].m_stimulus.VHX);
            artificialStimulusVoice.ConfigureProcess(pcm.wf.nSamplesPerSec);
            artificialStimulusAspiration.HighPass(2, cChars[i].m_stimulus.VHX);
            artificialStimulusAspiration.ConfigureProcess(pcm.wf.nSamplesPerSec);

            for (int j = int(10*pcm.wf.nSamplesPerSec/cChars[i].m_stimulus.VHX); j>0; j--)
            {
                artificialStimulusAspiration.ForwardTick(RandOneRms());
                artificialStimulusVoice.ForwardTick(Impulse(dImpulsePhase - j*dImpulsePhaseChange, nHarmonics));
            }
        }

        nSamples += cChars[i].m_duration*pcm.wf.nSamplesPerSec/1000;
        while (data.size() < nSamples)
        {
            double stimulus = 0;

            if (artificialStimulus)
            {
                stimulus += AF*RandOneRms();

                if (AH)
                {
                    stimulus += 0.1*AH*artificialStimulusAspiration.ForwardTick(RandOneRms());

                    if (dAspOpen < data.size() && cChars[i].m_stimulus.Pitch)
                    {
                        stimulus += 1.4*AH*artificialStimulusAspiration.ForwardTick(RandOneRms());
                    }
                }

                if (dAspClose <= data.size())
                {
                    if (cChars[i].m_stimulus.Pitch)
                    {
                        dAspClose += pcm.wf.nSamplesPerSec/cChars[i].m_stimulus.Pitch;
                        dAspOpen = dAspClose - 0.5*pcm.wf.nSamplesPerSec/cChars[i].m_stimulus.Pitch;
                    }
                    else
                    {
                        dAspClose = nSamples;
                        dAspOpen = dAspOpen + 10;
                    }
                }

                if (dVoice <= data.size())
                {
                    if (dVoicePitch)
                    {
                        if (dImpulsePhase >= pi)
                        {
                            dImpulsePhase -= 2*pi;
                        }
                    }
                    else if (dImpulsePhase >= pi)
                    {
                        dVoice = nSamples;
                        dImpulsePhase = pi;  // as soon as we are commanded to begin voicing start
                    }

                    if (dVoicePitch > 0)
                    {
                        double dImpulse = artificialStimulusVoice.ForwardTick(Impulse(dImpulsePhase, nHarmonics));
                        //            double dImpulse = impulse(dImpulsePhase, nHarmonics);

                        dImpulse *= 0.50*(1+cos(dImpulsePhase));


                        stimulus += AVSH*dImpulse;

                        dImpulsePhase += dImpulsePhaseChange;
                    }
                }
            }
            else
            {
                unsigned dataSize = data.size();
                stimulus = residual[dataSize];
            }

            if (m_bPreEmphasize)
            {
                // Handle preemphasis here... all residue must have reverse preemphasis applied...
                stimulus = inversePreEmphasis.Tick(stimulus);
            }

            double dOutput = 0;

            switch (m_nStructure)
            {
            case DIRECT:
            {
                dOutput = stimulus;

                for (int j = 0; j < nSections; j++)
                {
                    if (data.size() > (unsigned) j)
                    {
                        dOutput += cChars[i].m_pred[j]*data[data.size()-1-j];
                    }
                }
            }
            break;
            case RESIDUAL_ERROR:
                dOutput = stimulus;
                break;
            case VTGAIN:
                dOutput = stimulus * 0.5 * pow(10, cChars[i].m_dVTGain/20);
                break;
            case SQ_ERROR:
                dOutput = stimulus * stimulus * 0.5/10 * pow(10, (90. - 2*(cChars[i].m_dFrameEnergy - cChars[i].m_dVTGain))/20);
                break;
            default:
                dOutput = vt.Tick(stimulus, m_nStructure == LATTICE, TRUE);
            }

            data.push_back(dOutput);
        }
    }

    if (m_nTilt == DB12 || m_nTilt == DB6)
    {
        // Apply radiation and glottal spectral shape
        CButterworth filter(NULL);
        double FC = 20;
        double HP = 40;
        double NF = pcm.wf.nSamplesPerSec/2.;
        double dScale0to12 = sqrt(NF/(NF/2/(1+(NF*NF)/(FC*FC)) + FC/2*atan2(NF,FC) - FC/4 - FC/2*atan2(FC,FC)));
        double dScale0to6 = 0.7*sqrt((NF-FC)/(FC*atan2(NF,FC) - FC*atan2(FC,FC)));

        double dScale = m_nTilt == DB6 ? -1 : 1;
        if (!m_bPreEmphasize)
        {
            dScale *= (m_nTilt == DB6) ? dScale0to6 : dScale0to12;
        }
        else
        {
            dScale *= dScale0to12/dScale0to6;
        }


        filter.BandPass(1, (FC + HP)/2, (FC - HP));
        filter.ConfigureProcess(NF*2);

        CZTransform zReverse = filter.GetForward();
        for (unsigned int i = data.size(); i>0; i--)   // Apply filter in reverse to get better phase estimate
        {
            data[i-1] = zReverse.Tick(dScale*data[i-1]);
        }

        if (m_nTilt == DB12)
        {
            CZTransform zForward = filter.GetForward();
            for (unsigned int i = 0; i < data.size(); i++)
            {
                data[i] = zForward.Tick(data[i]);
            }
        }
    }

    if (m_bMirror)
    {
        std::vector<double> mirror;

        mirror.reserve(data.size());

        for (unsigned int i = data.size(); i>0; i--)   // Mirror waveform
        {
            mirror.push_back(data[i-1]);
        }

        mirror.swap(data);
    }

    std::vector<SHORT> shortData;
    shortData.reserve(data.size());
    for (unsigned int i = 0; i < data.size(); i++)   // Convert samples to double
    {
        shortData.push_back((SHORT) min(max(data[i],-0x8000), 0x7fff));
    }

    return mmioWrite(hmmioFile, (char *) &shortData[0], sizeof(SHORT)*data.size());
}

BOOL CDlgVocalTract::SynthesizeWave(LPCTSTR pszPathName, CIpaVTCharVector & cChars)
{
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application

    // open file
    HMMIO hmmioFile; // file handle
    hmmioFile = mmioOpen((LPTSTR)pszPathName, NULL, MMIO_READWRITE | MMIO_EXCLUSIVE);
    if (!hmmioFile)
    {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_FILEOPEN, pszPathName);
        return FALSE;
    }

    /* Create the output file RIFF chunk of form type 'WAVE'.
    */
    MMCKINFO mmckinfoParent;  // chunk info. for output RIFF chunk
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioCreateChunk(hmmioFile, &mmckinfoParent, MMIO_CREATERIFF) != 0)
    {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    /* We are now descended into the 'RIFF' chunk we just created.
    * Now create the 'fmt ' chunk. Since we know the size of this chunk,
    * specify it in the MMCKINFO structure so MMIO doesn't have to seek
    * back and set the chunk size after ascending from the chunk.
    */
    MMCKINFO       mmckinfoSubchunk;      // info. for a chunk in output file
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    mmckinfoSubchunk.cksize = sizeof(PCMWAVEFORMAT);  // we know the size of this ck.
    if (mmioCreateChunk(hmmioFile, &mmckinfoSubchunk, 0) != 0)
    {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    /* Write the PCMWAVEFORMAT structure to the 'fmt ' chunk.
    */
    PCMWAVEFORMAT pcm = pcmWaveFormat();
    if (mmioWrite(hmmioFile, (HPSTR) &pcm, sizeof(PCMWAVEFORMAT))
            != sizeof(PCMWAVEFORMAT))
    {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    /* Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk.
    */
    if (mmioAscend(hmmioFile, &mmckinfoSubchunk, 0) != 0)
    {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    /* Create the 'data' chunk that holds the waveform samples.
    */
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioCreateChunk(hmmioFile, &mmckinfoSubchunk, 0) != 0)
    {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    mmckinfoSubchunk.dwFlags = MMIO_DIRTY; // force ascend to update the chunk length
    mmckinfoParent.dwFlags = MMIO_DIRTY;

    SynthesizeDataChunk(hmmioFile, pcm, cChars);

    // At file end what is file position
    MMIOINFO mmioinfo;
    if (mmioGetInfo(hmmioFile,&mmioinfo,0))
    {
        // error writing RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    // get out of 'data' chunk
    if (mmioAscend(hmmioFile, &mmckinfoSubchunk, 0))
    {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    // get out of 'RIFF' chunk, to write RIFF size
    if (mmioAscend(hmmioFile, &mmckinfoParent, 0))
    {
        // error writing RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }
    if (!mmioClose(hmmioFile, 0))   // close file
    {
        // Set File Length ...
        // SDM 1.5Test10.2
        CFile WaveFile(pszPathName,CFile::modeReadWrite);
        //    WaveFile.SetLength(mmioinfo.lDiskOffset+2);
    }

    return TRUE;
}

// This function labels the document with the grid parameters
void CDlgVocalTract::LabelDocument(CSaDoc * pDoc)
{
    CIpaVTCharVector cChars;
    ParseParameterGrid(m_nSelectedMethod, cChars);

    CMusicPhraseSegment * pIndexSeg = (CMusicPhraseSegment *)pDoc->GetSegment(MUSIC_PL1);
    CPhoneticSegment * pCharSeg = (CPhoneticSegment *)pDoc->GetSegment(PHONETIC);

    double SR = pcmWaveFormat().wf.nSamplesPerSec;
    double elapsedTime = 0;
    double labelTime = 0;
    CSaString szIPA;
    double lastCharStopTime = 0;
    for (unsigned int i = 0; i < cChars.size(); i++)
    {
        if (labelTime <= elapsedTime)
        {
            const double minLabelTime = 0.005;
            DWORD dwStart = DWORD(elapsedTime*SR+0.5)*2;
            double length = minLabelTime > cChars[i].m_duration/1000. ? minLabelTime : cChars[i].m_duration/1000.;
            DWORD dwDuration = DWORD(length*SR+0.5)*2;

            CSaString szIndex;
            szIndex.Format(_T("%d"),i+1);
            pIndexSeg->Insert(pIndexSeg->GetOffsetSize(), szIndex, true, dwStart, dwDuration);

            labelTime = elapsedTime + minLabelTime;
        }

        if (szIPA != cChars[i].m_ipa || i == cChars.size() - 1)
        {
            DWORD dwStart = DWORD(lastCharStopTime*SR+0.5)*2;
            double length = elapsedTime - lastCharStopTime;
            DWORD dwDuration = DWORD(length*SR+0.5)*2;

            if (dwDuration)
            {
                pCharSeg->Insert(pCharSeg->GetOffsetSize(), szIPA, true, dwStart, dwDuration);
            }
            szIPA = cChars[i].m_ipa;

            lastCharStopTime += length;
        }

        elapsedTime += cChars[i].m_duration/1000.;
    }
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *) pDoc->GetNextView(pos);

    pView->PostMessage(WM_COMMAND, ID_PHRASE_L1_RAWDATA);
}

void CDlgVocalTract::OnPlayBoth()
{
    OnPlaySynth();
    OnPlaySource();
}

void CDlgVocalTract::OnPlaySynth()
{
    OnSynthesize();
    if (m_szSynthesizedFilename.GetLength())
    {
        PlaySound(m_szSynthesizedFilename, 0, SND_SYNC | SND_NODEFAULT | SND_FILENAME);
    }
}

void CDlgVocalTract::OnPlaySource()
{
    if (m_szSourceFilename.GetLength())
    {
        PlaySound(m_szSourceFilename, 0, SND_SYNC | SND_NODEFAULT | SND_FILENAME);
    }
}

void CDlgVocalTract::OnFileOpen()
{
    CFileDialog dlg(TRUE, _T("txt"), _T("*.txt"), OFN_FILEMUSTEXIST, _T("Text Files (*.txt)|*.txt|Labeled Grid Files (*.grd)|*.grd||"));

    if (dlg.DoModal() == IDOK)
    {
        CFile data(dlg.GetPathName(), CFile::modeRead | CFile::shareExclusive | CFile::typeBinary);

        CString szData;

        UINT uRead;

        do
        {
            char buf[1024];
            uRead = data.Read(buf, sizeof(buf)-1);

            buf[uRead] = 0;

            szData += buf;
        }
        while (uRead);

        CString szExt(dlg.GetFileExt());
        szExt.MakeUpper();
        m_cGrid[m_nSelectedView].ClearRange(rowFirst, columnFirst, getRow(rLast), m_cGrid[m_nSelectedView].GetCols(0) - 1);
        if (szExt == "GRD")
        {
            m_cGrid[m_nSelectedView].LoadRange(0, 0, szData, FALSE);
        }
        else
        {
            m_cGrid[m_nSelectedView].LoadRange(rowFirst, columnFirst, szData, TRUE);
        }
    }
}

void CDlgVocalTract::OnUpdateFileOpen(CCmdUI *)
{
}

void CDlgVocalTract::OnFileSaveAs()
{
    CFileDialog dlg(FALSE, _T("txt"), m_szGrid[m_nSelectedView], OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Text Files (*.txt)|*.txt|Labeled Grid Files (*.grd)|*.grd||"));

    if (dlg.DoModal() == IDOK)
    {
        CFile data(dlg.GetPathName(), CFile::modeWrite | CFile::shareExclusive | CFile::typeBinary | CFile::modeCreate);

        CString szData;
        CString szExt(dlg.GetFileExt());
        szExt.MakeUpper();
        if (szExt == "GRD")
        {
            // skip column numbers since they number entire grid, we need size of user area
            szData = m_cGrid[m_nSelectedView].SaveRange(rowFirst, 0, m_cGrid[m_nSelectedView].GetRows(), m_cGrid[m_nSelectedView].GetCols(0), FALSE);
            int nColumnsMax = 1;
            int nColumns = 1;

            for (int i = 0; i < szData.GetLength(); i++)
            {
                if (szData[i] == _T('\t'))
                {
                    nColumns++;
                    if (nColumns > nColumnsMax)
                    {
                        nColumnsMax = nColumns;
                    }
                }
                else if (szData[i] == _T('\n'))
                {
                    nColumns = 1;
                }
            }
            szData = m_cGrid[m_nSelectedView].SaveRange(0, 0, m_cGrid[m_nSelectedView].GetRows(), nColumnsMax, FALSE);
        }
        else
        {
            szData = m_cGrid[m_nSelectedView].SaveRange(rowFirst, columnFirst, m_cGrid[m_nSelectedView].GetRows(), m_cGrid[m_nSelectedView].GetCols(0), TRUE);
        }

        data.Write(szData,szData.GetLength());
    }
}

// ****************************************** //
void CDlgVocalTract::OnEditCopy()
{
    m_cGrid[m_nSelectedView].OnEditCopy();
}

void CDlgVocalTract::OnEditClear()
{
    m_cGrid[m_nSelectedView].OnEditClear();
}

void CDlgVocalTract::OnEditCut()
{
    m_cGrid[m_nSelectedView].OnEditCut();
}

void CDlgVocalTract::OnEditPaste()
{
    m_cGrid[m_nSelectedView].OnEditPaste();
}

void CDlgVocalTract::OnUpdateEditPaste(CCmdUI * pCmdUI)
{
    m_cGrid[m_nSelectedView].OnUpdateEditPaste(pCmdUI);
}

void CDlgVocalTract::OnUpdateEditCopy(CCmdUI * pCmdUI)
{
    m_cGrid[m_nSelectedView].OnUpdateEditCopy(pCmdUI);
}

void CDlgVocalTract::OnUpdateEditCut(CCmdUI * pCmdUI)
{
    m_cGrid[m_nSelectedView].OnUpdateEditCut(pCmdUI);
}

void CDlgVocalTract::OnUpdateEditClear(CCmdUI * pCmdUI)
{
    m_cGrid[m_nSelectedView].OnUpdateEditClear(pCmdUI);
}


void CDlgVocalTract::OnArtificial()
{
    m_bArtificialStimulus = !m_bArtificialStimulus;
}

void CDlgVocalTract::OnUpdateArtificial(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_bArtificialStimulus);
}

void CDlgVocalTract::OnWindow()
{
    m_bWindowLPC = !m_bWindowLPC;
}

void CDlgVocalTract::OnUpdateWindow(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_bWindowLPC);
}

void CDlgVocalTract::OnLosslessTube()
{
    m_nStructure = LOSSLESS_TUBE;
}

void CDlgVocalTract::OnUpdateLosslessTube(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nStructure == LOSSLESS_TUBE);
}

void CDlgVocalTract::OnLattice()
{
    m_nStructure = LATTICE;
}

void CDlgVocalTract::OnUpdateLattice(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nStructure == LATTICE);
}

void CDlgVocalTract::OnDirect()
{
    m_nStructure = DIRECT;
}

void CDlgVocalTract::OnUpdateDirect(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nStructure == DIRECT);
}

void CDlgVocalTract::OnResidual()
{
    m_nStructure = RESIDUAL_ERROR;
}

void CDlgVocalTract::OnUpdateResidual(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nStructure == RESIDUAL_ERROR);
}

void CDlgVocalTract::OnVTGain()
{
    m_nStructure = VTGAIN;
}

void CDlgVocalTract::OnUpdateVTGain(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nStructure == VTGAIN);
}

void CDlgVocalTract::OnSqError()
{
    m_nStructure = SQ_ERROR;
}

void CDlgVocalTract::OnUpdateSqError(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nStructure == SQ_ERROR);
}

void CDlgVocalTract::OnCovariance()
{
    m_nMethod = LPC_COVAR_LATTICE;
}

void CDlgVocalTract::OnUpdateCovariance(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nMethod == LPC_COVAR_LATTICE);
}

void CDlgVocalTract::OnAutocor()
{
    m_nMethod = LPC_AUTOCOR;
}

void CDlgVocalTract::OnUpdateAutocor(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nMethod == LPC_AUTOCOR);
}

void CDlgVocalTract::OnOrder()
{
    CDlgVTOrder dlgOrder;

    dlgOrder.m_nOrder = m_nRequestedOrder;

    if (dlgOrder.DoModal() == IDOK)
    {
        m_nRequestedOrder = dlgOrder.m_nOrder;
    }
}



void CDlgVocalTract::On0dB()
{
    m_nTilt = DB0;
}

void CDlgVocalTract::OnUpdate0dB(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nTilt == DB0);
}

void CDlgVocalTract::On6dB()
{
    m_nTilt = DB6;
}

void CDlgVocalTract::OnUpdate6dB(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nTilt == DB6);
}

void CDlgVocalTract::On12dB()
{
    m_nTilt = DB12;
}

void CDlgVocalTract::OnUpdate12dB(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nTilt == DB12);
}


void CDlgVocalTract::OnPreemphasize()
{
    m_bPreEmphasize = !m_bPreEmphasize;
}

void CDlgVocalTract::OnUpdatePreemphasize(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_bPreEmphasize);
}

void CDlgVocalTract::OnMirror()
{
    m_bMirror = !m_bMirror;
}

void CDlgVocalTract::OnUpdateMirror(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_bMirror);
}



void CDlgVocalTract::OnClosedPhase()
{
    m_bClosedPhase = !m_bClosedPhase;
}

void CDlgVocalTract::OnUpdateClosedPhase(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_bClosedPhase);
}

void CDlgVocalTract::OnAdjustCells()
{
    CDlgSynthesisAdjustCells dlg;

    if (dlg.DoModal() == IDOK)
    {
        long rowEnd = m_cGrid[m_nSelectedView].GetRowSel();
        long colEnd = m_cGrid[m_nSelectedView].GetColSel();
        long row = m_cGrid[m_nSelectedView].GetRow();
        long col = m_cGrid[m_nSelectedView].GetCol();

        for (long y = row; y <= rowEnd; y++)
        {
            for (long x = col; x <= colEnd; x++)
            {
                CString szValue = m_cGrid[m_nSelectedView].GetTextMatrix(y, x);
                double value;

                if (!szValue.IsEmpty() && swscanf_s(szValue, _T("%lf"), &value))
                {
                    CString szNewValue;
                    szNewValue.Format(_T("%.5g"), dlg.m_dScale*value + dlg.m_dOffset);
                    m_cGrid[m_nSelectedView].SetTextMatrix(y, x, szNewValue);
                }
            }
        }
    }
}

void CDlgVocalTract::OnFragments()
{
    m_nSelectedMethod = kFragment;

    if (m_nSelectedView != kFragment)
    {
        ShowGrid(m_nSelectedMethod);
    }
}

void CDlgVocalTract::OnUpdateFragments(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nSelectedView == kFragment);
}

void CDlgVocalTract::OnIpa()
{
    m_nSelectedMethod = kSegment;

    if (m_nSelectedView != kSegment)
    {
        ShowGrid(m_nSelectedMethod);
    }
}

void CDlgVocalTract::OnUpdateIpa(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nSelectedView == kSegment);
}


void CDlgVocalTract::OnIpaBlend()
{
    m_nSelectedMethod = kIpaBlended;

    if (m_nSelectedView != kIpaBlended)
    {
        ShowGrid(m_nSelectedMethod);
    }
}

void CDlgVocalTract::OnUpdateIpaBlend(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nSelectedView == kIpaBlended);
}

void CDlgVocalTract::OnIpaDefaults()
{
    if (m_nSelectedView != kDefaults)
    {
        ShowGrid(kDefaults);
    }
    else
    {
        ShowGrid(m_nSelectedMethod);
    }
}

void CDlgVocalTract::OnUpdateIpaDefaults(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_nSelectedView == kDefaults);
}



void CDlgVocalTract::OnDestroy()
{
    ParseParameterGrid(kDefaults, m_cDefaults);

    CFrameWnd::OnDestroy();
}

void CDlgVocalTract::OnSynthHide()
{
    m_bMinimize = !m_bMinimize;
}

void CDlgVocalTract::OnUpdateSynthHide(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_bMinimize);
}


