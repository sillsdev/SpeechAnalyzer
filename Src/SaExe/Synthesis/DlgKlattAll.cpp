// DlgKlattAll.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include <ctype.h>
#include "..\sa.h"
#include "sa_doc.h"
#include "Process\sa_proc.h"
#include "sa_segm.h"
#include "sa_view.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_spi.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_zcr.h"
#include "DlgKlattAll.h"
#include "dsp\mathx.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#import <mshflxgd.ocx>

/////////////////////////////////////////////////////////////////////////////
// CDlgSynthesisAdjustCells dialog


CDlgSynthesisAdjustCells::CDlgSynthesisAdjustCells(CWnd* pParent /*=NULL*/, double dScale, double dOffset)
: CDialog(CDlgSynthesisAdjustCells::IDD, pParent)
{
	m_dScale = dScale;
	m_dOffset = dOffset;
	//{{AFX_DATA_INIT(CDlgSynthesisAdjustCells)
	//}}AFX_DATA_INIT
}


void CDlgSynthesisAdjustCells::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSynthesisAdjustCells)
	DDX_Text(pDX, IDC_SCALE, m_dScale);
	DDX_Text(pDX, IDC_OFFSET, m_dOffset);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSynthesisAdjustCells, CDialog)
	//{{AFX_MSG_MAP(CDlgSynthesisAdjustCells)
	// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSynthesisAdjustCells message handlers

/////////////////////////////////////////////////////////////////////////////
// CDlgKlattAll dialog

double dKlattFrameIntervalMs = 10;
UINT nKlattFrameIntervalFragments = 0;
double dKlattFrameWindowMs = 25;
UINT nKlattFrameWindowFragments = 0;
BOOL CDlgKlattAll::m_bMinimize = TRUE;
CSaDoc* CDlgKlattAll::m_pShowDoc = NULL;


int CDlgKlattAll::m_nSelectedMethod = CDlgKlattAll::kFragment;
CDlgKlattAll* CDlgKlattAll::m_pDlgSynthesisKlatt = NULL;

CDlgKlattAll::CDlgKlattAll(CWnd* pParent /*=NULL*/, int nSelectedView)
: CFrameWnd()
{
	m_cConstants = (const CKlattConstants) GetGlobalKlattDefaults();
	m_cConstants.SR = 22050;  // override default sampling rate.
	m_cDefaults.Load(GetDefaultsPath());
	if (nSelectedView >= 0)
		m_nSelectedMethod = nSelectedView;
	m_nSelectedView = m_nSelectedMethod;
	m_bGetFragments = (m_nSelectedView == kFragment);
	m_bPitch = TRUE;
	m_bDuration = TRUE;
	m_bIpa = TRUE;
	m_szGrid[kConstants] = _T("Constants.txt");
	m_szGrid[kDefaults] = _T("Defaults.txt");
	m_szGrid[kFragment] = _T("Fragments.txt");
	m_szGrid[kSegment] = _T("IPA Segments.txt");
	m_szGrid[kIpaBlended] = _T("Blended IPA.txt");
	m_dTimeScale = 1.0;
	//{{AFX_DATA_INIT(CDlgKlattAll)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	DWORD dwStyle = 0;

	dwStyle |= WS_POPUPWINDOW;
	dwStyle |= WS_CAPTION;
	dwStyle |= WS_MINIMIZEBOX;
	dwStyle |= WS_MAXIMIZEBOX;
	dwStyle |= WS_THICKFRAME;
	//dwStyle |= WS_MAXIMIZE;

	BOOL bResult = LoadFrame(IDR_KLATT, dwStyle, pParent);

	ASSERT(bResult);

	CRect rc(20,20,400,400); // arbitrary rect
	if (pParent)
		pParent->GetWindowRect(rc);
	rc.DeflateRect(20,20);
	MoveWindow(rc);
	ShowWindow(SW_SHOWMAXIMIZED);
}

CDlgKlattAll::~CDlgKlattAll()
{
	m_cDefaults.Save(GetDefaultsPath());

	// remove synthesized wavefile in SA
	CFileStatus fileStatus; // file status    
	if (!m_szSynthesizedFilename.IsEmpty() && CFile::GetStatus(m_szSynthesizedFilename, fileStatus))
	{
		CFile::Remove(m_szSynthesizedFilename);
	}
}

void CDlgKlattAll::CreateSynthesizer(CWnd* pParent, int nMode)
{
	DestroySynthesizer();
	ASSERT(m_pDlgSynthesisKlatt == NULL);
	m_pDlgSynthesisKlatt = new CDlgKlattAll(pParent, nMode);
}

void CDlgKlattAll::DestroySynthesizer()
{
	if (m_pDlgSynthesisKlatt)
		m_pDlgSynthesisKlatt->DestroyWindow();

	if (m_pDlgSynthesisKlatt)
	{
		delete m_pDlgSynthesisKlatt;
		m_pDlgSynthesisKlatt = NULL;
	}
}

void CDlgKlattAll::OnDestroy() 
{
	ParseParameterGrid(kDefaults, m_cDefaults);

	CFrameWnd::OnDestroy();	
}

void CDlgKlattAll::PostNcDestroy() 
{
	if (m_pDlgSynthesisKlatt)
	{
		delete m_pDlgSynthesisKlatt;
		m_pDlgSynthesisKlatt = NULL;
	}

	CWnd::PostNcDestroy();
}

BEGIN_MESSAGE_MAP(CDlgKlattAll, CFrameWnd)
	//{{AFX_MSG_MAP(CDlgKlattAll)
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_KLATT_CONST, OnKlattConst)
	ON_UPDATE_COMMAND_UI(ID_KLATT_CONST, OnUpdateKlattConst)
	ON_COMMAND(ID_SYNTH_DISPLAY, OnKlattDisplay)
	ON_COMMAND(ID_KLATT_FRAGMENTS, OnKlattFragments)
	ON_UPDATE_COMMAND_UI(ID_KLATT_FRAGMENTS, OnUpdateKlattFragments)
	ON_COMMAND(ID_KLATT_IPA, OnKlattIpa)
	ON_UPDATE_COMMAND_UI(ID_KLATT_IPA, OnUpdateKlattIpa)
	ON_COMMAND(ID_KLATT_IPA_DEFAULTS, OnKlattIpaDefaults)
	ON_UPDATE_COMMAND_UI(ID_KLATT_IPA_DEFAULTS, OnUpdateKlattIpaDefaults)
	ON_COMMAND(ID_KLATT_GET_ALL, OnKlattGetAll)
	ON_COMMAND(ID_PLAY_BOTH, OnPlayBoth)
	ON_COMMAND(ID_PLAY_SYNTH, OnPlaySynth)
	ON_COMMAND(ID_PLAY_ORIGINAL, OnPlaySource)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_CLOSE, OnClose)
	ON_COMMAND(ID_EDIT_SMOOTH, OnSmoothe)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SMOOTH, OnUpdateEditSmoothe)
	ON_UPDATE_COMMAND_UI(ID_CLOSE, OnUpdateClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_KLATT_ELONGATE, OnKlattElongate)
	ON_UPDATE_COMMAND_UI(ID_KLATT_ELONGATE, OnUpdateKlattElongate)
	ON_COMMAND(ID_KLATT_HELP, OnKlattHelp)
	ON_COMMAND(ID_KLATT_IPA_BLEND, OnKlattIpaBlend)
	ON_UPDATE_COMMAND_UI(ID_KLATT_IPA_BLEND, OnUpdateKlattIpaBlend)
	ON_COMMAND(ID_SYNTHESIS_ADJUST_CELLS, OnAdjustCells)
	ON_COMMAND(ID_SYNTH_HIDE, OnSynthHide)
	ON_UPDATE_COMMAND_UI(ID_SYNTH_HIDE, OnUpdateSynthHide)
	ON_COMMAND(ID_SYNTH_SHOW, OnSynthShow)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_SYNTHESIS_KLATT_INTERVAL_0FRAG, ID_SYNTHESIS_KLATT_INTERVAL_1FRAG, OnIntervalNFrag)
	ON_COMMAND_RANGE(ID_SYNTHESIS_KLATT_INTERVAL_0MS, ID_SYNTHESIS_KLATT_INTERVAL_25MS, OnIntervalNMs)
	ON_COMMAND_RANGE(ID_SYNTHESIS_KLATT_WINDOW_0FRAG, ID_SYNTHESIS_KLATT_WINDOW_5FRAG, OnWindowNFrag)
	ON_COMMAND_RANGE(ID_SYNTHESIS_KLATT_WINDOW_0MS, ID_SYNTHESIS_KLATT_WINDOW_25MS, OnWindowNMs)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SYNTHESIS_KLATT_INTERVAL_0FRAG, ID_SYNTHESIS_KLATT_INTERVAL_1FRAG, OnUpdateIntervalNFrag)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SYNTHESIS_KLATT_INTERVAL_0MS, ID_SYNTHESIS_KLATT_INTERVAL_25MS, OnUpdateIntervalNMs)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SYNTHESIS_KLATT_WINDOW_0FRAG, ID_SYNTHESIS_KLATT_WINDOW_5FRAG, OnUpdateWindowNFrag)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SYNTHESIS_KLATT_WINDOW_0MS, ID_SYNTHESIS_KLATT_WINDOW_25MS, OnUpdateWindowNMs)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgKlattAll message handlers

int CDlgKlattAll::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rc(0,0,0,0);
	for(int i=0;i < kGrids;i++)
	{
		m_cGrid[i].Create(NULL, _T(""), WS_VISIBLE,rc,this,i);
		m_cGrid[i].SetBorderStyle(FALSE);
		m_cGrid[i].PreSubclassWindow();
		m_cGrid[i].SetAllowUserResizing(3 /*flexResizeBoth*/);
		m_cGrid[i].FakeArrowKeys(TRUE); // for some reason the arrow keys do not seem to work unless on a dialog ??
	}

	this->LabelGrid(kSegment);
	this->LabelGrid(kIpaBlended);
	this->LabelGrid(kFragment);
	this->LabelGrid(kDefaults);
	this->LabelGrid(kConstants);

	ShowGrid(m_nSelectedMethod);
	PopulateParameterGrid(kDefaults, m_cDefaults);
	OnKlattGetAll();

	return 0;
}

void CDlgKlattAll::OnClose() 
{
	DestroySynthesizer();
}

void CDlgKlattAll::OnUpdateClose(CCmdUI* ) 
{
}

void CDlgKlattAll::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);

	CRect rc;

	this->GetClientRect(rc);
	if (m_cGrid[m_nSelectedView].GetSafeHwnd())
		m_cGrid[m_nSelectedView].MoveWindow(rc);	
}

BOOL CDlgKlattAll::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CDlgKlattAll::OnSetFocus(CWnd* pOldWnd) 
{
	CFrameWnd::OnSetFocus(pOldWnd);

	m_cGrid[m_nSelectedView].SetFocus();
}

void CDlgKlattAll::PopulateParameterGrid(CFlexEditGrid &cGrid, const CIpaCharVector &cChars, BOOL bDuration) 
{
	CIpaCharVector::const_iterator pParm;
	int column = columnFirst;

	for(pParm = cChars.begin(); pParm != cChars.end(); pParm++)
	{
		PopulateParameterGrid(cGrid, *pParm, column, bDuration);
		column++;
	}
}

void CDlgKlattAll::PopulateParameterGrid(CFlexEditGrid &cGrid, const CIpaChar &cChar, int nColumn, BOOL bDuration)
{
	if (nColumn >= cGrid.GetCols(0))
		cGrid.SetCols(0, nColumn + 1);
	cGrid.SetTextMatrix(rowIpa,nColumn, cChar.ipa);
	if (bDuration)
	{
		CString szDuration;
		szDuration.Format(_T("%f"),cChar.duration);
		cGrid.SetTextMatrix(rowDuration,nColumn, szDuration);
	}
	PopulateParameterGrid(cGrid, &cChar.parameters, nColumn);
}

