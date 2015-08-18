/////////////////////////////////////////////////////////////////////////////
// sa_graph.cpp:
// Implementation of the CGraphWnd class.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//    1.06.6U4
//         SDM changed ScrollGraph to scroll exact number of pixels
//    1.06.6U5
//         SDM Change ScrollGraph to call RedrawGraph on page scroll
//         SDM Added SetCursor calls to RedrawGragh
//    1.06.6U6
//         SDM Added function SetPlaybackPosition
//    1.5Test8.1
//         SDM Added m_anAnnWndOrder[]
//         SDM Added support for Reference annotation
//    1.5Test11.1A
//         RLJ Add SetSpectroAB call so we know which set (A or B) of
//             parameters to save.
//    06/06/2000
//         RLJ Added SetProperties(int nNewID)
//    06/17/2000
//         RLJ Extend FileOpenAs to support not only Phonetic/Music Analysis,
//               but also OpenScreenF, OpenScreenG, OpenScreenI, OpenScreenK,
//               OpenScreenM, etc.
//    07/25/2000
//         RLJ Add check for non-null pMain to destructor
//
//    08/16/2000
//         DDO Fixed a problem in the update status bar function where
//             the auto pitch process object was being pointed to for
//             melograms. I changed it so the melogram process object
//             is being pointed to. Since the code, otherwise, is the
//             same for pitch as it is for melogram, I have
//
//    08/30/2000
//         DDO Made the recording graph an area graph.
//
//    09/22/2000
//         DDO Added a recording class derived from CGraphWnd. This allows
//             the PreCreateWindow to be overridden for more control over
//             the recording window's creation.
//
//    09/26/2000
//         DDO Added code to redraw the TWC graph when the melogram is
//             redrawn.
//
//    09/13/2000
//         DDO Added code in ShowLegend to make sure the melogram and
//             magnitude legend states are the same.
//
//    10/23/2000
//         DDO Added a fix so the legend on the melogram and magnitude
//             is turned on by default when there is no TWC graph.
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa.h"
#include "sa_minic.h"
#include "sa_graph.h"
#include "sa_plot.h"
#include "SpectroParm.h"
#include "Segment.h"
#include "math.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_g_3dPitch.h"
#include "sa_g_wavelet.h"                   // ARH 8/2/01 Added for wavelet graph
#include "sa_g_raw.h"
#include "sa_g_glo.h"
#include "sa_g_lou.h"
#include "sa_g_dur.h"
#include "sa_g_zcr.h"
#include "sa_g_pit.h"
#include "sa_g_cpi.h"
#include "sa_g_spi.h"
#include "sa_g_cha.h"
#include "sa_g_spe.h"
#include "sa_g_spg.h"
#include "sa_g_spu.h"
#include "sa_g_poa.h"
#include "sa_g_gra.h"
#include "sa_g_rec.h"
#include "sa_g_mel.h"
#include "sa_g_twc.h"
#include "sa_g_stf.h"
#include "sa_g_mag.h"
#include "sa_gZ3D.h"
#include "sa_g_rat.h"
#include "sa_gZpol.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "printdim.h"
#include "sa_mplot.h"
#include "time.h"
#include "Process\Process.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_cha.h"
#include "Process\sa_p_dur.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_pit.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_mel.h"
#include "Process\sa_p_spi.h"
#include "Process\sa_p_cpi.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_spg.h"
#include "Process\sa_p_twc.h"
#include "Process\sa_p_zcr.h"
#include "Process\FormantTracker.h"
#include "objectostream.h"
#include "LegendWnd.h"
#include "ReferenceWnd.h"
#include "GlossNatWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//###########################################################################
// CGraphWnd
// Base class for all graph windows. Does all jobs, common to the graphs.

IMPLEMENT_DYNCREATE(CGraphWnd, CMiniCaptionWnd)

/////////////////////////////////////////////////////////////////////////////
// CGraphWnd static members definition

DWORD CGraphWnd::m_dwLastStartCursor = UNDEFINED_OFFSET; // undefined
DWORD CGraphWnd::m_dwLastStopCursor = UNDEFINED_OFFSET; // undefined

BEGIN_MESSAGE_MAP(CGraphWnd, CMiniCaptionWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_SETFOCUS()
    ON_MESSAGE(WM_USER_INFO_GENDERCHANGED, OnGenderInfoChanged)
    ON_WM_NCMOUSEMOVE()
    ON_WM_DESTROY()
    ON_WM_CHAR()
END_MESSAGE_MAP()

EAnnotation CGraphWnd::m_anAnnWndOrder[] = { REFERENCE, PHONETIC, TONE, PHONEMIC, ORTHO, GLOSS, GLOSS_NAT, MUSIC_PL1, MUSIC_PL2, MUSIC_PL3, MUSIC_PL4};

/***************************************************************************/
// CGraphWnd::CGraphWnd Constructor
/***************************************************************************/
CGraphWnd::CGraphWnd(UINT nID) :
m_PopupMenuPos(UNDEFINED_OFFSET,UNDEFINED_OFFSET),
m_pPlot(NULL),
m_pLegend(NULL),
m_pXScale(NULL) {
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        m_apAnnWnd[nLoop] = NULL;
        m_abAnnWnd[nLoop] = FALSE;
    }
    m_bBoundaries = false;
    m_pPlot = NULL;
    m_pLegend = NULL;
    m_pXScale = NULL;
    m_bLegend = FALSE;
    m_bXScale = FALSE;
    m_bAreaGraph = FALSE;
    m_pPlot = NewPlotFromID(nID);
    m_nPlotID = nID;
    if (!m_pPlot) {
        m_pPlot = new CPlotBlank();
        m_pPlot->SetParent(this);
        m_nPlotID = IDD_BLANK;
    }
}

CGraphWnd::CGraphWnd(const  CGraphWnd & right) :
m_PopupMenuPos(UNDEFINED_OFFSET,UNDEFINED_OFFSET),
m_pPlot(NULL),
m_pLegend(NULL),
m_pXScale(NULL) {
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        m_apAnnWnd[nLoop] = NULL;
        m_abAnnWnd[nLoop] = FALSE;
    }
    Copy(right);
}

/***************************************************************************/
// CGraphWnd::~CGraphWnd Destructor
/***************************************************************************/
CGraphWnd::~CGraphWnd() {
    Clear();
}

CGraphWnd & CGraphWnd::operator=(const CGraphWnd & fromThis) {
    if (&fromThis != this) {
        Clear();
        Copy(fromThis);
    }
    return *this;
}

/***************************************************************************/
/***************************************************************************/
void CGraphWnd::Clear(void) {
    if (m_pPlot!=NULL) {
        delete m_pPlot;
		m_pPlot = NULL;
    }
    if (m_pLegend!=NULL) {
        delete m_pLegend;
		m_pLegend = NULL;
    }
    if (m_pXScale!=NULL) {
        delete m_pXScale;
		m_pXScale = NULL;
    }
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        if (m_apAnnWnd[nLoop]!=NULL) {
            delete m_apAnnWnd[nLoop];
			m_apAnnWnd[nLoop] = NULL;
        }
    }
}

/***************************************************************************/
/***************************************************************************/
void CGraphWnd::Copy( const CGraphWnd & right) {
	
	if (m_pPlot!=NULL) {
		delete m_pPlot;
		m_pPlot = NULL;
	}
    if (m_pLegend != NULL) {
		delete m_pLegend;
		m_pLegend = NULL;
	}
    m_pXScale = NULL;
    m_nPlotID = right.m_nPlotID;
    m_bAreaGraph = right.m_bAreaGraph;
    if (m_nPlotID == ID_GRAPHS_OVERLAY) {
        m_nPlotID = IDD_BLANK;
        CMultiPlotWnd * pPlot = (CMultiPlotWnd *)right.GetPlot();
        if ((pPlot!=NULL) && (pPlot->IsKindOf(RUNTIME_CLASS(CMultiPlotWnd)))) {
			// save base plot
            m_nPlotID = pPlot->GetBasePlotID();  
        }
    }
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		if (m_apAnnWnd[nLoop]!=NULL) {
			delete m_apAnnWnd[nLoop];
			m_apAnnWnd[nLoop] = NULL;
		}
    }

	PartialCopy(right);
}

void CGraphWnd::PartialCopy( const CGraphWnd & right) {
	// perform a partial copy
	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        m_abAnnWnd[nLoop] = right.m_abAnnWnd[nLoop];
    }
    m_bLegend = right.m_bLegend;
    m_bXScale = right.m_bXScale;
	m_bBoundaries = right.m_bBoundaries;
}

/***************************************************************************/
// CGraphWnd::DisableLegend Is legend window disabled for this graph type
/***************************************************************************/
BOOL CGraphWnd::DisableLegend() {
    return FALSE;
}

/***************************************************************************/
// CGraphWnd::ShowLegend Show or hide the legend window
/***************************************************************************/
void CGraphWnd::ShowLegend(BOOL bShow, BOOL bRedraw) {
    
	m_bLegend = ((bShow) && (!DisableLegend()));

    //***********************************************
    // 10/13/2000 - DDO Make sure the melogram and
    // magnitude legend states are the same. But
    // make sure we don't get into a recursion stack
    // overflow when calling ShowLegend.
    //***********************************************
    static BOOL bRecursion;

    if (!bRecursion) {
        if (IsPlotID(IDD_MELOGRAM)) {
            CSaView * pView = (CSaView *)GetParent();
            CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_MAGNITUDE);
            bRecursion = TRUE;
            if (pGraph!=NULL) {
                pGraph->ShowLegend(m_bLegend, bRedraw);
            }
            bRecursion = FALSE;
        } else if (IsPlotID(IDD_MAGNITUDE)) {
            CSaView * pView = (CSaView *)GetParent();
            CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_MELOGRAM);
            bRecursion = TRUE;
            if (pGraph!=NULL) {
                pGraph->ShowLegend(m_bLegend, bRedraw);
            }
            bRecursion = FALSE;
        }
    }

    if (bRedraw) {
		// repaint whole graph window
        ResizeGraph(TRUE, TRUE);    
    }
}

/***************************************************************************/
// CGraphWnd::DisableXScale Is x-scale window disabled for this graph type
/***************************************************************************/
BOOL CGraphWnd::DisableXScale() {
    BOOL result = FALSE;
    switch (m_nPlotID) {
    case IDD_STAFF:
    case IDD_MAGNITUDE:
        result = TRUE;
        break;
    default:
        result = FALSE;
    }
    return result;
}

/***************************************************************************/
// CGraphWnd::ShowXScale Show or hide the x-scale window
/***************************************************************************/
void CGraphWnd::ShowXScale(BOOL bShow, BOOL bRedraw) {
    m_bXScale = bShow && !DisableXScale();
    if (bRedraw) {
		// repaint whole graph window
        ResizeGraph(TRUE, TRUE);    
    }
}

/***************************************************************************/
// CGraphWnd::DisableAnnotation Is annotation window disabled for this graph type
/***************************************************************************/
BOOL CGraphWnd::DisableAnnotation(int /*nIndex*/) {
    BOOL result = FALSE;

    switch (m_nPlotID) {
    case IDD_STAFF:
    case IDD_MAGNITUDE:
    case IDD_F1F2:
    case IDD_F2F1:
    case IDD_F2F1F1:
    case IDD_3D:
        result = TRUE;
        break;
    default:
        result = FALSE;
    }
    return result;
}

/***************************************************************************/
// CGraphWnd::ShowAnnotation Show or hide annotation window
/***************************************************************************/
void CGraphWnd::ShowAnnotation(EAnnotation nIndex, BOOL bShow, BOOL bRedraw) {

    m_abAnnWnd[nIndex] = ((bShow) && (!DisableAnnotation(nIndex)));
    if (bRedraw) {
        ResizeGraph(TRUE, TRUE);    // repaint whole graph window
    }
}

/***************************************************************************/
// CGraphWnd::SetCaptionStyle Set window caption style
// Set the style of the window caption. If the caption has to be redrawn,
// it invalidates the caption rectangle of the window in the parent view
// (because this area is defined as non client, it cannot be invalidated
// directly). This will cause a draw message.
/***************************************************************************/
void CGraphWnd::SetCaptionStyle(int nStyle, BOOL bRedraw) {
    m_nCaption = nStyle;
    // set caption height
    switch (m_nCaption) {
    case Normal:
        m_iNCHeight = GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER);
        break;
    case Mini:
    case MiniWithCaption:
        m_iNCHeight = MINICAPTION_HEIGHT;
        break;
    default:
		// because of 3D border
        m_iNCHeight = 1; 
        break;
    }
    if (bRedraw) {
        // force a repaint of the whole window with NC area
		// get pointer to parent view
        CSaView * pView = (CSaView *)GetParent(); 
        // invalidate whole window
        CRect rWnd;
        GetWindowRect(&rWnd);
        pView->ScreenToClient(rWnd);
        pView->InvalidateRect(&rWnd, TRUE);
    }
}

