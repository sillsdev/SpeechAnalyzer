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
//         RLJ Added bSetProperties(int nNewID)
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
#include "sa_minic.h"
#include "sa_graph.h"
#include "sa_plot.h"
#include "SpectroParm.h"
#include "Segment.h"
#include "math.h"
#include "sa_doc.h"
#include "sa_view.h"
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
#include "dsp\signal.h"
#include "dsp\spectgrm.h"
#include "dsp\formants.h"
#include "dsp\ztransform.h"
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
#include "settings\obstream.h"

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

/////////////////////////////////////////////////////////////////////////////
// CGraphWnd message map

BEGIN_MESSAGE_MAP(CGraphWnd, CMiniCaptionWnd)
    //{{AFX_MSG_MAP(CGraphWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_SETFOCUS()
    ON_MESSAGE(WM_USER_INFO_GENDERCHANGED, OnGenderInfoChanged)
    ON_MESSAGE(WM_USER_CURSOR_CLICKED, OnCursorClicked)
    ON_WM_NCMOUSEMOVE()
    ON_WM_DESTROY()
    ON_WM_CHAR()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGraphWnd construction/destruction/creation

/***************************************************************************/
// CGraphWnd::CGraphWnd Constructor
/***************************************************************************/
CGraphWnd::CGraphWnd(UINT nID)
{
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        m_apAnnWnd[nLoop] = NULL;
        m_abAnnWnd[nLoop] = FALSE;
    }
    m_pPlot = NULL;
    m_pLegend = NULL;
    m_pXScale = NULL;
    m_bLegend = FALSE;
    m_bXScale = FALSE;
    m_bAreaGraph = FALSE;
    m_pPlot = NewPlotFromID(nID);
    m_nPlotID = nID;
    if (!m_pPlot)
    {
        m_pPlot = new CPlotBlank();
        m_pPlot->SetParent(this);
        m_nPlotID = IDD_BLANK;
    }
}

int CGraphWnd::m_anAnnWndOrder[] =
{REFERENCE, PHONETIC, TONE, PHONEMIC, ORTHO, GLOSS, MUSIC_PL1,MUSIC_PL2,MUSIC_PL3,MUSIC_PL4};

CGraphWnd::CGraphWnd(const  CGraphWnd & toBeCopied)
{
    Copy(toBeCopied);
}

CGraphWnd & CGraphWnd::operator=(const CGraphWnd & fromThis)
{
    if (&fromThis != this)
    {
        Clear();
        Copy(fromThis);
    }

    return *this;
}

/***************************************************************************/
/***************************************************************************/
void CGraphWnd::Clear(void)
{
    if (m_pPlot)
    {
        delete m_pPlot;
    }
    if (m_pLegend)
    {
        delete m_pLegend;
    }
    if (m_pXScale)
    {
        delete m_pXScale;
    }

    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        if (m_apAnnWnd[nLoop])
        {
            delete m_apAnnWnd[nLoop];
        }
    }
}

/***************************************************************************/
/***************************************************************************/
void CGraphWnd::Copy(const CGraphWnd & fromThis)
{
    this->m_pPlot   = NULL;
    m_pPlot         = NULL;
    m_pLegend       = NULL;
    m_pXScale       = NULL;
    m_nPlotID       = fromThis.m_nPlotID;
    m_bAreaGraph    = fromThis.m_bAreaGraph;
    if (m_nPlotID == ID_GRAPHS_OVERLAY)
    {
        m_nPlotID = IDD_BLANK;

        CMultiPlotWnd * pPlot = (CMultiPlotWnd *)fromThis.GetPlot();
        if (pPlot && pPlot->IsKindOf(RUNTIME_CLASS(CMultiPlotWnd)))
        {
            m_nPlotID = pPlot->GetBasePlotID();  // save base plot
        }
    }
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        m_apAnnWnd[nLoop] = NULL;
    }

    PartialCopy(fromThis);
}

/***************************************************************************/
/***************************************************************************/
void  CGraphWnd::PartialCopy(const CGraphWnd & fromThis)
{
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        m_abAnnWnd[nLoop] = fromThis.m_abAnnWnd[nLoop];
    }

    m_bLegend = fromThis.m_bLegend;
    m_bXScale = fromThis.m_bXScale;
}

/***************************************************************************/
// CGraphWnd::~CGraphWnd Destructor
/***************************************************************************/
CGraphWnd::~CGraphWnd()
{
    Clear();
}

/////////////////////////////////////////////////////////////////////////////
// CGraphWnd helper functions

/***************************************************************************/
// CGraphWnd::DisableLegend Is legend window disabled for this graph type
/***************************************************************************/
BOOL CGraphWnd::DisableLegend()
{
    BOOL result = FALSE;
    return result;
}

/***************************************************************************/
// CGraphWnd::ShowLegend Show or hide the legend window
/***************************************************************************/
void CGraphWnd::ShowLegend(BOOL bShow, BOOL bRedraw)
{
    m_bLegend = bShow && !DisableLegend();

    //***********************************************
    // 10/13/2000 - DDO Make sure the melogram and
    // magnitude legend states are the same. But
    // make sure we don't get into a recursion stack
    // overflow when calling ShowLegend.
    //***********************************************
    static BOOL bRecursion;

    if (!bRecursion)
    {
        if (IsPlotID(IDD_MELOGRAM))
        {
            CSaView * pView = (CSaView *)GetParent();
            CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_MAGNITUDE);
            bRecursion = TRUE;
            if (pGraph)
            {
                pGraph->ShowLegend(m_bLegend, bRedraw);
            }
            bRecursion = FALSE;
        }
        else if (IsPlotID(IDD_MAGNITUDE))
        {
            CSaView * pView = (CSaView *)GetParent();
            CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_MELOGRAM);
            bRecursion = TRUE;
            if (pGraph)
            {
                pGraph->ShowLegend(m_bLegend, bRedraw);
            }
            bRecursion = FALSE;
        }
    }

    if (bRedraw)
    {
        ResizeGraph(TRUE, TRUE);    // repaint whole graph window
    }
}

