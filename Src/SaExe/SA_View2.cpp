/////////////////////////////////////////////////////////////////////////////
// sa_view2.cpp:
// Implementation of the CSaView class.
// Author: Urs Ruchtie
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//  09/28/2000 - DDO Moved printing and cut/paste/copy editing code into
//                   here from sa_view.cpp.
//  10-25-2000 - TRE Moved constructors & deconstructor, helper functions,
//                   message map and drawing & printing code here.
//  10/25/2000 - DDO Added some code to redraw the position view plot when
//                   scrolling takes place.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_view.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "graphsTypes.h"
#include "graphsParameters.h"
#include "dsp\dspTypes.h"
#include "dsp\scale.h"
#include "sa_g_raw.h"
#include "sa_g_rec.h"
#include "stpwatch.h"
#include "Process\sa_proc.h"
#include "sa_segm.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_spg.h"
#include "Process\sa_p_sfmt.h"

#include "sa_doc.h"
#include "sa.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "math.h"
#include "printdim.h"
#include "printopt.h"
#include "sa_g_raw.h"
#include "sa_cdib.h"
#include "sa_mplot.h"
#include "pickover.h"
#include "settings\obstream.h"
#include "sa_exprt.h"
#include "sa_g_stf.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//###########################################################################
// CSaView

IMPLEMENT_DYNCREATE(CSaView, CView)

/////////////////////////////////////////////////////////////////////////////
// CSaView message map

BEGIN_MESSAGE_MAP(CSaView, CView)
	//{{AFX_MSG_MAP(CSaView)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdatePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFilenew)
	ON_COMMAND(ID_GRAPHS_TYPES, OnGraphsTypes)
	ON_COMMAND(ID_GRAPHS_PARAMETERS, OnGraphsParameters)
	ON_COMMAND(ID_EDIT_BOUNDARIES, OnEditBoundaries)
	ON_UPDATE_COMMAND_UI(ID_EDIT_BOUNDARIES, OnUpdateEditBoundaries)
	ON_COMMAND(ID_EXPORT_MIDI, OnExportStaff)
	ON_UPDATE_COMMAND_UI(ID_EXPORT_MIDI, OnUpdateExportStaff)
	ON_COMMAND(ID_IMPORT_MIDI, OnImportStaff)
	ON_UPDATE_COMMAND_UI(ID_IMPORT_MIDI, OnUpdateImportStaff)
	ON_COMMAND(ID_PLAYBACK_CURSORS, OnPlaybackCursors)
	ON_COMMAND(ID_PLAYBACK_FILE, OnPlaybackFile)
	ON_COMMAND(ID_PLAYBACK_WINDOW, OnPlaybackWindow)
	ON_COMMAND(ID_PLAYBACK_LTOSTART, OnPlaybackLtostart)
	ON_COMMAND(ID_PLAYBACK_STARTTOR, OnPlaybackStarttor)
	ON_COMMAND(ID_PLAYBACK_LTOSTOP, OnPlaybackLtoStop)
	ON_COMMAND(ID_PLAYBACK_STOPTOR, OnPlaybackStopToR)
	ON_COMMAND(ID_PLAYER_STOP, OnPlayerStop)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_STOP, OnUpdatePlayerStop)
	ON_COMMAND(ID_PLAYER, OnPlayer)
	ON_COMMAND(ID_EXPORT_XML, OnExportXML)
	ON_COMMAND(ID_EXPORT_SFM, OnExportSFM)
	ON_COMMAND(ID_EXPORT_TIME_TABLE, OnExportTimeTable)
	ON_COMMAND(ID_IMPORT_SFM, OnImportSFM)
	ON_COMMAND(ID_IMPORT_TIME_TABLE, OnImportSFT)
	ON_COMMAND(ID_FILE_INFORMATION, OnFileInformation)
	ON_COMMAND(ID_GRAPHS_TILE, OnGraphsRetile)
	ON_COMMAND(ID_NEXT_GRAPH, OnNextGraph)
	ON_COMMAND(ID_LEGEND_ALL, OnLegendAll)
	ON_UPDATE_COMMAND_UI(ID_LEGEND_ALL, OnUpdateLegendAll)
	ON_COMMAND(ID_LEGEND_RAWDATA, OnLegendRawdata)
	ON_UPDATE_COMMAND_UI(ID_LEGEND_RAWDATA, OnUpdateLegendRawdata)
	ON_COMMAND(ID_LEGEND_NONE, OnLegendNone)
	ON_UPDATE_COMMAND_UI(ID_LEGEND_NONE, OnUpdateLegendNone)
	ON_COMMAND(ID_XSCALE_ALL, OnXScaleAll)
	ON_UPDATE_COMMAND_UI(ID_XSCALE_ALL, OnUpdateXScaleAll)
	ON_COMMAND(ID_XSCALE_RAWDATA, OnXScaleRawdata)
	ON_UPDATE_COMMAND_UI(ID_XSCALE_RAWDATA, OnUpdateXScaleRawdata)
	ON_COMMAND(ID_XSCALE_NONE, OnXScaleNone)
	ON_UPDATE_COMMAND_UI(ID_XSCALE_NONE, OnUpdateXScaleNone)
	ON_COMMAND(ID_BOUNDARIES_ALL, OnBoundariesAll)
	ON_UPDATE_COMMAND_UI(ID_BOUNDARIES_ALL, OnUpdateBoundariesAll)
	ON_COMMAND(ID_BOUNDARIES_RAWDATA, OnBoundariesRawdata)
	ON_UPDATE_COMMAND_UI(ID_BOUNDARIES_RAWDATA, OnUpdateBoundariesRawdata)
	ON_COMMAND(ID_BOUNDARIES_NONE, OnBoundariesNone)
	ON_UPDATE_COMMAND_UI(ID_BOUNDARIES_NONE, OnUpdateBoundariesNone)
	ON_COMMAND(ID_GRAPHS_ZOOM_CURSORS, OnGraphsZoomCursors)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_ZOOM_CURSORS, OnUpdateGraphsZoomCursors)
	ON_COMMAND(ID_GRAPHS_ZOOM_ALL, OnGraphsZoomAll)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_ZOOM_ALL, OnUpdateGraphsZoomAll)
	ON_COMMAND(ID_GRAPHS_ZOOM_IN, OnGraphsZoomIn)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_ZOOM_IN, OnUpdateGraphsZoomIn)
	ON_COMMAND(ID_GRAPHS_ZOOM_OUT, OnGraphsZoomOut)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_ZOOM_OUT, OnUpdateGraphsZoomOut)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_COMMAND(ID_GRAPHS_STYLE_LINE, OnGraphsStyleLine)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_STYLE_LINE, OnUpdateGraphsStyleLine)
	ON_COMMAND(ID_GRAPHS_STYLE_SOLID, OnGraphsStyleSolid)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_STYLE_SOLID, OnUpdateGraphsStyleSolid)
	ON_COMMAND(ID_POPUPGRAPH_STYLE_LINE, OnPopupgraphStyleLine)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_STYLE_LINE, OnUpdatePopupgraphStyleLine)
	ON_COMMAND(ID_POPUPGRAPH_STYLE_SOLID, OnPopupgraphStyleSolid)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_STYLE_SOLID, OnUpdatePopupgraphStyleSolid)
	ON_COMMAND(ID_POPUPGRAPH_STYLE_DOTS, OnPopupgraphStyleDots)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_STYLE_DOTS, OnUpdatePopupgraphStyleDots)
	ON_WM_KEYDOWN()
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_TYPES, OnUpdateGraphsTypes)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_PARAMETERS, OnUpdateGraphsParameters)
	ON_COMMAND(ID_POPUPGRAPH_GRIDLINES, OnPopupgraphGridlines)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_GRIDLINES, OnUpdatePopupgraphGridlines)
	ON_COMMAND(ID_POPUPGRAPH_LEGEND, OnPopupgraphLegend)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_LEGEND, OnUpdatePopupgraphLegend)
	ON_COMMAND(ID_POPUPGRAPH_XSCALE, OnPopupgraphXScale)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_XSCALE, OnUpdatePopupgraphXScale)
	ON_COMMAND(ID_DRAWING_BOUNDARIES, OnDrawingBoundaries)
	ON_UPDATE_COMMAND_UI(ID_DRAWING_BOUNDARIES, OnUpdateDrawingBoundaries)
	ON_COMMAND(ID_GRAPHS_MAGNIFY1, OnGraphsMagnify1)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_MAGNIFY1, OnUpdateGraphsMagnify1)
	ON_COMMAND(ID_GRAPHS_MAGNIFY2, OnGraphsMagnify2)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_MAGNIFY2, OnUpdateGraphsMagnify2)
	ON_COMMAND(ID_GRAPHS_MAGNIFY4, OnGraphsMagnify4)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_MAGNIFY4, OnUpdateGraphsMagnify4)
	ON_COMMAND(ID_GRAPHS_MAGNIFYCUSTOM, OnGraphsMagnifycustom)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_MAGNIFYCUSTOM, OnUpdateGraphsMagnifycustom)
	ON_COMMAND(ID_POPUPGRAPH_MAGNIFY1, OnPopupgraphMagnify1)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_MAGNIFY1, OnUpdatePopupgraphMagnify1)
	ON_COMMAND(ID_POPUPGRAPH_MAGNIFY2, OnPopupgraphMagnify2)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_MAGNIFY2, OnUpdatePopupgraphMagnify2)
	ON_COMMAND(ID_POPUPGRAPH_MAGNIFY4, OnPopupgraphMagnify4)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_MAGNIFY4, OnUpdatePopupgraphMagnify4)
	ON_COMMAND(ID_POPUPGRAPH_MAGNIFYCUSTOM, OnPopupgraphMagnifycustom)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_MAGNIFYCUSTOM, OnUpdatePopupgraphMagnifycustom)
	ON_COMMAND(ID_RESTART_PROCESS, OnRestartProcess)
	ON_UPDATE_COMMAND_UI(ID_RESTART_PROCESS, OnUpdateRestartProcess)
	ON_COMMAND(ID_EDIT_SELECTWAVEFORM, OnEditSelectWaveform)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECTWAVEFORM, OnUpdateEditSelectWaveform)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_CURSORS, OnUpdatePlayback)
	ON_COMMAND(ID_SETUP_FNKEYS, OnSetupFnkeys)
	ON_UPDATE_COMMAND_UI(ID_SETUP_FNKEYS, OnUpdateSetupFnkeys)
	ON_COMMAND(ID_EDIT_REMOVE, OnEditRemove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REMOVE, OnUpdateEditRemove)
	ON_COMMAND(ID_EDIT_ADD, OnEditAdd)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD, OnUpdateEditAdd)
	ON_COMMAND(ID_EDIT_ADD_PHRASE_L1, OnEditAddPhraseL1)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_PHRASE_L1, OnUpdateEditAddPhraseL1)
	ON_COMMAND(ID_EDIT_ADD_PHRASE_L2, OnEditAddPhraseL2)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_PHRASE_L2, OnUpdateEditAddPhraseL2)
	ON_COMMAND(ID_EDIT_ADD_PHRASE_L3, OnEditAddPhraseL3)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_PHRASE_L3, OnUpdateEditAddPhraseL3)
	ON_COMMAND(ID_EDIT_ADD_PHRASE_L4, OnEditAddPhraseL4)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_PHRASE_L4, OnUpdateEditAddPhraseL4)
	ON_COMMAND(ID_EDIT_UP, OnEditUp)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UP, OnUpdateEditUp)
	ON_COMMAND(ID_EDIT_DOWN, OnEditDown)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DOWN, OnUpdateEditDown)
	ON_COMMAND(ID_EDIT_PREVIOUS, OnEditPrevious)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PREVIOUS, OnUpdateEditPrevious)
	ON_COMMAND(ID_EDIT_NEXT, OnEditNext)
	ON_UPDATE_COMMAND_UI(ID_EDIT_NEXT, OnUpdateEditNext)
	ON_COMMAND(ID_EDIT_ADD_SYLLABLE, OnEditAddSyllable)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_SYLLABLE, OnUpdateEditAddSyllable)
	ON_COMMAND(ID_EDIT_ADD_WORD, OnEditAddWord)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_WORD, OnUpdateEditAddWord)
	ON_COMMAND(ID_EDIT_ADD_BOOKMARK, OnEditAddBookmark)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_BOOKMARK, OnUpdateEditAddBookmark)
	ON_COMMAND(ID_MOVE_STOP_CURSOR_HERE, OnMoveStopCursorHere)
	ON_COMMAND(ID_EDIT_CURSOR_START_LEFT, OnEditCursorStartLeft)
	ON_COMMAND(ID_EDIT_CURSOR_STOP_LEFT, OnEditCursorStopLeft)
	ON_COMMAND(ID_EDIT_CURSOR_START_RIGHT, OnEditCursorStartRight)
	ON_COMMAND(ID_EDIT_CURSOR_STOP_RIGHT, OnEditCursorStopRight)
	ON_COMMAND(ID_PLAYBACK_SLOW, OnPlaybackSlow)
	ON_COMMAND(ID_DP_GRAPITCH, OnDpGrapitch)
	ON_UPDATE_COMMAND_UI(ID_DP_GRAPITCH, OnUpdateDpGrapitch)
	ON_UPDATE_COMMAND_UI(ID_DP_RAWDATA, OnUpdateDpRawdata)
	ON_COMMAND(ID_DP_RAWDATA, OnDpRawdata)
	ON_COMMAND(ID_DP_SPECTROGRAM, OnDpSpectrogram)
	ON_UPDATE_COMMAND_UI(ID_DP_SPECTROGRAM, OnUpdateDpSpectrogram)
	ON_WM_TIMER()
	ON_COMMAND(ID_EDIT_INPLACE, OnEditInplace)
	ON_WM_CHAR()
	ON_WM_KEYUP()
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_FILE, OnUpdatePlayback)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_WINDOW, OnUpdatePlayback)
	ON_UPDATE_COMMAND_UI(ID_PLAYER, OnUpdatePlayback)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_WM_MDIACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_LTOSTART, OnUpdatePlayback)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_STARTTOR, OnUpdatePlayback)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_LTOSTOP, OnUpdatePlayback)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_STOPTOR, OnUpdatePlayback)
	ON_COMMAND(ID_ADDOVERLAY, OnAddOverlay)
	ON_UPDATE_COMMAND_UI(ID_ADDOVERLAY, OnUpdateAddOverlay)
	ON_COMMAND(ID_REMOVE_OVERLAY, OnRemoveOverlay)
	ON_COMMAND(ID_REMOVE_OVERLAYS, OnRemoveOverlays)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_OVERLAY, OnUpdateRemoveOverlay)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_OVERLAYS, OnUpdateRemoveOverlay)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_SLOW, OnUpdatePlayback)
	ON_COMMAND(ID_SPECTRO_FORMANTS, OnSpectroFormants)
	ON_UPDATE_COMMAND_UI(ID_SPECTRO_FORMANTS, OnUpdateSpectroFormants)
	//}}AFX_MSG_MAP
	// Mapped messages n/a from class wizard.
	ON_COMMAND_RANGE(ID_LAYOUT_FIRST, ID_LAYOUT_LAST, OnLayout)
	ON_UPDATE_COMMAND_UI(ID_LAYOUT_LAST, OnUpdateLayout)
	ON_COMMAND_RANGE(ID_PLAY_F1, ID_PLAY_F24, OnPlayFKey)
	ON_UPDATE_COMMAND_UI(ID_PLAY_F24, OnUpdatePlayback)
	ON_COMMAND_RANGE(ID_GRAPHTYPES_SELECT_FIRST, ID_GRAPHTYPES_SELECT_LAST, OnGraphTypesSelect)
	ON_UPDATE_COMMAND_UI(ID_GRAPHTYPES_SELECT_LAST, OnUpdateGraphTypesSelect)
	ON_COMMAND_RANGE(ID_PHONETIC_ALL, ID_PHRASE_L4_ALL, OnAnnotationAll)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PHONETIC_ALL, ID_PHRASE_L4_ALL, OnUpdateAnnotationAll)
	ON_COMMAND_RANGE(ID_PHONETIC_RAWDATA, ID_PHRASE_L4_RAWDATA, OnAnnotationRawdata)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PHONETIC_RAWDATA, ID_PHRASE_L4_RAWDATA, OnUpdateAnnotationRawdata)
	ON_COMMAND_RANGE(ID_PHONETIC_NONE, ID_PHRASE_L4_NONE, OnAnnotationNone)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PHONETIC_NONE, ID_PHRASE_L4_NONE, OnUpdateAnnotationNone)
	ON_COMMAND_RANGE(ID_POPUPGRAPH_PHONETIC, ID_POPUPGRAPH_MUSIC_PL4, OnPopupgraphAnnotation)
	ON_UPDATE_COMMAND_UI_RANGE(ID_POPUPGRAPH_PHONETIC, ID_POPUPGRAPH_MUSIC_PL4, OnUpdatePopupgraphAnnotation)
	// Change Graph Commands 
	//  These could be consolidated to a single message pair if they were contiguous range
	//  Unfortunately the ID's are used in the PSA file so we can not easily change the IDs
	ON_COMMAND_RANGE(IDD_RAWDATA, IDD_RAWDATA, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_RAWDATA, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_GLOTWAVE, IDD_GLOTWAVE, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_GLOTWAVE, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_3D_PITCH, IDD_3D_PITCH, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_3D_PITCH, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_LOUDNESS, IDD_LOUDNESS, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_LOUDNESS, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_DURATION, IDD_DURATION, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_DURATION, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_CHANGE, IDD_CHANGE, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_CHANGE, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_SPECTROGRAM, IDD_SPECTROGRAM, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SPECTROGRAM, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_SNAPSHOT, IDD_SNAPSHOT, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SNAPSHOT, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_SPECTRUM, IDD_SPECTRUM, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SPECTRUM, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_PITCH, IDD_PITCH, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_PITCH, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_CHPITCH, IDD_CHPITCH, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_CHPITCH, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_SMPITCH, IDD_SMPITCH, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SMPITCH, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_GRAPITCH, IDD_GRAPITCH, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_GRAPITCH, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_RECORDING, IDD_RECORDING, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_RECORDING, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_MELOGRAM, IDD_MELOGRAM, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_MELOGRAM, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_TWC, IDD_TWC, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_TWC, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_STAFF, IDD_STAFF, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_STAFF, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_MAGNITUDE, IDD_MAGNITUDE, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_MAGNITUDE, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_CEPPITCH, IDD_CEPPITCH, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_CEPPITCH, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_ZCROSS, IDD_ZCROSS, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_ZCROSS, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_POA, IDD_POA, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_POA, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_WAVELET, IDD_WAVELET, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_WAVELET, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_3D, IDD_3D, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_3D, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_F1F2, IDD_F1F2, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_F1F2, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_F2F1, IDD_F2F1, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_F2F1, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_F2F1F1, IDD_F2F1F1, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_F2F1F1, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_RATIO, IDD_RATIO, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_RATIO, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_SDP_A, IDD_SDP_A, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SDP_A, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_SDP_B, IDD_SDP_B, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SDP_B, OnUpdateChangeGraph)
	ON_COMMAND_RANGE(IDD_INVSDP, IDD_INVSDP, OnChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_INVSDP, OnUpdateChangeGraph)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PAGE_SETUP, OnPrintPageSetup)
	// Custom messages
	ON_MESSAGE(WM_USER_GRAPH_DESTROYED, OnGraphDestroyed)
	ON_MESSAGE(WM_USER_GRAPH_STYLECHANGED, OnGraphStyleChanged)
	ON_MESSAGE(WM_USER_GRAPH_ORDERCHANGED, OnGraphOrderChanged)
	ON_MESSAGE(WM_USER_GRAPH_GRIDCHANGED, OnGraphGridChanged)
	ON_MESSAGE(WM_USER_GRAPH_COLORCHANGED, OnGraphColorChanged)
	ON_MESSAGE(WM_USER_GRAPH_FONTCHANGED, OnGraphFontChanged)
	ON_MESSAGE(WM_USER_VIEW_SCROLLZOOMCHANGED, OnScrollZoomChanged)
	ON_MESSAGE(WM_USER_CURSOR_IN_FRAGMENT, OnCursorInFragment)
	ON_MESSAGE(WM_USER_VIEW_CURSORALIGNCHANGED, OnCursorAlignmentChanged)
	ON_MESSAGE(WM_USER_VIEW_GRAPHUPDATECHANGED, OnGraphUpdateModeChanged)
	ON_MESSAGE(WM_USER_VIEW_ANIMATIONCHANGED, OnAnimationChanged)
	ON_MESSAGE(WM_USER_RECORDER, OnRecorder)
	ON_MESSAGE(WM_USER_APP_MESSAGE, OnAppMessage)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	//ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY_MEASUREMENTS, OnEditCopyMeasurements)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_PASTENEW, OnEditPasteNew)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTENEW, OnUpdateEditPasteNew)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateHasSel)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_COPY_PHONETIC_TO_PHONEMIC, OnEditCopyPhoneticToPhonemic)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY_PHONETIC_TO_PHONEMIC, OnUpdateEditCopyPhoneticToPhonemic)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaView message handlers, Part II

/***************************************************************************/
// CSaView::OnUpdateFilenew Menu Update
/***************************************************************************/
void CSaView::OnUpdateFilenew(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

/***************************************************************************/
// CSaView::OnCreate View creation
/***************************************************************************/
int CSaView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, WS_CLIPCHILDREN);

	pViewMainFrame->ComputeNumberOfViews(1); // add one view
	m_bViewCreated = TRUE;
	return 0;
}