/***************************************************************************/
// CGraphWnd::ScrollGraph Scroll a graph
// The scrolling is done in pixel units, the graphs are also
// drawn in pixel units.  The graph will not scroll, if the cursors
// are not visible, but just redraw to show the new annotations.
// For area graphs it does not do anything.
//***************************************************************************/
void CGraphWnd::ScrollGraph(CSaView * pView, DWORD dwNewPos, DWORD dwOldPos) {

    // don't process area graphs
    if (m_bAreaGraph) {
        return;
    }

    if (!m_pPlot->HasCursors()) {
        // no cursors visible
        if (HasPrivateCursor()) {
			// don't do anything
            return;    
        } else {
			// redraw graph, without legend
            RedrawGraph();    
        }
        return;
    }

    CRect rWnd;
    GetClientRect(rWnd);
    if (m_bLegend) {
        rWnd.left = m_pLegend->GetWindowWidth();
    }

    if (m_pPlot) {
        m_pPlot->GetClientRect(rWnd);
    }

	// number of data points displayed
    DWORD dwDataFrame = pView->AdjustDataFrame(rWnd.Width()); 

    BOOL bLessThanPage = ((dwNewPos > dwOldPos) && ((dwNewPos - dwOldPos) < dwDataFrame)) ||
                         ((dwOldPos > dwNewPos) && ((dwOldPos - dwNewPos) < dwDataFrame));
    bLessThanPage = FALSE;

    // check if there is less than a page to scroll
    if (bLessThanPage) {
        // prepare scrolling
		// calculate data samples per pixel
        double fBytesPerPix = dwDataFrame / (double)rWnd.Width(); 
        int nRealScroll = round2Int((double)dwOldPos/fBytesPerPix)-round2Int((double)dwNewPos/fBytesPerPix);

        if (nRealScroll) {
            // scroll the plot window
            m_pPlot->ScrollPlot(pView, nRealScroll, dwOldPos, dwDataFrame);
            // scroll the annotation windows
            for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
                if (m_abAnnWnd[nLoop]) {
                    m_apAnnWnd[nLoop]->ScrollWindow(nRealScroll, 0, 0, 0);
                    m_apAnnWnd[nLoop]->UpdateWindow();
                }
            }
            if (m_bXScale) {
                m_pXScale->InvalidateRect(NULL);
                m_pXScale->UpdateWindow();

            }
        }
    } else { 
		// page scroll
        RedrawGraph(TRUE, FALSE);
    }
}

/***************************************************************************/
// CGraphWnd::ResizeGraph Resize graph including plot and annotation windows
// If the flag bEntire is TRUE (default), the entire graph will be repainted,
// else only the part between (and with) the cursor windows (without
// annotation windows) are repainted, but only if boundaries are displayed.
// If the flag bGraph is set (default), the whole graph will be invalidated,
// if the two other flags are set, otherwise the graph will not be invalida-
// ted (only all its windows).
/***************************************************************************/
void CGraphWnd::ResizeGraph(BOOL bEntire, BOOL bLegend, BOOL bGraph) {
    
	if (bEntire) {
        // save bottom
        CRect rWnd;
        GetClientRect(rWnd);
        int nBottom = rWnd.bottom;
        // move legend window
        if (m_bLegend) {
            // invalidate graph without legend
            if (!bLegend) {
                rWnd.left = m_pLegend->GetWindowWidth();
            }
            if (bGraph) {
                InvalidateRect(rWnd);
            }
            // move legend window
            rWnd.right = m_pLegend->GetWindowWidth();
            rWnd.left = 0;
            ASSERT(rWnd.Width() == m_pLegend->GetWindowWidth());
            m_pLegend->Invalidate();
            m_pLegend->MoveWindow(rWnd, TRUE);
            GetClientRect(rWnd);
            rWnd.left = m_pLegend->GetWindowWidth();
        } else {
            // invalidate entire graph window
            InvalidateRect(NULL);
			// hide window
            m_pLegend->MoveWindow(0, 0, 0, 0, FALSE); 
        }
        // move annotation windows
        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
            // SDM 1.5Test8.1
            if (m_abAnnWnd[m_anAnnWndOrder[nLoop]]) {
                // move annotation window
                rWnd.bottom = rWnd.top + m_apAnnWnd[m_anAnnWndOrder[nLoop]]->GetWindowHeight();
                m_apAnnWnd[m_anAnnWndOrder[nLoop]]->MoveWindow(rWnd, TRUE);
                rWnd.top = rWnd.bottom;
            } else {
				// hide window
                m_apAnnWnd[m_anAnnWndOrder[nLoop]]->MoveWindow(0, 0, 0, 0, FALSE);
            }
        }
        // move x-scale window
        if (m_bXScale) {
            // move x-scale window
            rWnd.bottom = nBottom;
            nBottom = rWnd.top;
            rWnd.top = rWnd.bottom - m_pXScale->GetWindowHeight();
            m_pXScale->MoveWindow(rWnd, TRUE);
            rWnd.bottom = rWnd.top;
            rWnd.top = nBottom;
            nBottom = rWnd.bottom;
        } else {
			// hide window
            m_pXScale->MoveWindow(0, 0, 0, 0, FALSE);
        }
        // move plot window
        rWnd.bottom = nBottom;
        m_pPlot->MoveWindow(rWnd, TRUE);
        if (HasPrivateCursor()) {
			// initialize private cursor
            m_pPlot->SetInitialPrivateCursor();
        }
        // SDM 1.06.6U5
        if (m_pPlot->HasCursors()) {
            m_pPlot->SetStartCursor((CSaView *)GetParent());
            m_pPlot->SetStopCursor((CSaView *)GetParent());
            m_pPlot->SetPlaybackCursor((CSaView *)GetParent(),false);
        }

        //***********************************************************
        // 09/26/2000 - DDO If the graph we're Resizing is the
        // melogram graph then also Resize the TWC graph if it
        // exists.
        //***********************************************************
        if (IsPlotID(IDD_MELOGRAM)) {
            CSaView * pView = (CSaView *)GetParent();
            CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_TWC);
            if (pGraph!=NULL) {
                pGraph->ResizeGraph(TRUE, pGraph->HasLegend());
            }
        }
    } else {
        m_pPlot->RedrawPlot(bEntire);
    }
}

void CGraphWnd::RedrawPlot() {
	m_pPlot->RedrawPlot();
}

/***************************************************************************/
// CGraphWnd::RedrawGraph Redraw graph including plot and annotation windows
// If the flag bEntire is TRUE (default), the entire graph will be repainted,
// else only the part between (and with) the cursor windows (without
// annotation windows) are repainted, but only if boundaries are displayed.
// If the flag bGraph is set (default), the whole graph will be invalidated,
// if the two other flags are set, otherwise the graph will not be invalida-
// ted (only all its windows).
/***************************************************************************/
void CGraphWnd::RedrawGraph(BOOL bEntire, BOOL bLegend, BOOL bGraph) {
    // redraw the plot window
    m_pPlot->RedrawPlot(bEntire);

    if (bGraph) {
        InvalidateRect(NULL, FALSE);
    }

    if (bEntire) {
        // redraw the annotation windows
        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
            if (m_abAnnWnd[nLoop]) {
                m_apAnnWnd[nLoop]->InvalidateRect(NULL);
            }
        }

        if (m_bXScale) {
            m_pXScale->InvalidateRect(NULL);
        }

        if (m_bLegend && bLegend) {
            m_pLegend->InvalidateRect(NULL);
        }

        if (m_pPlot->HasCursors()) {
            m_pPlot->SetStartCursor((CSaView *)GetParent());
            m_pPlot->SetStopCursor((CSaView *)GetParent());
            m_pPlot->SetPlaybackCursor((CSaView *)GetParent(),false);
        }

        //***********************************************************
        // 09/26/2000 - DDO If the graph we're redrawing is the
        // melogram graph then also redraw the TWC graph if it
        // exists.
        //***********************************************************
        if (IsPlotID(IDD_MELOGRAM)) {
            CSaView * pView = (CSaView *)GetParent();
            CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_TWC);
            if (pGraph) {
                pGraph->RedrawGraph(TRUE, pGraph->HasLegend());
            }
        }
    }
}

/***************************************************************************/
// CGraphWnd::GetSemitone Calculates semitones from a given frequency
/***************************************************************************/
double CGraphWnd::GetSemitone(double fFreq) {
    // more accurate and compact way of calculating semitone
    // semi-tone 0 aligns with MIDI# 0 at 8.176Hz
    return log(fFreq / 440.0) / log(2.0) * 12.0 + 69.0;
}

/***************************************************************************/
// CGraphWnd::SemitoneToFrequency Calculates frequency from a given semitone
/***************************************************************************/
double CGraphWnd::SemitoneToFrequency(double fSemitone) {
    return 440.0 * pow(2.0, (fSemitone - 69.0) / 12.0);
}

/***************************************************************************/
// CGraphWnd::NoteNum2Name Calculates frequency from a given semitone
/***************************************************************************/
CSaString CGraphWnd::Semitone2Name(double fSemitone) {
    CSaString szNoteName;
    short nOctave;
    short nInterval;
    char sNoteNamesUtf8[24][5] = {"C ", "C‡", "C#", "Dd", "D ", "D‡",
                                  "Eb", "Ed", "E ", "E‡", "F ", "F‡",
                                  "F#", "Gd", "G ", "G‡", "G#", "Ad",
                                  "A ", "A‡", "Bb", "Bd", "B ", "B‡"
                                 };
    wchar_t pASCII[2] = {_T('\0'),_T('\0')};

    // limit range of input MIDI numbers
    if (fSemitone < 21.) {
        szNoteName = _T("   ");
        return szNoteName.GetBuffer(5);
    }
    // calculate interval and octave
    fSemitone -= 12; // MIDI starts one octave lower
    nOctave = (short)((fSemitone + 0.5) / 12.);
    nInterval = (short)(2. * (fSemitone - (double)nOctave * 12.) + 0.5);

    // construct note name
    szNoteName.setUtf8(sNoteNamesUtf8[nInterval]);
    _itow_s(nOctave, pASCII, _countof(pASCII), 10);
    szNoteName += pASCII;

    return CSaString(szNoteName);
}


static void UpdatePitchStatusBarNote(double fPitchData, double fUncertainty, BOOL bShowNoteName) {
    // get pointer to status bar
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CDataStatusBar * pStat = (CDataStatusBar *)pMainWnd->GetDataStatusBar();
    CSaString szText;

    if (fPitchData > 0.F) {
        // calculate frequency value
        double fFreqDecPlaces = (short)floor(-log10(fUncertainty) + .7);
        double fMultiplier = pow(10.0, -fFreqDecPlaces);
        fPitchData = round2Int(fPitchData / fMultiplier) * fMultiplier;
        if (fFreqDecPlaces < 0.) {
            fFreqDecPlaces = 0.;
        }
        // calculate semitone value
        double fSemitoneUncertainty = 12.0 * log((fPitchData + fUncertainty) / fPitchData) / log(2.);
        double fSemitoneDecPlaces = floor(-log10(fSemitoneUncertainty) + .7);
        // update semitone pane
        double fSemitone = CGraphWnd::GetSemitone(fPitchData);
        szText.Format(_T("      %.*f st"), (int)fSemitoneDecPlaces, fSemitone);
        if (bShowNoteName) {
            szText += _T(" (") + CSaString(CGraphWnd::Semitone2Name(fSemitone)) + _T(")");
        }
        CSaString test = CSaString(CGraphWnd::Semitone2Name(fSemitone));
        pStat->SetPaneSymbol(ID_STATUSPANE_NOTE);
        pStat->SetPaneText(ID_STATUSPANE_NOTE, szText);
        // update frequency pane
        szText.Format(_T("      %.*f Hz"), (int)fFreqDecPlaces, fPitchData);
        pStat->SetPaneSymbol(ID_STATUSPANE_PITCH); // switch symbol on
        pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);
    } else {
        szText = _T(" ");
        pStat->SetPaneSymbol(ID_STATUSPANE_NOTE); // amplitude symbol
        pStat->SetPaneText(ID_STATUSPANE_NOTE, szText);  // amplitude indicator

        pStat->SetPaneSymbol(ID_STATUSPANE_PITCH); // switch symbol on
        pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);
    }
}

static void UpdatePitchStatusBar(double fPitchData, double fUncertainty) {
    UpdatePitchStatusBarNote(fPitchData, fUncertainty, FALSE);
}