void CDlgKlattAll::PopulateParameterGrid(CFlexEditGrid &cGrid, const TEMPORAL *pParameters, int nColumn)
{
	const PARAMETER_DESC *parameterInfo = GetTemporalKlattDesc();

	if (nColumn >= cGrid.GetCols(0))
		cGrid.SetCols(0, nColumn + 1);
	for(int i=0;parameterInfo[i].parameterOffset != -1; i++)
	{
		int row = i + rowParameters;

		if (row >= cGrid.GetRows())
			cGrid.SetRows(row+1);
		CString value;
		if (parameterInfo[i].typeScanf[1] != 'd')
			value.Format(parameterInfo[i].typeScanf, *(Float*)(((char*)pParameters)+parameterInfo[i].parameterOffset));
		else
			value.Format(parameterInfo[i].typeScanf, *(int*)(((char*)pParameters)+parameterInfo[i].parameterOffset));

		cGrid.SetTextMatrix(row,nColumn, value);
	}
}

void CDlgKlattAll::ShowGrid(int nGrid) 
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
	for(int i=0; i < kGrids; i++)
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
		szText = szText.Left(nStart);
	szText += _T(" - ") + m_szGrid[nGrid];
	SetWindowText(szText);

	if (nGrid == kFragment && !m_bGetFragments)
	{
		m_bGetFragments = TRUE;
		OnKlattGetAll();
	}
}

void CDlgKlattAll::OnKlattDisplay() 
{
	OnSynthesize();
	// open synthesized wavefile in SA
	CFileStatus fileStatus; // file status
	if (CFile::GetStatus(m_szSynthesizedFilename, fileStatus))
	{
		if (fileStatus.m_size)
		{
			// file created open in SA
			CSaApp *pApp = (CSaApp*)(AfxGetApp());

			CSaDoc* pDoc = pApp->OpenWavFileAsNew(m_szSynthesizedFilename);
			m_szSynthesizedFilename.Empty();

			LabelDocument(pDoc);

			if (m_bMinimize)
				ShowWindow(SW_MINIMIZE);
		}
	}
}

void CDlgKlattAll::OnSynthShow() 
{
	CString szSave(m_szSynthesizedFilename);

	m_szSynthesizedFilename  = m_szShowFilename;

	OnSynthesize();
	// open synthesized wavefile in SA
	CFileStatus fileStatus; // file status
	if (CFile::GetStatus(m_szSynthesizedFilename, fileStatus))
	{
		if (fileStatus.m_size)
		{
			// file created open in SA
			CSaApp *pApp = (CSaApp*)(AfxGetApp());

			CSaDoc* pDoc = pApp->IsDocumentOpened(m_pShowDoc) ? m_pShowDoc : NULL;

			if (pDoc && pDoc->GetFileStatus()->m_szFullName != m_szSynthesizedFilename)
				pDoc = NULL;

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
				ShowWindow(SW_MINIMIZE);

			m_szSynthesizedFilename = szSave; 
			m_pShowDoc = pDoc;
		}
	}
}

void CDlgKlattAll::OnKlattFragments() 
{
	m_nSelectedMethod = kFragment;

	if (m_nSelectedView != kFragment)
		ShowGrid(m_nSelectedMethod);
}

void CDlgKlattAll::OnUpdateKlattFragments(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nSelectedView == kFragment);	
}

void CDlgKlattAll::OnKlattIpa() 
{
	m_nSelectedMethod = kSegment;

	if (m_nSelectedView != kSegment)
		ShowGrid(m_nSelectedMethod);
}

void CDlgKlattAll::OnUpdateKlattIpa(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nSelectedView == kSegment);	
}


void CDlgKlattAll::OnKlattIpaBlend() 
{
	m_nSelectedMethod = kIpaBlended;

	if (m_nSelectedView != kIpaBlended)
		ShowGrid(m_nSelectedMethod);
}

void CDlgKlattAll::OnUpdateKlattIpaBlend(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nSelectedView == kIpaBlended);	
}

void CDlgKlattAll::OnKlattIpaDefaults() 
{
	if (m_nSelectedView != kDefaults)
		ShowGrid(kDefaults);
	else
		ShowGrid(m_nSelectedMethod);
}

void CDlgKlattAll::OnUpdateKlattIpaDefaults(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nSelectedView == kDefaults);	
}

void CDlgKlattAll::OnKlattConst() 
{
	if (m_nSelectedView != kConstants)
		ShowGrid(kConstants);
	else
		ShowGrid(m_nSelectedMethod);
}

void CDlgKlattAll::OnUpdateKlattConst(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_nSelectedView == kConstants);	
}

void CDlgKlattAll::OnPlayBoth() 
{
	OnPlaySynth();
	OnPlaySource();
}

void CDlgKlattAll::OnPlaySynth() 
{
	OnSynthesize();
	if (m_szSynthesizedFilename.GetLength())
		PlaySound(m_szSynthesizedFilename, 0, SND_SYNC | SND_NODEFAULT | SND_FILENAME );
}

void CDlgKlattAll::OnPlaySource() 
{
	if (m_szSourceFilename.GetLength())
		PlaySound(m_szSourceFilename, 0, SND_SYNC | SND_NODEFAULT | SND_FILENAME );
}

PCMWAVEFORMAT CDlgKlattAll::pcmWaveFormat()
{
	PCMWAVEFORMAT pcm;

	pcm.wBitsPerSample = 16;
	pcm.wf.wFormatTag = 1;  // PCM
	pcm.wf.nChannels = 1; // mono

	pcm.wf.nSamplesPerSec = m_cConstants.SR;

	pcm.wf.nBlockAlign = (unsigned short)(pcm.wf.nChannels*(pcm.wBitsPerSample/8));
	pcm.wf.nAvgBytesPerSec = pcm.wf.nSamplesPerSec * pcm.wf.nBlockAlign;

	return pcm;
}

BOOL CDlgKlattAll::SynthesizeWave(const TCHAR* pszPathName, CIpaCharVector &cChars)
{
	CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application

	// open file
	HMMIO hmmioFile; // file handle
	hmmioFile = mmioOpen(const_cast<TCHAR*>(pszPathName), NULL, MMIO_READWRITE | MMIO_EXCLUSIVE);
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

	{
		// synthesize wavefile
		SPKRDEF spkrDef = m_cConstants;
		spkrDef.UI = (m_cConstants.SR * m_cConstants.UI + 500)/ 1000;
		CKSynth *pSynth = CreateKSynth(&spkrDef);
		INT16 *pWaveFrame = (INT16*) malloc(sizeof(INT16)*spkrDef.UI);

		double time = 0;
		double timeScale = m_dTimeScale;

		// initialize resonators (prevents initial "explosions")
		if (cChars[0].parameters.AV)
		{
			CIpaChar cInitColumn;
			cInitColumn.parameters = GetTemporalKlattDefaults();	         // use parameter defaults
			cInitColumn.duration = 7. / cInitColumn.parameters.F0 * 1000.;   // set duration to 7 pitch periods
			time += timeScale*cInitColumn.duration/1000.;
			int nFrame = int(time*spkrDef.SR/spkrDef.UI+0.5);
			time -= double(nFrame*spkrDef.UI)/spkrDef.SR;
			for(;nFrame > 0; nFrame--)
			{
				SynthesizeFrame(pSynth, &cInitColumn.parameters, pWaveFrame);
			}
			time = 0;
		}

		for(unsigned int i = 0;i < cChars.size();i++)
		{
			time += timeScale*cChars[i].duration/1000.;

			int nFrame = int(time*spkrDef.SR/spkrDef.UI+0.5);

			time -= double(nFrame*spkrDef.UI)/spkrDef.SR;
			for(;nFrame > 0; nFrame--)
			{
				SynthesizeFrame(pSynth, &cChars[i].parameters, pWaveFrame);
				mmioWrite(hmmioFile, (char*) pWaveFrame, sizeof(INT16)*spkrDef.UI);
			}
		}
		free(pWaveFrame);
		DestroyKSynth(pSynth);
	}

	// At file end what is file position
	// SDM 1.5Test10.2
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
	if (!mmioClose(hmmioFile, 0)) // close file
	{
		// Set File Length ...
		// SDM 1.5Test10.2
		CFile WaveFile(pszPathName,CFile::modeReadWrite);
		//    WaveFile.SetLength(mmioinfo.lDiskOffset+2);
	}

	return TRUE;
}

void CDlgKlattAll::OnUpdateSourceName()
{
	if (m_szSourceFilename.IsEmpty() || !static_cast<CSaApp*>(AfxGetApp())->IsFileOpened(m_szSourceFilename))
	{
		m_szSourceFilename.Empty();
		m_szShowFilename.Empty();
		m_pShowDoc = 0;

		// Populate Source
		CMDIChildWnd* pChild = static_cast<CMainFrame*>(AfxGetMainWnd())->MDIGetActive();
		while(pChild)
		{
			CDocument* pDoc = pChild->GetActiveDocument(); // get pointer to document
			if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CSaDoc)))
				m_szSourceFilename = pDoc->GetPathName();
			if (!m_szSourceFilename.IsEmpty()) 
				break;
			pChild = (CMDIChildWnd*) pChild->GetNextWindow();
		}
	}
}

CString CDlgKlattAll::GetDefaultsPath()
{
	CSaString szPath = AfxGetApp()->GetProfileString(_T(""), _T("DataLocation"));
	szPath += "\\";
	szPath += "IpaDefaults.txt";
	return szPath;
}


void CDlgKlattAll::ParseConstantsGrid(int nGrid, CKlattConstants &cConstants)
{
	const PARAMETER_DESC *parameterInfo = GetGlobalKlattDesc();

	for(int i=0;parameterInfo[i].parameterOffset != -1; i++)
	{
		int row = i + 1;

		CString value;

		value = m_cGrid[nGrid].GetTextMatrix(row,2);
		swscanf_s(value, parameterInfo[i].typeScanf, ((char*)&cConstants)+parameterInfo[i].parameterOffset);
	}
}

void CDlgKlattAll::ParseParameterGrid(int nGrid, CIpaCharVector &cChars)
{
	const PARAMETER_DESC *parameterInfo = GetTemporalKlattDesc();
	cChars.clear();
	CFlexEditGrid &cGrid = m_cGrid[nGrid];

	cChars.reserve(cGrid.GetCols(0));
	for(int column = columnFirst; column < cGrid.GetCols(0); column++)
	{
		CIpaChar columnChar;
		BOOL bColumnValid = FALSE;
		columnChar.ipa = cGrid.GetTextMatrix(rowIpa,column);
		if (columnChar.ipa.IsEmpty())
			continue;

		columnChar.parameters = GetTemporalKlattDefaults();

		for(int i=0;parameterInfo[i].parameterOffset != -1; i++)
		{
			TEMPORAL *pTemporal = &columnChar.parameters;
			int row = i + rowParameters;

			CString value;

			value = cGrid.GetTextMatrix(row,column);
			int scanned = swscanf_s(value, parameterInfo[i].typeScanf, ((char*)pTemporal)+parameterInfo[i].parameterOffset);
			if (scanned == 1)
			{
				bColumnValid = TRUE;
			}
		}
		if (bColumnValid)
		{
			CString value;

			value = cGrid.GetTextMatrix(rowDuration,column);
			int nScanned = swscanf_s(value, _T("%lf"), &columnChar.duration);

			if (nScanned != 1)
				columnChar.duration = 0;

			cChars.push_back(columnChar); // add to end of list
		}
	}
}

void CDlgKlattAll::OnSynthesize()
{
	CIpaCharVector cChars;

	ParseConstantsGrid(kConstants, m_cConstants);
	ParseParameterGrid(m_nSelectedMethod, cChars);

	if (cChars.begin() == cChars.end())
		return;

	if (m_szSynthesizedFilename.IsEmpty())
	{
		TCHAR lpszTempPath[_MAX_PATH];
		GetTempPath(_MAX_PATH, lpszTempPath);
		// create temp filename for synthesized waveform
		GetTempFileName(lpszTempPath, _T("klt"), 0, m_szSynthesizedFilename.GetBuffer(_MAX_PATH));
		m_szSynthesizedFilename.ReleaseBuffer();
	}

	SynthesizeWave(m_szSynthesizedFilename, cChars);
}