/***************************************************************************/
// CSaView::IDDSelected - TCJRLJ
//  Return TRUE if the check box associated is selected; else return FALSE
/***************************************************************************/
BOOL CSaView::IDDSelected(const UINT* pGraphIDs, UINT nIDD)
{
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if ((*(pGraphIDs + nLoop)) == nIDD)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/***************************************************************************/
// CSaView::GetGraphIndexForIDD - TCJ 3/27/00
//  Return the index of the graph that has the given IDD.
/***************************************************************************/
int CSaView::GetGraphIndexForIDD(UINT nIDD)
{
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
		if (m_anGraphID[nLoop] == nIDD)
			return nLoop;

	return -1;
}

/***************************************************************************/
// CSaView::CreateOpenAsGraphs Create graphs for "File-->Open As"
// Create graphs for "File-->Open As" (Phonetic/Music Analysis)
/***************************************************************************/
void CSaView::CreateOpenAsGraphs(UINT OpenAsID)
{
	DeleteGraphs(); // RLJ 07/28/2000
	switch (OpenAsID)
	{
	case ID_FILE_OPENAS_MUSICANALYSIS:
		// Open *.Wav & display Raw Waveform, Melogram, and Tonal Weighting Chart -- OpenMA
		m_nLayout = ID_LAYOUT_2A; // Use default layout.  // RLJ 07/28/2000
		CreateGraph(0, IDD_RECORDING,  CREATE_FROMSCRATCH);
		CreateGraph(1, IDD_MELOGRAM,  CREATE_FROMSCRATCH);
		CreateGraph(2, IDD_TWC,       CREATE_FROMSCRATCH);
		CreateGraph(3, IDD_MAGNITUDE, CREATE_FROMSCRATCH);
		CreateGraph(4, IDD_STAFF,     CREATE_FROMSCRATCH); // DDO - 08/09/2000
		break;

	case ID_FILE_OPENAS_NEW:
		// Display Blank Graph -- OpenNew
		m_nLayout = ID_LAYOUT_1; // Use default layout.
		CreateGraph(0, IDD_RAWDATA, CREATE_FROMSCRATCH);
		break;

	case ID_FILE_OPENAS_PHONETICANALYSIS:
	default:
		// Open *.wav & display Raw Waveform -- OpenPA
		m_nLayout = ID_LAYOUT_2A; // Use default layout.
		CreateGraph(0, IDD_RAWDATA, CREATE_FROMSCRATCH);
		CreateGraph(1, IDD_GRAPITCH, CREATE_FROMSCRATCH);
		break;
	}

	pSaApp->SetOpenAsID(ID_FILE_OPEN); // Restore default file open mode.
	// retile the graphs
	OnGraphsRetile(); // retile graphs
}

/***************************************************************************/
// CSaView::OnGraphsTypes Sets the displayed graph types
// The function keeps the already opened graphs and their settings and adds
// the new chosen graphs and deletes the no longer chosen graphs. The order
// will be after the new selection. The graphs will be retiled.
/***************************************************************************/
void CSaView::OnGraphsTypes()
{
	CDlgGraphsTypes dlgGraphsTypes(this, m_anGraphID, m_nLayout);

	//**************************************************************
	// If the user chose OK then store the chosen IDs in a temp.
	// array and process the request.
	//**************************************************************
	if (dlgGraphsTypes.DoModal() == IDOK)
	{
		UINT anNewGraphID[MAX_GRAPHS_NUMBER];
		int nLayout = -1;
		dlgGraphsTypes.GetCheckedGraphs(&anNewGraphID[0], &nLayout);
		OnGraphsTypesPostProcess(&anNewGraphID[0], nLayout);
	}
}

/***************************************************************************/
// CSaView::OnGraphTypesSelect Change the graphs
/***************************************************************************/
void CSaView::OnGraphTypesSelect(UINT nID)
{
	int nConfiguration = nID - ID_GRAPHTYPES_SELECT_FIRST;

	UINT anNewGraphID[MAX_GRAPHS_NUMBER];
	int nLayout = -1;
	CDlgGraphsTypes::GetStandardCheckedGraphs(nConfiguration, &anNewGraphID[0], &nLayout);
	OnGraphsTypesPostProcess(&anNewGraphID[0], nLayout);
}

/***************************************************************************/
// CSaView::OnUpdateGraphTypesSelect Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphTypesSelect(CCmdUI* pCmdUI)
{
	if(pCmdUI->m_nID >= ID_GRAPHTYPES_SELECT_FIRST && pCmdUI->m_nID <= ID_GRAPHTYPES_SELECT_LAST)
		pCmdUI->Enable();
	else
		pCmdUI->Enable(FALSE);
}

/***************************************************************************/
// CSaView::OnGraphsTypesPostProcess
/***************************************************************************/
void CSaView::OnGraphsTypesPostProcess(const UINT* anNewGraphID, int nLayout)
{
	UINT       anTmpGraphID[MAX_GRAPHS_NUMBER];
	CGraphWnd* apTmpGraphs[MAX_GRAPHS_NUMBER];

	//**************************************************************
	// Initialize the arrays that will be a temporary holding place
	// for the chosen graph IDs and graph pointers.
	//**************************************************************
	memset(&apTmpGraphs[0],  0, sizeof(CGraphWnd*) * MAX_GRAPHS_NUMBER);
	memset(&anTmpGraphID[0], 0, sizeof(UINT) * MAX_GRAPHS_NUMBER);

	int index = 0;

	//**************************************************************
	// Make sure the position data comes first in the array so it will
	// display first.
	//**************************************************************
	if (IDDSelected(&anNewGraphID[0], IDD_RECORDING))
		anTmpGraphID[index++] = IDD_RECORDING;

	//**************************************************************
	// Finally, all other chosen graphs that aren't one of the
	// graph IDs already copied to the temp. ID array should be
	// copied to the temp. array.
	//**************************************************************
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++)
	{
		if (anNewGraphID[i] && anNewGraphID[i] != IDD_RECORDING)
			anTmpGraphID[index++] = anNewGraphID[i];
	}

	//**************************************************************
	// Loop through all the graphs and delete all graphs that we 
	// will not need.
	// NOTE: Do this first because the destructors want the graph 
	// list intact to be able to inter communicate.
	//**************************************************************
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++)
	{
		if (m_apGraphs[i] && !IDDSelected(&anTmpGraphID[0], m_anGraphID[i]))
		{
			DeleteGraphs(i);
			m_apGraphs[i] = NULL;
			m_anGraphID[i] = 0;
		}
	}

	//**************************************************************
	// Run through new graph IDs and determine if they exist in the
	// new set of graph IDs. If they do, copy the graph pointer to
	// the temporary array of graph pointers and clear out the
	// elements in the permanent array of graph pointers and IDs
	// so those graphs won't get deleted later in this function.
	//**************************************************************
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++)
	{
		if (anTmpGraphID[i] && IDDSelected(&m_anGraphID[0], anTmpGraphID[i]))
		{
			index = GetGraphIndexForIDD(anTmpGraphID[i]);
			if (index >= 0)
			{
				apTmpGraphs[i] = m_apGraphs[index];
				m_apGraphs[index] = NULL;
				m_anGraphID[index] = 0;
			}
		}
	}

	if(nLayout == -1)
		m_nLayout = SetLayout(&anTmpGraphID[0]);
	else
		m_nLayout = nLayout;

	//**************************************************************
	// For those graph IDs in the temp. graph ID array that don't
	// have corresponding graph pointers, create graphs.
	//**************************************************************
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++)
	{
		ASSERT (m_apGraphs[i] == NULL);
		m_anGraphID[i] = anTmpGraphID[i];
		if (anTmpGraphID[i] && !apTmpGraphs[i])
			CreateGraph(i, m_anGraphID[i]);
		else
			m_apGraphs[i] = apTmpGraphs[i];
	}

	OnGraphsRetile();
}

/***************************************************************************/
// CSaView::OnUpdateGraphsTypes Menu Update
/***************************************************************************/
void CSaView::OnUpdateGraphsTypes(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetDataSize() != 0); // enable if data is available
}

/***************************************************************************/
// CSaView::OnGraphsParameters Changing the graph parameters
/***************************************************************************/
void CSaView::OnGraphsParameters()
{
	CSaString szCaption;
	szCaption.LoadString(IDS_DLGTITLE_GRAPHSPARA); // load caption string
	// set the pitch parameters to enable manual analysing
	CSaDoc* pDoc = (CSaDoc*)GetDocument();
	// create the property sheet according to the existing graphs
	CDlgGraphsParameters* dlgGraphsParameters; // graph parameters dialog

	dlgGraphsParameters = new CDlgGraphsParameters(szCaption, NULL);

	// set workbench process
	BOOL bProcessChange = FALSE;
	dlgGraphsParameters->m_dlgRawdataPage.m_nProcess = pDoc->GetWbProcess();
	if ((dlgGraphsParameters->DoModal() == IDOK) &&
		(pDoc->GetWbProcess() != dlgGraphsParameters->m_dlgRawdataPage.m_nProcess))
	{
		pDoc->SetWbProcess(dlgGraphsParameters->m_dlgRawdataPage.m_nProcess);
		bProcessChange = TRUE;
	}
	delete dlgGraphsParameters;

	// process workbench if necessary
	if ((bProcessChange) && (pDoc->WorkbenchProcess(TRUE, TRUE)))
	{
		RefreshGraphs(TRUE, TRUE);
	}
}

/***************************************************************************/
// CSaView::OnUpdateGraphsParameters Menu Update
/***************************************************************************/
void CSaView::OnUpdateGraphsParameters(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetDataSize() != 0); // enable if data is available
}

/***************************************************************************/
// CSaView::OnEditBoundaries
/***************************************************************************/
void CSaView::OnEditBoundaries()
{
	m_bEditBoundaries = !m_bEditBoundaries;
}


/***************************************************************************/
// CSaView::OnUpdateEditGraphs
/***************************************************************************/
void CSaView::OnUpdateEditBoundaries(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(GetEditBoundaries(0,FALSE)!=0);
}

/***************************************************************************/
// CSaView::GetEditBoundaries
/***************************************************************************/
int CSaView::GetEditBoundaries(int nFlags, BOOL checkKeys)
{
	if((nFlags&MK_CONTROL)&&(nFlags&MK_SHIFT)&&checkKeys)
	{
		return BOUNDARIES_EDIT_OVERLAP;
	}
	if(m_bEditBoundaries || ((GetAsyncKeyState(VK_MENU) < 0)&&checkKeys))
		return BOUNDARIES_EDIT_NO_OVERLAP;
	else
		return BOUNDARIES_EDIT_NULL;
}

/***************************************************************************/
// CSaView::OnExportStaff
/***************************************************************************/
void CSaView::OnExportStaff()
{
	int nGraphIndex = GetGraphIndexForIDD(IDD_STAFF);
	CPlotStaff *pMusicStaff;
	if (nGraphIndex >= 0)
	{
		CGraphWnd *pMusicGraph = GetGraph(nGraphIndex);
		pMusicStaff = (CPlotStaff *)pMusicGraph->GetPlot();
		pMusicStaff->ExportFile();
	}
}


/***************************************************************************/
// CSaView::OnUpdateExportStaff
/***************************************************************************/
void CSaView::OnUpdateExportStaff(CCmdUI* pCmdUI)
{
	int nGraphIndex = GetGraphIndexForIDD(IDD_STAFF);
	if (nGraphIndex >= 0)
	{
		pCmdUI->Enable(TRUE);
	}
	else
		pCmdUI->Enable(FALSE);
}


/***************************************************************************/
// CSaView::OnImportStaff
/***************************************************************************/
void CSaView::OnImportStaff()
{
	int nGraphIndex = GetGraphIndexForIDD(IDD_STAFF);
	CPlotStaff *pMusicStaff;
	if (nGraphIndex >= 0)
	{
		CGraphWnd *pMusicGraph = GetGraph(nGraphIndex);
		pMusicStaff = (CPlotStaff *)pMusicGraph->GetPlot();
		pMusicStaff->ImportFile();
	}
}


/***************************************************************************/
// CSaView::OnUpdateImportStaff
/***************************************************************************/
void CSaView::OnUpdateImportStaff(CCmdUI* pCmdUI)
{
	int nGraphIndex = GetGraphIndexForIDD(IDD_STAFF);
	if (nGraphIndex >= 0)
	{
		pCmdUI->Enable(TRUE);
	}
	else
		pCmdUI->Enable(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CSaView diagnostics

#ifdef _DEBUG
void CSaView::AssertValid() const
{
	CView::AssertValid();
}

void CSaView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}


CSaDoc* CSaView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSaDoc)));
	return (CSaDoc*)m_pDocument;
}

#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CSaView construction/destruction/creation

/***************************************************************************/
// CSaView::CSaView Constructor
/***************************************************************************/
CSaView::CSaView(const CSaView *pToBeCopied)
{
	// RLJ 06/01/2000
	pSaApp = (CSaApp *)AfxGetApp();
	pViewMainFrame = (CMainFrame*)AfxGetMainWnd();

	m_pStopwatch = NULL;
	m_restartPageOptions = FALSE;
	m_pPageLayout = new CPrintOptionsDlg;

	//********************************************************
	// 09/25/2000 - DDO
	//********************************************************
	m_bStaticTWC = TRUE;
	m_bNormalMelogram = TRUE;

	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		m_apGraphs[nLoop] = NULL;
		m_anGraphID[nLoop] = 0;
	}

	m_anGraphID[0] = IDD_RAWDATA; // default graph
	m_pFocusedGraph = NULL; // no graph focused
	m_nFocusedID = 0;
	m_nLayout = ID_LAYOUT_1; // default layout
	m_bLegendAll = TRUE;
	m_bLegendNone = FALSE;
	m_bXScaleAll = TRUE;
	m_bXScaleNone = FALSE;

	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
	{
		if (nLoop == PHONETIC)
		{
			m_abAnnAll[nLoop] = FALSE;
			m_abAnnNone[nLoop] = FALSE;
		}
		else
		{
			m_abAnnAll[nLoop] = FALSE;
			m_abAnnNone[nLoop] = TRUE;
		}
	}
	m_nCursorAlignment = MainFrame()->GetCursorAlignment();
	m_bBoundariesAll = FALSE;
	m_bBoundariesNone = TRUE;
	m_bUpdateBoundaries = TRUE;
	m_bDrawStyleLine = TRUE;
	m_dwDataPosition = 0; // start with first sample data
	m_fMagnify = 1.0; // no magnify
	m_fZoom = 1.0; // no zoom
	m_fMaxZoom = 0;
	m_bAnimating = FALSE;
	m_bPrintPreviewInProgress = FALSE;
	m_pCDibForPrint = NULL;
	m_pPgLayoutBackup = NULL;
	m_pPickOverlay = new CPickOverlayDlg;
	m_eInitialShowCmd = SW_SHOWNORMAL;
	m_z = 0;
	m_WeJustReadTheProperties = FALSE;
	m_bViewCreated = FALSE;
	VERIFY(CStopwatch::CreateObject(&m_pStopwatch));

	m_bEditBoundaries = FALSE;
	//kg 11/24/00 causes exceptions without this
	m_dwStopCursor = 0;
	m_dwStartCursor = 0;
	m_dwHScrollFactor = 0;
	m_fVScrollSteps = 0;
	m_dwScrollLine = 0;
	m_dPlaybackPosition = 0;
	m_dwPlaybackTime = 0;
	m_dPlaybackPositionLimit = 0;
	m_nPlaybackSpeed = 0;
	m_bViewIsActive = FALSE;
	m_printArea.x = 0;
	m_printArea.y = 0;
	m_newPrinterDPI = 0;
	m_printScaleX = 0;
	m_printScaleY = 0;

	if(pToBeCopied)
	{
		*this = *pToBeCopied;
	}
}

CSaView & CSaView::operator=(const CSaView& fromThis)
{
	if (&fromThis != this)
	{
		Clear();
		Copy(fromThis);
	}

	return *this;
}

void CSaView::Clear(void)
{
	DeleteGraphs(); // delete existing graph objects
	if (m_pPageLayout)
	{
		delete m_pPageLayout;
		m_pPageLayout = NULL;
	}
	if (m_pPgLayoutBackup)
	{
		delete m_pPgLayoutBackup;
		m_pPgLayoutBackup = NULL;
	}
	if (m_pPickOverlay)
	{
		delete m_pPickOverlay;
		m_pPickOverlay = NULL;
	}
}

void  CSaView::PartialCopy(const CSaView & fromThis)
{
	m_nFocusedID = 0;
	m_nLayout = fromThis.m_nLayout; // default layout
	m_bLegendAll = fromThis.m_bLegendAll;
	m_bLegendNone = fromThis.m_bLegendNone;
	m_bXScaleAll = fromThis.m_bXScaleAll;
	m_bXScaleNone = fromThis.m_bXScaleNone;
	m_bBoundariesAll = fromThis.m_bBoundariesAll ;
	m_bBoundariesNone = fromThis.m_bBoundariesNone;
	m_bUpdateBoundaries = fromThis.m_bUpdateBoundaries;
	m_bDrawStyleLine = fromThis.m_bDrawStyleLine;
	m_dwDataPosition = 0; // start with first sample data
	m_fMagnify = 1.0; // no magnify
	m_fZoom = 1.0; // no zoom
	m_fMaxZoom = 0;
	m_bPrintPreviewInProgress = FALSE;

	//***********************************************************
	// Added following two lines: DDO - 09/25/00
	//***********************************************************
	m_bStaticTWC = fromThis.m_bStaticTWC;
	m_bNormalMelogram = fromThis.m_bNormalMelogram;

	//***********************************************************
	// Added following two lines: DDO - 08/07/00
	//***********************************************************
	pSaApp = (CSaApp *)AfxGetApp();
	pViewMainFrame = (CMainFrame*)AfxGetMainWnd();

	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
	{
		m_abAnnAll[nLoop] = fromThis.m_abAnnAll[nLoop];
		m_abAnnNone[nLoop] = fromThis.m_abAnnNone[nLoop];

	}
	m_eInitialShowCmd = fromThis.m_eInitialShowCmd;
}

/***************************************************************************/
/***************************************************************************/
void  CSaView::Copy(const CSaView & fromThis)
{
	PartialCopy(fromThis);
	m_restartPageOptions = fromThis.m_restartPageOptions;
	m_pPageLayout = new CPrintOptionsDlg(fromThis.m_pPageLayout);
	m_pFocusedGraph = NULL; // no graph focused
	m_pCDibForPrint = NULL;
	m_pPgLayoutBackup = NULL;
	m_pPickOverlay = new CPickOverlayDlg;
	m_z = 0;
	m_WeJustReadTheProperties = FALSE;
	m_pDocument = NULL;

	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if(fromThis.m_apGraphs[nLoop])
		{
			m_apGraphs[nLoop] = new CGraphWnd(*(fromThis.m_apGraphs[nLoop]));
			m_anGraphID[nLoop] = m_apGraphs[nLoop]->GetPlotID();
		}
		else
		{
			m_apGraphs[nLoop] = NULL;
			m_anGraphID[nLoop] = 0;
		}
	}

	m_bViewCreated = FALSE;
}


/***************************************************************************/
// CSaView::~CSaView Destructor
/***************************************************************************/
CSaView::~CSaView()
{
	Clear();
	if (m_pStopwatch) delete m_pStopwatch;
}

/***************************************************************************/
// CSaView::PreCreateWindow Creation
// Called from the framework before the creation of the window. Changes the
// window style as desired.
/***************************************************************************/
BOOL CSaView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CView::PreCreateWindow(cs))
		return FALSE;
	cs.style |= WS_HSCROLL | WS_VSCROLL;  // enable scrolling
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSaView helper functions

/***************************************************************************/
// CSaView::DeleteGraphs Delete existing graph objects
// If parameter nPosition is -1, then all existing graph objects will be
// deleted, otherwise only the one on the given position in the graph object
// array will be deleted. The focused graph is reset. The function returns
// the rectangle of the deleted graph window in the case of one deleted
// graph, otherwise a 0 rectangle, used to create new graphs.
/***************************************************************************/
WINDOWPLACEMENT CSaView::DeleteGraphs(int nPosition, BOOL bClearID)
{
	WINDOWPLACEMENT WP;
	WP.length = sizeof(WINDOWPLACEMENT);
	WP.showCmd = SW_HIDE; // use default size if not changed

	if (nPosition == -1)
	{
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
		{
			if (DestroyGraph(&(m_apGraphs[nLoop]), FALSE))
				if (bClearID) 
					m_anGraphID[nLoop] = 0;
		}
		ResetFocusedGraph(); // no graph focused
	}
	else
	{
		// delete only given graph object
		if (m_apGraphs[nPosition])
		{
			m_apGraphs[nPosition]->GetWindowPlacement(&WP);                  // get window placement of the graph
			DestroyGraph(&(m_apGraphs[nPosition]));
			if (bClearID) m_anGraphID[nPosition] = 0;                        // reset ID
		}
	}

	return WP; // return window coordinates
}

/***************************************************************************/
// CSaView::GetNumberOfGraphs Get number of graphs out of the layout number
//
// 09/20/2000 - DDO Rewrote this so the number of graphs returned is based
//                  on the array of graph pointers and not the layout.
//                  That's because, for the purposes of the layout,
//                  several music graphs can be considered one graph.
/***************************************************************************/
int CSaView::GetNumberOfGraphs(UINT *pGraphIDs)
{
	int nNumOfGraphs = 0;
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++)
		if (pGraphIDs[i]) nNumOfGraphs++;

	return nNumOfGraphs;
}

/***************************************************************************/
// 09/20/2000 - DDO This function returns the number of graphs in a given
//                  layout.
/***************************************************************************/
int CSaView::GetNumberOfGraphsInLayout(UINT nLayoutID)
{
	switch(nLayoutID)
	{
	case ID_LAYOUT_1:  return 1;
	case ID_LAYOUT_2A:
	case ID_LAYOUT_2B:
	case ID_LAYOUT_2C: return 2;
	case ID_LAYOUT_3A:
	case ID_LAYOUT_3B:
	case ID_LAYOUT_3C: return 3;
	case ID_LAYOUT_4A:
	case ID_LAYOUT_4B:
	case ID_LAYOUT_4C: return 4;
	case ID_LAYOUT_5:  return 5;
	case ID_LAYOUT_6A:
	case ID_LAYOUT_6B: return 6;
	default: break;
	}

	return -1;
}

/***************************************************************************/
// CSaView::SetLayout Set layout number from the number of graph ID entries
//   09/20/2000 - DDO Tweaked code mainly so the recording graph isn't
//                    considered as part of the layout.
/***************************************************************************/
UINT CSaView::SetLayout(UINT* pGraphIDs)
{
	//******************************************************
	// Get the graph count.
	//******************************************************
	int nNumOfGraphs = GetNumberOfGraphs(pGraphIDs);

	//******************************************************
	// Since the TWC and Magnitude graphs take up space in
	// the layout where the melogram graph goes, don't count
	// those for the purpose of determining a layout. Also,
	// since the recording graph isn't part of the layout,
	// don't count it.
	//******************************************************
	if (IDDSelected(pGraphIDs, IDD_TWC))       nNumOfGraphs--;
	if (IDDSelected(pGraphIDs, IDD_MAGNITUDE)) nNumOfGraphs--;
	if (IDDSelected(pGraphIDs, IDD_RECORDING)) nNumOfGraphs--;

	return CLayoutListBox::GetPreferredLayout(nNumOfGraphs);
}

/***************************************************************************/
// CSaView::GetGraphTitle
//
// 09/23/2000 - DDO
/***************************************************************************/
CSaString CSaView::GetGraphTitle(UINT nID)
{
	CSaString szTitle;

	szTitle.LoadString(IDS_UNDEFINED);

	//**************************************************
	// Get the graph title from the floating popup menu
	//**************************************************
	if (nID != IDD_BLANK)
	{
		CMenu mPopup;
		if (mPopup.LoadMenu(IDR_SA_FLOATINGPOPUP))
		{
			CMenu& pmFloatingPopup = *mPopup.GetSubMenu(0);
			pmFloatingPopup.GetMenuString(nID, szTitle.GetBuffer(32), 32, MF_BYCOMMAND);
			szTitle.ReleaseBuffer(-1);
			mPopup.DestroyMenu();
		}
	}
	//**************************************************
	// Remove ampersand
	//**************************************************
	int nTemp = szTitle.Find('&');
	if (nTemp != -1) szTitle = szTitle.Left(nTemp) + szTitle.Right(szTitle.GetLength() - nTemp - 1);

	return szTitle;
}

/***************************************************************************/
// 09/28/2000 - DDO This function will perform misc. stuff before an
//                  overlay graph is created. I wrote this to get it out
//                  of the CreateGraphs functions which was too bloated.
/***************************************************************************/
UINT CSaView::PreCreateOverlayGraph(int nIndex)
{
	m_anGraphID[nIndex] = ID_GRAPHS_OVERLAY;       // set graph ID of new graph to create

	int pos = 0;
	CGraphWnd * pGrph = m_pPickOverlay->GetNextSelectedGraph(pos);
	if (!pGrph) m_anGraphID[nIndex] = IDD_BLANK;

	while (pGrph)
	{
		m_apGraphs[nIndex]->MergeInGraph(pGrph, (CSaView *)pGrph->GetParent(),false);
		pGrph = m_pPickOverlay->GetNextSelectedGraph(pos);
	}

	return m_anGraphID[nIndex];
}

/***************************************************************************/
// 09/28/2000 - DDO This function will create a graph object's window and
//                  finish up the graph creation process for a single graph.
//                  I wrote this to get it out of the CreateGraphs functions
//                  which was too bloated.
/***************************************************************************/
void CSaView::CreateOneGraph(UINT *pID, CGraphWnd **pGraph)
{
	CRect rWnd(0,0,0,0);
	CSaString szTitle = GetGraphTitle(*pID);

	if (!(*pGraph)->Create(NULL, szTitle, WS_CHILD | WS_VISIBLE | WS_BORDER, rWnd, this, 0))
	{
		delete *pGraph;
		*pID = 0;
		*pGraph = NULL;
	}

	if (!m_pFocusedGraph) SetFocusedGraph(*pGraph); // set focus to first graph
	//!! AKE 10/21/2000  The following line of code has been removed.  At this point,
	//                   the status bar cannot be updated until the graph is drawn because it displays data
	//                   calculated at the position of the start or private cursor, which is not known until
	//                   the plot code calls the process method for the data.  The function is instead called
	//                   in CPlotWnd::PlotPaintFinish.
	//  m_pFocusedGraph->UpdateStatusBar(m_dwStartCursor, m_dwStopCursor, TRUE);

}

/***************************************************************************/
// CSaView::CreateGraphs Create Graphs
// If the parameter nPosition is -1, then all graphs with the in the ID array
// given IDs will be created. Otherwise only the one on the given position
// in the graph object array with the given ID. Before creating graph objects,
// the already existing object(s) will be deleted. Then the new graph
// object(s) are created. Graph initialization and creation follow directly.
/***************************************************************************/
void CSaView::CreateGraph(int nPosition, int nNewID,
						  CREATE_HOW how         /* = CREATE_STANDARD */,
						  Object_istream * pObs  /* = NULL */,
						  CGraphWnd * pFromGraph /* = NULL */)
{
	ASSERT (nPosition != -1);

	int nLoop = nPosition;
	m_anGraphID[nLoop] = nNewID;

	if(nNewID == IDD_RECORDING)
	{
		m_apGraphs[nLoop] = CreateRecGraph(reinterpret_cast<CRecGraphWnd*>(pFromGraph), pObs);
		return;
	}

	if (nNewID == ID_GRAPHS_OVERLAY)
		nNewID = PreCreateOverlayGraph(nPosition);

	CreateOneGraphStepOne(nNewID, &m_apGraphs[nLoop], how, pFromGraph, pObs);

	if (nNewID == ID_GRAPHS_OVERLAY)
	{
		if (m_apGraphs[nLoop])
		{
			m_apGraphs[nLoop]->SetWindowText(_T("Overlay"));
			m_apGraphs[nLoop]->ResizeGraph(TRUE, TRUE);
		}
		m_nFocusedID = m_anGraphID[nLoop];
	}
}