/***************************************************************************/
// CGraphWnd::UpdateStatusBar Update the content of the status bar
// If the parameters dwStartCursor and dwStopCursor did not change since
// the last time the status bar has been updated, it won't be updated unless
// the flag bForceUpdate is TRUE (forced update). If the two parameters for
// start- and stopcursor both contain 0, the update call comes from a graph
// with a private cursor.
//**************************************************************************/
void CGraphWnd::UpdateStatusBar(DWORD dwStartCursor, DWORD dwStopCursor, BOOL bForceUpdate) {
    // get pointer to parent view and to document
    CSaView * pView = (CSaView *)GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    UINT nSmpSize = pDoc->GetSampleSize();  // number of bytes per sample
    // check if this graph has focus
    if ((m_bFocus) && (pView->ViewIsActive())) {
        // get pointer to status bar
        CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
        CDataStatusBar * pStat = (CDataStatusBar *)pMainWnd->GetDataStatusBar();
        int nPositionMode = pMainWnd->GetStatusPosReadout();
        int nPitchMode = pMainWnd->GetStatusPitchReadout();

        TCHAR szText[32];
        int nProcessIndex, nData=0;
        DWORD dwDataPos;
        double fSizeFactor = 0.0;
        double fData = 0.0;
        BOOL bRes=FALSE;
        CRect rWnd;
        if (HasPrivateCursor()) {
            // this is a private cursor graph
            CProcessSpectrum * pSpectrum = NULL;
            CSpectrumParm * pSpectrumParm = NULL;
            CProcessTonalWeightChart * pTWC;
            switch (m_nPlotID) {
            case IDD_SPECTRUM:
                m_pPlot->GetClientRect(rWnd); // get plot window size
                nProcessIndex = m_pPlot->GetPrivateCursorPosition();
                pSpectrum = (CProcessSpectrum *)pDoc->GetSpectrum(); // get pointer to spectrum object

                if (pSpectrum->IsDataReady()) {
                    pSpectrumParm = pSpectrum->GetSpectrumParms();
                    double SigBandwidth = (double)pDoc->GetSamplesPerSec() / 2.0;
                    double ScaleFactor = 1. + pSpectrumParm->nFreqScaleRange;
                    int nFreqLowerBound = 0;
                    int nFreqUpperBound = (int)(ceil(SigBandwidth / ScaleFactor));

                    // write to frequency pane
                    //fData = (double)pSpectrumParm->nFreqLowerBound + (double)(pSpectrumParm->nFreqUpperBound - pSpectrumParm->nFreqLowerBound) * (double)nProcessIndex / (double)(rWnd.Width() - 1);
                    fData = (double)nFreqLowerBound + (double)(nFreqUpperBound - nFreqLowerBound) * (double)nProcessIndex / (double)(rWnd.Width() - 1);
                    if (nPitchMode == HERTZ) {
                        swprintf_s(szText, _countof(szText),_T("    %.1f Hz"), fData);
                    } else {
                        swprintf_s(szText, _countof(szText),_T("    %.1f st"), GetSemitone(fData));
                    }
                    pStat->SetPaneSymbol(ID_STATUSPANE_FREQUENCY); // switch symbol on
                    pStat->SetPaneText(ID_STATUSPANE_FREQUENCY, szText);

                    double PlotXScaleMax = (double)nFreqUpperBound;
                    double PlotXScaleMin = (double)nFreqLowerBound;
                    double PlotXScale = (PlotXScaleMax - PlotXScaleMin) / (double)rWnd.Width();

                    short nSpectBands = (short)pSpectrum->GetSpectralCount();
                    double SpectFreqScale = SigBandwidth / ((double)nSpectBands - 1.);

                    short LBandEdge = (short)max(0,(short)Round((double)(nProcessIndex-0.5) * PlotXScale / SpectFreqScale));
                    short UBandEdge = (short)min(nSpectBands-1,(short)Round((double)(nProcessIndex+0.5) * PlotXScale / SpectFreqScale));

                    bRes = FALSE;
                    if (pSpectrum->GetSpectralData(0).Smooth != (float)UNDEFINED_DATA) {
                        fData = 0.;
                        for (short i = LBandEdge; i < UBandEdge; i++) {
                            fData += (double)pSpectrum->GetSpectralData((unsigned short)i).Smooth;
                        }
                        if (UBandEdge != LBandEdge) {
                            fData /= (double)(UBandEdge - LBandEdge);    // average the power within the band
                        }
                        if (pSpectrumParm->nScaleMode == DB) {
                            fData = (fData == 0.)?MIN_LOG_PWR:10.*log10((double)fData) - pSpectrum->GetSpectralPowerRange().fdBRef;
                        }
                        bRes = TRUE;
                    } else {
                        bRes = FALSE;
                    }
                    swprintf_s(szText, _countof(szText),_T("Cep:"));
                    if (bRes && pSpectrumParm->bShowCepSpectrum) {
                        if (pSpectrumParm->nScaleMode) {
                            swprintf_s(szText, _countof(szText),_T("Cep: %5.1f"), fData);
                        } else {
                            swprintf_s(szText, _countof(szText),_T("Cep: %5.1f dB"), fData);
                        }
                    }
                    // write to cepstral spectrum pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_CEPSTRALSPECTRUM); // switch symbol on
                    pStat->SetPaneText(ID_STATUSPANE_CEPSTRALSPECTRUM, szText);

                    bRes = FALSE;
                    if (pSpectrum->GetSpectralData(0).Raw != (float)UNDEFINED_DATA) {

                        fData = 0.;
                        for (short i = LBandEdge; i < UBandEdge; i++) {
                            fData += (double)pSpectrum->GetSpectralData((unsigned short)i).Raw;
                        }
                        if (UBandEdge != LBandEdge) {
                            fData /= (double)(UBandEdge - LBandEdge);    // average the power within the band
                        }
                        if (pSpectrumParm->nScaleMode == DB) {
                            fData = (fData == 0.)?MIN_LOG_PWR:10.*log10((double)fData) - pSpectrum->GetSpectralPowerRange().fdBRef;
                        }
                        bRes = TRUE;
                    } else {
                        bRes = FALSE;
                    }
                    swprintf_s(szText, _countof(szText),_T("Raw:"));
                    if (bRes) {
                        if (pSpectrumParm->nScaleMode) {
                            swprintf_s(szText, _countof(szText),_T("Raw: %5.1f"), fData);
                        } else {
                            swprintf_s(szText, _countof(szText),_T("Raw: %5.1f dB"), fData);
                        }
                    }
                    // write to raw spectrum pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_RAWSPECTRUM); // switch symbol on
                    pStat->SetPaneText(ID_STATUSPANE_RAWSPECTRUM, szText);

                    bRes = FALSE;
                    if (pSpectrum->GetSpectralData(0).Lpc != (float)UNDEFINED_DATA) {
                        fData = 0.;
                        for (short i = LBandEdge; i < UBandEdge; i++) {
                            fData += (double)pSpectrum->GetSpectralData((unsigned short)i).Lpc;
                        }
                        if (UBandEdge != LBandEdge) {
                            fData /= (double)(UBandEdge - LBandEdge);    // average the power within the band
                        }
                        double LpcRef;
                        double MaxLpcBandPwr = pSpectrum->GetSpectralPowerRange().Max.Lpc;
                        double MaxRawBandPwr = pSpectrum->GetSpectralPowerRange().Max.Raw;
                        if (pSpectrumParm->nScaleMode == DB) {
                            if (MaxRawBandPwr != (float)UNDEFINED_DATA) {
                                MaxRawBandPwr = (MaxRawBandPwr == 0.)?MIN_LOG_PWR:10.*log10((double)MaxRawBandPwr) - pSpectrum->GetSpectralPowerRange().fdBRef;
                                MaxLpcBandPwr = (MaxLpcBandPwr == 0.)?MIN_LOG_PWR:10.*log10((double)MaxLpcBandPwr);
                                LpcRef = MaxLpcBandPwr - MaxRawBandPwr;
                            } else {
                                LpcRef = pSpectrum->GetSpectralPowerRange().fdBRef;
                            }
                            fData = (fData == 0.)?MIN_LOG_PWR:(10.*log10(fData) - LpcRef);
                        } else {
                            if (MaxRawBandPwr != (float)UNDEFINED_DATA && MaxRawBandPwr != 0.F) {
                                LpcRef = MaxLpcBandPwr / MaxRawBandPwr;
                            } else {
                                LpcRef = pow(10.F, pSpectrum->GetSpectralPowerRange().fdBRef / 10.F);
                            }
                            fData /= LpcRef;
                        }
                        bRes = TRUE;
                    } else {
                        bRes = FALSE;
                    }
                    swprintf_s(szText, _countof(szText),_T("LPC:"));
                    if (bRes) {
                        if (pSpectrumParm->nScaleMode) {
                            swprintf_s(szText, _countof(szText),_T("LPC: %5.1f"), fData);
                        } else {
                            swprintf_s(szText, _countof(szText),_T("LPC: %5.1f dB"), fData);
                        }
                    }
                    // write to lpc spectrum pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_LPCSPECTRUM); // switch symbol on
                    pStat->SetPaneText(ID_STATUSPANE_LPCSPECTRUM, szText);

                }
                break;
            case IDD_TWC:
                pTWC = (CProcessTonalWeightChart *)pDoc->GetTonalWeightChart(); // get pointer to TWC chart
                bRes = TRUE;
                if (pTWC->IsDataReady()) {
                    m_pPlot->UpdateWindow(); // force correct legend settings
                    m_pPlot->GetClientRect(rWnd); // get plot window size
                    int nCursorHeight = (short)(rWnd.Height() - (m_pPlot->GetPrivateCursorPosition()));
                    CLegendWnd * pLegend = GetLegendWnd();
                    double fSemitoneOffset = -static_cast<CPlotTonalWeightChart *>(m_pPlot)->GetSemitoneOffset();
                    double fSemitone;
                    double fMinScale  = pLegend->GetScaleMinValue() + fSemitoneOffset;
                    if (nCursorHeight) {
                        double fSemitoneAbs = double(nCursorHeight)/(pLegend->GetGridDistance() / pLegend->GetScaleBase()) + fMinScale;
                        double fBinsPerSemitone = pTWC->GetBinsPerSemitone();
                        dwDataPos = DWORD((fSemitoneAbs - pTWC->GetMinSemitone())*fBinsPerSemitone) + 1;
                        fSemitone = double(dwDataPos)/fBinsPerSemitone + pTWC->GetMinSemitone();
                        nData = pTWC->GetProcessedData(dwDataPos, &bRes);
                    } else {
                        fSemitone  = fMinScale - 0.1;
                        nData        = 0;
                    }

                    // write to amplitude pane
                    swprintf_s(szText, _countof(szText),_T("     %3.2f st (%3s)"), fSemitone, Semitone2Name(fSemitone));

                    //                 pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE); // switch symbol on

                    pStat->SetPaneSymbol(ID_STATUSPANE_NOTE); // switch symbol on     //pja 6/14/00
                    pStat->SetPaneText(ID_STATUSPANE_NOTE, szText);

                    // prepare pitch pane
                    // DDO - 08/15/00 swprintf_s(szText, _countof(szText),"%7.0fHz (%3s)", SemitoneToFrequency(fSemitone), Semitone2Name(bySemitone, sNoteName));

                    // only show tenths below 173 Hz CLW 12/9/00
                    double fFrequency = SemitoneToFrequency(fSemitone);
                    short nPrecision = short(fFrequency < 173.0 ? 1 : 0);
                    fFrequency += 0.5 * pow(10.0, -nPrecision);
                    fData          = (double)nData / 10.0;
                    swprintf_s(szText, _countof(szText),_T("     %.*f Hz (%3.1f%%)"), nPrecision, fFrequency, fData);

                    // write to pitch pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_PITCH); // switch symbol on
                    pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);

                    // prepare scale pane - display relative zero definition
                    // only show tenths below 173 Hz
                    fFrequency = SemitoneToFrequency(fSemitoneOffset);
                    nPrecision = short(fFrequency < 173.0 ? 1 : 0);
                    fFrequency += 0.5 * pow(10.0, -nPrecision);
                    swprintf_s(szText, _countof(szText),_T(" %3.2f st (%.*f Hz)"), fSemitoneOffset, nPrecision, fFrequency, fData);

                    // write to scale pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_SCALE, FALSE); // switch symbol off
                    pStat->SetPaneText(ID_STATUSPANE_SCALE, szText);

                    // prepare rel pitch pane - display pitch relative to zero definition
                    // only show tenths below 173 Hz
                    fFrequency = SemitoneToFrequency(fSemitone) - SemitoneToFrequency(fSemitoneOffset);
                    nPrecision = short(fFrequency < 173.0 ? 1 : 0);
                    fFrequency += 0.5 * pow(10.0, -nPrecision);
                    swprintf_s(szText, _countof(szText),_T(" %3.2f st (%.*f Hz)"), fSemitone - fSemitoneOffset, nPrecision, fFrequency, fData);

                    // write to scale pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_REL_PITCH); // switch symbol on
                    pStat->SetPaneText(ID_STATUSPANE_REL_PITCH, szText);
                } else {
                    bRes = FALSE;
                }
                break;
            default:
                break;
            }
            return;
        }
        // this graph and the parent view have focus
        if ((!bForceUpdate) && (dwStartCursor == m_dwLastStartCursor) && (dwStopCursor == m_dwLastStopCursor)) {
            return;    // no update
        }

        // save the updated cursor positions
        m_dwLastStartCursor = dwStartCursor;
        m_dwLastStopCursor = dwStopCursor;
        double fDataSec;
        int nMinutes = 0;
        CProcessLoudness * pLoudness = NULL;
        CProcessPitch * pPitch = NULL;
        CProcessSmoothedPitch * pSmPitch = NULL;
        CProcessCustomPitch * pChPitch = NULL;
        CProcessGrappl * pGrappl = NULL;
        CProcessMelogram  * pMelogram = NULL;
        CProcessDurations * pDuration = NULL;
        CProcessZCross * pZCross = NULL;
        BOOL bShowPosition = FALSE;            // default for position indicators
        BOOL bShowDuration = FALSE;            // default for length indicator
        double fUncertainty = 0;               // Measurement uncertainty
        int nFormants = 2;

        swprintf_s(szText, _countof(szText),_T("         "));          // default indicator

        switch (m_nPlotID) {
        case IDD_SPECTROGRAM:
        case IDD_SNAPSHOT: {
            CProcessFormantTracker * pSpectroFormants = pDoc->GetFormantTracker();
            // prepare spectrograms formant data, if ready
            CProcessSpectrogram * pSpectrogram = pDoc->GetSpectrogram(m_nPlotID==IDD_SPECTROGRAM); // get pointer to spectrogram object
            const CSpectroParm * pSpectroParm = &pSpectrogram->GetSpectroParm();
            if ((pSpectroParm->bShowFormants) && (pSpectroFormants->IsDataReady())) {
                double fSizeFactor = (double)pDoc->GetDataSize() / (double)(pSpectroFormants->GetDataSize() - 1);
                dwDataPos = (DWORD)((DWORD)(dwStartCursor / fSizeFactor * 2 / sizeof(SFormantFreq))) * sizeof(SFormantFreq) / 2;
                SFormantFreq * pFormFreqCurr = (SFormantFreq *)pSpectroFormants->GetProcessedData(dwDataPos, sizeof(SFormantFreq));

                for (int n = 1; n < 5; n++) {
                    if (pFormFreqCurr->F[n] > 0) {
                        if (nPitchMode == HERTZ) {
                            swprintf_s(szText,_countof(szText), _T("F%d: %.1f Hz"), n, pFormFreqCurr->F[n]);
                        } else {
                            swprintf_s(szText,_countof(szText), _T("F%d: %.1f st"), GetSemitone(pFormFreqCurr->F[n]));
                        }
                    } else {
                        swprintf_s(szText,_countof(szText), _T("F%d:"), n);
                    }
                    // write to position pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_POSITION + n - 1, FALSE); // switch symbol off
                    pStat->SetPaneText(ID_STATUSPANE_POSITION + n - 1, szText);
                }
            } else {
                // show frequency and power at mouse pointer
                const BOOL bPreEmphasis = TRUE;
                SSigParms Signal;
                const BOOL bBlockBegin = TRUE;
                Signal.SmpRate = pDoc->GetSamplesPerSec();
                Signal.Length = CDspWin::CalcLength(pSpectroParm->Bandwidth(), Signal.SmpRate, ResearchSettings.m_cWindow.m_nType);
                DWORD dwHalfFrameSize = (Signal.Length/2) * nSmpSize;
                CPoint MousePosn = m_pPlot->GetMousePointerPosition();
                DWORD dwWaveOffset = m_pPlot->CalcWaveOffsetAtPixel(MousePosn);
                if (MousePosn.x != UNDEFINED_OFFSET &&
                        (!m_bAreaGraph || (m_bAreaGraph && pSpectrogram->IsDataReady())) &&
                        ((dwWaveOffset >= dwHalfFrameSize)*nSmpSize && (dwWaveOffset < pDoc->GetDataSize() - dwHalfFrameSize))) {
                    CRect rPlotWnd;
                    m_pPlot->GetClientRect(rPlotWnd);
                    float fFreq = (float)(rPlotWnd.bottom - MousePosn.y) * (float)pSpectroParm->nFrequency / (float)rPlotWnd.bottom;
                    float fPowerInDb;

                    DWORD dwFrameStart = dwWaveOffset - dwHalfFrameSize;
                    Signal.Start = pDoc->GetWaveData(dwFrameStart, bBlockBegin);
                    Signal.SmpDataFmt = (nSmpSize == 1) ? (int8)PCM_UBYTE: (int8)PCM_2SSHORT;
                    CSpectrogram::CalcPower(&fPowerInDb, fFreq, pSpectroParm->nResolution, (CWindowSettings::Type)ResearchSettings.m_cWindow.m_nType, Signal, bPreEmphasis);

                    swprintf_s(szText,_countof(szText), _T("%7.1f Hz"), fFreq);
                    pStat->SetPaneSymbol(ID_STATUSPANE_3, FALSE);
                    pStat->SetPaneText(ID_STATUSPANE_3, szText);
                    swprintf_s(szText,_countof(szText), _T("%6.1f dB"), fPowerInDb);
                    pStat->SetPaneSymbol(ID_STATUSPANE_4, FALSE);
                    pStat->SetPaneText(ID_STATUSPANE_4, szText);
                } else {
                    //blank out
                    swprintf_s(szText, _countof(szText),_T("         "));
                    pStat->SetPaneSymbol(ID_STATUSPANE_3, FALSE);
                    pStat->SetPaneText(ID_STATUSPANE_3, szText);
                    pStat->SetPaneSymbol(ID_STATUSPANE_4, FALSE);
                    pStat->SetPaneText(ID_STATUSPANE_4, szText);
                }
                bShowPosition = bShowDuration = TRUE;
            }
        }
        break;
        case IDD_3D:
            nFormants++;
        case IDD_F1F2:
        case IDD_F2F1:
        case IDD_F2F1F1: {
            CProcessFormantTracker * pSpectroFormants = pDoc->GetFormantTracker();
            if (pSpectroFormants->IsDataReady()) {
                double fSizeFactor = (double)pDoc->GetDataSize() / (double)(pSpectroFormants->GetDataSize() - 1);
                dwDataPos = (DWORD)((DWORD)(dwStartCursor / fSizeFactor * 2 / sizeof(SFormantFreq))) * sizeof(SFormantFreq) / 2;
                SFormantFreq * pFormFreqCurr = (SFormantFreq *)pSpectroFormants->GetProcessedData(dwDataPos, sizeof(SFormantFreq));

                for (int n = 1; n < 5; n++) {
                    if (n > nFormants) {
                        swprintf_s(szText,_countof(szText), _T("         "));    // default indicator
                    } else if (pFormFreqCurr->F[n] > 0) {
                        if (nPitchMode == HERTZ) {
                            swprintf_s(szText,_countof(szText), _T("F%d: %.1f Hz"), n, pFormFreqCurr->F[n]);
                        } else {
                            swprintf_s(szText,_countof(szText), _T("F%d: %.1f st"), GetSemitone(pFormFreqCurr->F[n]));
                        }
                    } else {
                        swprintf_s(szText, _countof(szText),_T("F%d:"), n);
                    }
                    // write to position pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_POSITION + n - 1, FALSE); // switch symbol off
                    pStat->SetPaneText(ID_STATUSPANE_POSITION + n - 1, szText);
                }
            }
            break;
        }
        case IDD_RAWDATA:
        case IDD_GLOTWAVE: // BELOW MAY NOT BE RIGHT FOR GLOTWAVE
            bShowPosition = bShowDuration = TRUE;

            bRes = TRUE;
            nData = pDoc->GetWaveData(dwStartCursor, &bRes);
            fData = 100.0 * (double)nData / pow(2.0, 8.0 * (double)nSmpSize - 1);
            if (bRes) { // no error reading data?
                // display percentage and raw value CLW 1/5/01
                if (fData >= 99.95) {
                    swprintf_s(szText,_countof(szText), _T("L:  %3.0f%%  (%d)"), 100.0, nData);
                } else if (fData <= -99.95) {
                    swprintf_s(szText,_countof(szText), _T("L:  %3.0f%%  (%d)"), -100.0, nData);
                } else {
                    swprintf_s(szText,_countof(szText), _T("L:  %3.1f%%  (%d)"), fData, nData);
                }
            } else {
                swprintf_s(szText,_countof(szText), _T("L:"));
            }
            // write to amplitude pane
            pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE, FALSE); // switch symbol off
            pStat->SetPaneText(ID_STATUSPANE_AMPLITUDE, szText);
            // get the stop cursor value
            bRes = TRUE;
            nData = pDoc->GetWaveData(dwStopCursor, &bRes);
            fData = 100.0 * (double)nData / pow(2.0, 8.0 * (double)nSmpSize - 1);
            if (bRes) { // no error reading data?
                // display percentage and raw value CLW 1/5/01
                if (fData >= 99.95) {
                    swprintf_s(szText,_countof(szText), _T("R:  %3.0f%%  (%d)"), 100.0, nData);
                } else if (fData <= -99.95) {
                    swprintf_s(szText,_countof(szText), _T("R:  %3.0f%%  (%d)"), -100.0, nData);
                } else {
                    swprintf_s(szText,_countof(szText), _T("R:  %3.1f%%  (%d)"), fData, nData);
                }
            } else {
                swprintf_s(szText, _countof(szText),_T("R:"));
            }
            // write to pitch pane
            pStat->SetPaneSymbol(ID_STATUSPANE_PITCH, FALSE); // switch symbol off
            pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);
            break;

        case IDD_ZCROSS: // RLJ 09/20/2000.

            bShowPosition = bShowDuration = TRUE;
            pZCross = (CProcessZCross *)pDoc->GetZCross(); // get pointer to zero crossings object
            if (pZCross->IsDataReady()) {
                //!! Likely, some of the (float) casts for the other cases should be changed to (double) as well.
                //             fSizeFactor = (float)pDoc->GetDataSize() / (float)pZCross->GetDataSize(m_pPlot);
                //             dwDataPos = (DWORD)((float)dwStartCursor / fSizeFactor);
                // calculate size factor between waveform data and ZCross
                //fSizeFactor = (double)pDoc->GetDataSize() / (double)pZCross->GetDataSize(m_pPlot); // Minimize round-off error
                fSizeFactor = (double)nSmpSize * ceil((double)(pDoc->GetDataSize()/nSmpSize)/ (double)(pZCross->GetDataSize()));

                // calculate ZCross data position corresponding to Begin cursor position
                dwDataPos = (DWORD)((double)dwStartCursor / fSizeFactor); // Minimize round-off error
                // read the data
                nData = pZCross->GetProcessedData(dwDataPos, &bRes);
                if (bRes) { // no error reading data?
                    swprintf_s(szText,_countof(szText), _T("L:  %6d"), nData);
                } else {
                    swprintf_s(szText,_countof(szText), _T("L:"));
                }
                // write to amplitude pane
                pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE, FALSE); // switch symbol off
                pStat->SetPaneText(ID_STATUSPANE_AMPLITUDE, szText);

                // calculate ZCross data position corresponding to End cursor position
                //             dwDataPos = (DWORD)((float)dwStopCursor / fSizeFactor);
                dwDataPos = (DWORD)((double)dwStopCursor / fSizeFactor);  // Minimize round-off error
                // read the data
                nData = pZCross->GetProcessedData(dwDataPos, &bRes);
                if (bRes) { // no error reading data?
                    swprintf_s(szText,_countof(szText), _T("R:  %6d"), nData);
                } else {
                    swprintf_s(szText,_countof(szText), _T("R:"));
                }
                // write to pitch pane
                pStat->SetPaneSymbol(ID_STATUSPANE_PITCH, FALSE); // switch symbol off
                pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);
            }

            break;

        case IDD_PITCH:
            bShowPosition = bShowDuration = TRUE;
            fData = -1.;
            // prepare pitch pane data, display pitch if ready
            pPitch = (CProcessPitch *)pDoc->GetPitch(); // get pointer to smoothed pitch object
            if (pPitch->IsDataReady() && pPitch->GetMaxValue() > 0) {
                // calculate pitch data position corresponding to start cursor position
                dwDataPos = (dwStartCursor / nSmpSize) / Grappl_calc_intvl;
                // read the data
                fData = (float)pPitch->GetProcessedData(dwDataPos, &bRes) / (float)PRECISION_MULTIPLIER;
                if (!bRes) {
                    fData = -1.;
                }
            }
            fUncertainty = pPitch->GetUncertainty(fData);
            UpdatePitchStatusBar(fData, fUncertainty);
            break;

        case IDD_SMPITCH:
            bShowPosition = bShowDuration = TRUE;
            fData = -1.;
            // prepare pitch pane data, display pitch if ready
            pSmPitch = (CProcessSmoothedPitch *)pDoc->GetSmoothedPitch(); // get pointer to smoothed pitch object
            if (pSmPitch->IsDataReady() && pSmPitch->GetMaxValue() > 0) {
                // calculate pitch data position corresponding to start cursor position
                dwDataPos = (dwStartCursor / nSmpSize) / Grappl_calc_intvl;
                // read the data
                fData = (float)pSmPitch->GetProcessedData(dwDataPos, &bRes) / (float)PRECISION_MULTIPLIER;
                if (!bRes) {
                    fData = -1.;
                }
            }
            fUncertainty = pSmPitch->GetUncertainty(fData);
            UpdatePitchStatusBar(fData, fUncertainty);
            break;
        case IDD_CHPITCH:
            bShowPosition = bShowDuration = TRUE;
            fData = -1.;
            // prepare pitch pane data, display pitch if ready
            pChPitch = (CProcessCustomPitch *)pDoc->GetCustomPitch(); // get pointer to custom pitch object
            if (pChPitch->IsDataReady() && pChPitch->GetMaxValue() > 0) {
                // calculate pitch data position corresponding to start cursor position
                dwDataPos = (dwStartCursor / nSmpSize) / Grappl_calc_intvl;
                // read the data
                fData = (float)pChPitch->GetProcessedData(dwDataPos, &bRes) / (float)PRECISION_MULTIPLIER;
                if (!bRes) {
                    fData = -1.;
                }
            }
            fUncertainty = pChPitch->GetUncertainty(fData);
            UpdatePitchStatusBar(fData, fUncertainty);
            break;

        case IDD_GRAPITCH:
            pGrappl = (CProcessGrappl *)pDoc->GetGrappl();      // get pointer to auto pitch object

            if (pGrappl->IsDataReady() && pGrappl->GetMaxValue() > 0) {
                bShowPosition = bShowDuration = TRUE;
                // calculate pitch data position corresponding to start cursor position
                dwDataPos = (dwStartCursor / nSmpSize) / Grappl_calc_intvl;
                // read the data
                fData = (float)pGrappl->GetProcessedData(dwDataPos, &bRes) / (float)PRECISION_MULTIPLIER;
                if (!bRes) {
                    fData = -1.;
                }
                fUncertainty = pGrappl->GetUncertainty(fData);
                UpdatePitchStatusBar(fData, fUncertainty);
            } else {
                UpdatePitchStatusBar(-1., 0.);
            }

            break;

        case IDD_MELOGRAM:    //pja 5/7/00
            // moved to separate section CLW 8/24/00
            pMelogram = (CProcessMelogram *)pDoc->GetMelogram();  // get pointer to melogram pitch object

            if (pMelogram->IsDataReady() && pMelogram->GetMaxValue() > 0) {
                bShowPosition = bShowDuration = TRUE;
                dwDataPos = (dwStartCursor / nSmpSize) / Grappl_calc_intvl;
                fData = (float)pMelogram->GetProcessedData(dwDataPos, &bRes) / 100.0;
                if (bRes && fData > 0.F) {
                    // read the data
                    double fPitchInHz = 220. * pow(2., ((double)fData - 57.) / 12.); // convert from semitones to Hz
                    fUncertainty = pMelogram->GetUncertainty(fPitchInHz);
                    UpdatePitchStatusBarNote(fPitchInHz, fUncertainty, TRUE);
                } else {
                    UpdatePitchStatusBar(-1., 0.);
                }
            }

            break;

        case IDD_LOUDNESS:
        case IDD_MAGNITUDE:        //pja 5/7/00 - added the IDD_MAGNITUDE to display db in 3rd pane for Melogram Magnitude
            // prepare amplitude data, if ready
            pLoudness = (CProcessLoudness *)pDoc->GetLoudness(); // get pointer to loudness object
            if (pLoudness->IsDataReady() && (pLoudness->GetMaxValue() > 0)) {
                bShowPosition = bShowDuration = TRUE;
                // calculate size factor between raw data and loudness
                //fSizeFactor = (float)pDoc->GetDataSize() / (float)pLoudness->GetDataSize(m_pPlot);
                fSizeFactor = (double)nSmpSize * ceil((double)(pDoc->GetDataSize()/nSmpSize)/ (double)(pLoudness->GetDataSize()));
                // calculate loudness data position corresponding to start cursor position
                dwDataPos = (DWORD)((float)dwStartCursor / fSizeFactor);
                // read the data
                nData = pLoudness->GetProcessedData(dwDataPos, &bRes);
                if (bRes && nData > 0) {
                    swprintf_s(szText,_countof(szText), _T("      %.1f dB"), 20. * log10(double(nData)/32767.) + 6.);
                }
                // write to amplitude pane
                pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE); // switch symbol on
                pStat->SetPaneText(ID_STATUSPANE_AMPLITUDE, szText);

                // put percentage magnitude in pitch pane
                if (bRes && nData > 0) {
                    swprintf_s(szText,_countof(szText), _T("      %2d%% fullscale"), (short)(double(nData) / (32767./(2)) * 100.0 + 0.5));
                }
                pStat->SetPaneSymbol(ID_STATUSPANE_PITCH, FALSE); // pitch symbol
                pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);  // pitch indicator
            }

            break;

        case IDD_DURATION:
            pDuration = (CProcessDurations *)pDoc->GetDurations(); // get pointer to loudness object
            if (pDuration->IsDataReady() && (pDuration->GetMaxDuration() > 0)) {
                bShowPosition = TRUE;
                CSegment * pSegment = pDoc->GetSegment(PHONETIC);
                int nSegmentIndex = pSegment->FindFromPosition(dwStartCursor, TRUE);
                if (nSegmentIndex > 0) {
                    fData = (float)pDuration->GetProcessedData(nSegmentIndex);
                    swprintf_s(szText,_countof(szText), _T("     %.2f ms"), fData * 1000.F / (float)(pDoc->GetBlockAlign() * pDoc->GetSamplesPerSec()));
                }
            }

            // write to duration pane
            pStat->SetPaneSymbol(ID_STATUSPANE_LENGTH); // switch symbol on
            pStat->SetPaneText(ID_STATUSPANE_LENGTH, szText);

            // clear out other panes
            pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE, FALSE); // amplitude symbol
            pStat->SetPaneText(ID_STATUSPANE_AMPLITUDE, _T("        "));  // amplitude indicator
            pStat->SetPaneSymbol(ID_STATUSPANE_PITCH, FALSE); // pitch symbol
            pStat->SetPaneText(ID_STATUSPANE_PITCH, _T("        "));  // pitch indicator
            break;

        case IDD_CHANGE: {
            bShowPosition = bShowDuration = TRUE;
            CProcessChange * pChange = (CProcessChange *)pDoc->GetChange(); // get pointer to change object
            if (pChange->IsDataReady()) {
                // calculate size factor between waveform data
                //fSizeFactor = (double)pDoc->GetDataSize() / (double)pChange->GetDataSize(); // Minimize round-off error
                fSizeFactor = (double)nSmpSize * ceil((double)(pDoc->GetDataSize()/nSmpSize)/ (double)(pChange->GetDataSize()));

                // calculate pChange data position corresponding to Begin cursor position
                dwDataPos = (DWORD)((double)dwStartCursor / fSizeFactor); // Minimize round-off error
                // read the data
                int nData = pChange->GetProcessedData(dwDataPos, &bRes) / PRECISION_MULTIPLIER;
                if (bRes) { // no error reading data?
                    swprintf_s(szText,_countof(szText), _T("L:  %6d"), nData);
                } else {
                    swprintf_s(szText,_countof(szText), _T("L:"));
                }
                // write to amplitude pane
                pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE, FALSE); // switch symbol off
                pStat->SetPaneText(ID_STATUSPANE_AMPLITUDE, szText);

                // calculate pChange data position corresponding to End cursor position
                dwDataPos = (DWORD)((double)dwStopCursor / fSizeFactor);  // Minimize round-off error
                // read the data
                nData = pChange->GetProcessedData(dwDataPos, &bRes) / PRECISION_MULTIPLIER;
                if (bRes) { // no error reading data?
                    swprintf_s(szText,_countof(szText), _T("R:  %6d"), nData);
                } else {
                    swprintf_s(szText,_countof(szText), _T("R:"));
                }
                // write to pitch pane
                pStat->SetPaneSymbol(ID_STATUSPANE_PITCH, FALSE); // switch symbol off
                pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);
            }

            // write to duration pane
            pStat->SetPaneSymbol(ID_STATUSPANE_LENGTH); // switch symbol on
            pStat->SetPaneText(ID_STATUSPANE_LENGTH, szText);
            break;
        }

        default:
            bShowPosition = bShowDuration = TRUE;
            pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE, FALSE);           // amplitude symbol
            pStat->SetPaneText(ID_STATUSPANE_AMPLITUDE, _T("        "));    // amplitude indicator
            pStat->SetPaneSymbol(ID_STATUSPANE_PITCH, FALSE);               // pitch symbol
            pStat->SetPaneText(ID_STATUSPANE_PITCH, _T("        "));        // pitch indicator
            break;
        }

        if (bShowPosition) {
            if (nPositionMode == TIME) {
                // calculate position time
                fDataSec = pDoc->GetTimeFromBytes(dwStartCursor);

                nMinutes = (int)fDataSec / 60;
                fDataSec = fDataSec - (nMinutes * 60.);
                if (nMinutes > 9) {
                    swprintf_s(szText,_countof(szText), _T("       %02i:%5.3f"), nMinutes, fDataSec);
                } else {
                    swprintf_s(szText,_countof(szText), _T("       %01i:%6.4f"), nMinutes, fDataSec);
                }
            } else {
                if (nPositionMode == SAMPLES) {
                    swprintf_s(szText, _countof(szText),_T("       %.0f"), (double)(dwStartCursor / nSmpSize));
                } else {
                    swprintf_s(szText, _countof(szText),_T("       %.0f"), (double)dwStartCursor);
                }
            }
            // write to position pane
            pStat->SetPaneSymbol(ID_STATUSPANE_POSITION); // switch symbol on
            pStat->SetPaneText(ID_STATUSPANE_POSITION, szText);
        }

        if (bShowDuration) {
            if (nPositionMode == TIME) {
                // calculate length time
                fDataSec = pDoc->GetTimeFromBytes(dwStopCursor - dwStartCursor);
                nMinutes = (int)fDataSec / 60;
                fDataSec = fDataSec - (nMinutes * 60.);
                if (nMinutes > 9) {
                    swprintf_s(szText,_countof(szText), _T("       %02i:%5.3f"), nMinutes, fDataSec);
                } else if (nMinutes || fDataSec >= .01) {
                    swprintf_s(szText,_countof(szText), _T("       %01i:%6.4f"), nMinutes, fDataSec);
                } else {
                    swprintf_s(szText,_countof(szText), _T("       %01i:%7.5f"), nMinutes, fDataSec);
                }
            } else {
                if (nPositionMode == SAMPLES) {
                    swprintf_s(szText,_countof(szText), _T("       %.0f"), (double)((dwStopCursor - dwStartCursor) / nSmpSize));
                } else {
                    swprintf_s(szText,_countof(szText), _T("       %.0f"), (double)(dwStopCursor - dwStartCursor));
                }
            }
            // write to length pane
            pStat->SetPaneSymbol(ID_STATUSPANE_LENGTH); // switch symbol on
            pStat->SetPaneText(ID_STATUSPANE_LENGTH, szText);
        }

        // show the frequency
        {
            DWORD samplesPerSecond = pDoc->GetSamplesPerSec();
            swprintf_s(szText, _countof(szText), _T("       %d Hz"), samplesPerSecond);
            // write to frequency pane
            pStat->SetPaneSymbol(ID_STATUSPANE_SAMPLES);
            pStat->SetPaneText(ID_STATUSPANE_SAMPLES, szText);
        }

        // show the storage format
        {
            WORD bitsPerSample = pDoc->GetBitsPerSample();
            swprintf_s(szText, _countof(szText), _T("       %d Bits"), bitsPerSample);
            // write to frequency pane
            pStat->SetPaneSymbol(ID_STATUSPANE_FORMAT);
            pStat->SetPaneText(ID_STATUSPANE_FORMAT, szText);
        }

        // show the number of channels
        {
            if (pDoc->GetNumChannels()==1) {
                swprintf_s(szText, _countof(szText), _T("       Mono"));
            } else if (pDoc->GetNumChannels()==2) {
                swprintf_s(szText, _countof(szText), _T("       Stereo"));
            } else {
                swprintf_s(szText, _countof(szText), _T("       %d Channels"),pDoc->GetNumChannels());
            }

            pStat->SetPaneSymbol(ID_STATUSPANE_CHANNELS);
            pStat->SetPaneText(ID_STATUSPANE_CHANNELS, szText);
        }