void CDlgKlattAll::ConvertCStringToCharVector(CString const &szGrid, CIpaCharVector &cChars)
{
	const PARAMETER_DESC *parameterInfo = GetTemporalKlattDesc();
	cChars.clear();

	int nLength = szGrid.GetLength();

	for(int nLineStart = 0; nLineStart < nLength; )
	{
		int nLineEnd = szGrid.Find(_T("\n"),nLineStart);

		if (nLineEnd == -1)
			nLineEnd = nLength;

		CString szLine = szGrid.Mid(nLineStart,nLineEnd-nLineStart);

		if (szLine.IsEmpty())
			continue;

		CIpaChar columnChar;
		BOOL bColumnValid = FALSE;

		// populate default values
		columnChar.ipa.Empty();
		columnChar.duration = 5;
		columnChar.parameters = GetTemporalKlattDefaults();

		int nFieldStart = 0;
		int nLineLength = szLine.GetLength();
		for(int row = rowFirst; nFieldStart < nLineLength; row++)
		{
			int nFieldEnd = szLine.Find(_T("\t"), nFieldStart);

			if (nFieldEnd == -1)
				// may not have a trailing \t
				nFieldEnd = nLineLength;

			CString szField = szLine.Mid(nFieldStart,nFieldEnd-nFieldStart);

			if (!szField.IsEmpty())
			{
				int i = row - rowParameters;
				switch (row)
				{
				case rowIpa:
					columnChar.ipa = szField;
					break;
				case rowDuration:
					{
						double value;
						int scanned = swscanf_s(szField, _T("%lf"), &value);
						if (scanned != 1)
						{
							columnChar.duration = value;
							bColumnValid = TRUE;
						}
					}
					break;
				default:
					if (row >= rowParameters && parameterInfo[i].parameterOffset != -1)
					{
						TEMPORAL *pTemporal = &columnChar.parameters;

						int scanned = swscanf_s(szField, parameterInfo[i].typeScanf, ((char*)pTemporal)+parameterInfo[i].parameterOffset);
						if (scanned == 1)
						{
							bColumnValid = TRUE;
						}
					}
				}
			}

			nFieldStart = nFieldEnd+1;
		}
		if (bColumnValid)
			cChars.push_back(columnChar); // add to end of list

		nLineStart = nLineEnd + 1;
	}
}

void CDlgKlattAll::LabelGrid(int nGrid)
{
	if (nGrid == -1)
		nGrid = m_nSelectedView;

	if (m_cGrid[nGrid].GetCols(0) <= columnFirst)
		m_cGrid[nGrid].SetCols(0,columnFirst+1);
	m_cGrid[nGrid].SetFixedCols(columnFirst);
	m_cGrid[nGrid].SetTextMatrix(rowHeading,columnDescription, _T("Description"));
	m_cGrid[nGrid].SetTextMatrix(rowHeading,columnSym, _T("Sym"));
	m_cGrid[nGrid].SetTextMatrix(rowHeading,columnDef, _T("Def"));
	m_cGrid[nGrid].SetColWidth(columnDescription,0, 4400);
	m_cGrid[nGrid].SetColWidth(columnSym,0, 600);
	m_cGrid[nGrid].SetColWidth(columnDef,0, 600);
	if (nGrid == kConstants)
	{
		m_cGrid[nGrid].SetColWidth(columnDescription,0, 5400);
		// Label Grid
		if (m_cGrid[nGrid].GetRows() < 3)
			m_cGrid[nGrid].SetRows(3);
		m_cGrid[nGrid].SetFixedCols(2);

		const PARAMETER_DESC *parameterInfo = GetGlobalKlattDesc();

		for(int i=0;parameterInfo[i].parameterOffset != -1; i++)
		{
			int row = i + rowFirst;

			if (row >= m_cGrid[nGrid].GetRows())
				m_cGrid[nGrid].SetRows(row+1);

			CString description;

			if (parameterInfo[i].units)
				description.Format(_T("%s, in %s"),parameterInfo[i].description,parameterInfo[i].units);
			else
				description = parameterInfo[i].description;

			CString value;
			if (parameterInfo[i].typeScanf[1] != 'd')
				value.Format(parameterInfo[i].typeScanf, *(Float*)(((char*)&m_cConstants)+parameterInfo[i].parameterOffset));
			else
				value.Format(parameterInfo[i].typeScanf, *(int*)(((char*)&m_cConstants)+parameterInfo[i].parameterOffset));

			m_cGrid[nGrid].SetTextMatrix(row,columnDescription, description);
			m_cGrid[nGrid].SetTextMatrix(row,columnSym, parameterInfo[i].symbolDescription);
			m_cGrid[nGrid].SetTextMatrix(row,columnFirst, value);

			CString defaultValue;
			defaultValue.Format(_T("%g"), parameterInfo[i].val);
			m_cGrid[nGrid].SetTextMatrix(row, columnDef, defaultValue);
		}
	}
	else
	{ 
		// Label Grid
		if (m_cGrid[nGrid].GetCols(0) < 100)
			m_cGrid[nGrid].SetCols(0,100);
		if (m_cGrid[nGrid].GetRows() < rowParameters)
			m_cGrid[nGrid].SetRows(rowParameters);
		m_cGrid[nGrid].SetTextMatrix(rowIpa,columnDescription, _T("IPA"));
		m_cGrid[nGrid].SetTextMatrix(rowIpa,columnSym, _T("IPA"));
		m_cGrid[nGrid].SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,columnFirst,1, -1);
		m_cGrid[nGrid].SetRowHeight(rowIpa, 500);
		m_cGrid[nGrid].SetTextMatrix(rowDuration,columnDescription, _T("Duration in ms"));
		m_cGrid[nGrid].SetTextMatrix(rowDuration,columnSym, _T("DU"));
		//    m_cGrid[nGrid].SetTextMatrix(rowPitch,columnDescription, "Pitch in Hz");
		//    m_cGrid[nGrid].SetTextMatrix(rowPitch,columnSym, "F0");


		NumberGrid(nGrid);

		const PARAMETER_DESC *parameterInfo = GetTemporalKlattDesc();

		for(register int i=0;parameterInfo[i].parameterOffset != -1; i++)
		{
			int row = i+rowParameters;

			if (row >= m_cGrid[nGrid].GetRows())
				m_cGrid[nGrid].SetRows(row+1);

			CString description;

			if (parameterInfo[i].units)
				description.Format(_T("%s, in %s"),parameterInfo[i].description,parameterInfo[i].units);
			else
				description = parameterInfo[i].description;      

			m_cGrid[nGrid].SetTextMatrix(row, columnDescription, description);
			m_cGrid[nGrid].SetTextMatrix(row, columnSym, parameterInfo[i].symbolDescription);

			CString defaultValue;
			defaultValue.Format(_T("%g"), parameterInfo[i].val);
			m_cGrid[nGrid].SetTextMatrix(row, columnDef, defaultValue);
		}
	}
}

void CDlgKlattAll::NumberGrid(int nGrid)
{
	for(int i=columnFirst;i<m_cGrid[nGrid].GetCols(0); i++)
	{
		CString number;
		number.Format(_T("%d"), i-columnFirst+1);
		m_cGrid[nGrid].SetTextMatrix(rowHeading,i, number);
	}
}


static void CurveFitPitch(CSaDoc *pDoc, double fSizeFactor, double dBeginWAV, double dEndWAV, double *offset, double *slope)    
{
	DWORD dwIndex;      
	DWORD dwBegin = (DWORD) (dBeginWAV/fSizeFactor);
	DWORD dwEnd = (DWORD) (dEndWAV/fSizeFactor);

	int n = 0;
	double sumX = 0;
	double sumY = 0;
	double sumXX = 0;
	double sumYY = 0;
	double sumXY = 0;

	BOOL bRes = TRUE;
	for(dwIndex = dwBegin; dwIndex <= dwEnd; dwIndex++)
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
			localSlope = (n*sumXY - sumX*sumY)/(n*sumXX - sumX*sumX);
		else
			localSlope = 0; // if no change in x, assume 0 slope
		double localOffset = sumY/n - localSlope*sumX/n;

		if (offset)
			*offset = localOffset;
		if (slope)
			*slope = localSlope;
	}
	else
	{
		if (offset)
			*offset = - 1.;
		if (slope)
			*slope = 0;
	}
}

void CDlgKlattAll::OnKlattGetAll() 
{
	OnUpdateSourceName();

	OnKlattGetSegments(m_cGrid[kSegment]);
	ParseParameterGrid(kDefaults, m_cDefaults);
	OnKlattApplyIpaDefaults(m_cGrid[kSegment]);
	OnKlattBlendSegments(kSegment,m_cGrid[kIpaBlended]);
	if (m_bGetFragments)
	{
		OnKlattGetFrames(m_cGrid[kFragment], 
			(int)dKlattFrameWindowMs, (int)dKlattFrameIntervalMs,
			nKlattFrameWindowFragments,nKlattFrameIntervalFragments);
	}

	LabelGrid(kSegment);
	LabelGrid(kIpaBlended);
	LabelGrid(kFragment);
	LabelGrid(kDefaults);
	LabelGrid(kConstants);
}


void CDlgKlattAll::OnKlattGetSegments(CFlexEditGrid &cGrid)
{
	CString szFilename = m_szSourceFilename;

	CSaApp* pApp = (CSaApp*)AfxGetApp();
	CSaDoc* pDoc = (CSaDoc*)pApp->IsFileOpened(szFilename);
	if (!pDoc)
		return;
	CSegment* pPhonetic = pDoc->GetSegment(PHONETIC);

	if (pPhonetic->IsEmpty()) // no annotations
		return;

	enum {PITCH, CALCULATIONS};
	double fSizeFactor[CALCULATIONS];

	if (m_bPitch) // formants need pitch info
	{
		CProcessSmoothedPitch* pPitch = pDoc->GetSmoothedPitch(); // SDM 1.5 Test 11.0
		short int nResult = LOWORD(pPitch->Process(this, pDoc)); // process data
		if (nResult == PROCESS_ERROR) m_bPitch = FALSE;
		else if (nResult == PROCESS_CANCELED) return;
		else fSizeFactor[PITCH] = (double)pDoc->GetDataSize() / (double)(pPitch->GetDataSize() - 1);
	}

	// process all flags
	CString szString;

	DWORD dwOffset = 0;
	DWORD dwPrevOffset;
	DWORD dwDuration = 0;
	int nIndex = 0;
	int column = columnFirst;
	const DWORD dwMinSilence = pDoc->GetBytesFromTime(0.0001);

	// construct table entries
	while(nIndex != -1)
	{
		// clear column
		szString.Empty();
		for(int row = rowFirst; row < cGrid.GetRows(); row++)
		{
			cGrid.SetTextMatrix(row,column,szString);
		}
		dwPrevOffset = dwOffset;
		dwOffset = pPhonetic->GetOffset(nIndex);
		if (dwPrevOffset + dwDuration + dwMinSilence < dwOffset)
		{
			// silence
			szString.Format(_T("silence"));
			cGrid.SetTextMatrix(rowIpa,column,szString);

			if (m_bDuration)
			{
				dwDuration = dwOffset - (dwPrevOffset + dwDuration);
				szString.Format(_T("%.2f"),pDoc->GetTimeFromBytes(dwDuration)*1000.);
				cGrid.SetTextMatrix(rowDuration,column,szString);
			}
			column++;
			// clear column
			szString.Empty();
			for(int row = rowFirst; row < cGrid.GetRows(); row++)
			{
				cGrid.SetTextMatrix(row,column,szString);
			}
		}
		dwDuration = pPhonetic->GetDuration(nIndex);

		szString = pPhonetic->GetSegmentString(nIndex);
		cGrid.SetTextMatrix(rowIpa,column,szString);

		if (m_bDuration)
		{
			szString.Format(_T("%.2f"),pDoc->GetTimeFromBytes(pPhonetic->GetDuration(nIndex))*1000.);
			cGrid.SetTextMatrix(rowDuration,column,szString);
		}

		if (m_bPitch)
		{
			double offset;
			double slope;

			CurveFitPitch(pDoc, fSizeFactor[PITCH], dwOffset, pPhonetic->GetStop(nIndex), &offset, &slope);
			if (offset > 0)
				szString.Format(_T("%.5g"),offset + slope*pPhonetic->GetDuration(nIndex)/2.);
			else
				szString.Empty();
			cGrid.SetTextMatrix(rowF0,column,szString);
		}

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
		// clear column
		szString.Empty();
		for(int row = rowFirst; row < cGrid.GetRows(); row++)
		{
			cGrid.SetTextMatrix(row,column,szString);
		}

		szString.Format(_T("silence"));
		cGrid.SetTextMatrix(rowIpa,column,szString);

		if (m_bDuration)
		{
			dwDuration = dwOffset - (dwPrevOffset + dwDuration);
			szString.Format(_T("%.2f"),pDoc->GetTimeFromBytes(dwDuration)*1000.);
			cGrid.SetTextMatrix(rowDuration,column,szString);
		}
		column++;
	}

	// clear residual columns
	for(;column < cGrid.GetCols(0); column++)
	{
		szString.Empty();
		for(int row = 1; row < cGrid.GetRows(); row++)
		{
			cGrid.SetTextMatrix(row,column,szString);
		}
	}
}