/***************************************************************************/
// 09/21/2000 - DDO
/***************************************************************************/
CRecGraphWnd* CSaView::CreateRecGraph(CRecGraphWnd *pFromGraph, Object_istream * pObs)
{
	CRect rWnd(0, 0, 0, 0);

	CRecGraphWnd* pRecGraph = new CRecGraphWnd;
	pRecGraph->SetCaptionStyle(NoneThin);

	if (pFromGraph)
		pRecGraph->PartialCopy(*pFromGraph);
	else if (pObs)
		pRecGraph->bReadProperties(*pObs);
	else
	{
		pRecGraph->ShowXScale(TRUE, FALSE);
		pRecGraph->ShowAnnotation(PHONETIC, FALSE, FALSE);
		pRecGraph->ShowLegend(m_bLegendAll);
		pRecGraph->ShowGrid(TRUE);
		pRecGraph->ShowBoundaries(m_bBoundariesAll);
		pRecGraph->SetMagnify(m_fMagnify);
	}

	if (!pRecGraph->Create(NULL, _T(""), WS_CHILD | WS_VISIBLE, rWnd, this, 0))
	{
		delete pRecGraph;
		pRecGraph = NULL;
	}
	else
	{
		pRecGraph->SetLineDraw(m_bDrawStyleLine);
		if (m_pFocusedGraph == NULL) SetFocusedGraph(pRecGraph);
		m_pFocusedGraph->UpdateStatusBar(m_dwStartCursor, m_dwStopCursor, TRUE);
	}

	return pRecGraph;
}

/***************************************************************************/
// CSaView::ChangeGraph Change the focused graph type
// The graph type of the focused graph will change to the one indicated by
// the parameter (ID). If this type of graph already exists, the focused
// graph will not change.
/***************************************************************************/
void CSaView::ChangeGraph( int nID)
{
	int i = m_nFocusedID ? GetGraphIndexForIDD(m_nFocusedID) : -1;
	ChangeGraph(i, nID);
}

/***************************************************************************/
// CSaView::ChangeGraph Change the focused graph type
// The graph type of the focused graph will change to the one indicated by
// the parameter (ID). If this type of graph already exists, the focused
// graph will not change.
/***************************************************************************/
void CSaView::ChangeGraph( int idx, int nID)
{
	if (GetGraphIndexForIDD(nID) >= 0 && nID != ID_GRAPHS_OVERLAY) 
	{
		if (nID == IDD_RECORDING)
		{
			int nRecording = GetGraphIndexForIDD(IDD_RECORDING);

			DeleteGraphs(nRecording);
			OnGraphsTypesPostProcess(&m_anGraphID[0], m_nLayout);
		}
		return;
	}

	if (nID == IDD_RECORDING)
	{
		UINT anTmpGraphID[MAX_GRAPHS_NUMBER];
		anTmpGraphID[0] = IDD_RECORDING;
		for (int i = 0; i < MAX_GRAPHS_NUMBER - 1; i++) anTmpGraphID[i+1] = m_anGraphID[i];
		OnGraphsTypesPostProcess(&anTmpGraphID[0], m_nLayout);
	}
	else 
	{
		int i = m_nFocusedID ? GetGraphIndexForIDD(m_nFocusedID) : -1;
		if (idx >= 0)
		{
			WINDOWPLACEMENT WP;
			WP.showCmd = SW_HIDE;
			if (nID != ID_GRAPHS_OVERLAY) WP = DeleteGraphs(idx);
			CreateGraph(idx, nID);
			if (m_apGraphs[idx])
			{
				// if the graph we are changing is the focused graph, then reset it
				if (idx == i)
					SetFocusedGraph(m_apGraphs[idx]);
				if(WP.showCmd != SW_HIDE)
					m_apGraphs[idx]->SetWindowPlacement(&WP);
			}
		}
	}
}

/***************************************************************************/
/***************************************************************************/
BOOL CSaView::AssignOverlay( CGraphWnd * pTarget, CSaView * pSourceView)
{
	int nTargetID = pTarget->GetPlotID();
	if (nTargetID == ID_GRAPHS_OVERLAY)
	{
		ASSERT(pTarget);
		ASSERT(pTarget->GetPlot());
		CMultiPlotWnd * pPlot = (CMultiPlotWnd*)pTarget->GetPlot();
		nTargetID = pPlot->GetBasePlotID();
		ASSERT(nTargetID >= 0);
	}

	TRACE(_T("AssignOverlay Focused ID = %d\n"),nTargetID);

	// get the matching graph from the source
	int nSourceIndex = pSourceView->GetGraphIndexForIDD(nTargetID);
	ASSERT(nSourceIndex >= 0);
	if (nSourceIndex < 0) return FALSE;
	CGraphWnd * pGraph = pSourceView->GetGraph(nSourceIndex);
	ASSERT(pGraph);
	if (pGraph == NULL) return FALSE;

	// here's the problem.  with this feature, there may end up
	// being more than one overlay in the view.  We need to find
	// the right one.
	int nPosition = -1;
	for (int j=0;j<MAX_GRAPHS_NUMBER;j++)
	{
		if (m_apGraphs[j] == pTarget)
		{
			nPosition = j;
			break;
		}
	}

	ASSERT(nPosition >= 0);
	if (nPosition < 0) return FALSE;

	m_anGraphID[nPosition] = ID_GRAPHS_OVERLAY;

	m_apGraphs[nPosition]->MergeInGraph( pGraph, (CSaView *)pSourceView, true);

	if (m_apGraphs[nPosition])
	{
		m_apGraphs[nPosition]->SetWindowText(_T("Overlay"));
		m_apGraphs[nPosition]->ResizeGraph(TRUE, TRUE);
	}

	m_nFocusedID = m_anGraphID[nPosition];
	if (m_apGraphs[nPosition])
		SetFocusedGraph(m_apGraphs[nPosition]);

	return TRUE;
}

/***************************************************************************/
// CSaView::ChangeLayout Change graph layout
//
// 09/20/2000 - DDO Reworked to accomodate multiple music graphs in a single
//                  layout section.
/***************************************************************************/
void CSaView::ChangeLayout(UINT nNewLayout)
{
	if (m_nLayout == nNewLayout) return;

	//**************************************************************
	// Get the number of existing graphs and the minimum number
	// needed for the new layout.
	//**************************************************************
	int nOldNumOfGraphs = GetNumberOfGraphs(&m_anGraphID[0]);
	int nMinNumGraphsInNewLayout = GetNumberOfGraphsInLayout(nNewLayout);
	if (IDDSelected(&m_anGraphID[0], IDD_TWC))       nOldNumOfGraphs--;
	if (IDDSelected(&m_anGraphID[0], IDD_MAGNITUDE)) nOldNumOfGraphs--;
	if (IDDSelected(&m_anGraphID[0], IDD_RECORDING)) nOldNumOfGraphs--;
	m_nLayout = nNewLayout;

	int nGraphDiff = nOldNumOfGraphs - nMinNumGraphsInNewLayout;

	//**************************************************************
	// If there are more graphs than will fit into the new layout
	// then delete graphs that won't fit into the new layout.
	//**************************************************************
	if (nGraphDiff > 0)
	{
		for (int i = MAX_GRAPHS_NUMBER - 1; nGraphDiff; i--)
		{
			if (m_apGraphs[i])
			{
				//********************************************************
				// If the graph we're on is any of the melogram-related
				// graphs then treat them as one graph and delete all of
				// them. Otherwise just delete the single graph.
				//********************************************************
				if (m_anGraphID[i] == IDD_TWC || m_anGraphID[i] == IDD_MELOGRAM || m_anGraphID[i] == IDD_MAGNITUDE)
				{
					int nidx = GetGraphIndexForIDD(IDD_TWC);
					if (nidx >= 0) DeleteGraphs(nidx);
					nidx = GetGraphIndexForIDD(IDD_MELOGRAM);
					if (nidx >= 0) DeleteGraphs(nidx);
					nidx = GetGraphIndexForIDD(IDD_MAGNITUDE);
					if (nidx >= 0) DeleteGraphs(nidx);
				}
				else if (m_anGraphID[i] == IDD_RECORDING)  // Position View does not count as a graph
					continue; 
				else DeleteGraphs(i);
				nGraphDiff--;
			}
		}
	}

	nGraphDiff = nOldNumOfGraphs - nMinNumGraphsInNewLayout;

	//**************************************************************
	// If there are not enough graphs to fill the new layout then
	// create enough to fit.
	//**************************************************************
	if (nGraphDiff < 0)
		for (int i = GetNumberOfGraphs(&m_anGraphID[0]); nGraphDiff; i++, nGraphDiff++)
			CreateGraph(i, IDD_BLANK);

	if (!GetFocusedGraphWnd())
		SetFocusedGraph(m_apGraphs[0]);
}

/***************************************************************************/
// CSaView::RefreshGraphs Refresh (redraw) the entire or partial graph window
//**************************************************************************/
void CSaView::RefreshGraphs(BOOL bEntire, BOOL bLegend, BOOL bLayout)
{
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if (m_apGraphs[nLoop])
		{
			if(bLayout)
				m_apGraphs[nLoop]->ResizeGraph(bEntire, bLegend);
			else
				m_apGraphs[nLoop]->RedrawGraph(bEntire, bLegend);
		}
	}
}

/***************************************************************************/
// CSaView::ChangeAnnotationSelection Change the annotation selection
// pSegment points to the segment, whose selection changes, all other
// segments have to be deselected. The start and stop cursors have to be
// moved in place of the selected segment (if not deselecting) and the
// annotation windows have to be informed (via graph). nSelection contains
// the new segment number to select and pSegment points to the annotation
// segment, whose segment will be selected. If a segment becomes selected,
// all the highlighted areas in the plots will be deleted.
//**************************************************************************/
void CSaView::ChangeAnnotationSelection(CSegment* pSegment, 
										int nSelection,
										DWORD dwStart, 
										DWORD dwStop)
{
	// set start- and stop cursor if not deselecting
	BOOL bSelect = FALSE;

	if (nSelection != pSegment->GetSelection()) {
		if (nSelection != -1) {
			// added conditional to prevent cursors from being set if just deselecting  AKE 8/3/2001      
			if (pSegment->GetOffset(nSelection) > GetStopCursorPosition()) {
				// new segment is right of current cursor location
				SetStopCursorPosition(dwStop, SNAP_LEFT);
				SetStartCursorPosition(dwStart, SNAP_RIGHT);        
			} else { // new segment is left of current cursor location
				SetStartCursorPosition(dwStart, SNAP_RIGHT);
				SetStopCursorPosition(dwStop, SNAP_LEFT);
			}      
		}
		bSelect = TRUE;
	}
	// select this segment, deselect all the others
	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		CSegment* pSegments = GetAnnotation(nLoop);
		if (pSegment == pSegments) {
			// this is the segment to select
			pSegment->SetSelection(nSelection);
			// change it in all graphs
			for (int nGraphLoop = 0; nGraphLoop < MAX_GRAPHS_NUMBER; nGraphLoop++) {
				if (m_apGraphs[nGraphLoop]) {
					m_apGraphs[nGraphLoop]->ChangeAnnotationSelection(nLoop);
					if (bSelect) {
						m_apGraphs[nGraphLoop]->GetPlot()->SetHighLightArea(0, 0); // deselect
					}
				}
			}
		} else {
			if (pSegments->GetSelection() != -1) {
				// this segment is to deselect
				pSegments->SetSelection(-1);
				// change it in all graphs
				for (int nGraphLoop = 0; nGraphLoop < MAX_GRAPHS_NUMBER; nGraphLoop++) {
					if (m_apGraphs[nGraphLoop]) {
						m_apGraphs[nGraphLoop]->ChangeAnnotationSelection(nLoop);
					}
				}
			}
		}
	}
}

void CSaView::ChangeAnnotationSelection(CSegment* pSegment, int nSelection)
{

	DWORD dwStart = GetStartCursorPosition();
	DWORD dwStop = GetStopCursorPosition();

	if (pSegment && (nSelection >=0))
		dwStart = pSegment->GetOffset(nSelection);
	if (pSegment && (nSelection >=0))
		dwStop = pSegment->GetDuration(nSelection) + dwStart;

	ChangeAnnotationSelection(pSegment, nSelection, dwStart, dwStop);
}

void CSaView::DeselectAnnotations(void)
{
	CSegment* pSegment = FindSelectedAnnotation();
	if (pSegment) {
		ChangeAnnotationSelection(pSegment, -1);
	}
	// clear virtual selection
	ASelection().Update(this, TRUE); 
}

/***************************************************************************/
// CSaView::SetFocusedGraph Set the graph with focus
// Give graph, pWnd focus and change its caption to the graphs title.
/***************************************************************************/
void CSaView::SetFocusedGraph(CGraphWnd* pWnd) {

	if (m_pFocusedGraph == pWnd) {
		return;
	}
	if (!pWnd) {
		return; // have to set focus to some window...
	}

	//****************************************************
	// Find the ID of the focused Graph.
	//****************************************************
	m_nFocusedID = GraphPtrtoID(pWnd);

	//******************************************************
	// If there was another graph that had focus, reset it.
	//******************************************************
	if (m_pFocusedGraph) m_pFocusedGraph->SetGraphFocus(FALSE);

	//******************************************************
	// Set pointer to new graph with focus.
	//******************************************************
	m_pFocusedGraph = pWnd;
	if (ViewIsActive())
		m_pFocusedGraph->SetGraphFocus(TRUE);

	//******************************************************
	// If new graph with focus is not the recording graph
	// then change its caption.
	//******************************************************
	{
		CSaString szGraph, szCaption;
		pWnd->GetWindowText(szGraph.GetBuffer(64), 64);     // load the graph caption
		szGraph.ReleaseBuffer(-1);
		CSaDoc* pDoc = GetDocument();
		szCaption = pDoc->GetTitle();                       // get current view's caption string
		int nFind = szCaption.Find(':');
		if (nFind != -1) szCaption = szCaption.Left(nFind); // extract part left of :
		szCaption += ": " + szGraph;                        // add new document title
		pDoc->SetTitle(szCaption);                          // write the new caption string
	}

	CSaDoc* pDoc = (CSaDoc*)GetDocument();
	// update the status bar
	if (pDoc->GetDataSize() > 0) {
		pWnd->UpdateStatusBar(GetStartCursorPosition(), GetStopCursorPosition(), TRUE);
	}
}

/***************************************************************************/
// CSaView::ResetFocusedGraph Reset the graph with focus
// No graph has focus.
/***************************************************************************/
void CSaView::ResetFocusedGraph()
{
	// reset focused graph pointer
	m_pFocusedGraph = NULL;
	m_nFocusedID = 0;
	// delete name of graph in caption of view
	CSaString szCaption;
	if (m_pDocument && GetSafeHwnd())
	{
		CSaDoc* pDoc = GetDocument(); // get pointer to document
		szCaption = pDoc->GetTitle(); // get the current view caption string
		int nFind = szCaption.Find(':');
		if (nFind != -1)
			szCaption = szCaption.Left(nFind); // extract part left of first space
		pDoc->SetTitle(szCaption); // write the new caption string
	}
}

/***************************************************************************/
// CSaView::ZoomIn Zoom in
// Tries to set the first displayed data in the manner, that the two cursors
// are in the middle of the data frame. If not possible, the first data
// stays the same. If bZoom is FALSE (default is TRUE), the function won't
// zoom, but only do the rest (center etc).
/***************************************************************************/
void CSaView::ZoomIn(double fZoomAmount, BOOL bZoom)
{
	CSaDoc* pDoc = GetDocument(); // get pointer to document
	CRect rWnd;
	GetClientRect(rWnd);
	FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
	WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);

	// zoom in
	if (bZoom)
		m_fZoom += fZoomAmount; // add zoom
	if (m_fZoom > m_fMaxZoom)
		m_fZoom = m_fMaxZoom;

	m_dwScrollLine = GetDataFrame() * LINE_SCROLL_PIXELWIDTH / rWnd.Width(); // one line scroll width
	if (m_dwScrollLine < wSmpSize)
		m_dwScrollLine = wSmpSize;

	// try to set data between the cursors into the middle of the frame
	DWORD dwDataCenter = GetStartCursorPosition() + (GetStopCursorPosition() - GetStartCursorPosition() + wSmpSize) / 2;

	// limit left border
	if (dwDataCenter > GetDataFrame() / 2)
	{
		m_dwDataPosition = dwDataCenter - GetDataFrame() / 2; // set new data position
		// for 16 bit data value must be even
		if (wSmpSize == 2) {
			m_dwDataPosition &= ~1;
		}
	}
	else 
		m_dwDataPosition = 0;

	// limit right border
	if (m_dwDataPosition > (pDoc->GetDataSize() - GetDataFrame())) // is data position too high?
	{
		m_dwDataPosition = pDoc->GetDataSize() - GetDataFrame(); // reduce data position to maximum
	}

	// set scroll bar
	SetScrollRange(SB_HORZ, 0, (int)((pDoc->GetDataSize() - GetDataFrame()) / m_dwHScrollFactor), FALSE);
	SetScrollPos(SB_HORZ, (int)(m_dwDataPosition / m_dwHScrollFactor), TRUE);

	// if scrolling zoom enabled, set the new position
	if (pViewMainFrame->IsScrollZoom())
	{
		SetScrollPos(SB_VERT, (int)(m_fVScrollSteps + ZOOM_SCROLL_RESOLUTION - m_fVScrollSteps / m_fZoom), TRUE);
	}

	// repaint all graphs (not area and private cursor graphs)
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if ((m_apGraphs[nLoop]) && 
			(!m_apGraphs[nLoop]->IsAreaGraph()) && 
			(!m_apGraphs[nLoop]->HavePrivateCursor()))
			m_apGraphs[nLoop]->RedrawGraph(); // repaint whole graph without legend window
	}
	if (GraphIDtoPtr(IDD_RECORDING)) 
		GraphIDtoPtr(IDD_RECORDING)->GetPlot()->RedrawWindow(NULL,NULL,RDW_INTERNALPAINT|RDW_UPDATENOW);

	pViewMainFrame->SetPlayerTimes();
}

/***************************************************************************/
// CSaView::ZoomOut Zoom out
/***************************************************************************/
void CSaView::ZoomOut(double fZoomAmount)
{
	CSaDoc* pDoc = GetDocument(); // get pointer to document
	FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
	WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);


	if (m_fZoom > 1.0)
	{
		m_fZoom -= fZoomAmount; // subtract zoom
		if (m_fZoom <= 1.0)
		{
			m_fZoom = 1.0;
			SetScrollRange(SB_HORZ, 0, 0, FALSE); // hide scroll bar
			m_dwDataPosition = 0;
		}
		else // repaint all graphs
		{
			CRect rWnd;
			GetClientRect(rWnd);

			// try to set data between the cursors into the middle of the frame
			DWORD dwDataCenter = GetStartCursorPosition() + (GetStopCursorPosition() - GetStartCursorPosition() + wSmpSize) / 2;

			// limit left border
			if (dwDataCenter > GetDataFrame() / 2)
			{
				m_dwDataPosition = dwDataCenter - GetDataFrame() / 2; // set new data position
				// for 16 bit data value must be even
				FmtParm* pFmtParm = pDoc->GetFmtParm();
				if (pFmtParm->wBlockAlign > 1)
					m_dwDataPosition &= ~1;
			}
			else
				m_dwDataPosition = 0;
			// limit right border
			if (m_dwDataPosition > (pDoc->GetDataSize() - GetDataFrame())) // is data position too high?
				m_dwDataPosition = pDoc->GetDataSize() - GetDataFrame(); // reduce data position to maximum
			// set scroll bar
			m_dwScrollLine = GetDataFrame() * LINE_SCROLL_PIXELWIDTH / rWnd.Width(); // one line scroll width
			if(m_dwScrollLine < wSmpSize)
				m_dwScrollLine = wSmpSize;

			SetScrollRange(SB_HORZ, 0, (int)((pDoc->GetDataSize() - GetDataFrame()) / m_dwHScrollFactor), FALSE);
			SetScrollPos(SB_HORZ, (int)(m_dwDataPosition / m_dwHScrollFactor), TRUE);
		}

		// repaint all graphs
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
		{
			if ((m_apGraphs[nLoop]) && (!m_apGraphs[nLoop]->IsAreaGraph()) && 
				(!m_apGraphs[nLoop]->HavePrivateCursor()))
				m_apGraphs[nLoop]->RedrawGraph(); // repaint whole graph without legend window
		}
		if (GraphIDtoPtr(IDD_RECORDING))
			GraphIDtoPtr(IDD_RECORDING)->GetPlot()->RedrawWindow(NULL,NULL,RDW_INTERNALPAINT|RDW_UPDATENOW);

		// if scrolling zoom enabled, set the new position
		if (pViewMainFrame->IsScrollZoom()) {
			SetScrollPos(SB_VERT, (int)(m_fVScrollSteps + ZOOM_SCROLL_RESOLUTION - m_fVScrollSteps / m_fZoom), TRUE);
		}
		pViewMainFrame->SetPlayerTimes();
	}
}

/***************************************************************************/
// CSaView::SetScrolling Set new scrolling parameters
/***************************************************************************/
void CSaView::SetScrolling()
{
	CSaDoc* pDoc = GetDocument(); // get pointer to document
	WORD wSmpSize = WORD(pDoc->GetFmtParm()->wBlockAlign / pDoc->GetFmtParm()->wChannels);
	CRect rWnd;
	GetClientRect(rWnd);
	if(rWnd.Width())
	{
		m_dwScrollLine = GetDataFrame() * LINE_SCROLL_PIXELWIDTH / rWnd.Width(); // one line scroll width
		if(m_dwScrollLine < wSmpSize)
			m_dwScrollLine = wSmpSize;
		m_fMaxZoom = (double)(pDoc->GetDataSize() / pDoc->GetFmtParm()->wBlockAlign) / (double)rWnd.Width() * 8.;  // max zoom factor
		m_fVScrollSteps = ZOOM_SCROLL_RESOLUTION * m_fMaxZoom;
		if (m_fVScrollSteps > 0x7FFFFFFF)
			m_fVScrollSteps = 0x7FFFFFFF;
		m_dwHScrollFactor = (GetDocument()->GetDataSize() / 0x7FFF) + 1;
	}
}

/***************************************************************************/
// CSaView::SetInitialCursors Set initial cursor positions
/***************************************************************************/
void CSaView::SetInitialCursors()
{
	CSaDoc* pDoc = (CSaDoc*)GetDocument(); // get pointer to document
	DWORD dwOffset = 0;
	if (pDoc->GetDataSize() > 0)
	{
		CRect rWnd;
		GetClientRect(rWnd);
		if (rWnd.Width() && GetDataFrame())
		{
			double fBytesPerPix = (double)GetDataFrame() / (double)rWnd.Width(); // calculate data samples per pixel
			if ((DWORD)rWnd.Width() > GetDataFrame())
				fBytesPerPix = (double)pDoc->GetFmtParm()->wBlockAlign;
			dwOffset = (DWORD)(CURSOR_WINDOW_HALFWIDTH / 2 * fBytesPerPix);
		}
		DWORD dwStop = (pDoc->GetDataSize() == 0)? 0 : pDoc->GetDataSize() - 1 - dwOffset;
		SetStartStopCursorPosition(dwOffset, dwStop, SNAP_BOTH); // start cursor position is first sample plus offset
	}
}