#define KB 1024.0
#define MB (1024.0*1024.0)
#define GB (1024.0*1024.0*1024.0)

        // show the number of size in KB/MB/GB
        {
			double size = (double)pDoc->GetDataSize()*pDoc->GetNumChannels();
            if (size < MB) {
                size /= KB;
                swprintf_s(szText,_countof(szText), _T("       %.2f KB"), size);
            } else if (size < GB) {
                size /= MB;
                swprintf_s(szText,_countof(szText), _T("       %.2f MB"), size);
            } else {
                size /= GB;
                swprintf_s(szText,_countof(szText), _T("       %.2f GB"), size);
            }
            pStat->SetPaneSymbol(ID_STATUSPANE_SIZE);
            pStat->SetPaneText(ID_STATUSPANE_SIZE, szText);
        }

        // show the number of length
        {
            // create and write length text
            double fDataSec = pDoc->GetTimeFromBytes(pDoc->GetDataSize());  // get sampled data size in seconds
            int nHours = (int)fDataSec / 3600;
            if (nHours > 0) {
                // calculate remainder
                fDataSec -= (double)(nHours*3600);
                int nMinutes = (int)fDataSec / 60;
                swprintf_s(szText, _countof(szText), _T("       %0d:%02d H:M"), nHours, nMinutes);
            } else {
                // show minutes and seconds
                int nMinutes = (int)fDataSec / 60;
                fDataSec -= (double)(nMinutes*60);
                int nSeconds = (fDataSec>=1.0)?(int)fDataSec:1;
                swprintf_s(szText, _countof(szText), _T("       %0d:%02d M:S"), nMinutes, nSeconds);
            }
            pStat->SetPaneSymbol(ID_STATUSPANE_TLENGTH);
            pStat->SetPaneText(ID_STATUSPANE_TLENGTH, szText);
        }

        // show the number of type
        {
            wstring name = pDoc->GetFilenameFromTitle();
            size_t idx = name.find_last_of('.');
            if (idx!=wstring::npos) {
                wstring extension = name.substr(idx+1);
                CString ext = extension.c_str();
                ext = ext.MakeUpper();
                swprintf_s(szText, _countof(szText), _T("       %s"), (LPCTSTR)ext);
                pStat->SetPaneSymbol(ID_STATUSPANE_TYPE);
                pStat->SetPaneText(ID_STATUSPANE_TYPE, szText);
            } else {
                pStat->SetPaneSymbol(ID_STATUSPANE_TYPE);
                pStat->SetPaneText(ID_STATUSPANE_TYPE, _T(""));
            }
        }

        // show the number of bitrate
        {
            double temp = pDoc->GetAvgBytesPerSec()*8;
            if (temp>KB) {
                swprintf_s(szText, _countof(szText), _T("       %dk bps"), (int)(temp/KB));
            } else {
                swprintf_s(szText, _countof(szText), _T("       %d bps"), (int)temp);
            }
            pStat->SetPaneSymbol(ID_STATUSPANE_BITRATE);
            pStat->SetPaneText(ID_STATUSPANE_BITRATE, szText);
        }

        pStat->UpdateWindow();
    } else {
        return;    // this graph has not focus
    }
}