/***************************************************************************/
// CGraphWnd::DisableXScale Is x-scale window disabled for this graph type
/***************************************************************************/
BOOL CGraphWnd::DisableXScale()
{
    BOOL result = FALSE;

    switch (m_nPlotID)
    {
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
void CGraphWnd::ShowXScale(BOOL bShow, BOOL bRedraw)
{
    m_bXScale = bShow && !DisableXScale();

    if (bRedraw)
    {
        ResizeGraph(TRUE, TRUE);    // repaint whole graph window
    }
}

/***************************************************************************/
// CGraphWnd::DisableAnnotation Is annotation window disabled for this graph type
/***************************************************************************/
BOOL CGraphWnd::DisableAnnotation(int /*nIndex*/)
{
    BOOL result = FALSE;

    switch (m_nPlotID)
    {
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
void CGraphWnd::ShowAnnotation(int nIndex, BOOL bShow, BOOL bRedraw)
{
    m_abAnnWnd[nIndex] = bShow && !DisableAnnotation(nIndex);

    if (bRedraw)
    {
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
void CGraphWnd::SetCaptionStyle(int nStyle, BOOL bRedraw)
{
    m_nCaption = nStyle;
    // set caption height
    switch (m_nCaption)
    {
    case Normal:
        m_iNCHeight = GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER);
        break;
    case Mini:
    case MiniWithCaption:
        m_iNCHeight = MINICAPTION_HEIGHT;
        break;
    default:
        m_iNCHeight = 1; // because of 3D border
        break;
    }
    if (bRedraw)
    {
        // force a repaint of the whole window with NC area
        CSaView * pView = (CSaView *)GetParent(); // get pointer to parent view
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
void CGraphWnd::ScrollGraph(CSaView * pView, DWORD dwNewPos, DWORD dwOldPos)
{
    //TRACE("ScrollGraph %lp %d %d\n",pView,dwNewPos,dwOldPos);
    if (m_bAreaGraph)
    {
        return;    // don't do anything
    }
    if (!HaveCursors())
    {
        // no cursors visible
        if (HavePrivateCursor())
        {
            return;    // don't do anything
        }
        else
        {
            RedrawGraph();    // redraw graph, without legend
        }
        return;
    }

    CRect rWnd;
    GetClientRect(rWnd);
    if (m_bLegend)
    {
        rWnd.left = m_pLegend->GetWindowWidth();
    }

    if (m_pPlot)
    {
        m_pPlot->GetClientRect(rWnd);
    }

    DWORD dwDataFrame = pView->AdjustDataFrame(rWnd.Width()); // number of data points displayed

    BOOL bLessThanPage = ((dwNewPos > dwOldPos) && ((dwNewPos - dwOldPos) < dwDataFrame)) ||
                         ((dwOldPos > dwNewPos) && ((dwOldPos - dwNewPos) < dwDataFrame));
    bLessThanPage = FALSE;

    // check if there is less than a page to scroll
    if (bLessThanPage)
    {
        // prepare scrolling
        double fBytesPerPix = dwDataFrame / (double)rWnd.Width(); // calculate data samples per pixel
        int nRealScroll = round((double)dwOldPos/fBytesPerPix)-round((double)dwNewPos/fBytesPerPix);

        if (nRealScroll)
        {
            // scroll the plot window
            m_pPlot->ScrollPlot(pView, nRealScroll, dwOldPos, dwDataFrame);
            // scroll the annotation windows
            for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
            {
                if (m_abAnnWnd[nLoop])
                {
                    m_apAnnWnd[nLoop]->ScrollWindow(nRealScroll, 0, 0, 0);
                    m_apAnnWnd[nLoop]->UpdateWindow();
                }
            }
            if (m_bXScale)
            {
                m_pXScale->InvalidateRect(NULL);
                m_pXScale->UpdateWindow();

            }
        }
    }
    else     // page scroll
    {
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
void CGraphWnd::ResizeGraph(BOOL bEntire, BOOL bLegend, BOOL bGraph)
{
    if (bEntire)
    {
        // save bottom
        CRect rWnd;
        GetClientRect(rWnd);
        int nBottom = rWnd.bottom;
        // move legend window
        if (m_bLegend)
        {
            // invalidate graph without legend
            if (!bLegend)
            {
                rWnd.left = m_pLegend->GetWindowWidth();
            }
            if (bGraph)
            {
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
        }
        else
        {
            // invalidate entire graph window
            InvalidateRect(NULL);
            m_pLegend->MoveWindow(0, 0, 0, 0, FALSE); // hide window
        }
        // move annotation windows
        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
        {
            // SDM 1.5Test8.1
            if (m_abAnnWnd[m_anAnnWndOrder[nLoop]])
            {
                // move annotation window
                rWnd.bottom = rWnd.top + m_apAnnWnd[m_anAnnWndOrder[nLoop]]->GetWindowHeight();
                m_apAnnWnd[m_anAnnWndOrder[nLoop]]->MoveWindow(rWnd, TRUE);
                rWnd.top = rWnd.bottom;
            }
            else
            {
                m_apAnnWnd[m_anAnnWndOrder[nLoop]]->MoveWindow(0, 0, 0, 0, FALSE);    // hide window
            }
        }
        // move x-scale window
        if (m_bXScale)
        {
            // move x-scale window
            rWnd.bottom = nBottom;
            nBottom = rWnd.top;
            rWnd.top = rWnd.bottom - m_pXScale->GetWindowHeight();
            m_pXScale->MoveWindow(rWnd, TRUE);
            rWnd.bottom = rWnd.top;
            rWnd.top = nBottom;
            nBottom = rWnd.bottom;
        }
        else
        {
            m_pXScale->MoveWindow(0, 0, 0, 0, FALSE);    // hide window
        }
        // move plot window
        rWnd.bottom = nBottom;
        m_pPlot->MoveWindow(rWnd, TRUE);
        if (HavePrivateCursor())
        {
            m_pPlot->SetInitialPrivateCursor();    // initialize private cursor
        }
        // SDM 1.06.6U5
        if (HaveCursors())
        {
            m_pPlot->SetStartCursor((CSaView *)GetParent());
            m_pPlot->SetStopCursor((CSaView *)GetParent());
        }

        //***********************************************************
        // 09/26/2000 - DDO If the graph we're Resizing is the
        // melogram graph then also Resize the TWC graph if it
        // exists.
        //***********************************************************
        if (IsPlotID(IDD_MELOGRAM))
        {
            CSaView * pView = (CSaView *)GetParent();
            CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_TWC);
            if (pGraph)
            {
                pGraph->ResizeGraph(TRUE, pGraph->HaveLegend());
            }
        }
    }

    else
    {
        m_pPlot->RedrawPlot(bEntire);
    }
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
void CGraphWnd::RedrawGraph(BOOL bEntire, BOOL bLegend, BOOL bGraph)
{
    // redraw the plot window
    m_pPlot->RedrawPlot(bEntire);

    if (bGraph)
    {
        InvalidateRect(NULL, FALSE);
    }

    if (bEntire)
    {
        // redraw the annotation windows
        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
        {
            if (m_abAnnWnd[nLoop])
            {
                m_apAnnWnd[nLoop]->InvalidateRect(NULL);
            }
        }

        if (m_bXScale)
        {
            m_pXScale->InvalidateRect(NULL);
        }

        if (m_bLegend && bLegend)
        {
            m_pLegend->InvalidateRect(NULL);
        }

        if (HaveCursors())
        {
            m_pPlot->SetStartCursor((CSaView *)GetParent());
            m_pPlot->SetStopCursor((CSaView *)GetParent());
        }

        //***********************************************************
        // 09/26/2000 - DDO If the graph we're redrawing is the
        // melogram graph then also redraw the TWC graph if it
        // exists.
        //***********************************************************
        if (IsPlotID(IDD_MELOGRAM))
        {
            CSaView * pView = (CSaView *)GetParent();
            CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_TWC);
            if (pGraph)
            {
                pGraph->RedrawGraph(TRUE, pGraph->HaveLegend());
            }
        }
    }
}

/***************************************************************************/
// CGraphWnd::GetSemitone Calculates semitones from a given frequency
/***************************************************************************/
double CGraphWnd::GetSemitone(double fFreq)
{
    // CLW 4/7/00
    // more accurate and compact way of calculating semitone
    // semi-tone 0 aligns with MIDI# 0 at 8.176Hz

    return log(fFreq / 440.0) / log(2.0) * 12.0 + 69.0;
}

/***************************************************************************/
// CGraphWnd::SemitoneToFrequency Calculates frequency from a given semitone
/***************************************************************************/
double CGraphWnd::SemitoneToFrequency(double fSemitone)
{
    return 440.0 * pow(2.0, (fSemitone - 69.0) / 12.0);
}

/***************************************************************************/
// CGraphWnd::NoteNum2Name Calculates frequency from a given semitone
/***************************************************************************/
CSaString CGraphWnd::Semitone2Name(double fSemitone)
{
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
    if (fSemitone < 21.)
    {
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


static void UpdatePitchStatusBarNote(double fPitchData, double fUncertainty, BOOL bShowNoteName)
{
    // get pointer to status bar
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CDataStatusBar * pStat = (CDataStatusBar *)pMainWnd->GetDataStatusBar();
    CSaString szText;

    if (fPitchData > 0.F)
    {
        // calculate frequency value
        double fFreqDecPlaces = (short)floor(-log10(fUncertainty) + .7);
        double fMultiplier = pow(10.0, -fFreqDecPlaces);
        fPitchData = round(fPitchData / fMultiplier) * fMultiplier;
        if (fFreqDecPlaces < 0.)
        {
            fFreqDecPlaces = 0.;
        }
        // calculate semitone value
        double fSemitoneUncertainty = 12.0 * log((fPitchData + fUncertainty) / fPitchData) / log(2.);
        double fSemitoneDecPlaces = floor(-log10(fSemitoneUncertainty) + .7);
        // update semitone pane
        double fSemitone = CGraphWnd::GetSemitone(fPitchData);
        szText.Format(_T("      %.*f st"), (int)fSemitoneDecPlaces, fSemitone);
        if (bShowNoteName)
        {
            szText += _T(" (") + CSaString(CGraphWnd::Semitone2Name(fSemitone)) + _T(")");
        }
        CSaString test = CSaString(CGraphWnd::Semitone2Name(fSemitone));
        pStat->SetPaneSymbol(ID_STATUSPANE_NOTE);
        pStat->SetPaneText(ID_STATUSPANE_NOTE, szText);
        // update frequency pane
        szText.Format(_T("      %.*f Hz"), (int)fFreqDecPlaces, fPitchData);
        pStat->SetPaneSymbol(ID_STATUSPANE_PITCH); // switch symbol on
        pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);
    }
    else
    {
        szText = _T(" ");
        pStat->SetPaneSymbol(ID_STATUSPANE_NOTE); // amplitude symbol
        pStat->SetPaneText(ID_STATUSPANE_NOTE, szText);  // amplitude indicator

        pStat->SetPaneSymbol(ID_STATUSPANE_PITCH); // switch symbol on
        pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);
    }
}

static void UpdatePitchStatusBar(double fPitchData, double fUncertainty)
{
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
void CGraphWnd::UpdateStatusBar(DWORD dwStartCursor, DWORD dwStopCursor, BOOL bForceUpdate)
{
    // get pointer to parent view and to document
    CSaView * pView = (CSaView *)GetParent();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    UINT nSmpSize = pDoc->GetSampleSize();  // number of bytes per sample
    // check if this graph has focus
    if ((m_bFocus) && (pView->ViewIsActive()))
    {
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
        //if ((dwStartCursor == 0) && (dwStopCursor == 0))
        if (HavePrivateCursor())
        {
            // this is a private cursor graph
            CProcessSpectrum * pSpectrum = NULL;
            CSpectrumParm * pSpectrumParm = NULL;
            CProcessTonalWeightChart * pTWC;
            switch (m_nPlotID)
            {
            case IDD_SPECTRUM:
                m_pPlot->GetClientRect(rWnd); // get plot window size
                nProcessIndex = m_pPlot->GetPrivateCursorPosition();
                pSpectrum = (CProcessSpectrum *)pDoc->GetSpectrum(); // get pointer to spectrum object

                if (pSpectrum->IsDataReady())
                {
                    pSpectrumParm = pSpectrum->GetSpectrumParms();
                    double SigBandwidth = (double)pDoc->GetSamplesPerSec() / 2.0;
                    double ScaleFactor = 1. + pSpectrumParm->nFreqScaleRange;
                    int nFreqLowerBound = 0;
                    int nFreqUpperBound = (int)(ceil(SigBandwidth / ScaleFactor));

                    // write to frequency pane
                    //fData = (double)pSpectrumParm->nFreqLowerBound + (double)(pSpectrumParm->nFreqUpperBound - pSpectrumParm->nFreqLowerBound) * (double)nProcessIndex / (double)(rWnd.Width() - 1);
                    fData = (double)nFreqLowerBound + (double)(nFreqUpperBound - nFreqLowerBound) * (double)nProcessIndex / (double)(rWnd.Width() - 1);
                    if (nPitchMode == HERTZ)
                    {
                        swprintf_s(szText, _countof(szText),_T("    %.1f Hz"), fData);
                    }
                    else
                    {
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
                    if (pSpectrum->GetSpectralData(0).Smooth != (float)UNDEFINED_DATA)
                    {
                        fData = 0.;
                        for (short i = LBandEdge; i < UBandEdge; i++)
                        {
                            fData += (double)pSpectrum->GetSpectralData((unsigned short)i).Smooth;
                        }
                        if (UBandEdge != LBandEdge)
                        {
                            fData /= (double)(UBandEdge - LBandEdge);    // average the power within the band
                        }
                        if (pSpectrumParm->nScaleMode == DB)
                        {
                            fData = (fData == 0.)?MIN_LOG_PWR:10.*log10((double)fData) - pSpectrum->GetSpectralPowerRange().fdBRef;
                        }
                        bRes = TRUE;
                    }
                    else
                    {
                        bRes = FALSE;
                    }
                    swprintf_s(szText, _countof(szText),_T("Cep:"));
                    if (bRes && pSpectrumParm->bShowCepSpectrum)
                    {
                        if (pSpectrumParm->nScaleMode)
                        {
                            swprintf_s(szText, _countof(szText),_T("Cep: %5.1f"), fData);
                        }
                        else
                        {
                            swprintf_s(szText, _countof(szText),_T("Cep: %5.1f dB"), fData);
                        }
                    }
                    // write to cepstral spectrum pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_CEPSTRALSPECTRUM); // switch symbol on
                    pStat->SetPaneText(ID_STATUSPANE_CEPSTRALSPECTRUM, szText);

                    bRes = FALSE;
                    if (pSpectrum->GetSpectralData(0).Raw != (float)UNDEFINED_DATA)
                    {

                        fData = 0.;
                        for (short i = LBandEdge; i < UBandEdge; i++)
                        {
                            fData += (double)pSpectrum->GetSpectralData((unsigned short)i).Raw;
                        }
                        if (UBandEdge != LBandEdge)
                        {
                            fData /= (double)(UBandEdge - LBandEdge);    // average the power within the band
                        }
                        if (pSpectrumParm->nScaleMode == DB)
                        {
                            fData = (fData == 0.)?MIN_LOG_PWR:10.*log10((double)fData) - pSpectrum->GetSpectralPowerRange().fdBRef;
                        }
                        bRes = TRUE;
                    }
                    else
                    {
                        bRes = FALSE;
                    }
                    swprintf_s(szText, _countof(szText),_T("Raw:"));
                    if (bRes)
                    {
                        if (pSpectrumParm->nScaleMode)
                        {
                            swprintf_s(szText, _countof(szText),_T("Raw: %5.1f"), fData);
                        }
                        else
                        {
                            swprintf_s(szText, _countof(szText),_T("Raw: %5.1f dB"), fData);
                        }
                    }
                    // write to raw spectrum pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_RAWSPECTRUM); // switch symbol on
                    pStat->SetPaneText(ID_STATUSPANE_RAWSPECTRUM, szText);

                    bRes = FALSE;
                    if (pSpectrum->GetSpectralData(0).Lpc != (float)UNDEFINED_DATA)
                    {
                        fData = 0.;
                        for (short i = LBandEdge; i < UBandEdge; i++)
                        {
                            fData += (double)pSpectrum->GetSpectralData((unsigned short)i).Lpc;
                        }
                        if (UBandEdge != LBandEdge)
                        {
                            fData /= (double)(UBandEdge - LBandEdge);    // average the power within the band
                        }
                        double LpcRef;
                        double MaxLpcBandPwr = pSpectrum->GetSpectralPowerRange().Max.Lpc;
                        double MaxRawBandPwr = pSpectrum->GetSpectralPowerRange().Max.Raw;
                        if (pSpectrumParm->nScaleMode == DB)
                        {
                            if (MaxRawBandPwr != (float)UNDEFINED_DATA)
                            {
                                MaxRawBandPwr = (MaxRawBandPwr == 0.)?MIN_LOG_PWR:10.*log10((double)MaxRawBandPwr) - pSpectrum->GetSpectralPowerRange().fdBRef;
                                MaxLpcBandPwr = (MaxLpcBandPwr == 0.)?MIN_LOG_PWR:10.*log10((double)MaxLpcBandPwr);
                                LpcRef = MaxLpcBandPwr - MaxRawBandPwr;
                            }
                            else
                            {
                                LpcRef = pSpectrum->GetSpectralPowerRange().fdBRef;
                            }
                            fData = (fData == 0.)?MIN_LOG_PWR:(10.*log10(fData) - LpcRef);
                        }
                        else
                        {
                            if (MaxRawBandPwr != (float)UNDEFINED_DATA && MaxRawBandPwr != 0.F)
                            {
                                LpcRef = MaxLpcBandPwr / MaxRawBandPwr;
                            }
                            else
                            {
                                LpcRef = pow(10.F, pSpectrum->GetSpectralPowerRange().fdBRef / 10.F);
                            }
                            fData /= LpcRef;
                        }
                        bRes = TRUE;
                    }
                    else
                    {
                        bRes = FALSE;
                    }
                    swprintf_s(szText, _countof(szText),_T("LPC:"));
                    if (bRes)
                    {
                        if (pSpectrumParm->nScaleMode)
                        {
                            swprintf_s(szText, _countof(szText),_T("LPC: %5.1f"), fData);
                        }
                        else
                        {
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
                if (pTWC->IsDataReady())
                {
                    m_pPlot->UpdateWindow(); // force correct legend settings
                    m_pPlot->GetClientRect(rWnd); // get plot window size
                    int nCursorHeight = (short)(rWnd.Height() - (m_pPlot->GetPrivateCursorPosition()));
                    CLegendWnd * pLegend = GetLegendWnd();
                    double fSemitoneOffset = -static_cast<CPlotTonalWeightChart *>(m_pPlot)->GetSemitoneOffset();
                    double fSemitone;
                    double fMinScale  = pLegend->GetScaleMinValue() + fSemitoneOffset;
                    if (nCursorHeight)
                    {
                        double fSemitoneAbs = double(nCursorHeight)/(pLegend->GetGridDistance() / pLegend->GetScaleBase()) + fMinScale;
                        double fBinsPerSemitone = pTWC->GetBinsPerSemitone();
                        dwDataPos = DWORD((fSemitoneAbs - pTWC->GetMinSemitone())*fBinsPerSemitone) + 1;
                        fSemitone = double(dwDataPos)/fBinsPerSemitone + pTWC->GetMinSemitone();
                        nData = pTWC->GetProcessedData(dwDataPos, &bRes);
                    }
                    else
                    {
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
                }
                else
                {
                    bRes = FALSE;
                }
                break;
            default:
                break;
            }
            return;
        }
        // this graph and the parent view have focus
        if ((!bForceUpdate) && (dwStartCursor == m_dwLastStartCursor) && (dwStopCursor == m_dwLastStopCursor))
        {
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

        switch (m_nPlotID)
        {
        case IDD_SPECTROGRAM:
        case IDD_SNAPSHOT:
        {
            CProcessFormantTracker * pSpectroFormants = pDoc->GetFormantTracker();
            // prepare spectrograms formant data, if ready
            CProcessSpectrogram * pSpectrogram = pDoc->GetSpectrogram(m_nPlotID==IDD_SPECTROGRAM); // get pointer to spectrogram object
            const CSpectroParm * pSpectroParm = &pSpectrogram->GetSpectroParm();
            BOOL bShowFormants = pSpectroParm->bShowF1 || pSpectroParm->bShowF2 || pSpectroParm->bShowF3 || pSpectroParm->bShowF4 || pSpectroParm->bShowF5andUp;
            if (bShowFormants && pSpectroFormants->IsDataReady())
            {
                double fSizeFactor = (double)pDoc->GetDataSize() / (double)(pSpectroFormants->GetDataSize() - 1);
                dwDataPos = (DWORD)((DWORD)(dwStartCursor / fSizeFactor * 2 / sizeof(FORMANT_FREQ))) * sizeof(FORMANT_FREQ) / 2;
                FORMANT_FREQ * pFormFreqCurr = (FORMANT_FREQ *)pSpectroFormants->GetProcessedData(dwDataPos, sizeof(FORMANT_FREQ));

                for (int n = 1; n < 5; n++)
                {
                    if (pFormFreqCurr->F[n] > 0)
                    {
                        if (nPitchMode == HERTZ)
                        {
                            swprintf_s(szText,_countof(szText), _T("F%d: %.1f Hz"), n, pFormFreqCurr->F[n]);
                        }
                        else
                        {
                            swprintf_s(szText,_countof(szText), _T("F%d: %.1f st"), GetSemitone(pFormFreqCurr->F[n]));
                        }
                    }
                    else
                    {
                        swprintf_s(szText,_countof(szText), _T("F%d:"), n);
                    }
                    // write to position pane
                    pStat->SetPaneSymbol(ID_STATUSPANE_POSITION + n - 1, FALSE); // switch symbol off
                    pStat->SetPaneText(ID_STATUSPANE_POSITION + n - 1, szText);
                }
            }
            else
            {
                // show frequency and power at mouse pointer
                const BOOL bPreEmphasis = TRUE;
                SIG_PARMS Signal;
                const BOOL bBlockBegin = TRUE;
                Signal.SmpRate = pDoc->GetSamplesPerSec();
                Signal.Length = DspWin::CalcLength(pSpectroParm->Bandwidth(), Signal.SmpRate, ResearchSettings.m_cWindow.m_nType);
                DWORD dwHalfFrameSize = (Signal.Length/2) * nSmpSize;
                CPoint MousePosn = m_pPlot->GetMousePointerPosition();
                DWORD dwWaveOffset = m_pPlot->CalcWaveOffsetAtPixel(MousePosn);
                if (MousePosn.x != UNDEFINED_OFFSET &&
                        (!m_bAreaGraph || (m_bAreaGraph && pSpectrogram->IsDataReady())) &&
                        ((dwWaveOffset >= dwHalfFrameSize)*nSmpSize && (dwWaveOffset < pDoc->GetDataSize() - dwHalfFrameSize)))
                {
                    CRect rPlotWnd;
                    m_pPlot->GetClientRect(rPlotWnd);
                    float fFreq = (float)(rPlotWnd.bottom - MousePosn.y) * (float)pSpectroParm->nFrequency / (float)rPlotWnd.bottom;
                    float fPowerInDb;

                    DWORD dwFrameStart = dwWaveOffset - dwHalfFrameSize;
                    Signal.Start = pDoc->GetWaveData(dwFrameStart, bBlockBegin);
                    Signal.SmpDataFmt = (nSmpSize == 1) ? (int8)PCM_UBYTE: (int8)PCM_2SSHORT;
                    Spectrogram::CalcPower(&fPowerInDb, fFreq, pSpectroParm->nResolution, (CWindowSettings::Type)ResearchSettings.m_cWindow.m_nType, Signal, bPreEmphasis);

                    swprintf_s(szText,_countof(szText), _T("%7.1f Hz"), fFreq);
                    pStat->SetPaneSymbol(ID_STATUSPANE_3, FALSE);
                    pStat->SetPaneText(ID_STATUSPANE_3, szText);
                    swprintf_s(szText,_countof(szText), _T("%6.1f dB"), fPowerInDb);
                    pStat->SetPaneSymbol(ID_STATUSPANE_4, FALSE);
                    pStat->SetPaneText(ID_STATUSPANE_4, szText);
                }
                else
                {
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
        case IDD_F2F1F1:
        {
            CProcessFormantTracker * pSpectroFormants = pDoc->GetFormantTracker();
            if (pSpectroFormants->IsDataReady())
            {
                double fSizeFactor = (double)pDoc->GetDataSize() / (double)(pSpectroFormants->GetDataSize() - 1);
                dwDataPos = (DWORD)((DWORD)(dwStartCursor / fSizeFactor * 2 / sizeof(FORMANT_FREQ))) * sizeof(FORMANT_FREQ) / 2;
                FORMANT_FREQ * pFormFreqCurr = (FORMANT_FREQ *)pSpectroFormants->GetProcessedData(dwDataPos, sizeof(FORMANT_FREQ));

                for (int n = 1; n < 5; n++)
                {
                    if (n > nFormants)
                    {
                        swprintf_s(szText,_countof(szText), _T("         "));    // default indicator
                    }
                    else if (pFormFreqCurr->F[n] > 0)
                    {
                        if (nPitchMode == HERTZ)
                        {
                            swprintf_s(szText,_countof(szText), _T("F%d: %.1f Hz"), n, pFormFreqCurr->F[n]);
                        }
                        else
                        {
                            swprintf_s(szText,_countof(szText), _T("F%d: %.1f st"), GetSemitone(pFormFreqCurr->F[n]));
                        }
                    }
                    else
                    {
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
            if (bRes)   // no error reading data?
            {
                // display percentage and raw value CLW 1/5/01
                if (fData >= 99.95)
                {
                    swprintf_s(szText,_countof(szText), _T("L:  %3.0f%%  (%d)"), 100.0, nData);
                }
                else if (fData <= -99.95)
                {
                    swprintf_s(szText,_countof(szText), _T("L:  %3.0f%%  (%d)"), -100.0, nData);
                }
                else
                {
                    swprintf_s(szText,_countof(szText), _T("L:  %3.1f%%  (%d)"), fData, nData);
                }
            }
            else
            {
                swprintf_s(szText,_countof(szText), _T("L:"));
            }
            // write to amplitude pane
            pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE, FALSE); // switch symbol off
            pStat->SetPaneText(ID_STATUSPANE_AMPLITUDE, szText);
            // get the stop cursor value
            bRes = TRUE;
            nData = pDoc->GetWaveData(dwStopCursor, &bRes);
            fData = 100.0 * (double)nData / pow(2.0, 8.0 * (double)nSmpSize - 1);
            if (bRes)   // no error reading data?
            {
                // display percentage and raw value CLW 1/5/01
                if (fData >= 99.95)
                {
                    swprintf_s(szText,_countof(szText), _T("R:  %3.0f%%  (%d)"), 100.0, nData);
                }
                else if (fData <= -99.95)
                {
                    swprintf_s(szText,_countof(szText), _T("R:  %3.0f%%  (%d)"), -100.0, nData);
                }
                else
                {
                    swprintf_s(szText,_countof(szText), _T("R:  %3.1f%%  (%d)"), fData, nData);
                }
            }
            else
            {
                swprintf_s(szText, _countof(szText),_T("R:"));
            }
            // write to pitch pane
            pStat->SetPaneSymbol(ID_STATUSPANE_PITCH, FALSE); // switch symbol off
            pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);
            break;

        case IDD_ZCROSS: // RLJ 09/20/2000.

            bShowPosition = bShowDuration = TRUE;
            pZCross = (CProcessZCross *)pDoc->GetZCross(); // get pointer to zero crossings object
            if (pZCross->IsDataReady())
            {
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
                if (bRes)   // no error reading data?
                {
                    swprintf_s(szText,_countof(szText), _T("L:  %6d"), nData);
                }
                else
                {
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
                if (bRes)   // no error reading data?
                {
                    swprintf_s(szText,_countof(szText), _T("R:  %6d"), nData);
                }
                else
                {
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
            if (pPitch->IsDataReady() && pPitch->GetMaxValue() > 0)
            {
                // calculate pitch data position corresponding to start cursor position
                dwDataPos = (dwStartCursor / nSmpSize) / Grappl_calc_intvl;
                // read the data
                fData = (float)pPitch->GetProcessedData(dwDataPos, &bRes) / (float)PRECISION_MULTIPLIER;
                if (!bRes)
                {
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
            if (pSmPitch->IsDataReady() && pSmPitch->GetMaxValue() > 0)
            {
                // calculate pitch data position corresponding to start cursor position
                dwDataPos = (dwStartCursor / nSmpSize) / Grappl_calc_intvl;
                // read the data
                fData = (float)pSmPitch->GetProcessedData(dwDataPos, &bRes) / (float)PRECISION_MULTIPLIER;
                if (!bRes)
                {
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
            if (pChPitch->IsDataReady() && pChPitch->GetMaxValue() > 0)
            {
                // calculate pitch data position corresponding to start cursor position
                dwDataPos = (dwStartCursor / nSmpSize) / Grappl_calc_intvl;
                // read the data
                fData = (float)pChPitch->GetProcessedData(dwDataPos, &bRes) / (float)PRECISION_MULTIPLIER;
                if (!bRes)
                {
                    fData = -1.;
                }
            }
            fUncertainty = pChPitch->GetUncertainty(fData);
            UpdatePitchStatusBar(fData, fUncertainty);
            break;

        case IDD_GRAPITCH:
            pGrappl = (CProcessGrappl *)pDoc->GetGrappl();      // get pointer to auto pitch object

            if (pGrappl->IsDataReady() && pGrappl->GetMaxValue() > 0)
            {
                bShowPosition = bShowDuration = TRUE;
                // calculate pitch data position corresponding to start cursor position
                dwDataPos = (dwStartCursor / nSmpSize) / Grappl_calc_intvl;
                // read the data
                fData = (float)pGrappl->GetProcessedData(dwDataPos, &bRes) / (float)PRECISION_MULTIPLIER;
                if (!bRes)
                {
                    fData = -1.;
                }
                fUncertainty = pGrappl->GetUncertainty(fData);
                UpdatePitchStatusBar(fData, fUncertainty);
            }
            else
            {
                UpdatePitchStatusBar(-1., 0.);
            }

            break;

        case IDD_MELOGRAM:    //pja 5/7/00
            // moved to separate section CLW 8/24/00
            pMelogram = (CProcessMelogram *)pDoc->GetMelogram();  // get pointer to melogram pitch object

            if (pMelogram->IsDataReady() && pMelogram->GetMaxValue() > 0)
            {
                bShowPosition = bShowDuration = TRUE;
                dwDataPos = (dwStartCursor / nSmpSize) / Grappl_calc_intvl;
                fData = (float)pMelogram->GetProcessedData(dwDataPos, &bRes) / 100.0;
                if (bRes && fData > 0.F)
                {
                    // read the data
                    double fPitchInHz = 220. * pow(2., ((double)fData - 57.) / 12.); // convert from semitones to Hz
                    fUncertainty = pMelogram->GetUncertainty(fPitchInHz);
                    UpdatePitchStatusBarNote(fPitchInHz, fUncertainty, TRUE);
                }
                else
                {
                    UpdatePitchStatusBar(-1., 0.);
                }
            }

            break;

        case IDD_LOUDNESS:
        case IDD_MAGNITUDE:        //pja 5/7/00 - added the IDD_MAGNITUDE to display db in 3rd pane for Melogram Magnitude
            // prepare amplitude data, if ready
            pLoudness = (CProcessLoudness *)pDoc->GetLoudness(); // get pointer to loudness object
            if (pLoudness->IsDataReady() && (pLoudness->GetMaxValue() > 0))
            {
                bShowPosition = bShowDuration = TRUE;
                // calculate size factor between raw data and loudness
                //fSizeFactor = (float)pDoc->GetDataSize() / (float)pLoudness->GetDataSize(m_pPlot);
                fSizeFactor = (double)nSmpSize * ceil((double)(pDoc->GetDataSize()/nSmpSize)/ (double)(pLoudness->GetDataSize()));
                // calculate loudness data position corresponding to start cursor position
                dwDataPos = (DWORD)((float)dwStartCursor / fSizeFactor);
                // read the data
                nData = pLoudness->GetProcessedData(dwDataPos, &bRes);
                if (bRes && nData > 0)
                {
                    swprintf_s(szText,_countof(szText), _T("      %.1f dB"), 20. * log10(double(nData)/32767.) + 6.);
                }
                // write to amplitude pane
                pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE); // switch symbol on
                pStat->SetPaneText(ID_STATUSPANE_AMPLITUDE, szText);

                // put percentage magnitude in pitch pane
                if (bRes && nData > 0)
                {
                    swprintf_s(szText,_countof(szText), _T("      %2d%% fullscale"), (short)(double(nData) / (32767./(2)) * 100.0 + 0.5));
                }
                pStat->SetPaneSymbol(ID_STATUSPANE_PITCH, FALSE); // pitch symbol
                pStat->SetPaneText(ID_STATUSPANE_PITCH, szText);  // pitch indicator
            }

            break;

        case IDD_DURATION:
            pDuration = (CProcessDurations *)pDoc->GetDurations(); // get pointer to loudness object
            if (pDuration->IsDataReady() && (pDuration->GetMaxDuration() > 0))
            {
                bShowPosition = TRUE;
                CSegment * pSegment = pDoc->GetSegment(PHONETIC);
                int nSegmentIndex = pSegment->FindFromPosition(dwStartCursor, TRUE);
                if (nSegmentIndex > 0)
                {
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

        case IDD_CHANGE:
        {
            bShowPosition = bShowDuration = TRUE;
            CProcessChange * pChange = (CProcessChange *)pDoc->GetChange(); // get pointer to change object
            if (pChange->IsDataReady())
            {
                // calculate size factor between waveform data
                //fSizeFactor = (double)pDoc->GetDataSize() / (double)pChange->GetDataSize(); // Minimize round-off error
                fSizeFactor = (double)nSmpSize * ceil((double)(pDoc->GetDataSize()/nSmpSize)/ (double)(pChange->GetDataSize()));

                // calculate pChange data position corresponding to Begin cursor position
                dwDataPos = (DWORD)((double)dwStartCursor / fSizeFactor); // Minimize round-off error
                // read the data
                int nData = pChange->GetProcessedData(dwDataPos, &bRes) / PRECISION_MULTIPLIER;
                if (bRes)   // no error reading data?
                {
                    swprintf_s(szText,_countof(szText), _T("L:  %6d"), nData);
                }
                else
                {
                    swprintf_s(szText,_countof(szText), _T("L:"));
                }
                // write to amplitude pane
                pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE, FALSE); // switch symbol off
                pStat->SetPaneText(ID_STATUSPANE_AMPLITUDE, szText);

                // calculate pChange data position corresponding to End cursor position
                dwDataPos = (DWORD)((double)dwStopCursor / fSizeFactor);  // Minimize round-off error
                // read the data
                nData = pChange->GetProcessedData(dwDataPos, &bRes) / PRECISION_MULTIPLIER;
                if (bRes)   // no error reading data?
                {
                    swprintf_s(szText,_countof(szText), _T("R:  %6d"), nData);
                }
                else
                {
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
            pStat->SetPaneSymbol(ID_STATUSPANE_AMPLITUDE, FALSE); // amplitude symbol
            pStat->SetPaneText(ID_STATUSPANE_AMPLITUDE, _T("        "));  // amplitude indicator
            pStat->SetPaneSymbol(ID_STATUSPANE_PITCH, FALSE); // pitch symbol
            pStat->SetPaneText(ID_STATUSPANE_PITCH, _T("        "));  // pitch indicator
            break;
        }

        if (bShowPosition)
        {
            if (nPositionMode == TIME)
            {
                // calculate position time
                fDataSec = pDoc->GetTimeFromBytes(dwStartCursor);

                nMinutes = (int)fDataSec / 60;
                fDataSec = fDataSec - (nMinutes * 60.);
                if (nMinutes > 9)
                {
                    swprintf_s(szText,_countof(szText), _T("       %02i:%5.3f"), nMinutes, fDataSec);
                }
                else
                {
                    swprintf_s(szText,_countof(szText), _T("       %01i:%6.4f"), nMinutes, fDataSec);
                }
            }
            else
            {
                if (nPositionMode == SAMPLES)
                {
                    swprintf_s(szText, _countof(szText),_T("       %.0f"), (double)(dwStartCursor / nSmpSize));
                }
                else
                {
                    swprintf_s(szText, _countof(szText),_T("       %.0f"), (double)dwStartCursor);
                }
            }
            // write to position pane
            pStat->SetPaneSymbol(ID_STATUSPANE_POSITION); // switch symbol on
            pStat->SetPaneText(ID_STATUSPANE_POSITION, szText);
        }

        if (bShowDuration)
        {
            if (nPositionMode == TIME)
            {
                // calculate length time
                fDataSec = pDoc->GetTimeFromBytes(dwStopCursor - dwStartCursor);
                nMinutes = (int)fDataSec / 60;
                fDataSec = fDataSec - (nMinutes * 60.);
                if (nMinutes > 9)
                {
                    swprintf_s(szText,_countof(szText), _T("       %02i:%5.3f"), nMinutes, fDataSec);
                }
                else if (nMinutes || fDataSec >= .01)
                {
                    swprintf_s(szText,_countof(szText), _T("       %01i:%6.4f"), nMinutes, fDataSec);
                }
                else
                {
                    swprintf_s(szText,_countof(szText), _T("       %01i:%7.5f"), nMinutes, fDataSec);
                }
            }
            else
            {
                if (nPositionMode == SAMPLES)
                {
                    swprintf_s(szText,_countof(szText), _T("       %.0f"), (double)((dwStopCursor - dwStartCursor) / nSmpSize));
                }
                else
                {
                    swprintf_s(szText,_countof(szText), _T("       %.0f"), (double)(dwStopCursor - dwStartCursor));
                }
            }
            // write to length pane
            pStat->SetPaneSymbol(ID_STATUSPANE_LENGTH); // switch symbol on
            pStat->SetPaneText(ID_STATUSPANE_LENGTH, szText);
        }

        pStat->UpdateWindow();
    }
    else
    {
        return;    // this graph has not focus
    }
}

/***************************************************************************/
// CGraphWnd::OnGenderInfoChanged  Gender info has been updated
/***************************************************************************/
LRESULT CGraphWnd::OnGenderInfoChanged(WPARAM nGender, LPARAM)
{
    m_pPlot->GenderInfoChanged(nGender);  // notify plot
    return 0L;
}

/***************************************************************************/
// CGraphWnd::OnCursorClicked  Cursor clicked
/***************************************************************************/
LRESULT CGraphWnd::OnCursorClicked(WPARAM /* Cursor */, LPARAM /* ButtonState */)
{
    //!!reserved for future use
    return 0L;
}


void CGraphWnd::OnDestroy()
{
    CMiniCaptionWnd::OnDestroy();

    // TODO: Add your message handler code here
    // the plot window is a child window and will be automatically destroyed
    // when this window is destroyed, also, the plot wnd handles it's own
    // destruction via PostNcDestroy, therefore, we only need to release
    // our reference to it.
    m_pPlot = NULL;

}

void CGraphWnd::RemoveRtPlots()
{
    if (m_pPlot)
    {
        m_pPlot->RemoveRtPlots();
    }
}

void CGraphWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    GetParent()->SendMessage(WM_CHAR, nChar, MAKELONG(nRepCnt, nFlags));
}

void CGraphWnd::OnNcMouseMove(UINT nHitTest, CPoint point)
{
    m_pPlot->SetMousePointerPosition(CPoint(UNDEFINED_OFFSET, UNDEFINED_OFFSET)); // clear mouse position
    m_pPlot->SetMouseButtonState(0);  // reset state
    const BOOL bForceUpdate = TRUE;
    CSaView * pView = (CSaView *)GetParent();
    UpdateStatusBar(pView->GetStartCursorPosition(), pView->GetStopCursorPosition(), bForceUpdate);
    CWnd::OnNcMouseMove(nHitTest, point);
}