// 1.06.5.2 // Removed changes from 1.06.5
/***************************************************************************/
// CSaView::AdjustCursors Adjust cursor positions to new file size
// The function adjusts the cursor positions to the new file size. The file
// size changed at the position dwSectionStart by dwSectionLength bytes and
// it shrunk, if the flag bShrink is TRUE, otherwise it grew.
/***************************************************************************/
void CSaView::AdjustCursors(DWORD dwSectionStart, DWORD dwSectionLength, BOOL bShrink)
{
	FmtParm* pFmtParm = GetDocument()->GetFmtParm();
	WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);

	if (bShrink)
	{
		if (GetStartCursorPosition() >= dwSectionStart)
		{
			if (GetStartCursorPosition() >= (dwSectionStart + dwSectionLength))
				m_dwStartCursor = GetStartCursorPosition() - dwSectionLength;
			else
				m_dwStartCursor = dwSectionStart;
		}
		if (GetStopCursorPosition() >= dwSectionStart)
		{
			if (GetStopCursorPosition() >= (dwSectionStart + dwSectionLength))
				m_dwStopCursor = GetStopCursorPosition() - dwSectionLength;
			else
				m_dwStopCursor = dwSectionStart + dwSectionLength - wSmpSize;
		}
	}
	else
	{
		if (GetStartCursorPosition() >= dwSectionStart)
			m_dwStartCursor = GetStartCursorPosition() + dwSectionLength;
		if (GetStopCursorPosition() >= dwSectionStart)
			m_dwStopCursor = GetStopCursorPosition() + dwSectionLength;
	}
	// for 16 bit data cursor positons must be even
	if (wSmpSize == 2)
	{
		m_dwStartCursor &= ~1;
		m_dwStopCursor &= ~1;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSaView drawing

/***************************************************************************/
// CSaView::OnDraw Drawing
/***************************************************************************/
void CSaView::OnDraw(CDC* /*pDC*/)
{
	CSaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

// SDM 1.06.6U5 split function
/***************************************************************************/
// CSaView::OnInitialUpdate initialization
/***************************************************************************/
void CSaView::OnInitialUpdate()
{
	InitialUpdate();
}

// SDM 1.06.6U5 split function
/***************************************************************************/
// CSaView::InitialUpdate initialization
/***************************************************************************/
void CSaView::InitialUpdate(BOOL bTemp) {

	if (s_pobsAutoload) {
		bReadProperties(*s_pobsAutoload, TRUE);
	} else if ((pSaApp->IsCreatingNewFile()) ||
				(!pViewMainFrame) ||
				(!pViewMainFrame->DefaultIsValid())) {
		CSaDoc *pDoc = GetDocument();
		CSaString docPath = pDoc->GetPathName();
		CFileStatus fileStatus;
		if (CFile::GetStatus(docPath, fileStatus) && (fileStatus.m_attribute & CFile::readOnly)) {
			CSaString docTitle = pDoc->GetTitle() + " (Read-Only)";
			pDoc->SetTitle(docTitle);
		}
		CreateOpenAsGraphs(pSaApp->GetOpenAsID()); // SDM don't open 10-20 blank graphs...
	} else {
		if (bTemp) {
			DeleteGraphs(); // ReadGraphListProperties assumes no graphs
		}
		PartialCopy(*(pViewMainFrame->pDefaultViewConfig()));

		ReadGraphListProperties(*(pViewMainFrame->pDefaultViewConfig()));

		// 08/31/2000 - DDO I'm not sure why this is called here.
		// It's not really necessary and writes over the fonts that
		// are read from the wave file. So it's not only uncessary,
		// it causes problems.
		//pViewMainFrame->SetDocDefaultFonts(this);
	}

	if (!bTemp) {
		CView::OnInitialUpdate();
	}

	// CMDIChildWnd* pwnd = (CMDIChildWnd*)GetParent();     // tdg 12/8/97
	// ASSERT ( pwnd );
	// ASSERT ( pwnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)) );
	// pwnd->ShowWindow(SW_SHOWMAXIMIZED);
	// MessageBeep(0);

	// set scrolling
	SetScrolling();
	SetScrollRange(SB_HORZ, 0, 0, FALSE); // hide horizontal scroll bar

	//**************************************************************
	// if scrolling zoom enabled, show the scroll bar and set the
	// new position otherwise hide the vertical scroll bar.
	//**************************************************************
	if ((pViewMainFrame->IsScrollZoom()) && (GetDocument()->GetDataSize() != 0)) {
		SetScrollRange(SB_VERT, ZOOM_SCROLL_RESOLUTION, (int)m_fVScrollSteps, FALSE);
		SetScrollPos(SB_VERT, (int)(m_fVScrollSteps + ZOOM_SCROLL_RESOLUTION - m_fVScrollSteps / m_fZoom), TRUE);
	} else {
		SetScrollRange(SB_VERT, 0, 0, FALSE);
	}

	//**************************************************************
	// Set initial cursors, Create initial graphs and retile graphs
	//**************************************************************
	SetInitialCursors();
	m_WeJustReadTheProperties = FALSE;
	OnGraphsRetile();

	// prepare scroll information
	m_dwHScrollFactor = (GetDocument()->GetDataSize() / 0x7FFF) + 1;
}

/////////////////////////////////////////////////////////////////////////////
// CSaView printing

/***************************************************************************/
// CSaView::OnPreparePrinting Prepare to print
/***************************************************************************/
BOOL CSaView::OnPreparePrinting(CPrintInfo* pInfo)
{
	BOOL ok = TRUE;
	if (m_pPageLayout->bIsHiRes())
	{
		int pages = CalculateHiResPrintPages();

		if (pages == -1)
		{
			CPrintOptionsDlg * pPgLayoutBckup = new CPrintOptionsDlg(m_pPageLayout);

			// error - bring up the dialog to fix
			// inconsistantcy between number of graphs
			// and number of graphs per page
			m_pPageLayout->SetGraphsPtr(m_apGraphs);
			ok = m_pPageLayout->DoModal()==IDOK;
			if (ok)
			{
				pages = CalculateHiResPrintPages();
			}
			else if (pPgLayoutBckup)
			{
				*m_pPageLayout = *pPgLayoutBckup;
			}

			if (pPgLayoutBckup)
			{
				delete pPgLayoutBckup;
			}
		}
		if (ok)
		{
			pInfo->SetMaxPage(pages);
		}
	}
	else
	{
		pInfo->SetMaxPage(1);
		PreparePrintingForScreenShot();
	}

	if (ok)
	{
		ok = SaDoPreparePrinting(pInfo, m_pPageLayout->bIsLandscape());
	}

	return ok;
}

/***************************************************************************/
// CSaView::OnBeginPrinting Begin printing
/***************************************************************************/
void CSaView::OnBeginPrinting(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	BOOL isColor = (pDC->GetDeviceCaps(NUMCOLORS)>2);

	if (m_pPageLayout->bIsHiRes())
	{
		if (!isColor)
		{
			m_saveColors = *pViewMainFrame->GetColors();
			pViewMainFrame->GetColors()->SetupDefault(TRUE, TRUE);
		}
	}
	else if (m_bPrintPreviewInProgress && !isColor && m_pCDibForPrint)
	{
		m_pCDibForPrint->GoGreyScale();
	}
}

/***************************************************************************/
// CSaView::OnEndPrinting End printing
/***************************************************************************/
void CSaView::OnEndPrinting(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	BOOL isColor = (pDC->GetDeviceCaps(NUMCOLORS)>2);

	if (m_pPageLayout->bIsHiRes())
	{
		if (!isColor)
		{
			*pViewMainFrame->GetColors() = m_saveColors;
		}
	}
	else if (m_pCDibForPrint)
	{
		delete m_pCDibForPrint;
		m_pCDibForPrint = NULL;
	}
	m_bPrintPreviewInProgress = FALSE;
	pViewMainFrame->ClearPreviewFlag();
	pViewMainFrame->ClearPrintingFlag();
	if (m_restartPageOptions)
	{
		PostMessage(WM_COMMAND, ID_FILE_PAGE_SETUP);
	}
}

/***************************************************************************/
// CSaView::OnUpdatePrint Menu Update
/***************************************************************************/
void CSaView::OnUpdateFilePrint(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

/***************************************************************************/
// CSaView::OnUpdatePrintPreview Menu Update
/***************************************************************************/
void CSaView::OnUpdatePrintPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


/***************************************************************************/
// CSaView::OnPrintPageSetup - Setup page layout preferences for printing
/***************************************************************************/
void CSaView::OnPrintPageSetup(void)
{
	if (m_restartPageOptions)
	{
		m_restartPageOptions = FALSE;
	}
	else
	{
		m_pPgLayoutBackup = new CPrintOptionsDlg(m_pPageLayout);
	}

	m_pPageLayout->SetGraphsPtr(m_apGraphs);

	if ((m_pPageLayout->DoModal()!=IDOK) && (m_pPgLayoutBackup))
	{
		*m_pPageLayout = *m_pPgLayoutBackup;
	}

	if (m_pPageLayout->UserClickedPrint())
	{
		PostMessage(WM_COMMAND, ID_FILE_PRINT);
	}
	else if (m_pPageLayout->UserClickedPreview())
	{
		m_restartPageOptions = TRUE;
		PostMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
	}

	if (!m_restartPageOptions && m_pPgLayoutBackup)
	{
		delete m_pPgLayoutBackup;
		m_pPgLayoutBackup = NULL;
	}
}


/***************************************************************************/
// CSaView::GetDataFrame Get the number of displayed data samples
/***************************************************************************/
DWORD CSaView::GetDataFrame()
{
	// calculate current data frame (width of displayed data)
	DWORD dwFrame = (DWORD)((double)GetDocument()->GetDataSize() / m_fZoom);
	// for 16 bit data value must be even
	FmtParm* pFmtParm = GetDocument()->GetFmtParm();
	UINT nSampleSize = pFmtParm->wBlockAlign / pFmtParm->wChannels;
	if (nSampleSize == 2)
		dwFrame &= ~1;
	return dwFrame;
}

/***************************************************************************/
// CSaView::GetDataPosition
// Returns the scroll bar position in bytes
// Since scrolling is really a pixel based function the position is first 
// converted to pixels then to bytes using the width provided
/***************************************************************************/
double CSaView::GetDataPosition(int nWndWidth)
{
	double fDataPos = m_dwDataPosition;

	DWORD dwDataFrame = GetDataFrame(); // number of data points to display

	// SDM 1.06.6U4 Calculate position based on pixel aligned graph
	if(nWndWidth && dwDataFrame)
	{
		double fBytesPerPix = (double)dwDataFrame / (double)(nWndWidth); // calculate data samples per pixel
		fDataPos = round(fDataPos/fBytesPerPix)*fBytesPerPix;
	}

	return fDataPos;
}


/***************************************************************************/
// CSaView::AdjustDataFrame Adjust data frame width for particular window
// If a window displayes the data frame, but the window width in pixels is
// larger than the frame, it wants to extend the frame to fill up the whole
// window with data. The function does not affect the views data frame
// settings. It checks and limits the result against the available data size.
/***************************************************************************/
DWORD CSaView::AdjustDataFrame(int nWndWidth)
{
	DWORD dwDataFrame = GetDataFrame(); // number of data points to display
	if (dwDataFrame == 0)
		return 0; // nothing to draw

	CSaDoc* pDoc = (CSaDoc*) GetDocument(); // get pointer to document
	DWORD dwDataSize = pDoc->GetDataSize();
	if (dwDataSize >= GetDataPosition(nWndWidth) && (GetDataPosition(nWndWidth) + dwDataFrame) > dwDataSize)
	{
		dwDataFrame = DWORD(dwDataSize - GetDataPosition(nWndWidth));
	}
	return dwDataFrame;
}


/***************************************************************************/
// CSaView::SetCursorPosition Set the specified cursor position
/***************************************************************************/
void CSaView::SetCursorPosition(int nCursorSelect, 
								DWORD dwNewPos, 
								SNAP_DIRECTION nSnapDirection,
								CURSOR_ALIGNMENT nCursorAlignment)
{
	switch(nCursorSelect)
	{
	case START_CURSOR:
		SetStartCursorPosition(dwNewPos, nSnapDirection, nCursorAlignment);
		if (GetDynamicGraphCount())
		{
			CSaDoc* pDoc = (CSaDoc*) GetDocument(); // get pointer to document
			FmtParm* pFmtParm = pDoc->GetFmtParm();
			UINT nSampleSize = pFmtParm->wBlockAlign / pFmtParm->wChannels;
			if (GetAnimationGraphCount())
				StartAnimation(m_dwStartCursor/nSampleSize, m_dwStopCursor/nSampleSize);
			EndAnimation();
		}
		break;

	case STOP_CURSOR:
		SetStopCursorPosition(dwNewPos, nSnapDirection, nCursorAlignment);
		break;
	}
}


/***************************************************************************/
// CSaView::SetStartCursorPosition Set the start cursor
/***************************************************************************/
void CSaView::SetStartCursorPosition(DWORD dwNewPos, SNAP_DIRECTION nSnapDirection, CURSOR_ALIGNMENT nCursorAlignment)
{
	SetStartStopCursorPosition(dwNewPos, m_dwStopCursor, nSnapDirection, nCursorAlignment);
}

/***************************************************************************/
// CSaView::SetStopCursorPosition Set the stop cursor
/***************************************************************************/
void CSaView::SetStopCursorPosition(DWORD dwNewPos, SNAP_DIRECTION nSnapDirection, CURSOR_ALIGNMENT nCursorAlignment)
{
	SetStartStopCursorPosition(m_dwStartCursor, dwNewPos, nSnapDirection, nCursorAlignment);
}

/***************************************************************************/
// CSaView::SetStartStopCursorPosition Set the start cursor
// kg in some case we need to update the member variables before
// we update the graphs in case the previous stop position is invalid
// for the new graph.
/***************************************************************************/
void CSaView::SetStartStopCursorPosition( DWORD dwNewStartPos,  
										 DWORD dwNewStopPos, 
										 SNAP_DIRECTION nSnapDirection, 
										 CURSOR_ALIGNMENT nCursorAlignment)
{
	if (nCursorAlignment == ALIGN_USER_SETTING)
		nCursorAlignment = GetCursorAlignment();

	CSaDoc* pDoc = (CSaDoc*) GetDocument(); // get pointer to document
	m_dwStartCursor = dwNewStartPos;
	m_dwStopCursor = dwNewStopPos;

	// position limit
	if (pDoc->GetDataSize() > 0)
	{
		if (m_dwStartCursor >= pDoc->GetDataSize())
			m_dwStartCursor = pDoc->GetDataSize() - 1;
		if (m_dwStopCursor >= pDoc->GetDataSize())
			m_dwStopCursor = pDoc->GetDataSize() - 1;
	}

	// for 16 bit data value must be even
	FmtParm* pFmtParm = pDoc->GetFmtParm();
	if (pFmtParm->wBlockAlign > 1)
	{
		m_dwStartCursor &= ~1;
		m_dwStopCursor &= ~1;
	}

	// snap the start cursor if necessary
	int nSampleSize = 1;
	if (pFmtParm->wBlockAlign > 1)
		nSampleSize = 2;

	m_dwStartCursor = pDoc->SnapCursor(START_CURSOR, m_dwStartCursor, 0, m_dwStopCursor - nSampleSize,
		nSnapDirection, nCursorAlignment);
	m_dwStopCursor = pDoc->SnapCursor(STOP_CURSOR, m_dwStopCursor, m_dwStartCursor + nSampleSize,
		GetDocument()->GetDataSize() - nSampleSize, nSnapDirection, nCursorAlignment);

	// move start cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if (m_apGraphs[nLoop]) m_apGraphs[nLoop]->SetStartCursor(this);
		if (m_apGraphs[nLoop]) m_apGraphs[nLoop]->SetStopCursor(this);
	}

	pViewMainFrame->SetPlayerTimes();

}

// SDM 1.5Test10.5
/***************************************************************************/
// CSaView::SetPlaybackPosition Set the playbackPosition
/***************************************************************************/
void CSaView::SetPlaybackPosition(double dNewPos, int nSpeed, BOOL bEstimate)
{
	//if(!bEstimate)
	//  TRACE(_T("PlaybackPosition %8.1f Position %8.1f nSpeed %d Estimate %d\n"),m_dPlaybackPosition, dNewPos, nSpeed, bEstimate);
	if(bEstimate)
	{
		SetTimer( ID_PLAYER, /*PLAYBACK_CURSOR_UPDATE_INTERVAL * 1000 */ 1 , NULL );
	}
	else
	{
		if (nSpeed)
		{
			m_dwPlaybackTime = GetTickCount();
			SetTimer( ID_PLAYER, /*PLAYBACK_CURSOR_UPDATE_INTERVAL * 1000 */ 1 , NULL );
		}
		m_dPlaybackPosition = dNewPos;
		m_nPlaybackSpeed = nSpeed;
	}

	DWORD dwPlaybackPosition = ((DWORD)dNewPos);

	// for 16 bit data value must be even
	FmtParm* pFmtParm = GetDocument()->GetFmtParm();
	if (pFmtParm->wBlockAlign > 1)
		dwPlaybackPosition = dwPlaybackPosition & ~1;
	// move start cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if (m_apGraphs[nLoop])
			m_apGraphs[nLoop]->SetPlaybackPosition(this, dwPlaybackPosition);
	}
}


// SDM 1.5Test10.5
/***************************************************************************/
// CSaView::OnTimer Set the playbackPosition
/***************************************************************************/
void CSaView::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == ID_PLAYER)
	{
		if (m_nPlaybackSpeed)
		{
			double dNewPos = m_dPlaybackPosition + (GetTickCount()-m_dwPlaybackTime)/GetDocument()->GetTimeFromBytes(1000) * m_nPlaybackSpeed / 100;
			SetPlaybackPosition(dNewPos,m_nPlaybackSpeed, TRUE);
		}
	}
	else
	{
		CView::OnTimer(nIDEvent);
	}
}

/***************************************************************************/
// CSaView::iGetStartCursorSegment Get segment index at start cursor position
/***************************************************************************/
int CSaView::iGetStartCursorSegment(int iSegment)
{
	int idxPosition = 0;

	CSegment* pSegment = GetAnnotation(iSegment);
	ASSERT(pSegment);

	// get pointer to annotation offsets
	CDWordArray* pOffsets = pSegment->GetOffsets();
	ASSERT(pOffsets);

	if (pOffsets->GetSize() > 0) // there is at least one segment
		// get the segment index at the given position
		idxPosition = pSegment->FindFromPosition(m_dwStartCursor);

	return idxPosition;
}

/***************************************************************************/
// CSaView::iGetStopCursorSegment Get segment index at stop cursor position
/***************************************************************************/
int CSaView::iGetStopCursorSegment(int iSegment)
{
	int idxPosition = 0;

	CSegment* pSegment = GetAnnotation(iSegment);
	ASSERT(pSegment);

	// get pointer to annotation offsets
	CDWordArray* pOffsets = pSegment->GetOffsets();
	ASSERT(pOffsets);

	if (pOffsets->GetSize() > 0) // there is at least one segment
		// get the segment index at the given position
		idxPosition = pSegment->FindFromPosition(m_dwStopCursor);

	return idxPosition;
}

/***************************************************************************/
// CSaView::MoveStartCursor Move the start cursor
/***************************************************************************/
void CSaView::MoveStartCursor(DWORD dwNewPos)
{
	// move start cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if (m_apGraphs[nLoop]) m_apGraphs[nLoop]->MoveStartCursor(this, dwNewPos);
	}
}

/***************************************************************************/
// CSaView::MoveStopCursor Move the stop cursor
/***************************************************************************/
void CSaView::MoveStopCursor(DWORD dwNewPos)
{
	// move stop cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if (m_apGraphs[nLoop]) m_apGraphs[nLoop]->MoveStopCursor(this, dwNewPos);
	}
}

/***************************************************************************/
// CSaView::SetDataFrame Set number of data samples to display
// The caller delivers a start position (first data sample he wants to have
// displayed) and the number of samples to display. Then this function
// calculates the nearest possible zooming factor and sets up the real dis-
// playable first data and the number of samples to display.
/***************************************************************************/
void CSaView::SetDataFrame(DWORD dwStart, DWORD dwFrame)
{
	CSaDoc* pDoc = GetDocument(); // get pointer to document
	WORD wSmpSize = WORD(pDoc->GetFmtParm()->wBlockAlign / pDoc->GetFmtParm()->wChannels);
	CRect rWnd;
	GetClientRect(rWnd);
	m_dwDataPosition = dwStart; // set start position
	// for 16 bit data value must be even
	FmtParm* pFmtParm = pDoc->GetFmtParm();
	if (pFmtParm->wBlockAlign > 1) m_dwDataPosition &= ~1;
	// calculate zooming factor
	DWORD dwDataSize = pDoc->GetDataSize();
	double fZoom = (double)dwDataSize / (double)dwFrame; // zoom factor
	if(fZoom > m_fMaxZoom)
	{
		fZoom = m_fMaxZoom;
		dwFrame = DWORD(dwDataSize/fZoom);
		if (pFmtParm->wBlockAlign > 1) dwFrame &= ~1;
	}
	// if scrolling zoom enabled, set the new position
	if (pViewMainFrame->IsScrollZoom()) SetScrollPos(SB_VERT, (int)(m_fVScrollSteps + ZOOM_SCROLL_RESOLUTION - m_fVScrollSteps / m_fZoom), TRUE);
	if (fZoom > 1.0) // zoom in
	{
		// set new scrolling parameters
		m_dwScrollLine = dwFrame * LINE_SCROLL_PIXELWIDTH / rWnd.Width(); // one line scroll width
		if(m_dwScrollLine < wSmpSize)
			m_dwScrollLine = wSmpSize;
		SetScrollRange(SB_HORZ, 0, (int)((dwDataSize - dwFrame) / m_dwHScrollFactor), FALSE);
		if (m_fZoom == 1.0) // first zoom in -> set scroll bar
			SetScrollPos(SB_HORZ, (int)(m_dwDataPosition / m_dwHScrollFactor), TRUE);
		else // not first zoom in, repaint all graphs
		{
			for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
			{
				if (m_apGraphs[nLoop]) m_apGraphs[nLoop]->RedrawGraph(); // repaint whole graph window without legend
			}
		}
		m_fZoom = fZoom; // set new zooming factor
	}
	else // no zooming
	{
		m_dwDataPosition = 0;
		if (m_fZoom > 1.0) // already zoomed in
			SendMessage(WM_COMMAND, ID_GRAPHS_ZOOM_ALL, 0L); // zoom all
	}
}

/***************************************************************************/
// CSaView::GetDataFrame returns the parameters set by set
/***************************************************************************/
void CSaView::GetDataFrame(DWORD &dwStart, DWORD &dwFrame)
{
	CSaDoc* pDoc = GetDocument(); // get pointer to document

	dwStart = m_dwDataPosition; // set start position

	// calculate zooming factor
	DWORD dwDataSize = pDoc->GetDataSize();

	dwFrame = (DWORD)(((double)dwDataSize) / m_fZoom + 0.5); // zoom factor
}

LRESULT CSaView::OnCursorInFragment(WPARAM nCursorSelect, LPARAM dwFragmentIndex)
{
	switch (nCursorSelect)
	{
	case START_CURSOR:
		CSaDoc *pDoc = GetDocument();
		CProcessFragments *pFragments = pDoc->GetFragments();
		FRAG_PARMS FragmentParms = pFragments->GetFragmentParms(dwFragmentIndex);
		if (GetDynamicGraphCount()) StartAnimation(FragmentParms.dwOffset, FragmentParms.dwOffset);
		break;
	}
	//  BroadcastMessage(WM_USER_CURSOR_IN_FRAGMENT, nCursorSelect, dwFragmentIndex);
	return 0L;
}

void CSaView::NotifyFragmentDone(void * /*pCaller*/)
{
	if(GetCursorAlignment() == ALIGN_AT_FRAGMENT)
		OnCursorAlignmentChanged();   // fragmenter finished, alignment mode just changed
}

UINT CSaView::GetDynamicGraphCount(void)
{
	UINT nCount = 0;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if (m_apGraphs[nLoop])
		{
			BOOL bDynamicMode = (GetGraphUpdateMode() == DYNAMIC_UPDATE && m_apGraphs[nLoop]->IsAnimationGraph());
			if (bDynamicMode) nCount++;
		}
		else break;
	}

	return nCount;
}

/***************************************************************************/
// CSaView::OnCursorAlignmentChanged Cursor alignment option has changed
/***************************************************************************/
LRESULT CSaView::OnCursorAlignmentChanged(WPARAM, LPARAM)
{
	SetStartCursorPosition(m_dwStartCursor, SNAP_BOTH);
	SetStopCursorPosition(m_dwStopCursor, SNAP_BOTH);

	OnGraphUpdateModeChanged();
	OnAnimationChanged();
	return 0;
}

/***************************************************************************/
// CSaView::OnGraphUpdateModeChanged  Graph update mode has changed
/***************************************************************************/
LRESULT CSaView::OnGraphUpdateModeChanged(WPARAM , LPARAM)
{
	int GraphUpdateMode = GetGraphUpdateMode();

	if (GraphUpdateMode == DYNAMIC_UPDATE)
	{
		CSaDoc* pDoc = GetDocument();
		FmtParm* pFmtParm = pDoc->GetFmtParm();
		WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
		CProcessFragments* pFragments = pDoc->GetFragments();
		DWORD dwFragmentIndex = pFragments->GetFragmentIndex(m_dwStartCursor/wSmpSize);
		OnCursorInFragment(START_CURSOR, dwFragmentIndex);
	}
	else
	{
		int nWaveGraphIndex = GetGraphIndexForIDD(IDD_RAWDATA);
		CGraphWnd* pWaveGraph = GetGraph(nWaveGraphIndex);
		if (pWaveGraph)
		{
			CPlotWnd* pWavePlot = pWaveGraph->GetPlot();
			pWavePlot->SetHighLightArea(0, 0);     // turn off highlighted area in raw waveform
		}
	}
	return 0;
}