/***************************************************************************/
// CGraphWnd::OnGenderInfoChanged  Gender info has been updated
/***************************************************************************/
LRESULT CGraphWnd::OnGenderInfoChanged(WPARAM nGender, LPARAM) {
    m_pPlot->GenderInfoChanged(nGender);  // notify plot
    return 0L;
}

void CGraphWnd::OnDestroy() {
    CMiniCaptionWnd::OnDestroy();

    // TODO: Add your message handler code here
    // the plot window is a child window and will be automatically destroyed
    // when this window is destroyed, also, the plot wnd handles it's own
    // destruction via PostNcDestroy, therefore, we only need to release
    // our reference to it.
    m_pPlot = NULL;
}

void CGraphWnd::RemoveRtPlots() {
    if (m_pPlot!=NULL) {
        m_pPlot->RemoveRtPlots();
    }
}

void CGraphWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
    GetParent()->SendMessage(WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags));
}

void CGraphWnd::OnNcMouseMove(UINT nHitTest, CPoint point) {
    m_pPlot->SetMousePointerPosition(CPoint(UNDEFINED_OFFSET, UNDEFINED_OFFSET)); // clear mouse position
    m_pPlot->SetMouseButtonState(0);  // reset state
    const BOOL bForceUpdate = TRUE;
    CSaView * pView = (CSaView *)GetParent();
    UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), bForceUpdate);
    CWnd::OnNcMouseMove(nHitTest, point);
}