void CDlgKlattAll::SilentColumn(CFlexEditGrid &cGrid, int column, CSaDoc *pDoc, DWORD dwDuration, WORD wSmpSize)
{
	CString szString;

	// clear parameters
	szString.Empty();
	for(int row = rowFirst; row < cGrid.GetRows(); row++)
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

static double GainAtResonance(double naturalFrequency, double bandwidth)
{
	// Transfer function of a single pole pair is A(w) = (Wn^2 + C^2)/((jw+C)^2 + Wn^2)

	// Where BW = C*((2)^0.5 - 1)^0.5   BW=C*0.64359  C= 1.5537*BW

	// The peak response ooccurs at the natural frequency
	//    A(Wn) = (Wn^2 + C^2)/((jWn+C)^2 + Wn^2)
	//    A(Wn) = (Wn^2 + C^2)/(C^2 + 2jCWn)
	//
	// The Magnitude is  |A(Wn)| = (Wn^2 + C^2)/(C (C^2 + 4Wn^2)^0.5)

	// The Magnitude is  |A(Wn)| = (1 + (Wn/C)^2)/(1 + 4(Wn/C)^2)^0.5)
	double WnC = naturalFrequency/(1.553773974*bandwidth);
	double WnC2 = WnC*WnC;

	return 20.*log10((1.+WnC2)/pow(1.+4.*WnC2, 0.5));
}

static double PreEmphasisAdjust(double frequencyHz, double samplingRateHz)
{
	// first difference funtion is 

	// f(t+T/2) - f(t-T/2) = sum(cos(w(t+T/2) +p) - cos(w(t+T/2) +p))
	//
	// cos(a+b) = cos(a)cos(b) - sin(a)sin(b)
	// cos(a-b) = cos(a)cos(b) + sin(a)sin(b)
	//
	// f(t+T/2) - f(t-T/2) = sum(sin(-2*wt +p)sin(wT/2))

	return 20.*log10( 2*sin((2*PI*frequencyHz)*(1/samplingRateHz/2)));
}

BOOL CDlgKlattAll::GetFormants(CFlexEditGrid &cGrid, int column, CSaView *pView, CProcessSpectrum *pSpectrum, CProcessGrappl* pAutoPitch, CProcessZCross *pZCross, DWORD dwFrameStart, DWORD dwFrameLength, double *pFormFreq)
{
	TEMPORAL temporalDefaults = GetTemporalKlattDefaults();
	double pFormBW[7] = {0,
		temporalDefaults.B1,
		temporalDefaults.B2,
		temporalDefaults.B3,
		temporalDefaults.B4,
		temporalDefaults.B5,
		temporalDefaults.B6};
	double pFormBWF[7] = {0,
		0,
		temporalDefaults.B2F,
		temporalDefaults.B3F,
		temporalDefaults.B4F,
		temporalDefaults.B5F,
		temporalDefaults.B6F};
	double pFormEnergy[7] = {0,0,0,0,0,0,0};
	double pFormAV[7] = {0,0,0,0,0,0,0};

	CSaDoc *pDoc = pView->GetDocument();
	WORD wSmpSize = WORD(pDoc->GetFmtParm()->wBlockAlign / pDoc->GetFmtParm()->wChannels);
	DWORD dwSamplesPerSec = pDoc->GetFmtParm()->dwSamplesPerSec;
	BOOL bRes = TRUE;
	CString szString;

	SPECT_PROC_SELECT SpectraSelected;
	SpectraSelected.bCepstralSpectrum = FALSE;    // turn off to reduce processing time
	SpectraSelected.bLpcSpectrum = TRUE;          // use Lpc method for estimating formants

	long nResult = pSpectrum->Process(this,pDoc,dwFrameStart * wSmpSize, dwFrameLength * wSmpSize, SpectraSelected);
	if (nResult == PROCESS_ERROR)
		return FALSE;

	// Get voicing/frication information
	double fPowerLo = pSpectrum->GetSpectralRegionPower(pDoc, 0, 2000);
	double fPowerHi = (dwSamplesPerSec >= 16000)?pSpectrum->GetSpectralRegionPower(pDoc, 6000, 7999):0;
	double fPowerAll = 10.0 * log10(pow(10.0, fPowerLo / 10.0) + pow(10.0, fPowerHi / 10.0));

	unsigned nAV = 0;
	unsigned nAF = 0;
	// Set weight factors
	const double fBEWt = 0.75; // band energy weight factor
	const double fAPWt = 0.25; // auto pitch weigth factor

	enum {ABSOLUTE_CALC, RELATIVE_CALC, CALCULATION};
	int nAmplitudeCalculation = ABSOLUTE_CALC;

	if (cGrid.GetTextMatrix(rowAV,column) != "0")
	{
		if (!m_cConstants.CP)
		{
			switch (nAmplitudeCalculation)
			{
			case ABSOLUTE_CALC:
				{
					// Band energy voicing detector
					nAV = (unsigned)(fBEWt * (fPowerLo +  85.0));
					nAF = (unsigned)(fBEWt * (fPowerHi + 105.0));

					// Auto-Pitch voicing detector
					if (pAutoPitch->IsVoiced(pDoc, dwFrameStart * wSmpSize))
						nAV += (unsigned)(fAPWt * (fPowerAll +  85.0));
					else
						nAF += (unsigned)(fAPWt * (fPowerAll + 100.0));
					if (nAF>70) nAF=70; // Avoid clipping
					// Check to see if this is an unvoiced fricative
					int nZC = pZCross->GetProcessedData(dwFrameStart / 100, &bRes);
					if (nZC > 90)
						nAV = 0; // turn voicing off completely
				}
				break;
			case RELATIVE_CALC:
				{
					double fFractionAV = 0.;
					double fFractionAF = 0.;
					double fAEffective = 65;
					double fPowerTotal = pow(10.0, (fAEffective / 10.0));
					// Band energy voicing detector
					fFractionAV = fBEWt * (fPowerLo/(fPowerLo+fPowerHi));
					fFractionAF = fBEWt * (fPowerHi/(fPowerLo+fPowerHi));

					// Auto-Pitch voicing detector
					if (pAutoPitch->IsVoiced(pDoc, dwFrameStart * wSmpSize))
						fFractionAV += fAPWt;
					else
						fFractionAF += fAPWt;

					nAF = (unsigned)(10 * log10(fFractionAF * fPowerTotal));
					nAV = (unsigned)(10 * log10(fFractionAV * fPowerTotal));
				}
			}
		}
		else
			nAV = 60;
	}

	// Send amplitude values to the grid
	szString.Format(_T("%u"),nAF);
	cGrid.SetTextMatrix(rowAF,column,szString);
	szString.Format(_T("%u"),nAV);
	cGrid.SetTextMatrix(rowAV,column,szString);

	const PARAMETER_DESC *parameterInfo = GetTemporalKlattDesc();
	double LpcRef;
	double MaxLpcBandPwr = pSpectrum->GetSpectralPowerRange().Max.Lpc;
	double MaxRawBandPwr = pSpectrum->GetSpectralPowerRange().Max.Raw;
	MaxLpcBandPwr = (MaxLpcBandPwr == 0.)?MIN_LOG_PWR:10.*log10((double)MaxLpcBandPwr);
	if (MaxRawBandPwr != (float)UNDEFINED_DATA)
		LpcRef = MaxLpcBandPwr - MaxRawBandPwr;
	else
		LpcRef = pSpectrum->GetSpectralPowerRange().fdBRef;
	// get data for this column
	for(int n = 1; n <= 6; n++)
	{
		FORMANT form = pSpectrum->GetFormant((unsigned short) n);
		if (form.Lpc.FrequencyInHertz == UNDEFINED_DATA || form.Lpc.PowerInDecibels == FLT_MAX_NEG)
		{
			continue;
		}
		// tilt spectrum back
		double fSlope = (m_cConstants.SS <= 3)?20.:10.0;
		form.Lpc.PowerInDecibels += (float)(fSlope*log10(1.9025-1.9*cos(2.*PI*(double)form.Lpc.FrequencyInHertz/(double)dwSamplesPerSec)));

		// get formant frequency values
		double fParmPrev, fParmCalc, fParmChange, fParmChangeMax, fParmChangeMin;
		const double fParmChangeMaxAbs = 1.3;
		fParmPrev = pFormFreq[n]; // store previous value
		if (nAV) // lock frequency value through unvoiced regions
			fParmCalc = form.Lpc.FrequencyInHertz;
		else
			fParmCalc = fParmPrev;
		if (3==column) fParmPrev = fParmCalc; // always use calculated formant value for first column
		// prevent wild changes in formant values
		fParmChange = fParmCalc/fParmPrev;
		switch(n)
		{
		case 1:
			{
				fParmChangeMax = (pFormFreq[n+1]/fParmPrev+1)/2;
				if (fParmChangeMax > fParmChangeMaxAbs) fParmChangeMax = fParmChangeMaxAbs;
				fParmChangeMin = 1/fParmChangeMaxAbs;
			}
			break;
		case 6:
			{
				fParmChangeMax = fParmChangeMaxAbs;
				fParmChangeMin = (pFormFreq[n-1]/fParmPrev+1)/2;
				if (fParmChangeMax < 1/fParmChangeMaxAbs) fParmChangeMax = 1/fParmChangeMaxAbs;
			}
			break;
		default:
			fParmChangeMax = (pFormFreq[n+1]/fParmPrev+1)/2;
			if (fParmChangeMax > fParmChangeMaxAbs) fParmChangeMax = fParmChangeMaxAbs;
			fParmChangeMin = (pFormFreq[n-1]/fParmPrev+1)/2;
			if (fParmChangeMax < 1/fParmChangeMaxAbs) fParmChangeMax = 1/fParmChangeMaxAbs;
		}
		if (fParmChange>fParmChangeMax) fParmChange=fParmChangeMax;
		if (fParmChange<fParmChangeMin) fParmChange=fParmChangeMin;
		pFormFreq[n] = fParmChange * fParmPrev;

		// get bandwidth values
		// pFormBW[n] = pow(10, pFormFreq[n]/6000.0 + 0.85 - (form.Lpc.PowerInDecibels-LpcRef)/20.0);
		if (column>3) // use defaults for first column
		{
			// voiced bandwidths
			fParmPrev = _ttoi(cGrid.GetTextMatrix(rowB1+2*(n-(1==n)), column-1));
			fParmChange = (form.Lpc.BandwidthInHertz - pFormBW[n]) / pFormBW[n];
			switch(n)
			{
			case 1:
				{
					fParmChangeMax = (pFormBW[n+1] / pFormBW[n] + 1) / 2;
					if (fParmChangeMax > fParmChangeMaxAbs) fParmChangeMax = fParmChangeMaxAbs;
					fParmChangeMin = 1/fParmChangeMaxAbs;
				}
				break;
			case 6:
				{
					fParmChangeMax = fParmChangeMaxAbs;
					fParmChangeMin = (pFormBW[n-1] / pFormBW[n] + 1) / 2;
					if (fParmChangeMax < 1/fParmChangeMaxAbs) fParmChangeMax = 1/fParmChangeMaxAbs;
				}
				break;
			default:
				fParmChangeMax = (pFormBW[n+1] / pFormBW[n] + 1) / 2;
				if (fParmChangeMax > fParmChangeMaxAbs) fParmChangeMax = fParmChangeMaxAbs;
				fParmChangeMin = (pFormBW[n-1] / pFormBW[n] + 1) / 2;
				if (fParmChangeMax < 1/fParmChangeMaxAbs) fParmChangeMax = 1/fParmChangeMaxAbs;
			}
			if (fParmChange > fParmChangeMax) fParmChange = fParmChangeMax;
			if (fParmChange < fParmChangeMin) fParmChange = fParmChangeMin;
			pFormBW[n] = fParmChange * fParmPrev;
			int nParameterRow = 10+2*(n-(1==n));
			if (pFormBW[n] < parameterInfo[nParameterRow].min)
				pFormBW[n] = parameterInfo[nParameterRow].min;

			// unvoiced bandwidths
			fParmPrev = _ttoi(cGrid.GetTextMatrix(rowB2F+n-2, column-1));
			fParmChange = (form.Lpc.BandwidthInHertz - pFormBWF[n]) / pFormBWF[n];
			switch(n)
			{
			case 1:
				{
					fParmChangeMax = (pFormBWF[n+1] / pFormBWF[n] + 1) / 2;
					if (fParmChangeMax > fParmChangeMaxAbs) fParmChangeMax = fParmChangeMaxAbs;
					fParmChangeMin = 1/fParmChangeMaxAbs;
				}
				break;
			case 6:
				{
					fParmChangeMax = fParmChangeMaxAbs;
					fParmChangeMin = (pFormBWF[n-1] / pFormBWF[n] + 1) / 2;
					if (fParmChangeMax < 1/fParmChangeMaxAbs) fParmChangeMax = 1/fParmChangeMaxAbs;
				}
				break;
			default:
				fParmChangeMax = (pFormBWF[n+1] / pFormBWF[n] + 1) / 2;
				if (fParmChangeMax > fParmChangeMaxAbs) fParmChangeMax = fParmChangeMaxAbs;
				fParmChangeMin = (pFormBWF[n-1] / pFormBWF[n] + 1) / 2;
				if (fParmChangeMax < 1/fParmChangeMaxAbs) fParmChangeMax = 1/fParmChangeMaxAbs;
			}
			if (fParmChange > fParmChangeMax) fParmChange = fParmChangeMax;
			if (fParmChange < fParmChangeMin) fParmChange = fParmChangeMin;
			pFormBWF[n] = fParmChange * fParmPrev;
			nParameterRow = 37+2*(n-(1==n));
			if (pFormBWF[n] < parameterInfo[nParameterRow].min)
				pFormBWF[n] = parameterInfo[nParameterRow].min;
		}
		form.Lpc.PowerInDecibels -= (float) GainAtResonance(form.Lpc.FrequencyInHertz,form.Lpc.BandwidthInHertz);
		pFormAV[n] = form.Lpc.PowerInDecibels - LpcRef + 90;
		pFormEnergy[n] = (form.Lpc.PowerInDecibels-LpcRef + 10.) * 0.70 + 80.;
	}

	AdjustParallelAVs(pFormAV, pFormFreq);
	if (pFormBW[1])
	{
		szString.Format(_T("%.5g"),pFormFreq[1]);
		cGrid.SetTextMatrix(rowF1,column,szString);
		szString.Format(_T("%.5g"),pFormBW[1]);
		cGrid.SetTextMatrix(rowB1,column,szString);
		szString.Format(_T("%.5g"),pFormAV[1]);
		cGrid.SetTextMatrix(rowA1V,column,szString);
	}
	if (pFormBW[2])
	{
		szString.Format(_T("%.5g"),pFormFreq[2]);
		cGrid.SetTextMatrix(rowF2,column,szString);
		szString.Format(_T("%.5g"),pFormBW[2]);
		cGrid.SetTextMatrix(rowB2,column,szString);
		szString.Format(_T("%.5g"),pFormBWF[2]);
		cGrid.SetTextMatrix(rowB2F,column,szString);
		szString.Format(_T("%.5g"),0.0);
		cGrid.SetTextMatrix(rowA2F,column,szString);
		szString.Format(_T("%.5g"),pFormAV[2] - PreEmphasisAdjust(pFormFreq[2],m_cConstants.SR));
		cGrid.SetTextMatrix(rowA2V,column,szString);
	}
	if (pFormBW[3])
	{
		szString.Format(_T("%.5g"),pFormFreq[3]);
		cGrid.SetTextMatrix(rowF3,column,szString);
		szString.Format(_T("%.5g"),pFormBW[3]);
		cGrid.SetTextMatrix(rowB3,column,szString);
		szString.Format(_T("%.5g"),pFormBWF[3]);
		cGrid.SetTextMatrix(rowB3F,column,szString);
		szString.Format(_T("%.5g"),0.0);
		cGrid.SetTextMatrix(rowA3F,column,szString);
		szString.Format(_T("%.5g"),pFormAV[3] - PreEmphasisAdjust(pFormFreq[2],m_cConstants.SR));
		cGrid.SetTextMatrix(rowA3V,column,szString);
	}
	if (pFormBW[4])
	{
		szString.Format(_T("%.5g"),pFormFreq[4]);
		cGrid.SetTextMatrix(rowF4,column,szString);
		szString.Format(_T("%.5g"),pFormBW[4]);
		cGrid.SetTextMatrix(rowB4,column,szString);
		szString.Format(_T("%.5g"),pFormBWF[4]>350?pFormBWF[4]:350);
		cGrid.SetTextMatrix(rowB4F,column,szString);
		szString.Format(_T("%.5g"),pFormEnergy[4]);
		cGrid.SetTextMatrix(rowA4F,column,szString);
		szString.Format(_T("%.5g"),pFormAV[4] - PreEmphasisAdjust(pFormFreq[2],m_cConstants.SR));
		cGrid.SetTextMatrix(rowA4V,column,szString);
	}
	if (pFormBW[5])
	{
		szString.Format(_T("%.5g"),pFormFreq[5]);
		cGrid.SetTextMatrix(rowF5,column,szString);
		szString.Format(_T("%.5g"),pFormBW[5]);
		cGrid.SetTextMatrix(rowB5,column,szString);
		szString.Format(_T("%.5g"),pFormBWF[5]>500?pFormBWF[5]:500);
		cGrid.SetTextMatrix(rowB5F,column,szString);
		szString.Format(_T("%.5g"),pFormEnergy[5] - PreEmphasisAdjust(pFormFreq[2],m_cConstants.SR));
		cGrid.SetTextMatrix(rowA5F,column,szString);
	}
	if (pFormBW[6])
	{
		szString.Format(_T("%.5g"),pFormFreq[6]);
		cGrid.SetTextMatrix(rowF6,column,szString);
		szString.Format(_T("%.5g"),pFormBW[6]);
		cGrid.SetTextMatrix(rowB6,column,szString);
		szString.Format(_T("%.5g"),pFormBWF[6]>1500?pFormBWF[6]:1500);
		cGrid.SetTextMatrix(rowB6F,column,szString);
		szString.Format(_T("%.5g"),pFormEnergy[6] - PreEmphasisAdjust(pFormFreq[2],m_cConstants.SR));
		cGrid.SetTextMatrix(rowA6F,column,szString);
	}
	return TRUE;
}

BOOL CDlgKlattAll::GetFrame(CFlexEditGrid &cGrid, int &column, CSaView *pView, 
							CProcessSpectrum *pSpectrum, CProcessGrappl* pAutoPitch, CProcessZCross *pZCross, 
							DWORD dwStart, DWORD dwLength, DWORD dwInterval, double *pFormFreq)
{ 
	CSaDoc *pDoc = pView->GetDocument();
	CSegment* pPhonetic = pDoc->GetSegment(PHONETIC);
	CProcessSmoothedPitch* pPitch = pDoc->GetSmoothedPitch();
	WORD wSmpSize = WORD(pDoc->GetFmtParm()->wBlockAlign / pDoc->GetFmtParm()->wChannels);
	BOOL bPitch = TRUE;
	BOOL bDuration = TRUE;
	BOOL bFormants = TRUE;


	CString szString;
	// clear parameters
	szString.Empty();
	for(int row = rowIpa; row < cGrid.GetRows(); row++)
	{
		cGrid.SetTextMatrix(row,column,szString);
	}

	int nIndex = pPhonetic->FindFromPosition((dwStart + dwLength/2)*wSmpSize, TRUE);
	if (nIndex != -1)
		szString = pPhonetic->GetSegmentString(nIndex);
	else
		szString = "silence";
	cGrid.SetTextMatrix(rowIpa,column,szString);

	if (bDuration)
	{
		szString.Format(_T("%.2f"),pDoc->GetTimeFromBytes(((DWORD)dwInterval * wSmpSize) * 1000));
		cGrid.SetTextMatrix(rowDuration,column,szString);
	}

	if (bPitch)
	{
		double offset;
		double slope;
		double fSizeFactor = (double)pDoc->GetDataSize() / (double)(pPitch->GetDataSize() - 1);

		CurveFitPitch(pDoc, fSizeFactor, dwStart * wSmpSize,
			(dwStart + dwLength) * wSmpSize, &offset, &slope);
		if (offset > 0)
		{
			szString.Format(_T("%.5g"),offset + slope*(dwLength + 1)/2.);
			cGrid.SetTextMatrix(rowF0,column,szString);
		}
		else
			cGrid.SetTextMatrix(rowAV,column,_T("0"));
	}

	// Formant info
	if (bFormants)
	{
		BOOL bRes = GetFormants(cGrid, column, pView, pSpectrum, pAutoPitch, pZCross, dwStart, dwLength, pFormFreq);

		if (!bRes)
			return FALSE;
	}

	column++;

	if (column >= cGrid.GetCols(0))
	{
		cGrid.SetCols(0, column+10);
		cGrid.SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,column,1,-1);
	}
	return TRUE;
}