/***************************************************************************/
// CSaView::OnAnimationChanged Animation selection has changed
/***************************************************************************/
LRESULT CSaView::OnAnimationChanged(WPARAM, LPARAM)
{
	BOOL AnimateChecked = IsAnimationRequested();
	if (AnimateChecked)
	{
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
		{
			if (m_apGraphs[nLoop])
			{
				BOOL bAnimationGraph = m_apGraphs[nLoop]->IsAnimationGraph();
				if (bAnimationGraph)
				{
					CPlotWnd* pPlot = m_apGraphs[nLoop]->GetPlot();
					if (pPlot) pPlot->RedrawWindow();
				}
			}
		}
	}
	return 0;
}

/***************************************************************************/
// CSaView::GetAnimationGraphCount  Return number of graphs to animate
/***************************************************************************/
UINT CSaView::GetAnimationGraphCount(void)
{
	UINT nCount = 0;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if (m_apGraphs[nLoop])
		{
			BOOL bAnimation = IsAnimationRequested() && m_apGraphs[nLoop]->IsAnimationGraph();
			if (bAnimation) nCount++;
		}
		else break;
	}
	return nCount;
}


/***************************************************************************/
// CSaView::StartAnimation  Animate from specified start to stop sample
/***************************************************************************/
BOOL CSaView::StartAnimation(DWORD dwStartWaveIndex, DWORD dwStopWaveIndex)
{

	CSaDoc* pDoc = (CSaDoc*) GetDocument(); // get pointer to document
	//  FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
	//  WORD wSmpSize = (WORD)(pFmtParm->wBlockAlign / pFmtParm->wChannels);  // calculate sample size in bytes
	CProcessFragments* pFragments = pDoc->GetFragments();
	if (!pFragments->IsDataReady()) return FALSE;
	DWORD dwStartFrame = pFragments->GetFragmentIndex(dwStartWaveIndex);
	DWORD dwStopFrame = pFragments->GetFragmentIndex(dwStopWaveIndex);
	BOOL bFreezeFrame = (dwStartFrame == dwStopFrame);
	float fResyncTime = 0.F;
	BOOL bCancelAnimation = FALSE;
	m_bAnimating = TRUE;
	/*
	int nWaveGraphIndex = GetGraphIndexForIDD(IDD_RAWDATA);
	CGraphWnd* pWaveGraph = GetGraph(nWaveGraphIndex);
	CPlotWnd* pWavePlot = NULL;
	if (pWaveGraph) pWavePlot = pWaveGraph->GetPlot();
	*/
	for (DWORD dwFrameIndex = dwStartFrame; dwFrameIndex <= dwStopFrame; dwFrameIndex++)
	{
		if (m_pStopwatch && !bFreezeFrame)
		{
			if (dwFrameIndex == dwStartFrame) m_pStopwatch->Reset();   // timing only required to synchronize animations
			else if (dwFrameIndex == dwStartFrame + 1)
			{
				float fFrameTime = m_pStopwatch->GetElapsedTime();
				float fFrameTimeRequested = 1.F / (float)GetAnimationFrameRate();
				fResyncTime = fFrameTimeRequested - fFrameTime;
				/* if (m_Animation.fResyncTime < 0.F)
				{
				//!!set top end of animation rate options
				} */
			}
		}
		if (fResyncTime > 0.F) m_pStopwatch->Wait(fResyncTime);  // delay to synchronize with requested frame rate

		/*
		FRAG_PARMS FragParms = pFragments->GetFragmentParms(dwFrameIndex);
		DWORD dwFrameStart = FragParms.dwOffset * wSmpSize;
		DWORD dwFrameSize = (DWORD)FragParms.wLength * (DWORD)wSmpSize;
		if (pWavePlot) 
		{
		pWavePlot->SetHighLightArea(dwFrameStart, dwFrameStart + dwFrameSize, TRUE, TRUE);
		pWavePlot->UpdateWindow();
		}
		*/
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
		{
			// Any keydown might cancel animation
			if(GetAsyncKeyState(VK_ESCAPE) < 0)
			{
				bCancelAnimation = TRUE;
				break;
			}
			if (!m_apGraphs[nLoop]) break;
			BOOL bDynamicMode = (GetGraphUpdateMode() == DYNAMIC_UPDATE && m_apGraphs[nLoop]->IsAnimationGraph());
			BOOL bAnimation = (IsAnimationRequested() && m_apGraphs[nLoop]->IsAnimationGraph());
			if (bFreezeFrame && bDynamicMode || !bFreezeFrame && bAnimation)
			{
				m_apGraphs[nLoop]->AnimateFrame(dwFrameIndex);
				if (m_apGraphs[nLoop]->IsCanceled())
				{
					bCancelAnimation = TRUE;
					break;
				}
			}
		}
		if (bCancelAnimation)
		{
			EndAnimation();
			break;
		}
	}
	/*
	if (pWavePlot)
	{
	pWavePlot->SetHighLightArea(0, 0);
	pWavePlot->UpdateWindow();
	}
	*/
	m_bAnimating = FALSE;
	return TRUE;
}

/***************************************************************************/
// CSaView::EndAnimation  Finish animation
/***************************************************************************/
void CSaView::EndAnimation()
{
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if (!m_apGraphs[nLoop]) break;
		BOOL bDynamicMode = (GetGraphUpdateMode() == DYNAMIC_UPDATE && m_apGraphs[nLoop]->IsAnimationGraph());
		if (bDynamicMode) m_apGraphs[nLoop]->EndAnimation();
	}
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
//
//    PRINTING CODE - PART 2  (also see PreparePrinting, BeginPrinting,
//                             and EndPrinting above)
//
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
// CSaView::OnPrepareDC - prepare printing context for printing.
//                        (set up scaling, page size, and margins)
/***************************************************************************/
void CSaView::OnPrepareDC(CDC * pDC, CPrintInfo * pInfo)
{
	if (pDC->IsPrinting())
	{
		// get the printers dots per inch
		m_printerDPI = CPoint(pDC->GetDeviceCaps(LOGPIXELSX),pDC->GetDeviceCaps(LOGPIXELSY));

		// calculate a m_printArea that will yield the desired margins, also checking
		// to see if the printable region will allow those margins.
		CalculatePrintArea(pDC, pInfo);

		if (m_pPageLayout->bIsHiRes())
		{
			PrepareDCforHiResPrint(pDC, pInfo);
		}
		//  set up a print origin that yields the desired margins
		CalculatePrintOrigin(pDC);

	}
	/*
	else
	{
	; // nothing needed here so far.
	}
	*/
}

/***************************************************************************/
// CSaView::CalculatePrintOrigin - calculate a m_printOrigin that will
// yield the desired margins
/***************************************************************************/
void CSaView::CalculatePrintOrigin(CDC * pDC)
{
	CPoint desiredOffset(fract_multiply(Print_Margin,m_printerDPI.x),
		fract_multiply(Print_Margin,m_printerDPI.y));
	POINT pageOffset;
	pDC->Escape(GETPRINTINGOFFSET, NULL, NULL, &pageOffset);
	m_printOrigin.x = desiredOffset.x - pageOffset.x;
	m_printOrigin.y = desiredOffset.y - pageOffset.y;
	if (m_printOrigin.x<0)
		m_printOrigin.x=0; // no printing out of the printable area
	if (m_printOrigin.y<0)
		m_printOrigin.y=0; // no printing out of the printable area
	pDC->DPtoLP(&m_printOrigin, 1);
}

/***************************************************************************/
// CSaView::PrepareDCforHiResPrint - prepare printing context for printing.
//                                  (set up scaling, page size, and margins)
/***************************************************************************/
void CSaView::PrepareDCforHiResPrint(CDC * pDC, CPrintInfo * /*pInfo*/)
{
	// set the logical dots per inch based on user preferences.
	// the higher logical dots per inch yields smaller fonts
	m_newPrinterDPI = (m_pPageLayout->bUseSmallFonts()) ? Small_Font_DPI : Standard_DPI;

	//  we stretch the pixels by ViewportExt/WindowExt.
	//  if the ViewportExt = printer dpi, then WindowExt determines the pixels per inch.
	pDC->SetMapMode(MM_ANISOTROPIC);
	pDC->SetWindowExt(m_newPrinterDPI, m_newPrinterDPI);
	pDC->SetViewportExt(m_printerDPI.x,m_printerDPI.y);
	pDC->DPtoLP(&m_printArea,1);
}



/***************************************************************************/
// CSaView::CalculatePrintArea
//
// Calculate a m_printArea that will yield the desired margins, also checking
// to see if the printable region will allow those margins.
/***************************************************************************/
void CSaView::CalculatePrintArea(CDC * pDC, CPrintInfo * pInfo)
{
	pDC->Escape(GETPHYSPAGESIZE, NULL, NULL, &m_printArea);
	m_printArea.x -= m_printerDPI.x; // subtract half inch margins on each side
	m_printArea.y -= m_printerDPI.y; // subtract half inch margins on each side
	CRect printableArea(pInfo->m_rectDraw);
	pDC->LPtoDP(&printableArea);
	if (printableArea.Width())
	{
		m_printArea.x = min(m_printArea.x , printableArea.Width());
	}
	if (printableArea.Height())
	{
		m_printArea.y = min(m_printArea.y , printableArea.Height());
	}
}




/***************************************************************************/
// CSaView::OnPrint
//
// Print the current page.
/***************************************************************************/
void CSaView::OnPrint(CDC * pDC, CPrintInfo * pInfo)
{
	OnPrepareDC(pDC,pInfo); // SDM1.5Test10.8 duplicate call (first call dependencies not initialized)

	if (m_pPageLayout->bIsHiRes())
	{
		DoHiResPrint(pDC, pInfo);
	}
	else
	{
		DoScreenShot(pDC, pInfo);
	}
}



/***************************************************************************/
// CSaView::DoScreenShot
//
// Print the screen.
/***************************************************************************/
void CSaView::DoScreenShot(CDC * pDC, CPrintInfo * /*pInfo*/)
{
	if (!m_pCDibForPrint)
	{
		// SDM 1.5Test10.8 do not use null pointer
		ASSERT(0);
	}
	else
	{
		CRect targRect(m_printOrigin.x,m_printOrigin.y,
			m_printOrigin.x + m_printArea.x,
			m_printOrigin.y + m_printArea.y);

		double targSizeInch1200thsX = targRect.Width() * (1200.0 / m_printerDPI.x);
		double targSizeInch1200thsY = targRect.Height() * (1200.0 / m_printerDPI.y);
		double srcSizePixelsX = m_pCDibForPrint->GetSize().cx;
		double srcSizePixelsY = m_pCDibForPrint->GetSize().cy;

		if ((targSizeInch1200thsX / srcSizePixelsX) >
			(targSizeInch1200thsY / srcSizePixelsY))
		{

			// we are strecthing x more than y.
			targSizeInch1200thsX = srcSizePixelsX *
				(targSizeInch1200thsY / srcSizePixelsY);
		}
		else
		{
			targSizeInch1200thsY = srcSizePixelsY *
				(targSizeInch1200thsX / srcSizePixelsX);
		}

		targRect = CRect(m_printOrigin.x,m_printOrigin.y,
			(int)(m_printOrigin.x + (m_printerDPI.x / 1200.0) * targSizeInch1200thsX),
			(int)(m_printOrigin.y + (m_printerDPI.y / 1200.0) * targSizeInch1200thsY));

		if (!m_pCDibForPrint->Paint(pDC, targRect))
			ASSERT(0);
	}
}




/***************************************************************************/
// CSaView::DoHiResPrint
//
// Print the graphs using the OnDraw() methods.
/***************************************************************************/
void CSaView::DoHiResPrint(CDC * pDC, CPrintInfo * pInfo)
{
	CRect viewRect;
	GetWindowRect(&viewRect);

	int titleAreaHeight = fract_multiply(Print_Title_Height,m_newPrinterDPI);  // half inch
	PrintPageTitle(pDC, titleAreaHeight);

	// adjust print area and origin to move past the title
	CPoint oldOrg(-m_printOrigin);
	oldOrg.y -= titleAreaHeight;
	pDC->SetWindowOrg(oldOrg);
	m_printArea.y -= titleAreaHeight;

	CalcPrintScaling(&viewRect);

	if (m_pPageLayout->LayoutType() == m_pPageLayout->LAYOUT_1GRAPH_PER_PAGE())
	{
		int graphIndex = CalcGraphIndexForCurrentPage(pInfo->m_nCurPage);
		if (graphIndex < MAX_GRAPHS_NUMBER)
		{
			PrintGraph(pDC, &viewRect, graphIndex, &viewRect, oldOrg.x, oldOrg.y);
		}
	}
	else if (m_pPageLayout->LayoutType() == m_pPageLayout->LAYOUT_AS_SCREEN())
	{
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
		{
			if (m_apGraphs[nLoop])
			{
				CRect graphRect;
				m_apGraphs[nLoop]->GetWindowRect(&graphRect);
				PrintGraph(pDC, &viewRect, nLoop, &graphRect, oldOrg.x, oldOrg.y);
			}
		}
	}
	else
	{
		// custom layout - user specifies number of rows and columns
		int virtualPage = (pInfo->m_nCurPage - 1) * m_pPageLayout->NumRows() * m_pPageLayout->NumCols() + 1;
		for (int row = 1;row <= m_pPageLayout->NumRows(); row++)
		{
			for (int col = 1;col <= m_pPageLayout->NumCols(); col++)
			{
				int graphIndex = CalcGraphIndexForCurrentPage(virtualPage);
				CRect customPage;
				CalcCustomPage(&customPage, &viewRect, row, col);
				if (graphIndex < MAX_GRAPHS_NUMBER)
				{
					PrintGraph(pDC, &viewRect, graphIndex, &customPage, oldOrg.x, oldOrg.y);
				}
				virtualPage++;
			}
		}
	}
}

/***************************************************************************/
// CSaView::CalcGraphIndexForCurrentPage
//
// Calculate which graph to use for the current page or virtual page
// (if there are 4 graphs to a page, then each graph is considered a
//  different virtual page).
/***************************************************************************/
int CSaView::CalcGraphIndexForCurrentPage(int currPage)
{
	int page = 0;
	int graphForPage = 0;

	for (; graphForPage < MAX_GRAPHS_NUMBER; graphForPage++)
	{
		if (m_apGraphs[graphForPage] && m_pPageLayout->IsGraphSelected(graphForPage))
		{
			page++;
		}

		if (page==currPage)
		{
			break;
		}
	}

	return graphForPage;
}


/***************************************************************************/
// CSaView::CalcCustomPage
//
// Calculate the area to use for the given graph row and column of a custom page.
/***************************************************************************/
void CSaView::CalcCustomPage(CRect * customPage, const CRect * viewRect, int row, int col)
{
	int seperator = fract_multiply(Print_Graph_Seperator,m_newPrinterDPI);
	CPoint customDimensions(viewRect->Width() / m_pPageLayout->NumCols(),
		viewRect->Height() / m_pPageLayout->NumRows());

	customPage->left = viewRect->left + (col - 1) * customDimensions.x + seperator;
	customPage->right = customPage->left + customDimensions.x - seperator;
	customPage->top  = viewRect->top + (row - 1) * customDimensions.y + seperator;
	customPage->bottom = customPage->top + customDimensions.y - seperator;
}


/***************************************************************************/
// CSaView::PrintPageTitle
//
// Print a title on the page.
/***************************************************************************/
void CSaView::PrintPageTitle(CDC * pDC, int titleAreaHeight)
{
	CSaString szDocTitle(GetDocument()->GetTitle()); // load file name
	CSaString szTitle("Speech Analyzer - ");

	int nFind = szDocTitle.Find(':');
	if (nFind != -1)
		szDocTitle = szDocTitle.Left(nFind); // extract part left of :
	szTitle += szDocTitle;

	int textWidth = pDC->GetTextExtent(szTitle,szTitle.GetLength()).cx;

	CRect titleRect((m_printArea.x - textWidth) / 2,  // centered on the page
		fract_multiply(Print_Title_Offset,m_newPrinterDPI),
		m_printArea.x,
		titleAreaHeight);

	COLORREF oldColor = pDC->SetTextColor(RGB(0,0,0)); // set font color

	const TCHAR* pszName = _T("Arial");

	CFont titleFont;
	titleFont.CreateFont(Print_Title_Size, 0, 0, 0, FW_NORMAL, 0, 0, 0,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN), pszName);

	CFont * pOldFont = pDC->SelectObject(&titleFont);

	pDC->DrawText(szTitle, szTitle.GetLength(), titleRect,
		DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOCLIP);

	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(oldColor);
}




/***************************************************************************/
// CSaView::PrintGraph
//
// Print a graph in an area defined by the graphRect and its offset from
// the viewRect.  Use graph index == graphNum.
/***************************************************************************/
void CSaView::PrintGraph(CDC * pDC, const CRect * viewRect, int graphNum,
						 const CRect * graphRect,
						 int originX, int originY)
{
	CRect printRect;

	// calculate printRect and scaledOffset
	CalcPrintRect(&printRect,graphRect);
	CPoint offset(viewRect->left - graphRect->left,
		viewRect->top  - graphRect->top);
	CPoint scaledOffset;
	CalcPrintPoint(&scaledOffset, &offset);

	originX += scaledOffset.x;
	originY += scaledOffset.y;

	pDC->SetWindowOrg(CPoint(originX,originY));

	m_apGraphs[graphNum]->OnDraw(pDC, &printRect, originX, originY);
}



/***************************************************************************/
// CSaView::CalcPrintScaling
//
// Calculate the scaling factor to fit the srcRect in the m_printArea.
/***************************************************************************/
void CSaView::CalcPrintScaling(const CRect * srcRect)
{
	m_printScaleX = (double)((1.0 * m_printArea.x)  / srcRect->Width());
	m_printScaleY = (double)((1.0 * m_printArea.y)  / srcRect->Height());

	if (m_pPageLayout->bFixedAspectR())
	{
		// force equal scaling of x and y
		if (m_printScaleX > m_printScaleY)
		{
			m_printScaleX = m_printScaleY;
		}
		else if (m_printScaleY > m_printScaleX)
		{
			m_printScaleY = m_printScaleX;
		}
	}
}





/***************************************************************************/
// CSaView::CalcPrintRect
//
// Scale a srcRect by the scaling factors, return as pPrintRect.
/***************************************************************************/
void CSaView::CalcPrintRect(CRect * pPrintRect, const CRect * srcRect)
{
	*pPrintRect = CRect(0,0,(int)(m_printScaleX * srcRect->Width()),
		(int)(m_printScaleY * srcRect->Height()));

}




//*************************************************************************
// CSaView::CalcPrintPoint
//
// Scale a srcPoint by the scaling factors, return as pPrintPoint.
/***************************************************************************/
void CSaView::CalcPrintPoint(CPoint * pPrintPoint, const CPoint * srcPoint)
{
	*pPrintPoint = CPoint((int)(m_printScaleX * srcPoint->x),
		(int)(m_printScaleY * srcPoint->y));
}

/***************************************************************************/
// CSaView::CalculateHiResPrintPages
//
// Calculates how many pages the hi-res print mode will
// take, and checks for an inconsistantcy between the
// number of graphs selected and the number of the number
// of graphs per page.
/***************************************************************************/
int CSaView::CalculateHiResPrintPages(void)
{
	int numGraphs = 0;

	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
	{
		if (m_apGraphs[nLoop])
		{
			numGraphs++;
		}
	}
	numGraphs = min(m_pPageLayout->NumGraphsSelected(),numGraphs);

	if (m_pPageLayout->LayoutType() == m_pPageLayout->LAYOUT_1GRAPH_PER_PAGE()  )
	{
		return numGraphs;
	}
	else if (m_pPageLayout->LayoutType() == m_pPageLayout->LAYOUT_AS_SCREEN())
	{
		return 1;
	}
	else
	{
		int graphsPerPage = m_pPageLayout->NumRows() * m_pPageLayout->NumCols();
		int remainder = numGraphs % graphsPerPage;

		if (!remainder)
		{
			return (numGraphs / graphsPerPage);
		}
	}

	return -1; // indicates an error
}

// SDM 1.5Test10.8 add error messages
/***************************************************************************/
// CSaView::PreparePrintingForScreenShot
//
// Takes a snapshot of the screen and puts it into a CDib so
// that the CDib can be printed later.
//
/***************************************************************************/
void CSaView::PreparePrintingForScreenShot(void)
{
	CWindowDC    scrn(pViewMainFrame);

	scrn.SelectClipRgn(NULL); // select entire window client area
	scrn.GetClipBox(&m_memRect);
	if (!m_memRect.Width() || !m_memRect.Height())
	{
		CSaString szError = "printing area empty, nothing to print";
		pSaApp->ErrorMessage(szError);
		return;
	}

	// create a temporary DC for the reading the screen
	CDC * pMemDC = new CDC;
	if (!pMemDC)
	{
		CSaString szError = "unable to allocate memory for screen shot printing";
		pSaApp->ErrorMessage(szError);
		ASSERT(0);
	}
	else if (!pMemDC->CreateCompatibleDC(&scrn))
	{
		CSaString szError = "unable to allocate memory for screen shot printing";
		pSaApp->ErrorMessage(szError);
		ASSERT(0);
		delete pMemDC;
	}
	else
	{
		// create a bitmap to read the screen into and select it
		// into the temporary DC
		CBitmap * pBitmapForPrint = new CBitmap;

		if (!pBitmapForPrint)
		{
			CSaString szError = "unable to allocate memory for screen shot printing";
			pSaApp->ErrorMessage(szError);
			ASSERT(0);
		}
		else if (!pBitmapForPrint->CreateCompatibleBitmap(&scrn, m_memRect.Width(), m_memRect.Height()))
		{
			CSaString szError = "unable to allocate memory for screen shot printing";
			pSaApp->ErrorMessage(szError);
			ASSERT(0);
			delete pBitmapForPrint;
		}
		else
		{
			CBitmap * oldBitmap = (CBitmap *)pMemDC->SelectObject(pBitmapForPrint);

			if (!oldBitmap)
			{
				CSaString szError = "unable to select bitmap for screen shot printing";
				pSaApp->ErrorMessage(szError);
				ASSERT(0);
			}
			else
			{
				// BitBlt the screen data into the bitmap
				if (!pMemDC->BitBlt(0,0, m_memRect.Width(), m_memRect.Height(), &scrn,
					m_memRect.left,    m_memRect.top, SRCCOPY))
				{
					CSaString szError = "unable to copy for screen shot printing";
					pSaApp->ErrorMessage(szError);
					ASSERT(0);
					m_pCDibForPrint = NULL;
				}
				else
				{
					// create a device independent bitmap from the regular bitmap
					m_pCDibForPrint = new CDib(pMemDC, 8, FALSE);  // 8-bit screen shot
					if (!m_pCDibForPrint)
					{
						CSaString szError = "unable to convert for screen shot printing";
						pSaApp->ErrorMessage(szError);
						ASSERT(m_pCDibForPrint);
					}
				}
				pMemDC->SelectObject((HBITMAP)oldBitmap->GetSafeHandle());
			}
			delete pBitmapForPrint;
		}
		delete pMemDC;
	}
}

/***************************************************************************/
// CSaView::SaDoPreparePrinting - based on MFC base code DoPreparePrinting,
// but modified slightly to allow us to set the print orientation (portrait/landscape
/***************************************************************************/
BOOL CSaView::SaDoPreparePrinting(CPrintInfo* pInfo, BOOL isLandscape)
{
	ASSERT(pInfo != NULL);
	ASSERT(pInfo->m_pPD != NULL);

	if (pInfo->m_bPreview)
	{
		// if preview, get default printer DC and create DC without calling
		//   print dialog.
		if ((!pSaApp->SaGetPrinterDeviceDefaults(&pInfo->m_pPD->m_pd, isLandscape)) &&
			(pSaApp->DoPrintDialog(pInfo->m_pPD) != IDOK))
			// bring up dialog to alert the user they need to install a printer.
		{
			return FALSE;
		}

		if ((pInfo->m_pPD->m_pd.hDC == NULL) &&
			(pInfo->m_pPD->CreatePrinterDC() == NULL))
			// call CreatePrinterDC if DC was not created by above
		{
			return FALSE;
		}

		// set up From and To page range from Min and Max
		pInfo->m_pPD->m_pd.nFromPage = (WORD)pInfo->GetMinPage();
		pInfo->m_pPD->m_pd.nToPage = (WORD)pInfo->GetMaxPage();

	}
	else
	{
		// otherwise, bring up the print dialog and allow user to change things

		// preset From-To range same as Min-Max range
		pInfo->m_pPD->m_pd.nFromPage = (WORD)pInfo->GetMinPage();
		pInfo->m_pPD->m_pd.nToPage = (WORD)pInfo->GetMaxPage();

		if (pSaApp->SaDoPrintDialog(pInfo->m_pPD, isLandscape) != IDOK)
			return FALSE;       // do not print
	}

	ASSERT(pInfo->m_pPD != NULL);
	ASSERT(pInfo->m_pPD->m_pd.hDC != NULL);

	pInfo->m_nNumPreviewPages = pSaApp->m_nNumPreviewPages;
	VERIFY(pInfo->m_strPageDesc.LoadString(AFX_IDS_PREVIEWPAGEDESC));
	return TRUE;
}