CGraphWnd::CGraphWnd() {
}

// return plot ID
UINT CGraphWnd::GetPlotID() const {
    return m_nPlotID;
}

// return plot ID
UINT CGraphWnd::IsPlotID(UINT test) const {
    return test == m_nPlotID;
}

void CGraphWnd::SetBoundaries( bool show) {
	m_bBoundaries = show;
}

void CGraphWnd::ShowTranscriptionBoundaries(BOOL bShow) {
    for (int nWnd = 0; nWnd < ANNOT_WND_NUMBER; nWnd++) {
        GetAnnotationWnd(nWnd)->ShowTranscriptionBoundaries(bShow);
        GetAnnotationWnd(nWnd)->Invalidate(TRUE);
    }
}

// return magnify factor
double CGraphWnd::GetMagnify() {
    return m_pPlot->GetMagnify();   
}

// boundaries visible?
bool CGraphWnd::HasBoundaries() {
    return m_bBoundaries;
}

// return drawing style
BOOL CGraphWnd::HaveDrawingStyleLine() {
    return m_pPlot->HaveDrawingStyleLine();   
}

// cursors visible?
bool CGraphWnd::HasCursors() {
    return m_pPlot->HasCursors();   
}

// private cursor visible?
bool CGraphWnd::HasPrivateCursor() {
    return m_pPlot->HasPrivateCursor();   
}

// gridlines visible?
BOOL CGraphWnd::HaveGrid() {
    return m_pPlot->HaveGrid();   
}

void CGraphWnd::SetLineDraw(BOOL bLine) {
    if (m_pPlot) {
        m_pPlot->SetLineDraw(bLine);
    }
}

void CGraphWnd::ShowGrid(BOOL bShow, BOOL bRedraw) {
    m_pPlot->ShowGrid(bShow, bRedraw);
}

void CGraphWnd::MoveStartCursor(CSaView * pView, DWORD dwNewPositon) {
    m_pPlot->MoveStartCursor(pView, dwNewPositon);
}

void CGraphWnd::MoveStopCursor(CSaView * pView, DWORD dwNewPositon) {
    m_pPlot->MoveStopCursor(pView, dwNewPositon);
}

void CGraphWnd::RestartProcess() {
    m_pPlot->RestartProcess();
}

CAnnotationWnd * CGraphWnd::GetAnnotationWnd(int nIndex) {
    // return pointer to indexed annotation window
    return m_apAnnWnd[nIndex];
}

CLegendWnd * CGraphWnd::GetLegendWnd() {
    // return pointer to legend window
    return m_pLegend;
}

CXScaleWnd * CGraphWnd::GetXScaleWnd() {
    // return pointer to x-scale window
    return m_pXScale;
}

void CGraphWnd::ShowCursors(bool bPrivate, bool bShow) {
    // set cursors visible/hidden
    m_pPlot->ShowCursors(bPrivate, bShow);
}

BOOL CGraphWnd::HasLegend() {
    // legend window visible?
    return m_bLegend;
}

BOOL CGraphWnd::HaveXScale() {
    // x-scale window visible?
    return m_bXScale;
}

BOOL CGraphWnd::HaveAnnotation(int nIndex) {
    // indexed annotation window visible?
    return m_abAnnWnd[nIndex];
}

void CGraphWnd::SetAreaGraph(BOOL bArea) {
    // set graph to area processed graph type
    m_bAreaGraph = bArea;
}

CPlotWnd * CGraphWnd::GetPlot() const {
    return m_pPlot;
}

BOOL CGraphWnd::IsAnnotationVisible(int nIndex) {
    return m_abAnnWnd[nIndex];
}

// is this an area processed graph?
BOOL CGraphWnd::IsAreaGraph() {
    return m_bAreaGraph;   
}

BOOL CGraphWnd::IsAnimationGraph() {
    return (m_pPlot->IsAnimationPlot());
}

BOOL CGraphWnd::IsCanceled() {
    return  m_pPlot->IsCanceled();
}

afx_msg void CGraphWnd::OnSetFocus(CWnd * /*cwp*/) {
    if (IsPlotID(IDD_STAFF)) {
        ::SetFocus(m_pPlot->m_hWnd);
    }
}

/***************************************************************************/
// CGraphWnd::SetMagnify Set new magnify factor, redraw plot and legend
/***************************************************************************/
void CGraphWnd::SetMagnify(double bFactor, BOOL bRedraw) {
    // set magnify in plot
    m_pPlot->SetMagnify(bFactor, bRedraw);
    if (m_bLegend && bRedraw) {
		// redraw whole graph
        RedrawGraph(TRUE, TRUE);    
    }
}

/***************************************************************************/
// CGraphWnd::SetStartCursor Position the start cursor
/***************************************************************************/
void CGraphWnd::SetStartCursor(CSaView * pView) {
    m_pPlot->SetStartCursor(pView);
    if (!m_pPlot->HasCursors()) {
        m_pPlot->Invalidate();
    }
	// update the status bar
    UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition());  
}

/***************************************************************************/
// CGraphWnd::SetStopCursor Position the stop cursor
/***************************************************************************/
void CGraphWnd::SetStopCursor(CSaView * pView) {
    m_pPlot->SetStopCursor(pView);
    if (!m_pPlot->HasCursors()) {
        m_pPlot->Invalidate();
    }
    UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition()); // update the status bar
}

/***************************************************************************/
// CGraphWnd::SetPlaybackCursor Position the playback cursor
/***************************************************************************/
void CGraphWnd::SetPlaybackCursor(CSaView * pView) {
    m_pPlot->SetPlaybackCursor(pView,false);
    if (!m_pPlot->HasCursors()) {
        m_pPlot->Invalidate();
    }
}

// SDM 1.06.6U6
/***************************************************************************/
// CGraphWnd::SetPlaybackPosition
/***************************************************************************/
void CGraphWnd::SetPlaybackPosition( CSaView * pView, bool scroll) {
    if (m_pPlot->HasCursors()) {
        m_pPlot->SetPlaybackCursor(pView, scroll);
    }
}

void CGraphWnd::SetPlaybackFlash(bool on) {
    if (m_pPlot->HasCursors()) {
        m_pPlot->SetPlaybackFlash(on);
    }
}

/***************************************************************************/
// CGraphWnd::CreateAnnotationWindows Creates the annotation windows
//**************************************************************************/
void CGraphWnd::CreateAnnotationWindows() {
    m_apAnnWnd[PHONETIC] = new CPhoneticWnd(PHONETIC);
    m_apAnnWnd[TONE] = new CToneWnd(TONE);
    m_apAnnWnd[PHONEMIC] = new CPhonemicWnd(PHONEMIC);
    m_apAnnWnd[ORTHO] = new COrthographicWnd(ORTHO);
    m_apAnnWnd[GLOSS] = new CGlossWnd(GLOSS);
    m_apAnnWnd[GLOSS_NAT] = new CGlossNatWnd(GLOSS_NAT);
    m_apAnnWnd[REFERENCE] = new CReferenceWnd(REFERENCE);
    m_apAnnWnd[MUSIC_PL1] = new CMusicPhraseWnd(MUSIC_PL1);
    m_apAnnWnd[MUSIC_PL2] = new CMusicPhraseWnd(MUSIC_PL2);
    m_apAnnWnd[MUSIC_PL3] = new CMusicPhraseWnd(MUSIC_PL3);
    m_apAnnWnd[MUSIC_PL4] = new CMusicPhraseWnd(MUSIC_PL4);
    CRect rWnd(0, 0, 0, 0);
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        if (!m_apAnnWnd[nLoop]->Create(NULL, _T("Transcription"), WS_CHILD | WS_VISIBLE, rWnd, this, 0)) {
            delete m_apAnnWnd[nLoop];
            m_apAnnWnd[nLoop] = NULL;
        }
    }
}

/***************************************************************************/
// CGraphWnd::SetLegendScale Set the scale of the legend window
//**************************************************************************/
BOOL CGraphWnd::SetLegendScale(int nMode, double dMinValue, double dMaxValue, TCHAR * pszDimension, int nDivisions, double d3dOffset) {
    // set legend scale
    BOOL bNewScale = m_pLegend->SetScale(nMode, dMinValue, dMaxValue, pszDimension, nDivisions, d3dOffset);
    if (bNewScale && m_nPlotID != ID_GRAPHS_OVERLAY) {
		// redraw whole graph
        RedrawGraph(TRUE, m_bLegend, FALSE);
    }

    return bNewScale && m_nPlotID != ID_GRAPHS_OVERLAY;
}

/***************************************************************************/
// CGraphWnd::SetXScale Set the scale of the x-scale window
//**************************************************************************/
void CGraphWnd::SetXScale(int nMode, int nMinValue, int nMaxValue, TCHAR * pszDimension, int nDivisions, double d3dOffset) {
    if (m_bXScale) {
        // set x-scale scale
        if (m_pXScale->SetScale(nMode, nMinValue, nMaxValue, pszDimension, nDivisions, d3dOffset) && m_nPlotID != ID_GRAPHS_OVERLAY) {
			// redraw whole graph
            RedrawGraph(TRUE, TRUE);    
        }
    }
}

/***************************************************************************/
// CGraphWnd::ChangeAnnotationSelection Change the annotation selection
//**************************************************************************/
void CGraphWnd::ChangeAnnotationSelection(int nIndex) {
    if (m_abAnnWnd[nIndex]) {
		// redraw
        m_apAnnWnd[nIndex]->Invalidate(TRUE);    
    }
}