//
// OnKlattGetFrames
//
// Fills the grid with data taken from frames of the file
//
void CDlgKlattAll::OnKlattGetFrames(CFlexEditGrid &cGrid, int nFrameLengthInMs, int nFrameIntervalInMs, int nFrameLengthInFrags, int nFrameIntervalInFrags)
{
	CString szFilename = m_szSourceFilename;

	CSaApp* pApp = (CSaApp*)AfxGetApp();
	CSaDoc* pDoc = (CSaDoc*)pApp->IsFileOpened(szFilename);
	if (!pDoc) return;
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pDoc->GetNextView(pos); // get pointer to view
	CSegment* pPhonetic = pDoc->GetSegment(PHONETIC);

	BOOL bTempTranscription = FALSE;
	if (pPhonetic->IsEmpty()) // no annotations
	{
		CSaString szTranscription = " ";
		pPhonetic->Insert(0, &szTranscription, 0, 0, pDoc->GetDataSize());
		bTempTranscription = TRUE;
	}

	enum {PITCH, FORMANTS, CALCULATIONS};
	double fSizeFactor[CALCULATIONS];

	BOOL bPitch = TRUE;
	BOOL bFormants = TRUE;

	TEMPORAL temporalDefaults = GetTemporalKlattDefaults();
	double pFormFreq[7] = {0,
		temporalDefaults.F1,
		temporalDefaults.F2,
		temporalDefaults.F3,
		temporalDefaults.F4,
		temporalDefaults.F5,
		temporalDefaults.F6};
	CProcessZCross *pZCross = NULL;
	CProcessSpectrum *pSpectrum = NULL;
	UttParm myUttParm;  UttParm* pUttParm = &myUttParm;
	pDoc->GetUttParm(pUttParm); // get sa parameters utterance member data
	UttParm cSavedUttParm;  UttParm* pSavedUttParm = &cSavedUttParm;

	CProcessSmoothedPitch* pPitch = NULL;
	CProcessGrappl* pAutoPitch = NULL;
	if (bPitch || bFormants) // formants need pitch info
	{
		pPitch = pDoc->GetSmoothedPitch(); // SDM 1.5 Test 11.0
		pAutoPitch = pDoc->GetGrappl();
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
			pPitch->SetDataInvalid();
		pDoc->SetUttParm(pUttParm);
		short int nResult = LOWORD(pPitch->Process(this, pDoc)); // process data
		pDoc->SetUttParm(pSavedUttParm); // restore smoothed pitch parameters
		if (nResult == PROCESS_ERROR) bPitch = FALSE;
		else if (nResult == PROCESS_CANCELED) return;
		nResult = LOWORD(pAutoPitch->Process(this, pDoc)); // process data
		if (nResult == PROCESS_ERROR) bPitch = FALSE;
		else if (nResult == PROCESS_CANCELED) return;
		else fSizeFactor[PITCH] = (double)pDoc->GetDataSize() / (double)(pPitch->GetDataSize() - 1);
	}

	if (bFormants) // process formants
	{
		pZCross = pDoc->GetZCross();
		long nResult = pZCross->Process(this, pDoc);
		if (nResult == PROCESS_ERROR) bFormants = FALSE;
		else if (nResult == PROCESS_CANCELED) return;
		pSpectrum = pDoc->GetSpectrum();
	}

	// process all flags
	CString szString;

	WORD wSmpSize = WORD(pDoc->GetFmtParm()->wBlockAlign / pDoc->GetFmtParm()->wChannels);
	DWORD dwSamplesPerSec = pDoc->GetFmtParm()->dwSamplesPerSec;

	DWORD dwDataLength = pDoc->GetDataSize() / wSmpSize;
	DWORD dwSamplesPerFrame = (DWORD)(dwSamplesPerSec * nFrameIntervalInMs / 1000.0);
	DWORD dwTotalFrames = dwSamplesPerFrame ? dwDataLength / dwSamplesPerFrame + 1 : dwDataLength/(dwSamplesPerSec / 50) + 1;

	m_cGrid[kFragment].SetCols(0,columnFirst + dwTotalFrames);
	NumberGrid(kFragment);
	m_cGrid[kFragment].SetFont(PHONETIC_DEFAULT_FONT,PHONETIC_DEFAULT_FONTSIZE,rowIpa,columnFirst,1, -1);

	int column = columnFirst;
	DWORD dwFrameLengthSamples = dwSamplesPerSec * nFrameLengthInMs / 1000; // length in samples

	CProcessFragments *pFragment = pDoc->GetFragments();
	pFragment->Process(this, pDoc); // process data
	DWORD dwLastFragmentIndex = pFragment->GetFragmentIndex((pDoc->GetDataSize() - 1) / wSmpSize);

	DWORD dwOffset = 0;
	DWORD dwPrev = 0;
	// construct table entries
	while(dwOffset < dwDataLength)
	{
		DWORD dwStart = dwOffset > dwFrameLengthSamples/2 ? dwOffset - dwFrameLengthSamples/2 : 0;
		DWORD dwEnd =  dwOffset + dwFrameLengthSamples/2 < dwDataLength ? dwOffset + dwFrameLengthSamples/2 : dwDataLength - 1;

		if (dwStart && nFrameLengthInFrags)
		{
			DWORD dwOffsetIndex = pFragment->GetFragmentIndex(dwOffset);
			DWORD dwStartIndex = pFragment->GetFragmentIndex(dwStart);

			if (dwStartIndex + (nFrameLengthInFrags+1)/2 > dwOffsetIndex)
			{
				dwStartIndex = dwOffsetIndex > DWORD(nFrameLengthInFrags-1)/2 ? dwOffsetIndex - (nFrameLengthInFrags-1)/2 : 0;

				FRAG_PARMS stFragment = pFragment->GetFragmentParms(dwStartIndex);

				if (dwStart > stFragment.dwOffset)
					dwStart = stFragment.dwOffset;
			}
		}

		if (nFrameLengthInFrags)
		{
			DWORD dwOffsetIndex = pFragment->GetFragmentIndex(dwOffset);
			DWORD dwEndIndex = pFragment->GetFragmentIndex(dwEnd);

			if (dwOffsetIndex + (nFrameLengthInFrags+1)/2 > dwEndIndex)
			{
				dwEndIndex = dwOffsetIndex + (nFrameLengthInFrags-1)/2 < dwLastFragmentIndex ? dwOffsetIndex + (nFrameLengthInFrags-1)/2 : dwLastFragmentIndex;

				FRAG_PARMS stFragment = pFragment->GetFragmentParms(dwEndIndex);

				if (dwEnd < stFragment.dwOffset + stFragment.wLength - 1)
					dwEnd = stFragment.dwOffset + stFragment.wLength - 1;
			}
		}

		DWORD dwLength = dwEnd - dwStart;
		DWORD dwInterval = dwOffset + dwSamplesPerFrame < dwDataLength ? dwSamplesPerFrame : dwDataLength - dwOffset;
		GetFrame(cGrid, column, pView, pSpectrum, pAutoPitch, pZCross, dwStart, dwLength, dwInterval, pFormFreq);

		dwPrev = dwOffset;
		dwOffset += dwSamplesPerFrame;

		if (nFrameIntervalInFrags)
		{
			DWORD dwFragmentIndex = pFragment->GetFragmentIndex(dwPrev);
			if (dwFragmentIndex == dwLastFragmentIndex)
				break;

			FRAG_PARMS stFragment = pFragment->GetFragmentParms(dwFragmentIndex);

			if (dwOffset < stFragment.dwOffset + stFragment.wLength)
			{
				dwOffset = stFragment.dwOffset + stFragment.wLength;
			}
		}
	}

	// clear residual columns
	for(;column < cGrid.GetCols(0); column++)
	{
		szString.Empty();
		for(int row = 1; row < cGrid.GetRows(); row++)
		{
			cGrid.SetTextMatrix(row,column,szString);
		}
	}

	// Get rid of smoothed pitch data so it doesn't interfere with an existing graph
	if (bPitch)
		pDoc->GetSmoothedPitch()->SetDataInvalid();

	if (bTempTranscription)
		pPhonetic->DeleteContents();
}