/***************************************************************************/
/*************************CUT/COPY/PASTE*************************************/
/***************************************************************************/

/***************************************************************************/
// CSaView::IsCutAllowed Checks, if cut possible (return TRUE)
/***************************************************************************/
BOOL CSaView::IsCutAllowed()
{
	BOOL bHighLighted = FALSE;
	if ((m_nFocusedID == IDD_RAWDATA) && !GetDocument()->GetWbProcess() && !GetDocument()->IsStereo() &&
		(m_pFocusedGraph->GetPlot()->GetHighLightLength() > 0))
	{
		bHighLighted = TRUE;
	}
	return (IsAnyAnnotationSelected() || bHighLighted);
}

/***************************************************************************/
// CSaView::OnEditCopy
/***************************************************************************/
void CSaView::OnEditCopy()
{
	int nSegment = FindSelectedAnnotationIndex();

	if (!IsCutAllowed()) // copy data point
	{
		OnEditCopyMeasurements();
		return;
	}

	if (nSegment != -1)
	{
		CSaString ctext(GetSelectedAnnotationString());
		HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, (ctext.GetLength() + 1)*sizeof(TCHAR));
		ASSERT(hData);
		if (hData)
		{
			LPTSTR lpData = (TCHAR *)GlobalLock(hData);
			ASSERT(lpData);
			if (lpData)
			{
				int len = ctext.GetLength();
				TCHAR * str = ctext.GetBuffer(len+1);
				ASSERT(str);
				if(nSegment == GLOSS)
				{
					str++; len--;
				}
				memcpy(lpData, str, len*sizeof(TCHAR));
				lpData[len] = 0;
				ctext.ReleaseBuffer();
				GlobalUnlock(hData);

				// Clear the current contents of the clipboard, and set
				// the data handle to the new string.
				if (OpenClipboard())
				{
					EmptyClipboard();
					SetClipboardData(CF_UNICODETEXT, hData);
					CloseClipboard();
				}
			}
		}
	}
	else
	{
		if ((m_nFocusedID == IDD_RAWDATA) && (m_pFocusedGraph->GetPlot()->GetHighLightLength() > 0))
		{
			// get the wave section boundaries
			DWORD dwSectionStart = m_pFocusedGraph->GetPlot()->GetHighLightPosition();
			DWORD dwSectionLength = m_pFocusedGraph->GetPlot()->GetHighLightLength();
			((CSaDoc*)GetDocument())->PutWaveToClipboard(dwSectionStart, dwSectionLength);
		}
	}
}

/***************************************************************************/
// CSaView::OnEditCopyMeasurements
/***************************************************************************/
void CSaView::OnEditCopyMeasurements()
{
	// get info from the document
	DWORD dwOffset = GetStartCursorPosition();
	BOOL bSection = (m_nFocusedID != IDD_MELOGRAM)
		&& (m_nFocusedID != IDD_RECORDING)
		&& (m_pFocusedGraph->GetPlot()->GetHighLightLength() > 0);
	DWORD dwSectionStart = m_pFocusedGraph->GetPlot()->GetHighLightPosition();
	DWORD dwSectionLength = m_pFocusedGraph->GetPlot()->GetHighLightLength();
	BOOL pbRes;
	CSaDoc* pDoc = GetDocument();

	CSaString ctext;
	if (bSection)
		ctext = pDoc->GetMeasurementsString(dwSectionStart, dwSectionLength, &pbRes);
	else
		ctext = pDoc->GetMeasurementsString(dwOffset, 0, &pbRes);

	int len = ctext.GetLength();

	// clear the clipboard and pass the new text
	HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, (len + 1)*sizeof(TCHAR));
	ASSERT(hData);
	if (hData)
	{
		LPTSTR lpData = (TCHAR *)GlobalLock(hData);
		ASSERT(lpData);
		if (lpData)
		{
			TCHAR * str = ctext.GetBuffer(len+1);
			ASSERT(str);
			memcpy(lpData, str, len*sizeof(TCHAR));
			lpData[len] = 0;
			ctext.ReleaseBuffer();
			GlobalUnlock(hData);

			if (OpenClipboard())
			{
				EmptyClipboard();
				SetClipboardData(CF_UNICODETEXT, hData);
				CloseClipboard();
			}
		}
	}

	return;
}

/***************************************************************************/
// CSaView::OnEditCut
/***************************************************************************/
void CSaView::OnEditCut()
{
	if (IsAnyAnnotationSelected())
	{
		OnEditCopy();
		RemoveSelectedAnnotation();
	}
	else
	{
		if ((m_nFocusedID == IDD_RAWDATA) && (m_pFocusedGraph->GetPlot()->GetHighLightLength() > 0))
		{
			// copy a wavefile section to the clipboard and delete it from the file
			// get the wave section boundaries
			DWORD dwSectionStart = m_pFocusedGraph->GetPlot()->GetHighLightPosition();
			DWORD dwSectionLength = m_pFocusedGraph->GetPlot()->GetHighLightLength();
			CSaDoc *pDoc = (CSaDoc*)GetDocument();
			if (pDoc->PutWaveToClipboard(dwSectionStart, dwSectionLength, TRUE))
			{
				pDoc->InvalidateAllProcesses();
				RefreshGraphs();
				m_pFocusedGraph->GetPlot()->SetHighLightArea(0, 0);
			}
		}
	}
}



/***************************************************************************/
// CSaView::OnEditPaste
/***************************************************************************/
void CSaView::OnEditPaste()
{
	CSaDoc* pDoc = (CSaDoc*)GetDocument(); // get pointer to document
	if (IsAnyAnnotationSelected() && OpenClipboard())
	{
		HGLOBAL hClipData = NULL;
		LPTSTR lpClipData = NULL;

		// get text from the clipboard
		if (NULL!=(hClipData = GetClipboardData(CF_UNICODETEXT)))
		{
			if (NULL!=(lpClipData = (LPTSTR)GlobalLock(hClipData)))
			{
				CSaString data(lpClipData);
				ASelection().SetSelectedAnnotationString(this, data, FALSE, TRUE);
				GlobalUnlock(hClipData);
				RefreshGraphs();
			}
		}

		CloseClipboard();
	}
	if (m_nFocusedID == IDD_RAWDATA)
	{
		if ((IsClipboardFormatAvailable(CF_WAVE)) &&
			(pDoc->PasteClipboardToWave(GetStartCursorPosition())))
		{
			// get wave from the clipboard
			pDoc->InvalidateAllProcesses();
			RefreshGraphs();
			m_pFocusedGraph->GetPlot()->SetHighLightArea(0, 0);
		}
	}
}

/***************************************************************************/
// CSaView::OnEditPasteNew
/***************************************************************************/
void CSaView::OnEditPasteNew()
{
	if (OpenClipboard())
	{
		if (IsClipboardFormatAvailable(CF_WAVE))
		{
			// call the application to create a new file and put in the clipboard contents
			((CSaApp*)pSaApp)->PasteClipboardToNewFile(GetClipboardData(CF_WAVE));
		}
		CloseClipboard();
	}
}

/***************************************************************************/
// CSaView::IsAnyAnnotationSelected
// Returns TRUE if something is selected in some annotation window, else FALSE.
/***************************************************************************/
BOOL CSaView::IsAnyAnnotationSelected(void)
{
	return (FindSelectedAnnotation() != NULL);
}

/***************************************************************************/
// CSaView::GetSelectedAnnotationString
/***************************************************************************/
CSaString CSaView::GetSelectedAnnotationString(void)
{
	CSaString ret;

	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
	{
		CSegment* pSegments = GetAnnotation(nLoop);
		if (pSegments->GetSelection() != -1)
		{
			int selection = pSegments->GetSelection();
			ret = pSegments->GetSegmentString(selection);
			break;
		}
	}

	return ret;
}

/***************************************************************************/
// CSaView::FindSelectedAnnotation
/***************************************************************************/
CSegment * CSaView::FindSelectedAnnotation() {

	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		CSegment* pSegments = GetAnnotation(nLoop);
		if ((pSegments!=NULL) && (pSegments->GetSelection() != -1)) 		{
			return pSegments;
		}
	}
	return NULL;
}



/***************************************************************************/
// CSaView::FindSelectedAnnotation
/***************************************************************************/
int CSaView::FindSelectedAnnotationIndex()
{
	int ret = -1;

	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
	{
		CSegment* pSegments = GetAnnotation(nLoop);
		if (pSegments && (pSegments->GetSelection() != -1))
		{
			ret = nLoop;
			break;
		}
	}

	return ret;
}



/***************************************************************************/
// CSaView::ChangeSelectedAnnotationData
/***************************************************************************/
void CSaView::ChangeSelectedAnnotationData(const CSaString & str)
{
	CSaDoc* pDoc = (CSaDoc*) GetDocument(); // get pointer to document
	ASSERT(pDoc);
	pDoc->CheckPoint();

	{
		CSegment * pAnnotationSet = FindSelectedAnnotation();
		ASSERT(pAnnotationSet);

		if (pAnnotationSet)
		{
			pAnnotationSet->ReplaceSelectedSegment(pDoc,str);
		}
	}
}



/***************************************************************************/
// CSaView::RemoveSelectedAnnotation
/***************************************************************************/
void CSaView::RemoveSelectedAnnotation()
{
	CSegment * pAnnotationSet = FindSelectedAnnotation();

	if (pAnnotationSet)
	{
		pAnnotationSet->Remove(GetDocument());
	}
}



/***************************************************************************/
// CSaView::OnUpdateEditPaste
// If something for the clipboard is selected, it enables the item.
/***************************************************************************/
void CSaView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	BOOL enablePaste = FALSE;

	if (IsAnyAnnotationSelected())
	{
		if (OpenClipboard())
		{
			if (IsClipboardFormatAvailable(CF_UNICODETEXT))
			{
				enablePaste = TRUE;
			}
			CloseClipboard();
		}
	}
	if ((m_nFocusedID == IDD_RAWDATA) 
		&& !((CSaDoc*)GetDocument())->IsStereo()
		&& !((CSaDoc*)GetDocument())->GetWbProcess())
	{
		if ((OpenClipboard()) &&
			(IsClipboardFormatAvailable(CF_WAVE)))
		{
			enablePaste = TRUE;
		}

		CloseClipboard();
	}
	pCmdUI->Enable(enablePaste);
}

/***************************************************************************/
// CSaView::OnUpdateEditPasteNew
// If waveform avaiable on the clipboard, it enables the item.
/***************************************************************************/
void CSaView::OnUpdateEditPasteNew(CCmdUI* pCmdUI)
{
	BOOL enablePaste = FALSE;
	if ((OpenClipboard()) &&
		(IsClipboardFormatAvailable(CF_WAVE)))
	{
		enablePaste = TRUE;
	}

	CloseClipboard();
	pCmdUI->Enable(enablePaste);
}

/***************************************************************************/
// CSaView::OnUpdateHasSel
// Is something for the clipboard is selected, it enables the item.
/***************************************************************************/
void CSaView::OnUpdateHasSel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsCutAllowed());
}

/***************************************************************************/
/************************UNDO/REDO******************************************/
/***************************************************************************/

/***************************************************************************/
// CSaView::OnEditUndo
/***************************************************************************/
void CSaView::OnEditUndo()
{
	CSaDoc* pDoc = (CSaDoc*)GetDocument();
	pDoc->Undo();
	if (pDoc->IsWaveToUndo())
		pDoc->UndoWaveFile();
	// SDM 1.06.6U4 Set cursors to selected segment on undo
	CSegment* pSegment = FindSelectedAnnotation();
	if (pSegment)
	{
		int nIndex = pSegment->GetSelection();
		SetStartCursorPosition(pSegment->GetOffset(nIndex));
		SetStopCursorPosition(pSegment->GetStop(nIndex));
	}
	RefreshGraphs();
}


/***************************************************************************/
// CSaView::OnUpdateEditUndo
/***************************************************************************/
void CSaView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(((CSaDoc *)GetDocument())->CanUndo());
}



/***************************************************************************/
// CSaView::OnEditRedo
/***************************************************************************/
void CSaView::OnEditRedo()
{
	((CSaDoc *)GetDocument())->Redo();
	RefreshGraphs();
}


/***************************************************************************/
// CSaView::OnUpdateEditRedo
/***************************************************************************/
void CSaView::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(((CSaDoc *)GetDocument())->CanRedo());
}

/***************************************************************************/
// CSaView::MoveStartCursorRight Move Start Cursor Right
/***************************************************************************/
void CSaView::MoveStartCursorRight()
{
	OnEditCursorStartRight();
}

/***************************************************************************/
// CSaView::MoveStartCursorLeft Move Start Cursor Right
/***************************************************************************/
void CSaView::MoveStartCursorLeft()
{
	OnEditCursorStartLeft();
}

/***************************************************************************/
// CSaView::OnEditCursorStartRight Move Start Cursor Right
/***************************************************************************/
void CSaView::OnEditCursorStartRight()
{
	DWORD dwOffset;
	DWORD dwOffsetNew;
	DWORD dwStop;
	DWORD dwStopNew;
	CSaDoc* pDoc = GetDocument();
	int nBlockAlign = pDoc->GetFmtParm()->wBlockAlign;
	DWORD minSeparation = ((DWORD) (CURSOR_MIN_DISTANCE * (m_fMaxZoom/m_fZoom))) * nBlockAlign;
	DWORD dataSize = pDoc->GetDataSize();
	DWORD movementScale = ((DWORD)(m_fMaxZoom/m_fZoom + 0.5))* nBlockAlign;


	dwOffset = GetStartCursorPosition();

	if (dwOffset + movementScale >= dataSize)
		return;

	dwOffsetNew = pDoc->SnapCursor(START_CURSOR, dwOffset + movementScale, SNAP_RIGHT);
	dwStop = GetStopCursorPosition();
	dwStopNew = dwStop;
	if (dwOffsetNew + minSeparation > dwStopNew)
		dwStopNew = pDoc->SnapCursor(STOP_CURSOR, dwOffsetNew + minSeparation, SNAP_RIGHT);
	if (dwOffsetNew + minSeparation > dwStopNew)
		dwOffsetNew = pDoc->SnapCursor(START_CURSOR, (dwStopNew > minSeparation) ? (dwStopNew - minSeparation): 0, SNAP_LEFT);

	if (dwOffsetNew + minSeparation > dwStopNew)
		return;

	if (dwStop != dwStopNew)
		SetStopCursorPosition(dwStopNew);

	if (dwOffset != dwOffsetNew)
		SetStartCursorPosition(dwOffsetNew);
}

/***************************************************************************/
// CSaView::OnEditCursorStopRight Move Stop Cursor Right
/***************************************************************************/
void CSaView::OnEditCursorStopRight()
{
	DWORD dwStop;
	DWORD dwStopNew;
	CSaDoc* pDoc = GetDocument();
	int nBlockAlign = pDoc->GetFmtParm()->wBlockAlign;
	DWORD dataSize = pDoc->GetDataSize();
	DWORD movementScale = ((DWORD)(m_fMaxZoom/m_fZoom + 0.5))* nBlockAlign;


	dwStop = GetStopCursorPosition();

	if(dwStop + movementScale >= dataSize)
		return;

	dwStopNew = pDoc->SnapCursor(STOP_CURSOR, dwStop + movementScale, SNAP_RIGHT);

	if(dwStop != dwStopNew)
		SetStopCursorPosition(dwStopNew);
}

/***************************************************************************/
// CSaView::OnEditCursorStartLeft Move Start Cursor Left
/***************************************************************************/
void CSaView::OnEditCursorStartLeft()
{
	DWORD dwOffset;
	DWORD dwOffsetNew;
	CSaDoc* pDoc = GetDocument();
	int nBlockAlign = pDoc->GetFmtParm()->wBlockAlign;
	DWORD movementScale = ((DWORD)(m_fMaxZoom/m_fZoom + 0.5))* nBlockAlign;


	dwOffset = GetStartCursorPosition();

	if(dwOffset < movementScale)
		return;

	dwOffsetNew = pDoc->SnapCursor(START_CURSOR, dwOffset - movementScale, SNAP_LEFT);

	if(dwOffset != dwOffsetNew)
		SetStartCursorPosition(dwOffsetNew);
}

/***************************************************************************/
// CSaView::OnEditCursorStopLeft Move Stop Cursor Left
/***************************************************************************/
void CSaView::OnEditCursorStopLeft()
{
	DWORD dwOffset;
	DWORD dwOffsetNew;
	DWORD dwStop;
	DWORD dwStopNew;
	CSaDoc* pDoc = GetDocument();
	int nBlockAlign = pDoc->GetFmtParm()->wBlockAlign;
	DWORD minSeparation = ((DWORD) (CURSOR_MIN_DISTANCE * (m_fMaxZoom/m_fZoom))) * nBlockAlign;
	DWORD movementScale = ((DWORD)(m_fMaxZoom/m_fZoom + 0.5))* nBlockAlign;


	dwStop = GetStopCursorPosition();

	if(dwStop < movementScale)
		return;

	dwStopNew = pDoc->SnapCursor(STOP_CURSOR, dwStop - movementScale, SNAP_LEFT);

	dwOffset = GetStartCursorPosition();
	dwOffsetNew = dwOffset;
	if (dwOffsetNew + minSeparation > dwStopNew)
		dwOffsetNew = pDoc->SnapCursor(START_CURSOR, (dwStopNew > minSeparation) ? (dwStopNew - minSeparation): 0, SNAP_LEFT);
	if (dwOffsetNew + minSeparation > dwStopNew)
		dwStopNew = pDoc->SnapCursor(STOP_CURSOR, dwOffsetNew + minSeparation, SNAP_RIGHT);

	if (dwOffsetNew + minSeparation > dwStopNew)
		return;

	if (dwOffset != dwOffsetNew)
		SetStartCursorPosition(dwOffsetNew);

	if (dwStop != dwStopNew)
		SetStopCursorPosition(dwStopNew);
}

//SDM 1.06.6U2
/***************************************************************************/
// CSaView::OnEditAddSyllable Add Syllable Break to Phonetic Segment
/***************************************************************************/
void CSaView::OnEditAddSyllable()
{
	CSaDoc* pDoc = GetDocument(); // get pointer to document
	pDoc->CheckPoint();
	CSaString szString = "."; //Fill new segment with segment break character

	CPhoneticSegment* pSeg = (CPhoneticSegment*) GetAnnotation(PHONETIC);

	if (pSeg->GetSelection()!=-1) // Phonetic Segment Selected
	{
		int nSelection = pSeg->GetSelection();
		DWORD dwStart = pSeg->GetOffset(nSelection); // Start at current start
		DWORD dwMaxStop;
		DWORD dwStop;

		dwMaxStop = dwStart + pSeg->GetDuration(nSelection) - pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME);

		if (pDoc->GetFmtParm()->wBlockAlign == 2)// SDM 1.5Test8.2
		{
			dwMaxStop = dwMaxStop & ~1; // Round down
		}

		// Snap Start Position
		dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);

		dwStop = (dwStart + pDoc->GetBytesFromTime(ADD_SYLLABLE_TIME));

		if (pDoc->GetFmtParm()->wBlockAlign == 2) // SDM 1.5Test8.2
		{
			dwStop = (dwStop + 1) & ~1; // Round up
		}

		dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStop, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);

		if (dwStop <= dwMaxStop) // enough room
		{
			pSeg->Adjust(pDoc,nSelection,dwStop,pSeg->GetDuration(nSelection)+dwStop-pSeg->GetOffset(nSelection));
			pSeg->Insert(nSelection, &szString, '#', dwStart,dwStop - dwStart);
			pDoc->SetModifiedFlag(TRUE); // document has been modified
			pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
			pSeg->SetSelection(-1);
			ASelection().SelectFromPosition(this, PHONETIC, dwStart, CASegmentSelection::FIND_EXACT);
			RefreshGraphs(TRUE);
		}
	}
	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i != -1) && m_apGraphs[i])
	{
		int nAnnot = PHONETIC;
		m_apGraphs[i]->ShowAnnotation(nAnnot, TRUE, TRUE);
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditAddSyllable
/***************************************************************************/
void CSaView::OnUpdateEditAddSyllable(CCmdUI* pCmdUI)
{
	CSaDoc* pDoc = GetDocument(); // get pointer to document
	BOOL bEnable = FALSE;

	CPhoneticSegment* pSeg = (CPhoneticSegment*) GetAnnotation(PHONETIC);

	if (pSeg->GetSelection()!=-1) // Phonetic Segment Selected
	{
		int nSelection = pSeg->GetSelection();
		DWORD dwStart = pSeg->GetOffset(nSelection); // Start at current start
		DWORD dwMaxStop;
		DWORD dwStop;

		dwMaxStop = dwStart + pSeg->GetDuration(nSelection) - pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME);

		if (pDoc->GetFmtParm()->wBlockAlign == 2)// SDM 1.5Test8.2
		{
			dwMaxStop = dwMaxStop & ~1; // Round down
		}

		// Snap Start Position
		dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);

		dwStop = (dwStart + pDoc->GetBytesFromTime(ADD_SYLLABLE_TIME));

		if (pDoc->GetFmtParm()->wBlockAlign == 2)// SDM 1.5Test8.2
		{
			dwStop = (dwStop + 1) & ~1; // Round up
		}

		dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStop, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);

		if (dwStop <= dwMaxStop) // enough room
		{
			bEnable=TRUE;
		}
	}
	pCmdUI->Enable(bEnable);
}