/***************************************************************************/
// CGraphWnd::OnCreate Window creation
/***************************************************************************/
int CGraphWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {

    if (CMiniCaptionWnd::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    ModifyStyle(0, WS_CLIPCHILDREN);
    // create scale and annotation window objects
    m_pLegend = new CLegendWnd();
    m_pXScale = new CXScaleWnd();
    CRect rWnd(0, 0, 0, 0);
    // create the plot window
    if (!m_pPlot->Create(NULL, _T("Plot"), WS_CHILD | WS_VISIBLE, rWnd, this, 0)) {
        delete m_pPlot;
        m_pPlot = NULL;
    }
    m_pPlot->SetPlotName(lpCreateStruct->lpszName);
    // create the legend window
    if (!m_pLegend->Create(NULL, _T("Legend"), WS_CHILD | WS_VISIBLE, rWnd, this, 0)) {
        delete m_pLegend;
        m_pLegend = NULL;
    }
    // create the x-scale window
    if (!m_pXScale->Create(NULL, _T("XScale"), WS_CHILD | WS_VISIBLE, rWnd, this, 0)) {
        delete m_pXScale;
        m_pXScale = NULL;
    }
    // create the annotation windows
    CreateAnnotationWindows();
    return 0;
}

/***************************************************************************/
// CGraphWnd::OnSize Window sizing
/***************************************************************************/
void CGraphWnd::OnSize(UINT nType, int cx, int cy) {
    CMiniCaptionWnd::OnSize(nType, cx, cy);
    ResizeGraph(TRUE, TRUE); // repaint whole graph
}

/***************************************************************************/
// CGraphWnd::OnDraw - draw the graph window.  Currently
// only used for printing.
/***************************************************************************/
void CGraphWnd::OnDraw(CDC * pDC, const CRect * printRect,
                       int originX, int originY) {
    if (pDC->IsPrinting()) {
        if (m_pPlot != NULL) {
            PrintHiResGraph(pDC, printRect, originX, originY);
        }
    }
}

void ScaleRect(CRect & rct, double scaleX, double scaleY) {
    rct.left   = (int)(scaleX * rct.left);
    rct.right  = (int)(scaleX * rct.right);
    rct.top    = (int)(scaleY * rct.top);
    rct.bottom = (int)(scaleY * rct.bottom);
}

/***************************************************************************/
// CGraphWnd::PrintHiResGraph - print the hi res graph by calling OnDraw()
// for every window object contained within this window.
/***************************************************************************/
void CGraphWnd::PrintHiResGraph(CDC * pDC, const CRect * printRect,
                                int originX, int originY) {
    int nLoop = 0;
    CRect graphRect, plotRect;
    CRect xScaleRect(0,0,0,0);
    CRect scaledXscaleRect(0,0,0,0);
    CRect legendRect(0,0,0,0);
    CRect scaledLegendRect(0,0,0,0);
    CRect annotRect[ANNOT_WND_NUMBER];
    CRect scaledAnnotRect[ANNOT_WND_NUMBER];
    int height = m_iNCHeight * 2;

    for (; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        annotRect[nLoop] = CRect(0,0,0,0);
        scaledAnnotRect[nLoop] = CRect(0,0,0,0);
    }
    GetWindowRect(&graphRect);
    double scaleX = (double)((1.0 * printRect->Width())  / graphRect.Width());
    double scaleY = (double)((1.0 * printRect->Height()) / graphRect.Height());

    // draw the window border
    OnNcDraw(pDC, *printRect, TRUE);

    // phase 1 - get the CRect's
    m_pPlot->GetWindowRect(plotRect);
    CRect scaledPlotRect(0,0,(int)(scaleX * plotRect.Width()),
                         (int)(scaleY * plotRect.Height()));

    for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        if (m_abAnnWnd[nLoop]) {
            m_apAnnWnd[nLoop]->GetWindowRect(&(annotRect[nLoop]));
            scaledAnnotRect[nLoop].right  = (int)(scaleX * annotRect[nLoop].Width());
            scaledAnnotRect[nLoop].bottom = (int)(scaleY * annotRect[nLoop].Height());
        }
    }
    if (m_pXScale) {
        m_pXScale->GetWindowRect(&xScaleRect);
        scaledXscaleRect.right  = (int)(scaleX * xScaleRect.Width());
        scaledXscaleRect.bottom = (int)(scaleY * xScaleRect.Height());
    }
    if (m_pLegend) {
        m_pLegend->GetWindowRect(&legendRect);
        scaledLegendRect.right  = (int)(scaleX * legendRect.Width());
        scaledLegendRect.bottom = (int)(scaleY * legendRect.Height());
    }

    // phase 2 - adjust it
    CRect T(printRect);

    T.bottom = T.top + height;

    CRect L(scaledLegendRect);

    L.top  = T.bottom;
    L.left = printRect->left + Graph_Inset;
    L.right = LEGEND_PRINT_WIDTH;
    L.bottom = printRect->bottom - Graph_Inset;

    CRect B(scaledXscaleRect);

    B.left = L.right;
    B.right = printRect->right - Graph_Inset;
    B.bottom = printRect->bottom - Graph_Inset;
    B.top = B.bottom - XSCALE_PRINT_HEIGHT;

    CRect A[ANNOT_WND_NUMBER];

    for (nLoop=0; nLoop<ANNOT_WND_NUMBER; nLoop++) {
        if (m_abAnnWnd[nLoop]) {
            break;
        }
    }

    int Abottom=T.bottom;

    if (nLoop < ANNOT_WND_NUMBER) {
        A[nLoop].top = T.bottom + Graph_Inset;
        A[nLoop].left = L.right;
        A[nLoop].right = printRect->right - Graph_Inset;
        A[nLoop].bottom = A[nLoop].top +  ANNOT_PRINT_HEIGHT;
        Abottom =A[nLoop].bottom;

        int prev = nLoop;

        for (int nLoop2=nLoop+1; nLoop2<ANNOT_WND_NUMBER; nLoop2++) {
            if (m_abAnnWnd[nLoop2]) {
                A[nLoop2].top = A[prev].bottom;
                A[nLoop2].left = L.right;
                A[nLoop2].right = printRect->right - Graph_Inset;
                A[nLoop2].bottom = A[nLoop2].top + ANNOT_PRINT_HEIGHT;
                Abottom = A[nLoop2].bottom;
                prev = nLoop2;
            }
        }
    }

    CRect P(L.right,Abottom,printRect->right-Graph_Inset,B.top);

    scaledLegendRect.right = L.Width();
    scaledLegendRect.bottom = L.Height();
    scaledXscaleRect.right = B.Width();
    scaledXscaleRect.bottom = B.Height();

    for (nLoop=0; nLoop<ANNOT_WND_NUMBER; nLoop++) {
        if (m_abAnnWnd[nLoop]) {
            scaledAnnotRect[nLoop].right = A[nLoop].Width();
            scaledAnnotRect[nLoop].bottom = A[nLoop].Height();
        }
    }

    scaledPlotRect.right = P.Width();
    scaledPlotRect.bottom = P.Height();


    // phase 3 - draw it

    for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
        if (m_abAnnWnd[nLoop]) {
            pDC->SetWindowOrg(CPoint(originX - A[nLoop].left,originY - A[nLoop].top));
            m_apAnnWnd[nLoop]->OnDraw(pDC,scaledAnnotRect[nLoop]);
        }
    }
    if (m_pXScale) {
        pDC->SetWindowOrg(CPoint(originX - B.left,originY - B.top));
        m_pXScale->OnDraw(pDC,scaledXscaleRect,scaledPlotRect);
    }
    if (m_pLegend) {
        pDC->SetWindowOrg(CPoint(originX - L.left,originY - L.top));

        m_pLegend->OnDraw(pDC,scaledLegendRect,scaledPlotRect,scaledXscaleRect,
                          scaledAnnotRect);
    }

    CPoint newOrg(originX - P.left, originY - P.top);
    pDC->SetWindowOrg(newOrg);
    CSaView * pView = (CSaView *)GetParent();

    CRect clipRect(scaledPlotRect);
    pDC->LPtoDP(clipRect);
    //clipRect.OffsetRect(-newOrg.x, -newOrg.y);
    if (pView->PrintPreviewInProgress()) {
        clipRect.OffsetRect(2000, 198);
        // transform to preview DP
        CPreviewDC * pPrevDC = static_cast<CPreviewDC *>(pDC);
        pPrevDC->PrinterDPtoScreenDP(&clipRect.TopLeft());
        pPrevDC->PrinterDPtoScreenDP(&clipRect.BottomRight());
    }

    m_pPlot->OnDraw(pDC, scaledPlotRect, clipRect, pView);

    CRect rctCur;
    double scaleCurX = (double)((scaledPlotRect.Width() * 1.0) / plotRect.Width());
    double scaleCurY = (double)((scaledPlotRect.Height() * 1.0) / plotRect.Height());

    // if there are cursors to print, print cursors
    if (m_pPlot->HasCursors()) {
        // print the start cursor
        CStartCursorWnd * pStartCur = m_pPlot->GetStartCursorWnd();
        pStartCur->GetWindowRect(&rctCur);
        rctCur.left += CURSOR_WINDOW_HALFWIDTH;
        rctCur.OffsetRect(-plotRect.left,-plotRect.top);
        ScaleRect(rctCur, scaleCurX, scaleCurY);
        rctCur.right = rctCur.left + 1;
        pDC->SetWindowOrg(CPoint(originX - P.left - rctCur.left,
                                 originY - P.top  - rctCur.top));
        rctCur.OffsetRect(-rctCur.left,-rctCur.top);
        pStartCur->OnDraw(pDC, rctCur);

        // print the stop cursor
        CStopCursorWnd * pStopCur = m_pPlot->GetStopCursorWnd();
        pStopCur->GetWindowRect(&rctCur);
        rctCur.left += CURSOR_WINDOW_HALFWIDTH;
        rctCur.OffsetRect(-plotRect.left,-plotRect.top);
        ScaleRect(rctCur, scaleCurX, scaleCurY);
        rctCur.right = rctCur.left + 1;
        pDC->SetWindowOrg(CPoint(originX - P.left - rctCur.left,
                                 originY - P.top  - rctCur.top));
        rctCur.OffsetRect(-rctCur.left,-rctCur.top);
        pStopCur->OnDraw(pDC, rctCur);
    } // end of if there are cursors to print, print cursors


    // if there is a private cursor to print, print it.
    if (m_pPlot->HasPrivateCursor()) {
        // print the private cursor
        m_pPlot->GetPrivateCursorWindowRect(&rctCur);
        rctCur.left += CURSOR_WINDOW_HALFWIDTH;
        rctCur.OffsetRect(-plotRect.left, -plotRect.top);
        ScaleRect(rctCur, scaleCurX, scaleCurY);
        rctCur.right = rctCur.left + 1;
        pDC->SetWindowOrg(CPoint(originX - P.left - rctCur.left, originY - P.top  - rctCur.top));
        rctCur.OffsetRect(-rctCur.left, -rctCur.top);
        m_pPlot->OnPrivateCursorDraw(pDC, rctCur);
    } // end of if there is a private cursor to print, print it.
}

BOOL CGraphWnd::IsIDincluded(UINT id) {
    return (m_nPlotID == ID_GRAPHS_OVERLAY) ? ((CMultiPlotWnd *)m_pPlot)->IsIDincluded(id) : (m_nPlotID == id);
}