void CDlgKlattAll::AdjustParallelAVs(double pFormAV[7], double /*pFormFreq*/[7])
{
	double h[7] = {0.,0.,0.,0.,0.,0.,0.};			// h = scale factor for each transfer function
	double A[7][7] = {{0.,0.,0.,0.,0.,0.,0.},		// A = matrix of values of each transfer function
	{0.,0.,0.,0.,0.,0.,0.},		//     evaluated at each formant frequency
	{0.,0.,0.,0.,0.,0.,0.},
	{0.,0.,0.,0.,0.,0.,0.},
	{0.,0.,0.,0.,0.,0.,0.},
	{0.,0.,0.,0.,0.,0.,0.},
	{0.,0.,0.,0.,0.,0.,0.}};

	UNUSED_ALWAYS(h);
	UNUSED_ALWAYS(A);

	// calculate transfer function values at formant frequencies

	// adjust AV values
	for(int n=0; n<8; n++)
	{
		pFormAV[n] = pFormAV[n];
	}

	return;
}

void CDlgKlattAll::OnKlattApplyIpaDefaults(CFlexEditGrid &cGrid)
{
	BOOL bFound = FALSE;

	CIpaCharMap *pMap = new CIpaCharMap(m_cDefaults);
	for(int column=columnFirst; column<cGrid.GetCols(0); column++)
	{
		CString ipa = cGrid.GetTextMatrix(rowIpa,column);
		CString szPitch = cGrid.GetTextMatrix(rowF0,column);
		if (ipa.GetLength())
		{
			CIpaCharMap::const_iterator pParm = pMap->find(ipa);

			if (pParm == pMap->end())
				pParm = pMap->find("undefined");

			if (pParm != pMap->end())
			{
				bFound = TRUE;

				TEMPORAL const *pTemporal = &pParm->second;

				PopulateParameterGrid(cGrid, pTemporal, column);
			}
			if (m_bPitch)
			{
				bFound = TRUE;
				if (szPitch.IsEmpty()) szPitch = "0";
				cGrid.SetTextMatrix(rowF0,column, szPitch);
			}
		}
	}
	delete pMap;
}


static double InterpolateWeight(double dLocation, double dBreakPoint, double dEndPoint)
{
	// second order 
	if (dLocation < dBreakPoint)
		return 1.0 - 0.5*(dLocation / dBreakPoint)*(dLocation / dBreakPoint);
	else
		return 0.5* (dEndPoint - dLocation) / (dEndPoint - dBreakPoint) * (dEndPoint - dLocation) / (dEndPoint - dBreakPoint);

	// simple linear for now
	if (dLocation < dBreakPoint)
		return 1.0 - 0.5*(dLocation / dBreakPoint);
	else
		return 0.5* (dEndPoint - dLocation) / (dEndPoint - dBreakPoint);
}

static void WeighChars(CIpaChar &cPrev, CIpaChar &cNext, double dWeight, CIpaChar &cThis)
{
#define WeighParameter(P) ((cThis. ## P) = ((cPrev. ## P)*dWeight + (cNext. ## P)*(1 - dWeight)))

	// For dB we are going to weieght based on power.... 
#define WeighParameterDB(P) ((cThis. ## P) = 10*log(10.F)*log(exp((cPrev. ## P)/10/log(10.F))*dWeight + exp((cNext. ## P)/10/log(10.F))*(1 - dWeight)))

	//WeighParameter(parameters.F0);
	WeighParameterDB(parameters.AV);
	WeighParameterDB(parameters.AH);
	WeighParameterDB(parameters.AF);

	WeighParameter(parameters.OQ);
	WeighParameter(parameters.FL);
	WeighParameter(parameters.DI);
	WeighParameter(parameters.F1);
	WeighParameter(parameters.B1);
	WeighParameter(parameters.DF1);
	WeighParameterDB(parameters.DB1);
	WeighParameter(parameters.F2);
	WeighParameter(parameters.B2);
	WeighParameter(parameters.F3);
	WeighParameter(parameters.B3);
	WeighParameter(parameters.F4);
	WeighParameter(parameters.B4);
	WeighParameter(parameters.F5);
	WeighParameter(parameters.B5);
	WeighParameter(parameters.F6);
	WeighParameter(parameters.B6);
	WeighParameter(parameters.FNP);
	WeighParameter(parameters.BNP);
	WeighParameter(parameters.FNZ);
	WeighParameter(parameters.BNZ);
	WeighParameter(parameters.FTP);
	WeighParameter(parameters.BTP);
	WeighParameter(parameters.FTZ);
	WeighParameter(parameters.BTZ);
	WeighParameterDB(parameters.A2F);
	WeighParameterDB(parameters.A3F);
	WeighParameterDB(parameters.A4F);
	WeighParameterDB(parameters.A5F);
	WeighParameterDB(parameters.A6F);
	WeighParameterDB(parameters.AB);
	WeighParameter(parameters.B2F);
	WeighParameter(parameters.B3F);
	WeighParameter(parameters.B4F);
	WeighParameter(parameters.B5F);
	WeighParameter(parameters.B6F);
	WeighParameterDB(parameters.ANV);
	WeighParameterDB(parameters.A1V);
	WeighParameterDB(parameters.A2V);
	WeighParameterDB(parameters.A3V);
	WeighParameterDB(parameters.A4V);
	WeighParameterDB(parameters.ATV);
	WeighParameterDB(parameters.A5V);
	WeighParameterDB(parameters.A6V);
	WeighParameterDB(parameters.A7V);
	WeighParameterDB(parameters.A8V);

#undef WeighParameter
#undef WeighParameterDB
}