//SDM 1.5Test11.3
/***************************************************************************/
// CSaView::OnEditAdd Add Phonetic Segment
/***************************************************************************/
void CSaView::OnEditAdd()
{
	CSaDoc* pDoc = (CSaDoc*) GetDocument();
	CPhoneticSegment* pSeg = (CPhoneticSegment*) GetAnnotation(PHONETIC);
	CGlossSegment* pGloss = (CGlossSegment*) GetAnnotation(GLOSS);

	pDoc->CheckPoint();
	CSaString szString = SEGMENT_DEFAULT_CHAR; //Fill new segment with default character

	int nInsertAt = pSeg->CheckPosition(pDoc,GetStartCursorPosition(),GetStopCursorPosition(),CSegment::MODE_ADD);
	if (nInsertAt != -1)
	{
		int nPrevious = pSeg->GetPrevious(nInsertAt);
		if ((nPrevious != -1) &&
			(pSeg->GetStop(nPrevious) + pDoc->GetBytesFromTime(MAX_ADD_JOIN_TIME)> GetStartCursorPosition()))
		{
			pSeg->Adjust(pDoc, nPrevious,pSeg->GetOffset(nPrevious),GetStartCursorPosition() - pSeg->GetOffset(nPrevious));
		}

		int nNext  = -1;
		if (nInsertAt > 0)
			nNext = pSeg->GetNext(nInsertAt - 1);
		else if (!pSeg->IsEmpty())
			nNext = nInsertAt;
		if ((nNext != -1) &&
			(pSeg->GetOffset(nNext) < GetStopCursorPosition()+pDoc->GetBytesFromTime(MAX_ADD_JOIN_TIME)))
		{
			pSeg->Adjust(pDoc, nNext,GetStopCursorPosition(),pSeg->GetStop(nNext)-GetStopCursorPosition());
		}

		pSeg->Insert(nInsertAt, &szString, '#', GetStartCursorPosition(),GetStopCursorPosition()-GetStartCursorPosition());
		// Adjust Gloss
		if ((!pGloss->IsEmpty()) && pSeg->GetPrevious(nInsertAt))
		{
			int nPrevious = pSeg->GetPrevious(nInsertAt);
			int nIndex = pGloss->FindStop(pSeg->GetStop(nPrevious));
			if (nIndex != -1)
				pGloss->Adjust(pDoc, nIndex, pGloss->GetOffset(nIndex), pGloss->CalculateDuration(pDoc,nIndex));
		}
		pDoc->SetModifiedFlag(TRUE); // document has been modified
		pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
		RefreshGraphs(TRUE);
		pSeg->SetSelection(-1);
		ASelection().SelectFromPosition(this, PHONETIC, GetStartCursorPosition(), CASegmentSelection::FIND_EXACT);
	}
	else  // Can we insert after selected segment
	{
		if (pSeg->GetSelection()!=-1) // Phonetic Segment Selected
		{
			int nSelection = pSeg->GetSelection();
			DWORD dwStart = pSeg->GetStop(nSelection); // Start at current stop
			DWORD dwMaxStop;
			DWORD dwStop;

			if (pSeg->GetNext(nSelection) == -1) // Last Selection
			{
				dwMaxStop = pDoc->GetUnprocessedDataSize();
			}
			else // Fit before next
			{
				dwMaxStop = pSeg->GetOffset(pSeg->GetNext(nSelection));
			}

			// Snap Start Position
			dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);

			dwStop = (dwStart + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));

			if (pDoc->GetFmtParm()->wBlockAlign == 2)// SDM 1.5Test8.2
			{
				dwStop = (dwStop + 1) & ~1; // Round up
			}

			if (pSeg->GetNext(nSelection) != -1)
				dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStop, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);

			if (dwStop <= dwMaxStop) // enough room
			{
				dwStop = dwStart + pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME);

				if (pDoc->GetFmtParm()->wBlockAlign == 2)// SDM 1.5Test8.2
				{
					dwStop = (dwStop + 1) & ~1; // Round up
				}

				if (dwStop > dwMaxStop)
					dwStop = dwMaxStop;
				else
					// Snap Stop Position
					dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStart, dwMaxStop, SNAP_LEFT);

				nInsertAt = pSeg->CheckPosition(pDoc,dwStart,dwStop,CSegment::MODE_ADD);
				ASSERT(nInsertAt >= 0);
				pSeg->Insert(nInsertAt, &szString, '#', dwStart,dwStop - dwStart);
				// Adjust Gloss
				if ((!pGloss->IsEmpty()) && pSeg->GetPrevious(nInsertAt))
				{
					int nPrevious = pSeg->GetPrevious(nInsertAt);
					int nIndex = pGloss->FindStop(pSeg->GetStop(nPrevious));
					if (nIndex != -1)
						pGloss->Adjust(pDoc, nIndex, pGloss->GetOffset(nIndex), pGloss->CalculateDuration(pDoc,nIndex));
				}
				pDoc->SetModifiedFlag(TRUE); // document has been modified
				pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
				RefreshGraphs(TRUE);
				pSeg->SetSelection(-1);
				ASelection().SelectFromPosition(this, PHONETIC, dwStart, CASegmentSelection::FIND_EXACT);
			}
		}
	}

	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i != -1) && m_apGraphs[i])
	{
		int nAnnot = PHONETIC;
		m_apGraphs[i]->ShowAnnotation(nAnnot, TRUE, TRUE);
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditAdd
/***************************************************************************/
void CSaView::OnUpdateEditAdd(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	CSaDoc* pDoc = GetDocument(); // get pointer to document

	CPhoneticSegment* pSeg = (CPhoneticSegment*) GetAnnotation(PHONETIC);
	int nInsertAt = pSeg->CheckPosition(pDoc,GetStartCursorPosition(),GetStopCursorPosition(),CSegment::MODE_ADD);
	if (nInsertAt != -1)
	{
		ASelection().Update(this);
		int nLoop = ASelection().GetSelection().nAnnotationIndex;
		if(nLoop == -1)
			bEnable = TRUE;
	}
	else
	{
		if (pSeg->GetSelection()!=-1) // Phonetic Segment Selected
		{
			int nSelection = pSeg->GetSelection();
			DWORD dwStart = pSeg->GetStop(nSelection); // Start at current stop
			DWORD dwMaxStop;
			DWORD dwStop;

			if (pSeg->GetNext(nSelection) == -1) // Last Selection
			{
				dwMaxStop = pDoc->GetUnprocessedDataSize();
			}
			else // Fit before next
			{
				dwMaxStop = pSeg->GetOffset(pSeg->GetNext(nSelection));
			}

			// Snap Start Position
			dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);;

			dwStop = (dwStart + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));
			if (pDoc->GetFmtParm()->wBlockAlign == 2)// SDM 1.5Test8.2
			{
				dwStop = (dwStop + 1) & ~1; // Round up
			}

			if (pSeg->GetNext(nSelection) != -1)
				dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStop, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);

			if (dwStop <= dwMaxStop) // enough room
			{
				dwStop = dwStart + pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME);

				if (pDoc->GetFmtParm()->wBlockAlign == 2)// SDM 1.5Test8.2
				{
					dwStop = (dwStop + 1) & ~1; // Round up
				}

				if (dwStop > dwMaxStop)
					dwStop = dwMaxStop;
				else
					// Snap Stop Position
					dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStart, dwMaxStop, SNAP_LEFT);

				nInsertAt = pSeg->CheckPosition(pDoc,dwStart,dwStop,CSegment::MODE_ADD);
				if(nInsertAt >= 0)
					bEnable = TRUE;
			}
		}
	}
	pCmdUI->Enable(bEnable);
}


/***************************************************************************/
// CSaView::OnEditAddPhrase
/***************************************************************************/
void CSaView::OnEditAddPhrase(CMusicPhraseSegment *pSeg)
{
	CSaDoc* pDoc = (CSaDoc*) GetDocument();

	pDoc->CheckPoint();
	CSaString szString = SEGMENT_DEFAULT_CHAR; //Fill new segment with default character

	int nInsertAt = pSeg->CheckPosition(pDoc,GetStartCursorPosition(),GetStopCursorPosition(),CSegment::MODE_ADD);
	if(nInsertAt != -1)
	{
		int nPrevious = pSeg->GetPrevious(nInsertAt);
		if(nPrevious != -1)
		{
			if(pSeg->GetStop(nPrevious) + pDoc->GetBytesFromTime(MAX_ADD_JOIN_TIME)> GetStartCursorPosition()) // SDM 1.5Test10.2
				pSeg->Adjust(pDoc, nPrevious,pSeg->GetOffset(nPrevious),GetStartCursorPosition() - pSeg->GetOffset(nPrevious));
		}
		int nNext  = -1;
		if(nInsertAt > 0) nNext = pSeg->GetNext(nInsertAt - 1);
		else if(!pSeg->IsEmpty()) nNext = nInsertAt;
		if(nNext != -1)
		{
			if(pSeg->GetOffset(nNext) < GetStopCursorPosition()+pDoc->GetBytesFromTime(MAX_ADD_JOIN_TIME)) // SDM 1.5Test10.2
				pSeg->Adjust(pDoc, nNext,GetStopCursorPosition(),pSeg->GetStop(nNext)-GetStopCursorPosition());
		}
		pSeg->Insert(nInsertAt, &szString, '#', GetStartCursorPosition(),GetStopCursorPosition()-GetStartCursorPosition());
		pDoc->SetModifiedFlag(TRUE); // document has been modified
		pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
		RefreshGraphs(TRUE);
		pSeg->SetSelection(-1);
		ASelection().SelectFromPosition(this, pSeg->GetAnnotationIndex(), GetStartCursorPosition(), CASegmentSelection::FIND_EXACT);
	}
	else  // Can we insert after selected segment
	{
		if(pSeg->GetSelection()!=-1) // Phonetic Segment Selected
		{
			int nSelection = pSeg->GetSelection();
			DWORD dwStart = pSeg->GetStop(nSelection); // Start at current stop
			DWORD dwMaxStop;
			DWORD dwStop;

			if(pSeg->GetNext(nSelection) == -1) // Last Selection
			{
				dwMaxStop = pDoc->GetUnprocessedDataSize();
			}
			else // Fit before next
			{
				dwMaxStop = pSeg->GetOffset(pSeg->GetNext(nSelection));
			}

			// Snap Start Position
			dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);

			dwStop = (dwStart + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));

			if(pDoc->GetFmtParm()->wBlockAlign == 2)// SDM 1.5Test8.2
			{
				dwStop = (dwStop + 1) & ~1; // Round up
			}

			if(pSeg->GetNext(nSelection) != -1)
				dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStop, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);

			if(dwStop <= dwMaxStop) // enough room
			{
				dwStop = dwStart + pDoc->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME);

				if(pDoc->GetFmtParm()->wBlockAlign == 2)// SDM 1.5Test8.2
				{
					dwStop = (dwStop + 1) & ~1; // Round up
				}

				if (dwStop > dwMaxStop)
					dwStop = dwMaxStop;
				else
					// Snap Stop Position
					dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStart, dwMaxStop, SNAP_LEFT);

				nInsertAt = pSeg->CheckPosition(pDoc,dwStart,dwStop,CSegment::MODE_ADD);
				pSeg->Insert(nInsertAt, &szString, '#', dwStart,dwStop - dwStart);
				pDoc->SetModifiedFlag(TRUE); // document has been modified
				pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
				RefreshGraphs(TRUE);
				pSeg->SetSelection(-1);
				ASelection().SelectFromPosition(this, pSeg->GetAnnotationIndex(), dwStart, CASegmentSelection::FIND_EXACT);
			}
		}
	}

	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i != -1) && m_apGraphs[i])
	{
		int nAnnot = pSeg->GetAnnotationIndex();
		m_apGraphs[i]->ShowAnnotation(nAnnot, TRUE, TRUE);
	}
}


/***************************************************************************/
// CSaView::OnUpdateEditAddPhrase
/***************************************************************************/
void CSaView::OnUpdateEditAddPhrase(CCmdUI* pCmdUI, CMusicPhraseSegment* pSeg)
{
	BOOL bEnable = FALSE;
	CSaDoc* pDoc = GetDocument(); // get pointer to document

	int nInsertAt = pSeg->CheckPosition(pDoc,GetStartCursorPosition(),GetStopCursorPosition(),CSegment::MODE_ADD);
	if(nInsertAt != -1)
		bEnable = TRUE;
	{
		if(pSeg->GetSelection()!=-1) // Phonetic Segment Selected
		{
			int nSelection = pSeg->GetSelection();
			DWORD dwStart = pSeg->GetStop(nSelection); // Start at current stop
			DWORD dwMaxStop;
			DWORD dwStop;

			if(pSeg->GetNext(nSelection) == -1) // Last Selection
			{
				dwMaxStop = pDoc->GetUnprocessedDataSize();
			}
			else // Fit before next
			{
				dwMaxStop = pSeg->GetOffset(pSeg->GetNext(nSelection));
			}

			// Snap Start Position
			dwStart = pDoc->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);

			dwStop = (dwStart + pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));
			if(pDoc->GetFmtParm()->wBlockAlign == 2)// SDM 1.5Test8.2
			{
				dwStop = (dwStop + 1) & ~1; // Round up
			}

			if(pSeg->GetNext(nSelection) != -1)
				dwStop = pDoc->SnapCursor(STOP_CURSOR, dwStop, dwStop, pDoc->GetUnprocessedDataSize(), SNAP_RIGHT);

			if(dwStop <= dwMaxStop) // enough room
			{
				bEnable = TRUE;
			}
		}
	}
	pCmdUI->Enable(bEnable);
}


void CSaView::OnEditAddPhraseL1()
{
	OnEditAddPhrase((CMusicPhraseSegment*) GetAnnotation(MUSIC_PL1));
}

void CSaView::OnEditAddPhraseL2()
{
	OnEditAddPhrase((CMusicPhraseSegment*) GetAnnotation(MUSIC_PL2));
}

void CSaView::OnEditAddPhraseL3()
{
	OnEditAddPhrase((CMusicPhraseSegment*) GetAnnotation(MUSIC_PL3));
}

void CSaView::OnEditAddPhraseL4()
{
	OnEditAddPhrase((CMusicPhraseSegment*) GetAnnotation(MUSIC_PL4));
}

void CSaView::OnUpdateEditAddPhraseL1(CCmdUI* pCmdUI)
{
	OnUpdateEditAddPhrase(pCmdUI, (CMusicPhraseSegment*) GetAnnotation(MUSIC_PL1));
}

void CSaView::OnUpdateEditAddPhraseL2(CCmdUI* pCmdUI)
{
	OnUpdateEditAddPhrase(pCmdUI, (CMusicPhraseSegment*) GetAnnotation(MUSIC_PL2));
}

void CSaView::OnUpdateEditAddPhraseL3(CCmdUI* pCmdUI)
{
	OnUpdateEditAddPhrase(pCmdUI, (CMusicPhraseSegment*) GetAnnotation(MUSIC_PL3));
}