CPlotWnd  * CGraphWnd::NewPlotFromID(UINT plotID) {

	CPlotWnd * pPlot = NULL;
    // create plot window object depending on which type of graph it is
    switch (plotID) {
    case IDD_RAWDATA:
        pPlot = new CPlotRawData();
        break;
    case IDD_LOUDNESS:
        pPlot = new CPlotLoudness();
        break;
    case IDD_DURATION:
        pPlot = new CPlotDurations();
        break;
    case IDD_CHANGE:
        pPlot = new CPlotChange();
        break;
    case IDD_GLOTWAVE:
        pPlot = new CPlotGlottis();
        break;
    case IDD_3D_PITCH:
        pPlot = new CPlot3dPitch();
        break;
    case IDD_WAVELET:
        pPlot = new CPlotWavelet();
        break;      // ARH 8/2/01 Added for the wavelet graph

    case IDD_SPECTROGRAM:
        pPlot = new CPlotSpectrogram();
        ((CPlotSpectrogram *)pPlot)->SetSpectroAB('A'); // RLJ 1.5Test11.1A
        break; //  automatic graph
    case IDD_SNAPSHOT:
        pPlot = new CPlotSpectrogram();
        ((CPlotSpectrogram *)pPlot)->SetSpectroAB('B'); // RLJ 1.5Test11.1A
        SetAreaGraph(TRUE);
        break; //  is an area processed graph

    case IDD_SPECTRUM:
        pPlot = new CPlotSpectrum();
        break;
    case IDD_TWC:
        pPlot = new CPlotTonalWeightChart();
        break;
    case IDD_PITCH:
        pPlot = new CPlotPitch();
        break;
    case IDD_CHPITCH:
        pPlot = new CPlotCustomPitch();
        break;
    case IDD_SMPITCH:
        pPlot = new CPlotSmoothedPitch();
        break;
    case IDD_GRAPITCH:
        pPlot = new CPlotGrappl();
        break;
    case IDD_RECORDING:
        pPlot = new CPlotRecording();
        SetAreaGraph(TRUE);
        break;
    case IDD_MELOGRAM:
        pPlot = new CPlotMelogram();
        break;
    case IDD_STAFF:
        pPlot = new CPlotStaff();
        break;
    case IDD_MAGNITUDE:
        pPlot = new CPlotMagnitude();
        if (pPlot) {
            pPlot->SetLineDraw(FALSE);
        }
        break;     // 09/29/2000 - DDO
        /* !!not currently in use  AKE 8/25/00
        case IDD_CEPPITCH:    pPlot = new CPlotCepstralPitch();
        SetAreaGraph(TRUE); break; //  is an area processed graph
        */
    case IDD_ZCROSS:
        pPlot = new CPlotZCross();
        break;
    case IDD_POA:
        pPlot = new CPlotPOA();
        break;
        // Z-Graph graphs
    case IDD_3D:
        pPlot = new CPlot3D();
        /* SetAreaGraph(TRUE); */
        break; //  is an area processed graph
    case IDD_F1F2:
        pPlot = new CPlotF1F2();
        /* SetAreaGraph(TRUE); */
        break; //  is an area processed graph
    case IDD_F2F1:
        pPlot = new CPlotF2F1();
        /* SetAreaGraph(TRUE); */
        break; //  is an area processed graph
    case IDD_F2F1F1:
        pPlot = new CPlotF2F1F1();
        /* SetAreaGraph(TRUE); */
        break; //  is an area processed graph
    case IDD_SDP_A:
    case IDD_SDP_B:
        pPlot = new CZPlotPolar(plotID==IDD_SDP_A ? 0:1);
        SetAreaGraph(TRUE);
        break; //  is an area processed graph
    case IDD_INVSDP:
        pPlot = new CPlotInvSDP();
        SetAreaGraph(TRUE);
        break; // this is an area processed graph
    case ID_GRAPHS_OVERLAY:
        pPlot = new CMultiPlotWnd();
        break;
    }

    if (pPlot) {
        pPlot->SetParent(this);
    }

    return pPlot;
}

void CGraphWnd::MergeInGraph(CGraphWnd * pGraphToMerge, CSaView * pView, bool bRtOverlay) {
    ASSERT(pGraphToMerge);
    TRACE(_T("Merge %lp\n"),pGraphToMerge);

    if ((IsMergeableGraph(pGraphToMerge)) &&
            (IsMergeableGraph(this, TRUE))) {
        if (m_nPlotID != ID_GRAPHS_OVERLAY) {
            //the very first time, we convert the target graph from something (raw,etc..)
            //into an overlay graph and add the graph back into it.
            CPlotWnd * pPlot = m_pPlot->NewCopy();
            pPlot->SetParent(this);
            m_pPlot->DestroyWindow();
            m_pPlot = NULL;

            m_pPlot = NewPlotFromID(ID_GRAPHS_OVERLAY);
            ASSERT(m_nPlotID != IDD_BLANK);
            if (m_nPlotID != IDD_BLANK) {
                ((CMultiPlotWnd *)m_pPlot)->AddPlot(pPlot,m_nPlotID, (CSaView *)GetParent());
            }
            m_nPlotID = ID_GRAPHS_OVERLAY;

            // create the plot window
            CRect rWnd(0, 0, 0, 0);
            if (!m_pPlot->Create(NULL, _T("Plot"), WS_CHILD | WS_VISIBLE, rWnd, this, 0)) {
                delete m_pPlot;
                m_pPlot = NULL;
            }
            m_pPlot->SetPlotName("Overlay");
        }

        //now that we have an overlay graph, we add the new graph into it,
        //but first, if we are in "realtime autopitch mode, we need to remove previous overlay graphs.
        if (m_pPlot) {
            CMultiPlotWnd * pMPlot = (CMultiPlotWnd *)m_pPlot;

            pMPlot->RemoveRtPlots();

            if (pGraphToMerge->m_nPlotID == ID_GRAPHS_OVERLAY) {
                CMultiPlotWnd * pMPlot2 = (CMultiPlotWnd *)pGraphToMerge->m_pPlot;
                pMPlot->MergeInOverlayPlot(pMPlot2);
            } else {
                CPlotWnd * pPlot2 = pGraphToMerge->m_pPlot->NewCopy();
                pPlot2->SetParent(this);
                if (bRtOverlay) {
                    pPlot2->SetRtOverlay();
                }
                pMPlot->AddPlot(pPlot2,pGraphToMerge->m_nPlotID, pView);
            }
        }
    }
}

void CGraphWnd::RemoveOverlayItem(const CPlotWnd * pPlot) {
    if (m_nPlotID == ID_GRAPHS_OVERLAY) {
        CMultiPlotWnd * pMPlot = (CMultiPlotWnd *)m_pPlot;
        pMPlot->RemovePlot(pPlot);
    }
}

BOOL CGraphWnd::IsMergeableGraph(CGraphWnd * pGraphToMerge, BOOL bBaseGraph) {
    CSaView * pView = (CSaView *)pGraphToMerge->GetParent(); // get pointer to parent view
    BOOL ret = FALSE;

    switch (pGraphToMerge->m_nPlotID) {
    case IDD_SPECTROGRAM:
        ret = bBaseGraph;  // Should be true only if graph is the base graph
        break;

    case IDD_LOUDNESS:   // magnitude
    case IDD_GRAPITCH:  // cecil auto pitch
    case ID_GRAPHS_OVERLAY:
    case IDD_RAWDATA:    // waveform
    case IDD_ZCROSS:     // zero crossing
    case IDD_CHANGE:     // cecil change
    case IDD_GLOTWAVE:   // glottal waveform
    case IDD_PITCH:
    case IDD_CHPITCH:
    case IDD_SMPITCH:
        ret = TRUE;
        break;

    case IDD_MELOGRAM:
        ret = !(pView->IDDSelected(pView->GetGraphIDs(),IDD_TWC) || pView->IDDSelected(pView->GetGraphIDs(),IDD_MAGNITUDE));
        break;

        // if you add a new one to the list, you at least
        // need to write a NewCopy() method for the corresponding class.
    case IDD_CEPPITCH:  // supposed to enable this one. (but its an area graph)

        // could enable these easily
    case IDD_DURATION:
    case IDD_POA:

        //////////////////  pja 5/8/00
    case IDD_MAGNITUDE:
        /////////////////

    default:
        ret = FALSE;
    }
    return ret;
}

static LPCSTR psz_sagraph		= "sagraph";
static LPCSTR psz_plotid		= "plotid";
static LPCSTR psz_showlegend	= "showlegend";
static LPCSTR psz_showxscale	= "showxscale";
static LPCSTR psz_annotwnd		= "annotwnd";
static LPCSTR psz_showwnd		= "showwnd";
static LPCSTR psz_showboundaries = "showboundaries";

void CGraphWnd::WriteProperties(CObjectOStream & obs) {

    obs.WriteBeginMarker(psz_sagraph);

    obs.WriteBool(psz_showlegend, m_bLegend);
    obs.WriteBool(psz_showxscale, m_bXScale);
    obs.WriteBeginMarker(psz_annotwnd);
    for (int i=0; i< ANNOT_WND_NUMBER; i++) {
        obs.WriteBool(psz_showwnd, m_abAnnWnd[i]);
    }
    obs.WriteEndMarker(psz_annotwnd);
    obs.WriteBool(psz_showboundaries, m_bBoundaries);
	obs.WriteEndMarker(psz_sagraph);
}

BOOL CGraphWnd::ReadProperties(CObjectIStream & obs) {
    
	if (!obs.bReadBeginMarker(psz_sagraph)) {
        return FALSE;
    }
	BOOL showBoundaries = FALSE;
    while (!obs.bAtEnd()) {
        if (obs.bReadBool(psz_showlegend, m_bLegend));
        else if (obs.bReadBool(psz_showxscale, m_bXScale));
        else if (obs.bReadBool(psz_showboundaries, showBoundaries));
        else if (obs.bReadBeginMarker(psz_annotwnd)) {
            for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
                if (!obs.bReadBool(psz_showwnd, m_abAnnWnd[i])) {
                    break;
                }
            }
            obs.SkipToEndMarker(psz_annotwnd);
        } else if (obs.bReadEndMarker(psz_sagraph)) {
            break;
        } else {
			// Skip unexpected field
            obs.ReadMarkedString();  
        }
    }

	m_bBoundaries = (showBoundaries)?true:false;

    return TRUE;
}

/***************************************************************************/
// CGraphWnd::SetProperties Set graph's default properties
//
// Set default properties for specified graph
// (needed for "FileOpenAs->Phonetic/Music Analysis")
/***************************************************************************/
BOOL CGraphWnd::SetProperties(int nID) {

	m_bBoundaries = (nID == IDD_RAWDATA);
    m_bXScale = !(nID == IDD_MAGNITUDE || nID == IDD_STAFF || nID == IDD_POA);
    m_bLegend = !(nID == IDD_POA);

    //**********************************************************************
    // 10/23/2000 - DDO  This considers the case when the melogram is
    // turned on but neither the TWC or magnitude are. When that's the case
    // then the melogram's legend is on by default.
    //**********************************************************************
    CSaView * pView = ((CMainFrame *)AfxGetMainWnd())->GetCurrSaView();
    if (pView) {
        if ((nID == IDD_MELOGRAM) && (pView->GetGraphIndexForIDD(IDD_TWC) == -1)) {
            m_bLegend = TRUE;
        } else if ((nID == IDD_MAGNITUDE) && (pView->GetGraphIndexForIDD(IDD_TWC) == -1)) {
            CGraphWnd * pGraph = (CGraphWnd *)pView->GraphIDtoPtr(IDD_MELOGRAM);
            if (pGraph!=NULL) {
                m_bLegend = pGraph->HasLegend();
            }
        }
    }

    if ((nID == IDD_TWC) || (nID == IDD_STAFF)) {
        ShowCursors(false,false);
    }
    SetLineDraw(nID != IDD_MAGNITUDE);

    for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
        m_abAnnWnd[i] = ((nID == IDD_RAWDATA) && (i<MUSIC_PL1));
    }

    return TRUE;
}

/***************************************************************************/
// CGraphWnd::SetGraphFocus Set/reset focus for the graph and propagate
// to the plot window.
/***************************************************************************/
void CGraphWnd::SetGraphFocus(BOOL bFocus) {
    CMiniCaptionWnd::SetGraphFocus(bFocus);
    m_pPlot->GraphHasFocus(bFocus);
}

/***************************************************************************/
// CGraphWnd::AnimateFrame  Call on plot to animate one frame if graph
// contains a plot that can be animated.
/***************************************************************************/
void CGraphWnd::AnimateFrame(DWORD dwFrameIndex) {
    if (m_pPlot->IsAnimationPlot()) {
        m_pPlot->AnimateFrame(dwFrameIndex);
        //!! update annotation here
    }
}

/***************************************************************************/
// CGraphWnd::EndAnimation  Call on plot to terminate animation if graph
// contains a plot that can be animated.
/***************************************************************************/
void CGraphWnd::EndAnimation() {
    if (m_pPlot->IsAnimationPlot()) {
        m_pPlot->EndAnimation();
    }
}

IMPLEMENT_DYNCREATE(CRecGraphWnd, CGraphWnd)

/***************************************************************************/
/***************************************************************************/
CRecGraphWnd::CRecGraphWnd() : CGraphWnd(IDD_RECORDING) {
}

/***************************************************************************/
// CRecGraphWnd::PreCreateWindow Creation
// Called from the framework before the creation of the window. Registers
// the new window class and changes the style as desired.
//
// 09/22/2000 - DDO Created
/***************************************************************************/
BOOL CRecGraphWnd::PreCreateWindow(CREATESTRUCT & cs) {
	CSaApp * pApp = (CSaApp*)AfxGetApp();
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
                                       pApp->LoadStandardCursor(IDC_ARROW), 0,
                                       LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_SPEECHANALYZER)));

    BOOL bRet = CWnd::PreCreateWindow(cs);
    cs.style = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER;
    return bRet;
}

void CGraphWnd::HideCursors() {
    if (m_pPlot!=NULL) {
        m_pPlot->HideCursors();
    }
}

void CGraphWnd::ShowCursors() {
    if (m_pPlot!=NULL) {
        m_pPlot->ShowCursors();
    }
}

CPoint CGraphWnd::GetPopupMenuPosition() {
	return m_PopupMenuPos;
}

void CGraphWnd::SetPopupMenuLocation( CPoint point) {
	m_PopupMenuPos = point;
}