static void Interpolate(CIpaCharVector &cInterpolated, int nPrevious, double dBreakPoint, int nNext)
{
	double breakTime = 0;
	for(int nLocation = nPrevious; nLocation < dBreakPoint; nLocation++)
		breakTime += cInterpolated[nLocation].duration;

	double endTime = 0;
	for(int nLocation = nPrevious; nLocation < nNext; nLocation++)
		endTime += cInterpolated[nLocation].duration;

	double time = 0;
	for(int nLocation = nPrevious + 1; nLocation < nNext; nLocation++)
	{
		time += cInterpolated[nLocation].duration/2.;
		double dWeight = InterpolateWeight(time, breakTime, endTime);

		WeighChars(cInterpolated[nPrevious],cInterpolated[nNext], dWeight, cInterpolated[nLocation]);
		time += cInterpolated[nLocation].duration/2.;
	}   
}

void CDlgKlattAll::OnKlattBlendSegments(int nSrc, CFlexEditGrid &cGrid)
{
	CIpaCharVector cSegments;
	CIpaCharVector cInterpolated;
	ParseParameterGrid(nSrc, cSegments);

	SPKRDEF spkrDef = m_cConstants;

	cInterpolated.reserve(cSegments.size()*8); // estimate 8x growth from unblended

	for(unsigned int nIndex = 0; nIndex < cSegments.size(); nIndex++)
	{
		if (cSegments[nIndex].parameters.AH == 0. &&
			cSegments[nIndex].parameters.AF == 0. &&
			(cSegments[nIndex].parameters.AV == 0. || cSegments[nIndex].parameters.F0 == 0.))
		{
			// Silence
			CIpaChar value(cSegments[nIndex].ipa, cSegments[nIndex > 0 ? nIndex - 1 : nIndex].parameters, 0);
			// Insert end marker to prevent blending amlitudes
			value.duration = 0;
			cInterpolated.push_back(value);
			// Sudden transition to silence
			value.parameters.AH = 0;
			value.parameters.AF = 0;
			value.parameters.F0 = 0;
			cInterpolated.push_back(value);
			value.duration = cSegments[nIndex].duration/2;
			cInterpolated.push_back(value);
			value.duration = 0;
			cInterpolated.push_back(value);

			// sudden transition to new formants during silence midpoint
			value.parameters = cSegments[nIndex + 1 < cSegments.size() ? nIndex + 1 : nIndex].parameters;
			value.parameters.AH = 0;
			value.parameters.AF = 0;
			value.parameters.F0 = 0;
			cInterpolated.push_back(value);
			value.duration = cSegments[nIndex].duration/2;
			cInterpolated.push_back(value);
			value.duration = 0;
			cInterpolated.push_back(value);
			// sudden transition to new amplitude
			value.parameters = cSegments[nIndex + 1 < cSegments.size() ? nIndex + 1 : nIndex].parameters;
			cInterpolated.push_back(value);
		}
		else
		{
			CIpaChar newValue(cSegments[nIndex]);

			double dLength = cSegments[nIndex].duration/2;

			newValue.duration = dLength/int(dLength/spkrDef.UI);
			for(int replicas = int(dLength/spkrDef.UI); replicas > 0; replicas--)
			{
				cInterpolated.push_back(newValue);
			}

			newValue.duration = 0;
			cInterpolated.push_back(newValue);

			newValue.duration = dLength/int(dLength/spkrDef.UI);
			for(int replicas = int(dLength/spkrDef.UI); replicas > 0; replicas--)
			{
				cInterpolated.push_back(newValue);
			}
		}
	}


	int nLastMarker = 0;
	double dBreakPoint= 0;
	CString szPrevious;
	for(unsigned int nIndex = 0; nIndex < cInterpolated.size(); nIndex++)
	{
		if (szPrevious != cInterpolated[nIndex].ipa)
		{
			dBreakPoint = nIndex - 0.5;
			szPrevious = cInterpolated[nIndex].ipa;
		}

		if (cInterpolated[nIndex].duration == 0)
		{
			if (dBreakPoint >= nLastMarker && nIndex)
			{
				// Interpolate between markers
				Interpolate(cInterpolated, nLastMarker, dBreakPoint, nIndex);
			}
			nLastMarker = nIndex;
		}
	}

	{
		// Add pitch

		CString szFilename = m_szSourceFilename;

		CSaApp* pApp = (CSaApp*)AfxGetApp();
		CSaDoc* pDoc = (CSaDoc*)pApp->IsFileOpened(szFilename);
		if (!pDoc) return;

		enum {PITCH, CALCULATIONS};
		double fSizeFactor[CALCULATIONS];

		BOOL bPitch = TRUE;

		// CProcessGrappl* pAutoPitch = NULL;
		CProcessSmoothedPitch* pPitch = NULL;
		UttParm myUttParm;  UttParm* pUttParm = &myUttParm;
		pDoc->GetUttParm(pUttParm); // get sa parameters utterance member data
		UttParm cSavedUttParm;  UttParm* pSavedUttParm = &cSavedUttParm;

		if (bPitch)
		{
			pPitch = pDoc->GetSmoothedPitch(); // SDM 1.5 Test 11.0
			// pAutoPitch = pDoc->GetGrappl();
			pDoc->GetUttParm(pSavedUttParm); // save current smoothed pitch parameters
			pUttParm->nMinFreq = 40;
			pUttParm->nMaxFreq = 500;
			pUttParm->nCritLoud = 2;
			pUttParm->nMaxChange = 10;
			pUttParm->nMinGroup = 6;
			pUttParm->nMaxInterp = 7;
			if (pUttParm->nMinFreq != pSavedUttParm->nMinFreq
				|| pUttParm->nMaxFreq != pSavedUttParm->nMaxFreq
				|| pUttParm->nCritLoud != pSavedUttParm->nCritLoud
				|| pUttParm->nMaxChange != pSavedUttParm->nMaxChange
				|| pUttParm->nMinGroup != pSavedUttParm->nMinGroup
				|| pUttParm->nMaxInterp != pSavedUttParm->nMaxInterp)
				pPitch->SetDataInvalid();
			pDoc->SetUttParm(pUttParm);
			short int nResult = LOWORD(pPitch->Process(this, pDoc)); // process data
			pDoc->SetUttParm(pSavedUttParm); // restore smoothed pitch parameters
			if (nResult == PROCESS_ERROR) bPitch = FALSE;
			else if (nResult == PROCESS_CANCELED) return;
			else fSizeFactor[PITCH] = (double)pDoc->GetDataSize() / (double)(pPitch->GetDataSize() - 1);
		}

		double nTime = 0;

		fSizeFactor[PITCH] /= pDoc->GetBytesFromTime(0.001);

		// construct table entries
		for(unsigned int nIndex = 0; nIndex < cInterpolated.size(); nIndex++)
		{
			if (bPitch && cInterpolated[nIndex].duration)
			{
				double offset;
				double slope;

				CurveFitPitch(pDoc, fSizeFactor[PITCH], nTime,
					(nTime + cInterpolated[nIndex].duration), &offset, &slope);
				if (offset > 0)
				{
					offset += slope*(cInterpolated[nIndex].duration)/2.;
					cInterpolated[nIndex].parameters.F0 = offset;
				}

				nTime += cInterpolated[nIndex].duration;
			}
		}
		// Get rid of smoothed pitch data so it doesn't interfere with an existing graph
		pDoc->GetSmoothedPitch()->SetDataInvalid();
	}

	PopulateParameterGrid(cGrid, cInterpolated, TRUE);
}

void CDlgKlattAll::OnFileOpen() 
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
		} while (uRead);

		CString szExt(dlg.GetFileExt());
		szExt.MakeUpper();
		m_cGrid[m_nSelectedView].ClearRange(rowFirst, columnFirst, rowATV, m_cGrid[m_nSelectedView].GetCols(0) - 1);
		if (szExt == "GRD")
			m_cGrid[m_nSelectedView].LoadRange(0, 0, szData, FALSE);
		else
			m_cGrid[m_nSelectedView].LoadRange(rowFirst, columnFirst, szData, TRUE);
	}
}

void CDlgKlattAll::OnUpdateFileOpen(CCmdUI* ) 
{
}

void CDlgKlattAll::OnFileSaveAs() 
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

			for(int i = 0; i < szData.GetLength(); i++)
			{
				if (szData[i] == _T('\t'))
				{
					nColumns++;
					if (nColumns > nColumnsMax)
						nColumnsMax = nColumns;
				}
				else if (szData[i] == _T('\n'))
					nColumns = 1;
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

#define NEXTVALUE_ERROR (-1.111)
static double NextValue(const TCHAR * szString,unsigned &uIndex)
{
	double iReturn = 0;
	CString szField;
	while(szString[uIndex] != 0 && szString[uIndex] != '\n')
	{ 
		szField += szString[uIndex];
		uIndex++;
	}
	if (!szString[uIndex]) return NEXTVALUE_ERROR;
	swscanf_s(szField,_T("%lf"),&iReturn);
	uIndex++;  // skip field delimiter

	return iReturn;
}

void CDlgKlattAll::OnSmoothe(void)
{
	//long columnLast = m_cGrid[m_nSelectedView].GetCols(0);

	for(register int cRow = rowF0; cRow<rowA8V; ++cRow)
	{
		CString szData = m_cGrid[m_nSelectedView].SaveRange(cRow, columnFirst, cRow+1, m_cGrid[m_nSelectedView].GetCols(0), TRUE);
		if (!szData.GetLength())
			continue;

		unsigned iData = (unsigned) szData.GetLength();

		unsigned uIndex = 0;
		double iValue[4] = {0,0,0,0};

		iValue[0] = NextValue((LPCTSTR)szData,uIndex);
		if (iValue[0]== NEXTVALUE_ERROR) return;
		iValue[1] = NextValue((LPCTSTR)szData,uIndex);
		if (iValue[1]== NEXTVALUE_ERROR) return;
		iValue[2] = NextValue((LPCTSTR)szData,uIndex);
		if (iValue[2]== NEXTVALUE_ERROR) return;
		iValue[3] = NextValue((LPCTSTR)szData,uIndex);
		if (iValue[3]== NEXTVALUE_ERROR) return;

		CString szOutstring = "";
		char szNumber[100];

		for(;uIndex<iData;)
		{
			// if next value is further than the one following,
			// make next value 1/2 way to the following one
#define ABdistance() fabs(iValue[0]-iValue[1])
#define ACdistance() fabs(iValue[0]-iValue[2])
			if ( ACdistance()*2<ABdistance())
			{
				iValue[1] = iValue[0] - ((iValue[0]-iValue[2]) / 2);
			}
			// output the number
			_gcvt_s(szNumber,_countof(szNumber),iValue[0],6);
			szOutstring += szNumber;
			szOutstring += _T("\n");
			// shift values to prepare for next one
			iValue[0] = iValue[1];
			iValue[1] = iValue[2];
			iValue[2] = iValue[3];
			iValue[3] = NextValue(szData,uIndex);

			if (iValue[3] == NEXTVALUE_ERROR) // finished, output stored values
			{
				_gcvt_s(szNumber,_countof(szNumber),iValue[0],6);
				szOutstring += szNumber;
				szOutstring += _T("\n");
				_gcvt_s(szNumber,_countof(szNumber),iValue[1],6);
				szOutstring += szNumber;
				szOutstring += _T("\n");
				_gcvt_s(szNumber,_countof(szNumber),iValue[2],6);
				szOutstring += szNumber;
				szOutstring += _T("\n");
				break;
			}
		}

		m_cGrid[m_nSelectedView].LoadRange(cRow, columnFirst, szOutstring, TRUE);
	}

} 


// This function labels the document with the grid parameters
// The function makes assumptons about how the synthesis function.
// The labels are placed at the start of the time in which they would be commanded to the 
// synthesis engine
void CDlgKlattAll::LabelDocument(CSaDoc* pDoc) 
{
	CIpaCharVector cChars;
	ParseParameterGrid(m_nSelectedMethod, cChars);

	CMusicPhraseSegment* pIndexSeg = (CMusicPhraseSegment*)pDoc->GetSegment(MUSIC_PL1);
	CPhoneticSegment* pCharSeg = (CPhoneticSegment*)pDoc->GetSegment(PHONETIC);
	POSITION pos = pDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*) pDoc->GetNextView(pos);

	// change to cursor alignment to sample mode
	CURSOR_ALIGNMENT nOldCursorAlignment = pView->GetCursorAlignment();
	pView->ChangeCursorAlignment(ALIGN_AT_SAMPLE);

	// synthesize wavefile
	SPKRDEF spkrDef = m_cConstants;
	spkrDef.UI = (m_cConstants.SR * m_cConstants.UI + 500)/ 1000;

	double time = 0;
	double timeScale = m_dTimeScale;

	double elapsedTime = 0;
	double labelTime = 0;
	CString szIPANext = "";
	double lastCharStopTime = 0;
	for(unsigned int i = 0;i < cChars.size();i++)
	{
		time += timeScale*cChars[i].duration/1000.;

		int nFrame = int(time*spkrDef.SR/spkrDef.UI+0.5);

		if (labelTime <= elapsedTime)
		{
			if (cChars[i].duration)
			{
				const double minLabelTime = 0.005;
				DWORD dwStart = DWORD(elapsedTime*spkrDef.SR+0.5)*2;
				double length = minLabelTime > double(nFrame*spkrDef.UI)/spkrDef.SR ? minLabelTime : double(nFrame*spkrDef.UI)/spkrDef.SR;
				DWORD dwDuration = DWORD(length*spkrDef.SR+0.5)*2;
				CSaString szIndex;

				szIndex.Format(_T("%d"),i+1);

				pIndexSeg->Insert(pIndexSeg->GetOffsetSize(), &szIndex, true, dwStart, dwDuration);

				labelTime = elapsedTime + length;
			}
		}
		elapsedTime += double(nFrame*spkrDef.UI)/spkrDef.SR;
		time -= double(nFrame*spkrDef.UI)/spkrDef.SR;

		if (i == cChars.size() - 1)
			szIPANext = "";
		else
			szIPANext = cChars[i+1].ipa;

		if (cChars[i].ipa != szIPANext)
		{
			DWORD dwStart = DWORD(lastCharStopTime*spkrDef.SR+0.5)*2;
			double length = elapsedTime - lastCharStopTime;
			DWORD dwDuration = DWORD(length*spkrDef.SR+0.5)*2;
			CSaString szIpa(cChars[i].ipa);
			pCharSeg->Insert(pCharSeg->GetOffsetSize(), &szIpa, true, dwStart, dwDuration);
			lastCharStopTime += length;
		}
	}
	// change cursor mode back
	pView->ChangeCursorAlignment(nOldCursorAlignment);

	pView->PostMessage(WM_COMMAND, ID_PHRASE_L1_RAWDATA);
}