void CSaView::OnUpdateEditAddPhraseL4(CCmdUI* pCmdUI)
{
	OnUpdateEditAddPhrase(pCmdUI, (CMusicPhraseSegment*) GetAnnotation(MUSIC_PL4));
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnEditAddWord Add Gloss Word Segment
/***************************************************************************/
void CSaView::OnEditAddWord()
{
	EditAddGloss(FALSE);
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnEditAddBookmark Add Gloss Bookmark Segment
/***************************************************************************/
void CSaView::OnEditAddBookmark()
{
	EditAddGloss(TRUE);
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::EditAddGloss Add Gloss Segment
/***************************************************************************/
void CSaView::EditAddGloss(int nDelimiter)
{
	CSaDoc* pDoc = (CSaDoc*)GetDocument(); // get pointer to document
	CSaString szString = ""; //Fill new segment with default character

	CTextSegment* pSeg = (CTextSegment*)GetAnnotation(GLOSS);
	int nInsertAt = pSeg->CheckPosition(pDoc,GetStartCursorPosition(),GetStopCursorPosition(),CSegment::MODE_ADD);
	if (nInsertAt != -1)
	{
		DWORD dwStart;

		pSeg->AdjustCursorsToMaster(pDoc, FALSE, &dwStart);

		pSeg->Add(GetDocument(), dwStart, szString, nDelimiter, TRUE); // add a segment
	}
	else
	{
		//SDM 1.5Test8.2
		DWORD dwStart = GetStartCursorPosition();

		int nPos = pSeg->FindFromPosition(dwStart,TRUE);
		if (nPos==-1)
			nPos = pSeg->FindFromPosition(dwStart,FALSE);
		else
			nPos++;

		DWORD dwStop;
		if ((nPos == -1) || (nPos >= pSeg->GetTexts()->GetSize())) dwStop = pDoc->GetUnprocessedDataSize();
		else dwStop = pSeg->GetOffset(nPos);

		nInsertAt = GetAnnotation(PHONETIC)->CheckPosition(pDoc,dwStart,dwStop,CSegment::MODE_ADD);
		if (nInsertAt != -1)
		{
			SetStopCursorPosition(dwStop);
			OnEditAdd();

			pSeg->AdjustCursorsToMaster(pDoc, FALSE, &dwStart);
			pSeg->Add(pDoc, dwStart, szString, nDelimiter, FALSE); // add a segment

			int i = GetGraphIndexForIDD(IDD_RAWDATA);
			if ((i != -1) && m_apGraphs[i])
			{
				int nAnnot = pSeg->GetAnnotationIndex();
				m_apGraphs[i]->ShowAnnotation(nAnnot, TRUE, TRUE);
			}
		}
		else if (pSeg->GetSelection()!=-1) // Set Delimiter
		{
			CSaString szString = GetSelectedAnnotationString();
			if (nDelimiter == TRUE)
				szString = TEXT_DELIMITER + szString.Mid(1);
			else
				szString = WORD_DELIMITER + szString.Mid(1);
			ASelection().Update(this);
			ASelection().SetSelectedAnnotationString(this, szString, TRUE, TRUE);
		}
	}

	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i != -1) && m_apGraphs[i])
	{
		int nAnnot = pSeg->GetAnnotationIndex();
		m_apGraphs[i]->ShowAnnotation(nAnnot, TRUE, TRUE);
	}
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnUpdateEditAddWord Update handler for AddWord
/***************************************************************************/
void CSaView::OnUpdateEditAddWord(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	CSaDoc* pDoc = (CSaDoc*) GetDocument(); // get pointer to document

	CSegment* pSeg = GetAnnotation(GLOSS);
	int nInsertAt = pSeg->CheckPosition(pDoc,GetStartCursorPosition(),GetStopCursorPosition(),CSegment::MODE_ADD);
	if (nInsertAt != -1)
		bEnable = TRUE;
	else
	{
		//SDM 1.5Test8.2
		DWORD dwStart = GetStartCursorPosition();

		int nPos = pSeg->FindFromPosition(dwStart,TRUE);
		if (nPos==-1)
			nPos = pSeg->FindFromPosition(dwStart,FALSE);
		else
			nPos++;

		DWORD dwStop;
		if ((nPos == -1) || (nPos >= pSeg->GetSize()))
			dwStop = pDoc->GetUnprocessedDataSize();
		else dwStop = pSeg->GetOffset(nPos);

		nInsertAt = GetAnnotation(PHONETIC)->CheckPosition(pDoc,dwStart,dwStop,CSegment::MODE_ADD);
		if (nInsertAt != -1)
			bEnable = TRUE;
		else if (pSeg->GetSelection()!=-1) // Set Delimiter
		{
			CSaString szString = GetSelectedAnnotationString();
			if (szString[0] != WORD_DELIMITER)
				bEnable = TRUE;
		}
	}
	pCmdUI->Enable(bEnable);
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnUpdateEditAddBookmark Update handler for AddBookmark
/***************************************************************************/
void CSaView::OnUpdateEditAddBookmark(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	CSaDoc* pDoc = (CSaDoc*) GetDocument(); // get pointer to document

	CSegment* pSeg = GetAnnotation(GLOSS);
	int nInsertAt = pSeg->CheckPosition(pDoc,GetStartCursorPosition(),GetStopCursorPosition(),CSegment::MODE_ADD);
	if (nInsertAt != -1)
		bEnable = TRUE;
	else
	{
		//SDM 1.5Test8.2
		DWORD dwStart = GetStartCursorPosition();

		int nPos = pSeg->FindFromPosition(dwStart,TRUE);
		if (nPos==-1)
			nPos = pSeg->FindFromPosition(dwStart,FALSE);
		else
			nPos++;

		DWORD dwStop;
		if ((nPos == -1) || (nPos >= pSeg->GetSize()))
			dwStop = pDoc->GetUnprocessedDataSize();
		else
			dwStop = pSeg->GetOffset(nPos);

		nInsertAt = GetAnnotation(PHONETIC)->CheckPosition(pDoc, dwStart,dwStop,CSegment::MODE_ADD);
		if (nInsertAt != -1)
			bEnable = TRUE;
		else if (pSeg->GetSelection()!=-1) // Set Delimiter
		{
			CSaString szString = GetSelectedAnnotationString();
			if (szString[0] != TEXT_DELIMITER)
				bEnable = TRUE;
		}
	}
	pCmdUI->Enable(bEnable);
}

/***************************************************************************/
// CSaView::OnEditRemove
/***************************************************************************/
void CSaView::OnEditRemove()
{
	CSegment* pSeg = FindSelectedAnnotation();
	if (pSeg && (pSeg->GetSelection() != -1))
		pSeg->Remove(GetDocument(), TRUE);
}


/***************************************************************************/
// CSaView::OnUpdateEditRemove
/***************************************************************************/
void CSaView::OnUpdateEditRemove(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	CSegment* pSeg = FindSelectedAnnotation();
	if (pSeg && (pSeg->GetSelection() != -1))
		bEnable = TRUE;
	pCmdUI->Enable(bEnable);
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnEditUp Move annotation selection one visible
// window up (or start at bottom left)
/***************************************************************************/
void CSaView::OnEditUp()
{
	int nLoop;
	DWORD dwOffset;

	if (m_pFocusedGraph) // needs to have a focused graph
	{
		ASelection().Update(this);
		nLoop = ASelection().GetSelection().nAnnotationIndex;
		dwOffset = ASelection().GetSelection().dwStart;

		if (nLoop == -1)
		{
			// no selection yet, search for last annotation which is not empty
			for(nLoop=ANNOT_WND_NUMBER - 1;nLoop >= 0;nLoop--)
			{
				if (GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
				{
					dwOffset = GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->GetOffset(0);
					nLoop++;
					break;
				}
			}
			if (nLoop == -1)
				return; // no segments navigation irrelevant
		}
		else // SDM 1.5Test8.1 find window position from segment index
		{
			for(int nIndex = 0; nIndex < ANNOT_WND_NUMBER; nIndex++)
			{
				if(CGraphWnd::m_anAnnWndOrder[nIndex] == nLoop)
				{
					nLoop = nIndex;
					break;
				}
			}
		}

		// search for next upper visible annotation window which is not empty
		for(nLoop--;nLoop >= 0;nLoop--)
		{
			if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop]) && ((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
				||((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS)&&(CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) // SDM 1.5Test8.1
			{
				ASelection().SelectFromPosition(this, CGraphWnd::m_anAnnWndOrder[nLoop], dwOffset, CASegmentSelection::FIND_EXACT);// SDM 1.5Test8.1

				if(ASelection().GetSelection().nAnnotationIndex == -1)
					ASelection().SelectFromPosition(this, CGraphWnd::m_anAnnWndOrder[nLoop], dwOffset);

				if ((ASelection().GetSelection().dwStart <= m_dwDataPosition)
					|| (ASelection().GetSelection().dwStop >= (m_dwDataPosition + GetDataFrame())))
					ZoomIn(0, FALSE); // center selection in view
				break;
			}
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditUp Able to move annotation selection one visible
// window up (or start at bottom left)
/***************************************************************************/
void CSaView::OnUpdateEditUp(CCmdUI* pCmdUI)
{
	int nLoop;

	if (m_pFocusedGraph) // needs to have a focused graph
	{
		ASelection().Update(this);
		nLoop = ASelection().GetSelection().nAnnotationIndex;

		if (nLoop == -1)
		{
			// no selection yet, search for last annotation which is not empty
			for(nLoop=ANNOT_WND_NUMBER - 1;nLoop >= 0;nLoop--)
			{
				if (GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
				{
					nLoop++;
					break;
				}
			}
			if (nLoop == -1)
			{
				pCmdUI->Enable(FALSE);
				return; // no segments navigation irrelevant
			}
		}
		else // SDM 1.5Test8.1 find window position from segment index
		{
			for(int nIndex = 0; nIndex < ANNOT_WND_NUMBER; nIndex++)
			{
				if(CGraphWnd::m_anAnnWndOrder[nIndex] == nLoop)
				{
					nLoop = nIndex;
					break;
				}
			}
		}


		// search for next upper visible annotation window which is not empty
		for(nLoop--;nLoop >= 0;nLoop--)
		{
			if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop]) && ((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
				||((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS)&&(CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC))))// SDM 1.5Test8.1
			{
				pCmdUI->Enable(TRUE);
				return; // possibility found
			}
		}
	}
	pCmdUI->Enable(FALSE);
	return; // no possibility found
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnEditDown Move annotation selection one visible
// window down (or start at top left)
/***************************************************************************/
void CSaView::OnEditDown()
{
	int nLoop;
	DWORD dwOffset;

	if (m_pFocusedGraph) // needs to have a focused graph
	{
		ASelection().Update(this);
		nLoop = ASelection().GetSelection().nAnnotationIndex;
		dwOffset = ASelection().GetSelection().dwStart;

		if (nLoop == -1)
		{
			// no selection yet, search for last annotation which is not empty
			for(nLoop=0;nLoop < ANNOT_WND_NUMBER;nLoop++)
			{
				if (GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
				{
					dwOffset = GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->GetOffset(0);
					nLoop--;
					break;
				}
			}
			if (nLoop == ANNOT_WND_NUMBER)
				return; // no segments navigation irrelevant
		}
		else // SDM 1.5Test8.1 find window position from segment index
		{
			for(int nIndex = 0; nIndex < ANNOT_WND_NUMBER; nIndex++)
			{
				if(CGraphWnd::m_anAnnWndOrder[nIndex] == nLoop)
				{
					nLoop = nIndex;
					break;
				}
			}
		}

		// search for next lower visible annotation window which is not empty
		for(nLoop++;nLoop < ANNOT_WND_NUMBER;nLoop++)
		{
			if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop]) && ((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
				||((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS)&&(CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC))))// SDM 1.5Test8.1
			{
				ASelection().SelectFromPosition(this, CGraphWnd::m_anAnnWndOrder[nLoop], dwOffset, CASegmentSelection::FIND_EXACT);// SDM 1.5Test8.1

				if(ASelection().GetSelection().nAnnotationIndex == -1)
					ASelection().SelectFromPosition(this, CGraphWnd::m_anAnnWndOrder[nLoop], dwOffset);


				if ((ASelection().GetSelection().dwStart <= m_dwDataPosition)
					|| (ASelection().GetSelection().dwStop >= (m_dwDataPosition + GetDataFrame())))
					ZoomIn(0, FALSE); // center selection in view
				break;
			}
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditDown Able to move annotation selection one visible
// window down (or start at top left)
/***************************************************************************/
void CSaView::OnUpdateEditDown(CCmdUI* pCmdUI)
{
	int nLoop;

	if (m_pFocusedGraph) // needs to have a focused graph
	{
		ASelection().Update(this);
		nLoop = ASelection().GetSelection().nAnnotationIndex;

		if (nLoop == -1)
		{
			// no selection yet, search for last annotation which is not empty
			for(nLoop=0;nLoop < ANNOT_WND_NUMBER;nLoop++)
			{
				if (GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
				{
					nLoop--;
					break;
				}
			}
			if (nLoop == ANNOT_WND_NUMBER)
			{
				pCmdUI->Enable(FALSE);
				return; // no segments navigation irrelevant
			}
		}
		else // SDM 1.5Test8.1 find window position from segment index
		{
			for(int nIndex = 0; nIndex < ANNOT_WND_NUMBER; nIndex++)
			{
				if(CGraphWnd::m_anAnnWndOrder[nIndex] == nLoop)
				{
					nLoop = nIndex;
					break;
				}
			}
		}

		// search for next lower visible annotation window which is not empty
		for(nLoop++;nLoop < ANNOT_WND_NUMBER;nLoop++)
		{
			if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])
				&& ((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
				||((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS)&&(CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC))))// SDM 1.5Test8.1
			{
				pCmdUI->Enable(TRUE);
				return; // possibility found
			}
		}
	}
	pCmdUI->Enable(FALSE);
	return; // no possibility found
}

/***************************************************************************/
// CSaView::OnEditPrevious Select previous segment
// Looks for visible annotation window with already selected segment and
// selects the previous segment (if available). If no selection present, it
// selects the last segment in the first visible annotation window. There
// needs to be a graph window with focus.
/***************************************************************************/
void CSaView::OnEditPrevious()
{
	int nLoop;

	if (m_pFocusedGraph) // needs to have a focused graph
	{
		ASelection().Update(this);
		nLoop = ASelection().GetSelection().nAnnotationIndex;
		// only work from visible selections
		if(( nLoop != -1)&& !(m_pFocusedGraph->HaveAnnotation(nLoop) )) nLoop=-1; // only work from visible selections

		int nSelection;
		if (nLoop == -1)
		{
			// no selection yet, search for first visible annotation with segments
			for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
			{
				if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])
					&& ((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
					||((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS)&&(CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) // SDM 1.5Test8.1
				{
					nLoop = CGraphWnd::m_anAnnWndOrder[nLoop];
					break;
				}
			}
			if (nLoop >= ANNOT_WND_NUMBER) return; // no possibility found
			// try to find selection within start cursor or left of it
			ASelection().SelectFromPosition(this, nLoop, GetStartCursorPosition(),CASegmentSelection::FIND_EXACT);
			if(ASelection().GetSelection().nAnnotationIndex == -1)
				ASelection().SelectFromPosition(this, nLoop, GetStartCursorPosition());
		}
		else
		{
			int MASTER = GetAnnotation(nLoop)->GetMasterIndex();
			if((MASTER!=-1)&&(nLoop != GLOSS)&&(nLoop != PHONETIC))
			{
				CSegment* pMaster=GetAnnotation(MASTER);

				DWORD dwPosition = ASelection().GetSelection().dwStart;
				int nIndexMaster = pMaster->FindOffset(dwPosition); // Should always find...
				if(nIndexMaster==-1) nIndexMaster = pMaster->FindFromPosition(dwPosition, TRUE); // Better Coverage

				if(nIndexMaster == -1) return;
				nIndexMaster = pMaster->GetPrevious(nIndexMaster);
				if(nIndexMaster == -1) return;
				ASelection().SelectFromStopPosition(this, nLoop, pMaster->GetStop(nIndexMaster),CASegmentSelection::FIND_EXACT);
				if(ASelection().GetSelection().nAnnotationIndex == -1)
					ASelection().SelectFromStopPosition(this, nLoop, pMaster->GetStop(nIndexMaster));
			}
			else
			{
				DWORD dwStop;
				DWORD dwStart;

				nSelection = GetAnnotation(nLoop)->GetSelection();
				if(nSelection == -1)
				{
					dwStart = ASelection().GetSelection().dwStart;
					nSelection = GetAnnotation(nLoop)->FindFromPosition(dwStart,FALSE);
					if(nSelection == -1)
						return;

					if(GetAnnotation(nLoop)->GetOffset(nSelection) < dwStart)
					{
						dwStop = GetAnnotation(nLoop)->GetStop(nSelection);
					}
					else
					{
						int nPrevious = GetAnnotation(nLoop)->GetPrevious(nSelection);
						if (nPrevious == -1)
							return;
						dwStop = GetAnnotation(nLoop)->GetStop(nPrevious);
					}
				}
				else
				{
					int nPrevious = GetAnnotation(nLoop)->GetPrevious();
					dwStart = GetAnnotation(nLoop)->GetOffset(nSelection);

					if (nPrevious == -1)
					{
						dwStop = 0;
					}
					else
					{
						dwStop = GetAnnotation(nLoop)->GetStop(nPrevious);
					}

					if((nLoop != PHONETIC) && (nLoop != GLOSS) && (dwStop + GetDocument()->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME)) < dwStart)
						dwStop = dwStart - 2;
				}

				if(dwStop < GetDocument()->GetBytesFromTime(MIN_ADD_SEGMENT_TIME))
					return;

				// change the selection
				ASelection().SelectFromStopPosition(this, nLoop, dwStop,CASegmentSelection::FIND_EXACT);
				if(ASelection().GetSelection().nAnnotationIndex == -1)
					ASelection().SelectFromStopPosition(this, nLoop, dwStop);
			}
		}
		// if necessary scroll selection into view
		if ((ASelection().GetSelection().dwStart <= m_dwDataPosition)
			|| (ASelection().GetSelection().dwStop >= (m_dwDataPosition + GetDataFrame())))
			ZoomIn(0, FALSE); // center selection in view
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditPrevious Menu update
/***************************************************************************/
void CSaView::OnUpdateEditPrevious(CCmdUI* pCmdUI)
{
	CSaDoc* pDoc = (CSaDoc*)GetDocument();
	int nLoop;

	BOOL bEnable = FALSE;
	if ((pDoc->GetUnprocessedDataSize() != 0) && GetFocusedGraphWnd()) // needs focused graph
	{
		ASelection().Update(this);
		nLoop = ASelection().GetSelection().nAnnotationIndex;
		// only work from visible selections
		if(( nLoop != -1)&& !(m_pFocusedGraph->HaveAnnotation(nLoop) )) nLoop=-1; // only work from visible selections

		int nSelection;
		if (nLoop == -1)
		{
			// no selection yet, search for first visible annotation with segments
			for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
			{
				if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])
					&& ((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
					||((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS)&&(CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) // SDM 1.5Test8.1
				{
					nLoop = CGraphWnd::m_anAnnWndOrder[nLoop];
					break;
				}
			}
			if (nLoop < ANNOT_WND_NUMBER)
				bEnable = TRUE;
		}
		else
		{
			int MASTER = GetAnnotation(nLoop)->GetMasterIndex();
			if((MASTER!=-1)&&(nLoop != GLOSS)&&(nLoop != PHONETIC))
			{
				CSegment* pMaster=GetAnnotation(MASTER);

				DWORD dwPosition = ASelection().GetSelection().dwStart;
				int nIndexMaster = pMaster->FindOffset(dwPosition); // Should always find...
				if(nIndexMaster==-1) nIndexMaster = pMaster->FindFromPosition(dwPosition, TRUE); // Better Coverage

				if(nIndexMaster != -1)
					nIndexMaster = pMaster->GetPrevious(nIndexMaster);
				if(nIndexMaster != -1)
					bEnable = TRUE;
			}
			else
			{
				DWORD dwStop = 0;
				DWORD dwStart;

				nSelection = GetAnnotation(nLoop)->GetSelection();
				if(nSelection == -1)
				{
					dwStart = ASelection().GetSelection().dwStart;
					nSelection = GetAnnotation(nLoop)->FindFromPosition(dwStart,FALSE);
					if(nSelection != -1)
					{

						int nPrevious = GetAnnotation(nLoop)->GetPrevious(nSelection);
						if (nPrevious != -1)
							dwStop = GetAnnotation(nLoop)->GetStop(nPrevious);
					}
				}
				else
				{
					int nPrevious = GetAnnotation(nLoop)->GetPrevious();
					dwStart = GetAnnotation(nLoop)->GetOffset(nSelection);

					if (nPrevious == -1)
					{
						dwStop = 0;
					}
					else
					{
						dwStop = GetAnnotation(nLoop)->GetStop(nPrevious);
					}

					if((nLoop != PHONETIC) && (nLoop != GLOSS) && (dwStop + GetDocument()->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME)) < dwStart)
						dwStop = dwStart - 2;

				}
				bEnable = TRUE;

				if(dwStop < GetDocument()->GetBytesFromTime(MIN_ADD_SEGMENT_TIME))
					bEnable = FALSE;
			}
		}
	}
	pCmdUI->Enable(bEnable);
}

/***************************************************************************/
// CSaView::OnEditNext Select next phonetic character
/***************************************************************************/
void CSaView::OnEditNext()
{
	int nLoop;

	if (m_pFocusedGraph) // needs to have a focused graph
	{
		ASelection().Update(this);
		nLoop = ASelection().GetSelection().nAnnotationIndex;
		// only work from visible selections
		if(( nLoop != -1)&& !(m_pFocusedGraph->HaveAnnotation(nLoop) )) nLoop=-1; // only work from visible selections

		if (GetAnnotation(PHONETIC)->IsEmpty()) return; // no segments
		int nSelection;
		if (nLoop == -1)
		{
			// no selection yet, search for first visible annotation with segments
			for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
			{
				if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])
					&& ((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
					||((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS)&&(CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) // SDM 1.5Test8.1
				{
					nLoop = CGraphWnd::m_anAnnWndOrder[nLoop];
					break;
				}
			}
			if (nLoop >= ANNOT_WND_NUMBER) return; // no possibility found
			// try to find selection within start cursor or left of it
			ASelection().SelectFromPosition(this, nLoop, GetStartCursorPosition(),CASegmentSelection::FIND_EXACT);
			if(ASelection().GetSelection().nAnnotationIndex == -1)
				ASelection().SelectFromPosition(this, nLoop, GetStartCursorPosition());
		}
		else
		{
			int MASTER = GetAnnotation(nLoop)->GetMasterIndex();
			if((MASTER!=-1)&&(nLoop != GLOSS)&&(nLoop != PHONETIC))
			{
				CSegment* pMaster=GetAnnotation(MASTER);

				DWORD dwPosition = ASelection().GetSelection().dwStop;
				int nIndexMaster = pMaster->FindStop(dwPosition); // Should always find...
				if(nIndexMaster==-1) nIndexMaster = pMaster->FindFromPosition(dwPosition, TRUE); // Better Coverage

				if(nIndexMaster == -1) return;
				nIndexMaster = pMaster->GetNext(nIndexMaster);
				if(nIndexMaster == -1) return;
				ASelection().SelectFromPosition(this, nLoop, pMaster->GetOffset(nIndexMaster),CASegmentSelection::FIND_EXACT);
				if(ASelection().GetSelection().nAnnotationIndex == -1)
					ASelection().SelectFromPosition(this, nLoop, pMaster->GetOffset(nIndexMaster));
			}
			else
			{
				DWORD dwStop;
				DWORD dwStart;

				nSelection = GetAnnotation(nLoop)->GetSelection();
				if(nSelection == -1)
				{
					dwStop = ASelection().GetSelection().dwStop;
					nSelection = GetAnnotation(nLoop)->FindFromPosition(dwStop,FALSE);
					if(nSelection == -1)
						return;

					if(dwStop >= GetAnnotation(nLoop)->GetOffset(nSelection))
						return;

					dwStart = GetAnnotation(nLoop)->GetOffset(nSelection);
				}
				else
				{
					int nNext = GetAnnotation(nLoop)->GetNext();
					dwStop = GetAnnotation(nLoop)->GetStop(nSelection);

					if (nNext == -1)
					{
						dwStart = GetDocument()->GetUnprocessedDataSize();
					}
					else
					{
						dwStart = GetAnnotation(nLoop)->GetOffset(nNext);
					}

					if((dwStop + GetDocument()->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME)) < dwStart)
						dwStart = dwStop + 2;
				}

				if(dwStart + GetDocument()->GetBytesFromTime(MIN_ADD_SEGMENT_TIME) > GetDocument()->GetUnprocessedDataSize())
					return;

				// change the selection
				ASelection().SelectFromPosition(this, nLoop, dwStart,CASegmentSelection::FIND_EXACT);
				if(ASelection().GetSelection().nAnnotationIndex == -1)
					ASelection().SelectFromPosition(this, nLoop, dwStart);
			}
		}
		// if necessary scroll selection into view
		if ((ASelection().GetSelection().dwStart <= m_dwDataPosition)
			|| (ASelection().GetSelection().dwStop >= (m_dwDataPosition + GetDataFrame())))
			ZoomIn(0, FALSE); // center selection in view
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditNext Menu update
/***************************************************************************/
void CSaView::OnUpdateEditNext(CCmdUI* pCmdUI)
{
	CSaDoc* pDoc = (CSaDoc*)GetDocument();
	int nLoop;

	BOOL bEnable = FALSE;
	if ((pDoc->GetUnprocessedDataSize() != 0) && GetFocusedGraphWnd() && !(GetAnnotation(PHONETIC)->IsEmpty())) // needs focused graph
	{
		ASelection().Update(this);
		nLoop = ASelection().GetSelection().nAnnotationIndex;
		// only work from visible selections
		if(( nLoop != -1)&& !(m_pFocusedGraph->HaveAnnotation(nLoop) )) nLoop=-1; // only work from visible selections

		int nSelection;
		if (nLoop == -1)
		{
			// no selection yet, search for first visible annotation with segments
			for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
			{
				if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])
					&& ((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
					||((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS)&&(CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) // SDM 1.5Test8.1
				{
					nLoop = CGraphWnd::m_anAnnWndOrder[nLoop];
					break;
				}
			}
			if (nLoop < ANNOT_WND_NUMBER)
				bEnable = TRUE;
		}
		else
		{
			int MASTER = GetAnnotation(nLoop)->GetMasterIndex();
			if((MASTER!=-1)&&(nLoop != GLOSS)&&(nLoop != PHONETIC))
			{
				CSegment* pMaster=GetAnnotation(MASTER);

				DWORD dwPosition = ASelection().GetSelection().dwStop;
				int nIndexMaster = pMaster->FindStop(dwPosition); // Should always find...
				if(nIndexMaster==-1) nIndexMaster = pMaster->FindFromPosition(dwPosition, TRUE); // Better Coverage

				if(nIndexMaster != -1)
					nIndexMaster = pMaster->GetNext(nIndexMaster);
				if(nIndexMaster != -1)
					bEnable = TRUE;
			}
			else
			{
				DWORD dwStop;
				DWORD dwStart;

				dwStart = GetDocument()->GetUnprocessedDataSize();

				nSelection = GetAnnotation(nLoop)->GetSelection();
				if(nSelection == -1)
				{
					dwStop = ASelection().GetSelection().dwStop;
					nSelection = GetAnnotation(nLoop)->FindFromPosition(dwStop,FALSE);
					if(nSelection != -1)
					{
						dwStart = GetAnnotation(nLoop)->GetOffset(nSelection);

						if(dwStop > GetAnnotation(nLoop)->GetOffset(nSelection))
							dwStart = GetDocument()->GetUnprocessedDataSize();
					}
				}
				else
				{
					int nNext = GetAnnotation(nLoop)->GetNext();
					dwStop = GetAnnotation(nLoop)->GetStop(nSelection);

					if (nNext == -1)
					{
						dwStart = GetDocument()->GetUnprocessedDataSize();
					}
					else
					{
						dwStart = GetAnnotation(nLoop)->GetOffset(nNext);
					}

					if((dwStop + GetDocument()->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME)) < dwStart)
						dwStart = dwStop + 2;
				}
				bEnable = TRUE;

				if(dwStart + GetDocument()->GetBytesFromTime(MIN_ADD_SEGMENT_TIME) > GetDocument()->GetUnprocessedDataSize())
					bEnable = FALSE;
			}
		}
	}
	pCmdUI->Enable(bEnable);
}

/***************************************************************************/
// CSaView::GetCursorAlignment()
/***************************************************************************/
CURSOR_ALIGNMENT CSaView::GetCursorAlignment()
{
	if(GetDocument()->GetFragments()->IsDataReady())
		return m_nCursorAlignment;
	else
		return m_nCursorAlignment == ALIGN_AT_FRAGMENT ? ALIGN_AT_ZERO_CROSSING : m_nCursorAlignment;
}

/***************************************************************************/
// CSaView::GetGraphUpdateMode
/***************************************************************************/
int CSaView::GetGraphUpdateMode()
{
	if(GetCursorAlignment() != ALIGN_AT_FRAGMENT)
		return STATIC_UPDATE;  // This file must be in static mode fragments disabled.
	else
		return MainFrame()->GetGraphUpdateMode();
}

/***************************************************************************/
// CSaView::IsAnimationRequested
/***************************************************************************/
BOOL CSaView::IsAnimationRequested()
{
	if(GetGraphUpdateMode() == STATIC_UPDATE)
		return FALSE;  // This file must be in dynamic mode to animate
	else
		return MainFrame()->IsAnimationRequested();
}

/***************************************************************************/
// CSaView::GetAnimationFrameRate
/***************************************************************************/
int CSaView::GetAnimationFrameRate()
{
	return MainFrame()->GetAnimationFrameRate();
}

void CSaView::OnSpectroFormants() 
{
	CSaDoc *pDoc = GetDocument();

	SpectroParm cParm = pDoc->GetSpectrogram(TRUE)->GetSpectroParm();

	BOOL bFormantSelected = cParm.bShowF1
		|| cParm.bShowF2
		|| cParm.bShowF3
		|| cParm.bShowF4
		|| cParm.bShowF5andUp
		|| cParm.bShowPitch;

	bFormantSelected = !bFormantSelected;

	for(int AB = FALSE; AB <= TRUE; AB++)
	{
		bool ab = (AB == TRUE);
		SpectroParm cParm = pDoc->GetSpectrogram(ab)->GetSpectroParm();

		cParm.bShowF1 = bFormantSelected;
		cParm.bShowF2 = bFormantSelected;
		cParm.bShowF3 = bFormantSelected;
		cParm.bShowF4 = FALSE;
		cParm.bShowF5andUp = FALSE;
		cParm.bShowPitch = FALSE;

		pDoc->GetSpectrogram(ab)->SetSpectroParm(cParm);
	}

	CMainFrame *pMain = (CMainFrame*) AfxGetMainWnd();
	{
		SpectroParm cParm = *pMain->GetSpectrogramParmDefaults();

		cParm.bShowF1 = bFormantSelected;
		cParm.bShowF2 = bFormantSelected;
		cParm.bShowF3 = bFormantSelected;
		cParm.bShowF4 = FALSE;
		cParm.bShowF5andUp = FALSE;
		cParm.bShowPitch = FALSE;

		pMain->SetSpectrogramParmDefaults(cParm);
	}

	{
		SpectroParm cParm = *pMain->GetSnapshotParmDefaults();

		cParm.bShowF1 = bFormantSelected;
		cParm.bShowF2 = bFormantSelected;
		cParm.bShowF3 = bFormantSelected;
		cParm.bShowF4 = FALSE;
		cParm.bShowF5andUp = FALSE;
		cParm.bShowPitch = FALSE;

		pMain->SetSnapshotParmDefaults(cParm);
	}

	if(bFormantSelected && pDoc->GetSpectrogram(TRUE)->GetFormantProcess()->IsCanceled())
		pDoc->RestartAllProcesses();

	RefreshGraphs();
}

void CSaView::OnUpdateSpectroFormants(CCmdUI* pCmdUI) 
{
	CSaDoc *pDoc = GetDocument();

	SpectroParm cParm = pDoc->GetSpectrogram(TRUE)->GetSpectroParm();

	BOOL bFormantSelected = cParm.bShowF1
		|| cParm.bShowF2
		|| cParm.bShowF3
		|| cParm.bShowF4
		|| cParm.bShowF5andUp
		|| cParm.bShowPitch;
	if (bFormantSelected && pDoc->GetSpectrogram(TRUE)->GetFormantProcess()->IsCanceled())
		OnSpectroFormants();

	pCmdUI->SetCheck(bFormantSelected);
}

void CSaView::OnMoveStopCursorHere() {

	// prevent crash
	if (m_pFocusedGraph==NULL) {
		TRACE("no graph in focus\n");
		return;
	}

	CPlotWnd * pPlot = m_pFocusedGraph->GetPlot();
	if (pPlot==NULL) {
		TRACE("no plot in focus\n");
		return;
	}

	// set the new positions
	CSaDoc* pDoc = GetDocument();						// get pointer to document

	CPoint point = pPlot->GetMousePointerPosition();
	// calculate plot client coordinates
	CRect rWnd;
	pPlot->GetClientRect(rWnd);

	// get actual data position, frame and data size and alignment
	double fDataPos;
	DWORD dwDataFrame;
	// check if area graph type
	if (m_pFocusedGraph->IsAreaGraph()) {
		// get necessary data from area plot
		fDataPos = pPlot->GetAreaPosition();
		dwDataFrame = pPlot->GetAreaLength();
	} else {
		// get necessary data from document and from view
		fDataPos = GetDataPosition(rWnd.Width());		// data index of first sample to display
		dwDataFrame = AdjustDataFrame(rWnd.Width());	// number of data points to display
	}

	DWORD dwDataSize = pDoc->GetDataSize();
	int nSmpSize = pDoc->GetFmtParm()->wBlockAlign / pDoc->GetFmtParm()->wChannels;
	// calculate data samples per pixel
	ASSERT(rWnd.Width());
	double fSamplesPerPix = (double)dwDataFrame / (double)(rWnd.Width()*nSmpSize);

	TRACE("sampperpix=%f\n",fSamplesPerPix);

	// calculate the start cursor position
	DWORD dwStopCursor = (DWORD) round(fDataPos/nSmpSize + ((double)point.x) * fSamplesPerPix);
	dwStopCursor = dwStopCursor*nSmpSize;
	// check the range
	if (dwStopCursor < (DWORD)nSmpSize) {
		dwStopCursor = (DWORD)nSmpSize;
	}
	// check the range
	if (dwStopCursor >= (dwDataSize - (DWORD)nSmpSize)) { 
		dwStopCursor = dwDataSize - (DWORD)nSmpSize;
	}

	// calculate maximum position for start cursor
	DWORD dwDifference = (DWORD)(CURSOR_MIN_DISTANCE * fSamplesPerPix * nSmpSize);
	DWORD dwStartCursor = 0;
	if (dwStopCursor > dwDifference) {
		dwStartCursor = dwStopCursor - dwDifference;
	}

	// check the calculated start position against the current setting
	if (GetStartCursorPosition() <= dwStartCursor)
	{
		dwStartCursor = GetStartCursorPosition();
	}

	int nLoop = FindSelectedAnnotationIndex();
	if (nLoop!=-1) {
		// Added and modified from CStartCursorWnd::OnLButtonUp and modified by AKE 7/22/01 to deselect segment
		// while cursor is moving
		CSegment * pSegment = FindSelectedAnnotation();
		int nIndex = pSegment->GetSelection();
		if ((dwStopCursor > pSegment->GetStop(nIndex)) ||
			(dwStartCursor < pSegment->GetOffset(nIndex))) {
			// Deselect segment
			DeselectAnnotations();
		}
	}

	TRACE("start=%lu stop=%lu\n",dwStartCursor,dwStopCursor);

	// move stop cursor also
	SetStartCursorPosition(dwStartCursor);
	SetStopCursorPosition(dwStopCursor);
}

/***************************************************************************/
// CSaDoc::OnAutoSnapUpdate Adjust all independent segments to snap boundaries
/***************************************************************************/
void CSaView::OnEditCopyPhoneticToPhonemic(void)
{
	// doesn't user want to keep existing gloss?
	if (AfxMessageBox(IDS_CONFIRM_PHONEMIC_COPY, MB_YESNO | MB_ICONQUESTION, 0) != IDYES) {
		return;
	}

	BeginWaitCursor();

	CSegment * pPhonemic = GetAnnotation(PHONEMIC);
	if (pPhonemic!=NULL) {
		pPhonemic->DeleteContents();
	}

	DWORD lastOffset = -1;
	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)GetAnnotation(PHONETIC);
	int size = pPhonetic->GetSize();
	for (int i=0;i<pPhonetic->GetSize();i++) {
		DWORD offset = pPhonetic->GetOffset(i);
		if (offset==lastOffset) {
			continue;
		}
		lastOffset = offset;
		CSaString text = pPhonetic->GetSegmentString(i);
		DWORD duration = pPhonetic->GetDuration(i);
		TRACE(L"text=%s offset=%d duration=%d\n",(LPCTSTR)text,offset,duration);
		pPhonemic->Insert(i,&text,0,offset,duration);
	}

	ShowAnnotation(PHONEMIC);

	RefreshGraphs(TRUE);

	EndWaitCursor();
}

/***************************************************************************/
// CSaDoc::OnUpdateAutoSnapUpdate
/***************************************************************************/
void CSaView::OnUpdateEditCopyPhoneticToPhonemic(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