// These command handler set the synthesis elongation option to create lengthen the 
// synthesized wave file by a scale factor (2x currently)
void CDlgKlattAll::OnKlattElongate() 
{
	if (m_dTimeScale <= 1.0)
		m_dTimeScale = 2.0;
	else
		m_dTimeScale = 1.0;
}

void CDlgKlattAll::OnUpdateKlattElongate(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_dTimeScale > 1.0);
}

// ****************************************** //
void CDlgKlattAll::OnEditCopy() 
{
	m_cGrid[m_nSelectedView].OnEditCopy();
}

void CDlgKlattAll::OnEditClear() 
{
	m_cGrid[m_nSelectedView].OnEditClear();
}

void CDlgKlattAll::OnEditCut() 
{
	m_cGrid[m_nSelectedView].OnEditCut();
}

void CDlgKlattAll::OnEditPaste() 
{
	m_cGrid[m_nSelectedView].OnEditPaste();
}

void CDlgKlattAll::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	m_cGrid[m_nSelectedView].OnUpdateEditPaste(pCmdUI);
}

void CDlgKlattAll::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	m_cGrid[m_nSelectedView].OnUpdateEditCopy(pCmdUI);
}

void CDlgKlattAll::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	m_cGrid[m_nSelectedView].OnUpdateEditCut(pCmdUI);
}

void CDlgKlattAll::OnUpdateEditClear(CCmdUI* pCmdUI) 
{
	m_cGrid[m_nSelectedView].OnUpdateEditClear(pCmdUI);
}

/***************************************************************************/
// CDlgKlattAll::OnKlattHelp
/***************************************************************************/
void CDlgKlattAll::OnKlattHelp() 
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath = szPath.Left(szPath.ReverseFind('\\'));
	szPath = szPath.Left(szPath.ReverseFind('\\')) + "\\HELP\\SYNTHESIS2.HLP";
	::WinHelp(AfxGetMainWnd()->GetSafeHwnd(), szPath, HELP_INDEX, 0);	
}

/***************************************************************************/
// extractTabField local helper function to get field from tab delimited string
/***************************************************************************/
static const CString extractTabField(const CString& szLine, const int nField)
{
	int nCount = 0;
	int nLoop = 0;

	if (nField < 0) return ""; // SDM 1.5Test10.1

	while((nLoop < szLine.GetLength()) && (nCount < nField))
	{
		if (szLine[nLoop] == '\t')
			nCount++;
		nLoop++;
	}
	int nBegin = nLoop;
	while((nLoop < szLine.GetLength()) && (szLine[nLoop] != '\t'))
	{
		nLoop++;
	}
	return szLine.Mid(nBegin, nLoop-nBegin);
}

void CIpaCharVector::Load(CString szPath)
{
	// is there a saved map???
	CFileStatus fileStatus; // file status

	reserve(1024);  // growth is exponential save some time here

	if (!CFile::GetStatus(szPath, fileStatus)  || !fileStatus.m_size)
		return;

	CStdioFile file;

	file.Open(szPath,CFile::modeRead | CFile::shareDenyWrite);

	const PARAMETER_DESC *parameterInfo = GetTemporalKlattDesc();

	CString line;
	while(file.ReadString(line))
	{
		TEMPORAL cTemporal;

		if (line[0] == '#') // skip comments
			continue; 

		for(int i=0;parameterInfo[i].parameterOffset != -1; i++)
		{
			TEMPORAL *pTemporal = &cTemporal;

			CString value;

			value = extractTabField(line, i+1);
			swscanf_s(value, parameterInfo[i].typeScanf, ((char*)pTemporal)+parameterInfo[i].parameterOffset);
		}

		this->push_back(CIpaChar(extractTabField(line, 0), cTemporal));
	}

	CIpaCharVector(*this).swap(*this); // shrink size using swap trick from Effective STL #18

	file.Close();
}

void CIpaCharVector::Save(CString szPath)
{
	CFile file;

	UINT nSuccess = file.Open(szPath,CFile::modeWrite | CFile::modeCreate | CFile::shareDenyWrite);
	if (!nSuccess)
	{ 
		//MsgBox(GetDesktopWindow(), "Could not open IpaDefaults.txt. Check Read-Only status.", "IPADefaults", MB_OK);
		::MessageBox(::GetDesktopWindow(),_T("Could not open IpaDefaults.txt.\n    Check Read-Only status."),
			_T("IPADefaults"),MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	const PARAMETER_DESC *parameterInfo = GetTemporalKlattDesc();

	const_iterator pParm;

	for(pParm = begin();pParm != end(); pParm++)
	{
		CString entry;

		TEMPORAL const *pTemporal = &pParm->parameters;

		entry = pParm->ipa + "\t";
		for(int i=0;parameterInfo[i].parameterOffset != -1; i++)
		{
			CString value;
			if (parameterInfo[i].typeScanf[1] != 'd')
				value.Format(parameterInfo[i].typeScanf, *(Float*)(((char*)pTemporal)+parameterInfo[i].parameterOffset));
			else
				value.Format(parameterInfo[i].typeScanf, *(int*)(((char*)pTemporal)+parameterInfo[i].parameterOffset));

			entry += value + "\t";
		}

		entry += "\r\n";

		file.Write(entry, entry.GetLength());
	}
}

CIpaCharMap::CIpaCharMap(CIpaCharVector &vect)
{
	CIpaCharVector::const_iterator pChar = vect.begin();
	for(pChar = vect.begin();pChar != vect.end();pChar++)
	{
		(*this)[pChar->ipa] = pChar->parameters;
	}
}


void CDlgKlattAll::OnIntervalNFrag(UINT nID) 
{
	nKlattFrameIntervalFragments = nID - ID_SYNTHESIS_KLATT_INTERVAL_0FRAG;
}

void CDlgKlattAll::OnUpdateIntervalNFrag(CCmdUI* pCmdUI) 
{
	UINT nID = pCmdUI->m_nID;

	pCmdUI->SetCheck(nKlattFrameIntervalFragments == nID - ID_SYNTHESIS_KLATT_INTERVAL_0FRAG);
	pCmdUI->Enable((nID - ID_SYNTHESIS_KLATT_INTERVAL_0FRAG) || (dKlattFrameIntervalMs > 0));
}

void CDlgKlattAll::OnIntervalNMs(UINT nID) 
{
	dKlattFrameIntervalMs = (nID - ID_SYNTHESIS_KLATT_INTERVAL_0MS)*5;
}

void CDlgKlattAll::OnUpdateIntervalNMs(CCmdUI* pCmdUI) 
{
	UINT nID = pCmdUI->m_nID;

	pCmdUI->SetCheck(dKlattFrameIntervalMs == (nID - ID_SYNTHESIS_KLATT_INTERVAL_0MS)*5);
	pCmdUI->Enable((nID - ID_SYNTHESIS_KLATT_INTERVAL_0MS) || (nKlattFrameIntervalFragments > 0));
}

void CDlgKlattAll::OnWindowNFrag(UINT nID) 
{
	nKlattFrameWindowFragments = 	nID - ID_SYNTHESIS_KLATT_WINDOW_0FRAG;
}

void CDlgKlattAll::OnUpdateWindowNFrag(CCmdUI* pCmdUI) 
{
	UINT nID = pCmdUI->m_nID;

	pCmdUI->SetCheck(nKlattFrameWindowFragments ==	nID - ID_SYNTHESIS_KLATT_WINDOW_0FRAG);
	pCmdUI->Enable((nID - ID_SYNTHESIS_KLATT_WINDOW_0FRAG) || (dKlattFrameWindowMs > 0));
}

void CDlgKlattAll::OnWindowNMs(UINT nID) 
{
	dKlattFrameWindowMs = (nID - ID_SYNTHESIS_KLATT_WINDOW_0MS)*5;	
}

void CDlgKlattAll::OnUpdateWindowNMs(CCmdUI* pCmdUI) 
{
	UINT nID = pCmdUI->m_nID;

	pCmdUI->SetCheck(dKlattFrameWindowMs == (nID - ID_SYNTHESIS_KLATT_WINDOW_0MS)*5);
	pCmdUI->Enable((nID - ID_SYNTHESIS_KLATT_WINDOW_0MS) || (nKlattFrameWindowFragments > 0));
}

void CDlgKlattAll::OnAdjustCells() 
{
	CDlgSynthesisAdjustCells dlg;

	if (dlg.DoModal() == IDOK)
	{
		long rowEnd = m_cGrid[m_nSelectedView].GetRowSel();
		long colEnd = m_cGrid[m_nSelectedView].GetColSel();
		long row = m_cGrid[m_nSelectedView].GetRow();
		long col = m_cGrid[m_nSelectedView].GetCol();

		for(long y = row; y <= rowEnd; y++)
		{
			for(long x = col; x <= colEnd; x++)
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

void CDlgKlattAll::OnSynthHide() 
{
	m_bMinimize = !m_bMinimize;	
}

void CDlgKlattAll::OnUpdateSynthHide(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bMinimize);	
}

