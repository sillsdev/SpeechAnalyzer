/////////////////////////////////////////////////////////////////////////////
// sa_view.cpp:
// Implementation of the CSaView class.
// Author: Urs Ruchtie
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.06.1.2
//        SDM Disable Hot Key F4 Edit Annotation when when edit not allowed
// 1.06.5
//        SDM Added command handlers for annotation movement, segment add,edit & remove
// 1.06.6U2
//        SDM Added OnEditAddSyllable
// 1.06.6U4
//        SDM Set cursors to selected segment on undo
//        SDM Remove references to defunct function CGraphWnd::ResetScrollFailure()
// 1.06.6U5
//        SDM Fixed problems with cursor movement function OnCursorStart/Stop/Left/Right
//        SDM Modified InitialUpdate to handle new files (record & paste as new)
//        SDM Changed DeleteGraphs() and CreateGraphs to use WINDOWPLACEMENT of old graph
// 1.06.6U6
//        SDM Added function SetPlaybackPosition
// 1.5Test8.1
//        SDM Added support for Transcriber & FreeTranslation
//        SDM Added Reference menu command support
//        SDM Changed OnEditUp/Down/Next/Previous to handle CGraphWnd::m_anAnnWndOrder[]
//        SDM Changed OnEditNext/Previous to handle CDependentTextSegments
// 1.5Test8.2
//        SDM Changed AddGloss to insert phonetic segment if appropriate
//        SDM Fixes related to GetBytesFromTime and block align
//        SDM OnGraphOrderChanged
// 1.5Test8.3
//        SDM Implemented File/Export
//        SDM Implemented File/Import
// 1.5Test10.2
//        SDM added OnPlaybackSlow
//        SDM changed OnEditAddPhonetic to Join only if segments closer than MAX_ADD_JOIN_TIME
// 1.5Test10.4
//        SDM added messages to support graph toggling for DisplayPlot
// 1.5Test10.5
//        SDM changed graph order for DisplayPlot
//        SDM changed SetPlaybackPosition for improved accuracy
//        SDM added OnTimer for SetPlaybackPosition accuracy
// 1.5Test10.7
//        SDM added OnInPlaceEdit
//        SDM fixed CreateGraphs() to not create NULL overlay
//        SDM changed OnPrint
// 1.5Test10.8
//        SDM added ErrorMessages on failed printing
// 1.5Test10.9
//        SDM fixed CreateGraphs to correct overlay bug introduced in 10.7
// 1.5Test11.0
//        SDM replaced GetOffset() + GetDuration() with CSegment::GetStop()
// 1.5Test11.3
//         SDM changed OnEditAddPhonetic to support new Gloss positions
//         SDM changed CSegment::AdjustPositionAll to CSegment::Adjust
// 03/29/2000
//        RLJ Modified File-->Export to support "Export MIDI"
// 06/01/2000
//        RLJ Added ChangeDisplayedGraphs(int OpenMode)
//        RLJ In order to reduce Sa_view.obj size enough for LINK/DEBUG to work:
//              Added pSaApp, pViewMainFrame, and pSourceParm.
//              Eliminated unused pModel definitions.
//              For any function in which pModel was defined but only used once,
//                eliminated the definition and replaced the reference with
//                a direct GetDocument() call.
//              For any function in which GetDocument() was directly called
//                more than once, defined pModel and replaced original
//                GetDocument() calls with pModel references.
// 1.5Test11.4
//        SDM added support for editing PHONEMIC/TONE/ORTHO to span multiple segments
//        SDM connected REFERENCE menus
// 06/06/2000
//        RLJ For use with "FileOpenAs->Phonetic/Music Analysis"
//              Added CREATE_FROMSCRATCH option to ::CreateGraphs,
//              Deleted ChangeDisplayedGraphs(int OpenMode)
//              Added CreateOpenAsGraphs(int OpenMode)
// 06/17/2000
//        RLJ Extend FileOpenAs to support not only Phonetic/Music Analysis,
//              but also OpenScreenF, OpenScreenG, OpenScreenI, OpenScreenK,
//              OpenScreenM, etc.
// 06/17/2000
//        RLJ Streamline code which is wasteful of space,
//              to reduce *.OBJ size for LINK/DEBUG
// 07/05/2000
//        AE  Moved some stuff to file Sa_View2.CPP
// 07/25/2000
//        RLJ Changed ReadProperties() and bWriteProperties() so that they
//              work with both 16 and 32 bit compilers
// 07/26/2000
//        DDO Changed the five canned layout with types menus to the Graph
//            menu and added the appropriate functions here.
// 07/27/2000
//        DDO Changed the sequence of exporting dialogs so the user is
//            asked what he wants to export before what file and path to
//            export to.
// 07/28/2000
//        RLJ Changes to CreateOpenAsGraphs(), including removing
//              Staff graph from list of graphs automatically opened
// 08/02/2000
//        DDO Fixed ReadProperties() to get the parent window the old way.
//            it was changed so it would compile in WIN32 but that breaks
//            it for WIN16 when loading a .wav file from the .psa file.
//            I #ifdef'd the problem code. However, it should probably be
//            tested in WIN32 the same way (i.e. loading a .wav from the
//            .psa file).
// 08/31/2000
//        DDO Fixed a problem in which the graph fonts read from the wave
//            file were being overwritten with the default graph fonts.
//
// 09/26/2000
//        DDO Deleted code for melogram buttons since that window nolonger
//            exists.
//
// 09/26/2000
//        DDO Deleted most of the code in OnPopupgraphsParameters() and
//            replaced it with a call to OnGraphsParameters() which does
//            the same thing as all the code I deleted.
//
// 10/13/2000
//        DDO Fixed so the legend for the melogram and magnitude turn on
//            automatically when the TWC is closed and turn off
//            automatically when the TWC wasn't showing but get's turned
//            on.
//
// 10/25/2000
//        TRE Moved some code to SA_View2.cpp
//
// 10/25/2000
//        DDO Added some code to redraw the position view plot when
//            scrolling takes place.
//
// 10/26/2000
//        DDO Added a line to redraw the plot of the position view graph
//            when the user zooms to the cursor. That's something I didn't
//            account for in yesterday's changes.
////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_view.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "fileInformation.h"
#include "graphsMagnify.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "math.h"
#include "printdim.h"
#include "printopt.h"
#include "sa_cdib.h"
#include "sa_mplot.h"
#include "pickover.h"
#include "objectistream.h"
#include "Segment.h"
#include "graphsTypes.h"
#include "ImportSFM.h"
#include "TextHelper.h"
#include "FileEncodingHelper.h"
#include "ImportELAN.h"
#include "FileUtils.h"

#include "sa_g_raw.h"
#include "sa_g_rec.h"
#include "sa_g_stf.h"

#include "GlossSegment.h"
#include "GlossNatSegment.h"
#include "MusicPhraseSegment.h"
#include "PhoneticSegment.h"
#include "PhonemicSegment.h"
#include "OrthoSegment.h"
#include "ReferenceSegment.h"
#include "ToneSegment.h"

#include "DlgExportFW.h"
#include "DlgExportXML.h"
#include "DlgExportSFM.h"
#include "DlgExportTimeTable.h"
#include "DlgRecorder.h"
#include "DlgPlayer.h"
#include "DlgExportLift.h"

#include "DlgPlayer.h"
#include "DlgSelectTranscriptionBars.h"
#include "DlgInsertSilence.h"
#include "DlgAlignTranscriptionDataSheet.h"
#include "DlgAdvancedParseWords.h"
#include "DlgAdvancedSegment.h"
#include "DlgAdvancedParsePhrases.h"
#include "DlgSplitFile.h"
#include "DlgSaveAsOptions.h"
#include "Dlgadvancedsegment.h"
#include "Dlgadvancedparsewords.h"
#include "Dlgadvancedparsephrases.h"
#include "Dlgautoreferencedata.h"
#include "DlgAutoRecorder.h"
#include "DlgSplitFile.h"
#include "DlgAligntranscriptiondatasheet.h"
#include "DlgImportsfmref.h"
#include "DlgExportFWResult.h"
#include "DlgExportFW.h"
#include "TextHelper.h"

#include "ScopedCursor.h"
#include "SplitFileUtils.h"

#include "graphsTypes.h"
#include "graphsParameters.h"

#include "StopWatch.h"

#include "WaveUtils.h"
#include "ScopedStatusBar.h"

#include <locale>
#include <codecvt>
#include <fstream>
#include <cstdlib>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CSaView

IMPLEMENT_DYNCREATE(CSaView, CView)

BEGIN_MESSAGE_MAP(CSaView, CView)
	ON_WM_KEYUP()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_ADDOVERLAY, OnAddOverlay)
	ON_COMMAND(ID_BOUNDARIES_ALL, OnBoundariesAll)
	ON_COMMAND(ID_BOUNDARIES_NONE, OnBoundariesNone)
	ON_COMMAND(ID_BOUNDARIES_RAWDATA, OnBoundariesRawdata)
	ON_COMMAND(ID_BOUNDARIES_THIS, OnBoundariesThis)
	ON_COMMAND(ID_VIEW_TRANSCRIPTION_BOUNDARIES, OnViewTranscriptionBoundaries)
	ON_COMMAND(ID_DP_GRAPITCH, OnDpGrapitch)
	ON_COMMAND(ID_DP_RAWDATA, OnDpRawdata)
	ON_COMMAND(ID_DP_SPECTROGRAM, OnDpSpectrogram)
	ON_COMMAND(ID_EDIT_ADD_PHONETIC, OnEditAddPhonetic)
	ON_COMMAND(ID_EDIT_ADD_AUTO_PHRASE_L2, OnEditAddAutoPhraseL2)
	ON_COMMAND(ID_EDIT_ADD_BOOKMARK, OnEditAddBookmark)
	ON_COMMAND(ID_EDIT_ADD_PHRASE_L1, OnEditAddPhraseL1)
	ON_COMMAND(ID_EDIT_ADD_PHRASE_L2, OnEditAddPhraseL2)
	ON_COMMAND(ID_EDIT_ADD_PHRASE_L3, OnEditAddPhraseL3)
	ON_COMMAND(ID_EDIT_ADD_PHRASE_L4, OnEditAddPhraseL4)
	ON_COMMAND(ID_EDIT_ADD_SYLLABLE, OnEditAddSyllable)
	ON_COMMAND(ID_EDIT_ADD_WORD, OnEditAddGloss)
	ON_COMMAND(ID_EDIT_ADD_MARKUP, OnEditAddMarkup)
	ON_COMMAND(ID_EDIT_AUTO_ADD, OnEditAutoAdd)
	ON_COMMAND(ID_EDIT_AUTO_ADD_STORY_SECTION, OnEditAutoAddStorySection)
	ON_COMMAND(ID_EDIT_BOUNDARY_START_LEFT, OnEditBoundaryStartLeft)
	ON_COMMAND(ID_EDIT_BOUNDARY_START_RIGHT, OnEditBoundaryStartRight)
	ON_COMMAND(ID_EDIT_BOUNDARY_STOP_LEFT, OnEditBoundaryStopLeft)
	ON_COMMAND(ID_EDIT_BOUNDARY_STOP_RIGHT, OnEditBoundaryStopRight)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_COPY_MEASUREMENTS, OnEditCopyMeasurements)
	ON_COMMAND(ID_EDIT_COPY_PHONETIC_TO_PHONEMIC, OnEditCopyPhoneticToPhonemic)
	ON_COMMAND(ID_EDIT_CURSOR_START_LEFT, OnEditCursorStartLeft)
	ON_COMMAND(ID_EDIT_CURSOR_START_RIGHT, OnEditCursorStartRight)
	ON_COMMAND(ID_EDIT_CURSOR_STOP_LEFT, OnEditCursorStopLeft)
	ON_COMMAND(ID_EDIT_CURSOR_STOP_RIGHT, OnEditCursorStopRight)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_DOWN, OnEditDown)
	ON_COMMAND(ID_EDIT_INPLACE, OnEditInplace)
	ON_COMMAND(ID_EDIT_NEXT, OnEditNext)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_PASTENEW, OnEditPasteNew)
	ON_COMMAND(ID_EDIT_PREVIOUS, OnEditPrevious)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_EDIT_REMOVE, OnEditRemove)

	ON_COMMAND(ID_NEXT_ERROR, OnNextError)
	ON_COMMAND(ID_PREVIOUS_ERROR, OnPreviousError)

	// edit menu operations
	ON_COMMAND(ID_EDIT_SPLIT, OnEditSplit)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SPLIT, OnUpdateEditSplit)
	ON_COMMAND(ID_EDIT_MERGE, OnEditMerge)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MERGE, OnUpdateEditMerge)
	ON_COMMAND(ID_EDIT_MOVE_LEFT, OnEditMoveLeft)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVE_LEFT, OnUpdateEditMoveLeft)
	ON_COMMAND(ID_EDIT_MOVE_RIGHT, OnEditMoveRight)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVE_RIGHT, OnUpdateEditMoveRight)
	ON_COMMAND(ID_EDIT_SPLIT_MOVE_LEFT, OnEditSplitMoveLeft)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SPLIT_MOVE_LEFT, OnUpdateEditSplitMoveLeft)
	ON_COMMAND(ID_EDIT_MOVE_RIGHT_MERGE_NEXT, OnEditMoveRightMergeNext)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVE_RIGHT_MERGE_NEXT, OnUpdateEditMoveRightMergeNext)
	ON_COMMAND(ID_EDIT_MOVE_RIGHT_MERGE_PREV, OnEditMoveRightMergePrev)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVE_RIGHT_MERGE_PREV, OnUpdateEditMoveRightMergePrev)

	// begin cursor and accelerator operations
	ON_COMMAND(ID_SPLIT_MOVE_LEFT_AT, OnSplitMoveLeftAt)
	ON_UPDATE_COMMAND_UI(ID_SPLIT_MOVE_LEFT_AT, OnUpdateSplitMoveLeftAt)
	ON_COMMAND(ID_MOVE_RIGHT_MERGE_PREV_AT, OnMoveRightMergePrevAt)
	ON_UPDATE_COMMAND_UI(ID_MOVE_RIGHT_MERGE_PREV_AT, OnUpdateMoveRightMergePrevAt)
	ON_COMMAND(ID_MOVE_RIGHT_MERGE_NEXT_AT, OnMoveRightMergeNextAt)
	ON_UPDATE_COMMAND_UI(ID_MOVE_RIGHT_MERGE_NEXT_AT, OnUpdateMoveRightMergeNextAt)

	// on mouse popup operations
	ON_COMMAND(ID_SPLIT_HERE, OnSplitHere)
	ON_UPDATE_COMMAND_UI(ID_SPLIT_HERE, OnUpdateSplitHere)
	ON_COMMAND(ID_MERGE_HERE, OnMergeHere)
	ON_UPDATE_COMMAND_UI(ID_MERGE_HERE, OnUpdateMergeHere)
	ON_COMMAND(ID_MOVE_LEFT_HERE, OnMoveLeftHere)
	ON_UPDATE_COMMAND_UI(ID_MOVE_LEFT_HERE, OnUpdateMoveLeftHere)
	ON_COMMAND(ID_MOVE_RIGHT_HERE, OnMoveRightHere)
	ON_UPDATE_COMMAND_UI(ID_MOVE_RIGHT_HERE, OnUpdateMoveRightHere)
	ON_COMMAND(ID_SPLIT_MOVE_LEFT_HERE, OnSplitMoveLeftHere)
	ON_UPDATE_COMMAND_UI(ID_SPLIT_MOVE_LEFT_HERE, OnUpdateSplitMoveLeftHere)
	ON_COMMAND(ID_MOVE_RIGHT_MERGE_NEXT_HERE, OnMoveRightMergeNextHere)
	ON_UPDATE_COMMAND_UI(ID_MOVE_RIGHT_MERGE_NEXT_HERE, OnUpdateMoveRightMergeNextHere)
	ON_COMMAND(ID_MOVE_RIGHT_MERGE_PREV_HERE, OnMoveRightMergePrevHere)
	ON_UPDATE_COMMAND_UI(ID_MOVE_RIGHT_MERGE_PREV_HERE, OnUpdateMoveRightMergePrevHere)

	ON_COMMAND(ID_EDIT_SEGMENT_BOUNDARIES, OnEditBoundaries)
	ON_COMMAND(ID_EDIT_SEGMENT_SIZE, OnEditSegmentSize)
	ON_COMMAND(ID_EDIT_SELECTWAVEFORM_SEG, OnEditSelectWaveformSeg)
	ON_COMMAND(ID_EDIT_SELECTWAVEFORM, OnEditSelectWaveform)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_UP, OnEditUp)
	ON_COMMAND(ID_EXPORT_FW, OnExportFW)
	ON_COMMAND(ID_EXPORT_MIDI, OnExportStaff)
	ON_COMMAND(ID_EXPORT_SFM, OnExportSFM)
	ON_COMMAND(ID_EXPORT_TIME_TABLE, OnExportTimeTable)
	ON_COMMAND(ID_EXPORT_XML, OnExportXML)
	ON_COMMAND(ID_EXPORT_LIFT, OnExportLift)
	ON_COMMAND(ID_FILE_INFORMATION, OnFileInformation)
	ON_COMMAND(ID_FILE_PAGE_SETUP, OnPrintPageSetup)
	ON_COMMAND(ID_FILE_PHONOLOGYASSISTANT, OnFilePhonologyAssistant)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_GRAPHS_MAGNIFY1, OnGraphsMagnify1)
	ON_COMMAND(ID_GRAPHS_MAGNIFY2, OnGraphsMagnify2)
	ON_COMMAND(ID_GRAPHS_MAGNIFY4, OnGraphsMagnify4)
	ON_COMMAND(ID_GRAPHS_MAGNIFYCUSTOM, OnGraphsMagnifycustom)
	ON_COMMAND(ID_GRAPHS_PARAMETERS, OnGraphsParameters)
	ON_COMMAND(ID_GRAPHS_STYLE_LINE, OnGraphsStyleLine)
	ON_COMMAND(ID_GRAPHS_STYLE_SOLID, OnGraphsStyleSolid)
	ON_COMMAND(ID_GRAPHS_TILE, OnGraphsRetile)
	ON_COMMAND(ID_GRAPHS_TYPES, OnGraphsTypes)
	ON_COMMAND(ID_SHOW_BORDERS, OnShowBorders)
	ON_COMMAND(ID_GRAPHS_ZOOM_ALL, OnGraphsZoomAll)
	ON_COMMAND(ID_GRAPHS_ZOOM_CURSORS, OnGraphsZoomCursors)
	ON_COMMAND(ID_GRAPHS_ZOOM_IN, OnGraphsZoomIn)
	ON_COMMAND(ID_GRAPHS_ZOOM_OUT, OnGraphsZoomOut)
	ON_COMMAND(ID_IMPORT_MIDI, OnImportStaff)
	ON_COMMAND(ID_IMPORT_SFM, OnImportSFM)
	ON_COMMAND(ID_IMPORT_SAB, OnImportSAB)
	ON_COMMAND(ID_IMPORT_ELAN, OnImportELAN)
	ON_COMMAND(ID_IMPORT_TIME_TABLE, OnImportSFT)
	ON_COMMAND(ID_LEGEND_ALL, OnLegendAll)
	ON_COMMAND(ID_LEGEND_NONE, OnLegendNone)
	ON_COMMAND(ID_LEGEND_RAWDATA, OnLegendRawdata)
	ON_COMMAND(ID_MOVE_STOP_CURSOR_HERE, OnMoveStopCursorHere)
	ON_COMMAND(ID_NEXT_GRAPH, OnNextGraph)
	ON_COMMAND(ID_PREVIOUS_GRAPH, OnPreviousGraph)
	ON_COMMAND(ID_PLAYBACK_CURSORS, OnPlaybackCursors)
	ON_COMMAND(ID_PLAYBACK_FILE, OnPlaybackFile)
	ON_COMMAND(ID_PLAYBACK_LEFTWIN_TO_STARTCUR, OnPlaybackLeftToStart)
	ON_COMMAND(ID_PLAYBACK_LEFTWIN_TO_ENDCUR, OnPlaybackLeftToStop)
	ON_COMMAND(ID_PLAYBACK_SLOW, OnPlaybackSlow)
	ON_COMMAND(ID_PLAYBACK_STARTCUR_TO_RIGHTWIN, OnPlaybackStartToRight)
	ON_COMMAND(ID_PLAYBACK_ENDCUR_TO_RIGHTWIN, OnPlaybackStopToRight)
	ON_COMMAND(ID_PLAYBACK_WINDOW, OnPlaybackWindow)
	ON_COMMAND(ID_PLAYBACK_ENCCUR_TO_FILEEND, OnPlaybackEndCursor)
	ON_COMMAND(ID_PLAYBACK_STARTCUR_TO_FILEEND, OnPlaybackBeginCursor)
	ON_COMMAND(ID_PLAYER, OnPlayer)
	ON_COMMAND(ID_PLAYER_PAUSE, OnPlayerPause)
	ON_COMMAND(ID_PLAYER_RESUME, OnPlayerResume)
	ON_COMMAND(ID_PLAYER_TOGGLE, OnPlayerToggle)
	ON_COMMAND(ID_PLAYER_STOP, OnPlayerStop)
	ON_COMMAND(ID_POPUPGRAPH_GRIDLINES, OnPopupgraphGridlines)
	ON_COMMAND(ID_POPUPGRAPH_LEGEND, OnPopupgraphLegend)
	ON_COMMAND(ID_POPUPGRAPH_MAGNIFY1, OnPopupgraphMagnify1)
	ON_COMMAND(ID_POPUPGRAPH_MAGNIFY2, OnPopupgraphMagnify2)
	ON_COMMAND(ID_POPUPGRAPH_MAGNIFY4, OnPopupgraphMagnify4)
	ON_COMMAND(ID_POPUPGRAPH_MAGNIFYCUSTOM, OnPopupgraphMagnifycustom)
	ON_COMMAND(ID_POPUPGRAPH_STYLE_DOTS, OnPopupgraphStyleDots)
	ON_COMMAND(ID_POPUPGRAPH_STYLE_LINE, OnPopupgraphStyleLine)
	ON_COMMAND(ID_POPUPGRAPH_STYLE_SOLID, OnPopupgraphStyleSolid)
	ON_COMMAND(ID_POPUPGRAPH_XSCALE, OnPopupgraphXScale)
	ON_COMMAND(ID_REMOVE_OVERLAY, OnRemoveOverlay)
	ON_COMMAND(ID_REMOVE_OVERLAYS, OnRemoveOverlays)
	ON_COMMAND(ID_RESTART_PROCESS, OnRestartProcess)
	ON_COMMAND(ID_SETUP_FNKEYS, OnSetupFnkeys)
	ON_COMMAND(ID_SPECTRO_FORMANTS, OnSpectroFormants)
	ON_COMMAND(ID_XSCALE_ALL, OnXScaleAll)
	ON_COMMAND(ID_XSCALE_NONE, OnXScaleNone)
	ON_COMMAND(ID_XSCALE_RAWDATA, OnXScaleRawdata)
	ON_COMMAND(ID_SELECT_TRANSCRIPTION_BARS, OnSelectTranscriptionBars)
	ON_COMMAND_RANGE(IDD_3D, IDD_3D, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_3D_PITCH, IDD_3D_PITCH, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_CEPPITCH, IDD_CEPPITCH, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_CHANGE, IDD_CHANGE, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_CHPITCH, IDD_CHPITCH, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_DURATION, IDD_DURATION, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_F1F2, IDD_F1F2, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_F2F1, IDD_F2F1, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_F2F1F1, IDD_F2F1F1, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_GLOTWAVE, IDD_GLOTWAVE, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_GRAPITCH, IDD_GRAPITCH, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_INVSDP, IDD_INVSDP, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_LOUDNESS, IDD_LOUDNESS, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_MAGNITUDE, IDD_MAGNITUDE, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_MELOGRAM, IDD_MELOGRAM, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_PITCH, IDD_PITCH, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_POA, IDD_POA, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_RATIO, IDD_RATIO, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_RAWDATA, IDD_RAWDATA, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_RECORDING, IDD_RECORDING, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_SDP_A, IDD_SDP_A, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_SDP_B, IDD_SDP_B, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_SMPITCH, IDD_SMPITCH, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_SNAPSHOT, IDD_SNAPSHOT, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_SPECTROGRAM, IDD_SPECTROGRAM, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_SPECTRUM, IDD_SPECTRUM, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_STAFF, IDD_STAFF, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_TWC, IDD_TWC, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_WAVELET, IDD_WAVELET, OnChangeGraph)
	ON_COMMAND_RANGE(IDD_ZCROSS, IDD_ZCROSS, OnChangeGraph)
	ON_COMMAND_RANGE(ID_GRAPHTYPES_SELECT_FIRST, ID_GRAPHTYPES_SELECT_LAST, OnGraphTypesSelect)
	ON_COMMAND_RANGE(ID_LAYOUT_FIRST, ID_LAYOUT_LAST, OnLayout)
	ON_COMMAND_RANGE(ID_PHONETIC_ALL, ID_PHRASE_L4_ALL, OnAnnotationAll)
	ON_COMMAND_RANGE(ID_PHONETIC_NONE, ID_PHRASE_L4_NONE, OnAnnotationNone)
	ON_COMMAND_RANGE(ID_PHONETIC_RAWDATA, ID_PHRASE_L4_RAWDATA, OnAnnotationRawdata)
	ON_COMMAND_RANGE(ID_PLAY_F1, ID_PLAY_F24, OnPlayFKey)
	ON_COMMAND_RANGE(ID_POPUPGRAPH_PHONETIC, ID_POPUPGRAPH_MUSIC_PL4, OnPopupgraphAnnotation)
	ON_MESSAGE(WM_USER_APP_MESSAGE, OnAppMessage)
	ON_MESSAGE(WM_USER_AUTOSAVE, OnAutoSave)
	ON_MESSAGE(WM_USER_CURSOR_IN_FRAGMENT, OnCursorInFragment)
	ON_MESSAGE(WM_USER_GRAPH_COLORCHANGED, OnGraphColorChanged)
	ON_MESSAGE(WM_USER_GRAPH_DESTROYED, OnGraphDestroyed)
	ON_MESSAGE(WM_USER_GRAPH_FONTCHANGED, OnGraphFontChanged)
	ON_MESSAGE(WM_USER_GRAPH_GRIDCHANGED, OnGraphGridChanged)
	ON_MESSAGE(WM_USER_GRAPH_ORDERCHANGED, OnGraphOrderChanged)
	ON_MESSAGE(WM_USER_GRAPH_STYLECHANGED, OnGraphStyleChanged)
	ON_MESSAGE(WM_USER_RECORDER, OnRecorder)
	ON_MESSAGE(WM_USER_VIEW_ANIMATIONCHANGED, OnAnimationChanged)
	ON_MESSAGE(WM_USER_VIEW_CURSORALIGNCHANGED, OnCursorAlignmentChanged)
	ON_MESSAGE(WM_USER_VIEW_GRAPHUPDATECHANGED, OnGraphUpdateModeChanged)
	ON_MESSAGE(WM_USER_VIEW_SCROLLZOOMCHANGED, OnScrollZoomChanged)
	ON_UPDATE_COMMAND_UI(IDD_3D, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_3D_PITCH, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_CEPPITCH, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_CHANGE, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_CHPITCH, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_DURATION, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_F1F2, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_F2F1, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_F2F1F1, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_GLOTWAVE, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_GRAPITCH, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_INVSDP, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_LOUDNESS, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_MAGNITUDE, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_MELOGRAM, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_PITCH, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_POA, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_RATIO, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_RAWDATA, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_RECORDING, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SDP_A, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SDP_B, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SMPITCH, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SNAPSHOT, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SPECTROGRAM, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_SPECTRUM, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_STAFF, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_TWC, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_WAVELET, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(IDD_ZCROSS, OnUpdateChangeGraph)
	ON_UPDATE_COMMAND_UI(ID_ADDOVERLAY, OnUpdateAddOverlay)
	ON_UPDATE_COMMAND_UI(ID_BOUNDARIES_ALL, OnUpdateBoundariesAll)
	ON_UPDATE_COMMAND_UI(ID_BOUNDARIES_NONE, OnUpdateBoundariesNone)
	ON_UPDATE_COMMAND_UI(ID_BOUNDARIES_RAWDATA, OnUpdateBoundariesRawdata)
	ON_UPDATE_COMMAND_UI(ID_BOUNDARIES_THIS, OnUpdateBoundariesThis)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TRANSCRIPTION_BOUNDARIES, OnUpdateViewTranscriptionBoundaries)
	ON_UPDATE_COMMAND_UI(ID_DP_GRAPITCH, OnUpdateDpGrapitch)
	ON_UPDATE_COMMAND_UI(ID_DP_RAWDATA, OnUpdateDpRawdata)
	ON_UPDATE_COMMAND_UI(ID_DP_SPECTROGRAM, OnUpdateDpSpectrogram)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_PHONETIC, OnUpdateEditAddPhonetic)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_AUTO_PHRASE_L2, OnUpdateEditAddAutoPhraseL2)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_BOOKMARK, OnUpdateEditAddBookmark)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_PHRASE_L1, OnUpdateEditAddPhraseL1)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_PHRASE_L2, OnUpdateEditAddPhraseL2)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_PHRASE_L3, OnUpdateEditAddPhraseL3)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_PHRASE_L4, OnUpdateEditAddPhraseL4)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_SYLLABLE, OnUpdateEditAddSyllable)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_WORD, OnUpdateEditAddGloss)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ADD_MARKUP, OnUpdateEditAddMarkup)
	ON_UPDATE_COMMAND_UI(ID_EDIT_AUTO_ADD, OnUpdateEditAutoAdd)
	ON_UPDATE_COMMAND_UI(ID_EDIT_AUTO_ADD_STORY_SECTION, OnUpdateEditAutoAddStorySection)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY_PHONETIC_TO_PHONEMIC, OnUpdateEditCopyPhoneticToPhonemic)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateHasSel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DOWN, OnUpdateEditDown)
	ON_UPDATE_COMMAND_UI(ID_EDIT_NEXT, OnUpdateEditNext)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTENEW, OnUpdateEditPasteNew)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PREVIOUS, OnUpdateEditPrevious)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REMOVE, OnUpdateEditRemove)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SEGMENT_BOUNDARIES, OnUpdateEditBoundaries)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SEGMENT_SIZE, OnUpdateEditSegmentSize)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECTWAVEFORM_SEG, OnUpdateEditSelectWaveformSeg)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECTWAVEFORM, OnUpdateEditSelectWaveform)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UP, OnUpdateEditUp)
	ON_UPDATE_COMMAND_UI(ID_EXPORT_MIDI, OnUpdateExportStaff)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFilenew)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdatePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_MAGNIFY1, OnUpdateGraphsMagnify1)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_MAGNIFY2, OnUpdateGraphsMagnify2)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_MAGNIFY4, OnUpdateGraphsMagnify4)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_MAGNIFYCUSTOM, OnUpdateGraphsMagnifycustom)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_PARAMETERS, OnUpdateGraphsParameters)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_STYLE_LINE, OnUpdateGraphsStyleLine)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_STYLE_SOLID, OnUpdateGraphsStyleSolid)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_TYPES, OnUpdateGraphsTypes)
	ON_UPDATE_COMMAND_UI(ID_SHOW_BORDERS, OnUpdateShowBorders)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_ZOOM_ALL, OnUpdateGraphsZoomAll)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_ZOOM_CURSORS, OnUpdateGraphsZoomCursors)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_ZOOM_IN, OnUpdateGraphsZoomIn)
	ON_UPDATE_COMMAND_UI(ID_GRAPHS_ZOOM_OUT, OnUpdateGraphsZoomOut)
	ON_UPDATE_COMMAND_UI(ID_GRAPHTYPES_SELECT_LAST, OnUpdateGraphTypesSelect)
	ON_UPDATE_COMMAND_UI(ID_IMPORT_MIDI, OnUpdateImportStaff)
	ON_UPDATE_COMMAND_UI(ID_LAYOUT_LAST, OnUpdateLayout)
	ON_UPDATE_COMMAND_UI(ID_LEGEND_ALL, OnUpdateLegendAll)
	ON_UPDATE_COMMAND_UI(ID_LEGEND_NONE, OnUpdateLegendNone)
	ON_UPDATE_COMMAND_UI(ID_LEGEND_RAWDATA, OnUpdateLegendRawdata)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_CURSORS, OnUpdatePlaybackPortion)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_FILE, OnUpdatePlaybackPortion)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_SLOW, OnUpdatePlayback)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_LEFTWIN_TO_STARTCUR, OnUpdatePlaybackPortion)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_LEFTWIN_TO_ENDCUR, OnUpdatePlaybackPortion)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_STARTCUR_TO_RIGHTWIN, OnUpdatePlaybackPortion)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_ENDCUR_TO_RIGHTWIN, OnUpdatePlaybackPortion)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_WINDOW, OnUpdatePlaybackPortion)
	ON_UPDATE_COMMAND_UI(ID_PLAYER, OnUpdatePlayback)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_PAUSE, OnUpdatePlayerPause)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_RESUME, OnUpdatePlayerResume)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_TOGGLE, OnUpdatePlayerToggle)
	ON_UPDATE_COMMAND_UI(ID_PLAYER_STOP, OnUpdatePlayerStop)
	ON_UPDATE_COMMAND_UI(ID_PLAY_F24, OnUpdatePlayback)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_STARTCUR_TO_FILEEND, OnUpdatePlaybackPortion)
	ON_UPDATE_COMMAND_UI(ID_PLAYBACK_ENCCUR_TO_FILEEND, OnUpdatePlaybackPortion)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_GRIDLINES, OnUpdatePopupgraphGridlines)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_LEGEND, OnUpdatePopupgraphLegend)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_MAGNIFY1, OnUpdatePopupgraphMagnify1)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_MAGNIFY2, OnUpdatePopupgraphMagnify2)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_MAGNIFY4, OnUpdatePopupgraphMagnify4)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_MAGNIFYCUSTOM, OnUpdatePopupgraphMagnifycustom)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_STYLE_DOTS, OnUpdatePopupgraphStyleDots)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_STYLE_LINE, OnUpdatePopupgraphStyleLine)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_STYLE_SOLID, OnUpdatePopupgraphStyleSolid)
	ON_UPDATE_COMMAND_UI(ID_POPUPGRAPH_XSCALE, OnUpdatePopupgraphXScale)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_OVERLAY, OnUpdateRemoveOverlay)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_OVERLAYS, OnUpdateRemoveOverlay)
	ON_UPDATE_COMMAND_UI(ID_RESTART_PROCESS, OnUpdateRestartProcess)
	ON_UPDATE_COMMAND_UI(ID_SETUP_FNKEYS, OnUpdateSetupFnkeys)
	ON_UPDATE_COMMAND_UI(ID_SPECTRO_FORMANTS, OnUpdateSpectroFormants)
	ON_UPDATE_COMMAND_UI(ID_XSCALE_ALL, OnUpdateXScaleAll)
	ON_UPDATE_COMMAND_UI(ID_XSCALE_NONE, OnUpdateXScaleNone)
	ON_UPDATE_COMMAND_UI(ID_XSCALE_RAWDATA, OnUpdateXScaleRawdata)
	ON_UPDATE_COMMAND_UI(ID_SELECT_TRANSCRIPTION_BARS, OnUpdateSelectTranscriptionBars)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PHONETIC_ALL, ID_PHRASE_L4_ALL, OnUpdateAnnotationAll)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PHONETIC_NONE, ID_PHRASE_L4_NONE, OnUpdateAnnotationNone)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PHONETIC_RAWDATA, ID_PHRASE_L4_RAWDATA, OnUpdateAnnotationRawdata)
	ON_UPDATE_COMMAND_UI_RANGE(ID_POPUPGRAPH_PHONETIC, ID_POPUPGRAPH_MUSIC_PL4, OnUpdatePopupgraphAnnotation)
	ON_COMMAND(ID_TOOLS_ADJUST_SILENCE, OnToolsAdjustSilence)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_ADJUST_SILENCE, OnUpdateToolsAdjustSilence)
	ON_COMMAND(ID_TOOLS_ADJUST_ZERO, OnToolsAdjustZero)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_ADJUST_ZERO, OnUpdateToolsAdjustZero)
	ON_COMMAND(ID_TOOLS_ADJUST_NORMALIZE, OnToolsAdjustNormalize)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_ADJUST_NORMALIZE, OnUpdateToolsAdjustNormalize)
	ON_COMMAND(ID_TOOLS_ADJUST_INVERT, OnToolsAdjustInvert)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_ADJUST_INVERT, OnUpdateToolsAdjustInvert)
	ON_COMMAND(ID_EDIT_AUTO_SNAP_UPDATE, OnAutoSnapUpdate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_AUTO_SNAP_UPDATE, OnUpdateAutoSnapUpdate)
	ON_COMMAND(ID_EDIT_UPDATE_BOUNDARIES, OnUpdateBoundaries)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UPDATE_BOUNDARIES, OnUpdateUpdateBoundaries)
	ON_COMMAND(ID_AUTO_MARKUP_REFERENCE_DATA, OnAddReferenceData)
	ON_UPDATE_COMMAND_UI(ID_AUTO_MARKUP_REFERENCE_DATA, OnUpdateAddReferenceData)
	ON_COMMAND(ID_AUTO_ALIGN, OnAutoAlign)
	ON_UPDATE_COMMAND_UI(ID_AUTO_ALIGN, OnUpdateAutoAlign)
	ON_COMMAND(ID_ADVANCED_PARSE_WORD, OnAdvancedParseWords)
	ON_UPDATE_COMMAND_UI(ID_ADVANCED_PARSE_WORD, OnUpdateAdvancedParseWords)

	ON_COMMAND(ID_ADVANCED_PARSE_PHRASE, OnAdvancedParsePhrases)
	ON_UPDATE_COMMAND_UI(ID_ADVANCED_PARSE_PHRASE, OnUpdateAdvancedParsePhrases)
	ON_COMMAND(ID_ADVANCED_SEGMENT, OnAdvancedSegment)
	ON_UPDATE_COMMAND_UI(ID_ADVANCED_SEGMENT, OnUpdateAdvancedSegment)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
	ON_COMMAND(ID_FILE_SPLIT, OnFileSplitFile)
	ON_UPDATE_COMMAND_UI(ID_FILE_SPLIT, OnUpdateFileSplit)
	ON_COMMAND(ID_AUTO_GENERATE_CV_DATA, OnGenerateCVData)
	ON_COMMAND(ID_EXPORT_CV_DATA, OnExportCVData)
	ON_WM_CHAR()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_WM_KEYDOWN()
	ON_WM_MDIACTIVATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

CObjectIStream * CSaView::s_pobsAutoload = NULL;
static LPCSTR psz_sagraph = "sagraph";

CSaView::CSaView() {

	Init();
}

CSaView::CSaView(const CSaView * right) {

	Init();

	*this = *right;

	lastBoundaryStartCursor = UNDEFINED_OFFSET;
	lastBoundaryStopCursor = UNDEFINED_OFFSET;
	lastBoundaryIndex = -1;
	lastBoundaryCursor = UNDEFINED_CURSOR;

	enableScrolling = false;
}

CSaView::~CSaView() {
	Clear();
	if (m_pStopwatch) {
		delete m_pStopwatch;
		m_pStopwatch = NULL;
	}
}

CSaView & CSaView::operator=(const CSaView & right) {

	// don't copy ourselves..
	if (&right == this) {
		return *this;
	}

	Clear();
	PartialCopy(right);
	m_restartPageOptions = right.m_restartPageOptions;
	m_pPageLayout = new CDlgPrintOptions(right.m_pPageLayout);
	m_pFocusedGraph = NULL; // no graph focused
	m_pCDibForPrint = NULL;
	m_pPgLayoutBackup = NULL;
	m_pPickOverlay = new CDlgPickOver();
	m_z = 0;
	m_WeJustReadTheProperties = FALSE;
	m_pDocument = NULL;

	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (right.m_apGraphs[nLoop]) {
			CGraphWnd & rightGraph = *(right.m_apGraphs[nLoop]);
			m_apGraphs[nLoop] = new CGraphWnd(rightGraph);
			m_anGraphID[nLoop] = m_apGraphs[nLoop]->GetPlotID();
		} else {
			m_apGraphs[nLoop] = NULL;
			m_anGraphID[nLoop] = 0;
		}
	}
	m_bViewCreated = FALSE;
	return *this;
}

void CSaView::Init() {

	m_pDlgAdvancedSegment = NULL;
	m_pDlgAdvancedParsePhrases = NULL;
	m_pDlgAdvancedParseWords = NULL;

	m_pStopwatch = NULL;
	m_restartPageOptions = FALSE;
	m_pPageLayout = new CDlgPrintOptions();

	//********************************************************
	// 09/25/2000 - DDO
	//********************************************************
	m_bStaticTWC = TRUE;
	m_bNormalMelogram = TRUE;

	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		m_apGraphs[nLoop] = NULL;
		m_anGraphID[nLoop] = 0;
	}

	// default graph
	m_anGraphID[0] = IDD_RAWDATA;
	// no graph focused
	m_pFocusedGraph = NULL;
	m_nFocusedID = 0;
	// default layout
	m_nLayout = ID_LAYOUT_1;
	m_bLegendAll = TRUE;
	m_bLegendNone = FALSE;
	m_bXScaleAll = TRUE;
	m_bXScaleNone = FALSE;

	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		if (nLoop == PHONETIC) {
			m_abAnnAll[nLoop] = FALSE;
			m_abAnnNone[nLoop] = FALSE;
		} else {
			m_abAnnAll[nLoop] = FALSE;
			m_abAnnNone[nLoop] = TRUE;
		}
	}
	m_nCursorAlignment = GetMainFrame().GetCursorAlignment();
	m_bTranscriptionBoundaries = TRUE;
	m_bUpdateBoundaries = TRUE;
	m_bDrawStyleLine = TRUE;
	// start with first sample data
	m_dwDataPosition = 0;
	// no magnify
	m_fMagnify = 1.0;
	// no zoom
	m_fZoom = 1.0;
	m_fMaxZoom = 0;
	m_bAnimating = FALSE;
	m_bPrintPreviewInProgress = FALSE;
	m_pCDibForPrint = NULL;
	m_pPgLayoutBackup = NULL;
	m_pPickOverlay = new CDlgPickOver();
	m_eInitialShowCmd = SW_SHOWNORMAL;
	m_z = 0;
	m_WeJustReadTheProperties = FALSE;
	m_bViewCreated = FALSE;
	VERIFY(CStopwatch::CreateObject(&m_pStopwatch));

	m_bEditBoundaries = false;
	m_bEditSegmentSize = false;

	m_dwStopCursor = 0;
	m_dwStartCursor = 0;
	m_dwHScrollFactor = 0;
	m_fVScrollSteps = 0;
	m_dwScrollLine = 0;
	m_dwPlaybackPosition = 0;
	m_dwLastPlaybackPosition = 0;
	m_dwLastGloss = -1;
	TRACE("init=%d\n", m_dwLastPlaybackPosition);
	m_dwPlaybackTime = 0;
	m_dPlaybackPositionLimit = 0;
	m_nPlaybackSpeed = 0;
	m_bViewIsActive = FALSE;
	m_printArea.x = 0;
	m_printArea.y = 0;
	m_newPrinterDPI = 0;
	m_printScaleX = 0;
	m_printScaleY = 0;

	lastBoundaryStartCursor = UNDEFINED_OFFSET;
	lastBoundaryStopCursor = UNDEFINED_OFFSET;
	lastBoundaryIndex = -1;
	lastBoundaryCursor = UNDEFINED_CURSOR;

	enableScrolling = false;
}

/***************************************************************************/
// 09/28/2000 - DDO This function will create a graph object and read its
//                  properties depending on how it should be created. I
//                  wrote this to get it out of the CreateGraphs functions
//                  which was too bloated.
/***************************************************************************/
void CSaView::CreateOneGraphStepOne(UINT nID, CGraphWnd ** pGraph, CREATE_HOW how, CGraphWnd * pFromGraph, CObjectIStream * pObs) {

	if (nID != ID_GRAPHS_OVERLAY) {
		(*pGraph) = new CGraphWnd(nID);
		switch (how) {
		case CREATE_FROMSTREAM:
			ASSERT(pObs);
			if (*pGraph) {
				m_WeJustReadTheProperties = (*pGraph)->ReadProperties(*pObs);
			} else {
				pObs->SkipToEndMarker(psz_sagraph);
			}
			break;

		case CREATE_FROMGRAPH:
			ASSERT(pFromGraph);
			if (*pGraph) {
				(*pGraph)->PartialCopy(*pFromGraph);
				m_WeJustReadTheProperties = TRUE;
			}
			break;

			// Use program defaults
		case CREATE_FROMSCRATCH:
			if (*pGraph) {
				(*pGraph)->SetProperties(nID);
				m_WeJustReadTheProperties = TRUE;
			}
			break;
		default:
			// Use current user settings
			;
		}
		CreateOneGraph(&nID, pGraph);
		if (!nID) {
			return;
		}
	}

	// set caption style
	(*pGraph)->SetCaptionStyle(GetMainFrame().GetCaptionStyle());
	// set the magnify factor
	(*pGraph)->SetMagnify(m_fMagnify);
	(*pGraph)->ShowTranscriptionBoundaries(m_bTranscriptionBoundaries);

	switch (nID) {

	case IDD_RAWDATA:
		// Set properties for raw data graphs
		(*pGraph)->ShowGrid(TRUE);
		if (!m_WeJustReadTheProperties) {
			(*pGraph)->ShowLegend(!m_bLegendNone);
			(*pGraph)->ShowXScale(!m_bXScaleNone);
			for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
				(*pGraph)->ShowAnnotation((EAnnotation)i, !m_abAnnNone[i]);
			}
		}
		break;

	case IDD_POA:
		// do nothing for POA
		break;

	default:
		// everything else....
		(*pGraph)->ShowGrid(TRUE);
		if (!m_WeJustReadTheProperties) {
			(*pGraph)->ShowLegend(m_bLegendAll);
			(*pGraph)->ShowXScale(m_bXScaleAll);
			for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
				(*pGraph)->ShowAnnotation((EAnnotation)i, m_abAnnAll[i]);
			}
		}
		break;
	}
}

/***************************************************************************/
// CSaView::SendPlayMessage Send specified IDC_PLAY message to player.
/***************************************************************************/
void CSaView::SendPlayMessage(WORD Int1, WORD Int2) {

	DWORD lParam = MAKELONG(Int1, Int2);
	// send message to start player
	GetMainFrame().SendMessage(WM_USER_PLAYER, CDlgPlayer::PLAYING, lParam);
}

/***************************************************************************/
// CSaView::OnPlaybackCursors Playback between cursors
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackCursors() {
	// send message to start player
	SendPlayMessage(ID_PLAYBACK_CURSORS, FALSE);
}

/***************************************************************************/
// CSaView::OnPlaybackSegment Playback current selected segment
/***************************************************************************/
DWORD CSaView::OnPlaybackSegment() {

	DWORD dwStart = GetStartCursorPosition();
	DWORD dwStop = GetStopCursorPosition();

	CSegment * pSelected = FindSelectedAnnotation();
	if (pSelected != NULL) {
		// Adjust Cursors to Current Boundaries
		int nSelection = pSelected->GetSelection();
		// set cursor to segment boundaries
		SetStartCursorPosition(pSelected->GetOffset(nSelection));
		SetStopCursorPosition(pSelected->GetStop(nSelection));
	}

	DWORD dwReturn = GetStopCursorPosition() - GetStartCursorPosition();

	OnPlaybackCursors();

	// return cursors
	SetStartCursorPosition(dwStart);
	SetStopCursorPosition(dwStop);

	return dwReturn;
}

/***************************************************************************/
// CSaView::OnPlaybackWord Playback current selected word
/***************************************************************************/
DWORD CSaView::OnPlaybackWord() {

	// find actual gloss segment for playback
	CSaDoc * pModel = GetDocument();
	if (pModel == NULL) {
		//no document
		return 0;
	}
	CGlossSegment * pGloss = (CGlossSegment *)GetAnnotation(GLOSS);
	CSegment * pSelected = FindSelectedAnnotation();
	int nActualGloss = -1;
	int nSelection = -1;
	DWORD dwStart = GetStartCursorPosition();
	DWORD dwStop = GetStopCursorPosition();

	if (pSelected != NULL) {
		nSelection = pSelected->GetSelection();
		nActualGloss = pGloss->FindFromPosition(pSelected->GetOffset(nSelection), TRUE);
	}
	if (nActualGloss == -1) {
		// nothing within, check if there is gloss
		if (pGloss->GetOffsetSize() > 0) {
			// there is gloss, so the segment must be below the first gloss
			// select first gloss
			ToggleSegmentSelection(pGloss, 0);
			// playback below start cursor
			SetStartCursorPosition(pGloss->GetOffset(0));
			SetStopCursorPosition(pGloss->GetOffset(0) + pGloss->GetDuration(0));
		} else {
			// there is no gloss, playback the whole file
			OnPlaybackFile();
			return pModel->GetDataSize();
		}
	} else {
		if (pGloss->GetSelection() == nActualGloss) {
			ToggleSegmentSelection(pGloss, -1);
		}
		ToggleSegmentSelection(pGloss, nActualGloss);
	}

	DWORD dwReturn = GetStopCursorPosition() - GetStartCursorPosition();

	// playback
	OnPlaybackCursors();

	ToggleSegmentSelection(pGloss, -1);

	// set back actual segment
	if (nSelection != -1) {
		// Select segment (do not toggle off.)
		if (pSelected->GetSelection() != nSelection) {
			ToggleSegmentSelection(pSelected, nSelection);
		}
	}
	// return cursors
	SetStartCursorPosition(dwStart);
	SetStopCursorPosition(dwStop);

	return dwReturn;
}

/***************************************************************************/
// CSaView::OnPlaybackPhraseL1 Playback current selected phrase
/***************************************************************************/
DWORD CSaView::OnPlaybackPhraseL1() {

	// find actual gloss segment for playback
	CSaDoc * pModel = GetDocument();
	if (!pModel) {
		//no document
		return 0;
	}
	CSegment * pSegment = GetAnnotation(MUSIC_PL1);
	CSegment * pSelected = FindSelectedAnnotation();
	int nActualPhrase = -1;
	int nSelection = -1;
	DWORD dwStart = GetStartCursorPosition();
	DWORD dwStop = GetStopCursorPosition();

	if (pSelected != NULL) {
		nSelection = pSelected->GetSelection();
		nActualPhrase = pSegment->FindFromPosition(pSelected->GetOffset(nSelection), TRUE);
	}
	if (nActualPhrase == -1) {
		// nothing within, check if there is phrase
		if (pSegment->GetOffsetSize() > 0) {
			// there is phrase, so the segment must be below the first phrase
			// select first phrase
			ToggleSegmentSelection(pSegment, 0);
			// playback below start cursor
			SetStartCursorPosition(0);
			SetStopCursorPosition(pSegment->GetOffset(0));
		} else {
			// there is no gloss, playback the whole file
			OnPlaybackFile();
			return pModel->GetDataSize();
		}
	} else {
		if (pSegment->GetSelection() == nActualPhrase) {
			ToggleSegmentSelection(pSegment, -1);
		}
		ToggleSegmentSelection(pSegment, nActualPhrase);
	}

	DWORD dwReturn = GetStopCursorPosition() - GetStartCursorPosition();

	// playback
	OnPlaybackCursors();

	ToggleSegmentSelection(pSegment, -1);

	// set back actual segment
	if (nSelection != -1) {
		// Select segment (do not toggle off.)
		if (pSelected->GetSelection() != nSelection) {
			ToggleSegmentSelection(pSelected, nSelection);
		}
	}
	// return cursors
	SetStartCursorPosition(dwStart);
	SetStopCursorPosition(dwStop);

	return dwReturn;
}

// SDM 1.5Test10.2
/***************************************************************************/
// CSaView::OnPlaybackSlow Slowly playback between cursors
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackSlow() {

	enum {
		Player_Slow = 25
	};

	CFnKeys * pKeys = GetMainFrame().GetFnKeys(0);
	// TRUE, if playback repeat enabled
	pKeys->bRepeat[Player_Slow] = FALSE;
	// repeat delay time in ms
	pKeys->nDelay[Player_Slow] = 100;
	// replay mode
	pKeys->nMode[Player_Slow] = ID_PLAYBACK_CURSORS;
	// default replay speed in %
	pKeys->nSpeed[Player_Slow] = 50;
	// default play volume in %
	pKeys->nVolume[Player_Slow] = 50;
	GetMainFrame().PostMessage(WM_USER_PLAYER, CDlgPlayer::PLAYING, MAKELONG(Player_Slow, -1));
}

/***************************************************************************/
// CSaView::OnPlaybackFile Playback the hole wave file
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackFile() {

	// send message to start player
	SendPlayMessage(ID_PLAYBACK_FILE, FALSE);
}

/***************************************************************************/
// CSaView::OnPlaybackWindow Playback the displayed data
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackWindow() {

	// send message to start player
	SendPlayMessage(ID_PLAYBACK_WINDOW, FALSE);
}

/***************************************************************************/
// CSaView::OnPlaybackLeftToStart Playback left window border to start cursor
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackLeftToStart() {

	// send message to start player
	SendPlayMessage(ID_PLAYBACK_LEFTWIN_TO_STARTCUR, FALSE);
}

/***************************************************************************/
// CSaView::OnPlaybackStartToRight Playback start cursor to right window border
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackStartToRight() {

	// send message to start player
	SendPlayMessage(ID_PLAYBACK_STARTCUR_TO_RIGHTWIN, FALSE);
}

/***************************************************************************/
// CSaView::OnPlaybackLeftToStop Playback left window border to stop cursor
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackLeftToStop() {

	// send message to start player
	SendPlayMessage(ID_PLAYBACK_LEFTWIN_TO_ENDCUR, FALSE);
}

/***************************************************************************/
// CSaView::OnPlaybackBeginCursor Playback start cursor to end of file
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackBeginCursor() {

	// send message to start player
	SendPlayMessage(ID_PLAYBACK_STARTCUR_TO_FILEEND, FALSE);
}

/***************************************************************************/
// CSaView::OnPlaybackEndCursor Playback stop cursor to end of file
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackEndCursor() {

	// send message to start player
	SendPlayMessage(ID_PLAYBACK_ENCCUR_TO_FILEEND, FALSE);
}

/***************************************************************************/
// CSaView::OnPlaybackStopToRight Playback stop cursor to right window border
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackStopToRight() {
	SendPlayMessage(ID_PLAYBACK_ENDCUR_TO_RIGHTWIN, FALSE); // send message to start player
}

/***************************************************************************/
// CSaView::OnPlayerPause Pause the player
// The mainframe is informed and it will pause the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE). If the submode is -1, it stays as it was before.
/***************************************************************************/
void CSaView::OnPlayerPause() {
	// send message to pause player
	GetMainFrame().SendMessage(WM_USER_PLAYER, CDlgPlayer::PAUSED, MAKELONG(-1, FALSE));
}

/***************************************************************************/
// CSaView::OnUpdatePlayerPause Menu update
/***************************************************************************/
void CSaView::OnUpdatePlayerPause(CCmdUI * pCmdUI) {
	// enable if player is playing
	pCmdUI->Enable(GetMainFrame().IsPlayerPlaying());
}

/***************************************************************************/
// CSaView::OnPlayerResume Resume the player
// The mainframe is informed and it will resume the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE). 
// If the submode is -1, it stays as it was before.
/***************************************************************************/
void CSaView::OnPlayerResume() {

	if (GetMainFrame().IsPlayerPaused()) {
		// send message to start player
		SendPlayMessage(ID_PLAYBACK_FILE, FALSE);
	}
}

/***************************************************************************/
// CSaView::OnUpdatePlayerStop Menu update
/***************************************************************************/
void CSaView::OnUpdatePlayerResume(CCmdUI * pCmdUI) {

	if (GetMainFrame().IsPlayerPaused()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

/***************************************************************************/
// CSaView::OnPlayerToggle Pause or Resume the player
// The mainframe is informed and it will pause or resume the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE). 
// If the submode is -1, it stays as it was before.
/***************************************************************************/
void CSaView::OnPlayerToggle() {

	if (GetMainFrame().IsPlayerPlaying()) {
		// send message to pause or resume player
		GetMainFrame().SendMessage(WM_USER_PLAYER, CDlgPlayer::STOPPED, MAKELONG(-1, FALSE));
	} else {
		// send message to start player
		SendPlayMessage(ID_PLAYBACK_STARTCUR_TO_FILEEND, FALSE);
	}
}

/***************************************************************************/
// CSaView::OnUpdatePlayerStop Menu update
/***************************************************************************/
void CSaView::OnUpdatePlayerToggle(CCmdUI * pCmdUI) {

	if (GetMainFrame().IsPlayerPlaying()) {
		pCmdUI->Enable(TRUE);
	} else if (GetMainFrame().IsPlayerPaused()) {
		pCmdUI->Enable(TRUE);
	} else {
		if (GetDocument()->GetDataSize() == 0) {
			pCmdUI->Enable(FALSE);
			return;
		}
		CDlgPlayer * pPlayer = GetMainFrame().GetPlayer(false);
		if (pPlayer == NULL) {
			pCmdUI->Enable(TRUE);
			return;
		}
		if (pPlayer->IsPaused()) {
			if (pCmdUI->m_nID != pPlayer->GetSubmode()) {
				pCmdUI->Enable(FALSE);
				return;
			}
		}

		if (pPlayer->IsPlaying()) {
			pCmdUI->Enable(FALSE);
			return;
		}

		pCmdUI->Enable(TRUE);
	}
}

/***************************************************************************/
// CSaView::OnPlayerStop Stop the player
// The mainframe is informed and it will stop the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE). If the submode is -1, it stays as it was before.
// ## Under construction!
/***************************************************************************/
void CSaView::OnPlayerStop() {
	// send message to stop player
	GetMainFrame().SendMessage(WM_USER_PLAYER, CDlgPlayer::STOPPED, MAKELONG(-1, FALSE));
}

/***************************************************************************/
// CSaView::OnUpdatePlayerStop Menu update
/***************************************************************************/
void CSaView::OnUpdatePlayerStop(CCmdUI * pCmdUI) {
	// enable if player is playing
	pCmdUI->Enable((GetMainFrame().IsPlayerPlaying()) || (GetMainFrame().IsPlayerPaused()));
}

/***************************************************************************/
// CSaView::OnPlayer Launch the player
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
// ## Under construction!
/***************************************************************************/
void CSaView::OnPlayer() {
	GetMainFrame().SendMessage(WM_USER_PLAYER, CDlgPlayer::STOPPED, MAKELONG(-1, TRUE)); // send message to start player
}

/***************************************************************************/
// CSaView::OnLayout Change the layout type
/***************************************************************************/
void CSaView::OnLayout(UINT nID) {
	ChangeLayout(nID);
	OnGraphsRetile(); // retile graphs
}

/***************************************************************************/
// CSaView::OnUpdateLayout Menu update
/***************************************************************************/
void CSaView::OnUpdateLayout(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_nLayout == pCmdUI->m_nID); // m_nID is the menu ID
}

/***************************************************************************/
// CSaView::OnExportXML Export wave file data using Extensible Markup Language
/***************************************************************************/
void CSaView::OnExportXML() {
	CDlgExportXML dlg(((CSaDoc *)GetDocument())->GetTitle());
	dlg.DoModal();
}

/***************************************************************************/
// CSaView::OnExportLift Export wave file data using Lexicon Interchange Format
/***************************************************************************/
void CSaView::OnExportLift() {

	CSaDoc * pModel = GetDocument();
	int count = pModel->GetSegmentSize(REFERENCE);
	if (count == 0) {
		ErrorMessage(IDS_ERROR_NO_REFERENCE);
		return;
	}

	CSaString title = pModel->GetTitle();
	BOOL gloss = pModel->HasSegmentData(GLOSS);
	BOOL glossNat = pModel->HasSegmentData(GLOSS_NAT);
	BOOL ortho = pModel->HasSegmentData(ORTHO);
	BOOL phonemic = pModel->HasSegmentData(PHONEMIC);
	BOOL phonetic = pModel->HasSegmentData(PHONETIC);
	BOOL reference = pModel->HasSegmentData(REFERENCE);

	wstring path = GetApp().m_pszExeName;
	path.append(L".exe");
	HMODULE hmod = GetModuleHandle(path.c_str());

	CString fullPath;
	DWORD pathLen = ::GetModuleFileName(hmod, fullPath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);  // hmod of zero gets the main EXE
	fullPath.ReleaseBuffer(pathLen);
	int pos2 = fullPath.ReverseFind('\\');
	fullPath = fullPath.Left(pos2);

	map<wstring, wstring> codes;
	ExtractCountryCodes(fullPath, codes);

	// determine the export directory
	CSaApp * pApp = (CSaApp*)AfxGetApp();
	CString exportDir = pApp->GetProfileString(L"Lift", L"LastExport", L"");;
	if (exportDir.GetLength() == 0) {
		// Set the DataLocation path and write it to the registry
		exportDir = GetShellFolderPath(CSIDL_PERSONAL);
		if (exportDir.Right(1) != "\\") {
			exportDir += _T("\\");
		}
		exportDir += _T("Speech Analyzer");
	}
	if (exportDir.Right(1) == "\\") {
		exportDir = exportDir.Left(exportDir.GetLength() - 1);
	}

	CDlgExportLift dlg(title, exportDir, gloss, glossNat, ortho, phonemic, phonetic, reference, codes);
	if (dlg.DoModal() == IDOK) {
		pApp->WriteProfileStringW(L"Lift", L"LastExport", dlg.settings.szPath);
		pModel->DoExportLift(dlg.settings);
	}
}

void CSaView::ExtractCountryCodes(LPCTSTR fullPath, map<wstring, wstring> & codes) {

	wstring filepath = (LPCTSTR)fullPath;
	filepath.append(L"\\");
	filepath.append(L"iso639.txt");

	// Read UTF-8 file into wstring
	// https://stackoverflow.com/questions/4775437/read-unicode-utf-8-file-into-wstring
	const std::locale empty_locale = std::locale::empty();
	typedef std::codecvt_utf8<wchar_t> converter_type;
	const converter_type* converter = new converter_type;
	const std::locale utf8_locale = std::locale(empty_locale, converter);
	wifstream stream(filepath);
	if (stream) {
		stream.imbue(utf8_locale);
		wstring line;
		while (std::getline(stream, line)) {
			vector<wstring> tokens = CTextHelper::Tokenize(line, L"|");
			if (tokens.size() > 3) {
				// we only extract if both the BCP-47 tag and the name are present
				if ((tokens[2].length() > 0) && (tokens[3].length() > 0)) {
					codes[tokens[3]] = tokens[2];
				}
			}
		}
	}	else {
		GetApp().ErrorMessage(IDS_NO_ISO, filepath.c_str());
		return;
	}
}

/***************************************************************************/
// CSaView::OnExportFW Export wave file data using Standard Format Markers
//
// Modified on 07/27/2000
/***************************************************************************/
void CSaView::OnExportFW() {
	CSaDoc * pModel = GetDocument();
	int count = pModel->GetSegmentSize(REFERENCE);
	if (count == 0) {
		ErrorMessage(IDS_ERROR_NO_REFERENCE);
		return;
	}

	CSaString title = pModel->GetTitle();
	BOOL gloss = pModel->HasSegmentData(GLOSS);
	BOOL glossNat = pModel->HasSegmentData(GLOSS_NAT);
	BOOL ortho = pModel->HasSegmentData(ORTHO);
	BOOL tone = pModel->HasSegmentData(TONE);
	BOOL phonemic = pModel->HasSegmentData(PHONEMIC);
	BOOL phonetic = pModel->HasSegmentData(PHONETIC);
	BOOL reference = pModel->HasSegmentData(REFERENCE);
	BOOL phrase = pModel->HasSegmentData(MUSIC_PL1) | pModel->HasSegmentData(MUSIC_PL1);

	CDlgExportFW dlg(title, gloss, glossNat, ortho, tone, phonemic, phonetic, reference, phrase);
	if (dlg.DoModal() == IDOK) {
		pModel->DoExportFieldWorks(dlg.settings);
	}
}

/***************************************************************************/
// CSaView::OnExportSFM Export wave file data using Standard Format Markers
//
// Modified on 07/27/2000
/***************************************************************************/
void CSaView::OnExportSFM() {
	CSaDoc * pModel = GetDocument();
	CDlgExportSFM dlg(pModel->GetTitle());
	dlg.DoModal();
}

/***************************************************************************/
// CSaView::OnExportTimeTable Export wave file data as time table
/***************************************************************************/
void CSaView::OnExportTimeTable() {

	CDlgExportTimeTable dlg;
	if (dlg.DoModal() == IDOK) {
		CSaDoc * pModel = GetDocument();
		wstring filename;
		int result = pModel->GetSaveAsFilename(pModel->GetTitle(), _T("SFM Time Table (*.sft) |*.sft||"), _T("sft"), NULL, filename);
		if (result != IDOK) {
			return;
		}
		pModel->ExportTimeTable(filename.c_str(),
			dlg.m_bF1,
			dlg.m_bF2,
			dlg.m_bF3,
			dlg.m_bF4,
			dlg.m_bGloss,
			dlg.m_bGlossNat,
			dlg.m_szIntervalTime,
			dlg.m_bSegmentLength,
			dlg.m_bMagnitude,
			dlg.m_bOrtho,
			dlg.m_bPhonemic,
			dlg.m_bPhonetic,
			dlg.m_bPitch,
			dlg.m_bReference,
			dlg.m_bSegmentStart,
			dlg.m_bSampleTime,
			dlg.m_bTone,
			dlg.m_bZeroCrossings,
			dlg.m_nSampleRate,
			dlg.m_nCalculationMethod,
			(dlg.m_nRegion != 0),
			dlg.m_bMelogram);

	}
}

/***************************************************************************/
// CSaView::OnExportMIDI Export MIDI file data
/***************************************************************************/
void CSaView::OnExportMIDI() {

	///// RLJ 06/01/2000
	/////
	///// THIS NEEDS TO BE REPLACED WITH AN INTERFACE TO TIM'S PARTITURE.CPP CODE
	///// FOR THIS.

	// Get Export File Type and Name
	/*
	CSaDoc* pModel = (CSaDoc*)GetDocument();
	CSaString szTitle;
	szTitle = pModel->GetTitle();                                  // load file name
	int nFind = szTitle.Find(':');
	if (nFind != -1)
	szTitle = szTitle.Left(nFind);              // extract part left of :
	nFind = szTitle.ReverseFind('.');
	if (nFind >= ((szTitle.GetLength() > 3) ? (szTitle.GetLength()-4) : 0))
	szTitle = szTitle.Left(nFind);               // remove extension
	CSaString szFilter = "MIDI Format (*.mid) |*.mid||";
	CFileDialog dlgFile(FALSE,"mid",szTitle,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,szFilter,NULL);
	if (dlgFile.DoModal()!=IDOK)
	return;

	CSaString szPath = dlgFile.GetPathName();
	CFile* pFile = new CFile(szPath, CFile::modeCreate|CFile::modeWrite);
	CSaString szString;
	CSaString szCrLf = "\r\n";

	szString = "Wave file is " + pModel->GetPathName() + szCrLf;
	pFile->Write(szString,szString.GetLength());

	if (pFile)
	delete pFile;
	*/
}

/***************************************************************************/
// CSaView::OnImportSFM Import wave file data
/***************************************************************************/
void CSaView::OnImportSFM() {
	// Get Export File Type and Name
	// load file name
	CSaString szTitle = ((CSaDoc *)GetDocument())->GetFilenameFromTitle().c_str();
	int nFind = szTitle.ReverseFind('.');
	if (nFind >= ((szTitle.GetLength() > 3) ? (szTitle.GetLength() - 4) : 0)) {
		szTitle = szTitle.Left(nFind);    // remove extension
	}
	CSaString szFilter = "Standard Format (*.sfm)(*.txt)|*.sfm;*.txt|All Files (*.*) |*.*||";
	CFileDialog dlgFile(TRUE, _T("sfm"), szTitle, OFN_HIDEREADONLY, szFilter, NULL);
	if (dlgFile.DoModal() != IDOK) {
		return;
	}

	CSaString szPath = dlgFile.GetPathName();

	CFileEncodingHelper feh(szPath);
	if (!feh.CheckEncoding(true)) {
		return;
	}

	CImportSFM import(szPath);
	import.Import();
}

/***************************************************************************/
// CSaView::OnImportELAN Import wave file data from ELAN transcription
/***************************************************************************/
void CSaView::OnImportELAN() {
	// Get Export File Type and Name
	CSaString szTitle;
	szTitle = ((CSaDoc *)GetDocument())->GetFilenameFromTitle().c_str(); // load file name
	int nFind = szTitle.ReverseFind('.');
	if (nFind >= ((szTitle.GetLength() > 3) ? (szTitle.GetLength() - 4) : 0)) {
		szTitle = szTitle.Left(nFind);    // remove extension
	}
	CSaString szFilter = "ELAN Format (*.eaf)|*.eaf||";
	CFileDialog dlgFile(TRUE, _T("eaf"), szTitle, OFN_HIDEREADONLY, szFilter, NULL);
	if (dlgFile.DoModal() != IDOK) {
		return;
	}

	CSaString szPath = dlgFile.GetPathName();

	CFileEncodingHelper feh(szPath);
	if (!feh.CheckEncoding(true)) {
		return;
	}

	CImportELAN import(szPath);
	import.Import();
}

/***************************************************************************/
// CSaView::ImportSFT Import wave file data
/***************************************************************************/
void CSaView::OnImportSFT() {

	// Get Export File Type and Name
	// load file name
	CSaString szTitle = ((CSaDoc *)GetDocument())->GetFilenameFromTitle().c_str();
	int nFind = szTitle.ReverseFind('.');
	if (nFind >= ((szTitle.GetLength() > 3) ? (szTitle.GetLength() - 4) : 0)) {
		// remove extension
		szTitle = szTitle.Left(nFind);
	}

	CSaString szFilter = "Standard Format (*.sft) |*.sft||";
	CFileDialog dlgFile(TRUE, _T("sft"), szTitle, OFN_HIDEREADONLY, szFilter, NULL);
	if (dlgFile.DoModal() != IDOK) {
		return;
	}

	CSaString szPath = dlgFile.GetPathName();

	CImportSFM import(szPath);
	import.Import();
}

void CSaView::OnFilePhonologyAssistant() {

	if (AfxMessageBox(IDS_ENABLE_PA, MB_OKCANCEL, 0) != IDOK) {
		return;
	}

	OSVERSIONINFO OSInfo;
	ZeroMemory(&OSInfo, sizeof(OSVERSIONINFO));
	OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&OSInfo);
	bool vista = (OSInfo.dwMajorVersion >= 6);

	CSaApp * pApp = (CSaApp*)AfxGetApp();

	/*
	WOW64 is the x86 emulator that allows 32-bit Windows-based applications to run seamlessly on 64-bit Windows.
	WOW64 is provided with the operating system and does not have to be explicitly enabled.
	For more information, see WOW64 Implementation Details. http://msdn.microsoft.com/en-us/library/aa384274%28v=vs.85%29.aspx
	*/
	BOOL wow64 = FALSE;
	::IsWow64Process(GetCurrentProcess(), &wow64);

	TRACE("running Vista or Greater = %d\n", vista);
	TRACE("running on x64 = %d\n", wow64);

	// obtain the location of the currently running app.
	CString exeName = L"";
	exeName.Append(pApp->m_pszExeName);
	exeName.Append(L".exe");
	HMODULE hmod = GetModuleHandle(exeName);

	TCHAR fullPath[_MAX_PATH + 1];
	GetModuleFileName(hmod, fullPath, MAX_PATH);

	// write to both the 32-bit and 64-bit hives...

	// KEY_WOW64_64KEY allows us to write to the 64-bit hive
	// wow64 is true if we are a 32-bit app on a 64-bit system.
	// we now write to the 64-bit hive
	if (wow64) {
		REGSAM sam = KEY_ALL_ACCESS | KEY_WOW64_64KEY;
		HKEY hKey = 0;
		DWORD disposition = 0;
		LONG lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\SIL\\Speech Analyzer"), 0, NULL, 0, sam, NULL, &hKey, &disposition);
		if (lResult != ERROR_SUCCESS) {
			AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK | MB_ICONEXCLAMATION, 0);
			return;
		}
		DWORD len = (wcslen(fullPath) + 1)*sizeof(TCHAR);
		lResult = RegSetValueEx(hKey, L"Location", 0, REG_SZ, (const BYTE *)fullPath, len);
		if (lResult != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK | MB_ICONEXCLAMATION, 0);
			return;
		}
		lResult = RegCloseKey(hKey);
		if (lResult != ERROR_SUCCESS) {
			AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK | MB_ICONEXCLAMATION, 0);
			return;
		}
	}

	// we now write to the 32-bit hive
	REGSAM sam = KEY_ALL_ACCESS;
	HKEY hKey = 0;
	DWORD disposition = 0;
	LONG lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\SIL\\Speech Analyzer"), 0, NULL, 0, sam, NULL, &hKey, &disposition);
	if (lResult != ERROR_SUCCESS) {
		AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK | MB_ICONEXCLAMATION, 0);
		return;
	}
	DWORD len = (wcslen(fullPath) + 1)*sizeof(TCHAR);
	lResult = RegSetValueEx(hKey, L"Location", 0, REG_SZ, (const BYTE *)fullPath, len);
	if (lResult != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK | MB_ICONEXCLAMATION, 0);
		return;
	}
	lResult = RegCloseKey(hKey);
	if (lResult != ERROR_SUCCESS) {
		AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK | MB_ICONEXCLAMATION, 0);
		return;
	}
}

/***************************************************************************/
// CSaView::OnFileInformation File information
/***************************************************************************/

void CSaView::OnFileInformation() {
	CSaDoc * pModel = (CSaDoc *)GetDocument();
	SourceParm * pSourceParm = pModel->GetSourceParm();
	CSaString szCaption, szTitle;
	szCaption.LoadString(IDS_DLGTITLE_FILEINFO);                // load caption string
	szTitle = pModel->GetFilenameFromTitle().c_str();                      // load file name
	szCaption += " - " + szTitle;                               // build new caption string
	CDlgFileInformation dlg(szCaption, NULL, 0);                // file information dialog
	// set file description string
	dlg.m_dlgUserPage.m_szFileDesc = pModel->GetDescription();
	dlg.m_dlgUserPage.m_szFreeTranslation = pSourceParm->szFreeTranslation;
	if (dlg.DoModal() == IDOK) {
		// get new file description string
		BOOL modified = FALSE;
		if (!pModel->MatchesDescription(dlg.m_dlgUserPage.m_szFileDesc)) {
			modified = TRUE;
		}
		if (pSourceParm->szFreeTranslation != dlg.m_dlgUserPage.m_szFreeTranslation) {
			modified = TRUE;
		}

		BOOL bCountryChanged = (pSourceParm->szCountry != dlg.m_dlgSourcePage.m_szCountry);
		BOOL bDailectChanged = (pSourceParm->szDialect != dlg.m_dlgSourcePage.m_szDialect);
		BOOL bEthnoIDChanged = (pSourceParm->szEthnoID != dlg.m_dlgSourcePage.m_szEthnoID);
		BOOL bFamilyChanged = (pSourceParm->szFamily != dlg.m_dlgSourcePage.m_szFamily);
		BOOL bLanguageChanged = (pSourceParm->szLanguage != dlg.m_dlgSourcePage.m_szLanguage);
		BOOL bGenderChanged = (pSourceParm->nGender != dlg.m_dlgSourcePage.m_nGender);
		BOOL bRegionChanged = (pSourceParm->szRegion != dlg.m_dlgSourcePage.m_szRegion);
		BOOL bSpeakerChanged = (pSourceParm->szSpeaker != dlg.m_dlgSourcePage.m_szSpeaker);
		BOOL bReferenceChanged = (pSourceParm->szReference != dlg.m_dlgSourcePage.m_szReference);
		BOOL bTranscriberChanged = (pSourceParm->szTranscriber != dlg.m_dlgSourcePage.m_szTranscriber);
		if (bCountryChanged || bDailectChanged || bEthnoIDChanged || bFamilyChanged || bLanguageChanged ||
			bGenderChanged || bRegionChanged || bSpeakerChanged || bReferenceChanged || bTranscriberChanged) {
			modified = TRUE;
		}

		if (modified) {
			pModel->CheckPoint();
		}


		if (!pModel->MatchesDescription(dlg.m_dlgUserPage.m_szFileDesc)) {
			pModel->SetDescription(dlg.m_dlgUserPage.m_szFileDesc);
			pModel->SetModifiedFlag(TRUE);                        // document has been modified
			pModel->SetTransModifiedFlag(TRUE);                   // transcription data has been modified
		}
		if (pSourceParm->szFreeTranslation != dlg.m_dlgUserPage.m_szFreeTranslation) {
			pSourceParm->szFreeTranslation = dlg.m_dlgUserPage.m_szFreeTranslation;
			pModel->SetModifiedFlag(TRUE);                        // document has been modified
			pModel->SetTransModifiedFlag(TRUE);                   // transcription data has been modified
		}


		if (bCountryChanged || bDailectChanged || bEthnoIDChanged || bFamilyChanged || bLanguageChanged ||
			bGenderChanged || bRegionChanged || bSpeakerChanged || bReferenceChanged || bTranscriberChanged) {
			pSourceParm->szCountry = dlg.m_dlgSourcePage.m_szCountry;
			pSourceParm->szDialect = dlg.m_dlgSourcePage.m_szDialect;
			if (dlg.m_dlgSourcePage.m_szEthnoID.GetLength() < 3) {
				dlg.m_dlgSourcePage.m_szEthnoID += "   ";
			}
			pSourceParm->szEthnoID = dlg.m_dlgSourcePage.m_szEthnoID.Left(3);
			pSourceParm->szFamily = dlg.m_dlgSourcePage.m_szFamily;
			pSourceParm->szLanguage = dlg.m_dlgSourcePage.m_szLanguage;
			pSourceParm->nGender = dlg.m_dlgSourcePage.m_nGender;
			pSourceParm->szRegion = dlg.m_dlgSourcePage.m_szRegion;
			pSourceParm->szSpeaker = dlg.m_dlgSourcePage.m_szSpeaker;
			pSourceParm->szReference = dlg.m_dlgSourcePage.m_szReference;
			pSourceParm->szTranscriber = dlg.m_dlgSourcePage.m_szTranscriber;
			pModel->SetModifiedFlag(TRUE);                        // document has been modified
			pModel->SetTransModifiedFlag(TRUE);                   // transcription data has been modified
		}

		if (bGenderChanged) {
			BroadcastMessage(WM_USER_INFO_GENDERCHANGED, pSourceParm->nGender);
		}
	}
}

/***************************************************************************/
// CSaView::OnGraphsRetile Retile graphs
// Retile the graphs depending on the selected type of layout. A maximized
// graph will first be normalized, retiled and then again maximized. Iconized
// windows will be rearranged. Counting order is from left top to left bottom
// and then right top to right bottom.
/***************************************************************************/
void CSaView::OnGraphsRetile() {
	int nPos = 0;
	BOOL bLastGraph = FALSE;
	CWnd * pMaxWnd = NULL;
	CRect rWnd, rGraph;

	GetClientRect(&rWnd);
	rGraph = rWnd;
	// arrange icons
	ArrangeIconicWindows();
	// find zoomed (maximized) window and set it to normal state
	WINDOWPLACEMENT wpl;

	wpl.length = sizeof(WINDOWPLACEMENT);
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++) {
		if ((m_apGraphs[i] != NULL) && (m_apGraphs[i]->IsZoomed())) {
			// window is maximized
			pMaxWnd = m_apGraphs[i];
			// get window placement information
			pMaxWnd->GetWindowPlacement(&wpl);
			// set to normal
			wpl.showCmd = SW_SHOWNORMAL;
			// set new window placement
			pMaxWnd->SetWindowPlacement(&wpl);
		}
	}

	for (int i = 0; (i < MAX_GRAPHS_NUMBER && !bLastGraph); i++) {
		//********************************************************************
		// If the recording graph is to be displayed then display it and
		// resize the remaining area to accomodate the layout. For example,
		// if the layout is 4b but includes the recording graph, the rec.
		// graph will not be included in the 4 graphs but will sit on top of
		// the grid of 4 graphs.
		//********************************************************************
		if (m_apGraphs[i] && m_anGraphID[i] == IDD_RECORDING) {
			rGraph = rWnd;
			rGraph.bottom = rGraph.bottom / 8;
			if (!m_apGraphs[i]->IsIconic()) {
				m_apGraphs[i]->MoveWindow(rGraph, TRUE);
			}
			rWnd.top = rWnd.bottom / 8;
			continue;
		}

		if ((m_apGraphs[i] != NULL) &&
			(!m_apGraphs[i]->IsIconic()) &&
			(m_anGraphID[i] != IDD_TWC) &&
			(m_anGraphID[i] != IDD_MAGNITUDE)) {

			//********************************************************************
			// Get the rectangle in which to move the graph. Then move it into
			// that rectangle.
			//********************************************************************
			bLastGraph = GetGraphSubRect(&rWnd, &rGraph, nPos++, m_anGraphID);
			if (m_anGraphID[i] == IDD_MELOGRAM) {
				ArrangeMelogramGraphs(&rGraph, &m_anGraphID[0]);
			} else {
				m_apGraphs[i]->MoveWindow(rGraph, TRUE);
			}
		}
	}

	//****************************************************
	// There was a maximized window so maximize it again.
	//****************************************************
	if (pMaxWnd) {
		pMaxWnd->GetWindowPlacement(&wpl);
		wpl.showCmd = SW_SHOWMAXIMIZED;
		pMaxWnd->SetWindowPlacement(&wpl);
	}

	if (!GetFocusedGraphWnd()) {
		SetFocusedGraph(m_apGraphs[0]);
	}

}

/***************************************************************************/
// CSaView::ArrangeMelogramGraphs()
//
// 09/23/2000 - DDO
//
// This function will arrange the melogram graph (and the TWC and Magnitude
// graphs if they exist) into one graph position within the current layout.
// pRect is the rectangle within the view's client area that deliniates the
// section in which the melogram graphs are stuffed.
/***************************************************************************/
void CSaView::ArrangeMelogramGraphs(const CRect * pRect, UINT *) {
	int nMelIdx = GetGraphIndexForIDD(IDD_MELOGRAM);
	if (nMelIdx == -1) {
		return;
	}
	int nTWCIdx = GetGraphIndexForIDD(IDD_TWC);
	int nMagIdx = GetGraphIndexForIDD(IDD_MAGNITUDE);

	CRect rSubGraph = *pRect;

	//*****************************************************
	// If the TWC graph exists then make it's height the
	// full height of the rectangle and it's width is one
	// quarter the width of the rectangle. Then move it
	// to the left side of the rectangle.
	//*****************************************************
	if (nTWCIdx >= 0) {
		rSubGraph.right = pRect->left + pRect->Width() / 4;
		m_apGraphs[nTWCIdx]->MoveWindow(rSubGraph, TRUE);
		rSubGraph.left = rSubGraph.right;
		rSubGraph.right = pRect->right;
	}

	//*****************************************************
	// If the magnitude exists, make it's height one-third
	// the height of the rectangle and place it just to
	// the right of the TWC graph.
	//*****************************************************
	if (nMagIdx >= 0) {
		rSubGraph.top = pRect->top + (pRect->Height() / 4 * 3);
		m_apGraphs[nMagIdx]->MoveWindow(rSubGraph, TRUE);
		rSubGraph.bottom = rSubGraph.top;
		rSubGraph.top = pRect->top;
	}

	m_apGraphs[nMelIdx]->MoveWindow(rSubGraph, TRUE);
}

#define STAFF_IS_FIRST (anGraphID[0]==IDD_STAFF)
#define STAFF_IS_SECOND (anGraphID[1]==IDD_STAFF)
#define STAFF_IS_THIRD (anGraphID[2]==IDD_STAFF)

#define MAXIMUM_STAFF_HEIGHT 188


/***************************************************************************/
// CSaView::GetGraphSubRect()
//
// 09/19/2000 - DDO This function will receive the rectangle of the entire
//                  client area on which graphs are to be drawn. It will
//                  also accept the position of a particular graph in a
//                  specified layout. For example if the layout is 2A the
//                  top graph is position zero and the bottom graph is
//                  position one. If the layout is 3b, the top graph is
//                  zero, the bottom left is one and the bottom right is
//                  two. The postions always increase from left to right
//                  first and then from top to bottom. The rectangle where
//                  the particular graph specified by nPos within the
//                  larger client area is returned.
//
//                  This function will also return TRUE if nPos is the last
//                  graph in the layout.
/***************************************************************************/
BOOL CSaView::GetGraphSubRect(int nLayout, const CRect * pWndRect, CRect * pSubRect, int nPos, const UINT * anIncomingGraphID) {
	double dThirdHeight = pWndRect->Height() / 3.;
	double dHalfHeight = pWndRect->Height() / 2.;
	int nHalfWidth = int(pWndRect->Width() / 2. + 0.5);
	pSubRect->CopyRect(pWndRect);

	CGraphList cList(anIncomingGraphID);

	cList.ClearGraphs(TRUE, FALSE);  // Remove graph which are not related to the layout

	const UINT * anGraphID = cList.GetGraphs();

	switch (nLayout) {
		//**************************************************************
		// One graph on top of another
		//**************************************************************
	case ID_LAYOUT_2A:
		if (STAFF_IS_SECOND) {
			double dHeightOfStaff = min(dThirdHeight, MAXIMUM_STAFF_HEIGHT);

			((nPos == 0) ? pSubRect->bottom : pSubRect->top) = pSubRect->bottom - int(dHeightOfStaff + 0.5);
		} else if (STAFF_IS_FIRST) {
			double dHeightOfStaff = min(dThirdHeight, MAXIMUM_STAFF_HEIGHT);

			((nPos == 0) ? pSubRect->bottom : pSubRect->top) = pSubRect->top + int(dHeightOfStaff + 0.5);
		} else {
			((nPos == 0) ? pSubRect->bottom : pSubRect->top) = pSubRect->top + int(dHalfHeight + 0.5);
		}
		return (nPos == 1);
		break;

		//**************************************************************
		// Two graphs side-by-side
		//**************************************************************
	case ID_LAYOUT_2B:
		((nPos == 0) ? pSubRect->right : pSubRect->left) = pSubRect->left + nHalfWidth;
		return (nPos == 1);
		break;

		//**************************************************************
		// Two graphs stacked vertically the bottom graph larger
		//**************************************************************
	case ID_LAYOUT_2C:
		((nPos == 0) ? pSubRect->bottom : pSubRect->top) = pSubRect->top + int(dThirdHeight + 0.5);
		return (nPos == 1);
		break;

		//**************************************************************
		// Three graphs stacked horizontally
		//**************************************************************
	case ID_LAYOUT_3A:
		if (STAFF_IS_THIRD) {
			double dHeightOfStaff = min(dThirdHeight * 2 / 3, MAXIMUM_STAFF_HEIGHT);

			if (nPos == 0) {
				pSubRect->bottom = pSubRect->top + int(dThirdHeight + 0.5);
			} else if (nPos == 1) {
				pSubRect->bottom -= int(dHeightOfStaff + 0.5);
				pSubRect->top += int(dThirdHeight + 0.5);
			} else {
				pSubRect->top = pSubRect->bottom - int(dHeightOfStaff + 0.5);
			}
		} else if (STAFF_IS_SECOND) {
			double dHeightOfStaff = min(dThirdHeight * 2 / 3, MAXIMUM_STAFF_HEIGHT);

			if (nPos == 0) {
				pSubRect->bottom = pSubRect->top + int((dThirdHeight * 2) - dHeightOfStaff + 0.5);
			} else if (nPos == 1) {
				pSubRect->bottom = pSubRect->top + int(dThirdHeight * 2 + 0.5);
				pSubRect->top = pSubRect->top + int((dThirdHeight * 2) - dHeightOfStaff + 0.5);
			} else {
				pSubRect->top += int(dThirdHeight * 2 + 0.5);
			}
		} else if (STAFF_IS_FIRST) {
			double dHeightOfStaff = min(dThirdHeight * 2 / 3, MAXIMUM_STAFF_HEIGHT);

			if (nPos == 0) {
				pSubRect->bottom = pSubRect->top + int(dHeightOfStaff + 0.5);
			} else if (nPos == 1) {
				pSubRect->bottom = pSubRect->top + int(dThirdHeight * 2 + 0.5);
				pSubRect->top += int(dHeightOfStaff + 0.5);
			} else {
				pSubRect->top += int(dThirdHeight * 2 + 0.5);
			}
		} else {
			if (nPos < 2) {
				pSubRect->bottom = pSubRect->top + int(dThirdHeight * (nPos + 1) + 0.5);
			}
			if (nPos > 0) {
				pSubRect->top = pSubRect->top + int(dThirdHeight * nPos + 0.5);
			}
		}
		return (nPos == 2);
		break;

		//**************************************************************
		// Three graphs, one on top with two below aligned side-by-side
		//**************************************************************
	case ID_LAYOUT_3B:
		((nPos == 0) ? pSubRect->bottom : pSubRect->top) = pSubRect->top + int(dHalfHeight + 0.5);
		if (nPos > 0) {
			((nPos == 1) ? pSubRect->right : pSubRect->left) = pSubRect->left + nHalfWidth;
		}
		return (nPos == 2);
		break;

		//**************************************************************
		// Three graphs, one on top with two below aligned side-by-side
		//**************************************************************
	case ID_LAYOUT_3C:
		((nPos == 0) ? pSubRect->bottom : pSubRect->top) = pSubRect->top + int(dThirdHeight + 0.5);
		if (nPos > 0) {
			((nPos == 1) ? pSubRect->right : pSubRect->left) = pSubRect->left + nHalfWidth;
		}
		return (nPos == 2);
		break;

		//**************************************************************
		// Four graphs stacked horizonatally
		//**************************************************************
	case ID_LAYOUT_4A:
		if (nPos < 3) {
			pSubRect->bottom = pSubRect->top + int(pWndRect->Height() / 4. * (nPos + 1) + 0.5);
		}
		if (nPos > 0) {
			pSubRect->top = pSubRect->top + int(pWndRect->Height() / 4. * nPos + 0.5);
		}
		return (nPos == 3);
		break;

		//**************************************************************
		// Four graphs, two side-by-side stacked on two side-by-side
		//**************************************************************
	case ID_LAYOUT_4B:
		((nPos < 2) ? pSubRect->bottom : pSubRect->top) = pSubRect->top + int(dHalfHeight + 0.5);
		((nPos == 0 || nPos == 2) ? pSubRect->right : pSubRect->left) = pSubRect->left + nHalfWidth;
		return (nPos == 3);
		break;

		//**************************************************************
		// Four graphs, three horizontal slices w/ two in the bottom slice
		//**************************************************************
	case ID_LAYOUT_4C:
		pSubRect->bottom = pSubRect->top + int(dThirdHeight*(nPos > 2 ? 3 : nPos + 1) + 0.5);
		pSubRect->top = pSubRect->top + int(dThirdHeight*(nPos > 2 ? 2 : nPos) + 0.5);
		if (nPos >= 2) {
			(nPos == 2 ? pSubRect->right : pSubRect->left) = pSubRect->left + nHalfWidth;
		}
		return (nPos == 3);
		break;

		//**************************************************************
		// Five graphs, one across the top and four below it arranged
		// like layout 4b
		//**************************************************************
	case ID_LAYOUT_5:
		if (nPos == 0) {
			pSubRect->bottom = pSubRect->top + int(dThirdHeight + 0.5);
		} else {
			pSubRect->top = pSubRect->top + int(dThirdHeight * ((nPos < 3) ? 1 : 2) + 0.5);
			if (nPos < 3) {
				pSubRect->bottom = pSubRect->bottom - int(dThirdHeight + 0.5);
			}
			((nPos == 1 || nPos == 3) ? pSubRect->right : pSubRect->left) = pSubRect->left + nHalfWidth;
		}
		return (nPos == 4);
		break;

		//**************************************************************
		// Six graphs stacked horizontally
		//**************************************************************
	case ID_LAYOUT_6A:
		if (nPos < 5) {
			pSubRect->bottom = pSubRect->top + int(pWndRect->Height() / 6. * (nPos + 1) + 0.5);
		}
		if (nPos > 0) {
			pSubRect->top = pSubRect->top + int(pWndRect->Height() / 6. * nPos + 0.5);
		}
		return (nPos == 5);
		break;

		//**************************************************************
		// Six graphs in a 3 x 2 grid (three rows by two cols.)
		//**************************************************************
	case ID_LAYOUT_6B:
		if (nPos > 1) {
			pSubRect->top = pSubRect->top + int(dThirdHeight * ((nPos < 4) ? 1 : 2) + 0.5);
		}
		if (nPos < 4) {
			pSubRect->bottom = pSubRect->bottom - int(dThirdHeight * ((nPos < 2) ? 2 : 1) + 0.5);
		}
		((nPos == 0 || nPos == 2 || nPos == 4) ? pSubRect->right : pSubRect->left) = pSubRect->left + nHalfWidth;
		return (nPos == 5);
		break;

	default:
		break;
	}

	return TRUE;
}

/***************************************************************************/
// CSaView::OnLegendAll Show the legend window on all graphs
/***************************************************************************/
void CSaView::OnLegendAll() {
	m_bLegendAll = TRUE; // show legend windows in all the graphs
	m_bLegendNone = FALSE;
	// show legend windows
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			m_apGraphs[nLoop]->ShowLegend(TRUE, TRUE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateLegendAll Menu update
/***************************************************************************/
void CSaView::OnUpdateLegendAll(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_bLegendAll);
}

/***************************************************************************/
// CSaView::OnLegendRawdata Show the legend wnd on raw data graph only
/***************************************************************************/
void CSaView::OnLegendRawdata() {
	m_bLegendAll = FALSE; // show legend windows only in raw data graph
	m_bLegendNone = FALSE;
	// hide legend windows except for raw data
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			m_apGraphs[nLoop]->ShowLegend(m_anGraphID[nLoop] == IDD_RAWDATA, TRUE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateLegendRawdata Menu update
/***************************************************************************/
void CSaView::OnUpdateLegendRawdata(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(!(m_bLegendAll || m_bLegendNone));
}

/***************************************************************************/
// CSaView::OnLegendNone Hide legend windows in all the graphs
/***************************************************************************/
void CSaView::OnLegendNone() {
	m_bLegendNone = TRUE; // hide legend windows in all the graphs
	m_bLegendAll = FALSE;
	// hide legend windows
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			m_apGraphs[nLoop]->ShowLegend(FALSE, TRUE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateLegendNone Menu update
/***************************************************************************/
void CSaView::OnUpdateLegendNone(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_bLegendNone);
}

/***************************************************************************/
// CSaView::OnXScaleAll Show the x-scale window on all graphs
/***************************************************************************/
void CSaView::OnXScaleAll() {
	// show x-scale windows in all the graphs
	m_bXScaleAll = TRUE;
	m_bXScaleNone = FALSE;
	// show x-scale windows
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			m_apGraphs[nLoop]->ShowXScale(m_apGraphs[nLoop]->HasCursors(), m_apGraphs[nLoop]->HasCursors());
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateXScaleAll Menu update
/***************************************************************************/
void CSaView::OnUpdateXScaleAll(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_bXScaleAll);
}

/***************************************************************************/
// CSaView::OnXScaleRawdata Show the x-scale wnd on raw data graph only
/***************************************************************************/
void CSaView::OnXScaleRawdata() {
	// show x-scale windows only in raw data graph
	m_bXScaleAll = FALSE;
	m_bXScaleNone = FALSE;
	// hide x-scale windows
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			m_apGraphs[nLoop]->ShowXScale(m_anGraphID[nLoop] == IDD_RAWDATA, TRUE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateXScaleRawdata Menu update
/***************************************************************************/
void CSaView::OnUpdateXScaleRawdata(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(!(m_bXScaleAll || m_bXScaleNone));
}

/***************************************************************************/
// CSaView::OnXScaleNone Hide x-scale windows in all the graphs
/***************************************************************************/
void CSaView::OnXScaleNone() {
	// hide x-scale windows in all the graphs
	m_bXScaleNone = TRUE;
	m_bXScaleAll = FALSE;
	// hide x-scale windows
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->ShowXScale(FALSE, TRUE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateXScaleNone Menu update
/***************************************************************************/
void CSaView::OnUpdateXScaleNone(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_bXScaleNone);
}

/***************************************************************************/
// 09/24/2000 - DDO
/***************************************************************************/
void CSaView::ToggleAnnotation(int nAnnot, BOOL bShow, BOOL bRawDataOnly) {
	if (!bRawDataOnly) {
		//Show annotation in all graphs
		m_abAnnAll[nAnnot] = bShow;
		m_abAnnNone[nAnnot] = !bShow;

		//**************************************************
		// Show annotation window in all but music graphs.
		//**************************************************
		for (int i = 0; i < MAX_GRAPHS_NUMBER; i++) {
			if (m_apGraphs[i]) {
				m_apGraphs[i]->ShowAnnotation((EAnnotation)nAnnot, bShow, TRUE);
			}
		}
	} else {
		// show annotation only in raw data graph
		m_abAnnAll[nAnnot] = FALSE;
		m_abAnnNone[nAnnot] = FALSE;

		//**************************************************
		// Hide annotation window in all graphs.
		//**************************************************
		for (int i = 0; i < MAX_GRAPHS_NUMBER; i++)
			if (m_apGraphs[i]) {
				m_apGraphs[i]->ShowAnnotation((EAnnotation)nAnnot, m_anGraphID[i] == IDD_RAWDATA, TRUE);
			}
	}
}

/***************************************************************************/
// CSaView::OnAnnotationAll Show the Annotation window on all graphs
/***************************************************************************/
void CSaView::OnAnnotationAll(UINT nID) {
	int nAnnotationID = nID - ID_PHONETIC_ALL;
	ToggleAnnotation(nAnnotationID, TRUE);
}

/***************************************************************************/
// CSaView::OnUpdateAnnotationAll Menu update
/***************************************************************************/
void CSaView::OnUpdateAnnotationAll(CCmdUI * pCmdUI) {
	int nAnnotationID = pCmdUI->m_nID - ID_PHONETIC_ALL;
	pCmdUI->SetCheck(m_abAnnAll[nAnnotationID]);
}


/***************************************************************************/
// CSaView::OnAnnotationRawdata Show the Annotation wnd on raw data graph only
/***************************************************************************/
void CSaView::OnAnnotationRawdata(UINT nID) {
	int nAnnotationID = nID - ID_PHONETIC_RAWDATA;
	ToggleAnnotation(nAnnotationID, TRUE, TRUE);
}


/***************************************************************************/
// CSaView::OnUpdateAnnotationRawdata Menu update
/***************************************************************************/
void CSaView::OnUpdateAnnotationRawdata(CCmdUI * pCmdUI) {
	int nAnnotationID = pCmdUI->m_nID - ID_PHONETIC_RAWDATA;
	pCmdUI->SetCheck(!(m_abAnnAll[nAnnotationID] || m_abAnnNone[nAnnotationID]));
}


/***************************************************************************/
// CSaView::OnAnnotationNone Hide Annotation windows in all the graphs
/***************************************************************************/
void CSaView::OnAnnotationNone(UINT nID) {
	int nAnnotationID = nID - ID_PHONETIC_NONE;
	ToggleAnnotation(nAnnotationID, FALSE);
}


/***************************************************************************/
// CSaView::OnUpdateAnnotationNone Menu update
/***************************************************************************/
void CSaView::OnUpdateAnnotationNone(CCmdUI * pCmdUI) {
	int nAnnotationID = pCmdUI->m_nID - ID_PHONETIC_NONE;
	pCmdUI->SetCheck(m_abAnnNone[nAnnotationID]);
}

/***************************************************************************/
// CSaView::OnPopupgraphAnnotation Show or hide Annotation window
/***************************************************************************/
void CSaView::OnPopupgraphAnnotation(UINT nID) {
	EAnnotation nAnnotationID = (EAnnotation)(nID - ID_POPUPGRAPH_PHONETIC);
	ShowAnnotation(nAnnotationID);
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphAnnotation Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphAnnotation(CCmdUI * pCmdUI) {
	int nAnnotationID = pCmdUI->m_nID - ID_POPUPGRAPH_PHONETIC;
	BOOL bEnable = ((m_pFocusedGraph != NULL) &&
		(GetDocument()->GetDataSize() != 0) &&
		((m_pFocusedGraph->HaveAnnotation(REFERENCE)) || (!m_pFocusedGraph->DisableAnnotation(nAnnotationID))) &&
		(m_nFocusedID != IDD_TWC));
	pCmdUI->Enable(bEnable);
	if (m_pFocusedGraph) {
		pCmdUI->SetCheck(m_pFocusedGraph->HaveAnnotation(nAnnotationID));    // check if graph has reference window
	} else {
		pCmdUI->SetCheck(FALSE);
	}
}

void CSaView::OnViewTranscriptionBoundaries() {

	m_bTranscriptionBoundaries = (!m_bTranscriptionBoundaries);
	// refresh the annotation windows
	for (int nGraph = 0; nGraph < MAX_GRAPHS_NUMBER; nGraph++) {
		CGraphWnd * pGraph = GetGraph(nGraph);
		if (pGraph == NULL) continue;
		for (int nWnd = 0; nWnd < ANNOT_WND_NUMBER; nWnd++) {
			pGraph->GetAnnotationWnd(nWnd)->ShowTranscriptionBoundaries(m_bTranscriptionBoundaries);
			pGraph->GetAnnotationWnd(nWnd)->Invalidate(TRUE);
		}
	}
}

void CSaView::OnUpdateViewTranscriptionBoundaries(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_bTranscriptionBoundaries);
}

/***************************************************************************/
// CSaView::OnBoundariesAll Show the boundaries on all graphs
/***************************************************************************/
void CSaView::OnBoundariesAll() {
	// show boundaries
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] == NULL) continue;
		m_apGraphs[nLoop]->SetBoundaries(true);
		m_apGraphs[nLoop]->RedrawPlot();
	}
}

/***************************************************************************/
// CSaView::OnUpdateBoundariesAll Menu update
/***************************************************************************/
void CSaView::OnUpdateBoundariesAll(CCmdUI * pCmdUI) {
	BOOL bShown = TRUE;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] == NULL) continue;
		if (!m_apGraphs[nLoop]->HasBoundaries()) {
			bShown = FALSE;
			break;
		}
	}
	pCmdUI->SetCheck(bShown);
}

/***************************************************************************/
// CSaView::OnBoundariesRawdata Show the boundaries on raw data graph only
/***************************************************************************/
void CSaView::OnBoundariesRawdata() {
	// hide boundaries except for raw data
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] == NULL) continue;
		m_apGraphs[nLoop]->SetBoundaries(m_anGraphID[nLoop] == IDD_RAWDATA);
		m_apGraphs[nLoop]->RedrawPlot();
	}
}

/***************************************************************************/
// CSaView::OnUpdateBoundariesRawdata Menu update
/***************************************************************************/
void CSaView::OnUpdateBoundariesRawdata(CCmdUI * pCmdUI) {
	// check to see if we are only show boundaries on the raw data graph
	BOOL bCondition = TRUE;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] == NULL) continue;
		if (m_anGraphID[nLoop] == IDD_RAWDATA) {
			if (!m_apGraphs[nLoop]->HasBoundaries()) {
				bCondition = FALSE;
			}
		} else {
			if (m_apGraphs[nLoop]->HasBoundaries()) {
				bCondition = FALSE;
			}
		}
	}
	pCmdUI->SetCheck(bCondition);
}

/***************************************************************************/
// CSaView::OnDrawingBoundaries Show or hide boundaries
/***************************************************************************/
void CSaView::OnBoundariesThis() {
	if (m_pFocusedGraph != NULL) {
		m_pFocusedGraph->SetBoundaries(!m_pFocusedGraph->HasBoundaries());
		m_pFocusedGraph->RedrawPlot();
	}
}

/***************************************************************************/
// CSaView::OnUpdateBoundariesThis Menu update
/***************************************************************************/
void CSaView::OnUpdateBoundariesThis(CCmdUI * pCmdUI) {

	pCmdUI->Enable((m_pFocusedGraph != NULL) &&
		(GetDocument()->GetDataSize() != 0) &&       // enable if data is available
		(m_pFocusedGraph->HasCursors()));			// enable if cursors visible

	if (m_pFocusedGraph) {
		// check if graph has boundaries
		pCmdUI->SetCheck(m_pFocusedGraph->HasBoundaries());
	} else {
		pCmdUI->SetCheck(FALSE);
	}
}

/***************************************************************************/
// CSaView::OnBoundariesNone Hide the boundaries on all graphs
/***************************************************************************/
void CSaView::OnBoundariesNone() {
	// hide boundaries
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] == NULL) continue;
		m_apGraphs[nLoop]->SetBoundaries(false);
		m_apGraphs[nLoop]->RedrawPlot();
	}
}

/***************************************************************************/
// CSaView::OnUpdateBoundariesNone Menu update
/***************************************************************************/
void CSaView::OnUpdateBoundariesNone(CCmdUI * pCmdUI) {
	BOOL bShown = FALSE;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] == NULL) continue;
		bShown |= (m_apGraphs[nLoop]->HasBoundaries())?TRUE:FALSE;
	}
	pCmdUI->SetCheck(!bShown);
}

/***************************************************************************/
// CSaView::OnGraphsZoomAll Zoom all
/***************************************************************************/
void CSaView::OnGraphsZoomCursors() {
	GraphsZoomCursors(m_dwStartCursor, m_dwStopCursor, 99);
}

/***************************************************************************/
// CSaView::OnGraphsZoomAll Zoom all
/***************************************************************************/
void CSaView::GraphsZoomCursors(DWORD startReq, DWORD stopReq, DWORD percent) {
	// divide by two to guarantee result even
	DWORD start = ((startReq) / 2);
	DWORD stop = ((stopReq + 1) / 2);
	DWORD offset;

	ASSERT(stop > start);
	ASSERT(percent <= 100);

	offset = (DWORD)((stop - start)*(100 - percent) / 100.0 / 2.0);

	// zoom cursors to fill % of screen
	if (start > offset) {
		start = (start - offset) * 2;
	} else {
		start = 0;
	}

	stop = (stop + offset) * 2;
	if (stop > GetDocument()->GetDataSize()) {
		stop = GetDocument()->GetDataSize();
	}

	SetDataFrame(start, stop - start);

	ZoomIn(0, TRUE);  // Handle Zoom
}


/***************************************************************************/
// CSaView::OnUpdateGraphsZoomCursors Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphsZoomCursors(CCmdUI * pCmdUI) {
	BOOL enable = TRUE;
	if ((GetDocument()->GetDataSize() == 0) ||  // nothing to zoom
		(m_fZoom >= m_fMaxZoom) ||              // zoom limit
		(m_dwStopCursor == m_dwStartCursor)) {  // zoom limit
		enable = FALSE;
	} else {
		// divide by two to guarantee result even
		DWORD start = ((m_dwStartCursor) / 2);
		DWORD stop = ((m_dwStopCursor + 1) / 2);
		DWORD frameStart;
		DWORD frameWidth;
		DWORD offset;

		if (stop <= start) {
			enable = FALSE;
		} else {
			offset = (DWORD)((stop - start)*(100 - 97) / 100.0 / 2.0);

			// zoom cursors to fill 97% of screen
			if (start > offset) {
				start = (start - offset) * 2;
			} else {
				start = 0;
			}

			stop = (stop + offset) * 2;
			if (stop > GetDocument()->GetDataSize()) {
				stop = GetDocument()->GetDataSize();
			}

			GetDataFrame(frameStart, frameWidth);
			if ((frameStart == start) && (frameWidth == (stop - start))) {
				enable = FALSE;
			}
		}
	}

	pCmdUI->Enable(enable);
}

/***************************************************************************/
// CSaView::OnGraphsZoomAll Zoom all
/***************************************************************************/
void CSaView::OnGraphsZoomAll() {
	// no zoom
	m_fZoom = 1.0;
	// Handle Zoom
	ZoomIn(0, TRUE);
}

/***************************************************************************/
// CSaView::OnUpdateGraphsZoomAll Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphsZoomAll(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_fZoom > 1.0);
}

/***************************************************************************/
// CSaView::OnGraphsZoomIn Zoom in
/***************************************************************************/
void CSaView::OnGraphsZoomIn() {
	// double zooming
	ZoomIn(m_fZoom);
}

/***************************************************************************/
// CSaView::OnUpdateGraphsZoomIn Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphsZoomIn(CCmdUI * pCmdUI) {
	CRect rWnd;
	GetClientRect(rWnd);
	if ((GetDocument()->GetDataSize() == 0) // nothing to zoom
		|| (m_fZoom >= m_fMaxZoom)) { // zoom limit
		pCmdUI->Enable(FALSE);
	} else {
		pCmdUI->Enable(TRUE);
	}
}

/***************************************************************************/
// CSaView::OnGraphsZoomOut Zoom out
/***************************************************************************/
void CSaView::OnGraphsZoomOut() {
	ZoomOut(m_fZoom / 2); // divide zoom by two
}

/***************************************************************************/
// CSaView::OnUpdateGraphsZoomOut Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphsZoomOut(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_fZoom > 1.0);
}

/***************************************************************************/
// CSaView::OnHScroll Horizontal scrolling
/***************************************************************************/
void CSaView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar) {

	//TRACE(">>OnHScroll %d %d %d %d\n",nSBCode,nPos,m_dwDataPosition,m_dwScrollLine);
	// get pointer to document
	CSaDoc * pModel = GetDocument();
	// zooming is enabled
	if (m_fZoom > 1.0) {
		// save actual data position
		DWORD dwOldDataPosition = m_dwDataPosition;
		switch (nSBCode) {
		case SB_LEFT:
			// scroll to the leftmost position
			m_dwDataPosition = 0;
			break;
		case SB_LINELEFT:
			// scroll one line left
			if (m_dwDataPosition >= m_dwScrollLine) {
				m_dwDataPosition -= m_dwScrollLine;
			} else {
				m_dwDataPosition = 0;
			}
			break;
		case SB_RIGHT:
			// scroll to the rightmost position
			m_dwDataPosition = pModel->GetDataSize() - GetDataFrame();
			break;
		case SB_LINERIGHT:
			// scroll one line right
			if ((m_dwDataPosition <= (pModel->GetDataSize() - GetDataFrame() - m_dwScrollLine)) &&
				(pModel->GetDataSize() >= (GetDataFrame() + m_dwScrollLine))) {
				m_dwDataPosition += m_dwScrollLine;
			} else {
				m_dwDataPosition = pModel->GetDataSize() - GetDataFrame();
			}
			break;
		case SB_PAGELEFT:
			// scroll one page left
			if (m_dwDataPosition >= GetDataFrame()) {
				m_dwDataPosition -= GetDataFrame();
			} else {
				m_dwDataPosition = 0;
			}
			break;
		case SB_PAGERIGHT:
			// scroll one page right
			if ((m_dwDataPosition <= (pModel->GetDataSize() - 2 * GetDataFrame())) &&
				(pModel->GetDataSize() >= (2 * GetDataFrame()))) {
				m_dwDataPosition += GetDataFrame();
			} else {
				m_dwDataPosition = pModel->GetDataSize() - GetDataFrame();
			}
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			// scroll to position
			m_dwDataPosition = nPos * m_dwHScrollFactor;
			if (m_dwDataPosition > (pModel->GetDataSize() - GetDataFrame())) {
				m_dwDataPosition = pModel->GetDataSize() - GetDataFrame();
			}
			break;
		case SB_ENDSCROLL:
		default:
			break;
		}
		// for 16 bit data value must be even
		if (pModel->Is16Bit()) {
			m_dwDataPosition &= ~1;
		}
		// is scrolling necessary?
		// scroll
		if (dwOldDataPosition != m_dwDataPosition) {
			// set scroll bar
			SetScrollPos(SB_HORZ, (int)(m_dwDataPosition / m_dwHScrollFactor), TRUE);
			// scroll all graph windows (only if the have cursors visible)
			for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
				if (m_apGraphs[nLoop]) {
					m_apGraphs[nLoop]->ScrollGraph(this, m_dwDataPosition, dwOldDataPosition);
				}
			}

			if (GraphIDtoPtr(IDD_RECORDING)) {
				GraphIDtoPtr(IDD_RECORDING)->GetPlot()->RedrawWindow(NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW);
			}
		}
	}
	//TRACE("<<m_dwDataPosition=%lu\n",m_dwDataPosition);
	CView::OnHScroll(nSBCode, nPos, pScrollBar);
	GetMainFrame().SetPlayerTimes();
}

/***************************************************************************/
// CSaView::OnVScroll Vertical scrolling
/***************************************************************************/
void CSaView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar) {
	TRACE("OnVScroll %d %f %f\n", nPos, m_fZoom, m_fVScrollSteps);

	double fZoom = m_fZoom;
	// actual position
	double fActualPos = m_fVScrollSteps / m_fZoom;
	switch (nSBCode) {
	case SB_BOTTOM:
		// zoom maximum
		fZoom = m_fMaxZoom;
		break;
	case SB_LINEDOWN:
		// zoom one step more
		fZoom = m_fVScrollSteps / (fActualPos - 1);
		break;
	case SB_TOP:
		// no zoom
		fZoom = (double)0.5;
		// to be sure it will be set to 1.0 (rounding errors)
		break;
	case SB_LINEUP:
		// zoom one step less
		fZoom = m_fVScrollSteps / (fActualPos + 1);
		break;
	case SB_PAGEDOWN:
		// double zoom
		fZoom = 2.*m_fZoom;
		break;
	case SB_PAGEUP:
		// divide zoom by two
		fZoom = 0.5*m_fZoom;
		break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
	{
		// zoom from position
		SCROLLINFO info;
		GetScrollInfo(SB_VERT, &info, SIF_TRACKPOS);
		nPos = info.nTrackPos;
		fZoom = (m_fVScrollSteps / (m_fVScrollSteps + ZOOM_SCROLL_RESOLUTION - (double)nPos));
		break;
	}
	case SB_ENDSCROLL:
	default:
		break;
	}

	// is zooming necessary?
	if (fZoom != m_fZoom) { // zoom
		if (fZoom > m_fZoom) {
			ZoomIn(fZoom - m_fZoom);
		} else {
			ZoomOut(-(fZoom - m_fZoom));
		}
	}

	CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

/***************************************************************************/
// CSaView::OnSize Sizeing the view
/***************************************************************************/
void CSaView::OnSize(UINT nType, int cx, int cy) {
	CSaDoc * pModel = GetDocument(); // get pointer to document

	CView::OnSize(nType, cx, cy);

	if ((nType == SIZE_MAXIMIZED) || (nType == SIZE_RESTORED)) {
		// check if there is at least one graph
		int nLoop;
		for (nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
			if (m_apGraphs[nLoop]) {
				break;
			}
		}
		if (nLoop >= MAX_GRAPHS_NUMBER) {
			return;    // no graph to retile
		}
		// calculate new scroll parameters
		SetScrolling();
		if (m_fZoom > 1.0) {
			// set horizontal scroll bar
			SetScrollRange(SB_HORZ, 0, (int)((pModel->GetDataSize() - GetDataFrame()) / m_dwHScrollFactor), FALSE);
			SetScrollPos(SB_HORZ, (int)(m_dwDataPosition / m_dwHScrollFactor), TRUE);
		}
		if (GetMainFrame().IsScrollZoom() && (pModel->GetDataSize() > 0)) {
			// set vertical scroll bar
			SetScrollRange(SB_VERT, ZOOM_SCROLL_RESOLUTION, (int)m_fVScrollSteps, FALSE);
			SetScrollPos(SB_VERT, (int)(m_fVScrollSteps + ZOOM_SCROLL_RESOLUTION - m_fVScrollSteps / m_fZoom), TRUE);
		}
		OnGraphsRetile(); // retile graphs
	}
}

/***************************************************************************/
// CSaView::OnGraphDestroyed Graph has been destroyed
// Message from one of the graphs, that he has been destroyed. The view
// erases it from the graph arrays and if it was the last graph it closes
// itself.
/***************************************************************************/
LRESULT CSaView::OnGraphDestroyed(WPARAM, LPARAM lParam) {
	int index;

	//************************************************
	// Get the index in the pointers array of the
	// graph being destroyed. It should always be
	// found or there is big trouble.
	//************************************************
	if ((index = GraphPtrToOffset((CGraphWnd *)lParam)) == -1) {
		return 0;
	}

	DestroyGraph(&(m_apGraphs[index]), TRUE);

	//************************************************
	// if melogram is being destroyed then make
	// sure the TWC and magnitude are also destroyed.
	//************************************************
	if (m_anGraphID[index] == IDD_MELOGRAM) {
		int i = GetGraphIndexForIDD(IDD_TWC);
		if (i >= 0) {
			DestroyGraph(&(m_apGraphs[i]), TRUE);
			m_anGraphID[i] = 0;
		}

		i = GetGraphIndexForIDD(IDD_MAGNITUDE);
		if (i >= 0) {
			DestroyGraph(&(m_apGraphs[i]), TRUE);
			m_anGraphID[i] = 0;
		}
	}
	//**************************************************
	// If the magnitude is being destroyed and the TWC
	// graph exists then redraw the TWC graph so its
	// x-scale window is removed.
	//**************************************************
	else if (m_anGraphID[index] == IDD_MAGNITUDE) {
		int i = GetGraphIndexForIDD(IDD_TWC);
		if (i >= 0) {
			m_apGraphs[i]->ResizeGraph(TRUE, TRUE);
		}
	}

	//**************************************************
	// If we're destroying the TWC graph then make sure
	// the legend is displayed for the melogram and
	// magnitude graphs.
	//**************************************************
	else if (m_anGraphID[index] == IDD_TWC) {
		int i = GetGraphIndexForIDD(IDD_MELOGRAM);
		if (i >= 0) {
			m_apGraphs[i]->ShowLegend(TRUE, FALSE);
		}
		i = GetGraphIndexForIDD(IDD_MAGNITUDE);
		if (i >= 0) {
			m_apGraphs[i]->ShowLegend(TRUE, FALSE);
		}
	}

	m_anGraphID[index] = 0;
	MakeGraphArraysContiguous();

	//**************************************************
	// Set a new layout.
	//**************************************************
	m_nLayout = SetLayout(&m_anGraphID[0]);

	//**************************************************
	// No more graphs in view so close view.
	//**************************************************
	if (m_nLayout == -1) {
		// close view
		SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0);
	} else {
		OnGraphsRetile();
	}

	return 0;
}

/***************************************************************************/
// 09/21/2000 - DDO
/***************************************************************************/
BOOL CSaView::DestroyGraph(CGraphWnd ** pGraph, BOOL bResetFocus) {
	if (*pGraph) {
		(*pGraph)->DestroyWindow();
		if (*pGraph == m_pFocusedGraph && bResetFocus) {
			ResetFocusedGraph();
		}
		delete *pGraph;
		*pGraph = NULL;
		return TRUE;
	}

	return FALSE;
}

/***************************************************************************/
// 09/21/2000 - DDO
//
// This function will make sure all elements in the graph pointer and graph
// ID arrays are contiguous starting from the beginning of the array. There
// are no holes allowed.
/***************************************************************************/
void CSaView::MakeGraphArraysContiguous() {
	for (int i = 0; i < MAX_GRAPHS_NUMBER - 1; i++) {
		if (!m_apGraphs[i]) {
			int j;
			for (j = i + 1; j < MAX_GRAPHS_NUMBER && !m_apGraphs[j]; j++);
			if (j < MAX_GRAPHS_NUMBER && m_apGraphs[j]) {
				m_apGraphs[i] = m_apGraphs[j];
				m_anGraphID[i] = m_anGraphID[j];
				m_apGraphs[j] = NULL;
				m_anGraphID[j] = 0;
			}
		}
	}
}

/***************************************************************************/
// CSaView::OnGraphStyleChanged Graphs caption styles have changed
/***************************************************************************/
LRESULT CSaView::OnGraphStyleChanged(WPARAM, LPARAM) {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_anGraphID[nLoop] == ID_GRAPHS_OVERLAY) {
			CMultiPlotWnd * pMPlot = (CMultiPlotWnd *)(m_apGraphs[nLoop]->GetPlot());
			int baseType = pMPlot->GetBasePlotID();

			DeleteGraphs(nLoop);
			CreateGraph(nLoop, baseType);
		} else if (m_anGraphID[nLoop]) {
			// 09/26/2000 - DDO Delete graphs without clear IDs.
			DeleteGraphs(nLoop, FALSE);
			// delete and recreate all graphs with new caption style
			CreateGraph(nLoop, m_anGraphID[nLoop]);
		}
	}
	// retile graphs
	OnGraphsRetile();
	return 0;
}

/***************************************************************************/
// CSaView::OnGraphGridChanged Graphs grids have changed
/***************************************************************************/
LRESULT CSaView::OnGraphGridChanged(WPARAM, LPARAM) {
	RedrawGraphs();
	return 0;
}

// 1.5Test8.2
/***************************************************************************/
// CSaView::OnGraphOrderChanged Graphs colors have changed
/***************************************************************************/
LRESULT CSaView::OnGraphOrderChanged(WPARAM, LPARAM) {
	ResizeGraphs(TRUE, TRUE);
	return 0;
}

/***************************************************************************/
// CSaView::OnGraphColorChanged Graphs colors have changed
/***************************************************************************/
LRESULT CSaView::OnGraphColorChanged(WPARAM, LPARAM) {
	RedrawGraphs(TRUE, TRUE);
	return 0;
}

/***************************************************************************/
// CSaView::OnGraphFontChanged Graphs font styles have changed
/***************************************************************************/
LRESULT CSaView::OnGraphFontChanged(WPARAM, LPARAM) {
	ResizeGraphs(TRUE, TRUE);
	return 0;
}

/***************************************************************************/
// CSaView::OnScrollZoomChanged Scroll zooming option has changed
/***************************************************************************/
LRESULT CSaView::OnScrollZoomChanged(WPARAM wParam, LPARAM) {
	if ((wParam == 0) || (GetDocument()->GetDataSize() == 0)) {
		SetScrollRange(SB_VERT, 0, 0, FALSE);    // hide scroll bar
	} else {
		SetScrollRange(SB_VERT, ZOOM_SCROLL_RESOLUTION, (int)m_fVScrollSteps, FALSE);
		SetScrollPos(SB_VERT, (int)(m_fVScrollSteps + ZOOM_SCROLL_RESOLUTION - m_fVScrollSteps / m_fZoom), TRUE);
	}
	return 0;
}

/***************************************************************************/
// CSaView::OnRecorder Launches the recorder
/***************************************************************************/
LRESULT CSaView::OnRecorder(WPARAM /*wParam*/, LPARAM /*lParam*/) {
	CDlgRecorder dlg;
	if (dlg.DoModal() != IDOK) {
		SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L);    // close the file
	}
	return 0;
}

/***************************************************************************/
// CSaView::OnGraphsStyleLine Set the graphs drawing style to line
/***************************************************************************/
void CSaView::OnGraphsStyleLine() {
	m_bDrawStyleLine = TRUE; // draw style is line for all graphs
	// set graph styles
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			m_apGraphs[nLoop]->SetLineDraw(m_bDrawStyleLine);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateGraphsStyleLine Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphsStyleLine(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(m_bDrawStyleLine);
}

/***************************************************************************/
// CSaView::OnGraphsStyleSolid Set the graphs drawing style to solid
/***************************************************************************/
void CSaView::OnGraphsStyleSolid() {
	m_bDrawStyleLine = FALSE; // draw style is solid for all graphs
	// set graph styles
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			m_apGraphs[nLoop]->SetLineDraw(m_bDrawStyleLine);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateGraphsStyleSolid Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphsStyleSolid(CCmdUI * pCmdUI) {
	pCmdUI->SetCheck(!m_bDrawStyleLine);
}

/***************************************************************************/
// CSaView::OnPopupgraphStyleLine Set the graph drawing style to line
/***************************************************************************/
void CSaView::OnPopupgraphStyleLine() {
	if (!m_pFocusedGraph) {
		return;
	}

	m_pFocusedGraph->SetLineDraw(TRUE);
	(m_pFocusedGraph->GetPlot())->SetDotsDraw(FALSE);
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphStyleLine Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphStyleLine(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_pFocusedGraph // enable if focused graph
		&& (GetDocument()->GetDataSize() != 0)	// enable if data is available
		&& (m_nFocusedID != IDD_SPECTROGRAM)		// enable if graph is not this type
		&& (m_nFocusedID != IDD_SNAPSHOT)		// enable if graph is not this type
		&& (m_nFocusedID != IDD_3D)				// enable if graph is not this type
		&& (m_nFocusedID != IDD_F1F2)			// enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1)			// enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1F1)			// enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_A)			// enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_B)			// enable if graph is not this type
		&& (m_nFocusedID != IDD_INVSDP));		// enable if graph is not this type

	if (m_pFocusedGraph)
		pCmdUI->SetCheck((m_pFocusedGraph->HaveDrawingStyleLine() && (m_nFocusedID != IDD_RAWDATA))
			|| (m_pFocusedGraph->HaveDrawingStyleLine() &&
				(!(m_pFocusedGraph->GetPlot())->HaveDrawingStyleDots())));
	else {
		pCmdUI->SetCheck(FALSE);
	}

}

/***************************************************************************/
// CSaView::OnPopupgraphStyleSolid Set the graph drawing style to solid
/***************************************************************************/
void CSaView::OnPopupgraphStyleSolid() {
	if (!m_pFocusedGraph) {
		return;
	}

	m_pFocusedGraph->SetLineDraw(FALSE);
	(m_pFocusedGraph->GetPlot())->SetDotsDraw(FALSE);
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphStyleSolid Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphStyleSolid(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_pFocusedGraph // enable if focused graph
		&& (GetDocument()->GetDataSize() != 0) // enable if data is available
		&& (m_nFocusedID != IDD_SPECTROGRAM) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SNAPSHOT) // enable if graph is not this type
		&& (m_nFocusedID != IDD_3D) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F1F2) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1F1) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_A) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_B) // enable if graph is not this type
		&& (m_nFocusedID != IDD_INVSDP)); // enable if graph is not this type
	if (m_pFocusedGraph)
		pCmdUI->SetCheck((!m_pFocusedGraph->HaveDrawingStyleLine() && (m_nFocusedID != IDD_RAWDATA))
			|| (!m_pFocusedGraph->HaveDrawingStyleLine() && (!(m_pFocusedGraph->GetPlot())->HaveDrawingStyleDots())));
	else {
		pCmdUI->SetCheck(FALSE);
	}
}

/***************************************************************************/
// CSaView::OnPopupgraphStyleDots Set the raw data graph drawing style to dots
/***************************************************************************/
void CSaView::OnPopupgraphStyleDots() {
	if (!m_pFocusedGraph) {
		return;
	}

	(m_pFocusedGraph->GetPlot())->SetDotsDraw(TRUE);
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphStyleDots Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphStyleDots(CCmdUI * pCmdUI) {
	pCmdUI->Enable((GetDocument()->GetDataSize() != 0) // enable if data is available
		&& ((m_nFocusedID == IDD_RAWDATA) || (m_nFocusedID == IDD_GLOTWAVE) || (m_nFocusedID == IDD_MELOGRAM))); // enable if graph is this type
	pCmdUI->SetCheck(((m_nFocusedID == IDD_RAWDATA) || (m_nFocusedID == IDD_GLOTWAVE) || (m_nFocusedID == IDD_MELOGRAM))
		&& ((m_pFocusedGraph->GetPlot())->HaveDrawingStyleDots())); // check if graph has style dots
}

/***************************************************************************/
// CSaView::OnPopupRawdata switch focus to next graph
/***************************************************************************/
void CSaView::OnNextGraph() {
	int nSelection = -1;

	if (m_pFocusedGraph) {
		nSelection = GraphPtrToOffset(m_pFocusedGraph) + 1;
		if (nSelection >= MAX_GRAPHS_NUMBER || !m_apGraphs[nSelection]) {
			nSelection = 0;
		}
	} else {
		nSelection = 0;
	}

	m_apGraphs[nSelection]->SendMessage(WM_LBUTTONDOWN, 0, MAKELONG(0, 0)); // change focus
}

/***************************************************************************/
// CSaView::OnPopupRawdata switch focus to previous graph
/***************************************************************************/
void CSaView::OnPreviousGraph() {
	int nSelection = -1;
	int nNumberOfGraphs = GetNumberOfGraphs(&m_anGraphID[0]);

	if (m_pFocusedGraph) {
		nSelection = GraphPtrToOffset(m_pFocusedGraph) - 1;
		if (nSelection < 0 || !m_apGraphs[nSelection]) {
			nSelection = nNumberOfGraphs-1;
		}
	}
	else {
		nSelection = nNumberOfGraphs-1;
	}

	m_apGraphs[nSelection]->SendMessage(WM_LBUTTONDOWN, 0, MAKELONG(0, 0)); // change focus
}

/***************************************************************************/
// CSaView::OnActivateView View activating or deactivating
// If the view loses or gains active state, its focused graph has to
// gain or lose focus. The statusbar has to be updated. Also the mainframe
// has to be informed, because of the player or recorder launched, which have
// to be stopped immediately.
/***************************************************************************/
void CSaView::OnActivateView(BOOL bActivate, CView * pActivateView, CView * pDeactivateView) {
	CSaDoc * pModel = (CSaDoc *)GetDocument(); // get pointer to document
	CView::OnActivateView(bActivate, pActivateView, pDeactivateView);

	m_bViewIsActive = bActivate;
	if (m_pFocusedGraph) {
		if (m_bViewIsActive) {
			m_pFocusedGraph->SetGraphFocus(TRUE);
		}
		m_pFocusedGraph->RedrawCaption(); // graph has to lose or gain focus too
	}
	if (bActivate) { // activating
		// inform mainframe
		GetMainFrame().SendMessage(WM_USER_CHANGEVIEW, TRUE, (LONG)this);
		// process workbench if necessary
		if (pModel->WorkbenchProcess()) {
			RedrawGraphs(TRUE, TRUE);
		};
		// redraw statusbar if data is present
		if (pModel->GetDataSize() != 0) {
			if (m_pFocusedGraph) {
				m_pFocusedGraph->UpdateStatusBar(GetStartCursorPosition(), GetStopCursorPosition(), TRUE);
			}
		} else { // clear status bar panes
			// get pointer to status bar
			CDataStatusBar * pStat = GetMainFrame().GetDataStatusBar();
			// turn off symbols
			pStat->SetPaneSymbol(ID_STATUSPANE_SAMPLES, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_FORMAT, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_CHANNELS, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_SIZE, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_TLENGTH, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_TYPE, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_BITRATE, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_EMPTY, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_1, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_2, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_3, FALSE);
			pStat->SetPaneSymbol(ID_STATUSPANE_4, FALSE);
			// clear the panes
			pStat->SetPaneText(ID_STATUSPANE_SAMPLES, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_FORMAT, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_CHANNELS, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_SIZE, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_TLENGTH, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_TYPE, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_BITRATE, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_EMPTY, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_1, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_2, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_3, _T(""));
			pStat->SetPaneText(ID_STATUSPANE_4, _T(""));
		}
	}
}

/***************************************************************************/
// CSaView::OnDestroy Destroying the view
// The mainframe has to be informed, if the last view has been destroyed,
// because the player or recorder then have to be stopped and closed.
/***************************************************************************/
void CSaView::OnDestroy() {
	CView::OnDestroy();

	if (GetMainFrame().ComputeNumberOfViews(-1) == 0) { // last view destroyed?
		GetMainFrame().SendMessage(WM_USER_CHANGEVIEW, FALSE, (LONG)this);    // inform mainframe
	}
}

/***************************************************************************/
// 09/26/2000 - DDO This function will return TRUE or FALSE for enabling
//                  and disabling graph types. The conditions are standard
//                  conditions for the types.
/***************************************************************************/
BOOL CSaView::GraphTypeEnabled(int nID, BOOL bIncludeCtrlKeyCheck) {
	BOOL bTest = ((GetDocument()->GetDataSize() != 0) &&
		(m_nFocusedID != IDD_MELOGRAM &&
			m_nFocusedID != IDD_TWC && nID != IDD_TWC &&
			m_nFocusedID != IDD_MAGNITUDE && nID != IDD_MAGNITUDE &&
			m_nFocusedID != IDD_RECORDING) || nID == IDD_RECORDING);

	bTest &= (nID != IDD_RATIO);

	if (bIncludeCtrlKeyCheck) {
		bTest &= (GetKeyState(VK_CONTROL) < 0);
	}

	return bTest;
}

struct SGraphTypeInfo {
	int nID;
	BOOL bIncludeCtrlKeyCheck;
};

static const SGraphTypeInfo * GetGraphTypeInfo(int nID) {
	static const SGraphTypeInfo kGraphTypeInfo[] = {
		// nID              bCtrl
		{  IDD_3D,          FALSE },
		{  IDD_CEPPITCH,    FALSE },
		{  IDD_CHANGE,      FALSE },
		{  IDD_CHPITCH,     FALSE },
		{  IDD_DURATION,    FALSE },
		{  IDD_F1F2,        FALSE },
		{  IDD_F2F1,        FALSE },
		{  IDD_F2F1F1,      FALSE },
		{  IDD_GRAPITCH,    FALSE },
		{  IDD_LOUDNESS,    FALSE },
		{  IDD_MAGNITUDE,   FALSE },
		{  IDD_MELOGRAM,    FALSE },
		{  IDD_PITCH,       FALSE },
		{  IDD_RATIO,       FALSE },
		{  IDD_RAWDATA,     FALSE },
		{  IDD_RECORDING,   FALSE },
		{  IDD_SPECTRUM,    FALSE },
		{  IDD_SPECTROGRAM, FALSE },
		{  IDD_SMPITCH,     FALSE },
		{  IDD_SNAPSHOT,    FALSE },
		{  IDD_STAFF,       FALSE },
		{  IDD_TWC,         FALSE },
		{  IDD_ZCROSS,      FALSE },
		{  IDD_GLOTWAVE,    TRUE },
		{  IDD_POA,         TRUE },
		{  IDD_SDP_A,       TRUE },
		{  IDD_SDP_B,       TRUE },
		{  IDD_INVSDP,      TRUE },
		{  IDD_3D_PITCH,    TRUE },
		{  IDD_WAVELET,     TRUE },
		{  NULL,            FALSE },
	};

	for (int i = 0; kGraphTypeInfo[i].nID; i++) {
		if (kGraphTypeInfo[i].nID == nID) {
			return &kGraphTypeInfo[i];
		}
	}
	ASSERT(FALSE);
	return NULL;
}

/***************************************************************************/
// CSaView::OnChangeGraph Change the graph type
/***************************************************************************/
void CSaView::OnChangeGraph(UINT nID) {
	if (GetGraphTypeInfo(nID)) {
		ChangeGraph(nID);
	}
}

/***************************************************************************/
// CSaView::OnUpdateChangeGraph Menu update
/***************************************************************************/
void CSaView::OnUpdateChangeGraph(CCmdUI * pCmdUI) {
	const SGraphTypeInfo * pInfo = GetGraphTypeInfo(pCmdUI->m_nID);
	ASSERT(pInfo);

	BOOL bSelected = GraphIDincluded(pCmdUI->m_nID);
	BOOL bEnable = !bSelected || pCmdUI->m_nID == IDD_RECORDING;

	// enable if data is available
	pCmdUI->Enable(bEnable && GraphTypeEnabled(pCmdUI->m_nID, pInfo->bIncludeCtrlKeyCheck));
	// check if graph selected
	pCmdUI->SetCheck(bSelected);
}

/***************************************************************************/
// 09/27/2000 - DDO
/***************************************************************************/
void CSaView::ToggleDpGraph(UINT nID) {
	int i = GetGraphIndexForIDD(nID);
	if (i >= 0) {
		// delete this graph
		DeleteGraphs(i);
		// clean up the old arrays
		MakeGraphArraysContiguous();
		if (m_apGraphs[0] != NULL) {
			// change focus SDM 1.5Test10.6
			m_apGraphs[0]->SendMessage(WM_LBUTTONDOWN, 0, MAKELONG(0, 0));
		}
	} else {
		// create new graph
		for (i = 0; i < MAX_GRAPHS_NUMBER; i++) {
			if (m_anGraphID[i] < nID) {
				if (m_apGraphs[i] != NULL) {
					DeleteGraphs(i);
				}
				m_apGraphs[i] = NULL;
				m_anGraphID[i] = 0;
				MakeGraphArraysContiguous();
				CreateGraph(i, nID);
				if (m_apGraphs[i] != NULL) {
					// change focus SDM 1.5Test10.6
					m_apGraphs[i]->SendMessage(WM_LBUTTONDOWN, 0, MAKELONG(0, 0));
				}
				break;
			}
		}
	}

	// set corresponding layout to new chosen graphs
	m_nLayout = SetLayout(&m_anGraphID[0]);
	OnGraphsRetile(); // retile graphs
}

/***************************************************************************/
// 09/27/2000 - DDO
/***************************************************************************/
void CSaView::UpdateDpGraphsMenu(CCmdUI * pCmdUI, int nID) {
	int index = GetGraphIndexForIDD(nID);
	pCmdUI->SetCheck(index >= 0);
	pCmdUI->Enable(GetNumberOfGraphs(&m_anGraphID[0]) > 1 || index == -1);
}

// SDM 1.5Test10.4
/***************************************************************************/
// CSaView::OnDpGrapitch Toggle AutoPitch graph
/***************************************************************************/
void CSaView::OnDpGrapitch() {
	ToggleDpGraph(IDD_GRAPITCH);
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnUpdateDpGrapitch(CCmdUI * pCmdUI) {
	UpdateDpGraphsMenu(pCmdUI, IDD_GRAPITCH);
}

// SDM 1.5Test10.4
/***************************************************************************/
// CSaView::OnDpGrapitch Toggle waveform graph
/***************************************************************************/
void CSaView::OnDpRawdata() {
	ToggleDpGraph(IDD_RAWDATA);
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnUpdateDpRawdata(CCmdUI * pCmdUI) {
	UpdateDpGraphsMenu(pCmdUI, IDD_RAWDATA);
}

// SDM 1.5Test10.4
/***************************************************************************/
// CSaView::OnDpGrapitch Toggle Spectrogram graph
/***************************************************************************/
void CSaView::OnDpSpectrogram() {
	ToggleDpGraph(IDD_SPECTROGRAM);
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnUpdateDpSpectrogram(CCmdUI * pCmdUI) {
	UpdateDpGraphsMenu(pCmdUI, IDD_SPECTROGRAM);
}

/***************************************************************************/
// CSaView::OnPopupgraphGridlines Change gridlines
/***************************************************************************/
void CSaView::OnPopupgraphGridlines() {
	if (m_pFocusedGraph) {
		m_pFocusedGraph->ShowGrid(!m_pFocusedGraph->HaveGrid(), TRUE);
	}
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphGridlines Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphGridlines(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_pFocusedGraph && GetDocument()->GetDataSize() != 0); // enable if data is available
	if (m_pFocusedGraph) {
		pCmdUI->SetCheck(m_pFocusedGraph->HaveGrid());    // check if graph has gridlines
	} else {
		pCmdUI->SetCheck(FALSE);
	}
}

/***************************************************************************/
// CSaView::OnPopupgraphLegend Show or hide legend window
/***************************************************************************/
void CSaView::OnPopupgraphLegend() {
	if (m_pFocusedGraph) {
		m_pFocusedGraph->ShowLegend(!m_pFocusedGraph->HasLegend(), TRUE);
	}

	//*****************************************************
	// 09/29/2000 - DDO When turning off the legend for
	// the TWC graph and the melogram graph isn't showing
	// it's legend, then automatically show the legend for
	// the melogram.
	//*****************************************************
	if (m_nFocusedID == IDD_TWC) {
		int i = GetGraphIndexForIDD(IDD_MELOGRAM);
		if ((m_apGraphs[i] != NULL) &&
			(!m_apGraphs[i]->HasLegend())) {
			m_apGraphs[i]->ShowLegend(TRUE, TRUE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphLegend Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphLegend(CCmdUI * pCmdUI) {
	BOOL bEnable = m_pFocusedGraph && GetDocument()->GetDataSize() != 0;
	if (bEnable && m_nFocusedID != IDD_MELOGRAM) {
		bEnable &= (!m_pFocusedGraph->DisableLegend() || m_pFocusedGraph->HasLegend());
	}
	pCmdUI->Enable(bEnable);
	if (m_pFocusedGraph) {
		pCmdUI->SetCheck(m_pFocusedGraph->HasLegend());    // check if graph has legend window
	} else {
		pCmdUI->SetCheck(FALSE);
	}
}

/***************************************************************************/
// CSaView::OnPopupgraphXScale Show or hide x-scale window
/***************************************************************************/
void CSaView::OnPopupgraphXScale() {
	if (m_pFocusedGraph) {
		m_pFocusedGraph->ShowXScale(!m_pFocusedGraph->HaveXScale(), TRUE);
	}
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphXScale Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphXScale(CCmdUI * pCmdUI) {
	BOOL bEnable = ((m_pFocusedGraph && GetDocument()->GetDataSize() != 0) &&
		((m_pFocusedGraph->HaveXScale()) || (!m_pFocusedGraph->DisableXScale())) &&
		(m_nFocusedID != IDD_TWC));
	pCmdUI->Enable(bEnable);
	if (m_pFocusedGraph) {
		pCmdUI->SetCheck(m_pFocusedGraph->HaveXScale());    // check if graph has x-scale window
	} else {
		pCmdUI->SetCheck(FALSE);
	}
}

/***************************************************************************/
// 09/27/2000 - DDO This function will show an annotation in the focused
//                  graph. If the focused graph is the melogram then the
//                  annotation is also shown in the TWC graph. I would have
//                  added this code in the graph's ShowAnnotation function
//                  but I was having trouble getting a pointer to the
//                  current view when the view is first being started up.
//                  I don't understand what the deal is.
/***************************************************************************/
void CSaView::ShowAnnotation(EAnnotation nAnnot) {

	if (m_pFocusedGraph) {
		m_pFocusedGraph->ShowAnnotation(nAnnot, (!m_pFocusedGraph->HaveAnnotation(nAnnot)), TRUE);
	}
	if (m_nFocusedID == IDD_MELOGRAM) {
		int i = GetGraphIndexForIDD(IDD_TWC);
		if ((i != -1) && (m_apGraphs[i] != NULL)) {
			m_apGraphs[i]->ShowAnnotation(nAnnot, (!m_apGraphs[i]->HaveAnnotation(nAnnot)), TRUE);
		}
	}
}


/***************************************************************************/
// CSaView::OnGraphsMagnify1 Set magnify factor
/***************************************************************************/
void CSaView::OnGraphsMagnify1() {
	m_fMagnify = 1.0;
	// set graphs magnify
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->SetMagnify(m_fMagnify, TRUE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateGraphsMagnify1 Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphsMagnify1(CCmdUI * pCmdUI) {
	pCmdUI->Enable(GetDocument()->GetDataSize() != 0); // enable if data is available
	pCmdUI->SetCheck(m_fMagnify == 1.0); // check if factor matches
}

/***************************************************************************/
// CSaView::OnGraphsMagnify2 Set magnify factor
/***************************************************************************/
void CSaView::OnGraphsMagnify2() {
	m_fMagnify = 2.0;
	// set graphs magnify
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->SetMagnify(m_fMagnify, TRUE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateGraphsMagnify2 Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphsMagnify2(CCmdUI * pCmdUI) {
	pCmdUI->Enable(GetDocument()->GetDataSize() != 0); // enable if data is available
	pCmdUI->SetCheck(m_fMagnify == 2.0); // check if factor matches
}

/***************************************************************************/
// CSaView::OnGraphsMagnify4 Set magnify factor
/***************************************************************************/
void CSaView::OnGraphsMagnify4() {
	m_fMagnify = 4.0;
	// set graphs magnify
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->SetMagnify(m_fMagnify, TRUE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateGraphsMagnify4 Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphsMagnify4(CCmdUI * pCmdUI) {
	pCmdUI->Enable(GetDocument()->GetDataSize() != 0); // enable if data is available
	pCmdUI->SetCheck(m_fMagnify == 4.0); // check if factor matches
}

/***************************************************************************/
// CSaView::OnGraphsMagnifycustom Set magnify factor
/***************************************************************************/
void CSaView::OnGraphsMagnifycustom() {
	// input dialog
	CDlgMagnify * pDlgMagnify = new CDlgMagnify;
	pDlgMagnify->m_fMagnify = m_fMagnify;
	if (pDlgMagnify->DoModal() == IDOK) {
		m_fMagnify = pDlgMagnify->m_fMagnify;
		// set graphs magnify
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
			if (m_apGraphs[nLoop] != NULL) {
				m_apGraphs[nLoop]->SetMagnify(m_fMagnify, TRUE);
			}
		}
	}
	delete pDlgMagnify;
}

/***************************************************************************/
// CSaView::OnUpdateGraphsMagnifycustom Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphsMagnifycustom(CCmdUI * pCmdUI) {
	pCmdUI->Enable(GetDocument()->GetDataSize() != 0); // enable if data is available
	pCmdUI->SetCheck((m_fMagnify != 1.0) // check if factor does not match
		&& (m_fMagnify != 2.0)
		&& (m_fMagnify != 4.0));
}

/***************************************************************************/
// CSaView::OnPopupgraphMagnify1 Set magnify factor for graph
/***************************************************************************/
void CSaView::OnPopupgraphMagnify1() {
	if (m_pFocusedGraph) {
		m_pFocusedGraph->SetMagnify(1.0, TRUE);
	}
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphMagnify1 Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphMagnify1(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_pFocusedGraph // enable if focused graph
		&& (GetDocument()->GetDataSize() != 0) // enable if data is available
		&& (m_nFocusedID != IDD_SPECTROGRAM) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SNAPSHOT) // enable if graph is not this type
		&& (m_nFocusedID != IDD_3D) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F1F2) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1F1) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_A) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_B) // enable if graph is not this type
		&& (m_nFocusedID != IDD_INVSDP) // enable if graph is not this type
		&& (m_pFocusedGraph->HasCursors())); // enable if cursors visible
	if (m_pFocusedGraph)
		pCmdUI->SetCheck((m_pFocusedGraph->GetMagnify() == 1.0) // check if factor matches
			|| (m_nFocusedID == IDD_SPECTROGRAM) // enable if graph is not this type
			|| (m_nFocusedID == IDD_SNAPSHOT) // enable if graph is not this type
			|| (m_nFocusedID == IDD_3D) // check if graph is this type
			|| (m_nFocusedID == IDD_F1F2) // check if graph is this type
			|| (m_nFocusedID == IDD_F2F1) // check if graph is this type
			|| (m_nFocusedID == IDD_F2F1F1) // check if graph is this type
			|| (m_nFocusedID == IDD_SDP_A) // check if graph is this type
			|| (m_nFocusedID == IDD_SDP_B) // check if graph is this type
			|| (m_nFocusedID == IDD_INVSDP)); // check if graph is this type
	else {
		pCmdUI->SetCheck(FALSE);
	}
}

/***************************************************************************/
// CSaView::OnPopupgraphMagnify2 Set magnify factor for focused graph
/***************************************************************************/
void CSaView::OnPopupgraphMagnify2() {
	if (m_pFocusedGraph) {
		m_pFocusedGraph->SetMagnify(2.0, TRUE);
	}
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphMagnify2 Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphMagnify2(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_pFocusedGraph // enable if focused graph
		&& (GetDocument()->GetDataSize() != 0) // enable if data is available
		&& (m_nFocusedID != IDD_SPECTROGRAM) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SNAPSHOT) // enable if graph is not this type
		&& (m_nFocusedID != IDD_3D) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F1F2) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1F1) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_A) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_B) // enable if graph is not this type
		&& (m_nFocusedID != IDD_INVSDP) // enable if graph is not this type
		&& (m_pFocusedGraph->HasCursors())); // enable if cursors visible
	if (m_pFocusedGraph)
		pCmdUI->SetCheck((m_pFocusedGraph->GetMagnify() == 2.0) // check if factor matches
			&& (m_nFocusedID != IDD_SPECTROGRAM) // check only if graph is not this type
			&& (m_nFocusedID != IDD_SNAPSHOT) // check only if graph is not this type
			&& (m_nFocusedID != IDD_3D) // check only if graph is not this type
			&& (m_nFocusedID != IDD_F1F2) // check only if graph is not this type
			&& (m_nFocusedID != IDD_F2F1) // check only if graph is not this type
			&& (m_nFocusedID != IDD_F2F1F1) // check only if graph is not this type
			&& (m_nFocusedID != IDD_SDP_A) // check only if graph is not this type
			&& (m_nFocusedID != IDD_SDP_B) // check only if graph is not this type
			&& (m_nFocusedID != IDD_INVSDP)); // check only if graph is not this type
	else {
		pCmdUI->SetCheck(FALSE);
	}
}

/***************************************************************************/
// CSaView::OnPopupgraphMagnify4 Set magnify factor for focused graph
/***************************************************************************/
void CSaView::OnPopupgraphMagnify4() {
	if (m_pFocusedGraph) {
		m_pFocusedGraph->SetMagnify(4.0, TRUE);
	}
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphMagnify4 Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphMagnify4(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_pFocusedGraph // enable if focused graph
		&& (GetDocument()->GetDataSize() != 0) // enable if data is available
		&& (m_nFocusedID != IDD_SPECTROGRAM) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SNAPSHOT) // enable if graph is not this type
		&& (m_nFocusedID != IDD_3D) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F1F2) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1F1) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_A) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_B) // enable if graph is not this type
		&& (m_nFocusedID != IDD_INVSDP) // enable if graph is not this type
		&& (m_pFocusedGraph->HasCursors())); // enable if cursors visible
	if (m_pFocusedGraph)
		pCmdUI->SetCheck((m_pFocusedGraph->GetMagnify() == 4.0) // check if factor matches
			&& (m_nFocusedID != IDD_SPECTROGRAM) // check only if graph is not this type
			&& (m_nFocusedID != IDD_SNAPSHOT) // check only if graph is not this type
			&& (m_nFocusedID != IDD_3D) // check only if graph is not this type
			&& (m_nFocusedID != IDD_F1F2) // check only if graph is not this type
			&& (m_nFocusedID != IDD_F2F1) // check only if graph is not this type
			&& (m_nFocusedID != IDD_F2F1F1) // check only if graph is not this type
			&& (m_nFocusedID != IDD_SDP_A) // check only if graph is not this type
			&& (m_nFocusedID != IDD_SDP_B) // check only if graph is not this type
			&& (m_nFocusedID != IDD_INVSDP)); // check only if graph is not this type
	else {
		pCmdUI->SetCheck(FALSE);
	}
}

/***************************************************************************/
// CSaView::OnPopupgraphMagnifycustom Set magnify factor for focused graph
/***************************************************************************/
void CSaView::OnPopupgraphMagnifycustom() {
	if (!m_pFocusedGraph) {
		return;
	}

	// input dialog
	CDlgMagnify * pDlgMagnify = new CDlgMagnify;
	pDlgMagnify->m_fMagnify = m_pFocusedGraph->GetMagnify();
	if (pDlgMagnify->DoModal() == IDOK) {
		m_pFocusedGraph->SetMagnify(pDlgMagnify->m_fMagnify, TRUE);
	}
	delete pDlgMagnify;
}

/***************************************************************************/
// CSaView::OnUpdatePopupgraphMagnifycustom Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphMagnifycustom(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_pFocusedGraph // enable if focused graph
		&& (GetDocument()->GetDataSize() != 0) // enable if data is available
		&& (m_nFocusedID != IDD_SPECTROGRAM) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SNAPSHOT) // enable if graph is not this type
		&& (m_nFocusedID != IDD_3D) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F1F2) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1) // enable if graph is not this type
		&& (m_nFocusedID != IDD_F2F1F1) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_A) // enable if graph is not this type
		&& (m_nFocusedID != IDD_SDP_B) // enable if graph is not this type
		&& (m_nFocusedID != IDD_INVSDP) // enable if graph is not this type
		&& (m_pFocusedGraph->HasCursors())); // enable if cursors visible
	if (m_pFocusedGraph)
		pCmdUI->SetCheck((m_pFocusedGraph->GetMagnify() != 1.0) // check if factor does not match
			&& (m_pFocusedGraph->GetMagnify() != 2.0)
			&& (m_pFocusedGraph->GetMagnify() != 4.0)
			&& (m_nFocusedID != IDD_SPECTROGRAM) // check only if graph is not this type
			&& (m_nFocusedID != IDD_SNAPSHOT) // check only if graph is not this type
			&& (m_nFocusedID != IDD_3D) // check only if graph is not this type
			&& (m_nFocusedID != IDD_F1F2) // check only if graph is not this type
			&& (m_nFocusedID != IDD_F2F1) // check only if graph is not this type
			&& (m_nFocusedID != IDD_F2F1F1) // check only if graph is not this type
			&& (m_nFocusedID != IDD_SDP_A) // check only if graph is not this type
			&& (m_nFocusedID != IDD_SDP_B) // check only if graph is not this type
			&& (m_nFocusedID != IDD_INVSDP)); // check only if graph is not this type
	else {
		pCmdUI->SetCheck(FALSE);
	}
}

/***************************************************************************/
// CSaView::OnRestartProcess Restart the canceled process
/***************************************************************************/
void CSaView::OnRestartProcess() {
	// invoked via Recalc button on area graph or Enter key on non-area graph
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			if (!m_apGraphs[nLoop]->IsAreaGraph() && m_apGraphs[nLoop]->IsCanceled()) {
				m_apGraphs[nLoop]->RestartProcess();    // restart all canceled non-area graphs
			}
			// process results may be needed by area graph,
			// so restart even if the function is invoked
			// via Recalc
			if (m_apGraphs[nLoop]->IsPlotID(IDD_SNAPSHOT)) {
				// restart area graph process only if graph has focus
				m_apGraphs[nLoop]->RestartProcess();
			}
		}
	}
	if (m_pFocusedGraph && m_pFocusedGraph->IsAreaGraph() && !m_pFocusedGraph->IsPlotID(IDD_SNAPSHOT)) {
		// restart area graph process only if graph has focus
		m_pFocusedGraph->RestartProcess();
	}
	// Clear canceled flag on all processes still canceled
	GetDocument()->RestartAllProcesses();
}

/***************************************************************************/
// CSaView::OnUpdateRestartProcess Menu update
/***************************************************************************/
void CSaView::OnUpdateRestartProcess(CCmdUI * pCmdUI) {
	BOOL bEnable = GetDocument()->AnyProcessCanceled();
	if ((m_pFocusedGraph) && (m_pFocusedGraph->IsAreaGraph()) && (!m_pFocusedGraph->IsPlotID(IDD_RECORDING))) {
		// restart area graph process only if graph has focus
		bEnable = TRUE;
	}
	if (GraphIDtoPtr(IDD_SNAPSHOT)) {
		bEnable = TRUE;
	}

	pCmdUI->Enable(bEnable);
}

/***************************************************************************/
// CSaView::OnEditSelectWaveformSeg Select Waveform data between cursors
/***************************************************************************/
void CSaView::OnEditSelectWaveformSeg() {
	// select/deselect raw data area
	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i >= 0) && (m_apGraphs[i] != NULL)) {
		// check if already area selected
		if (m_apGraphs[i]->GetPlot()->GetHighLightLength()) {
			// deselect
			m_apGraphs[i]->GetPlot()->ClearHighLightArea();
		} else {
			// select
			m_apGraphs[i]->GetPlot()->SetHighLightArea(GetStartCursorPosition(), GetStopCursorPosition(), TRUE, FALSE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditSelectWaveformSeg Select Waveform data between cursors
/***************************************************************************/
void CSaView::OnUpdateEditSelectWaveformSeg(CCmdUI * pCmdUI) {
	pCmdUI->Enable(true);
}

/***************************************************************************/
// CSaView::OnEditSelectWaveform Select Waveform data between cursors
/***************************************************************************/
void CSaView::OnEditSelectWaveform() {
}

/***************************************************************************/
// CSaView::OnUpdateEditSelectWaveform Select Waveform data between cursors
/***************************************************************************/
void CSaView::OnUpdateEditSelectWaveform(CCmdUI * pCmdUI) {
	pCmdUI->Enable(true);
}

/***************************************************************************/
// CSaView::OnPlayFKey Playback according to function key setting
/***************************************************************************/
void CSaView::OnPlayFKey(UINT nID) {
	// send message to start player
	SendPlayMessage(WORD(nID - ID_PLAY_F1), WORD(-1));
}

/***************************************************************************/
// CSaView::OnUpdatePlayback Menu update
/***************************************************************************/
void CSaView::OnUpdatePlayback(CCmdUI * pCmdUI) {
	// enable if data is available
	pCmdUI->Enable(GetDocument()->GetDataSize() != 0);
}

/***************************************************************************/
// CSaView::OnUpdatePlayback Menu update
/***************************************************************************/
void CSaView::OnUpdatePlaybackPortion(CCmdUI * pCmdUI) {

	if (GetDocument()->GetDataSize() == 0) {
		pCmdUI->Enable(FALSE);
		return;
	}

	CDlgPlayer * pPlayer = GetMainFrame().GetPlayer(false);
	if (pPlayer == NULL) {
		pCmdUI->Enable(TRUE);
		return;
	}

	if (pPlayer->IsPaused()) {
		if (pCmdUI->m_nID != pPlayer->GetSubmode()) {
			pCmdUI->Enable(FALSE);
			return;
		}
	}

	if (pPlayer->IsPlaying()) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(TRUE);
}

/***************************************************************************/
// CSaView::OnSetupFnkeys Calls the player and the setup Fn-keys dialog
/***************************************************************************/
void CSaView::OnSetupFnkeys() {
	GetMainFrame().SendMessage(WM_USER_PLAYER, CDlgPlayer::STOPPED, MAKELONG(-1, FALSE));
	GetMainFrame().SetupFunctionKeys();
}

/***************************************************************************/
// CSaView::OnUpdateSetupFnkeys Menu update
/***************************************************************************/
void CSaView::OnUpdateSetupFnkeys(CCmdUI * pCmdUI) {
	pCmdUI->Enable(GetDocument()->GetDataSize() != 0); // enable if data is available
}

/***************************************************************************/
// CSaView::GetAnnotation - returns a pointer tothe annotation number annotSetID.
/***************************************************************************/
CSegment * CSaView::GetAnnotation(int annotSetID) {
	if ((annotSetID >= 0) && (annotSetID < ANNOT_WND_NUMBER)) {
		return GetDocument()->GetSegment(annotSetID);
	}
	return NULL;
}

/***************************************************************************/
// CSaView::GetAnnotation - returns a pointer tothe annotation number annotSetID.
/***************************************************************************/
CSegment * CSaView::GetAnnotation(EAnnotation annot) {
	return GetDocument()->GetSegment(annot);
}

/***************************************************************************/
// CSaView::OnFilePrint
/***************************************************************************/
void CSaView::OnFilePrint() {
	// Repaint window in case a screen shot print is requested
	GetMainFrame().UpdateWindow();
	GetMainFrame().SetPrintingFlag();
	CView::OnFilePrint();
}

/***************************************************************************/
// CSaView::OnFilePrintPreview
/***************************************************************************/
void CSaView::OnFilePrintPreview() {
	GetMainFrame().UpdateWindow();
	GetMainFrame().SetPrintingFlag();
	m_bPrintPreviewInProgress = TRUE;
	GetMainFrame().SetPreviewFlag();
	CView::OnFilePrintPreview();
}


/***************************************************************************/
/***************************************************************************/
BOOL CSaView::IsFocusGraph(UINT id) {
	if (m_nFocusedID == ID_GRAPHS_OVERLAY) {
		return GraphIDtoPtr(m_nFocusedID)->IsIDincluded(id);
	}

	return (m_nFocusedID == id);
}


/***************************************************************************/
/***************************************************************************/
UINT CSaView::GraphPtrtoID(CGraphWnd * pGraph) {
	int offset = GraphPtrToOffset(pGraph);
	if (offset >= 0) {
		return m_anGraphID[offset];
	}
	return UINT(-1);
}

/***************************************************************************/
/***************************************************************************/
int CSaView::GraphPtrToOffset(CGraphWnd * pGraph) {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] == pGraph) {
			return nLoop;
		}
	}
	return -1;
}

/***************************************************************************/
/***************************************************************************/
CGraphWnd * CSaView::GraphIDtoPtr(UINT id) {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_anGraphID[nLoop] == id) {
			return m_apGraphs[nLoop];
		}
	}
	return NULL;
}

/***************************************************************************/
/***************************************************************************/
BOOL CSaView::GraphIDincluded(UINT id) {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (id == m_anGraphID[nLoop]) {
			return TRUE;
		}
	}
	return FALSE;
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnAddOverlay() {
	// if the focused graph is mergeable, bring up a list of all other
	// graphs that can be merged with it, (m_pPickOverlay) then
	// call ChangeGraph to merge them in.
	if (m_pFocusedGraph && CGraphWnd::IsMergeableGraph(m_pFocusedGraph, TRUE)) {
		// get pointer to document
		CSaDoc * pModel = GetDocument();
		m_pPickOverlay->ResetGraphsPtr();

		POSITION position = pModel->GetTemplate()->GetFirstDocPosition();
		while (position != NULL) {
			// get pointer to document
			CDocument * pNextDoc = pModel->GetTemplate()->GetNextDoc(position);
			if (pNextDoc) {
				POSITION pos = pNextDoc->GetFirstViewPosition();
				while (pos != NULL) {
					CSaView * pView = (CSaView *)(pNextDoc->GetNextView(pos));
					m_pPickOverlay->SetGraphsPtr(pView->m_apGraphs, m_pFocusedGraph);
				}
			}
		}

		if ((m_pPickOverlay->DoModal() == IDOK) &&
			(m_pPickOverlay->GraphsCount())) {
			ChangeGraph(ID_GRAPHS_OVERLAY);
		}
	}
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnUpdateAddOverlay(CCmdUI * pCmdUI) {
	BOOL enable = FALSE;

	if (m_pFocusedGraph && CGraphWnd::IsMergeableGraph(m_pFocusedGraph, TRUE)) {
		m_pPickOverlay->ResetGraphsPtr();

		POSITION position = GetDocument()->GetTemplate()->GetFirstDocPosition();
		while (position != NULL) {
			// get pointer to document
			CDocument * pModel = GetDocument()->GetTemplate()->GetNextDoc(position);
			if (pModel) {
				POSITION pos = pModel->GetFirstViewPosition();
				while (pos != NULL) {
					CSaView * pView = (CSaView *)(pModel->GetNextView(pos));
					m_pPickOverlay->SetGraphsPtr(pView->m_apGraphs, m_pFocusedGraph);
				}
			}
		}

		if (m_pPickOverlay->GraphsCount()) {
			enable = TRUE;
		}
	}
	pCmdUI->Enable(enable);
	pCmdUI->SetCheck(FALSE);
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnRemoveOverlay() {
	if (m_pFocusedGraph && (m_nFocusedID == ID_GRAPHS_OVERLAY)) {
		m_pPickOverlay->ResetPlots();
		CMultiPlotWnd * pMPlot = (CMultiPlotWnd *)(m_pFocusedGraph->GetPlot());
		POSITION pos = pMPlot->m_List.GetHeadPosition();
		while (pos) {
			CPlotWndInfo * pInfo = pMPlot->m_List.GetNext(pos);
			m_pPickOverlay->AddPlot(pInfo->m_pPlot, pInfo->m_pDoc->GetPathName());
		}

		if (m_pPickOverlay->DoModal() == IDOK) {
			for (int i = (pMPlot->m_List.GetCount() - 1); i >= 0; i--) {
				if (m_pPickOverlay->IsItemSelected(i)) {
					m_pFocusedGraph->RemoveOverlayItem(m_pPickOverlay->GetPlot(i));
				}
			}
			int numPlots = pMPlot->GetNumPlots();
			if (numPlots > 1) {
				m_pFocusedGraph->RedrawGraph(TRUE, TRUE, TRUE);
			} else {
				if (numPlots == 0) {
					ChangeGraph(IDD_BLANK);
				} else {
					ASSERT(numPlots == 1);
					UINT plotID = pMPlot->GetBasePlotID();

					int nLoop;
					for (nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
						if (m_anGraphID[nLoop] == plotID) {
							break; // graph exists already
						}
					}
					if (nLoop >= MAX_GRAPHS_NUMBER) {
						ChangeGraph(plotID);
					} else {
						// since the last graph already exists and we don't
						// wan't to create two of them, we will just make it a blank graph.
						ChangeGraph(IDD_BLANK);
					}
				}
			}
		}
	}
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnRemoveOverlays() {
	if (m_pFocusedGraph && (m_nFocusedID == ID_GRAPHS_OVERLAY)) {
		CMultiPlotWnd * pMPlot = (CMultiPlotWnd *)(m_pFocusedGraph->GetPlot());

		UINT plotID = pMPlot->GetBasePlotID();

		int nLoop;
		for (nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
			if (m_anGraphID[nLoop] == plotID) {
				break; // graph exists already
			}
		}
		if (nLoop >= MAX_GRAPHS_NUMBER) {
			ChangeGraph(plotID);
		} else {
			// since the last graph already exists and we don't
			// wan't to create two of them, we will just make it a blank graph.
			ChangeGraph(IDD_BLANK);
		}
	}
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnUpdateRemoveOverlay(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_nFocusedID == ID_GRAPHS_OVERLAY);
	pCmdUI->SetCheck(FALSE);
}

/***************************************************************************/
/***************************************************************************/
void CSaView::ShowInitialStateAndZ() {

	CSaApp * pApp = (CSaApp*)AfxGetApp();
	CMDIChildWnd * pwnd = pwndChildFrame();

	// Minimize this window if it had been when SA was closed.
	if (m_eInitialShowCmd == SW_SHOWMINIMIZED) {
		pwnd->ShowWindow(m_eInitialShowCmd);
	}

	// Find the MDI child window which corresponds to the one --out of those
	// opened so far-- just above this when SA was closed.
	int zThis = GetZ();
	CSaView * pviewAboveThis = NULL;

	CSaView * pview = pApp->GetViewBelow(this);
	for (; pview; pview = pApp->GetViewBelow(pview))
		if (zThis < pview->GetZ()) {
			pviewAboveThis = pview;
		}

	// Set this window's z-order. (The framework has opened it on top.)
	if (pviewAboveThis) {
		BOOL bSetZ = pwnd->SetWindowPos(pviewAboveThis->pwndChildFrame(), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		ASSERT(bSetZ);
	}
}

/***************************************************************************/
/***************************************************************************/
void CSaView::ShowInitialTopState() {

	CSaApp * pApp = (CSaApp*)AfxGetApp();
	CMDIChildWnd * pwnd = pwndChildFrame();

	// Make this the active MDI child window, since it had been
	// the top when SA was closed.
	CMainFrame * pwndMainFrame = (CMainFrame *)pApp->m_pMainWnd;
	ASSERT(pwndMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	pwndMainFrame->MDIActivate(pwnd);

	// Maximize this window if it had been when SA was closed.
	if (m_eInitialShowCmd == SW_SHOWMAXIMIZED) {
		pwnd->ShowWindow(m_eInitialShowCmd);
	}
}

/***************************************************************************/
/***************************************************************************/
CMDIChildWnd * CSaView::pwndChildFrame() const {
	CMDIChildWnd * pwnd = (CMDIChildWnd *)GetParent();
	ASSERT(pwnd);
	ASSERT(pwnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)));

	return pwnd;
}

/***************************************************************************/
/***************************************************************************/
CSaView * CSaView::GetViewActiveChild(CMDIChildWnd * pwnd) {
	ASSERT(pwnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)));
	CSaView * pview = (CSaView *)pwnd->GetActiveView();
	ASSERT(pview);
	ASSERT(pview->IsKindOf(RUNTIME_CLASS(CSaView)));

	return pview;
}

/***************************************************************************/
/***************************************************************************/
void CSaView::SetObjectStream(CObjectIStream & obs) {
	ASSERT(!s_pobsAutoload);
	s_pobsAutoload = &obs;
}

/***************************************************************************/
/***************************************************************************/
void CSaView::ClearObjectStream() {
	s_pobsAutoload = NULL;
}

/***************************************************************************/
static LPCSTR psz_saview = "saview";
static LPCSTR psz_placement = "placement";
static LPCSTR psz_z = "z";
static LPCSTR psz_legendall = "legendall";
static LPCSTR psz_legendnone = "legendnone";
static LPCSTR psz_xscaleall = "xscaleall";
static LPCSTR psz_xscalenone = "xscalenone";
static LPCSTR psz_transcription_boundaries = "transcriptionboundaries";
static LPCSTR psz_drawstyleline = "drawstyleline";
static LPCSTR psz_updateboundaries = "updateboundaries";
static LPCSTR psz_graphlist = "graphlist";
static LPCSTR psz_graphid = "graphid";
static LPCSTR psz_layout = "layout2";
static LPCSTR psz_annotallornonelist = "annotallornonelist";
static LPCSTR psz_annotall = "annotall";
static LPCSTR psz_annotnone = "annotnone";

/***************************************************************************/
/***************************************************************************/
void CSaView::WriteProperties(CObjectOStream & obs) {

	// Write the marker beginning the properties for this view.
	obs.WriteNewline();
	obs.WriteBeginMarker(psz_saview);

	// save the window placement.
	if (m_bViewCreated && GetSafeHwnd()) {
		WINDOWPLACEMENT wpl;
		GetParent()->GetWindowPlacement(&wpl);
		obs.WriteWindowPlacement(psz_placement, wpl);
		obs.WriteInteger(psz_z, m_z);
	}

	// TODO - save more data members.

	obs.WriteBeginMarker(psz_graphlist);

	// Save the properties for each graph that exists.
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++) {
		if (m_apGraphs[i] != NULL) {
			int nID = m_anGraphID[i];
			if (nID == ID_GRAPHS_OVERLAY) {
				CMultiPlotWnd * pPlot = (CMultiPlotWnd *)m_apGraphs[i]->GetPlot();
				if (pPlot) {
					nID = pPlot->GetBasePlotID();
				} else {
					nID = IDD_BLANK;
				}
			}

			obs.WriteUInt(psz_graphid, nID);
			m_apGraphs[i]->WriteProperties(obs);
		}
	}

	obs.WriteEndMarker(psz_graphlist);

	// Write page layout properties if they exist.
	if (m_pPageLayout) {
		m_pPageLayout->WriteProperties(obs);
	}

	// Write layout, legend and x-scale settings.
	obs.WriteUInt(psz_layout, m_nLayout);
	obs.WriteBool(psz_legendall, m_bLegendAll);
	obs.WriteBool(psz_legendnone, m_bLegendNone);
	obs.WriteBool(psz_xscaleall, m_bXScaleAll);
	obs.WriteBool(psz_xscalenone, m_bXScaleNone);

	// Write the global properties for the annotation windows.
	obs.WriteBeginMarker(psz_annotallornonelist);
	for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
		obs.WriteBool(psz_annotall, m_abAnnAll[i]);
		obs.WriteBool(psz_annotnone, m_abAnnNone[i]);
	}
	obs.WriteEndMarker(psz_annotallornonelist);

	// Write other misc. global properties.
	obs.WriteBool(psz_transcription_boundaries, m_bTranscriptionBoundaries);
	obs.WriteBool(psz_drawstyleline, m_bDrawStyleLine);
	obs.WriteBool(psz_updateboundaries, m_bUpdateBoundaries);

	obs.WriteEndMarker(psz_saview);
	obs.WriteNewline();
}

/***************************************************************************/
/***************************************************************************/
BOOL CSaView::ReadProperties(CObjectIStream & obs, BOOL bCreateGraphs) {

	CWnd * pwndFrame = NULL;
	WINDOWPLACEMENT wpl;

	if (GetSafeHwnd()) {
		pwndFrame = GetParent();
	}

	if (!obs.bReadBeginMarker(psz_saview)) {
		return FALSE;
	}
	ASSERT(m_pPageLayout);

	while (!obs.bAtEnd()) {
		if ((pwndFrame != NULL) && (obs.bReadWindowPlacement(psz_placement, wpl))) {
			m_eInitialShowCmd = wpl.showCmd;
			wpl.showCmd = SW_HIDE;
			pwndFrame->SetWindowPlacement(&wpl);
		} else if (obs.bReadInteger(psz_z, m_z));
		else if (ReadGraphListProperties(obs, bCreateGraphs));
		else if (m_pPageLayout->ReadProperties(obs));
		else if (obs.bReadUInt(psz_layout, m_nLayout));
		else if (obs.bReadBool(psz_legendall, m_bLegendAll));
		else if (obs.bReadBool(psz_legendnone, m_bLegendNone));
		else if (obs.bReadBool(psz_xscaleall, m_bXScaleAll));
		else if (obs.bReadBool(psz_xscalenone, m_bXScaleNone));
		else if (obs.bReadBool(psz_transcription_boundaries, m_bTranscriptionBoundaries));
		else if (obs.bReadBool(psz_drawstyleline, m_bDrawStyleLine));
		else if (obs.bReadBool(psz_updateboundaries, m_bUpdateBoundaries));
		else if (obs.bReadBeginMarker(psz_annotallornonelist)) {
			for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
				obs.bReadBool(psz_annotall, m_abAnnAll[i]);
				obs.bReadBool(psz_annotnone, m_abAnnNone[i]);
			}
			obs.SkipToEndMarker(psz_annotallornonelist);
		} else if (obs.bReadEndMarker(psz_saview)) {
			break;
		} else {
			// Skip unexpected field
			obs.ReadMarkedString();
		}
	}

	UINT nDefaultLayout = SetLayout(m_anGraphID);
	if (GetNumberOfGraphsInLayout(m_nLayout) != GetNumberOfGraphsInLayout(nDefaultLayout)) {
		m_nLayout = nDefaultLayout;
	}

	return TRUE;
}

/***************************************************************************/
// This function to read graph properties is used when graphs are not being
// created from a template view. When bCreateGraphs is false it means the
// graphs being read are properties for graphs in the permanent template
// view.
/***************************************************************************/
BOOL CSaView::ReadGraphListProperties(CObjectIStream & obs, BOOL bCreateGraphs) {

	if (!obs.bReadBeginMarker(psz_graphlist)) {
		return FALSE;
	}

	int  i = 0;
	UINT id = 0;
	CSaApp * pApp = (CSaApp*)AfxGetApp();
	UINT openAsID = pApp->GetOpenAsID();

	memset(&m_apGraphs[0], 0, sizeof(CGraphWnd *) * MAX_GRAPHS_NUMBER);
	memset(&m_anGraphID[0], 0, sizeof(UINT) * MAX_GRAPHS_NUMBER);

	while (!obs.bAtEnd()) {
		if (obs.bReadUInt(psz_graphid, id)) {

			//*********************************************************
			// If we're suppposed to create visible graphs then create
			// the appropriate graph (i.e. a position view or other).
			//*********************************************************
			if (bCreateGraphs && openAsID == ID_FILE_OPEN) {
				if (id == IDD_RECORDING) {
					m_apGraphs[i] = CreateRecGraph(NULL, &obs);
					m_anGraphID[i] = IDD_RECORDING;
				} else {
					CreateGraph(i, id, CREATE_FROMSTREAM, &obs);
				}
			} else {
				BOOL bSkipToEnd = TRUE;

				//*******************************************************
				// At this point, it's likely we're reading properties
				// for the default template view. Therefore, the graphs
				// aren't created to be visible but are created so
				// views opened in the future can use the template's
				// properties.
				//*******************************************************
				// we shouldn't be destroying a pointer to a graph
				ASSERT(m_apGraphs[i] == NULL);
				m_apGraphs[i] = new CGraphWnd(id);
				if (m_apGraphs[i] != NULL) {
					m_anGraphID[i] = id;
					m_WeJustReadTheProperties = m_apGraphs[i]->ReadProperties(obs);
					bSkipToEnd = FALSE;
				}

				//*******************************************************
				// If for some reason the construction of graphs failed
				// then skip to the end marker for this section.
				//*******************************************************
				if (bSkipToEnd) {
					// move past this graph record
					obs.SkipToEndMarker(psz_sagraph);
				}
			}

			if (m_apGraphs[i] != NULL) {
				// successfully filled in a space, move to next space.
				i++;
			}
		} else if (obs.bReadEndMarker(psz_graphlist)) {
			break;
		}
	}

	// RLJ 06/06/2000 - If OpenAsID = "Phonetic/Music Analysis", then create corresponding graphs.
	if ((bCreateGraphs) && (openAsID != ID_FILE_OPEN)) {
		CreateOpenAsGraphs(openAsID);
	}

	return TRUE;
}

/***************************************************************************/
// This function to read graph properties is used when the graphs are being
// created from a template view.
/***************************************************************************/
BOOL CSaView::ReadGraphListProperties(const CSaView & pTemplateView) {

	CSaApp * pApp = (CSaApp*)AfxGetApp();
	int openAsID = pApp->GetOpenAsID();
	if (openAsID == ID_FILE_OPEN) {
		for (int i = 0, nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
			if (pTemplateView.m_apGraphs[nLoop] != NULL) {
				CreateGraph(i, pTemplateView.m_anGraphID[nLoop], CREATE_FROMGRAPH, NULL, pTemplateView.m_apGraphs[nLoop]);
			}
			if (m_apGraphs[i] != NULL) {
				i++;
			}
		}
	} else {
		// RLJ 06/06/2000 - If openAsID = "Phonetic/Music Analysis", then create corresponding graphs.
		CreateOpenAsGraphs(openAsID);
	}
	return TRUE;
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnEditInplace() {

	m_advancedSelection.Update(this);
	int nAnnotationIndex = m_advancedSelection.GetSelectionIndex();

	if ((nAnnotationIndex != -1) &&
		(GetFocusedGraphWnd() != NULL) &&
		(GetFocusedGraphWnd()->HaveAnnotation(nAnnotationIndex))) {
		// Selected annotation is visible
		CAnnotationWnd * pWnd = GetFocusedGraphWnd()->GetAnnotationWnd(nAnnotationIndex);
		pWnd->OnCreateEdit();
	}
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {

	m_advancedSelection.Update(this);
	int nAnnotationIndex = m_advancedSelection.GetSelectionIndex();

	if (nChar < 32) {
		CView::OnChar(nChar, nRepCnt, nFlags);
		return;
	}

	if ((nAnnotationIndex != -1) &&
		(GetFocusedGraphWnd() != NULL) &&
		(GetFocusedGraphWnd()->HaveAnnotation(nAnnotationIndex))) {
		// Selected annotation is visible
		CSaString szString(static_cast<TCHAR>(nChar));
		CAnnotationWnd * pWnd = GetFocusedGraphWnd()->GetAnnotationWnd(nAnnotationIndex);
		pWnd->OnCreateEdit(&szString);
	}
}

/***************************************************************************/
// CSaView::BroadcastMessage  sends a message to every graph
/***************************************************************************/
void CSaView::BroadcastMessage(UINT Message, WPARAM wParam, LPARAM lParam) {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (!m_apGraphs[nLoop]) {
			break;
		}
		m_apGraphs[nLoop]->SendMessage(Message, wParam, lParam);
	}
}

/***************************************************************************/
// CSaView::OnAppMessage App notification
/***************************************************************************/
LRESULT CSaView::OnAppMessage(WPARAM wParam, LPARAM /*lParam*/) {
	switch (wParam) {
	case HINT_APP_DOC_LIST_CHANGED:
		OnVerifyOverlays();
		break;
	}
	return TRUE;
}

void CSaView::OnVerifyOverlays() {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_anGraphID[nLoop] == ID_GRAPHS_OVERLAY) {
			CMultiPlotWnd * pMPlot = (CMultiPlotWnd *)(m_apGraphs[nLoop]->GetPlot());
			int numPlots = pMPlot->GetNumPlots();

			pMPlot->VerifyPlots();

			numPlots = pMPlot->GetNumPlots();

			if (numPlots > 1) {
				m_apGraphs[nLoop]->RedrawGraph(TRUE, TRUE, TRUE);
			} else if (numPlots == 0) {
				ChangeGraph(IDD_BLANK);
			} else {
				ASSERT(numPlots == 1);
				UINT plotID = pMPlot->GetBasePlotID();
				int i2;
				for (i2 = 0; i2 < MAX_GRAPHS_NUMBER; i2++) {
					if (m_anGraphID[i2] == plotID) {
						break; // graph exists already
					}
				}
				if (i2 >= MAX_GRAPHS_NUMBER) {
					ChangeGraph(nLoop, plotID);
				} else {
					// since the last graph already exists and we don't
					// want to create two of them, we will just make it a blank graph.
					ChangeGraph(IDD_BLANK);
				}
			}
		}
	}
}

//**********************************************************
// CSaString CSaView::GetGraphsDescription()
// Build a string from graph captions.
//**********************************************************
CSaString CSaView::GetGraphsDescription() const {
	return GetGraphsDescription(m_anGraphID);
}

CSaString CSaView::GetGraphsDescription(const UINT * anGraphID) {
	CSaString szDescription;
	for (int i = 0; anGraphID[i]; i++) {
		if (i > 0) {
			szDescription += ", ";
		}
		int nID = anGraphID[i];
		szDescription += GetGraphTitle(nID);
	}
	return szDescription;
}

void CSaView::RemoveRtPlots() {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			m_apGraphs[nLoop]->RemoveRtPlots();
		}
	}
}


// Override default SetScrollRange function so that we do not hide the scroll bar
void CSaView::SetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL bRedraw) {
	if (nBar == SB_HORZ) {
		SCROLLINFO info;

		info.nMin = nMinPos;
		info.nMax = nMaxPos;
		info.cbSize = sizeof(info);
		info.fMask = SIF_RANGE | SIF_DISABLENOSCROLL;

		SetScrollInfo(nBar, &info, bRedraw);
	} else {
		CView::SetScrollRange(nBar, nMinPos, nMaxPos, bRedraw);
	}
}

/***************************************************************************/
// CSaView::OnKeyDown Keyboard interface
/***************************************************************************/
void CSaView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {

	CProgressStatusBar * pStatusBar = NULL;
	CProcess * pProcessOwner = NULL;
	CRect rWnd;

	// are these keys already pressed?
	bool ctrlPressed = (GetAsyncKeyState(VK_CONTROL) < 0);
	bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) < 0);

	if ((!ctrlPressed) && (!shiftPressed)) {
		switch (nChar) {
		case VK_HOME:
			OnHScroll(SB_LEFT, 0, GetScrollBarCtrl(SB_HORZ)); // scroll to minimum
			break;
		case VK_END:
			OnHScroll(SB_RIGHT, 0, GetScrollBarCtrl(SB_HORZ)); // scroll to maximum
			break;
		case VK_RIGHT:
			OnHScroll(SB_LINERIGHT, 0, GetScrollBarCtrl(SB_HORZ)); // scroll right
			break;
		case VK_LEFT:
			OnHScroll(SB_LINELEFT, 0, GetScrollBarCtrl(SB_HORZ)); // scroll left
			break;
		case VK_UP:
			// zoom out
			OnVScroll(SB_LINEUP, 0, GetScrollBarCtrl(SB_VERT));
			break;
		case VK_DOWN:
			// zoom in
			OnVScroll(SB_LINEDOWN, 0, GetScrollBarCtrl(SB_VERT));
			break;
		case VK_NEXT:
			// page down
			// scroll left one page
			OnHScroll(SB_PAGEUP, 0, GetScrollBarCtrl(SB_HORZ));
			break;
		case VK_PRIOR:
			// page up
			// scroll right one page
			OnHScroll(SB_PAGEDOWN, 0, GetScrollBarCtrl(SB_HORZ));
			break;
		case VK_ESCAPE:
			// process interrupt from user and stop player
			if (GetMainFrame().IsPlayerPlaying()) {
				// send message to stop player
				GetMainFrame().SendMessage(WM_COMMAND, ID_PLAYER_STOP, 0L);
			} else if (!IsAnimating()) {
				// Do not cancel processes during animation
				// get pointer to status bar
				pStatusBar = (CProgressStatusBar *)GetMainFrame().GetProgressStatusBar();
				// get the current process owner
				pProcessOwner = (CProcess *)pStatusBar->GetProcessOwner();
				if (pProcessOwner) {
					// cancel the process
					pProcessOwner->CancelProcess();
				} else {
					m_advancedSelection.DeselectAnnotations(this);
				}
			}
			break;
		default:
			CView::OnKeyDown(nChar, nRepCnt, nFlags);
			break;
		}
	} else if ((shiftPressed) && (!ctrlPressed)) {
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
	} else if ((ctrlPressed) && (!shiftPressed)) {
		switch (nChar) {
		case VK_END:
			OnVScroll(SB_BOTTOM, 0, GetScrollBarCtrl(SB_VERT)); // zoom to maximum
			break;
		default:
			CView::OnKeyDown(nChar, nRepCnt, nFlags);
			break;
		}
	} else {
		// everything else
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}


/**
* tie mouse wheel movements to the zoom control
*
* nFlags
*    Indicates whether various virtual keys are down. This parameter can be any combination of the following values:
*        MK_CONTROL   Set if the CTRL key is down.
*        MK_LBUTTON   Set if the left mouse button is down.
*        MK_MBUTTON   Set if the middle mouse button is down.
*        MK_RBUTTON   Set if the right mouse button is down.
*        MK_SHIFT   Set if the SHIFT key is down.
* zDelta
*    Indicates distance rotated. The zDelta value is expressed in multiples or divisions of WHEEL_DELTA, which is 120. A value less than zero indicates rotating back (toward the user) while a value greater than zero indicates rotating forward (away from the user). The user can reverse this response by changing the Wheel setting in the mouse software. See the Remarks for more information about this parameter.
* pt
*    Specifies the x- and y-coordinate of the cursor. These coordinates are always relative to the upper-left corner of the screen.
*/
BOOL CSaView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
	//TRACE("OnMouseWheel %d\n",zDelta);
	if (zDelta < 0) {
		double fZoom = 1.25*m_fZoom;
		ZoomIn(fZoom - m_fZoom);
	} else {
		double fZoom = (1.0 / 1.25)*m_fZoom;
		ZoomOut(-(fZoom - m_fZoom));
	}
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CSaView::EnableScrolling(bool value) {
	enableScrolling = value;
}

void CSaView::ToggleSegmentSelection(CSegment * pSegment, int index) {
	DWORD dwStart = pSegment->GetOffset(index);
	DWORD dwStop = pSegment->GetStop(index);
	ChangeAnnotationSelection(pSegment, index, dwStart, dwStop);
	// refresh the graphs between cursors
	RedrawGraphs(FALSE);
}

/***************************************************************************/
// CSaView::OnImportSAB Import SAB text file and automatically process it
/***************************************************************************/
void CSaView::OnImportSAB() {

	// Get Export File Type and Name
	// load file name
	CSaString szTitle = ((CSaDoc *)GetDocument())->GetFilenameFromTitle().c_str();
	int nFind = szTitle.ReverseFind('.');
	if (nFind >= ((szTitle.GetLength() > 3) ? (szTitle.GetLength() - 4) : 0)) {
		// remove extension
		szTitle = szTitle.Left(nFind);
	}
	CSaString szFilter = "Audio-Sync Phrases (*.phrases) |*.phrases|All Files (*.*) |*.*||";
	CFileDialog dlgFile(TRUE, _T("phrases"), szTitle, OFN_HIDEREADONLY, szFilter, NULL);
	if (dlgFile.DoModal() != IDOK) {
		return;
	}

	CSaString szPath = dlgFile.GetPathName();

	CMainFrame * pMainFrame = (CMainFrame*)AfxGetMainWnd();
	CSaDoc * pModel = GetDocument();
	pModel->ImportSAB(*this, szPath, pMainFrame->GetAudioSyncAlgorithm());

	// refresh the graphs between cursors
	RedrawGraphs(TRUE);
}

void CSaView::OnUpdateFilenew(CCmdUI * pCmdUI) {
	pCmdUI->Enable(FALSE);
}

/***************************************************************************/
// CSaView::OnCreate View creation
/***************************************************************************/
int CSaView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CView::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	ModifyStyle(0, WS_CLIPCHILDREN);

	// add one view
	GetMainFrame().ComputeNumberOfViews(1);
	m_bViewCreated = TRUE;
	return 0;
}

/***************************************************************************/
// CSaView::IDDSelected - TCJRLJ
//  Return TRUE if the check box associated is selected; else return FALSE
/***************************************************************************/
BOOL CSaView::IDDSelected(const UINT * pGraphIDs, UINT nIDD) {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if ((*(pGraphIDs + nLoop)) == nIDD) {
			return TRUE;
		}
	}
	return FALSE;
}

/***************************************************************************/
// CSaView::GetGraphIndexForIDD - TCJ 3/27/00
//  Return the index of the graph that has the given IDD.
/***************************************************************************/
int CSaView::GetGraphIndexForIDD(UINT nIDD) {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_anGraphID[nLoop] == nIDD) {
			return nLoop;
		}
	}
	return -1;
}

/***************************************************************************/
// CSaView::CreateOpenAsGraphs Create graphs for "File-->Open As"
// Create graphs for "File-->Open As" (Phonetic/Music Analysis)
/***************************************************************************/
void CSaView::CreateOpenAsGraphs(UINT openAsID) {
	// RLJ 07/28/2000
	DeleteGraphs();
	switch (openAsID) {
	case ID_FILE_OPENAS_MUSICANALYSIS:
		// Open *.Wav & display Raw Waveform, Melogram, and Tonal Weighting Chart -- OpenMA
		m_nLayout = ID_LAYOUT_2A; // Use default layout.  // RLJ 07/28/2000
		CreateGraph(0, IDD_RECORDING, CREATE_FROMSCRATCH);
		CreateGraph(1, IDD_MELOGRAM, CREATE_FROMSCRATCH);
		CreateGraph(2, IDD_TWC, CREATE_FROMSCRATCH);
		CreateGraph(3, IDD_MAGNITUDE, CREATE_FROMSCRATCH);
		// DDO - 08/09/2000
		CreateGraph(4, IDD_STAFF, CREATE_FROMSCRATCH);
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

	// Restore default file open mode.
	CSaApp * pApp = (CSaApp*)AfxGetApp();
	pApp->SetOpenMore(true);
	pApp->SetOpenAsID(ID_FILE_OPEN);
	// retile the graphs
	// retile graphs
	OnGraphsRetile();
}

/***************************************************************************/
// CSaView::OnGraphsTypes Sets the displayed graph types
// The function keeps the already opened graphs and their settings and adds
// the new chosen graphs and deletes the no longer chosen graphs. The order
// will be after the new selection. The graphs will be retiled.
/***************************************************************************/
void CSaView::OnGraphsTypes() {
	CDlgGraphsTypes dlg(this, m_anGraphID, m_nLayout);

	//**************************************************************
	// If the user chose OK then store the chosen IDs in a temp.
	// array and process the request.
	//**************************************************************
	if (dlg.DoModal() == IDOK) {
		UINT anNewGraphID[MAX_GRAPHS_NUMBER];
		int nLayout = -1;
		dlg.GetCheckedGraphs(&anNewGraphID[0], &nLayout);
		OnGraphsTypesPostProcess(&anNewGraphID[0], nLayout);
	}
}

void CSaView::OnShowBorders() {

	// if one or more aren't on, turn everything on, otherwise, turn them off
	BOOL bSegmentBoundariesShown = TRUE;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] == NULL) continue;
		if (!m_apGraphs[nLoop]->HasBoundaries()) {
			bSegmentBoundariesShown = FALSE;
			break;
		}
	}

	bool bShow = ((!m_bTranscriptionBoundaries) || (!bSegmentBoundariesShown)) ? true : false;
	m_bTranscriptionBoundaries = bShow;

	// refresh the annotation windows
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->ShowTranscriptionBoundaries(bShow);
			m_apGraphs[nLoop]->SetBoundaries(bShow);
			m_apGraphs[nLoop]->RedrawPlot();
		}
	}
}

/***************************************************************************/
// CSaView::OnGraphTypesSelect Change the graphs
/***************************************************************************/
void CSaView::OnGraphTypesSelect(UINT nID) {
	int nConfiguration = nID - ID_GRAPHTYPES_SELECT_FIRST;

	UINT anNewGraphID[MAX_GRAPHS_NUMBER];
	int nLayout = -1;
	CDlgGraphsTypes::GetStandardCheckedGraphs(nConfiguration, &anNewGraphID[0], &nLayout);
	OnGraphsTypesPostProcess(&anNewGraphID[0], nLayout);
}

/***************************************************************************/
// CSaView::OnUpdateGraphTypesSelect Menu update
/***************************************************************************/
void CSaView::OnUpdateGraphTypesSelect(CCmdUI * pCmdUI) {
	if (pCmdUI->m_nID >= ID_GRAPHTYPES_SELECT_FIRST && pCmdUI->m_nID <= ID_GRAPHTYPES_SELECT_LAST) {
		pCmdUI->Enable();
	} else {
		pCmdUI->Enable(FALSE);
	}
}

/***************************************************************************/
// CSaView::OnGraphsTypesPostProcess
/***************************************************************************/
void CSaView::OnGraphsTypesPostProcess(const UINT * anNewGraphID, int nLayout) {
	UINT anTmpGraphID[MAX_GRAPHS_NUMBER];
	CGraphWnd * apTmpGraphs[MAX_GRAPHS_NUMBER];

	//**************************************************************
	// Initialize the arrays that will be a temporary holding place
	// for the chosen graph IDs and graph pointers.
	//**************************************************************
	memset(&apTmpGraphs[0], 0, sizeof(CGraphWnd *) * MAX_GRAPHS_NUMBER);
	memset(&anTmpGraphID[0], 0, sizeof(UINT) * MAX_GRAPHS_NUMBER);

	int index = 0;

	//**************************************************************
	// Make sure the position data comes first in the array so it will
	// display first.
	//**************************************************************
	if (IDDSelected(&anNewGraphID[0], IDD_RECORDING)) {
		anTmpGraphID[index++] = IDD_RECORDING;
	}

	//**************************************************************
	// Finally, all other chosen graphs that aren't one of the
	// graph IDs already copied to the temp. ID array should be
	// copied to the temp. array.
	//**************************************************************
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++) {
		if ((anNewGraphID[i] != NULL) && (anNewGraphID[i] != IDD_RECORDING)) {
			anTmpGraphID[index++] = anNewGraphID[i];
		}
	}

	//**************************************************************
	// Loop through all the graphs and delete all graphs that we
	// will not need.
	// NOTE: Do this first because the destructors want the graph
	// list intact to be able to inter communicate.
	//**************************************************************
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++) {
		if ((m_apGraphs[i] != NULL) &&
			(!IDDSelected(&anTmpGraphID[0], m_anGraphID[i]))) {
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
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++) {
		if (anTmpGraphID[i] && IDDSelected(&m_anGraphID[0], anTmpGraphID[i])) {
			index = GetGraphIndexForIDD(anTmpGraphID[i]);
			if (index >= 0) {
				apTmpGraphs[i] = m_apGraphs[index];
				m_apGraphs[index] = NULL;
				m_anGraphID[index] = 0;
			}
		}
	}

	if (nLayout == -1) {
		m_nLayout = SetLayout(&anTmpGraphID[0]);
	} else {
		m_nLayout = nLayout;
	}

	//**************************************************************
	// For those graph IDs in the temp. graph ID array that don't
	// have corresponding graph pointers, create graphs.
	//**************************************************************
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++) {
		ASSERT(m_apGraphs[i] == NULL);
		m_anGraphID[i] = anTmpGraphID[i];
		if (anTmpGraphID[i] && !apTmpGraphs[i]) {
			CreateGraph(i, m_anGraphID[i]);
		} else {
			m_apGraphs[i] = apTmpGraphs[i];
		}
	}

	OnGraphsRetile();
}

/***************************************************************************/
// CSaView::OnUpdateGraphsTypes Menu Update
/***************************************************************************/
void CSaView::OnUpdateGraphsTypes(CCmdUI * pCmdUI) {
	pCmdUI->Enable(GetDocument()->GetDataSize() != 0); // enable if data is available
}

/***************************************************************************/
// CSaView::OnUpdateShowBorders Menu Update
/***************************************************************************/
void CSaView::OnUpdateShowBorders(CCmdUI * pCmdUI) {
	pCmdUI->Enable(TRUE);
}

/***************************************************************************/
// CSaView::OnGraphsParameters Changing the graph parameters
/***************************************************************************/
void CSaView::OnGraphsParameters() {
	CSaString szCaption;
	// load caption string
	szCaption.LoadString(IDS_DLGTITLE_GRAPHSPARA);
	// set the pitch parameters to enable manual analysing
	CSaDoc * pModel = (CSaDoc *)GetDocument();
	// create the property sheet according to the existing graphs
	CDlgGraphsParameters * dlgGraphsParameters; // graph parameters dialog

	dlgGraphsParameters = new CDlgGraphsParameters(szCaption, NULL);

	// set workbench process
	BOOL bProcessChange = FALSE;
	dlgGraphsParameters->m_dlgRawdataPage.m_nProcess = pModel->GetWbProcess();
	if ((dlgGraphsParameters->DoModal() == IDOK) &&
		(pModel->GetWbProcess() != dlgGraphsParameters->m_dlgRawdataPage.m_nProcess)) {
		pModel->SetWbProcess(dlgGraphsParameters->m_dlgRawdataPage.m_nProcess);
		bProcessChange = TRUE;
	}
	delete dlgGraphsParameters;

	// process workbench if necessary
	if ((bProcessChange) && (pModel->WorkbenchProcess(TRUE, TRUE))) {
		RedrawGraphs(TRUE, TRUE);
	}
}

/***************************************************************************/
// CSaView::OnUpdateGraphsParameters Menu Update
/***************************************************************************/
void CSaView::OnUpdateGraphsParameters(CCmdUI * pCmdUI) {
	pCmdUI->Enable(GetDocument()->GetDataSize() != 0); // enable if data is available
}

/***************************************************************************/
// CSaView::OnEditBoundaries
/***************************************************************************/
void CSaView::OnEditBoundaries() {
	m_bEditBoundaries = !m_bEditBoundaries;
	m_bEditSegmentSize = false;
}

/***************************************************************************/
// CSaView::OnUpdateEditBoundaries
/***************************************************************************/
void CSaView::OnUpdateEditBoundaries(CCmdUI * pCmdUI) {
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(GetEditBoundaries() == BOUNDARIES_EDIT_BOUNDARIES);
}

/***************************************************************************/
// CSaView::GetEditBoundaries
/***************************************************************************/
EBoundary CSaView::GetEditBoundaries(bool checkKeys) {
	if ((m_bEditSegmentSize) && (checkKeys)) {
		return BOUNDARIES_EDIT_SEGMENT_SIZE;
	}
	if ((m_bEditBoundaries) && (checkKeys)) {
		return BOUNDARIES_EDIT_BOUNDARIES;
	}
	return BOUNDARIES_EDIT_NULL;
}

/***************************************************************************/
// CSaView::OnEditSegmentSize
/***************************************************************************/
void CSaView::OnEditSegmentSize() {
	m_bEditSegmentSize = !m_bEditSegmentSize;
	m_bEditBoundaries = false;
}

/***************************************************************************/
// CSaView::OnUpdateEditSegmentSize
/***************************************************************************/
void CSaView::OnUpdateEditSegmentSize(CCmdUI * pCmdUI) {
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(GetEditBoundaries() == BOUNDARIES_EDIT_SEGMENT_SIZE);
}

/***************************************************************************/
// CSaView::OnExportStaff
/***************************************************************************/
void CSaView::OnExportStaff() {
	int nGraphIndex = GetGraphIndexForIDD(IDD_STAFF);
	CPlotStaff * pMusicStaff;
	if (nGraphIndex >= 0) {
		CGraphWnd * pMusicGraph = GetGraph(nGraphIndex);
		pMusicStaff = (CPlotStaff *)pMusicGraph->GetPlot();
		pMusicStaff->ExportFile();
	}
}


/***************************************************************************/
// CSaView::OnUpdateExportStaff
/***************************************************************************/
void CSaView::OnUpdateExportStaff(CCmdUI * pCmdUI) {
	int nGraphIndex = GetGraphIndexForIDD(IDD_STAFF);
	if (nGraphIndex >= 0) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}


/***************************************************************************/
// CSaView::OnImportStaff
/***************************************************************************/
void CSaView::OnImportStaff() {
	int nGraphIndex = GetGraphIndexForIDD(IDD_STAFF);
	CPlotStaff * pMusicStaff;
	if (nGraphIndex >= 0) {
		CGraphWnd * pMusicGraph = GetGraph(nGraphIndex);
		pMusicStaff = (CPlotStaff *)pMusicGraph->GetPlot();
		pMusicStaff->ImportFile();
	}
}


/***************************************************************************/
// CSaView::OnUpdateImportStaff
/***************************************************************************/
void CSaView::OnUpdateImportStaff(CCmdUI * pCmdUI) {
	int nGraphIndex = GetGraphIndexForIDD(IDD_STAFF);
	if (nGraphIndex >= 0) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSaView diagnostics

#ifdef _DEBUG
void CSaView::AssertValid() const {
	CView::AssertValid();
}

void CSaView::Dump(CDumpContext & dc) const {
	CView::Dump(dc);
}

#endif

CSaDoc * CSaView::GetDocument() { // non-debug version is inline
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSaDoc)));
	return (CSaDoc *)m_pDocument;
}

void CSaView::Clear() {

	// delete existing graph objects
	DeleteGraphs();
	if (m_pPageLayout) {
		delete m_pPageLayout;
		m_pPageLayout = NULL;
	}
	if (m_pPgLayoutBackup) {
		delete m_pPgLayoutBackup;
		m_pPgLayoutBackup = NULL;
	}
	if (m_pPickOverlay) {
		delete m_pPickOverlay;
		m_pPickOverlay = NULL;
	}
	if (m_pDlgAdvancedParseWords) {
		delete m_pDlgAdvancedParseWords;
		m_pDlgAdvancedParseWords = NULL;
	}
	if (m_pDlgAdvancedSegment) {
		delete m_pDlgAdvancedSegment;
		m_pDlgAdvancedSegment = NULL;
	}
	if (m_pDlgAdvancedParsePhrases) {
		delete m_pDlgAdvancedParsePhrases;
		m_pDlgAdvancedParsePhrases = NULL;
	}
}

void CSaView::PartialCopy(const CSaView & right) {

	m_nFocusedID = 0;
	// default layout
	m_nLayout = right.m_nLayout;
	m_bLegendAll = right.m_bLegendAll;
	m_bLegendNone = right.m_bLegendNone;
	m_bXScaleAll = right.m_bXScaleAll;
	m_bXScaleNone = right.m_bXScaleNone;
	m_bTranscriptionBoundaries = right.m_bTranscriptionBoundaries;
	m_bUpdateBoundaries = right.m_bUpdateBoundaries;
	m_bDrawStyleLine = right.m_bDrawStyleLine;
	// start with first sample data
	m_dwDataPosition = 0;
	// no magnify
	m_fMagnify = 1.0;
	// no zoom
	m_fZoom = 1.0;
	m_fMaxZoom = 0;
	m_bPrintPreviewInProgress = FALSE;

	m_bStaticTWC = right.m_bStaticTWC;
	m_bNormalMelogram = right.m_bNormalMelogram;

	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		m_abAnnAll[nLoop] = right.m_abAnnAll[nLoop];
		m_abAnnNone[nLoop] = right.m_abAnnNone[nLoop];

	}
	m_eInitialShowCmd = right.m_eInitialShowCmd;
}

/***************************************************************************/
// CSaView::PreCreateWindow Creation
// Called from the framework before the creation of the window. Changes the
// window style as desired.
/***************************************************************************/
BOOL CSaView::PreCreateWindow(CREATESTRUCT & cs) {
	if (!CView::PreCreateWindow(cs)) {
		return FALSE;
	}
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
WINDOWPLACEMENT CSaView::DeleteGraphs(int nPosition, BOOL bClearID) {
	WINDOWPLACEMENT WP;
	WP.length = sizeof(WINDOWPLACEMENT);
	// use default size if not changed
	WP.showCmd = SW_HIDE;

	if (nPosition == -1) {
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
			if (m_apGraphs[nLoop] != NULL) {
				if (DestroyGraph(&(m_apGraphs[nLoop]), FALSE)) {
					if (bClearID) {
						m_anGraphID[nLoop] = 0;
					}
				}
			}
		}
		// no graph focused
		ResetFocusedGraph();
	} else {
		// delete only given graph object
		if (m_apGraphs[nPosition] != NULL) {
			// get window placement of the graph
			m_apGraphs[nPosition]->GetWindowPlacement(&WP);
			DestroyGraph(&(m_apGraphs[nPosition]), TRUE);
			if (bClearID) {
				// reset ID
				m_anGraphID[nPosition] = 0;
			}
		}
	}

	// return window coordinates
	return WP;
}

/***************************************************************************/
// CSaView::GetNumberOfGraphs Get number of graphs out of the layout number
//
// 09/20/2000 - DDO Rewrote this so the number of graphs returned is based
//                  on the array of graph pointers and not the layout.
//                  That's because, for the purposes of the layout,
//                  several music graphs can be considered one graph.
/***************************************************************************/
int CSaView::GetNumberOfGraphs(UINT * pGraphIDs) {
	int nNumOfGraphs = 0;
	for (int i = 0; i < MAX_GRAPHS_NUMBER; i++)
		if (pGraphIDs[i]) {
			nNumOfGraphs++;
		}

	return nNumOfGraphs;
}

/***************************************************************************/
// 09/20/2000 - DDO This function returns the number of graphs in a given
//                  layout.
/***************************************************************************/
int CSaView::GetNumberOfGraphsInLayout(UINT nLayoutID) {
	switch (nLayoutID) {
	case ID_LAYOUT_1:
		return 1;
	case ID_LAYOUT_2A:
	case ID_LAYOUT_2B:
	case ID_LAYOUT_2C:
		return 2;
	case ID_LAYOUT_3A:
	case ID_LAYOUT_3B:
	case ID_LAYOUT_3C:
		return 3;
	case ID_LAYOUT_4A:
	case ID_LAYOUT_4B:
	case ID_LAYOUT_4C:
		return 4;
	case ID_LAYOUT_5:
		return 5;
	case ID_LAYOUT_6A:
	case ID_LAYOUT_6B:
		return 6;
	default:
		break;
	}

	return -1;
}

/***************************************************************************/
// CSaView::SetLayout Set layout number from the number of graph ID entries
//   09/20/2000 - DDO Tweaked code mainly so the recording graph isn't
//                    considered as part of the layout.
/***************************************************************************/
UINT CSaView::SetLayout(UINT * pGraphIDs) {
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
	if (IDDSelected(pGraphIDs, IDD_TWC)) {
		nNumOfGraphs--;
	}
	if (IDDSelected(pGraphIDs, IDD_MAGNITUDE)) {
		nNumOfGraphs--;
	}
	if (IDDSelected(pGraphIDs, IDD_RECORDING)) {
		nNumOfGraphs--;
	}

	return CLayoutListBox::GetPreferredLayout(nNumOfGraphs);
}

UINT CSaView::GetGraphResourceID(UINT nID) {
	switch (nID) {
	case IDD_RAWDATA: return IDS_GRAPH_TITLE_RAWDATA;
	case IDD_LOUDNESS: return IDS_GRAPH_TITLE_LOUDNESS;
	case IDD_DURATION: return IDS_GRAPH_TITLE_DURATION;
	case IDD_CHANGE: return IDS_GRAPH_TITLE_CHANGE;
	case IDD_PITCH: return IDS_GRAPH_TITLE_PITCH;
	case IDD_CHPITCH: return IDS_GRAPH_TITLE_CHPITCH;
	case IDD_SMPITCH: return IDS_GRAPH_TITLE_SMPITCH;
	case IDD_GRAPITCH: return IDS_GRAPH_TITLE_GRAPITCH;
	case IDD_SPECTROGRAM: return IDS_GRAPH_TITLE_SPECTROGRAM;
	case IDD_SNAPSHOT: return IDS_GRAPH_TITLE_SNAPSHOT;
	case IDD_SPECTRUM: return IDS_GRAPH_TITLE_SPECTRUM;
	case IDD_ZCROSS: return IDS_GRAPH_TITLE_ZCROSS;
	case IDD_F1F2: return IDS_GRAPH_TITLE_F1F2;
	case IDD_F2F1: return IDS_GRAPH_TITLE_F2F1;
	case IDD_F2F1F1: return IDS_GRAPH_TITLE_F2F1F1;
	case IDD_3D: return IDS_GRAPH_TITLE_3D;
	case IDD_MELOGRAM: return IDS_GRAPH_TITLE_MELOGRAM;
	case IDD_TWC: return IDS_GRAPH_TITLE_TWC;
	case IDD_MAGNITUDE: return IDS_GRAPH_TITLE_MAGNITUDE;
	case IDD_STAFF: return IDS_GRAPH_TITLE_STAFF;
	case IDD_RECORDING: return IDS_GRAPH_TITLE_RECORDING;
	case IDD_BLANK: return IDS_UNDEFINED;
	}
	return IDS_UNDEFINED;
}

/***************************************************************************/
// CSaView::GetGraphTitle
//
// 09/23/2000 - DDO
/***************************************************************************/
CSaString CSaView::GetGraphTitle(UINT nID) {

	CSaString szTitle;
	szTitle.LoadString(GetGraphResourceID(nID));
	return szTitle;
}

/***************************************************************************/
// 09/28/2000 - DDO This function will perform misc. stuff before an
//                  overlay graph is created. I wrote this to get it out
//                  of the CreateGraphs functions which was too bloated.
/***************************************************************************/
UINT CSaView::PreCreateOverlayGraph(int nIndex) {
	m_anGraphID[nIndex] = ID_GRAPHS_OVERLAY;       // set graph ID of new graph to create

	int pos = 0;
	CGraphWnd * pGrph = m_pPickOverlay->GetNextSelectedGraph(pos);
	if (!pGrph) {
		m_anGraphID[nIndex] = IDD_BLANK;
	}

	while (pGrph) {
		m_apGraphs[nIndex]->MergeInGraph(pGrph, (CSaView *)pGrph->GetParent(), false);
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
void CSaView::CreateOneGraph(UINT * pID, CGraphWnd ** pGraph) {
	CRect rWnd(0, 0, 0, 0);
	CSaString szTitle = GetGraphTitle(*pID);

	if (!(*pGraph)->Create(NULL, szTitle, WS_CHILD | WS_VISIBLE | WS_BORDER, rWnd, this, 0)) {
		delete *pGraph;
		*pID = 0;
		*pGraph = NULL;
	}

	if (!m_pFocusedGraph) {
		// set focus to first graph
		SetFocusedGraph(*pGraph);
	}
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
	CObjectIStream * pObs  /* = NULL */,
	CGraphWnd * pFromGraph /* = NULL */) {
	ASSERT(nPosition != -1);

	int nLoop = nPosition;
	m_anGraphID[nLoop] = nNewID;

	if (nNewID == IDD_RECORDING) {
		m_apGraphs[nLoop] = CreateRecGraph(reinterpret_cast<CRecGraphWnd *>(pFromGraph), pObs);
		return;
	}

	if (nNewID == ID_GRAPHS_OVERLAY) {
		nNewID = PreCreateOverlayGraph(nPosition);
	}

	CreateOneGraphStepOne(nNewID, &m_apGraphs[nLoop], how, pFromGraph, pObs);

	if (nNewID == ID_GRAPHS_OVERLAY) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->SetWindowText(_T("Overlay"));
			m_apGraphs[nLoop]->ResizeGraph(TRUE, TRUE);
		}
		m_nFocusedID = m_anGraphID[nLoop];
	}
}

/***************************************************************************/
// 09/21/2000 - DDO
/***************************************************************************/
CRecGraphWnd * CSaView::CreateRecGraph(CRecGraphWnd * pFromGraph, CObjectIStream * pObs) {

	CRect rWnd(0, 0, 0, 0);

	CRecGraphWnd * pRecGraph = new CRecGraphWnd();
	pRecGraph->SetCaptionStyle(NoneThin);

	if (pFromGraph) {
		pRecGraph->PartialCopy(*pFromGraph);
	} else if (pObs) {
		pRecGraph->ReadProperties(*pObs);
	} else {
		pRecGraph->ShowXScale(TRUE, FALSE);
		pRecGraph->ShowAnnotation(PHONETIC, FALSE, FALSE);
		pRecGraph->ShowLegend(m_bLegendAll);
		pRecGraph->ShowGrid(TRUE);
		pRecGraph->SetMagnify(m_fMagnify);
	}

	if (!pRecGraph->Create(NULL, _T(""), WS_CHILD | WS_VISIBLE, rWnd, this, 0)) {
		delete pRecGraph;
		pRecGraph = NULL;
	} else {
		pRecGraph->SetLineDraw(m_bDrawStyleLine);
		if (m_pFocusedGraph == NULL) {
			SetFocusedGraph(pRecGraph);
		}
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
void CSaView::ChangeGraph(int nID) {
	int i = m_nFocusedID ? GetGraphIndexForIDD(m_nFocusedID) : -1;
	ChangeGraph(i, nID);
}

/***************************************************************************/
// CSaView::ChangeGraph Change the focused graph type
// The graph type of the focused graph will change to the one indicated by
// the parameter (ID). If this type of graph already exists, the focused
// graph will not change.
/***************************************************************************/
void CSaView::ChangeGraph(int idx, int nID) {
	if (GetGraphIndexForIDD(nID) >= 0 && nID != ID_GRAPHS_OVERLAY) {
		if (nID == IDD_RECORDING) {
			int nRecording = GetGraphIndexForIDD(IDD_RECORDING);
			DeleteGraphs(nRecording);
			OnGraphsTypesPostProcess(&m_anGraphID[0], m_nLayout);
		}
		return;
	}

	if (nID == IDD_RECORDING) {
		UINT anTmpGraphID[MAX_GRAPHS_NUMBER];
		anTmpGraphID[0] = IDD_RECORDING;
		for (int i = 0; i < MAX_GRAPHS_NUMBER - 1; i++) {
			anTmpGraphID[i + 1] = m_anGraphID[i];
		}
		OnGraphsTypesPostProcess(&anTmpGraphID[0], m_nLayout);
	} else {
		int i = m_nFocusedID ? GetGraphIndexForIDD(m_nFocusedID) : -1;
		if (idx >= 0) {
			WINDOWPLACEMENT WP;
			WP.showCmd = SW_HIDE;
			if (nID != ID_GRAPHS_OVERLAY) {
				WP = DeleteGraphs(idx);
			}
			CreateGraph(idx, nID);
			if (m_apGraphs[idx]) {
				// if the graph we are changing is the focused graph, then reset it
				if (idx == i) {
					SetFocusedGraph(m_apGraphs[idx]);
				}
				if (WP.showCmd != SW_HIDE) {
					m_apGraphs[idx]->SetWindowPlacement(&WP);
				}
			}
		}
	}
}

/***************************************************************************/
/***************************************************************************/
BOOL CSaView::AssignOverlay(CGraphWnd * pTarget, CSaView * pSourceView) {
	int nTargetID = pTarget->GetPlotID();
	if (nTargetID == ID_GRAPHS_OVERLAY) {
		ASSERT(pTarget);
		ASSERT(pTarget->GetPlot());
		CMultiPlotWnd * pPlot = (CMultiPlotWnd *)pTarget->GetPlot();
		nTargetID = pPlot->GetBasePlotID();
		ASSERT(nTargetID >= 0);
	}

	TRACE(_T("AssignOverlay Focused ID = %d\n"), nTargetID);

	// get the matching graph from the source
	int nSourceIndex = pSourceView->GetGraphIndexForIDD(nTargetID);
	ASSERT(nSourceIndex >= 0);
	if (nSourceIndex < 0) {
		return FALSE;
	}
	CGraphWnd * pGraph = pSourceView->GetGraph(nSourceIndex);
	ASSERT(pGraph);
	if (pGraph == NULL) {
		return FALSE;
	}

	// here's the problem.  with this feature, there may end up
	// being more than one overlay in the view.  We need to find
	// the right one.
	int nPosition = -1;
	for (int j = 0; j < MAX_GRAPHS_NUMBER; j++) {
		if (m_apGraphs[j] == pTarget) {
			nPosition = j;
			break;
		}
	}

	ASSERT(nPosition >= 0);
	if (nPosition < 0) {
		return FALSE;
	}

	m_anGraphID[nPosition] = ID_GRAPHS_OVERLAY;

	m_apGraphs[nPosition]->MergeInGraph(pGraph, (CSaView *)pSourceView, true);

	if (m_apGraphs[nPosition] != NULL) {
		m_apGraphs[nPosition]->SetWindowText(_T("Overlay"));
		m_apGraphs[nPosition]->ResizeGraph(TRUE, TRUE);
	}

	m_nFocusedID = m_anGraphID[nPosition];
	if (m_apGraphs[nPosition] != NULL) {
		SetFocusedGraph(m_apGraphs[nPosition]);
	}

	return TRUE;
}

/***************************************************************************/
// CSaView::ChangeLayout Change graph layout
//
// 09/20/2000 - DDO Reworked to accomodate multiple music graphs in a single
//                  layout section.
/***************************************************************************/
void CSaView::ChangeLayout(UINT nNewLayout) {

	if (m_nLayout == nNewLayout) {
		return;
	}

	//**************************************************************
	// Get the number of existing graphs and the minimum number
	// needed for the new layout.
	//**************************************************************
	int nOldNumOfGraphs = GetNumberOfGraphs(&m_anGraphID[0]);
	int nMinNumGraphsInNewLayout = GetNumberOfGraphsInLayout(nNewLayout);
	if (IDDSelected(&m_anGraphID[0], IDD_TWC)) {
		nOldNumOfGraphs--;
	}
	if (IDDSelected(&m_anGraphID[0], IDD_MAGNITUDE)) {
		nOldNumOfGraphs--;
	}
	if (IDDSelected(&m_anGraphID[0], IDD_RECORDING)) {
		nOldNumOfGraphs--;
	}
	m_nLayout = nNewLayout;

	int nGraphDiff = nOldNumOfGraphs - nMinNumGraphsInNewLayout;

	//**************************************************************
	// If there are more graphs than will fit into the new layout
	// then delete graphs that won't fit into the new layout.
	//**************************************************************
	if (nGraphDiff > 0) {
		for (int i = MAX_GRAPHS_NUMBER - 1; nGraphDiff; i--) {
			if (m_apGraphs[i] != NULL) {
				//********************************************************
				// If the graph we're on is any of the melogram-related
				// graphs then treat them as one graph and delete all of
				// them. Otherwise just delete the single graph.
				//********************************************************
				if (m_anGraphID[i] == IDD_TWC || m_anGraphID[i] == IDD_MELOGRAM || m_anGraphID[i] == IDD_MAGNITUDE) {
					int nidx = GetGraphIndexForIDD(IDD_TWC);
					if (nidx >= 0) {
						DeleteGraphs(nidx);
					}
					nidx = GetGraphIndexForIDD(IDD_MELOGRAM);
					if (nidx >= 0) {
						DeleteGraphs(nidx);
					}
					nidx = GetGraphIndexForIDD(IDD_MAGNITUDE);
					if (nidx >= 0) {
						DeleteGraphs(nidx);
					}
				} else if (m_anGraphID[i] == IDD_RECORDING) {
					// Position View does not count as a graph
					continue;
				} else {
					DeleteGraphs(i);
				}
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
		for (int i = GetNumberOfGraphs(&m_anGraphID[0]); nGraphDiff; i++, nGraphDiff++) {
			CreateGraph(i, IDD_BLANK);
		}

	if (!GetFocusedGraphWnd()) {
		SetFocusedGraph(m_apGraphs[0]);
	}
}

/***************************************************************************/
// CSaView::RefreshGraphs Refresh (redraw) the entire or partial graph window
//**************************************************************************/
void CSaView::ResizeGraphs(BOOL bEntire, BOOL bLegend) {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->ResizeGraph(bEntire, bLegend);
		}
	}
}

/***************************************************************************/
// CSaView::RefreshGraphs Refresh (redraw) the entire or partial graph window
//**************************************************************************/
void CSaView::RedrawGraphs(BOOL bEntire, BOOL bLegend) {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->RedrawGraph(bEntire, bLegend);
		}
	}
}

/***************************************************************************
* CSaView::ChangeAnnotationSelection 
* Change the annotation selection.
* pSegment - points to the segment whose selection changes, all other segments have to be deselected.
* The start and stop cursors have to be moved in place of the selected segment (if not deselecting) 
* and the annotation windows have to be informed (via graph). 
* nSelection - contains the new segment number to select and pSegment points to the annotation
* segment, whose segment will be selected. 
* If a segment becomes selected, all the highlighted areas in the plots will be deleted.
**************************************************************************/
void CSaView::ChangeAnnotationSelection(CSegment * pSegment, int nSelection, DWORD dwStart, DWORD dwStop) {

	if (pSegment->IsEmpty()) {
		nSelection = -1;
	}
	// set start and stop cursor if not changing selection
	bool bSelect = false;
	// is it changing?
	if (nSelection != pSegment->GetSelection()) {
		// we are selecting something else
		if (nSelection != -1) {
			// added conditional to prevent cursors from being set if just deselecting  AKE 8/3/2001
			if (pSegment->GetOffset(nSelection) > GetStopCursorPosition()) {
				// new segment is right of current cursor location
				SetStopCursorPosition(dwStop, SNAP_LEFT);
				SetStartCursorPosition(dwStart, SNAP_RIGHT);
			} else {
				// new segment is left of current cursor location
				SetStartCursorPosition(dwStart, SNAP_RIGHT);
				SetStopCursorPosition(dwStop, SNAP_LEFT);
			}
		}
		bSelect = true;
	}

	// select this segment, deselect all the others
	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		CSegment * pCurrent = GetAnnotation(nLoop);
		// is this 'our' segment?
		if (pSegment == pCurrent) {
			// this is the segment to select or deselect
			pSegment->SetSelection(nSelection);
			// change it in all graphs
			for (int nGraphLoop = 0; nGraphLoop < MAX_GRAPHS_NUMBER; nGraphLoop++) {
				if (m_apGraphs[nGraphLoop]) {
					m_apGraphs[nGraphLoop]->ChangeAnnotationSelection(nLoop);
					if (bSelect) {
						m_apGraphs[nGraphLoop]->GetPlot()->SetHighLightArea(0, 0, TRUE, FALSE);
					}
				}
			}
		} else {
			if (pCurrent->GetSelection() != -1) {
				// this segment is to deselect
				pCurrent->SetSelection(-1);
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

/**
* Change the annotation selection
*/
void CSaView::ChangeAnnotationSelection(CSegment * pSegment, int nSelection) {

	DWORD dwStart = GetStartCursorPosition();
	DWORD dwStop = GetStopCursorPosition();
	if (pSegment != NULL) {
		if (nSelection >= 0) {
			dwStart = pSegment->GetOffset(nSelection);
			dwStop = pSegment->GetDuration(nSelection) + dwStart;
		}
	}
	ChangeAnnotationSelection(pSegment, nSelection, dwStart, dwStop);
}

/***************************************************************************/
// CSaView::SetFocusedGraph Set the graph with focus
// Give graph, pWnd focus and change its caption to the graphs title.
/***************************************************************************/
void CSaView::SetFocusedGraph(CGraphWnd * pWnd) {

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
	if (m_pFocusedGraph) {
		m_pFocusedGraph->SetGraphFocus(FALSE);
	}

	//******************************************************
	// Set pointer to new graph with focus.
	//******************************************************
	m_pFocusedGraph = pWnd;
	if (ViewIsActive()) {
		m_pFocusedGraph->SetGraphFocus(TRUE);
	}

	//******************************************************
	// If new graph with focus is not the recording graph
	// then change its caption.
	//******************************************************
	{
		CSaString szGraph;
		CSaString szCaption;
		pWnd->GetWindowText(szGraph.GetBuffer(64), 64);     // load the graph caption
		szGraph.ReleaseBuffer(-1);
		CSaDoc * pModel = GetDocument();
		szCaption = pModel->GetFilenameFromTitle().c_str();   // get current view's caption string

		if ((pModel->IsTempFile()) && (pModel->CanEdit())) {
			CString szCopy;
			szCopy.LoadString(IDS_COPY);
			szCaption.Append(L" : ");
			szCaption += szCopy;
		}
		szCaption.Append(L" : ");
		szCaption.Append(szGraph);                          // add new document title
		pModel->SetTitle(szCaption);                          // write the new caption string
	}

	CSaDoc * pModel = (CSaDoc *)GetDocument();
	// update the status bar
	if (pModel->GetDataSize() > 0) {
		pWnd->UpdateStatusBar(GetStartCursorPosition(), GetStopCursorPosition(), TRUE);
	}
}

/***************************************************************************/
// CSaView::ResetFocusedGraph Reset the graph with focus
// No graph has focus.
/***************************************************************************/
void CSaView::ResetFocusedGraph() {
	// reset focused graph pointer
	m_pFocusedGraph = NULL;
	m_nFocusedID = 0;
	// delete name of graph in caption of view
	CSaString szCaption;
	if (m_pDocument && GetSafeHwnd()) {
		CSaDoc * pModel = GetDocument();  // get pointer to document
		szCaption = pModel->GetFilenameFromTitle().c_str();    // get the current view caption string
		pModel->SetTitle(szCaption);      // write the new caption string
	}
}

/***************************************************************************/
// CSaView::ZoomIn Zoom in
// Tries to set the first displayed data in the manner, that the two cursors
// are in the middle of the data frame. If not possible, the first data
// stays the same. If bZoom is FALSE (default is TRUE), the function won't
// zoom, but only do the rest (center etc).
/***************************************************************************/
void CSaView::ZoomIn(double fZoomAmount, BOOL bZoom) {

	// get pointer to document
	CSaDoc * pModel = GetDocument();
	CRect rWnd;
	GetClientRect(rWnd);
	DWORD wSmpSize = pModel->GetSampleSize();

	// zoom in
	if (bZoom) {
		// add zoom
		m_fZoom += fZoomAmount;
	}
	if (m_fZoom > m_fMaxZoom) {
		m_fZoom = m_fMaxZoom;
	}

	// one line scroll width
	m_dwScrollLine = GetDataFrame() * LINE_SCROLL_PIXELWIDTH / rWnd.Width();
	if (m_dwScrollLine < wSmpSize) {
		m_dwScrollLine = wSmpSize;
	}

	// try to set data between the cursors into the middle of the frame
	DWORD dwDataCenter = GetStartCursorPosition() + (GetStopCursorPosition() - GetStartCursorPosition() + wSmpSize) / 2;

	// limit left border
	if (dwDataCenter > GetDataFrame() / 2) {
		// set new data position
		m_dwDataPosition = dwDataCenter - GetDataFrame() / 2;
		// for 16 bit data value must be even
		if (wSmpSize == 2) {
			m_dwDataPosition &= ~1;
		}
	} else {
		m_dwDataPosition = 0;
	}

	// limit right border
	// is data position too high?
	if (m_dwDataPosition > (pModel->GetDataSize() - GetDataFrame())) {
		// reduce data position to maximum
		m_dwDataPosition = pModel->GetDataSize() - GetDataFrame();
	}

	// set scroll bar
	SetScrollRange(SB_HORZ, 0, (int)((pModel->GetDataSize() - GetDataFrame()) / m_dwHScrollFactor), FALSE);
	SetScrollPos(SB_HORZ, (int)(m_dwDataPosition / m_dwHScrollFactor), TRUE);

	// if scrolling zoom enabled, set the new position
	if (GetMainFrame().IsScrollZoom()) {
		SetScrollPos(SB_VERT, (int)(m_fVScrollSteps + ZOOM_SCROLL_RESOLUTION - m_fVScrollSteps / m_fZoom), TRUE);
	}

	// repaint all graphs (not area and private cursor graphs)
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if ((m_apGraphs[nLoop] != NULL) &&
			(!m_apGraphs[nLoop]->IsAreaGraph()) &&
			(!m_apGraphs[nLoop]->HasPrivateCursor())) {
			// repaint whole graph without legend window
			m_apGraphs[nLoop]->RedrawGraph();
		}
	}
	if (GraphIDtoPtr(IDD_RECORDING)) {
		GraphIDtoPtr(IDD_RECORDING)->GetPlot()->RedrawWindow(NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW);
	}

	GetMainFrame().SetPlayerTimes();
}

/***************************************************************************/
// CSaView::ZoomOut Zoom out
/***************************************************************************/
void CSaView::ZoomOut(double fZoomAmount) {

	// get pointer to document
	CSaDoc * pModel = GetDocument();
	DWORD wSmpSize = pModel->GetSampleSize();

	if (m_fZoom > 1.0) {
		// subtract zoom
		m_fZoom -= fZoomAmount;
		if (m_fZoom <= 1.0) {
			m_fZoom = 1.0;
			// hide scroll bar
			SetScrollRange(SB_HORZ, 0, 0, FALSE);
			m_dwDataPosition = 0;
		} else {
			// repaint all graphs
			CRect rWnd;
			GetClientRect(rWnd);

			// try to set data between the cursors into the middle of the frame
			DWORD dwDataCenter = GetStartCursorPosition() + (GetStopCursorPosition() - GetStartCursorPosition() + wSmpSize) / 2;

			// limit left border
			if (dwDataCenter > GetDataFrame() / 2) {
				// set new data position
				m_dwDataPosition = dwDataCenter - GetDataFrame() / 2;
				// for 16 bit data value must be even
				if (pModel->Is16Bit()) {
					m_dwDataPosition &= ~1;
				}
			} else {
				m_dwDataPosition = 0;
			}
			// limit right border
			// is data position too high?
			if (m_dwDataPosition > (pModel->GetDataSize() - GetDataFrame())) {
				// reduce data position to maximum
				m_dwDataPosition = pModel->GetDataSize() - GetDataFrame();
			}
			// set scroll bar
			// one line scroll width
			m_dwScrollLine = GetDataFrame() * LINE_SCROLL_PIXELWIDTH / rWnd.Width();
			if (m_dwScrollLine < wSmpSize) {
				m_dwScrollLine = wSmpSize;
			}

			SetScrollRange(SB_HORZ, 0, (int)((pModel->GetDataSize() - GetDataFrame()) / m_dwHScrollFactor), FALSE);
			SetScrollPos(SB_HORZ, (int)(m_dwDataPosition / m_dwHScrollFactor), TRUE);
		}

		// repaint all graphs
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
			if ((m_apGraphs[nLoop] != NULL) &&
				(!m_apGraphs[nLoop]->IsAreaGraph()) &&
				(!m_apGraphs[nLoop]->HasPrivateCursor())) {
				// repaint whole graph without legend window
				m_apGraphs[nLoop]->RedrawGraph();
			}
		}
		if (GraphIDtoPtr(IDD_RECORDING)) {
			GraphIDtoPtr(IDD_RECORDING)->GetPlot()->RedrawWindow(NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW);
		}

		// if scrolling zoom enabled, set the new position
		if (GetMainFrame().IsScrollZoom()) {
			SetScrollPos(SB_VERT, (int)(m_fVScrollSteps + ZOOM_SCROLL_RESOLUTION - m_fVScrollSteps / m_fZoom), TRUE);
		}

		GetMainFrame().SetPlayerTimes();
	}
}

/***************************************************************************/
// CSaView::SetScrolling Set new scrolling parameters
/***************************************************************************/
void CSaView::SetScrolling() {
	CSaDoc * pModel = GetDocument();                                                  // get pointer to document
	DWORD wSmpSize = pModel->GetSampleSize();
	CRect rWnd;
	GetClientRect(rWnd);
	if (rWnd.Width()) {
		m_dwScrollLine = GetDataFrame() * LINE_SCROLL_PIXELWIDTH / rWnd.Width();    // one line scroll width
		if (m_dwScrollLine < wSmpSize) {
			m_dwScrollLine = wSmpSize;
		}
		m_fMaxZoom = (double)(pModel->GetNumSamples()) / (double)rWnd.Width() * 8.;  // max zoom factor
		m_fVScrollSteps = ZOOM_SCROLL_RESOLUTION * m_fMaxZoom;
		if (m_fVScrollSteps > 0x7FFFFFFF) {
			m_fVScrollSteps = 0x7FFFFFFF;
		}
		m_dwHScrollFactor = (GetDocument()->GetDataSize() / 0x7FFF) + 1;
	}
}

/***************************************************************************/
// CSaView::SetInitialCursors Set initial cursor positions
/***************************************************************************/
void CSaView::SetInitialCursors() {
	CSaDoc * pModel = (CSaDoc *)GetDocument(); // get pointer to document
	DWORD dwOffset = 0;
	if (pModel->GetDataSize() > 0) {
		CRect rWnd;
		GetClientRect(rWnd);
		if (rWnd.Width() && GetDataFrame()) {
			double fBytesPerPix = (double)GetDataFrame() / (double)rWnd.Width(); // calculate data samples per pixel
			if ((DWORD)rWnd.Width() > GetDataFrame()) {
				fBytesPerPix = (double)pModel->GetBlockAlign();
			}
			dwOffset = (DWORD)(CURSOR_WINDOW_HALFWIDTH / 2 * fBytesPerPix);
		}
		//DWORD dwStop = (pModel->GetDataSize() == 0)? 0 : pModel->GetDataSize() - 1 - dwOffset;
		DWORD dwStop = dwOffset * 2;
		SetStartStopCursorPosition(dwOffset, dwStop, SNAP_BOTH);  // start cursor position is first sample plus offset
	}
}

// 1.06.5.2 // Removed changes from 1.06.5
/***************************************************************************/
// CSaView::AdjustCursors Adjust cursor positions to new file size
// The function adjusts the cursor positions to the new file size. The file
// size changed at the position dwSectionStart by dwSectionLength bytes and
// it shrunk, if the flag bShrink is TRUE, otherwise it grew.
/***************************************************************************/
void CSaView::AdjustCursors(DWORD dwSectionStart, DWORD dwSectionLength, BOOL bShrink) {
	DWORD wSmpSize = GetDocument()->GetSampleSize();
	if (bShrink) {
		if (GetStartCursorPosition() >= dwSectionStart) {
			if (GetStartCursorPosition() >= (dwSectionStart + dwSectionLength)) {
				m_dwStartCursor = GetStartCursorPosition() - dwSectionLength;
			} else {
				m_dwStartCursor = dwSectionStart;
			}
		}
		if (GetStopCursorPosition() >= dwSectionStart) {
			if (GetStopCursorPosition() >= (dwSectionStart + dwSectionLength)) {
				m_dwStopCursor = GetStopCursorPosition() - dwSectionLength;
			} else {
				m_dwStopCursor = dwSectionStart + dwSectionLength - wSmpSize;
			}
		}
	} else {
		if (GetStartCursorPosition() >= dwSectionStart) {
			m_dwStartCursor = GetStartCursorPosition() + dwSectionLength;
		}
		if (GetStopCursorPosition() >= dwSectionStart) {
			m_dwStopCursor = GetStopCursorPosition() + dwSectionLength;
		}
	}
	// for 16 bit data cursor positons must be even
	if (wSmpSize == 2) {
		m_dwStartCursor &= ~1;
		m_dwStopCursor &= ~1;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSaView drawing

/***************************************************************************/
// CSaView::OnDraw Drawing
/***************************************************************************/
void CSaView::OnDraw(CDC * /*pDC*/) {
	CSaDoc * pModel = GetDocument();
	ASSERT_VALID(pModel);
}

// SDM 1.06.6U5 split function
/***************************************************************************/
// CSaView::OnInitialUpdate initialization
/***************************************************************************/
void CSaView::OnInitialUpdate() {
	InitialUpdate();
}

// SDM 1.06.6U5 split function
/***************************************************************************/
// CSaView::InitialUpdate initialization
/***************************************************************************/
void CSaView::InitialUpdate(BOOL bTemp) {

	CSaApp * pApp = (CSaApp*)AfxGetApp();

	if (s_pobsAutoload) {
		ReadProperties(*s_pobsAutoload, TRUE);
	} else if ((pApp->IsCreatingNewFile()) || (!GetMainFrame().DefaultIsValid())) {
		CSaDoc * pModel = GetDocument();
		CSaString docPath = pModel->GetPathName();
		CFileStatus status;
		if ((CFile::GetStatus(docPath, status)) &&
			((status.m_attribute & CFile::readOnly) == CFile::readOnly)) {
			CSaString docTitle = pModel->GetTitle() + " (Read-Only)";
			pModel->SetTitle(docTitle);
		}
		// SDM don't open 10-20 blank graphs...
		CreateOpenAsGraphs(pApp->GetOpenAsID());
	} else {
		if (bTemp) {
			// ReadGraphListProperties assumes no graphs
			DeleteGraphs();
		}
		PartialCopy(*(GetMainFrame().pDefaultViewConfig()));
		ReadGraphListProperties(*(GetMainFrame().pDefaultViewConfig()));
	}

	if (!bTemp) {
		CView::OnInitialUpdate();
	}

	// set scrolling
	SetScrolling();
	// hide horizontal scroll bar
	SetScrollRange(SB_HORZ, 0, 0, FALSE);

	//**************************************************************
	// if scrolling zoom enabled, show the scroll bar and set the
	// new position otherwise hide the vertical scroll bar.
	//**************************************************************
	if ((GetMainFrame().IsScrollZoom()) && (GetDocument()->GetDataSize() != 0)) {
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
BOOL CSaView::OnPreparePrinting(CPrintInfo * pInfo) {
	BOOL ok = TRUE;
	if (m_pPageLayout->bIsHiRes()) {
		int pages = CalculateHiResPrintPages();

		if (pages == -1) {
			CDlgPrintOptions * pPgLayoutBckup = new CDlgPrintOptions(m_pPageLayout);

			// error - bring up the dialog to fix
			// inconsistantcy between number of graphs
			// and number of graphs per page
			m_pPageLayout->SetGraphsPtr(m_apGraphs);
			ok = m_pPageLayout->DoModal() == IDOK;
			if (ok) {
				pages = CalculateHiResPrintPages();
			} else if (pPgLayoutBckup) {
				*m_pPageLayout = *pPgLayoutBckup;
			}

			if (pPgLayoutBckup) {
				delete pPgLayoutBckup;
			}
		}
		if (ok) {
			pInfo->SetMaxPage(pages);
		}
	} else {
		pInfo->SetMaxPage(1);
		PreparePrintingForScreenShot();
	}

	if (ok) {
		ok = SaDoPreparePrinting(pInfo, m_pPageLayout->bIsLandscape());
	}

	return ok;
}

/***************************************************************************/
// CSaView::OnBeginPrinting Begin printing
/***************************************************************************/
void CSaView::OnBeginPrinting(CDC * pDC, CPrintInfo * /*pInfo*/) {
	BOOL isColor = (pDC->GetDeviceCaps(NUMCOLORS) > 2);

	if (m_pPageLayout->bIsHiRes()) {
		if (!isColor) {
			m_saveColors = *GetMainFrame().GetColors();
			GetMainFrame().GetColors()->SetupColors(Colors::PRINTING);
		}
	} else if ((m_bPrintPreviewInProgress) && (!isColor) && (m_pCDibForPrint)) {
		m_pCDibForPrint->GoGreyScale();
	}
}

/***************************************************************************/
// CSaView::OnEndPrinting End printing
/***************************************************************************/
void CSaView::OnEndPrinting(CDC * pDC, CPrintInfo * /*pInfo*/) {
	BOOL isColor = (pDC->GetDeviceCaps(NUMCOLORS) > 2);

	if (m_pPageLayout->bIsHiRes()) {
		if (!isColor) {
			GetMainFrame().SetColors(m_saveColors);
		}
	} else if (m_pCDibForPrint) {
		delete m_pCDibForPrint;
		m_pCDibForPrint = NULL;
	}
	m_bPrintPreviewInProgress = FALSE;
	GetMainFrame().ClearPreviewFlag();
	GetMainFrame().ClearPrintingFlag();
	if (m_restartPageOptions) {
		PostMessage(WM_COMMAND, ID_FILE_PAGE_SETUP);
	}
}

/***************************************************************************/
// CSaView::OnUpdatePrint Menu Update
/***************************************************************************/
void CSaView::OnUpdateFilePrint(CCmdUI * pCmdUI) {
	pCmdUI->Enable(TRUE);
}

/***************************************************************************/
// CSaView::OnUpdatePrintPreview Menu Update
/***************************************************************************/
void CSaView::OnUpdatePrintPreview(CCmdUI * pCmdUI) {
	pCmdUI->Enable(TRUE);
}

/***************************************************************************/
// CSaView::OnPrintPageSetup - Setup page layout preferences for printing
/***************************************************************************/
void CSaView::OnPrintPageSetup(void) {
	if (m_restartPageOptions) {
		m_restartPageOptions = FALSE;
	} else {
		m_pPgLayoutBackup = new CDlgPrintOptions(m_pPageLayout);
	}

	m_pPageLayout->SetGraphsPtr(m_apGraphs);

	if ((m_pPageLayout->DoModal() != IDOK) && (m_pPgLayoutBackup)) {
		*m_pPageLayout = *m_pPgLayoutBackup;
	}

	if (m_pPageLayout->UserClickedPrint()) {
		PostMessage(WM_COMMAND, ID_FILE_PRINT);
	} else if (m_pPageLayout->UserClickedPreview()) {
		m_restartPageOptions = TRUE;
		PostMessage(WM_COMMAND, ID_FILE_PRINT_PREVIEW);
	}

	if (!m_restartPageOptions && m_pPgLayoutBackup) {
		delete m_pPgLayoutBackup;
		m_pPgLayoutBackup = NULL;
	}
}


/***************************************************************************/
// CSaView::GetDataFrame Get the number of displayed data samples
/***************************************************************************/
DWORD CSaView::GetDataFrame() {
	// calculate current data frame (width of displayed data)
	DWORD dwFrame = (DWORD)((double)GetDocument()->GetDataSize() / m_fZoom);
	// for 16 bit data value must be even
	DWORD nSampleSize = GetDocument()->GetSampleSize();
	if (nSampleSize == 2) {
		dwFrame &= ~1;
	}
	return dwFrame;
}

/***************************************************************************/
// CSaView::GetDataPosition
// Returns the scroll bar position in bytes
// Since scrolling is really a pixel based function the position is first
// converted to pixels then to bytes using the width provided
/***************************************************************************/
double CSaView::GetDataPosition(int nWndWidth) {

	double fDataPos = m_dwDataPosition;
	// number of data points to display
	DWORD dwDataFrame = GetDataFrame();

	// SDM 1.06.6U4 Calculate position based on pixel aligned graph
	if ((nWndWidth != 0) && (dwDataFrame > 0)) {
		// calculate data samples per pixel
		double fBytesPerPix = (double)dwDataFrame / (double)(nWndWidth);
		fDataPos = round2Int(fDataPos / fBytesPerPix)*fBytesPerPix;
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
DWORD CSaView::CalcDataFrame(int nWndWidth) {

	// number of data points to display
	DWORD dwDataFrame = GetDataFrame();
	if (dwDataFrame == 0) {
		// nothing to draw
		return 0;
	}

	// get pointer to document
	CSaDoc * pModel = (CSaDoc *)GetDocument();
	DWORD dwDataSize = pModel->GetDataSize();
	if (dwDataSize >= GetDataPosition(nWndWidth) && (GetDataPosition(nWndWidth) + dwDataFrame) > dwDataSize) {
		dwDataFrame = DWORD(dwDataSize - GetDataPosition(nWndWidth));
	}
	return dwDataFrame;
}

/***************************************************************************/
// CSaView::SetCursorPosition Set the specified cursor position
/***************************************************************************/
void CSaView::SetCursorPosition(ECursorSelect nCursorSelect,
	DWORD dwNewPos,
	ESnapDirection nSnapDirection,
	ECursorAlignment nCursorAlignment) {
	switch (nCursorSelect) {
	case START_CURSOR:
		SetStartCursorPosition(dwNewPos, nSnapDirection, nCursorAlignment);
		if (GetDynamicGraphCount()) {
			CSaDoc * pModel = (CSaDoc *)GetDocument(); // get pointer to document
			DWORD nSampleSize = pModel->GetSampleSize();
			if (GetAnimationGraphCount()) {
				StartAnimation(m_dwStartCursor / nSampleSize, m_dwStopCursor / nSampleSize);
			}
			EndAnimation();
		}
		break;

	case STOP_CURSOR:
		SetStopCursorPosition(dwNewPos, nSnapDirection, nCursorAlignment);
		break;
	}
}

void CSaView::HideCursors() {
	// move start cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->HideCursors();
		}
	}
}

void CSaView::ShowCursors() {
	// move start cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->ShowCursors();
		}
	}
}

/***************************************************************************/
// CSaView::SetStartCursorPosition Set the start cursor
/***************************************************************************/
void CSaView::SetStartCursorPosition(DWORD dwNewPos, ESnapDirection nSnapDirection, ECursorAlignment nCursorAlignment) {
	SetStartStopCursorPosition(dwNewPos, m_dwStopCursor, nSnapDirection, nCursorAlignment);
}

/***************************************************************************/
// CSaView::SetStopCursorPosition Set the stop cursor
/***************************************************************************/
void CSaView::SetStopCursorPosition(DWORD dwNewPos, ESnapDirection nSnapDirection, ECursorAlignment nCursorAlignment) {
	SetStartStopCursorPosition(m_dwStartCursor, dwNewPos, nSnapDirection, nCursorAlignment);
}

/***************************************************************************/
// CSaView::SetStartStopCursorPosition Set the start cursor
// kg in some case we need to update the member variables before
// we update the graphs in case the previous stop position is invalid
// for the new graph.
/***************************************************************************/
void CSaView::SetStartStopCursorPosition(WAVETIME startTime,
	WAVETIME stopTime,
	ESnapDirection nSnapDirection,
	ECursorAlignment nCursorAlignment) {
	CSaDoc * pModel = (CSaDoc *)GetDocument(); // get pointer to document
	DWORD startPos = pModel->ToBytes(startTime, true);
	DWORD stopPos = pModel->ToBytes(stopTime, true);
	SetStartStopCursorPosition(startPos, stopPos, nSnapDirection, nCursorAlignment);
}

/***************************************************************************/
// CSaView::SetStartStopCursorPosition Set the start cursor
// kg in some case we need to update the member variables before
// we update the graphs in case the previous stop position is invalid
// for the new graph.
/***************************************************************************/
void CSaView::SetStartStopCursorPosition(DWORD dwNewStartPos,
	DWORD dwNewStopPos,
	ESnapDirection nSnapDirection,
	ECursorAlignment nCursorAlignment) {
	if (nCursorAlignment == ALIGN_USER_SETTING) {
		nCursorAlignment = GetCursorAlignment();
	}

	CSaDoc * pModel = (CSaDoc *)GetDocument(); // get pointer to document
	m_dwStartCursor = dwNewStartPos;
	m_dwStopCursor = dwNewStopPos;

	// position limit
	if (pModel->GetDataSize() > 0) {
		if (m_dwStartCursor >= pModel->GetDataSize()) {
			m_dwStartCursor = pModel->GetDataSize() - 1;
		}
		if (m_dwStopCursor >= pModel->GetDataSize()) {
			m_dwStopCursor = pModel->GetDataSize() - 1;
		}
	}

	// for 16 bit data value must be even
	if (pModel->Is16Bit()) {
		m_dwStartCursor &= ~1;
		m_dwStopCursor &= ~1;
	}

	// snap the start cursor if necessary
	DWORD nSampleSize = pModel->GetSampleSize();

	m_dwStartCursor = pModel->SnapCursor(START_CURSOR, m_dwStartCursor, 0, m_dwStopCursor - nSampleSize, nSnapDirection, nCursorAlignment);
	m_dwStopCursor = pModel->SnapCursor(STOP_CURSOR, m_dwStopCursor, m_dwStartCursor + nSampleSize, GetDocument()->GetDataSize() - nSampleSize, nSnapDirection, nCursorAlignment);

	// move start cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->SetStartCursor(this);
		}
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->SetStopCursor(this);
		}
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->SetPlaybackCursor(this);
		}
	}

	GetMainFrame().SetPlayerTimes();

}

// SDM 1.5Test10.5
/***************************************************************************/
// CSaView::SetPlaybackPosition Set the playbackPosition
/***************************************************************************/
void CSaView::SetPlaybackPosition(DWORD dwNewPos, int nSpeed, BOOL bEstimate) {

	if (bEstimate) {
		SetTimer(ID_TIMER_PLAYBACK, 1, NULL);
	} else {
		if (nSpeed != 0) {
			m_dwPlaybackTime = GetTickCount();
			SetTimer(ID_TIMER_PLAYBACK, 1, NULL);
		}
		m_dwPlaybackPosition = dwNewPos;
		m_nPlaybackSpeed = nSpeed;
	}

	// for 16 bit data value must be even
	if (GetDocument()->Is16Bit()) {
		m_dwLastPlaybackPosition = dwNewPos & ~1;
	} else {
		m_dwLastPlaybackPosition = dwNewPos;
	}
	//TRACE(">>m_dwLastPlaybackPosition=%d\n");

	// move start cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->SetPlaybackPosition(this, enableScrolling);
		}
	}
}

// SDM 1.5Test10.5
/***************************************************************************/
// CSaView::SetPlaybackPosition Set the playbackPosition
/***************************************************************************/
void CSaView::StopPlaybackTimer() {
	KillTimer(ID_TIMER_PLAYBACK);
}

void CSaView::SetPlaybackFlash(bool on) {
	// move start cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			m_apGraphs[nLoop]->SetPlaybackFlash(on);
		}
	}
}

// SDM 1.5Test10.5
/***************************************************************************/
// CSaView::OnTimer Set the playbackPosition
/***************************************************************************/
void CSaView::OnTimer(UINT nIDEvent) {

	if (nIDEvent == ID_TIMER_PLAYBACK) {
		if (m_nPlaybackSpeed > 0) {
			DWORD dwNewPos = (DWORD)(m_dwPlaybackPosition + (GetTickCount() - m_dwPlaybackTime) / GetDocument()->GetTimeFromBytes(1000) * m_nPlaybackSpeed / 100);
			SetPlaybackPosition(dwNewPos, m_nPlaybackSpeed, TRUE);
		}
		return;
	}

	// everything else
	CView::OnTimer(nIDEvent);
}

/***************************************************************************/
// CSaView::iGetStartCursorSegment Get segment index at start cursor position
/***************************************************************************/
int CSaView::iGetStartCursorSegment(int iSegment) {
	int idxPosition = 0;

	CSegment * pSegment = GetAnnotation(iSegment);
	ASSERT(pSegment);

	// get pointer to annotation offsets
	if (pSegment->GetOffsetSize() > 0) // there is at least one segment
		// get the segment index at the given position
	{
		idxPosition = pSegment->FindFromPosition(m_dwStartCursor);
	}

	return idxPosition;
}

/***************************************************************************/
// CSaView::iGetStopCursorSegment Get segment index at stop cursor position
/***************************************************************************/
int CSaView::iGetStopCursorSegment(int iSegment) {
	int idxPosition = 0;

	CSegment * pSegment = GetAnnotation(iSegment);
	ASSERT(pSegment);

	// get pointer to annotation offsets
	if (pSegment->GetOffsetSize() > 0) {
		// there is at least one segment
		// get the segment index at the given position
		idxPosition = pSegment->FindFromPosition(m_dwStopCursor);
	}

	return idxPosition;
}

/***************************************************************************/
// CSaView::MoveStartCursor Move the start cursor
/***************************************************************************/
void CSaView::MoveStartCursor(DWORD dwNewPos) {
	// move start cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->MoveStartCursor(this, dwNewPos);
		}
	}
}

/***************************************************************************/
// CSaView::MoveStopCursor Move the stop cursor
/***************************************************************************/
void CSaView::MoveStopCursor(DWORD dwNewPos) {
	// move stop cursors in all the graphs
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] != NULL) {
			m_apGraphs[nLoop]->MoveStopCursor(this, dwNewPos);
		}
	}
}

/***************************************************************************/
// CSaView::SetDataFrame Set number of data samples to display
// The caller delivers a start position (first data sample he wants to have
// displayed) and the number of samples to display. 
// Then this function calculates the nearest possible zooming factor 
// and sets up the real displayable first data and the number of samples to 
// display.
/***************************************************************************/
void CSaView::SetDataFrame(DWORD dwStart, DWORD dwFrame) {

	// get pointer to document
	CSaDoc * pModel = GetDocument();
	DWORD wSmpSize = pModel->GetSampleSize();
	CRect rWnd;
	GetClientRect(rWnd);
	// set start position
	m_dwDataPosition = dwStart;
	// for 16 bit data value must be even
	if (pModel->Is16Bit()) {
		m_dwDataPosition &= ~1;
	}
	// calculate zooming factor
	DWORD dwDataSize = pModel->GetDataSize();
	// zoom factor
	double fZoom = (double)dwDataSize / (double)dwFrame;
	if (fZoom > m_fMaxZoom) {
		fZoom = m_fMaxZoom;
		dwFrame = DWORD(dwDataSize / fZoom);
		if (pModel->Is16Bit()) {
			dwFrame &= ~1;
		}
	}
	// if scrolling zoom enabled, set the new position
	if (GetMainFrame().IsScrollZoom()) {
		SetScrollPos(SB_VERT, (int)(m_fVScrollSteps + ZOOM_SCROLL_RESOLUTION - m_fVScrollSteps / m_fZoom), TRUE);
	}
	if (fZoom > 1.0) {
		// zoom in
		// set new scrolling parameters
		// one line scroll width
		m_dwScrollLine = dwFrame * LINE_SCROLL_PIXELWIDTH / rWnd.Width();
		if (m_dwScrollLine < wSmpSize) {
			m_dwScrollLine = wSmpSize;
		}
		SetScrollRange(SB_HORZ, 0, (int)((dwDataSize - dwFrame) / m_dwHScrollFactor), FALSE);
		if (m_fZoom == 1.0) {
			// first zoom in -> set scroll bar
			SetScrollPos(SB_HORZ, (int)(m_dwDataPosition / m_dwHScrollFactor), TRUE);
		} else {
			// not first zoom in, repaint all graphs
			for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
				if (m_apGraphs[nLoop] != NULL) {
					// repaint whole graph window without legend
					m_apGraphs[nLoop]->RedrawGraph();
				}
			}
		}
		// set new zooming factor
		m_fZoom = fZoom;
	} else {
		// no zooming
		m_dwDataPosition = 0;
		if (m_fZoom > 1.0) {
			// already zoomed in
			// zoom all
			SendMessage(WM_COMMAND, ID_GRAPHS_ZOOM_ALL, 0L);
		}
	}
}

/***************************************************************************/
// CSaView::GetDataFrame returns the parameters set by set
/***************************************************************************/
void CSaView::GetDataFrame(DWORD & dwStart, DWORD & dwFrame) {

	// get pointer to document
	CSaDoc * pModel = GetDocument();
	// set start position
	dwStart = m_dwDataPosition;
	// calculate zooming factor
	DWORD dwDataSize = pModel->GetDataSize();
	// zoom factor
	dwFrame = (DWORD)(((double)dwDataSize) / m_fZoom + 0.5);
}

LRESULT CSaView::OnCursorInFragment(WPARAM nCursorSelect, LPARAM dwFragmentIndex) {
	switch (nCursorSelect) {
	case START_CURSOR:
		CSaDoc * pModel = GetDocument();
		CProcessFragments * pFragments = pModel->GetFragments();
		SFragParms FragmentParms = pFragments->GetFragmentParms(dwFragmentIndex);
		if (GetDynamicGraphCount()) {
			StartAnimation(FragmentParms.dwOffset, FragmentParms.dwOffset);
		}
		break;
	}
	//  BroadcastMessage(WM_USER_CURSOR_IN_FRAGMENT, nCursorSelect, dwFragmentIndex);
	return 0L;
}

void CSaView::NotifyFragmentDone(void * /*pCaller*/) {
	if (GetCursorAlignment() == ALIGN_AT_FRAGMENT) {
		OnCursorAlignmentChanged();    // fragmenter finished, alignment mode just changed
	}
}

UINT CSaView::GetDynamicGraphCount(void) {
	UINT nCount = 0;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			BOOL bDynamicMode = (GetGraphUpdateMode() == DYNAMIC_UPDATE && m_apGraphs[nLoop]->IsAnimationGraph());
			if (bDynamicMode) {
				nCount++;
			}
		} else {
			break;
		}
	}

	return nCount;
}

/***************************************************************************/
// CSaView::OnCursorAlignmentChanged Cursor alignment option has changed
/***************************************************************************/
LRESULT CSaView::OnCursorAlignmentChanged(WPARAM, LPARAM) {
	SetStartCursorPosition(m_dwStartCursor, SNAP_BOTH);
	SetStopCursorPosition(m_dwStopCursor, SNAP_BOTH);

	OnGraphUpdateModeChanged();
	OnAnimationChanged();
	return 0;
}

/***************************************************************************/
// CSaView::OnGraphUpdateModeChanged  Graph update mode has changed
/***************************************************************************/
LRESULT CSaView::OnGraphUpdateModeChanged(WPARAM, LPARAM) {
	int GraphUpdateMode = GetGraphUpdateMode();

	if (GraphUpdateMode == DYNAMIC_UPDATE) {
		CSaDoc * pModel = GetDocument();
		DWORD wSmpSize = pModel->GetSampleSize();
		CProcessFragments * pFragments = pModel->GetFragments();
		DWORD dwFragmentIndex = pFragments->GetFragmentIndex(m_dwStartCursor / wSmpSize);
		OnCursorInFragment(START_CURSOR, dwFragmentIndex);
	} else {
		int nWaveGraphIndex = GetGraphIndexForIDD(IDD_RAWDATA);
		CGraphWnd * pWaveGraph = GetGraph(nWaveGraphIndex);
		if (pWaveGraph != NULL) {
			CPlotWnd * pWavePlot = pWaveGraph->GetPlot();
			// turn off highlighted area in raw waveform
			pWavePlot->ClearHighLightArea();
		}
	}
	return 0;
}

/***************************************************************************/
// CSaView::OnAnimationChanged Animation selection has changed
/***************************************************************************/
LRESULT CSaView::OnAnimationChanged(WPARAM, LPARAM) {
	BOOL AnimateChecked = IsAnimationRequested();
	if (AnimateChecked) {
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
			if (m_apGraphs[nLoop]) {
				BOOL bAnimationGraph = m_apGraphs[nLoop]->IsAnimationGraph();
				if (bAnimationGraph) {
					CPlotWnd * pPlot = m_apGraphs[nLoop]->GetPlot();
					if (pPlot) {
						pPlot->RedrawWindow();
					}
				}
			}
		}
	}
	return 0;
}

/***************************************************************************/
// CSaView::GetAnimationGraphCount  Return number of graphs to animate
/***************************************************************************/
UINT CSaView::GetAnimationGraphCount(void) {
	UINT nCount = 0;
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			BOOL bAnimation = IsAnimationRequested() && m_apGraphs[nLoop]->IsAnimationGraph();
			if (bAnimation) {
				nCount++;
			}
		} else {
			break;
		}
	}
	return nCount;
}


/***************************************************************************/
// CSaView::StartAnimation  Animate from specified start to stop sample
/***************************************************************************/
BOOL CSaView::StartAnimation(DWORD dwStartWaveIndex, DWORD dwStopWaveIndex) {

	CSaDoc * pModel = (CSaDoc *)GetDocument(); // get pointer to document
	CProcessFragments * pFragments = pModel->GetFragments();
	if (!pFragments->IsDataReady()) {
		return FALSE;
	}
	DWORD dwStartFrame = pFragments->GetFragmentIndex(dwStartWaveIndex);
	DWORD dwStopFrame = pFragments->GetFragmentIndex(dwStopWaveIndex);
	BOOL bFreezeFrame = (dwStartFrame == dwStopFrame);
	float fResyncTime = 0.F;
	BOOL bCancelAnimation = FALSE;
	m_bAnimating = TRUE;

	for (DWORD dwFrameIndex = dwStartFrame; dwFrameIndex <= dwStopFrame; dwFrameIndex++) {
		if (m_pStopwatch && !bFreezeFrame) {
			if (dwFrameIndex == dwStartFrame) {
				m_pStopwatch->Reset();    // timing only required to synchronize animations
			} else if (dwFrameIndex == dwStartFrame + 1) {
				float fFrameTime = m_pStopwatch->GetElapsedTime();
				float fFrameTimeRequested = 1.F / (float)GetAnimationFrameRate();
				fResyncTime = fFrameTimeRequested - fFrameTime;
			}
		}
		if (fResyncTime > 0.F) {
			m_pStopwatch->Wait(fResyncTime);    // delay to synchronize with requested frame rate
		}

		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
			// Any keydown might cancel animation
			if (GetAsyncKeyState(VK_ESCAPE) < 0) {
				bCancelAnimation = TRUE;
				break;
			}
			if (m_apGraphs[nLoop] == NULL) {
				break;
			}
			BOOL bDynamicMode = (GetGraphUpdateMode() == DYNAMIC_UPDATE && m_apGraphs[nLoop]->IsAnimationGraph());
			BOOL bAnimation = (IsAnimationRequested() && m_apGraphs[nLoop]->IsAnimationGraph());
			if (bFreezeFrame && bDynamicMode || !bFreezeFrame && bAnimation) {
				m_apGraphs[nLoop]->AnimateFrame(dwFrameIndex);
				if (m_apGraphs[nLoop]->IsCanceled()) {
					bCancelAnimation = TRUE;
					break;
				}
			}
		}
		if (bCancelAnimation) {
			EndAnimation();
			break;
		}
	}
	m_bAnimating = FALSE;
	return TRUE;
}

/***************************************************************************/
// CSaView::EndAnimation  Finish animation
/***************************************************************************/
void CSaView::EndAnimation() {
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop] == NULL) {
			break;
		}
		BOOL bDynamicMode = (GetGraphUpdateMode() == DYNAMIC_UPDATE && m_apGraphs[nLoop]->IsAnimationGraph());
		if (bDynamicMode) {
			m_apGraphs[nLoop]->EndAnimation();
		}
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
// CSaView::OnPrepareDC - prepare printing context for printing.
//                        (set up scaling, page size, and margins)
/***************************************************************************/
void CSaView::OnPrepareDC(CDC * pDC, CPrintInfo * pInfo) {
	if (pDC->IsPrinting()) {
		// get the printers dots per inch
		m_printerDPI = CPoint(pDC->GetDeviceCaps(LOGPIXELSX), pDC->GetDeviceCaps(LOGPIXELSY));

		// calculate a m_printArea that will yield the desired margins, also checking
		// to see if the printable region will allow those margins.
		CalculatePrintArea(pDC, pInfo);

		if (m_pPageLayout->bIsHiRes()) {
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
void CSaView::CalculatePrintOrigin(CDC * pDC) {
	CPoint desiredOffset(fract_multiply(Print_Margin, m_printerDPI.x),
		fract_multiply(Print_Margin, m_printerDPI.y));
	POINT pageOffset;
	pDC->Escape(GETPRINTINGOFFSET, NULL, NULL, &pageOffset);
	m_printOrigin.x = desiredOffset.x - pageOffset.x;
	m_printOrigin.y = desiredOffset.y - pageOffset.y;
	if (m_printOrigin.x < 0) {
		m_printOrigin.x = 0;    // no printing out of the printable area
	}
	if (m_printOrigin.y < 0) {
		m_printOrigin.y = 0;    // no printing out of the printable area
	}
	pDC->DPtoLP(&m_printOrigin, 1);
}

/***************************************************************************/
// CSaView::PrepareDCforHiResPrint - prepare printing context for printing.
//                                  (set up scaling, page size, and margins)
/***************************************************************************/
void CSaView::PrepareDCforHiResPrint(CDC * pDC, CPrintInfo * /*pInfo*/) {
	// set the logical dots per inch based on user preferences.
	// the higher logical dots per inch yields smaller fonts
	m_newPrinterDPI = (m_pPageLayout->bUseSmallFonts()) ? Small_Font_DPI : Standard_DPI;

	//  we stretch the pixels by ViewportExt/WindowExt.
	//  if the ViewportExt = printer dpi, then WindowExt determines the pixels per inch.
	pDC->SetMapMode(MM_ANISOTROPIC);
	pDC->SetWindowExt(m_newPrinterDPI, m_newPrinterDPI);
	pDC->SetViewportExt(m_printerDPI.x, m_printerDPI.y);
	pDC->DPtoLP(&m_printArea, 1);
}



/***************************************************************************/
// CSaView::CalculatePrintArea
//
// Calculate a m_printArea that will yield the desired margins, also checking
// to see if the printable region will allow those margins.
/***************************************************************************/
void CSaView::CalculatePrintArea(CDC * pDC, CPrintInfo * pInfo) {
	pDC->Escape(GETPHYSPAGESIZE, NULL, NULL, &m_printArea);
	m_printArea.x -= m_printerDPI.x; // subtract half inch margins on each side
	m_printArea.y -= m_printerDPI.y; // subtract half inch margins on each side
	CRect printableArea(pInfo->m_rectDraw);
	pDC->LPtoDP(&printableArea);
	if (printableArea.Width()) {
		m_printArea.x = min(m_printArea.x, printableArea.Width());
	}
	if (printableArea.Height()) {
		m_printArea.y = min(m_printArea.y, printableArea.Height());
	}
}




/***************************************************************************/
// CSaView::OnPrint
//
// Print the current page.
/***************************************************************************/
void CSaView::OnPrint(CDC * pDC, CPrintInfo * pInfo) {
	OnPrepareDC(pDC, pInfo); // SDM1.5Test10.8 duplicate call (first call dependencies not initialized)

	if (m_pPageLayout->bIsHiRes()) {
		DoHiResPrint(pDC, pInfo);
	} else {
		DoScreenShot(pDC, pInfo);
	}
}



/***************************************************************************/
// CSaView::DoScreenShot
//
// Print the screen.
/***************************************************************************/
void CSaView::DoScreenShot(CDC * pDC, CPrintInfo * /*pInfo*/) {
	if (!m_pCDibForPrint) {
		// SDM 1.5Test10.8 do not use null pointer
		ASSERT(0);
	} else {
		CRect targRect(m_printOrigin.x, m_printOrigin.y,
			m_printOrigin.x + m_printArea.x,
			m_printOrigin.y + m_printArea.y);

		double targSizeInch1200thsX = targRect.Width() * (1200.0 / m_printerDPI.x);
		double targSizeInch1200thsY = targRect.Height() * (1200.0 / m_printerDPI.y);
		double srcSizePixelsX = m_pCDibForPrint->GetSize().cx;
		double srcSizePixelsY = m_pCDibForPrint->GetSize().cy;

		if ((targSizeInch1200thsX / srcSizePixelsX) >
			(targSizeInch1200thsY / srcSizePixelsY)) {

			// we are strecthing x more than y.
			targSizeInch1200thsX = srcSizePixelsX *
				(targSizeInch1200thsY / srcSizePixelsY);
		} else {
			targSizeInch1200thsY = srcSizePixelsY *
				(targSizeInch1200thsX / srcSizePixelsX);
		}

		targRect = CRect(m_printOrigin.x, m_printOrigin.y,
			(int)(m_printOrigin.x + (m_printerDPI.x / 1200.0) * targSizeInch1200thsX),
			(int)(m_printOrigin.y + (m_printerDPI.y / 1200.0) * targSizeInch1200thsY));

		if (!m_pCDibForPrint->Paint(pDC, targRect)) {
			ASSERT(0);
		}
	}
}

/***************************************************************************/
// CSaView::DoHiResPrint
//
// Print the graphs using the OnDraw() methods.
/***************************************************************************/
void CSaView::DoHiResPrint(CDC * pDC, CPrintInfo * pInfo) {
	CRect viewRect;
	GetWindowRect(&viewRect);

	int titleAreaHeight = fract_multiply(Print_Title_Height, m_newPrinterDPI);  // half inch
	PrintPageTitle(pDC, titleAreaHeight);

	// adjust print area and origin to move past the title
	CPoint oldOrg(-m_printOrigin);
	oldOrg.y -= titleAreaHeight;
	pDC->SetWindowOrg(oldOrg);
	m_printArea.y -= titleAreaHeight;

	CalcPrintScaling(&viewRect);

	if (m_pPageLayout->LayoutType() == m_pPageLayout->LAYOUT_1GRAPH_PER_PAGE()) {
		int graphIndex = CalcGraphIndexForCurrentPage(pInfo->m_nCurPage);
		if (graphIndex < MAX_GRAPHS_NUMBER) {
			PrintGraph(pDC, &viewRect, graphIndex, &viewRect, oldOrg.x, oldOrg.y);
		}
	} else if (m_pPageLayout->LayoutType() == m_pPageLayout->LAYOUT_AS_SCREEN()) {
		for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
			if (m_apGraphs[nLoop]) {
				CRect graphRect;
				m_apGraphs[nLoop]->GetWindowRect(&graphRect);
				PrintGraph(pDC, &viewRect, nLoop, &graphRect, oldOrg.x, oldOrg.y);
			}
		}
	} else {
		// custom layout - user specifies number of rows and columns
		int virtualPage = (pInfo->m_nCurPage - 1) * m_pPageLayout->NumRows() * m_pPageLayout->NumCols() + 1;
		for (int row = 1; row <= m_pPageLayout->NumRows(); row++) {
			for (int col = 1; col <= m_pPageLayout->NumCols(); col++) {
				int graphIndex = CalcGraphIndexForCurrentPage(virtualPage);
				CRect customPage;
				CalcCustomPage(&customPage, &viewRect, row, col);
				if (graphIndex < MAX_GRAPHS_NUMBER) {
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
int CSaView::CalcGraphIndexForCurrentPage(int currPage) {
	int page = 0;
	int graphForPage = 0;

	for (; graphForPage < MAX_GRAPHS_NUMBER; graphForPage++) {
		if (m_apGraphs[graphForPage] && m_pPageLayout->IsGraphSelected(graphForPage)) {
			page++;
		}

		if (page == currPage) {
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
void CSaView::CalcCustomPage(CRect * customPage, const CRect * viewRect, int row, int col) {
	int seperator = fract_multiply(Print_Graph_Seperator, m_newPrinterDPI);
	CPoint customDimensions(viewRect->Width() / m_pPageLayout->NumCols(),
		viewRect->Height() / m_pPageLayout->NumRows());

	customPage->left = viewRect->left + (col - 1) * customDimensions.x + seperator;
	customPage->right = customPage->left + customDimensions.x - seperator;
	customPage->top = viewRect->top + (row - 1) * customDimensions.y + seperator;
	customPage->bottom = customPage->top + customDimensions.y - seperator;
}


/***************************************************************************/
// CSaView::PrintPageTitle
//
// Print a title on the page.
/***************************************************************************/
void CSaView::PrintPageTitle(CDC * pDC, int titleAreaHeight) {

	// load file name
	CSaString szDocTitle(GetDocument()->GetTitle());
	CSaString szTitle = "Speech Analyzer - ";

	int nFind = szDocTitle.Find(':');
	if (nFind != -1) {
		// extract part left of :
		szDocTitle = szDocTitle.Left(nFind);
	}
	szTitle += szDocTitle;

	int textWidth = pDC->GetTextExtent(szTitle, szTitle.GetLength()).cx;

	// centered on the page
	CRect titleRect((m_printArea.x - textWidth) / 2,
		fract_multiply(Print_Title_Offset, m_newPrinterDPI),
		m_printArea.x,
		titleAreaHeight);

	// set font color
	COLORREF oldColor = pDC->SetTextColor(RGB(0, 0, 0));

	LPCTSTR pszName = _T("Arial");

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
	int originX, int originY) {
	CRect printRect;

	// calculate printRect and scaledOffset
	CalcPrintRect(&printRect, graphRect);
	CPoint offset(viewRect->left - graphRect->left,
		viewRect->top - graphRect->top);
	CPoint scaledOffset;
	CalcPrintPoint(&scaledOffset, &offset);

	originX += scaledOffset.x;
	originY += scaledOffset.y;

	pDC->SetWindowOrg(CPoint(originX, originY));

	m_apGraphs[graphNum]->OnDraw(pDC, &printRect, originX, originY);
}

/***************************************************************************/
// CSaView::CalcPrintScaling
//
// Calculate the scaling factor to fit the srcRect in the m_printArea.
/***************************************************************************/
void CSaView::CalcPrintScaling(const CRect * srcRect) {
	m_printScaleX = (double)((1.0 * m_printArea.x) / srcRect->Width());
	m_printScaleY = (double)((1.0 * m_printArea.y) / srcRect->Height());

	if (m_pPageLayout->bFixedAspectR()) {
		// force equal scaling of x and y
		if (m_printScaleX > m_printScaleY) {
			m_printScaleX = m_printScaleY;
		} else if (m_printScaleY > m_printScaleX) {
			m_printScaleY = m_printScaleX;
		}
	}
}

/***************************************************************************/
// CSaView::CalcPrintRect
//
// Scale a srcRect by the scaling factors, return as pPrintRect.
/***************************************************************************/
void CSaView::CalcPrintRect(CRect * pPrintRect, const CRect * srcRect) {
	*pPrintRect = CRect(0, 0, (int)(m_printScaleX * srcRect->Width()),
		(int)(m_printScaleY * srcRect->Height()));

}

//*************************************************************************
// CSaView::CalcPrintPoint
//
// Scale a srcPoint by the scaling factors, return as pPrintPoint.
/***************************************************************************/
void CSaView::CalcPrintPoint(CPoint * pPrintPoint, const CPoint * srcPoint) {
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
int CSaView::CalculateHiResPrintPages(void) {
	int numGraphs = 0;

	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		if (m_apGraphs[nLoop]) {
			numGraphs++;
		}
	}
	numGraphs = min(m_pPageLayout->NumGraphsSelected(), numGraphs);

	if (m_pPageLayout->LayoutType() == m_pPageLayout->LAYOUT_1GRAPH_PER_PAGE()) {
		return numGraphs;
	} else if (m_pPageLayout->LayoutType() == m_pPageLayout->LAYOUT_AS_SCREEN()) {
		return 1;
	} else {
		int graphsPerPage = m_pPageLayout->NumRows() * m_pPageLayout->NumCols();
		int remainder = numGraphs % graphsPerPage;

		if (!remainder) {
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
void CSaView::PreparePrintingForScreenShot(void) {

	CWindowDC scrn(AfxGetMainWnd());

	scrn.SelectClipRgn(NULL); // select entire window client area
	scrn.GetClipBox(&m_memRect);
	if (!m_memRect.Width() || !m_memRect.Height()) {
		CSaString szError = "printing area empty, nothing to print";
		ErrorMessage(szError);
		return;
	}

	// create a temporary DC for the reading the screen
	CDC * pMemDC = new CDC;
	if (!pMemDC) {
		CSaString szError = "unable to allocate memory for screen shot printing";
		ErrorMessage(szError);
		ASSERT(0);
	} else if (!pMemDC->CreateCompatibleDC(&scrn)) {
		CSaString szError = "unable to allocate memory for screen shot printing";
		ErrorMessage(szError);
		ASSERT(0);
		delete pMemDC;
	} else {
		// create a bitmap to read the screen into and select it
		// into the temporary DC
		CBitmap * pBitmapForPrint = new CBitmap;

		if (!pBitmapForPrint) {
			CSaString szError = "unable to allocate memory for screen shot printing";
			ErrorMessage(szError);
			ASSERT(0);
		} else if (!pBitmapForPrint->CreateCompatibleBitmap(&scrn, m_memRect.Width(), m_memRect.Height())) {
			CSaString szError = "unable to allocate memory for screen shot printing";
			ErrorMessage(szError);
			ASSERT(0);
			delete pBitmapForPrint;
		} else {
			CBitmap * oldBitmap = (CBitmap *)pMemDC->SelectObject(pBitmapForPrint);

			if (!oldBitmap) {
				CSaString szError = "unable to select bitmap for screen shot printing";
				ErrorMessage(szError);
				ASSERT(0);
			} else {
				// BitBlt the screen data into the bitmap
				if (!pMemDC->BitBlt(0, 0, m_memRect.Width(), m_memRect.Height(), &scrn, m_memRect.left, m_memRect.top, SRCCOPY)) {
					CSaString szError = "unable to copy for screen shot printing";
					ErrorMessage(szError);
					ASSERT(0);
					m_pCDibForPrint = NULL;
				} else {
					// create a device independent bitmap from the regular bitmap
					m_pCDibForPrint = new CDib(pMemDC, 8, FALSE);  // 8-bit screen shot
					if (!m_pCDibForPrint) {
						CSaString szError = "unable to convert for screen shot printing";
						ErrorMessage(szError);
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
BOOL CSaView::SaDoPreparePrinting(CPrintInfo * pInfo, BOOL isLandscape) {

	ASSERT(pInfo != NULL);
	ASSERT(pInfo->m_pPD != NULL);

	CSaApp * pApp = (CSaApp*)AfxGetApp();

	if (pInfo->m_bPreview) {
		// if preview, get default printer DC and create DC without calling
		//   print dialog.
		if ((!pApp->SaGetPrinterDeviceDefaults(&pInfo->m_pPD->m_pd, isLandscape)) &&
			(pApp->DoPrintDialog(pInfo->m_pPD) != IDOK))
			// bring up dialog to alert the user they need to install a printer.
		{
			return FALSE;
		}

		// call CreatePrinterDC if DC was not created by above
		if ((pInfo->m_pPD->m_pd.hDC == NULL) &&
			(pInfo->m_pPD->CreatePrinterDC() == NULL)) {
			return FALSE;
		}

		// set up From and To page range from Min and Max
		pInfo->m_pPD->m_pd.nFromPage = (WORD)pInfo->GetMinPage();
		pInfo->m_pPD->m_pd.nToPage = (WORD)pInfo->GetMaxPage();

	} else {
		// otherwise, bring up the print dialog and allow user to change things

		// preset From-To range same as Min-Max range
		pInfo->m_pPD->m_pd.nFromPage = (WORD)pInfo->GetMinPage();
		pInfo->m_pPD->m_pd.nToPage = (WORD)pInfo->GetMaxPage();

		if (pApp->SaDoPrintDialog(pInfo->m_pPD, isLandscape) != IDOK) {
			// do not print
			return FALSE;
		}
	}

	ASSERT(pInfo->m_pPD != NULL);
	ASSERT(pInfo->m_pPD->m_pd.hDC != NULL);

	pInfo->m_nNumPreviewPages = pApp->m_nNumPreviewPages;
	VERIFY(pInfo->m_strPageDesc.LoadString(AFX_IDS_PREVIEWPAGEDESC));
	return TRUE;
}

/***************************************************************************/
// CSaView::IsCutAllowed Checks, if cut possible (return TRUE)
/***************************************************************************/
BOOL CSaView::IsCutAllowed() {
	BOOL bHighLighted = FALSE;
	if ((m_nFocusedID == IDD_RAWDATA) &&
		(!GetDocument()->GetWbProcess()) &&
		(m_pFocusedGraph->GetPlot()->GetHighLightLength() > 0)) {
		bHighLighted = TRUE;
	}
	return (IsAnyAnnotationSelected() || bHighLighted);
}

/***************************************************************************/
// CSaView::OnEditCopy
/***************************************************************************/
void CSaView::OnEditCopy() {
	int nSegment = FindSelectedAnnotationIndex();
	if (!IsCutAllowed()) { // copy data point
		OnEditCopyMeasurements();
		return;
	}

	if (nSegment != -1) {
		CSaString ctext(GetSelectedAnnotationString());
		HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, (ctext.GetLength() + 1)*sizeof(TCHAR));
		ASSERT(hData);
		if (hData) {
			LPTSTR lpData = (TCHAR *)GlobalLock(hData);
			ASSERT(lpData);
			if (lpData) {
				int len = ctext.GetLength();
				TCHAR * str = ctext.GetBuffer(len + 1);
				ASSERT(str);
				if (nSegment == GLOSS) {
					str++;
					len--;
				}
				memcpy(lpData, str, len*sizeof(TCHAR));
				lpData[len] = 0;
				ctext.ReleaseBuffer();
				GlobalUnlock(hData);

				// Clear the current contents of the clipboard, and set
				// the data handle to the new string.
				if (OpenClipboard()) {
					EmptyClipboard();
					SetClipboardData(CF_UNICODETEXT, hData);
					CloseClipboard();
				}
			}
		}
	} else {
		if ((m_nFocusedID == IDD_RAWDATA) && (m_pFocusedGraph->GetPlot()->GetHighLightLength() > 0)) {
			// get the wave section boundaries
			DWORD dwSectionStart = m_pFocusedGraph->GetPlot()->GetHighLightPosition();
			DWORD dwSectionLength = m_pFocusedGraph->GetPlot()->GetHighLightLength();
			CSaDoc * pModel = GetDocument();
			WAVETIME start = pModel->toTime(dwSectionStart, true);
			WAVETIME length = pModel->toTime(dwSectionLength, true);
			pModel->PutWaveToClipboard(start, length);
		}
	}
}

/***************************************************************************/
// CSaView::OnEditCopy
/***************************************************************************/
void CSaView::OnUpdateEditCopy(CCmdUI * pCmdUI) {
	BOOL enable = FALSE;
	int nSegment = FindSelectedAnnotationIndex();
	if (nSegment != -1) {
		enable = TRUE;
	} else if ((m_nFocusedID == IDD_RAWDATA) && (m_pFocusedGraph->GetPlot()->GetHighLightLength() > 0)) {
		enable = TRUE;
	}

	pCmdUI->Enable(enable);
}

/***************************************************************************/
// CSaView::OnEditCopyMeasurements
/***************************************************************************/
void CSaView::OnEditCopyMeasurements() {

	// get info from the document
	DWORD dwOffset = GetStartCursorPosition();
	BOOL bSection = (m_nFocusedID != IDD_MELOGRAM) && (m_nFocusedID != IDD_RECORDING) && (m_pFocusedGraph->GetPlot()->GetHighLightLength() > 0);
	DWORD dwSectionStart = m_pFocusedGraph->GetPlot()->GetHighLightPosition();
	DWORD dwSectionLength = m_pFocusedGraph->GetPlot()->GetHighLightLength();
	BOOL pbRes;
	CSaDoc * pModel = GetDocument();

	CSaString ctext;
	if (bSection) {
		ctext = pModel->GetMeasurementsString(dwSectionStart, dwSectionLength, &pbRes);
	} else {
		ctext = pModel->GetMeasurementsString(dwOffset, 0, &pbRes);
	}

	int len = ctext.GetLength();

	// clear the clipboard and pass the new text
	HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, (len + 1)*sizeof(TCHAR));
	ASSERT(hData);
	if (hData) {
		LPTSTR lpData = (TCHAR *)GlobalLock(hData);
		ASSERT(lpData);
		if (lpData) {
			TCHAR * str = ctext.GetBuffer(len + 1);
			ASSERT(str);
			memcpy(lpData, str, len*sizeof(TCHAR));
			lpData[len] = 0;
			ctext.ReleaseBuffer();
			GlobalUnlock(hData);

			if (OpenClipboard()) {
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
void CSaView::OnEditCut() {

	// cut annontation
	if (IsAnyAnnotationSelected()) {
		OnEditCopy();
		RemoveSelectedAnnotation();
		return;
	}

	// cut wave
	if ((m_nFocusedID == IDD_RAWDATA) && (m_pFocusedGraph->GetPlot()->GetHighLightLength() > 0)) {
		// copy a wavefile section to the clipboard and delete it from the file
		// get the wave section boundaries
		DWORD dwSectionStart = m_pFocusedGraph->GetPlot()->GetHighLightPosition();
		DWORD dwSectionLength = m_pFocusedGraph->GetPlot()->GetHighLightLength();
		CSaDoc * pModel = (CSaDoc *)GetDocument();
		WAVETIME start = pModel->toTime(dwSectionStart, true);
		WAVETIME length = pModel->toTime(dwSectionLength, true);
		if (pModel->PutWaveToClipboard(start, length, TRUE)) {
			pModel->InvalidateAllProcesses();
			RedrawGraphs();
			m_pFocusedGraph->GetPlot()->ClearHighLightArea();
		}
	}
}

/***************************************************************************/
// CSaView::OnEditPaste
/***************************************************************************/
void CSaView::OnEditPaste() {
	// get pointer to document
	CSaDoc * pModel = (CSaDoc *)GetDocument();
	// is an annotation selected?
	if (IsAnyAnnotationSelected()) {
		if (OpenClipboard()) {
			HGLOBAL hClipData = NULL;
			LPTSTR lpClipData = NULL;
			// get text from the clipboard
			if (NULL != (hClipData = GetClipboardData(CF_UNICODETEXT))) {
				if (NULL != (lpClipData = (LPTSTR)GlobalLock(hClipData))) {
					CSaString data(lpClipData);
					m_advancedSelection.SetSelectedAnnotationString(this, data, false, true);
					GlobalUnlock(hClipData);
					RedrawGraphs();
				}
			}
			CloseClipboard();
		}
	}
	// is a wave selected?
	if (m_nFocusedID == IDD_RAWDATA) {
		if (OpenClipboard()) {
			if (IsClipboardFormatAvailable(CF_WAVE)) {
				HGLOBAL hGlobal = GetClipboardData(CF_WAVE);
				if (hGlobal != NULL) {
					DWORD start = GetStartCursorPosition();
					WAVETIME startTime = pModel->toTime(start, true);
					if (pModel->PasteClipboardToWave(hGlobal, startTime)) {
						// get wave from the clipboard
						pModel->InvalidateAllProcesses();
						RedrawGraphs();
						m_pFocusedGraph->GetPlot()->ClearHighLightArea();
					}
				}
			}
			CloseClipboard();
		}
	}
}

/***************************************************************************/
// CSaView::OnEditPasteNew
/***************************************************************************/
void CSaView::OnEditPasteNew() {
	if (OpenClipboard()) {
		if (IsClipboardFormatAvailable(CF_WAVE)) {
			// call the application to create a new file and put in the clipboard contents
			CSaApp * pApp = (CSaApp*)AfxGetApp();
			pApp->PasteClipboardToNewFile(GetClipboardData(CF_WAVE));
		}
		CloseClipboard();
	}
}

/***************************************************************************/
// CSaView::IsAnyAnnotationSelected
// Returns TRUE if something is selected in some annotation window, else FALSE.
/***************************************************************************/
bool CSaView::IsAnyAnnotationSelected(void) {
	return (FindSelectedAnnotation() != NULL);
}

/***************************************************************************/
// CSaView::FindSelectedAnnotation
/***************************************************************************/
CSegment * CSaView::FindSelectedAnnotation() {
	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		CSegment * pSegment = GetAnnotation(nLoop);
		if ((pSegment != NULL) && (pSegment->GetSelection() != -1)) {
			return pSegment;
		}
	}
	return NULL;
}

/***************************************************************************/
// CSaView::FindSelectedAnnotation
/***************************************************************************/
int CSaView::FindSelectedAnnotationIndex() {
	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		CSegment * pSegment = GetAnnotation(nLoop);
		if ((pSegment != NULL) && (pSegment->GetSelection() != -1)) {
			return nLoop;
		}
	}
	return -1;
}

/***************************************************************************/
// CSaView::ChangeSelectedAnnotationData
/***************************************************************************/
void CSaView::ChangeSelectedAnnotationData(const CSaString & str) {
	// get pointer to document
	CSaDoc * pModel = (CSaDoc *)GetDocument();
	ASSERT(pModel);
	pModel->CheckPoint();
	{
		CSegment * pAnnotationSet = FindSelectedAnnotation();
		ASSERT(pAnnotationSet);
		if (pAnnotationSet != NULL) {
			pAnnotationSet->ReplaceSelectedSegment(pModel, str, false);
		}
	}
}

/***************************************************************************/
// CSaView::RemoveSelectedAnnotation
/***************************************************************************/
void CSaView::RemoveSelectedAnnotation() {
	CSegment * pSegment = FindSelectedAnnotation();
	if (pSegment == NULL) {
		return;
	}
	int index = pSegment->GetSelection();
	if (index == -1) {
		return;
	}
	pSegment->Remove(GetDocument(), index, TRUE);
}

/***************************************************************************/
// CSaView::OnUpdateEditPaste
// If something for the clipboard is selected, it enables the item.
/***************************************************************************/
void CSaView::OnUpdateEditPaste(CCmdUI * pCmdUI) {

	BOOL enablePaste = FALSE;
	if (IsAnyAnnotationSelected()) {
		if (OpenClipboard()) {
			if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
				CSaApp * pApp = (CSaApp*)AfxGetApp();
				LPCTSTR path = pApp->GetLastClipboardPath();
				if (wcslen(path) > 0) {
					enablePaste = TRUE;
				}
			}
			CloseClipboard();
		}
	}

	CSaDoc * pModel = GetDocument();
	if (m_nFocusedID == IDD_RAWDATA) {
		if (!pModel->GetWbProcess()) {
			if (OpenClipboard()) {
				if (IsClipboardFormatAvailable(CF_WAVE)) {
					enablePaste = TRUE;
				}
				CloseClipboard();
			}
		}
	}
	pCmdUI->Enable(enablePaste);
}

/***************************************************************************/
// CSaView::OnUpdateEditPasteNew
// If waveform available on the clipboard, it enables the item.
/***************************************************************************/
void CSaView::OnUpdateEditPasteNew(CCmdUI * pCmdUI) {

	BOOL enablePaste = FALSE;
	if (OpenClipboard()) {
		if (IsClipboardFormatAvailable(CF_WAVE)) {
			CSaApp * pApp = (CSaApp*)AfxGetApp();
			LPCTSTR path = pApp->GetLastClipboardPath();
			if (wcslen(path) > 0) {
				enablePaste = TRUE;
			}
		}
		CloseClipboard();
	}
	pCmdUI->Enable(enablePaste);
}

/***************************************************************************/
// CSaView::OnUpdateHasSel
// Is something for the clipboard is selected, it enables the item.
/***************************************************************************/
void CSaView::OnUpdateHasSel(CCmdUI * pCmdUI) {
	pCmdUI->Enable(IsCutAllowed());
}

/***************************************************************************/
/************************UNDO/REDO******************************************/
/***************************************************************************/

/***************************************************************************/
// CSaView::OnEditUndo
/***************************************************************************/
void CSaView::OnEditUndo() {
	CSaDoc * pModel = (CSaDoc *)GetDocument();
	pModel->Undo();
	if (pModel->IsWaveToUndo()) {
		pModel->UndoWaveFile();
	}
	// SDM 1.06.6U4 Set cursors to selected segment on undo
	CSegment * pSegment = FindSelectedAnnotation();
	if (pSegment != NULL) {
		int nIndex = pSegment->GetSelection();
		SetStartCursorPosition(pSegment->GetOffset(nIndex));
		SetStopCursorPosition(pSegment->GetStop(nIndex));
	}
	RedrawGraphs();
}

/***************************************************************************/
// CSaView::OnUpdateEditUndo
/***************************************************************************/
void CSaView::OnUpdateEditUndo(CCmdUI * pCmdUI) {
	pCmdUI->Enable(((CSaDoc *)GetDocument())->CanUndo());
}

/***************************************************************************/
// CSaView::OnEditRedo
/***************************************************************************/
void CSaView::OnEditRedo() {
	((CSaDoc *)GetDocument())->Redo();
	RedrawGraphs();
}

/***************************************************************************/
// CSaView::OnUpdateEditRedo
/***************************************************************************/
void CSaView::OnUpdateEditRedo(CCmdUI * pCmdUI) {
	pCmdUI->Enable(((CSaDoc *)GetDocument())->CanRedo());
}

/***************************************************************************/
// CSaView::OnEditCursorStartLeft Move Start Cursor Left
/***************************************************************************/
void CSaView::OnEditCursorStartLeft() {
	CSaDoc * pModel = GetDocument();
	int nBlockAlign = pModel->GetBlockAlign();
	DWORD movementScale = ((DWORD)(m_fMaxZoom / m_fZoom + 0.5))* nBlockAlign;
	DWORD dwOffset = GetStartCursorPosition();
	if (dwOffset < movementScale) {
		return;
	}

	DWORD dwOffsetNew = pModel->SnapCursor(START_CURSOR, dwOffset - movementScale, SNAP_LEFT);
	if (dwOffset != dwOffsetNew) {
		SetStartCursorPosition(dwOffsetNew);
	}
}

/***************************************************************************/
// CSaView::OnEditCursorStartRight Move Start Cursor Right
/***************************************************************************/
void CSaView::OnEditCursorStartRight() {
	CSaDoc * pModel = GetDocument();
	int nBlockAlign = pModel->GetBlockAlign();
	DWORD minSeparation = ((DWORD)(CURSOR_MIN_DISTANCE * (m_fMaxZoom / m_fZoom))) * nBlockAlign;
	DWORD dataSize = pModel->GetDataSize();
	DWORD movementScale = ((DWORD)(m_fMaxZoom / m_fZoom + 0.5))* nBlockAlign;
	DWORD dwOffset = GetStartCursorPosition();
	// off the end!
	if ((dwOffset + movementScale) >= dataSize) {
		return;
	}

	DWORD dwOffsetNew = pModel->SnapCursor(START_CURSOR, dwOffset + movementScale, SNAP_RIGHT);
	DWORD dwStop = GetStopCursorPosition();
	DWORD dwStopNew = dwStop;
	if (dwOffsetNew + minSeparation > dwStopNew) {
		dwStopNew = pModel->SnapCursor(STOP_CURSOR, dwOffsetNew + minSeparation, SNAP_RIGHT);
	}
	if (dwOffsetNew + minSeparation > dwStopNew) {
		dwOffsetNew = pModel->SnapCursor(START_CURSOR, (dwStopNew > minSeparation) ? (dwStopNew - minSeparation) : 0, SNAP_LEFT);
	}
	if (dwOffsetNew + minSeparation > dwStopNew) {
		return;
	}
	if (dwStop != dwStopNew) {
		SetStopCursorPosition(dwStopNew);
	}
	if (dwOffset != dwOffsetNew) {
		SetStartCursorPosition(dwOffsetNew);
	}
}

/***************************************************************************/
// CSaView::OnEditCursorStopRight Move Stop Cursor Right
/***************************************************************************/
void CSaView::OnEditCursorStopRight() {
	CSaDoc * pModel = GetDocument();
	int nBlockAlign = pModel->GetBlockAlign();
	DWORD dataSize = pModel->GetDataSize();
	DWORD movementScale = ((DWORD)(m_fMaxZoom / m_fZoom + 0.5))* nBlockAlign;
	DWORD dwStop = GetStopCursorPosition();
	if (dwStop + movementScale >= dataSize) {
		return;
	}

	DWORD dwStopNew = pModel->SnapCursor(STOP_CURSOR, dwStop + movementScale, SNAP_RIGHT);
	if (dwStop != dwStopNew) {
		SetStopCursorPosition(dwStopNew);
	}
}

/***************************************************************************/
// CSaView::OnEditCursorStopLeft Move Stop Cursor Left
/***************************************************************************/
void CSaView::OnEditCursorStopLeft() {
	CSaDoc * pModel = GetDocument();
	int nBlockAlign = pModel->GetBlockAlign();
	DWORD minSeparation = ((DWORD)(CURSOR_MIN_DISTANCE * (m_fMaxZoom / m_fZoom))) * nBlockAlign;
	DWORD movementScale = ((DWORD)(m_fMaxZoom / m_fZoom + 0.5))* nBlockAlign;
	DWORD dwStop = GetStopCursorPosition();
	if (dwStop < movementScale) {
		return;
	}

	DWORD dwStopNew = pModel->SnapCursor(STOP_CURSOR, dwStop - movementScale, SNAP_LEFT);

	DWORD dwOffset = GetStartCursorPosition();
	DWORD dwOffsetNew = dwOffset;
	if ((dwOffsetNew + minSeparation) > dwStopNew) {
		dwOffsetNew = pModel->SnapCursor(START_CURSOR, (dwStopNew > minSeparation) ? (dwStopNew - minSeparation) : 0, SNAP_LEFT);
	}
	if ((dwOffsetNew + minSeparation) > dwStopNew) {
		dwStopNew = pModel->SnapCursor(STOP_CURSOR, dwOffsetNew + minSeparation, SNAP_RIGHT);
	}

	if (dwOffsetNew + minSeparation > dwStopNew) {
		return;
	}

	if (dwOffset != dwOffsetNew) {
		SetStartCursorPosition(dwOffsetNew);
	}

	if (dwStop != dwStopNew) {
		SetStopCursorPosition(dwStopNew);
	}
}

/***************************************************************************/
// CSaView::OnEditBoundaryStartLeft Move Start Cursor Left
/***************************************************************************/
void CSaView::OnEditBoundaryStartLeft() {
	MoveBoundary(true, true);
}

/***************************************************************************/
// CSaView::OnEditBoundaryStartRight Move Start Cursor Right
/***************************************************************************/
void CSaView::OnEditBoundaryStartRight() {
	MoveBoundary(true, false);
}

/***************************************************************************/
// CSaView::OnEditCursorStopLeft Move Stop Cursor Left
/***************************************************************************/
void CSaView::OnEditBoundaryStopLeft() {
	MoveBoundary(false, true);
}

/***************************************************************************/
// CSaView::OnEditCursorStopRight Move Stop Cursor Right
/***************************************************************************/
void CSaView::OnEditBoundaryStopRight() {
	MoveBoundary(false, false);
}

void CSaView::MoveBoundary(bool start, bool left) {
	bool editSegment = (m_bEditSegmentSize | m_bEditBoundaries);
	if (!editSegment) {
		return;
	}

	bool overlap = m_bEditSegmentSize;
	if (overlap) {
		TRACE("clearing\n");
		lastBoundaryStartCursor = UNDEFINED_OFFSET;
		lastBoundaryStopCursor = UNDEFINED_OFFSET;
		lastBoundaryCursor = UNDEFINED_CURSOR;
		lastBoundaryIndex = -1;
	}

	ECursorSelect cursor = (start) ? START_CURSOR : STOP_CURSOR;
	if (cursor != lastBoundaryCursor) {
		TRACE("clearing\n");
		lastBoundaryStartCursor = UNDEFINED_OFFSET;
		lastBoundaryStopCursor = UNDEFINED_OFFSET;
		//lastBoundaryIndex = -1;
		lastBoundaryCursor = cursor;
	}

	// if there's no annotation selection, we can't move anything...
	int nLoop = FindSelectedAnnotationIndex();
	if (nLoop == -1) {
		TRACE("no selection\n");
		return;
	}
	if (nLoop != lastBoundaryIndex) {
		TRACE("clearing\n");
		lastBoundaryStartCursor = UNDEFINED_OFFSET;
		lastBoundaryStopCursor = UNDEFINED_OFFSET;
		//lastBoundaryCursor = UNDEFINED_CURSOR;
		lastBoundaryIndex = nLoop;
	}

	// are we editing segment size?
	CSegment * pSegment = GetAnnotation(nLoop);
	if (pSegment == NULL) {
		return;
	}
	CGraphWnd * pGraph = GetGraphForAnnotation(nLoop);
	if (pGraph == NULL) {
		return;
	}
	CAnnotationWnd * pWnd = pGraph->GetAnnotationWnd(nLoop);
	if (pWnd == NULL) {
		return;
	}
	CPlotWnd * pPlot = pGraph->GetPlot();
	if (pPlot == NULL) {
		return;
	}

	// Limit positions of cursors
	CSegment::ELimit mode = (start) ? ((overlap) ? CSegment::LIMIT_MOVING_START : CSegment::LIMIT_MOVING_START_NO_OVERLAP) : ((overlap) ? CSegment::LIMIT_MOVING_STOP : CSegment::LIMIT_MOVING_STOP_NO_OVERLAP);

	CSaDoc * pModel = GetDocument();

	int nBlockAlign = pModel->GetBlockAlign();
	DWORD movement = ((DWORD)(m_fMaxZoom / m_fZoom + 0.5))* nBlockAlign;
	DWORD minSeparation = GetMinimumSeparation(pModel, pGraph, pPlot);
	TRACE("minSep=%d\n", minSeparation);

	DWORD dwStart = GetStartCursorPosition();
	DWORD dwStop = GetStopCursorPosition();
	//TRACE("start=%d stop=%d\n",dwStart,dwStop);

	// record the original cursor positions
	if (!overlap) {
		if (start) {
			if (!left) {
				if (lastBoundaryStopCursor == UNDEFINED_OFFSET) {
					TRACE("setting stop\n");
					lastBoundaryStopCursor = dwStop;
				}
			}
		} else {
			if (left) {
				if (lastBoundaryStartCursor == UNDEFINED_OFFSET) {
					TRACE("setting start\n");
					lastBoundaryStartCursor = dwStart;
				}
			}
		}
	}

	DWORD dataSize = pModel->GetDataSize();

	ESnapDirection snap = (left) ? SNAP_LEFT : SNAP_RIGHT;

	DWORD minStart = 0;
	DWORD maxStart = dataSize - minSeparation;
	DWORD minStop = minSeparation;
	DWORD maxStop = dataSize;

	DWORD startGap = dwStart + minSeparation;
	DWORD stopGap = dwStop - minSeparation;

	if (start) {
		dwStart = (left) ? ((dwStart < movement) ? 0 : (dwStart - movement)) : (dwStart += movement);
		dwStart = (overlap) ? ((dwStart > stopGap) ? stopGap : dwStart) : dwStart;
		// check limits
		dwStart = (dwStart < minStart) ? minStart : dwStart;
		dwStart = (dwStart > maxStart) ? maxStart : dwStart;
		if (left) {
		}
		dwStart = pModel->SnapCursor(START_CURSOR, dwStart, snap);
	} else {
		dwStop = (left) ? ((dwStop < movement) ? 0 : (dwStop - movement)) : (dwStop += movement);
		dwStop = (overlap) ? ((dwStop < startGap) ? startGap : dwStop) : dwStop;
		// check limits
		dwStop = (dwStop < minStop) ? minStop : dwStop;
		dwStop = (dwStop > maxStop) ? maxStop : dwStop;
		dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, snap);
	}

	//TRACE("start=%d stop=%d\n",dwStart,dwStop);

	// on edit boundaries, we push the other cursor if we run into it.
	// we need to manage restore a 'pushed' cursor to it's original position
	// if the 'pulling' cursor restores the 'pushed' cursor paste it's original location.
	if (!overlap) {
		if (start) {
			startGap = dwStart + minSeparation;
			dwStop = (dwStop<startGap) ? startGap : dwStop;
			if (lastBoundaryStopCursor != UNDEFINED_OFFSET) {
				if ((left) && (dwStop>startGap)) {
					// 'pull' the stop cursor if necessary
					dwStop = startGap;
				}
				if (dwStop<lastBoundaryStopCursor) {
					TRACE("recovering\n");
					dwStop = lastBoundaryStopCursor;
				}
				//TRACE("start=%d stop1=%d stop2=%d last=%d startgap=%d\n",dwStart,a,dwStop,lastBoundaryStopCursor,startGap);
			}
			dwStop = (dwStop < minStop) ? minStop : dwStop;
			dwStop = (dwStop > maxStop) ? maxStop : dwStop;
			dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop);
		} else {
			stopGap = dwStop - minSeparation;
			dwStart = (dwStart>stopGap) ? stopGap : dwStart;
			if (lastBoundaryStartCursor != UNDEFINED_OFFSET) {
				if ((!left) && (dwStart < stopGap)) {
					// 'pull' the stop cursor if necessary
					dwStart = stopGap;
				}
				if (dwStart > lastBoundaryStartCursor) {
					TRACE("recovering\n");
					dwStart = lastBoundaryStartCursor;
				}
			}
			dwStart = (dwStart < minStart) ? minStart : dwStart;
			dwStart = (dwStart > maxStart) ? maxStart : dwStart;
			dwStart = pModel->SnapCursor(START_CURSOR, dwStart);
		}
	}

	// see if it will fly...
	if (pSegment->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_AUTOMATIC, overlap) == -1) {
		TRACE("failed check\n");
		return;
	}

	// start making changes...
	pModel->CheckPoint(); // Save state

	TRACE("(e) start=%d stop=%d\n", dwStart, dwStop);

	pSegment->LimitPosition(pModel, dwStart, dwStop, mode);

	SetStartCursorPosition(dwStart);
	SetStopCursorPosition(dwStop);
	pWnd->SetHintUpdateBoundaries(false, overlap);
	pModel->UpdateSegmentBoundaries(overlap);
}

//SDM 1.06.6U2
/***************************************************************************/
// CSaView::OnEditAddSyllable Add Syllable Break to Phonetic Segment
/***************************************************************************/
void CSaView::OnEditAddSyllable() {
	CSaDoc * pModel = GetDocument(); // get pointer to document
	pModel->CheckPoint();
	CSaString szString = "."; //Fill new segment with segment break character

	CPhoneticSegment * pSeg = (CPhoneticSegment *)GetAnnotation(PHONETIC);

	if (pSeg->GetSelection() != -1) { // Phonetic Segment Selected
		int nSelection = pSeg->GetSelection();
		DWORD dwStart = pSeg->GetOffset(nSelection); // Start at current start
		DWORD dwMaxStop;
		DWORD dwStop;

		dwMaxStop = dwStart + pSeg->GetDuration(nSelection) - pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME);

		if (pModel->Is16Bit()) { // SDM 1.5Test8.2
			dwMaxStop = dwMaxStop & ~1; // Round down
		}

		// Snap Start Position
		dwStart = pModel->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);

		dwStop = (dwStart + pModel->GetBytesFromTime(ADD_SYLLABLE_TIME));

		if (pModel->Is16Bit()) { // SDM 1.5Test8.2
			dwStop = (dwStop + 1) & ~1; // Round up
		}

		dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStop, pModel->GetDataSize(), SNAP_RIGHT);

		if (dwStop <= dwMaxStop) { // enough room
			pSeg->Adjust(pModel, nSelection, dwStop, pSeg->GetDuration(nSelection) + dwStop - pSeg->GetOffset(nSelection), false);
			pSeg->Insert(nSelection, szString, true, dwStart, dwStop - dwStart);
			pModel->SetModifiedFlag(TRUE); // document has been modified
			pModel->SetTransModifiedFlag(TRUE); // transcription data has been modified
			pSeg->SetSelection(-1);
			m_advancedSelection.SelectFromPosition(this, PHONETIC, dwStart, true);
			RedrawGraphs(TRUE);
		}
	}
	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i != -1) && (m_apGraphs[i] != NULL)) {
		m_apGraphs[i]->ShowAnnotation(PHONETIC, TRUE, TRUE);
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditAddSyllable
/***************************************************************************/
void CSaView::OnUpdateEditAddSyllable(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument(); // get pointer to document
	BOOL bEnable = FALSE;

	CPhoneticSegment * pSeg = (CPhoneticSegment *)GetAnnotation(PHONETIC);

	if (pSeg->GetSelection() != -1) { // Phonetic Segment Selected
		int nSelection = pSeg->GetSelection();
		DWORD dwStart = pSeg->GetOffset(nSelection); // Start at current start
		DWORD dwMaxStop;
		DWORD dwStop;

		dwMaxStop = dwStart + pSeg->GetDuration(nSelection) - pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME);

		if (pModel->Is16Bit()) {          // SDM 1.5Test8.2
			dwMaxStop = dwMaxStop & ~1; // Round down
		}

		// Snap Start Position
		dwStart = pModel->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);

		dwStop = (dwStart + pModel->GetBytesFromTime(ADD_SYLLABLE_TIME));

		if (pModel->Is16Bit()) {          // SDM 1.5Test8.2
			dwStop = (dwStop + 1) & ~1; // Round up
		}

		dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStop, pModel->GetDataSize(), SNAP_RIGHT);

		if (dwStop <= dwMaxStop) { // enough room
			bEnable = TRUE;
		}
	}
	pCmdUI->Enable(bEnable);
}

//SDM 1.5Test11.3
/***************************************************************************/
// CSaView::OnEditAutoAdd WAT macro
/***************************************************************************/
void CSaView::OnEditAutoAdd() {

	DWORD start = GetStartCursorPosition();
	DWORD stop = GetStopCursorPosition();

	TRACE("Running WAT macro\n");
	OnEditAddPhonetic();
	OnEditAddGloss();
	OnEditAddPhraseL1();
	DWORD save = GetStartCursorPosition();
	OnEditAddAutoPhraseL2();

	start = GetStartCursorPosition();
	stop = GetStopCursorPosition();
	if (stop < start) {
		SetStopCursorPosition(start + 1);
	}

	m_advancedSelection.SelectFromPosition(this, MUSIC_PL1, save, true);
}

//SDM 1.5Test11.3
/***************************************************************************
* CSaView::OnEditAutoAddStorySection Story macro
*
* NOTE: If the start cursor is not in the correct position, we need
* to move it to the end of the last segment before the stop cursor
***************************************************************************/
void CSaView::OnEditAutoAddStorySection() {

	DWORD stop = GetStopCursorPosition();

	// move the start cursor to the end of the last phonetic segment
	CSaDoc * pModel = GetDocument();  // get pointer to document
	int nSelection = pModel->GetLastSegmentBeforePosition(PHONETIC, stop);
	DWORD offset = 0;
	if (nSelection != -1) {
		CPhoneticSegment * pSeg = (CPhoneticSegment *)GetAnnotation(PHONETIC);
		offset = pSeg->GetStop(nSelection);
	}
	SetCursorPosition(START_CURSOR, offset);
	DWORD start = GetStartCursorPosition();

	TRACE("Running Story Section macro\n");
	OnEditAddPhonetic();
	OnEditAddGloss();
	OnEditAddPhraseL1();
	DWORD save = GetStartCursorPosition();
	OnEditAddAutoPhraseL2();

	start = GetStartCursorPosition();
	stop = GetStopCursorPosition();
	if (stop < start) {
		SetStopCursorPosition(start + 1);
	}

	m_advancedSelection.SelectFromPosition(this, MUSIC_PL1, save, true);
}

BOOL CSaView::IsPhoneticOverlapping(bool story) {
	DWORD start = GetStartCursorPosition();
	DWORD stop = GetStopCursorPosition();
	// Check if either the Begin cursor or the End cursor are inside of any existing phonetic segment.
	// If either of them are, then that would result in an invalid new segment, so again do nothing.
	CPhoneticSegment * pSeg = (CPhoneticSegment *)GetAnnotation(PHONETIC);
	for (int i = 0; i < pSeg->GetOffsetSize(); i++) {
		DWORD begin = pSeg->GetOffset(i);
		DWORD end = pSeg->GetStop(i);
		// for story mode, only worry about the stop cursor.
		// the start cursor will be automatically advanced
		if (story) {
			if ((begin <= stop) && (stop <= end)) {
				return TRUE;
			}
		} else {
			if (((begin <= start) && (start <= end)) || ((begin <= stop) && (stop <= end))) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CSaView::AllowAutoAdd(bool story) {
	// check the phonetic location
	if (!AllowEditAdd(story)) {
		TRACE("edit-add not allowed\n");
		return FALSE;
	}

	if (IsPhoneticOverlapping(story)) {
		TRACE("phonetic overlap\n");
		return FALSE;
	}

	// no checks for gloss at this point...

	// check the PL1 location
	if (!AllowAddPhrase(MUSIC_PL1, story)) {
		TRACE("not allowed to add PL1\n");
		return FALSE;
	}

	// check the PL2 location
	if (!AllowAddPhrase(MUSIC_PL2, story)) {
		TRACE("not allowed to add PL2\n");
		return FALSE;
	}

	return TRUE;
}

//SDM 1.5Test11.3
/***************************************************************************/
// CSaView::OnEditAutoAdd WAT macro
/***************************************************************************/
void CSaView::OnUpdateEditAutoAdd(CCmdUI * pCmdUI) {
	pCmdUI->Enable(AllowAutoAdd(false));
}

/***************************************************************************/
// CSaView::OnEditAutoAddStorySection Story Section macro
/***************************************************************************/
void CSaView::OnUpdateEditAutoAddStorySection(CCmdUI * pCmdUI) {
	pCmdUI->Enable(AllowAutoAdd(true));
}

//SDM 1.5Test11.3
/***************************************************************************/
// CSaView::OnEditAddPhonetic
// Add Phonetic Segment
/***************************************************************************/
void CSaView::OnEditAddPhonetic() {

	CSaDoc * pModel = (CSaDoc *)GetDocument();
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)GetAnnotation(PHONETIC);
	CGlossSegment * pGloss = (CGlossSegment *)GetAnnotation(GLOSS);
	CGlossNatSegment * pGlossNat = (CGlossNatSegment *)GetAnnotation(GLOSS_NAT);
	CReferenceSegment * pReference = (CReferenceSegment *)GetAnnotation(REFERENCE);

	pModel->CheckPoint();
	//Fill new segment with default character
	CSaString szString = SEGMENT_DEFAULT_CHAR;

	int nInsertAt = pPhonetic->CheckPosition(pModel, GetStartCursorPosition(), GetStopCursorPosition(), CSegment::MODE_ADD);
	if (nInsertAt != -1) {
		int nPrevious = pPhonetic->GetPrevious(nInsertAt);
		if ((nPrevious != -1) &&
			(pPhonetic->GetStop(nPrevious) + pModel->GetBytesFromTime(MAX_ADD_JOIN_TIME) > GetStartCursorPosition())) {
			pPhonetic->Adjust(pModel, nPrevious, pPhonetic->GetOffset(nPrevious), GetStartCursorPosition() - pPhonetic->GetOffset(nPrevious), false);
		}

		int nNext = -1;
		if (nInsertAt > 0) {
			nNext = pPhonetic->GetNext(nInsertAt - 1);
		} else if (!pPhonetic->IsEmpty()) {
			nNext = nInsertAt;
		}
		if ((nNext != -1) &&
			(pPhonetic->GetOffset(nNext) < GetStopCursorPosition() + pModel->GetBytesFromTime(MAX_ADD_JOIN_TIME))) {
			pPhonetic->Adjust(pModel, nNext, GetStopCursorPosition(), pPhonetic->GetStop(nNext) - GetStopCursorPosition(), false);
		}

		pPhonetic->AddAt(pModel, nInsertAt, GetStartCursorPosition(), GetStopCursorPosition() - GetStartCursorPosition());

		// Adjust Gloss
		if ((!pGloss->IsEmpty()) && (pPhonetic->GetPrevious(nInsertAt))) {
			int nIndex = pGloss->FindStop(pPhonetic->GetStop(pPhonetic->GetPrevious(nInsertAt)));
			if (nIndex != -1) {
				pGloss->Adjust(pModel, nIndex, pGloss->GetOffset(nIndex), pGloss->CalculateDuration(pModel, nIndex), false);
				pGlossNat->Adjust(pModel, nIndex, pGlossNat->GetOffset(nIndex), pGlossNat->CalculateDuration(pModel, nIndex), false);
				pReference->Adjust(pModel, nIndex, pReference->GetOffset(nIndex), pReference->CalculateDuration(pModel, nIndex), false);
			}
		}

		pModel->SetModifiedFlag(TRUE); // document has been modified
		pModel->SetTransModifiedFlag(TRUE); // transcription data has been modified
		RedrawGraphs(TRUE);
		pPhonetic->SetSelection(-1);
		m_advancedSelection.SelectFromPosition(this, PHONETIC, GetStartCursorPosition(), true);
	} else {
		// Can we insert after selected segment
		if (pPhonetic->GetSelection() != -1) {
			// Phonetic Segment Selected
			int nSelection = pPhonetic->GetSelection();
			DWORD dwStart = pPhonetic->GetStop(nSelection); // Start at current stop
			DWORD dwMaxStop;
			DWORD dwStop;

			if (pPhonetic->GetNext(nSelection) == -1) { // Last Selection
				dwMaxStop = pModel->GetDataSize();
			} else { // Fit before next
				dwMaxStop = pPhonetic->GetOffset(pPhonetic->GetNext(nSelection));
			}

			// Snap Start Position
			dwStart = pModel->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);
			dwStop = (dwStart + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));

			if (pModel->Is16Bit()) {          // SDM 1.5Test8.2
				dwStop = (dwStop + 1) & ~1; // Round up
			}

			if (pPhonetic->GetNext(nSelection) != -1) {
				dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStop, pModel->GetDataSize(), SNAP_RIGHT);
			}

			if (dwStop <= dwMaxStop) { // enough room
				dwStop = dwStart + pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME);

				if (pModel->Is16Bit()) {          // SDM 1.5Test8.2
					dwStop = (dwStop + 1) & ~1; // Round up
				}

				if (dwStop > dwMaxStop) {
					dwStop = dwMaxStop;
				} else
					// Snap Stop Position
				{
					dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStart, dwMaxStop, SNAP_LEFT);
				}

				nInsertAt = pPhonetic->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_ADD);
				ASSERT(nInsertAt >= 0);
				pPhonetic->AddAt(pModel, nInsertAt, dwStart, dwStop - dwStart);

				// Adjust Gloss
				if ((!pGloss->IsEmpty()) && (pPhonetic->GetPrevious(nInsertAt))) {
					int nPrevious = pPhonetic->GetPrevious(nInsertAt);
					int nIndex = pGloss->FindStop(pPhonetic->GetStop(nPrevious));
					if (nIndex != -1) {
						pGloss->Adjust(pModel, nIndex, pGloss->GetOffset(nIndex), pGloss->CalculateDuration(pModel, nIndex), false);
						pGlossNat->Adjust(pModel, nIndex, pGlossNat->GetOffset(nIndex), pGlossNat->CalculateDuration(pModel, nIndex), false);
						pReference->Adjust(pModel, nIndex, pReference->GetOffset(nIndex), pReference->CalculateDuration(pModel, nIndex), false);
					}
				}

				pModel->SetModifiedFlag(TRUE); // document has been modified
				pModel->SetTransModifiedFlag(TRUE); // transcription data has been modified
				RedrawGraphs(TRUE);
				pPhonetic->SetSelection(-1);
				m_advancedSelection.SelectFromPosition(this, PHONETIC, dwStart, true);
			}
		}
	}

	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i != -1) && (m_apGraphs[i] != NULL)) {
		m_apGraphs[i]->ShowAnnotation(PHONETIC, TRUE, TRUE);
	}
}

/**
* Check and see if we are able to add a new segment
* @param story true if we are performing a check for adding a story section
*
* NOTE: Story sections can be automatically added to the end of the last phonetic
* section
* NOTE: Be careful not to change cursor positions in this code!
*/
BOOL CSaView::AllowEditAdd(bool story) {

	CSaDoc * pModel = GetDocument(); // get pointer to document
	CPhoneticSegment * pSeg = (CPhoneticSegment *)GetAnnotation(PHONETIC);
	DWORD startCursor = GetStartCursorPosition();
	DWORD stopCursor = GetStopCursorPosition();
	int nInsertAt = pSeg->CheckPosition(pModel, startCursor, stopCursor, CSegment::MODE_ADD);
	if (nInsertAt != -1) {
		m_advancedSelection.Update(this);
		int nLoop = m_advancedSelection.GetSelectionIndex();
		if (nLoop == -1) {
			return TRUE;
		}
		return FALSE;
	}

	int nSelection = pSeg->GetSelection();
	if (nSelection == -1) {           // Phonetic Segment not selected
		if (story) {
			nSelection = pModel->GetLastSegmentBeforePosition(PHONETIC, stopCursor);
		} else {
			return FALSE;
		}
	}

	DWORD dwStart = pSeg->GetStop(nSelection);  // Start at current stop
	DWORD dwMaxStop = 0;
	if (pSeg->GetNext(nSelection) == -1) {      // Last Selection
		dwMaxStop = pModel->GetDataSize();
	} else { // Fit before next
		dwMaxStop = pSeg->GetOffset(pSeg->GetNext(nSelection));
	}

	// Snap Start Position
	dwStart = pModel->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);;
	DWORD dwStop = (dwStart + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));
	if (pModel->Is16Bit()) {          // SDM 1.5Test8.2
		dwStop = (dwStop + 1) & ~1; // Round up
	}

	if (pSeg->GetNext(nSelection) != -1) {
		dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStop, pModel->GetDataSize(), SNAP_RIGHT);
	}

	if (dwStop > dwMaxStop) { // not enough room
		return FALSE;
	}

	dwStop = dwStart + pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME);
	if (pModel->Is16Bit()) { // SDM 1.5Test8.2
		dwStop = (dwStop + 1) & ~1; // Round up
	}

	if (dwStop > dwMaxStop) {
		dwStop = dwMaxStop;
	} else {
		// Snap Stop Position
		dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStart, dwMaxStop, SNAP_LEFT);
	}

	nInsertAt = pSeg->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_ADD);
	if (nInsertAt >= 0) {
		return TRUE;
	}
	return FALSE;
}

/***************************************************************************/
// CSaView::OnUpdateEditAddPhonetic
/***************************************************************************/
void CSaView::OnUpdateEditAddPhonetic(CCmdUI * pCmdUI) {
	pCmdUI->Enable(AllowEditAdd(false));
}

/***************************************************************************/
// CSaView::OnEditAddPhrase
/***************************************************************************/
void CSaView::OnEditAddPhrase(CMusicPhraseSegment * pSeg) {
	CSaDoc * pModel = (CSaDoc *)GetDocument();

	pModel->CheckPoint();
	//Fill new segment with default character
	CSaString szString = SEGMENT_DEFAULT_CHAR;

	int nInsertAt = pSeg->CheckPosition(pModel, GetStartCursorPosition(), GetStopCursorPosition(), CSegment::MODE_ADD);
	if (nInsertAt != -1) {
		int nPrevious = pSeg->GetPrevious(nInsertAt);
		if (nPrevious != -1) {
			// is the previous segment+jointime overlapping our start?
			if (pSeg->GetStop(nPrevious) + pModel->GetBytesFromTime(MAX_ADD_JOIN_TIME) > GetStartCursorPosition()) { // SDM 1.5Test10.2
				pSeg->Adjust(pModel, nPrevious, pSeg->GetOffset(nPrevious), GetStartCursorPosition() - pSeg->GetOffset(nPrevious), false);
			}
		}

		if (pSeg->GetOffsetSize() > 0) {
			int nNext = pSeg->GetNext(nInsertAt);
			if (nNext != -1) {
				CURSORPOS stopPos = GetStopCursorPosition();
				DWORD byteCount = pModel->GetBytesFromTime(MAX_ADD_JOIN_TIME);
				DWORD nextOffset = pSeg->GetOffset(nNext);
				if (nextOffset < (stopPos + byteCount)) { // SDM 1.5Test10.2
					DWORD stop = pSeg->GetStop(nNext);
					CURSORPOS stopPos2 = GetStopCursorPosition();
					pSeg->Adjust(pModel, nNext, stopPos2, stop - stopPos2, false);
				}
			}
		}
		pSeg->Insert(nInsertAt, szString, false, GetStartCursorPosition(), GetStopCursorPosition() - GetStartCursorPosition());
		// document has been modified
		pModel->SetModifiedFlag(TRUE);
		// transcription data has been modified
		pModel->SetTransModifiedFlag(TRUE);
		RedrawGraphs(TRUE);
		pSeg->SetSelection(-1);
		m_advancedSelection.SelectFromPosition(this, pSeg->GetAnnotationIndex(), GetStartCursorPosition(), true);
	} else {
		// Can we insert after selected segment
		if (pSeg->GetSelection() != -1) {
			// Phonetic Segment Selected
			int nSelection = pSeg->GetSelection();
			// Start at current stop
			DWORD dwStart = pSeg->GetStop(nSelection);
			DWORD dwMaxStop;
			DWORD dwStop;

			if (pSeg->GetNext(nSelection) == -1) { // Last Selection
				dwMaxStop = pModel->GetDataSize();
			} else { // Fit before next
				dwMaxStop = pSeg->GetOffset(pSeg->GetNext(nSelection));
			}

			// Snap Start Position
			dwStart = pModel->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);

			dwStop = (dwStart + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));

			if (pModel->Is16Bit()) { // SDM 1.5Test8.2
				dwStop = (dwStop + 1) & ~1; // Round up
			}

			if (pSeg->GetNext(nSelection) != -1) {
				dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStop, pModel->GetDataSize(), SNAP_RIGHT);
			}

			if (dwStop <= dwMaxStop) { // enough room
				dwStop = dwStart + pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME);

				if (pModel->Is16Bit()) { // SDM 1.5Test8.2
					dwStop = (dwStop + 1) & ~1; // Round up
				}

				if (dwStop > dwMaxStop) {
					dwStop = dwMaxStop;
				} else
					// Snap Stop Position
				{
					dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStart, dwMaxStop, SNAP_LEFT);
				}

				nInsertAt = pSeg->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_ADD);
				pSeg->Insert(nInsertAt, szString, true, dwStart, dwStop - dwStart);
				pModel->SetModifiedFlag(TRUE); // document has been modified
				pModel->SetTransModifiedFlag(TRUE); // transcription data has been modified
				RedrawGraphs(TRUE);
				pSeg->SetSelection(-1);
				m_advancedSelection.SelectFromPosition(this, pSeg->GetAnnotationIndex(), dwStart, true);
			}
		}
	}

	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i != -1) && (m_apGraphs[i] != NULL)) {
		m_apGraphs[i]->ShowAnnotation(pSeg->GetAnnotationIndex(), TRUE, TRUE);
	}
}

BOOL CSaView::AllowAddPhrase(EAnnotation annot, bool story) {
	CSaDoc * pModel = GetDocument(); // get pointer to document
	CMusicPhraseSegment * pSeg = (CMusicPhraseSegment *)GetAnnotation(annot);
	DWORD startCursor = GetStartCursorPosition();
	DWORD stopCursor = GetStopCursorPosition();
	int nInsertAt = pSeg->CheckPosition(pModel, startCursor, stopCursor, CSegment::MODE_ADD);
	if (nInsertAt != -1) {
		return TRUE;
	}

	int nSelection = pSeg->GetSelection();
	if (nSelection == -1) {           // no segment selected
		if (story) {
			nSelection = pModel->GetLastSegmentBeforePosition(PHONETIC, stopCursor);
		} else {
			return FALSE;
		}
	}

	DWORD dwStart = pSeg->GetStop(nSelection);  // Start at current stop
	DWORD dwMaxStop = 0;

	if (pSeg->GetNext(nSelection) == -1) {      // Last Selection
		dwMaxStop = pModel->GetDataSize();
	} else { // Fit before next
		dwMaxStop = pSeg->GetOffset(pSeg->GetNext(nSelection));
	}

	// Snap Start Position
	dwStart = pModel->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);

	DWORD dwStop = (dwStart + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));
	if (pModel->Is16Bit()) { // SDM 1.5Test8.2
		dwStop = (dwStop + 1) & ~1; // Round up
	}

	if (pSeg->GetNext(nSelection) != -1) {
		dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStop, pModel->GetDataSize(), SNAP_RIGHT);
	}

	if (dwStop <= dwMaxStop) { // enough room
		return TRUE;
	}

	return FALSE;
}

/***************************************************************************/
// CSaView::OnUpdateEditAddPhrase
/***************************************************************************/
void CSaView::OnUpdateEditAddPhrase(CCmdUI * pCmdUI, EAnnotation annot) {
	pCmdUI->Enable(AllowAddPhrase(annot, false));
}

void CSaView::OnEditAddAutoPhraseL2() {
	CMusicPhraseSegment * pSeg = (CMusicPhraseSegment *)GetAnnotation(MUSIC_PL2);

	CSaDoc * pModel = (CSaDoc *)GetDocument();
	pModel->CheckPoint();
	CSaString szString = SEGMENT_DEFAULT_CHAR; //Fill new segment with default character

	if (pSeg->IsEmpty()) {
		// if there are no segment yet, add at the beginning of the file
		SetStartCursorPosition(0);
	} else {
		// there are segments. // get the last one and add after it.
		int size = pSeg->GetOffsetSize();

		// determine if we are adding beyond the end of the last segment, or if we are in between two segments...
		DWORD newStart = GetStartCursorPosition();
		DWORD newStop = GetStopCursorPosition();
		ASSERT(newStart < newStop);

		bool found = false;
		for (int i = 0; i < size; i++) {
			// if there's only one, where are we?
			DWORD thisStart = pSeg->GetOffset(i);
			DWORD thisStop = pSeg->GetStop(i);
			// are we after?
			if (newStart > thisStop) {
				continue;
			}

			// are we before?
			if (newStop <= thisStart) {
				// it's before or overlapping
				if (i > 0) {
					//place after previous segment
					SetStartCursorPosition(pSeg->GetStop(i - 1));
					TRACE("Adding PL2 after previous segment\n");
					found = true;
					break;
				} else {
					// place at beginning of file
					SetStartCursorPosition(0);
					TRACE("Adding PL2 at beginning of file\n");
					found = true;
					break;
				}
			} else {
				// we are overlapping at end
				if ((newStart == thisStop) && (newStop > thisStop)) {
					// just insert as normal
					SetStartCursorPosition(thisStop);
					TRACE("Adding PL2 after previous segment\n");
					found = true;
					break;
				}
				// we are overlapping in some way...
				else if ((newStart<thisStart) && (newStop>thisStart) && (newStop < thisStop)) {
					// we are overlapping beginning
					SetStartCursorPosition(newStart);
					found = true;
					break;
				} else if ((newStart > thisStart) && (newStart<thisStop) && (newStop>thisStop)) {
					// we are overlapping at end
					// we are overlapping beginning
					SetStartCursorPosition(thisStop);
					found = true;
					break;
				} else {
				}
				// it's overlapping
				// leave the position alone - it will default in the following code and
				// place the segment after the overlapping one.
				found = true;
				break;
			}
		}
		if (!found) {
			// set at end
			SetStartCursorPosition(pSeg->GetStop(size - 1));
			TRACE("Adding PL2 at end of file\n");
		}
	}

	int nInsertAt = pSeg->CheckPosition(pModel, GetStartCursorPosition(), GetStopCursorPosition(), CSegment::MODE_ADD);
	if (nInsertAt != -1) {
		int nPrevious = pSeg->GetPrevious(nInsertAt);
		if (nPrevious != -1) {
			// SDM 1.5Test10.2
			if (pSeg->GetStop(nPrevious) + pModel->GetBytesFromTime(MAX_ADD_JOIN_TIME) > GetStartCursorPosition()) {
				pSeg->Adjust(pModel, nPrevious, pSeg->GetOffset(nPrevious), GetStartCursorPosition() - pSeg->GetOffset(nPrevious), false);
			}
		}
		int nNext = -1;
		if (nInsertAt > 0) {
			nNext = pSeg->GetNext(nInsertAt - 1);
		} else if (!pSeg->IsEmpty()) {
			nNext = nInsertAt;
		}
		if (nNext != -1) {
			// SDM 1.5Test10.2
			if (pSeg->GetOffset(nNext) < GetStopCursorPosition() + pModel->GetBytesFromTime(MAX_ADD_JOIN_TIME)) {
				pSeg->Adjust(pModel, nNext, GetStopCursorPosition(), pSeg->GetStop(nNext) - GetStopCursorPosition(), false);
			}
		}
		pSeg->Insert(nInsertAt, szString, false, GetStartCursorPosition(), GetStopCursorPosition() - GetStartCursorPosition());
		pModel->SetModifiedFlag(TRUE); // document has been modified
		pModel->SetTransModifiedFlag(TRUE); // transcription data has been modified
		RedrawGraphs(TRUE);
		pSeg->SetSelection(-1);
		m_advancedSelection.SelectFromPosition(this, pSeg->GetAnnotationIndex(), GetStartCursorPosition(), true);
	} else { // Can we insert after selected segment?
		if (pSeg->GetSelection() != -1) { // Phonetic Segment Selected
			int nSelection = pSeg->GetSelection();
			DWORD dwStart = pSeg->GetStop(nSelection); // Start at current stop
			DWORD dwMaxStop;
			DWORD dwStop;

			if (pSeg->GetNext(nSelection) == -1) { // Last Selection
				dwMaxStop = pModel->GetDataSize();
			} else { // Fit before next
				dwMaxStop = pSeg->GetOffset(pSeg->GetNext(nSelection));
			}

			// Snap Start Position
			dwStart = pModel->SnapCursor(START_CURSOR, dwStart, dwStart, dwMaxStop, SNAP_RIGHT);

			dwStop = (dwStart + pModel->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));

			if (pModel->Is16Bit()) { // SDM 1.5Test8.2
				dwStop = (dwStop + 1) & ~1; // Round up
			}

			if (pSeg->GetNext(nSelection) != -1) {
				dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStop, pModel->GetDataSize(), SNAP_RIGHT);
			}

			if (dwStop <= dwMaxStop) { // enough room
				dwStop = dwStart + pModel->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME);

				if (pModel->Is16Bit()) { // SDM 1.5Test8.2
					dwStop = (dwStop + 1) & ~1; // Round up
				}

				if (dwStop > dwMaxStop) {
					dwStop = dwMaxStop;
				} else
					// Snap Stop Position
				{
					dwStop = pModel->SnapCursor(STOP_CURSOR, dwStop, dwStart, dwMaxStop, SNAP_LEFT);
				}

				nInsertAt = pSeg->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_ADD);
				pSeg->Insert(nInsertAt, szString, true, dwStart, dwStop - dwStart);
				pModel->SetModifiedFlag(TRUE); // document has been modified
				pModel->SetTransModifiedFlag(TRUE); // transcription data has been modified
				RedrawGraphs(TRUE);
				pSeg->SetSelection(-1);
				m_advancedSelection.SelectFromPosition(this, pSeg->GetAnnotationIndex(), dwStart, true);
			}
		}
	}

	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i != -1) && (m_apGraphs[i] != NULL)) {
		m_apGraphs[i]->ShowAnnotation(pSeg->GetAnnotationIndex(), TRUE, TRUE);
	}
}

void CSaView::OnUpdateEditAddAutoPhraseL2(CCmdUI * pCmdUI) {
	pCmdUI->Enable(AllowAddPhrase(MUSIC_PL2, false));
}

void CSaView::OnEditAddPhraseL1() {
	OnEditAddPhrase((CMusicPhraseSegment *)GetAnnotation(MUSIC_PL1));
}

void CSaView::OnEditAddPhraseL2() {
	OnEditAddPhrase((CMusicPhraseSegment *)GetAnnotation(MUSIC_PL2));
}

void CSaView::OnEditAddPhraseL3() {
	OnEditAddPhrase((CMusicPhraseSegment *)GetAnnotation(MUSIC_PL3));
}

void CSaView::OnEditAddPhraseL4() {
	OnEditAddPhrase((CMusicPhraseSegment *)GetAnnotation(MUSIC_PL4));
}

void CSaView::OnUpdateEditAddPhraseL1(CCmdUI * pCmdUI) {
	OnUpdateEditAddPhrase(pCmdUI, MUSIC_PL1);
}

void CSaView::OnUpdateEditAddPhraseL2(CCmdUI * pCmdUI) {
	OnUpdateEditAddPhrase(pCmdUI, MUSIC_PL2);
}

void CSaView::OnUpdateEditAddPhraseL3(CCmdUI * pCmdUI) {
	OnUpdateEditAddPhrase(pCmdUI, MUSIC_PL3);
}

void CSaView::OnUpdateEditAddPhraseL4(CCmdUI * pCmdUI) {
	OnUpdateEditAddPhrase(pCmdUI, MUSIC_PL4);
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnEditAddGloss
// Add Gloss Word Segment
/***************************************************************************/
void CSaView::OnEditAddGloss() {
	EditAddGloss(false);
}

void CSaView::OnEditAddMarkup() {

	DWORD start = GetStartCursorPosition();

	OnEditAddPhonetic();
	OnEditAddGloss();

	CSaDoc * pModel = GetDocument();  // get pointer to document
	pModel->SetModifiedFlag(TRUE); // document has been modified
	pModel->SetTransModifiedFlag(TRUE); // transcription data has been modified
	RedrawGraphs(TRUE);
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)GetAnnotation(PHONETIC);
	pPhonetic->SetSelection(-1);
	m_advancedSelection.SelectFromPosition(this, PHONETIC, start, true);

}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnEditAddBookmark
// Add Gloss Bookmark Segment
/***************************************************************************/
void CSaView::OnEditAddBookmark() {
	EditAddGloss(true);
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::EditAddGloss
// Add Gloss Segment
/***************************************************************************/
void CSaView::EditAddGloss(bool bDelimiter) {

	CSaDoc * pModel = (CSaDoc *)GetDocument();    // get pointer to document
	CSaString szString = "";                    //Fill new segment with default character
	CSaString szEmpty = "";

	CGlossSegment * pGloss = (CGlossSegment *)GetAnnotation(GLOSS);
	CReferenceSegment * pReference = (CReferenceSegment *)GetAnnotation(REFERENCE);
	CGlossNatSegment * pGlossNat = (CGlossNatSegment *)GetAnnotation(GLOSS_NAT);

	DWORD dwStartR = GetStartCursorPosition();
	DWORD dwStopR = GetStopCursorPosition();

	int nInsertAt = pGloss->CheckPosition(pModel, dwStartR, dwStopR, CSegment::MODE_ADD);
	if (nInsertAt != -1) {
		DWORD dwStart = 0;
		pGloss->AdjustCursorsToMaster(pModel, FALSE, &dwStart);
		// add a segment
		pGloss->Add(pModel, this, dwStart, szString, bDelimiter, true);
		pReference->Add(pModel, this, dwStart, szEmpty, bDelimiter, true);
		pGlossNat->Add(pModel, this, dwStart, szEmpty, bDelimiter, true);
		RedrawGraphs(FALSE);
	} else {
		//SDM 1.5Test8.2
		DWORD dwStart = GetStartCursorPosition();
		int nPos = pGloss->FindFromPosition(dwStart, TRUE);
		if (nPos == -1) {
			nPos = pGloss->FindFromPosition(dwStart, FALSE);
		} else {
			nPos++;
		}

		DWORD dwStop = 0;
		if ((nPos == -1) || (nPos >= pGloss->GetOffsetSize())) {
			dwStop = pModel->GetDataSize();
		} else {
			dwStop = pGloss->GetOffset(nPos);
		}

		nInsertAt = GetAnnotation(PHONETIC)->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_ADD);
		if (nInsertAt != -1) {
			SetStopCursorPosition(dwStop);
			OnEditAddPhonetic();
			
			pGloss->AdjustCursorsToMaster(pModel, FALSE, &dwStart);
			// add a segment
			pGloss->Add(pModel, this, dwStart, szString, bDelimiter, false);
			pReference->Add(pModel, this, dwStart, szEmpty, bDelimiter, true);
			pGlossNat->Add(pModel, this, dwStart, szEmpty, bDelimiter, true);
			RedrawGraphs(FALSE);

			int i = GetGraphIndexForIDD(IDD_RAWDATA);
			if ((i != -1) && (m_apGraphs[i])) {
				m_apGraphs[i]->ShowAnnotation(pGloss->GetAnnotationIndex(), TRUE, TRUE);
			}
		} else if (pGloss->GetSelection() != -1) { // Set Delimiter
			CSaString temp = GetSelectedAnnotationString();
			if (bDelimiter) {
				temp = TEXT_DELIMITER + temp.Mid(1);
			} else {
				temp = WORD_DELIMITER + temp.Mid(1);
			}
			m_advancedSelection.Update(this);
			m_advancedSelection.SetSelectedAnnotationString(this, temp, true, true);
		}
	}

	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i != -1) && (m_apGraphs[i] != NULL)) {
		m_apGraphs[i]->ShowAnnotation(pGloss->GetAnnotationIndex(), TRUE, TRUE);
	}
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnUpdateEditAddGloss
// Update handler for AddWord
/***************************************************************************/
void CSaView::OnUpdateEditAddGloss(CCmdUI * pCmdUI) {
	// get pointer to document
	CSaDoc * pModel = (CSaDoc *)GetDocument();

	CSegment * pSeg = GetAnnotation(GLOSS);
	if (pSeg->CheckPosition(pModel, GetStartCursorPosition(), GetStopCursorPosition(), CSegment::MODE_ADD) != -1) {
		// if we are completely within a segment we are good...
		pCmdUI->Enable(TRUE);
		return;
	}

	//SDM 1.5Test8.2
	DWORD dwStart = GetStartCursorPosition();
	int nPos = pSeg->FindFromPosition(dwStart, TRUE);
	if (nPos == -1) {
		// if there aren't any segments yet...
		if (pSeg->GetOffsetSize() == 0) {
			pCmdUI->Enable(TRUE);
			return;
		}

		// if we are outside and beyond the last segment we are done.
		if (dwStart > pSeg->GetStop(pSeg->GetOffsetSize() - 1)) {
			//TRACE("outside and beyond last segment\n");
			pCmdUI->Enable(FALSE);
			return;
		}

		// start cursor was not within a segment
		nPos = pSeg->FindFromPosition(dwStart, FALSE);
		DWORD dwStop = 0;
		// if we weren't within a segment or we are on the last segment
		if ((nPos == -1) || (nPos >= pSeg->GetOffsetSize())) {
			// stop will be at end of data
			dwStop = pModel->GetDataSize();
		} else {
			// stop is at end of selected segment
			dwStop = pSeg->GetOffset(nPos);
		}
		// if segments are within phonetic
		if (GetAnnotation(PHONETIC)->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_ADD) != -1) {
			pCmdUI->Enable(TRUE);
			return;
		}

		// if a gloss character is selected...
		if (pSeg->GetSelection() != -1) {
			// Set Delimiter
			CSaString szString = GetSelectedAnnotationString();
			if (szString[0] != WORD_DELIMITER) {
				pCmdUI->Enable(TRUE);
				return;
			}
		}

		pCmdUI->Enable(FALSE);
		return;
	}

	// at this point we were within a segment
	// point to the next segment
	nPos++;
	DWORD dwStop = 0;
	// if we weren't within a segment or we are on the last segment
	if (nPos >= pSeg->GetOffsetSize()) {
		// stop will be at end of data
		dwStop = pModel->GetDataSize();
	} else {
		// stop is at end of selected segment
		dwStop = pSeg->GetOffset(nPos);
	}

	// if segments are within phonetic
	if (GetAnnotation(PHONETIC)->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_ADD) != -1) {
		pCmdUI->Enable(TRUE);
		return;
	}

	// if a gloss character is selected...
	if (pSeg->GetSelection() != -1) {
		// Set Delimiter
		CSaString szString = GetSelectedAnnotationString();
		if (szString[0] != WORD_DELIMITER) {
			pCmdUI->Enable(TRUE);
			return;
		}
	}

	pCmdUI->Enable(FALSE);
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnUpdateEditAddGloss
// Update handler for AddWord
/***************************************************************************/
void CSaView::OnUpdateEditAddMarkup(CCmdUI * pCmdUI) {
	pCmdUI->Enable(AllowEditAdd(false));
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnUpdateEditAddBookmark Update handler for AddBookmark
/***************************************************************************/
void CSaView::OnUpdateEditAddBookmark(CCmdUI * pCmdUI) {

	BOOL bEnable = FALSE;
	CSaDoc * pModel = (CSaDoc *)GetDocument();

	CSegment * pSeg = GetAnnotation(GLOSS);
	int nInsertAt = pSeg->CheckPosition(pModel, GetStartCursorPosition(), GetStopCursorPosition(), CSegment::MODE_ADD);
	if (nInsertAt != -1) {
		bEnable = TRUE;
	} else {
		//SDM 1.5Test8.2
		DWORD dwStart = GetStartCursorPosition();

		int nPos = pSeg->FindFromPosition(dwStart, TRUE);
		if (nPos == -1) {
			nPos = pSeg->FindFromPosition(dwStart, FALSE);
		} else {
			nPos++;
		}

		DWORD dwStop;
		if ((nPos == -1) || (nPos >= pSeg->GetOffsetSize())) {
			dwStop = pModel->GetDataSize();
		} else {
			dwStop = pSeg->GetOffset(nPos);
		}

		nInsertAt = GetAnnotation(PHONETIC)->CheckPosition(pModel, dwStart, dwStop, CSegment::MODE_ADD);
		if (nInsertAt != -1) {
			bEnable = TRUE;
		} else if (pSeg->GetSelection() != -1) { // Set Delimiter
			CSaString szString = GetSelectedAnnotationString();
			if (szString[0] != TEXT_DELIMITER) {
				bEnable = TRUE;
			}
		}
	}
	pCmdUI->Enable(bEnable);
}

/***************************************************************************/
// CSaView::OnEditRemove
/***************************************************************************/
void CSaView::OnEditRemove() {
	RemoveSelectedAnnotation();
}

/**
* CSaView::OnUpdateEditSplit
**/
void CSaView::OnUpdateEditSplit(CCmdUI * pCmdUI) {
	CSegment * pSeg = FindSelectedAnnotation();
	pCmdUI->Enable((CanSplit(pSeg) ? TRUE : FALSE));
}

/**
* CSaView::OnEditSplit
**/
void CSaView::OnEditSplit() {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	EditSplit();
}

void CSaView::EditSplit() {

	CSaDoc * pModel = GetDocument();
	CSegment * pSeg = FindSelectedAnnotation();
	if (pSeg == NULL) {
		return;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		return;
	}
	if (!pSeg->Is(PHONETIC)) {
		return;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	pModel->SplitSegment(pPhonetic, sel, -1);
	int newsel = pSeg->GetNext(sel);
	DWORD newStart = pSeg->GetOffset(newsel);
	DWORD newStop = pSeg->GetStop(newsel);
	m_advancedSelection.DeselectAnnotations(this);
	pSeg->SetSelection(newsel);
	SetCursorPosition(START_CURSOR, newStart);
	SetCursorPosition(STOP_CURSOR, newStop);
	RedrawGraphs(TRUE, FALSE);
}

/**
* CSaView::OnUpdateEditSplitHere
**/
void CSaView::OnUpdateSplitHere(CCmdUI * pCmdUI) {
	TRACE("OnUpdateEditSplitHere\n");
	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pSeg = (CPhoneticSegment*)pModel->GetSegment(PHONETIC);
	BOOL enable = FALSE;
	DWORD position = CalculatePositionFromMouse();
	if (position != -1) {
		if (CanSplitAt(pModel, pSeg, position)) {
			enable = TRUE;
		}
	}
	pCmdUI->Enable(enable);
}

/**
* CSaView::OnEditSplitHere
**/
void CSaView::OnSplitHere() {
	DWORD position = CalculatePositionFromMouse();
	if (position == -1) {
		return;
	}
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	m_advancedSelection.SelectFromPosition(this, PHONETIC, position, false);
	EditSplitAt(position);
}

DWORD CSaView::EditSplitAt(DWORD position) {

	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pSeg = (CPhoneticSegment*)pModel->GetSegment(PHONETIC);
	if (pSeg == NULL) {
		return -1;
	}
	int sel = pSeg->FindWithin(position);
	if (sel == -1) {
		return -1;
	}
	pModel->SplitSegment(pSeg, sel, position);
	int newsel = pSeg->GetNext(sel);
	DWORD newStart = pSeg->GetOffset(newsel);
	DWORD newStop = pSeg->GetStop(newsel);
	m_advancedSelection.DeselectAnnotations(this);
	pSeg->SetSelection(newsel);
	SetCursorPosition(START_CURSOR, newStart);
	SetCursorPosition(STOP_CURSOR, newStop);
	RedrawGraphs(TRUE, FALSE);
	return newStart;
}

/***************************************************************************/
// CSaView::OnUpdateEditMerge
/***************************************************************************/
void CSaView::OnUpdateEditMerge(CCmdUI * pCmdUI) {
	CSegment * pSeg = FindSelectedAnnotation();
	pCmdUI->Enable((CanMerge(pSeg) ? TRUE : FALSE));
}

/***************************************************************************/
// CSaView::OnEditMerge
/***************************************************************************/
void CSaView::OnEditMerge() {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	EditMerge();
}

void CSaView::EditMerge() {

	CSaDoc * pModel = GetDocument();
	CSegment * pSeg = FindSelectedAnnotation();
	if (pSeg == NULL) {
		return;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		return;
	}

	// standard case
	if (!pSeg->Is(PHONETIC)) {
		return;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	pModel->MergeSegments(pPhonetic, sel);
	int newsel = pSeg->GetPrevious(sel);
	DWORD newStart = pSeg->GetOffset(newsel);
	DWORD newStop = pSeg->GetStop(newsel);
	m_advancedSelection.DeselectAnnotations(this);
	m_advancedSelection.SelectFromPosition(this, GLOSS, newStart, false);
	SetCursorPosition(START_CURSOR, newStart);
	SetCursorPosition(STOP_CURSOR, newStop);
	RedrawGraphs(TRUE, FALSE);
}

/**
* CSaView::OnUpdateEditMergeHere
**/
void CSaView::OnUpdateMergeHere(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pSeg = (CPhoneticSegment *)pModel->GetSegment(PHONETIC);
	BOOL enable = FALSE;
	DWORD position = CalculatePositionFromMouse();
	if (position != -1) {
		if (CanMergeAt(pModel, pSeg, position)) {
			enable = TRUE;
		}
	}
	pCmdUI->Enable(enable);
}

/**
* CSaView::OnEditMergeHere
**/
void CSaView::OnMergeHere() {
	DWORD position = CalculatePositionFromMouse();
	if (position == -1) {
		return;
	}
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	m_advancedSelection.SelectFromPosition(this, PHONETIC, position, false);
	EditMergeAt(position);
}

void CSaView::EditMergeAt(DWORD position) {

	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pSeg = (CPhoneticSegment*)pModel->GetSegment(PHONETIC);
	if (pSeg == NULL) {
		return;
	}
	int sel = pSeg->FindWithin(position);
	if (sel == -1) {
		return;
	}

	// standard case
	pModel->MergeSegments(pSeg, sel);
	int newsel = pSeg->GetPrevious(sel);
	DWORD newStart = pSeg->GetOffset(newsel);
	DWORD newStop = pSeg->GetStop(newsel);
	m_advancedSelection.DeselectAnnotations(this);
	pSeg->SetSelection(newsel);
	SetCursorPosition(START_CURSOR, newStart);
	SetCursorPosition(STOP_CURSOR, newStop);
	RedrawGraphs(TRUE, FALSE);
}

/**
* CSaView::OnUpdateEditMoveLeft
**/
void CSaView::OnUpdateEditMoveLeft(CCmdUI * pCmdUI) {
	CSegment * pSeg = FindSelectedAnnotation();
	pCmdUI->Enable((CanMoveDataLeft(pSeg, true)) ? TRUE : FALSE);
}

/**
* CSaView::OnEditMoveLeft
* The selection should be on the segment to be filled - a blank segment
**/
void CSaView::OnEditMoveLeft() {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	EditMoveLeft();
}

void CSaView::EditMoveLeft() {

	CSaDoc * pModel = GetDocument();
	CSegment * pSeg = FindSelectedAnnotation();
	if (pSeg == NULL) {
		TRACE("segment not found\n");
		return;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		TRACE("segment not selected\n");
		return;
	}
	// in combined mode we will be creating an
	if (AnySegmentHasData(pSeg, sel)) {
		TRACE("segment contains data\n");
		return;
	}

	if (!pSeg->Is(PHONETIC)) {
		TRACE("selected segment is not PHONETIC\n");
		return;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)pSeg;
	DWORD start = pPhonetic->GetOffset(sel);
	DWORD stop = pSeg->GetStop(sel);
	pModel->MoveDataLeft(start);
	m_advancedSelection.DeselectAnnotations(this);
	m_advancedSelection.SelectFromPosition(this, GLOSS, start, false);
	SetCursorPosition(START_CURSOR, start);
	SetCursorPosition(STOP_CURSOR, stop);
	RedrawGraphs(TRUE, FALSE);
}

/**
* CSaView::OnUpdateEditMoveLeftHere
**/
void CSaView::OnUpdateMoveLeftHere(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pSeg = (CPhoneticSegment *)pModel->GetSegment(PHONETIC);
	BOOL enable = FALSE;
	DWORD position = CalculatePositionFromMouse();
	if (position != -1) {
		if (CanMoveDataLeftAt(pModel, pSeg, position, true)) {
			enable = TRUE;
		}
	}
	pCmdUI->Enable(enable);
}

/**
* CSaView::OnEditMoveLeftHere
* The selection should be on the segment to be filled - a blank segment
**/
void CSaView::OnMoveLeftHere() {
	DWORD position = CalculatePositionFromMouse();
	if (position == -1) {
		return;
	}
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	m_advancedSelection.SelectFromPosition(this, PHONETIC, position, false);
	EditMoveLeftAt(pModel, position);
}

void CSaView::EditMoveLeftAt(CSaDoc * pModel, DWORD position) {

	CPhoneticSegment * pSeg = (CPhoneticSegment *)pModel->GetSegment(PHONETIC);
	if (pSeg == NULL) {
		TRACE("segment not found\n");
		return;
	}
	int sel = pSeg->FindWithin(position);
	if (sel == -1) {
		TRACE("cursor not in segment\n");
		return;
	}
	// in combined mode we will be creating an
	if (AnySegmentHasData(pSeg, sel)) {
		TRACE("segment contains data\n");
		return;
	}

	DWORD start = pSeg->GetOffset(sel);
	DWORD stop = pSeg->GetStop(sel);
	pModel->MoveDataLeft(start);
	m_advancedSelection.DeselectAnnotations(this);
	m_advancedSelection.SelectFromPosition(this, GLOSS, start, false);
	SetCursorPosition(START_CURSOR, start);
	SetCursorPosition(STOP_CURSOR, stop);
	RedrawGraphs(TRUE, FALSE);
}

/**
* CSaView::OnUpdateEditSplitMoveLeft
**/
void CSaView::OnUpdateEditSplitMoveLeft(CCmdUI * pCmdUI) {
	CSegment * pSeg = FindSelectedAnnotation();
	BOOL enable = FALSE;
	if (CanSplit(pSeg)) {
		if (CanMoveDataLeft(pSeg, false)) {
			enable = TRUE;
		}
	}
	pCmdUI->Enable(enable);
}

/**
* CSaView::OnEditSplitMoveLeft
**/
void CSaView::OnEditSplitMoveLeft() {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	EditSplit();
	EditMoveLeft();
}

/**
* CSaView::OnUpdateEditSplitMoveLeftHere
**/
void CSaView::OnUpdateSplitMoveLeftHere(CCmdUI * pCmdUI) {
	TRACE("OnUpdateEditSplitMoveLeftHere\n");
	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)pModel->GetSegment(PHONETIC);
	BOOL enable = FALSE;
	DWORD position = CalculatePositionFromMouse();
	if (position != -1) {
		if (CanSplitAt(pModel, pPhonetic, position)) {
			if (CanMoveDataLeftAt(pModel, pPhonetic, position, false)) {
				enable = TRUE;
			}
		}
	}
	pCmdUI->Enable(enable);
}

/**
* CSaView::OnEditSplitMoveLeftHere
**/
void CSaView::OnSplitMoveLeftHere() {
	DWORD position = CalculatePositionFromMouse();
	if (position == -1) {
		return;
	}
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	m_advancedSelection.SelectFromPosition(this, PHONETIC, position, false);
	DWORD newPosition = EditSplitAt(position);
	EditMoveLeftAt(pModel, newPosition + 1);
}

/**
* CSaView::OnUpdateEditMoveRight
**/
void CSaView::OnUpdateEditMoveRight(CCmdUI * pCmdUI) {
	CSegment * pSeg = FindSelectedAnnotation();
	pCmdUI->Enable(CanMoveDataRight(pSeg) ? TRUE : FALSE);
}

/**
* CSaView::OnEditMoveRight
**/
void CSaView::OnEditMoveRight() {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	EditMoveRight();
}

void CSaView::EditMoveRight() {
	CSaDoc * pModel = GetDocument();
	CSegment * pSeg = FindSelectedAnnotation();
	if (pSeg == NULL) {
		return;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		return;
	}

	// standard case
	if (!pSeg->Is(PHONETIC)) {
		return;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	DWORD start = pPhonetic->GetOffset(sel);
	pModel->MoveDataRight(start);
	start = pSeg->GetOffset(sel);
	DWORD stop = pSeg->GetStop(sel);
	m_advancedSelection.DeselectAnnotations(this);
	pSeg->SetSelection(sel);
	SetCursorPosition(START_CURSOR, start);
	SetCursorPosition(STOP_CURSOR, stop);
	RedrawGraphs(TRUE, FALSE);
}

/**
* CSaView::EditMoveRightNext
**/
void CSaView::EditMoveRightNext() {

	CSaDoc * pModel = GetDocument();
	CSegment * pSeg = FindSelectedAnnotation();
	if (pSeg == NULL) {
		return;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		return;
	}

	sel += 1;
	if (sel >= pSeg->GetOffsetSize()) {
		return;
	}

	// standard case
	if (!pSeg->Is(PHONETIC)) {
		return;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	DWORD start = pPhonetic->GetOffset(sel);
	pModel->MoveDataRight(start);
	start = pSeg->GetOffset(sel);
	DWORD stop = pSeg->GetStop(sel);
	m_advancedSelection.DeselectAnnotations(this);
	pSeg->SetSelection(sel);
	SetCursorPosition(START_CURSOR, start);
	SetCursorPosition(STOP_CURSOR, stop);
	RedrawGraphs(TRUE, FALSE);
}

/**
* CSaView::OnUpdateEditMoveRightHere
**/
void CSaView::OnUpdateMoveRightHere(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pSeg = (CPhoneticSegment *)pModel->GetSegment(PHONETIC);
	BOOL enable = FALSE;
	DWORD position = CalculatePositionFromMouse();
	if (position != -1) {
		if (CanMoveDataRightAt(pSeg, position)) {
			enable = TRUE;
		}
	}
	pCmdUI->Enable(enable);
}

/**
* CSaView::OnMoveRightHere
**/
void CSaView::OnMoveRightHere() {
	DWORD position = CalculatePositionFromMouse();
	if (position == -1) {
		return;
	}
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	m_advancedSelection.SelectFromPosition(this, PHONETIC, position, false);
	CPhoneticSegment * pSeg = (CPhoneticSegment*)pModel->GetSegment(PHONETIC);
	if (pSeg == NULL) {
		return;
	}
	int sel = pSeg->FindWithin(position);
	if (sel == -1) {
		return;
	}

	// standard case
	if (!pSeg->Is(PHONETIC)) {
		return;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	DWORD start = pPhonetic->GetOffset(sel);
	pModel->MoveDataRight(start);
	start = pSeg->GetOffset(sel);
	DWORD stop = pSeg->GetStop(sel);
	m_advancedSelection.DeselectAnnotations(this);
	pSeg->SetSelection(sel);
	SetCursorPosition(START_CURSOR, start);
	SetCursorPosition(STOP_CURSOR, stop);
	RedrawGraphs(TRUE, FALSE);
}

/***************************************************************************/
// CSaView::OnUpdateEditMoveRightMergeNext
/***************************************************************************/
void CSaView::OnUpdateEditMoveRightMergeNext(CCmdUI * pCmdUI) {
	CSegment * pSeg = FindSelectedAnnotation();
	bool a = CanMoveDataRightNext(pSeg);
	bool b = CanMergeNext(pSeg);
	TRACE("a=%d b=%d\n", a, b);
	pCmdUI->Enable((a&b) ? TRUE : FALSE);
}

/***************************************************************************/
// CSaView::OnEditMoveRightMergeNext
/***************************************************************************/
void CSaView::OnEditMoveRightMergeNext() {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	EditMoveRightNext();
	EditMerge();
}

/***************************************************************************/
// CSaView::OnUpdateEditMoveRightMergePrev
/***************************************************************************/
void CSaView::OnUpdateEditMoveRightMergePrev(CCmdUI * pCmdUI) {
	CSegment * pSeg = FindSelectedAnnotation();
	BOOL enable = FALSE;
	if (CanMoveDataRight(pSeg)) {
		if (CanMerge(pSeg)) {
			enable = TRUE;
		}
	}
	pCmdUI->Enable(enable);
}

/***************************************************************************/
// CSaView::OnEditMoveRightMergePrev
/***************************************************************************/
void CSaView::OnEditMoveRightMergePrev() {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	EditMoveRight();
	EditMerge();
}

/***************************************************************************/
// CSaView::OnUpdateEditRemove
/***************************************************************************/
void CSaView::OnUpdateEditRemove(CCmdUI * pCmdUI) {
	BOOL bEnable = FALSE;
	CSegment * pSeg = FindSelectedAnnotation();
	if ((pSeg != NULL) && (pSeg->GetSelection() != -1)) {
		bEnable = TRUE;
	}
	pCmdUI->Enable(bEnable);
}

void CSaView::OnUpdateMoveRightMergeNextHere(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pSeg = (CPhoneticSegment*)pModel->GetSegment(PHONETIC);
	BOOL enable = FALSE;
	DWORD position = CalculatePositionFromMouse();
	if (position != -1) {
		if (CanMoveDataRightNextAt(pSeg, position)) {
			if (CanMergeAtNext(pModel, pSeg, position)) {
				enable = TRUE;
			}
		}
	}
	pCmdUI->Enable(enable);
}

void CSaView::OnMoveRightMergeNextHere() {
	DWORD position = CalculatePositionFromMouse();
	if (position == -1) {
		return;
	}
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	// select the new annotation
	m_advancedSelection.SelectFromPosition(this, PHONETIC, position, false);
	EditMoveRightNext();
	EditMerge();
}

void CSaView::OnUpdateMoveRightMergePrevHere(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pSeg = (CPhoneticSegment*)pModel->GetSegment(PHONETIC);
	BOOL enable = FALSE;
	DWORD position = CalculatePositionFromMouse();
	if (position != -1) {
		if (CanMoveDataRightAt(pSeg, position)) {
			if (CanMergeAt(pModel, pSeg, position)) {
				enable = TRUE;
			}
		}
	}
	pCmdUI->Enable(enable);
}

void CSaView::OnMoveRightMergePrevHere() {
	DWORD position = CalculatePositionFromMouse();
	if (position == -1) {
		return;
	}
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	// select the new annotation
	m_advancedSelection.SelectFromPosition(this, PHONETIC, position, false);
	EditMoveRight();
	EditMerge();
}

//SDM 1.06.5
/***************************************************************************/
// CSaView::OnEditUp Move annotation selection one visible
// window up (or start at bottom left)
/***************************************************************************/
void CSaView::OnEditUp() {
	int nLoop;
	DWORD dwOffset;

	if (m_pFocusedGraph) { // needs to have a focused graph
		m_advancedSelection.Update(this);
		nLoop = m_advancedSelection.GetSelectionIndex();
		dwOffset = m_advancedSelection.GetSelectionStart();

		if (nLoop == -1) {
			// no selection yet, search for last annotation which is not empty
			for (nLoop = ANNOT_WND_NUMBER - 1; nLoop >= 0; nLoop--) {
				if (GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE) {
					dwOffset = GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->GetOffset(0);
					nLoop++;
					break;
				}
			}
			if (nLoop == -1) {
				return;    // no segments navigation irrelevant
			}
		} else { // SDM 1.5Test8.1 find window position from segment index
			for (int nIndex = 0; nIndex < ANNOT_WND_NUMBER; nIndex++) {
				if (CGraphWnd::m_anAnnWndOrder[nIndex] == nLoop) {
					nLoop = nIndex;
					break;
				}
			}
		}

		// search for next upper visible annotation window which is not empty
		for (nLoop--; nLoop >= 0; nLoop--) {
			if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop]) &&
				((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE) ||
					// SDM 1.5Test8.1
					((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS) && (CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) {
				// SDM 1.5Test8.1
				m_advancedSelection.SelectFromPosition(this, CGraphWnd::m_anAnnWndOrder[nLoop], dwOffset, true);
				if (m_advancedSelection.GetSelectionIndex() == -1) {
					m_advancedSelection.SelectFromPosition(this, CGraphWnd::m_anAnnWndOrder[nLoop], dwOffset, false);
				}

				if ((m_advancedSelection.GetSelectionStart() <= m_dwDataPosition) ||
					(m_advancedSelection.GetSelectionStop() >= (m_dwDataPosition + GetDataFrame()))) {
					// center selection in view
					ZoomIn(0, FALSE);
				}
				break;
			}
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditUp Able to move annotation selection one visible
// window up (or start at bottom left)
/***************************************************************************/
void CSaView::OnUpdateEditUp(CCmdUI * pCmdUI) {
	int nLoop;

	if (m_pFocusedGraph) { // needs to have a focused graph
		m_advancedSelection.Update(this);
		nLoop = m_advancedSelection.GetSelectionIndex();

		if (nLoop == -1) {
			// no selection yet, search for last annotation which is not empty
			for (nLoop = ANNOT_WND_NUMBER - 1; nLoop >= 0; nLoop--) {
				if (GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE) {
					nLoop++;
					break;
				}
			}
			if (nLoop == -1) {
				pCmdUI->Enable(FALSE);
				// no segments navigation irrelevant
				return;
			}
		} else {
			// SDM 1.5Test8.1 find window position from segment index
			for (int nIndex = 0; nIndex < ANNOT_WND_NUMBER; nIndex++) {
				if (CGraphWnd::m_anAnnWndOrder[nIndex] == nLoop) {
					nLoop = nIndex;
					break;
				}
			}
		}


		// search for next upper visible annotation window which is not empty
		for (nLoop--; nLoop >= 0; nLoop--) {
			if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop]) && ((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
				|| ((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS) && (CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) { // SDM 1.5Test8.1
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
void CSaView::OnEditDown() {

	// needs to have a focused graph
	if (m_pFocusedGraph != NULL) {
		m_advancedSelection.Update(this);
		int nLoop = m_advancedSelection.GetSelectionIndex();
		DWORD dwOffset = m_advancedSelection.GetSelectionStart();

		if (nLoop == -1) {
			// no selection yet, search for last annotation which is not empty
			for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
				if (GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE) {
					dwOffset = GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->GetOffset(0);
					nLoop--;
					break;
				}
			}
			if (nLoop == ANNOT_WND_NUMBER) {
				// no segments navigation irrelevant
				return;
			}
		} else {
			// SDM 1.5Test8.1 find window position from segment index
			for (int nIndex = 0; nIndex < ANNOT_WND_NUMBER; nIndex++) {
				if (CGraphWnd::m_anAnnWndOrder[nIndex] == nLoop) {
					nLoop = nIndex;
					break;
				}
			}
		}

		// search for next lower visible annotation window which is not empty
		for (nLoop++; nLoop < ANNOT_WND_NUMBER; nLoop++) {
			if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop]) &&
				((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE) ||
					// SDM 1.5Test8.1
					((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS) && (CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) {
				// SDM 1.5Test8.1
				m_advancedSelection.SelectFromPosition(this, CGraphWnd::m_anAnnWndOrder[nLoop], dwOffset, true);
				if (m_advancedSelection.GetSelectionIndex() == -1) {
					m_advancedSelection.SelectFromPosition(this, CGraphWnd::m_anAnnWndOrder[nLoop], dwOffset, false);
				}

				if ((m_advancedSelection.GetSelectionStart() <= m_dwDataPosition) ||
					(m_advancedSelection.GetSelectionStop() >= (m_dwDataPosition + GetDataFrame()))) {
					// center selection in view
					ZoomIn(0, FALSE);
				}
				break;
			}
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditDown Able to move annotation selection one visible
// window down (or start at top left)
/***************************************************************************/
void CSaView::OnUpdateEditDown(CCmdUI * pCmdUI) {

	// needs to have a focused graph
	if (m_pFocusedGraph != NULL) {
		m_advancedSelection.Update(this);
		int nLoop = m_advancedSelection.GetSelectionIndex();

		if (nLoop == -1) {
			// no selection yet, search for last annotation which is not empty
			for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
				if (GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE) {
					nLoop--;
					break;
				}
			}
			if (nLoop == ANNOT_WND_NUMBER) {
				pCmdUI->Enable(FALSE);
				// no segments navigation irrelevant
				return;
			}
		} else {
			// SDM 1.5Test8.1 find window position from segment index
			for (int nIndex = 0; nIndex < ANNOT_WND_NUMBER; nIndex++) {
				if (CGraphWnd::m_anAnnWndOrder[nIndex] == nLoop) {
					nLoop = nIndex;
					break;
				}
			}
		}

		// search for next lower visible annotation window which is not empty
		for (nLoop++; nLoop < ANNOT_WND_NUMBER; nLoop++) {
			EAnnotation type = CGraphWnd::m_anAnnWndOrder[nLoop];
			if (m_pFocusedGraph->HaveAnnotation(type) &&
				((GetAnnotation(type)->IsEmpty() == FALSE) ||
					((type != GLOSS) && (type != PHONETIC)))) {
				pCmdUI->Enable(TRUE);
				return;
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
void CSaView::OnEditPrevious() {
	int nLoop;

	if (m_pFocusedGraph) { // needs to have a focused graph
		m_advancedSelection.Update(this);
		nLoop = m_advancedSelection.GetSelectionIndex();
		// only work from visible selections
		if ((nLoop != -1) && !(m_pFocusedGraph->HaveAnnotation(nLoop))) {
			nLoop = -1;    // only work from visible selections
		}

		int nSelection;
		if (nLoop == -1) {
			// no selection yet, search for first visible annotation with segments
			for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
				if ((m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])) &&
					((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE) ||
						((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS) && (CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) { // SDM 1.5Test8.1
					nLoop = CGraphWnd::m_anAnnWndOrder[nLoop];
					break;
				}
			}
			if (nLoop >= ANNOT_WND_NUMBER) {
				return;    // no possibility found
			}
			// try to find selection within start cursor or left of it
			m_advancedSelection.SelectFromPosition(this, nLoop, GetStartCursorPosition(), true);
			if (m_advancedSelection.GetSelectionIndex() == -1) {
				m_advancedSelection.SelectFromPosition(this, nLoop, GetStartCursorPosition(), false);
			}
		} else {
			int MASTER = GetAnnotation(nLoop)->GetMasterIndex();
			if ((MASTER != -1) && (nLoop != GLOSS) && (nLoop != PHONETIC)) {
				CSegment * pMaster = GetAnnotation(MASTER);

				DWORD dwPosition = m_advancedSelection.GetSelectionStart();
				int nIndexMaster = pMaster->FindOffset(dwPosition); // Should always find...
				if (nIndexMaster == -1) {
					nIndexMaster = pMaster->FindFromPosition(dwPosition, TRUE);    // Better Coverage
				}

				if (nIndexMaster == -1) {
					return;
				}
				nIndexMaster = pMaster->GetPrevious(nIndexMaster);
				if (nIndexMaster == -1) {
					return;
				}
				m_advancedSelection.SelectFromStopPosition(this, nLoop, pMaster->GetStop(nIndexMaster), true);
				if (m_advancedSelection.GetSelectionIndex() == -1) {
					m_advancedSelection.SelectFromStopPosition(this, nLoop, pMaster->GetStop(nIndexMaster), false);
				}
			} else {
				DWORD dwStop;
				DWORD dwStart;

				nSelection = GetAnnotation(nLoop)->GetSelection();
				if (nSelection == -1) {
					dwStart = m_advancedSelection.GetSelectionStart();
					nSelection = GetAnnotation(nLoop)->FindFromPosition(dwStart, FALSE);
					if (nSelection == -1) {
						return;
					}

					if (GetAnnotation(nLoop)->GetOffset(nSelection) < dwStart) {
						dwStop = GetAnnotation(nLoop)->GetStop(nSelection);
					} else {
						int nPrevious = GetAnnotation(nLoop)->GetPrevious(nSelection);
						if (nPrevious == -1) {
							return;
						}
						dwStop = GetAnnotation(nLoop)->GetStop(nPrevious);
					}
				} else {
					int nPrevious = GetAnnotation(nLoop)->GetPrevious();
					dwStart = GetAnnotation(nLoop)->GetOffset(nSelection);

					if (nPrevious == -1) {
						dwStop = 0;
					} else {
						dwStop = GetAnnotation(nLoop)->GetStop(nPrevious);
					}

					if ((nLoop != PHONETIC) && (nLoop != GLOSS) && (dwStop + GetDocument()->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME)) < dwStart) {
						dwStop = dwStart - 2;
					}
				}

				if (dwStop < GetDocument()->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) {
					return;
				}

				// change the selection
				m_advancedSelection.SelectFromStopPosition(this, nLoop, dwStop, true);
				if (m_advancedSelection.GetSelectionIndex() == -1) {
					m_advancedSelection.SelectFromStopPosition(this, nLoop, dwStop, false);
				}
			}
		}
		// if necessary scroll selection into view
		if ((m_advancedSelection.GetSelectionStart() <= m_dwDataPosition) ||
			(m_advancedSelection.GetSelectionStop() >= (m_dwDataPosition + GetDataFrame()))) {
			ZoomIn(0, FALSE);    // center selection in view
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditPrevious Menu update
/***************************************************************************/
void CSaView::OnUpdateEditPrevious(CCmdUI * pCmdUI) {
	CSaDoc * pModel = (CSaDoc *)GetDocument();
	int nLoop;

	BOOL bEnable = FALSE;
	if ((pModel->GetDataSize() != 0) && GetFocusedGraphWnd()) { // needs focused graph
		m_advancedSelection.Update(this);
		nLoop = m_advancedSelection.GetSelectionIndex();
		// only work from visible selections
		if ((nLoop != -1) && !(m_pFocusedGraph->HaveAnnotation(nLoop))) {
			nLoop = -1;    // only work from visible selections
		}

		int nSelection;
		if (nLoop == -1) {
			// no selection yet, search for first visible annotation with segments
			for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
				if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])
					&& ((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE)
						|| ((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS) && (CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) { // SDM 1.5Test8.1
					nLoop = CGraphWnd::m_anAnnWndOrder[nLoop];
					break;
				}
			}
			if (nLoop < ANNOT_WND_NUMBER) {
				bEnable = TRUE;
			}
		} else {
			int MASTER = GetAnnotation(nLoop)->GetMasterIndex();
			if ((MASTER != -1) && (nLoop != GLOSS) && (nLoop != PHONETIC)) {
				CSegment * pMaster = GetAnnotation(MASTER);

				DWORD dwPosition = m_advancedSelection.GetSelectionStart();
				int nIndexMaster = pMaster->FindOffset(dwPosition); // Should always find...
				if (nIndexMaster == -1) {
					nIndexMaster = pMaster->FindFromPosition(dwPosition, TRUE);    // Better Coverage
				}

				if (nIndexMaster != -1) {
					nIndexMaster = pMaster->GetPrevious(nIndexMaster);
				}
				if (nIndexMaster != -1) {
					bEnable = TRUE;
				}
			} else {
				DWORD dwStop = 0;
				DWORD dwStart;

				nSelection = GetAnnotation(nLoop)->GetSelection();
				if (nSelection == -1) {
					dwStart = m_advancedSelection.GetSelectionStart();
					nSelection = GetAnnotation(nLoop)->FindFromPosition(dwStart, FALSE);
					if (nSelection != -1) {

						int nPrevious = GetAnnotation(nLoop)->GetPrevious(nSelection);
						if (nPrevious != -1) {
							dwStop = GetAnnotation(nLoop)->GetStop(nPrevious);
						}
					}
				} else {
					int nPrevious = GetAnnotation(nLoop)->GetPrevious();
					dwStart = GetAnnotation(nLoop)->GetOffset(nSelection);

					if (nPrevious == -1) {
						dwStop = 0;
					} else {
						dwStop = GetAnnotation(nLoop)->GetStop(nPrevious);
					}

					if ((nLoop != PHONETIC) && (nLoop != GLOSS) && (dwStop + GetDocument()->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME)) < dwStart) {
						dwStop = dwStart - 2;
					}

				}
				bEnable = TRUE;

				if (dwStop < GetDocument()->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) {
					bEnable = FALSE;
				}
			}
		}
	}
	pCmdUI->Enable(bEnable);
}

/***************************************************************************/
// CSaView::OnEditNext Select next phonetic character
/***************************************************************************/
void CSaView::OnEditNext() {
	int nLoop;

	// needs to have a focused graph
	if (m_pFocusedGraph) {
		m_advancedSelection.Update(this);
		nLoop = m_advancedSelection.GetSelectionIndex();
		// only work from visible selections
		if ((nLoop != -1) && !(m_pFocusedGraph->HaveAnnotation(nLoop))) {
			// only work from visible selections
			nLoop = -1;
		}
		TRACE("nLoop=%d\n");
		if (GetAnnotation(PHONETIC)->IsEmpty()) {
			// no segments
			return;
		}
		int nSelection;
		if (nLoop == -1) {
			// no selection yet, search for first visible annotation with segments
			for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
				if (m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop]) &&
					((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE) ||
						((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS) && (CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) {
					nLoop = CGraphWnd::m_anAnnWndOrder[nLoop];
					break;
				}
			}
			if (nLoop >= ANNOT_WND_NUMBER) {
				// no possibility found
				return;
			}
			// try to find selection within start cursor or left of it
			m_advancedSelection.SelectFromPosition(this, nLoop, GetStartCursorPosition(), true);
			if (m_advancedSelection.GetSelectionIndex() == -1) {
				m_advancedSelection.SelectFromPosition(this, nLoop, GetStartCursorPosition(), false);
			}
		} else {
			int MASTER = GetAnnotation(nLoop)->GetMasterIndex();
			if ((MASTER != -1) && (nLoop != GLOSS) && (nLoop != PHONETIC)) {
				CSegment * pMaster = GetAnnotation(MASTER);

				DWORD dwPosition = m_advancedSelection.GetSelectionStop();
				// Should always find...
				int nIndexMaster = pMaster->FindStop(dwPosition);
				if (nIndexMaster == -1) {
					// Better Coverage
					nIndexMaster = pMaster->FindFromPosition(dwPosition, TRUE);
				}

				if (nIndexMaster == -1) {
					return;
				}
				nIndexMaster = pMaster->GetNext(nIndexMaster);
				if (nIndexMaster == -1) {
					return;
				}
				m_advancedSelection.SelectFromPosition(this, nLoop, pMaster->GetOffset(nIndexMaster), true);
				if (m_advancedSelection.GetSelectionIndex() == -1) {
					m_advancedSelection.SelectFromPosition(this, nLoop, pMaster->GetOffset(nIndexMaster), false);
				}
			} else {
				DWORD dwStop;
				DWORD dwStart;

				nSelection = GetAnnotation(nLoop)->GetSelection();
				if (nSelection == -1) {
					dwStop = m_advancedSelection.GetSelectionStop();
					nSelection = GetAnnotation(nLoop)->FindFromPosition(dwStop, FALSE);
					if (nSelection == -1) {
						return;
					}

					if (dwStop >= GetAnnotation(nLoop)->GetOffset(nSelection)) {
						return;
					}

					dwStart = GetAnnotation(nLoop)->GetOffset(nSelection);
				} else {
					int nNext = GetAnnotation(nLoop)->GetNext();
					dwStop = GetAnnotation(nLoop)->GetStop(nSelection);

					if (nNext == -1) {
						dwStart = GetDocument()->GetDataSize();
					} else {
						dwStart = GetAnnotation(nLoop)->GetOffset(nNext);
					}

					if ((dwStop + GetDocument()->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME)) < dwStart) {
						dwStart = dwStop + 2;
					}
				}

				if (dwStart + GetDocument()->GetBytesFromTime(MIN_ADD_SEGMENT_TIME) > GetDocument()->GetDataSize()) {
					return;
				}

				// change the selection
				m_advancedSelection.SelectFromPosition(this, nLoop, dwStart, true);
				if (m_advancedSelection.GetSelectionIndex() == -1) {
					m_advancedSelection.SelectFromPosition(this, nLoop, dwStart, false);
				}
			}
		}
		// if necessary scroll selection into view
		if ((m_advancedSelection.GetSelectionStart() <= m_dwDataPosition) ||
			(m_advancedSelection.GetSelectionStop() >= (m_dwDataPosition + GetDataFrame()))) {
			// center selection in view
			ZoomIn(0, FALSE);
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateEditNext Menu update
/***************************************************************************/
void CSaView::OnUpdateEditNext(CCmdUI * pCmdUI) {
	CSaDoc * pModel = (CSaDoc *)GetDocument();
	int nLoop = 0;

	BOOL bEnable = FALSE;
	if ((pModel->GetDataSize() != 0) &&
		(GetFocusedGraphWnd() != NULL) &&
		(!(GetAnnotation(PHONETIC)->IsEmpty()))) { // needs focused graph
		m_advancedSelection.Update(this);
		nLoop = m_advancedSelection.GetSelectionIndex();
		// only work from visible selections
		if ((nLoop != -1) && !(m_pFocusedGraph->HaveAnnotation(nLoop))) {
			nLoop = -1;    // only work from visible selections
		}

		int nSelection = -1;
		if (nLoop == -1) {
			// no selection yet, search for first visible annotation with segments
			for (nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
				if ((m_pFocusedGraph->HaveAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])) &&
					((GetAnnotation(CGraphWnd::m_anAnnWndOrder[nLoop])->IsEmpty() == FALSE) ||
						((CGraphWnd::m_anAnnWndOrder[nLoop] != GLOSS) && (CGraphWnd::m_anAnnWndOrder[nLoop] != PHONETIC)))) { // SDM 1.5Test8.1
					nLoop = CGraphWnd::m_anAnnWndOrder[nLoop];
					break;
				}
			}
			if (nLoop < ANNOT_WND_NUMBER) {
				bEnable = TRUE;
			}
		} else {
			int MASTER = GetAnnotation(nLoop)->GetMasterIndex();
			if ((MASTER != -1) && (nLoop != GLOSS) && (nLoop != PHONETIC)) {
				CSegment * pMaster = GetAnnotation(MASTER);

				DWORD dwPosition = m_advancedSelection.GetSelectionStop();
				// Should always find...
				int nIndexMaster = pMaster->FindStop(dwPosition);
				if (nIndexMaster == -1) {
					nIndexMaster = pMaster->FindFromPosition(dwPosition, TRUE);    // Better Coverage
				}

				if (nIndexMaster != -1) {
					nIndexMaster = pMaster->GetNext(nIndexMaster);
				}
				if (nIndexMaster != -1) {
					bEnable = TRUE;
				}
			} else {
				DWORD dwStop = 0;
				DWORD dwStart = GetDocument()->GetDataSize();
				CSegment * pSegment = GetAnnotation(nLoop);
				nSelection = pSegment->GetSelection();
				if (nSelection == -1) {
					dwStop = m_advancedSelection.GetSelectionStop();
					nSelection = pSegment->FindFromPosition(dwStop, FALSE);
					if (nSelection != -1) {
						dwStart = pSegment->GetOffset(nSelection);

						if (dwStop > pSegment->GetOffset(nSelection)) {
							dwStart = GetDocument()->GetDataSize();
						}
					}

					bEnable = TRUE;
					DWORD dwBytes = GetDocument()->GetBytesFromTime(MIN_ADD_SEGMENT_TIME);
					if ((dwStart + dwBytes) > GetDocument()->GetDataSize()) {
						bEnable = FALSE;
					}
				} else {
					int nNext = pSegment->GetNext();
					dwStop = pSegment->GetStop(nSelection);
					if (nNext == -1) {
						dwStart = GetDocument()->GetDataSize();
					} else {
						dwStart = GetAnnotation(nLoop)->GetOffset(nNext);
					}

					if ((dwStop + GetDocument()->GetBytesFromTime(DEFAULT_ADD_SEGMENT_TIME)) < dwStart) {
						dwStart = dwStop + 2;
					}

					if (nNext != -1) {
						bEnable = TRUE;
					}
					DWORD dwBytes = GetDocument()->GetBytesFromTime(MIN_ADD_SEGMENT_TIME);
					if ((dwStart + dwBytes) > GetDocument()->GetDataSize()) {
						bEnable = FALSE;
					}
				}
			}
		}
	}
	pCmdUI->Enable(bEnable);
}

/***************************************************************************/
// CSaView::GetCursorAlignment()
/***************************************************************************/
ECursorAlignment CSaView::GetCursorAlignment() {
	if (GetDocument()->GetFragments()->IsDataReady()) {
		return m_nCursorAlignment;
	} else {
		return m_nCursorAlignment == ALIGN_AT_FRAGMENT ? ALIGN_AT_ZERO_CROSSING : m_nCursorAlignment;
	}
}

/***************************************************************************/
// CSaView::GetGraphUpdateMode
/***************************************************************************/
int CSaView::GetGraphUpdateMode() {
	if (GetCursorAlignment() != ALIGN_AT_FRAGMENT) {
		return STATIC_UPDATE;    // This file must be in static mode fragments disabled.
	} else {
		return GetMainFrame().GetGraphUpdateMode();
	}
}

/***************************************************************************/
// CSaView::IsAnimationRequested
/***************************************************************************/
BOOL CSaView::IsAnimationRequested() {
	if (GetGraphUpdateMode() == STATIC_UPDATE) {
		return FALSE;    // This file must be in dynamic mode to animate
	} else {
		return GetMainFrame().IsAnimationRequested();
	}
}

/***************************************************************************/
// CSaView::GetAnimationFrameRate
/***************************************************************************/
int CSaView::GetAnimationFrameRate() {
	return GetMainFrame().GetAnimationFrameRate();
}

void CSaView::OnSpectroFormants() {
	CSaDoc * pModel = GetDocument();
	pModel->ToggleSpectrogram();
	RedrawGraphs();
}

void CSaView::OnUpdateSpectroFormants(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	CSpectroParm parameters = pModel->GetSpectrogram()->GetSpectroParm();
	if ((parameters.GetShowFormants()) && pModel->GetSpectrogram()->IsProcessCanceled()) {
		// update the display with the current state
		OnSpectroFormants();
	}
	pCmdUI->SetCheck(parameters.GetShowFormants());
}

void CSaView::OnMoveStopCursorHere() {

	// prevent crash
	if (m_pFocusedGraph == NULL) {
		TRACE("no graph in focus\n");
		return;
	}

	CPlotWnd * pPlot = m_pFocusedGraph->GetPlot();
	if (pPlot == NULL) {
		TRACE("no plot in focus\n");
		return;
	}

	// set the new positions
	// get pointer to document
	CSaDoc * pModel = GetDocument();

	CPoint point = m_pFocusedGraph->GetPopupMenuPosition();
	if ((point.x == UNDEFINED_OFFSET) && (point.y == UNDEFINED_OFFSET)) {
		return;
	}

	// calculate plot client coordinates
	CRect rWnd;
	pPlot->GetClientRect(rWnd);
	int nWidth = rWnd.Width();

	// get actual data position, frame and data size and alignment
	double fDataPos = 0;
	DWORD dwDataFrame = 0;
	// check if area graph type
	if (m_pFocusedGraph->IsAreaGraph()) {
		// get necessary data from area plot
		fDataPos = pPlot->GetAreaPosition();
		dwDataFrame = pPlot->GetAreaLength();
	} else {
		// get necessary data from document and from view
		// data index of first sample to display
		fDataPos = GetDataPosition(nWidth);
		// number of data points to display
		dwDataFrame = CalcDataFrame(nWidth);
	}

	DWORD dwDataSize = pModel->GetDataSize();
	DWORD nSmpSize = pModel->GetSampleSize();
	// calculate data samples per pixel
	ASSERT(nWidth);
	double fSamplesPerPix = (double)dwDataFrame / (double)(nWidth*nSmpSize);

	// calculate the start cursor position
	DWORD dwStopCursor = (DWORD)round2Int(fDataPos / nSmpSize + ((double)point.x) * fSamplesPerPix);
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
	if (GetStartCursorPosition() <= dwStartCursor) {
		dwStartCursor = GetStartCursorPosition();
	}

	// Added and modified from CStartCursorWnd::OnLButtonUp and modified by AKE 7/22/01 to deselect segment
	// while cursor is moving
	CSegment * pSegment = FindSelectedAnnotation();
	if (pSegment != NULL) {
		int nIndex = pSegment->GetSelection();
		if ((dwStopCursor > pSegment->GetStop(nIndex)) ||
			(dwStartCursor < pSegment->GetOffset(nIndex))) {
			// Deselect segment
			m_advancedSelection.DeselectAnnotations(this);
		}
	}

	//TRACE("start=%lu stop=%lu\n",dwStartCursor,dwStopCursor);

	// move stop cursor also
	SetStartCursorPosition(dwStartCursor);
	SetStopCursorPosition(dwStopCursor);
}

/***************************************************************************/
// CSaDoc::OnAutoSnapUpdate Adjust all independent segments to snap boundaries
/***************************************************************************/
void CSaView::OnEditCopyPhoneticToPhonemic(void) {

	// doesn't user want to keep existing gloss?
	if (AfxMessageBox(IDS_CONFIRM_PHONEMIC_COPY, MB_YESNO | MB_ICONQUESTION, 0) != IDYES) {
		return;
	}

	BeginWaitCursor();

	CSegment * pPhonemic = GetAnnotation(PHONEMIC);
	if (pPhonemic != NULL) {
		pPhonemic->DeleteContents();
	}

	DWORD lastOffset = -1;
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)GetAnnotation(PHONETIC);
	for (int i = 0; i < pPhonetic->GetOffsetSize(); i++) {
		DWORD offset = pPhonetic->GetOffset(i);
		if (offset == lastOffset) {
			continue;
		}
		lastOffset = offset;
		CSaString text = pPhonetic->GetSegmentString(i);
		DWORD duration = pPhonetic->GetDuration(i);
		TRACE(L"text=%s offset=%d duration=%d\n", (LPCTSTR)text, offset, duration);
		pPhonemic->Insert(i, text, false, offset, duration);
	}

	// show the phonemic annotation if it isn't already displayed
	if (m_pFocusedGraph != NULL) {
		if (!m_pFocusedGraph->HaveAnnotation(PHONEMIC)) {
			ShowAnnotation(PHONEMIC);
		}
	}

	RedrawGraphs(TRUE);

	EndWaitCursor();
}

/***************************************************************************/
// CSaDoc::OnUpdateAutoSnapUpdate
/***************************************************************************/
void CSaView::OnUpdateEditCopyPhoneticToPhonemic(CCmdUI * pCmdUI) {
	pCmdUI->Enable(TRUE);
}

BOOL CSaView::GetGraphSubRect(const CRect * pWndRect, CRect * pSubRect, int nPos, const UINT * anGraphID) const {
	return GetGraphSubRect(m_nLayout, pWndRect, pSubRect, nPos, anGraphID);
}

UINT CSaView::GetLayout(void) {
	// DDO - 08/07/00
	return m_nLayout;
}

UINT * CSaView::GetGraphIDs() {
	// get the graph IDs
	return &m_anGraphID[0];
}

CGraphWnd * CSaView::GetGraph(int nIndex) {
	// get the pointers to a graph
	if (nIndex < 0) {
		return NULL;
	}
	if (nIndex > MAX_GRAPHS_NUMBER) {
		return NULL;
	}
	return m_apGraphs[nIndex];
}

BOOL CSaView::IsAnimating() {
	return m_bAnimating;
}

void CSaView::ChangeCursorAlignment(ECursorAlignment nCursorSetting) {
	m_nCursorAlignment = nCursorSetting;
	OnCursorAlignmentChanged();
}

CURSORPOS CSaView::GetStartCursorPosition() {
	// get the start cursor position
	return m_dwStartCursor;
}

CURSORPOS CSaView::GetStopCursorPosition() {
	// get the stop cursor position
	return m_dwStopCursor;
}

DWORD CSaView::GetPlaybackCursorPosition() {
	//TRACE("GetPlaybackCursorPosition=%d\n",m_dwLastPlaybackPosition);
	return m_dwLastPlaybackPosition;
}

CGraphWnd * CSaView::GetFocusedGraphWnd() {
	// gets the focused graph window pointer
	return m_pFocusedGraph;
}

UINT CSaView::GetFocusedGraphID() {
	// gets the focused graph ID
	return m_nFocusedID;
}

BOOL CSaView::ViewIsActive() {
	// returns TRUE, if view is active
	return m_bViewIsActive;
};

BOOL CSaView::IsUpdateBoundaries() {
	// return TRUE, if boundaries updated
	return m_bUpdateBoundaries;
}

void CSaView::SetUpdateBoundaries(BOOL bUpdate) {
	m_bUpdateBoundaries = bUpdate;
}

void CSaView::Scroll(DWORD desiredPosition) {
	UINT nPos = (UINT)(desiredPosition / m_dwHScrollFactor);
	SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nPos));
};

void CSaView::Scroll(UINT nSBCode, UINT nPos) {
	SendMessage(WM_HSCROLL, MAKEWPARAM(nSBCode, nPos));
}

BOOL CSaView::PrintPreviewInProgress() {
	return m_bPrintPreviewInProgress;
};

int CSaView::PrinterDPI() {
	return m_newPrinterDPI;
};

CPoint CSaView::RealPrinterDPI() {
	return m_printerDPI;
};

// The bottom window's z is zero.
int CSaView::GetZ() const {
	return m_z;
}

// Greater z means above; lesser z means below.
void CSaView::SetZ(int z) {
	m_z = z;
}

BOOL CSaView::GetStaticTWC() {
	return m_bStaticTWC;
}

void CSaView::SetStaticTWC(BOOL bChecked) {
	m_bStaticTWC = bChecked;
}

BOOL CSaView::GetNormalMelogram() {
	return m_bNormalMelogram;
}

void CSaView::SetNormalMelogram(BOOL bChecked) {
	m_bNormalMelogram = bChecked;
}

LRESULT CSaView::OnAutoSave(WPARAM, LPARAM) {
	GetDocument()->StoreAutoRecoveryInformation();
	return 0;
}

CGraphWnd * CSaView::GetGraphForAnnotation(int annotation) {
	for (size_t i = 0; i < MAX_GRAPHS_NUMBER; i++) {
		CGraphWnd * pGraph = m_apGraphs[i];
		if (pGraph == NULL) {
			continue;
		}
		if (pGraph->HaveAnnotation(annotation)) {
			return pGraph;
		}
	}
	return NULL;
}

DWORD CSaView::GetMinimumSeparation(CSaDoc * pModel, CGraphWnd * pGraph, CPlotWnd * pPlot) {

	// get window coordinates of parent
	CRect rWnd;
	pPlot->GetClientRect(rWnd);
	int nWidth = rWnd.Width();

	// get actual data position, frame and data size and alignment
	double fDataPos = 0;
	DWORD dwDataFrame = 0;
	// check if area graph type
	if (pGraph->IsAreaGraph()) {
		// get necessary data from area plot
		fDataPos = pPlot->GetAreaPosition();
		dwDataFrame = pPlot->GetAreaLength();
	} else {
		// get necessary data from document and from view
		// data index of first sample to display
		fDataPos = GetDataPosition(nWidth);
		// number of data points to display
		dwDataFrame = CalcDataFrame(nWidth);
	}
	DWORD nSmpSize = pModel->GetSampleSize();
	// calculate data samples per pixel
	ASSERT(nWidth);
	double fSamplesPerPix = (double)dwDataFrame / (double)(nWidth*nSmpSize);
	// calculate minimum position for stop cursor
	return (DWORD)(CURSOR_MIN_DISTANCE*fSamplesPerPix*nSmpSize);
}

BOOL CSaView::SelectFromPosition(int nSegmentIndex, DWORD dwPosition, bool bFindExact) {
	return m_advancedSelection.SelectFromPosition(this, nSegmentIndex, dwPosition, bFindExact);
}

BOOL CSaView::SetSelectedAnnotationString(CSaString & szString, bool bIncludesDelimiter, bool bCheck) {
	return m_advancedSelection.SetSelectedAnnotationString(this, szString, bIncludesDelimiter, bCheck);
}

CString CSaView::GetSelectedAnnotationString(BOOL bRemoveDelimiter) {
	return m_advancedSelection.GetSelectedAnnotationString(this, bRemoveDelimiter);
}

/***************************************************************************/
// CSaView::GetSelectedAnnotationString
/***************************************************************************/
CSaString CSaView::GetSelectedAnnotationString() {
	
	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		CSegment * pSegment = GetAnnotation(nLoop);
		if (pSegment->GetSelection() != -1) {
			int selection = pSegment->GetSelection();
			return pSegment->GetSegmentString(selection);
			break;
		}
	}
	return CSaString("");
}

void CSaView::UpdateSelection(BOOL bClearVirtual) {
	m_advancedSelection.Update(this, bClearVirtual);
}

DWORD CSaView::GetSelectionStart() {
	return m_advancedSelection.GetSelectionStart();
}

DWORD CSaView::GetSelectionStop() {
	return m_advancedSelection.GetSelectionStop();
}

int CSaView::GetSelectionIndex() {
	return m_advancedSelection.GetSelectionIndex();
}

bool CSaView::IsSelectionVirtual() {
	return m_advancedSelection.IsSelectionVirtual();
}

void CSaView::OnSelectTranscriptionBars() {

	// validity of m_pFocusedGraph is checked in OnUpdate call..
	BOOL bReference = FALSE;
	BOOL bPhonetic = FALSE;
	BOOL bTone = FALSE;
	BOOL bPhonemic = FALSE;
	BOOL bOrthographic = FALSE;
	BOOL bGloss = FALSE;
	BOOL bGlossNat = FALSE;
	BOOL bPhraseList1 = FALSE;
	BOOL bPhraseList2 = FALSE;
	BOOL bPhraseList3 = FALSE;
	BOOL bPhraseList4 = FALSE;
	if (m_pFocusedGraph != NULL) {
		bReference = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_REFERENCE - ID_POPUPGRAPH_PHONETIC);
		bPhonetic = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_PHONETIC - ID_POPUPGRAPH_PHONETIC);
		bTone = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_TONE - ID_POPUPGRAPH_PHONETIC);
		bPhonemic = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_PHONEMIC - ID_POPUPGRAPH_PHONETIC);
		bOrthographic = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_ORTHO - ID_POPUPGRAPH_PHONETIC);
		bGloss = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_GLOSS - ID_POPUPGRAPH_PHONETIC);
		bGlossNat = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_GLOSS_NAT - ID_POPUPGRAPH_PHONETIC);
		bPhraseList1 = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_MUSIC_PL1 - ID_POPUPGRAPH_PHONETIC);
		bPhraseList2 = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_MUSIC_PL2 - ID_POPUPGRAPH_PHONETIC);
		bPhraseList3 = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_MUSIC_PL3 - ID_POPUPGRAPH_PHONETIC);
		bPhraseList4 = m_pFocusedGraph->HaveAnnotation(ID_POPUPGRAPH_MUSIC_PL4 - ID_POPUPGRAPH_PHONETIC);
	}

	CDlgSelectTranscriptionBars dlg;
	dlg.bReference = bReference;
	dlg.bPhonetic = bPhonetic;
	dlg.bTone = bTone;
	dlg.bPhonemic = bPhonemic;
	dlg.bOrthographic = bOrthographic;
	dlg.bGloss = bGloss;
	dlg.bGlossNat = bGlossNat;
	dlg.bPhraseList1 = bPhraseList1;
	dlg.bPhraseList2 = bPhraseList2;
	dlg.bPhraseList3 = bPhraseList3;
	dlg.bPhraseList4 = bPhraseList4;

	if (dlg.DoModal() != IDOK) {
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_REFERENCE - ID_POPUPGRAPH_PHONETIC), bReference, TRUE);
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_PHONETIC - ID_POPUPGRAPH_PHONETIC), bPhonetic, TRUE);
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_TONE - ID_POPUPGRAPH_PHONETIC), bTone, TRUE);
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_PHONEMIC - ID_POPUPGRAPH_PHONETIC), bPhonemic, TRUE);
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_ORTHO - ID_POPUPGRAPH_PHONETIC), bOrthographic, TRUE);
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_GLOSS - ID_POPUPGRAPH_PHONETIC), bGloss, TRUE);
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_GLOSS_NAT - ID_POPUPGRAPH_PHONETIC), bGlossNat, TRUE);
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_MUSIC_PL1 - ID_POPUPGRAPH_PHONETIC), bPhraseList1, TRUE);
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_MUSIC_PL2 - ID_POPUPGRAPH_PHONETIC), bPhraseList2, TRUE);
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_MUSIC_PL3 - ID_POPUPGRAPH_PHONETIC), bPhraseList3, TRUE);
		m_pFocusedGraph->ShowAnnotation((EAnnotation)(ID_POPUPGRAPH_MUSIC_PL4 - ID_POPUPGRAPH_PHONETIC), bPhraseList4, TRUE);
	}
}

void CSaView::OnUpdateSelectTranscriptionBars(CCmdUI * pCmdUI) {
	pCmdUI->Enable(m_pFocusedGraph != NULL);
}

void CSaView::OnToolsAdjustSilence() {

	CURSORPOS oldStart = GetStartCursorPosition();
	CURSORPOS oldStop = GetStopCursorPosition();

	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();

	CDlgInsertSilence dlg(pModel);
	if (dlg.DoModal() != IDOK) {
		pModel->UndoWaveFile();
		// get wave from the clipboard
		pModel->InvalidateAllProcesses();
		// restore the cursors
		SetStartCursorPosition(oldStart);
		SetStopCursorPosition(oldStop);
		RedrawGraphs(TRUE);
	}
}

void CSaView::OnUpdateToolsAdjustSilence(CCmdUI * pCmdUI) {
	pCmdUI->Enable(TRUE);
}

void CSaView::OnToolsAdjustZero() {
	CSaDoc * pModel = GetDocument();
	pModel->AdjustZero();
	RedrawGraphs();
}

void CSaView::OnUpdateToolsAdjustZero(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	pCmdUI->SetCheck(pModel->GetZero());
}

void CSaView::OnToolsAdjustNormalize() {
	CSaDoc * pModel = GetDocument();
	pModel->AdjustNormalize();
	RedrawGraphs();
}

void CSaView::OnUpdateToolsAdjustNormalize(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	pCmdUI->SetCheck(pModel->GetNormalize());
}

void CSaView::OnToolsAdjustInvert() {
	CSaDoc * pModel = GetDocument();
	pModel->AdjustInvert();
	RedrawGraphs();
}

void CSaView::OnUpdateToolsAdjustInvert(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	pCmdUI->SetCheck(pModel->GetInvert());
}

/***************************************************************************/
// CSaView::OnAutoSnapUpdate Adjust all independent segments to snap boundaries
/***************************************************************************/
void CSaView::OnAutoSnapUpdate(void) {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	pModel->AutoSnapUpdate();
}

/***************************************************************************/
// CSaView::OnUpdateAutoSnapUpdate
/***************************************************************************/
void CSaView::OnUpdateAutoSnapUpdate(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	pCmdUI->Enable(pModel->AutoSnapUpdateNeeded());
}

// SDM Added Function in Release 1.06.5
/***************************************************************************/
// CSaView::OnUpdateBoundaries
/***************************************************************************/
void CSaView::OnUpdateBoundaries(void) {

	int nLoop = FindSelectedAnnotationIndex();
	if (nLoop == -1) {
		return;
	}

	CSaDoc * pModel = GetDocument();
	if (pModel->GetSegment(nLoop)->CheckCursors(pModel, GetEditBoundaries(false) == BOUNDARIES_EDIT_SEGMENT_SIZE) != -1) {
		// save state for undo ability
		DWORD dwNewStart = GetStartCursorPosition();
		DWORD dwNewStop = GetStopCursorPosition();
		int nIndex = pModel->GetSegment(nLoop)->GetSelection();

		// first adjust cursors to old segment boundaries (undo to here)
		SetStartCursorPosition(pModel->GetSegment(nLoop)->GetOffset(nIndex), SNAP_RIGHT);
		SetStopCursorPosition(pModel->GetSegment(nLoop)->GetStop(nIndex), SNAP_LEFT);

		// If independent segment boundaries have moved they need to be updated for snapping
		if (((pModel->GetSegment(nLoop)->GetMasterIndex() == -1) &&
			((GetStartCursorPosition() != pModel->GetSegment(nLoop)->GetOffset(nIndex)) ||
				(GetStopCursorPosition() != (pModel->GetSegment(nLoop)->GetStop(nIndex)))))) {
			// Phonetic Segment was not snapped - automatically update it
			PostMessage(WM_COMMAND, ID_EDIT_AUTO_SNAP_UPDATE, 0L);
		}

		// Save state
		pModel->CheckPoint();

		// Reload cursor locations to new segment boundaries
		SetStartCursorPosition(dwNewStart, SNAP_LEFT);
		SetStopCursorPosition(dwNewStop, SNAP_RIGHT);

		// Do update
		//SDM 1.5Test8.1
		pModel->UpdateSegmentBoundaries(GetEditBoundaries(false) == BOUNDARIES_EDIT_SEGMENT_SIZE);
	}
}

// SDM Added Function in Release 1.06.5
/***************************************************************************/
// CSaView::OnUpdateUpdateBondaries
/***************************************************************************/
void CSaView::OnUpdateUpdateBoundaries(CCmdUI * pCmdUI) {

	CSaDoc * pModel = GetDocument();
	BOOL enable = FALSE;
	int nLoop = FindSelectedAnnotationIndex();
	if ((nLoop != -1) && (pModel->GetSegment(nLoop)->CheckCursors(pModel, GetEditBoundaries(false) == BOUNDARIES_EDIT_SEGMENT_SIZE) != -1)) {
		int nSelection = pModel->GetSegment(nLoop)->GetSelection();
		DWORD dwStart = GetStartCursorPosition();
		DWORD dwStop = GetStopCursorPosition();

		if ((pModel->GetSegment(nLoop)->GetMasterIndex() == -1) &&
			((dwStart != pModel->GetSegment(nLoop)->GetOffset(nSelection)) || (dwStop != pModel->GetSegment(nLoop)->GetStop(nSelection)))) {
			enable = TRUE;
		} else {
			if (dwStart != pModel->SnapCursor(START_CURSOR, pModel->GetSegment(nLoop)->GetOffset(nSelection), 0, pModel->GetDataSize(), SNAP_RIGHT)) {
				enable = TRUE;
			}
			if (dwStop != pModel->SnapCursor(STOP_CURSOR, pModel->GetSegment(nLoop)->GetStop(nSelection), 0, pModel->GetDataSize(), SNAP_LEFT)) {
				enable = TRUE;
			}
		}
	}
	pCmdUI->Enable(enable);
}

// SDM 1.06.4
/***************************************************************************/
// CSaView::OnAddReferenceData  Add reference data
/***************************************************************************/
void CSaView::OnAddReferenceData() {

	// get pointer to application
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();

	// determine how many words there are
	CGlossSegment * pGloss = (CGlossSegment *)pModel->GetSegment(GLOSS);
	if (pGloss->IsEmpty()) {
		ErrorMessage(IDS_ERROR_NO_WORDS_ON_AUTO_REFERENCE);
		return;
	}

	int begin = 1;
	int end = pGloss->GetOffsetSize();

	int selection = pGloss->GetSelection();
	bool glossSelected = (selection != -1);

	int numWords = pGloss->GetOffsetSize();

	// query the user
	CDlgAutoReferenceData dlg(pModel, numWords);

	CSaApp * pApp = (CSaApp*)AfxGetApp();

	dlg.mLastImport = pApp->GetProfileString(L"AutoRef", L"LastImport", L"");
	dlg.mBeginRef = pApp->GetProfileString(L"AutoRef", L"BeginRef", L"");
	dlg.mEndRef = pApp->GetProfileString(L"AutoRef", L"EndRef", L"");
	dlg.mUsingNumbers = (pApp->GetProfileInt(L"AutoRef", L"UsingNumbers", 1) != 0) ? true : false;
	dlg.mUsingFirstGloss = (pApp->GetProfileInt(L"AutoRef", L"UsingFirstGloss", 1) != 0) ? true : false;
	dlg.mBegin = begin;
	dlg.mEnd = end;
	dlg.mGlossSelected = glossSelected;

	if (dlg.DoModal() != IDOK) {
		// do nothing on cancel
		return;
	}

	pApp->WriteProfileString(L"AutoRef", L"LastImport", dlg.mLastImport);
	pApp->WriteProfileString(L"AutoRef", L"BeginRef", dlg.mBeginRef);
	pApp->WriteProfileString(L"AutoRef", L"EndRef", dlg.mEndRef);
	pApp->WriteProfileInt(L"AutoRef", L"UsingNumbers", ((dlg.mUsingNumbers) ? 1 : 0));
	pApp->WriteProfileInt(L"AutoRef", L"UsingFirstGloss", ((dlg.mUsingFirstGloss) ? 1 : 0));

	if (pModel->PreflightAddReferenceData(dlg, selection)) {
		int result = AfxMessageBox(IDS_DELETE_REFERENCE, MB_YESNO | MB_ICONQUESTION);
		if (result != IDYES) {
			return;
		}
	}

	// do it for real
	pModel->AddReferenceData(dlg, selection);

	pModel->SetModifiedFlag(TRUE); // data has been modified

	// refresh the tables
	CGraphWnd * pGraph = GraphIDtoPtr(IDD_RAWDATA);
	if (pGraph != NULL) {
		pGraph->ShowAnnotation(REFERENCE, TRUE, TRUE);
	}

	RedrawGraphs(TRUE);
}

void CSaView::OnUpdateAddReferenceData(CCmdUI * pCmdUI) {
	// enable if data is available
	CSaDoc * pModel = GetDocument();
	pCmdUI->Enable(pModel->GetDataSize() != 0);
}

// SDM 1.06.4
/***************************************************************************/
// CSaView::OnAutoAlign Start Align Wizard
/***************************************************************************/
void CSaView::OnAutoAlign() {

	CSaDoc * pModel = GetDocument();
	pModel->ClearTranscriptionCount();

	CDlgAlignTranscriptionDataSheet dlg(NULL, pModel);

	CSaApp * pApp = (CSaApp*)AfxGetApp();

	dlg.init.m_bGloss = (pApp->GetProfileInt(L"TranscriptionAlignment", L"Gloss", 0) != 0);
	dlg.init.m_bGlossNat = (pApp->GetProfileInt(L"TranscriptionAlignment", L"GlossNat", 0) != 0);
	dlg.init.m_bOrthographic = (pApp->GetProfileInt(L"TranscriptionAlignment", L"Orthographic", 0) != 0);
	dlg.init.m_bPhonemic = (pApp->GetProfileInt(L"TranscriptionAlignment", L"Phonemic", 0) != 0);
	dlg.init.m_bPhonetic = (pApp->GetProfileInt(L"TranscriptionAlignment", L"Phonetic", 1) != 0);
	dlg.init.m_bReference = (pApp->GetProfileInt(L"TranscriptionAlignment", L"Reference", 1) != 0);
	dlg.init.m_bUseReference = (pApp->GetProfileInt(L"TranscriptionAlignment", L"UseReference", 0) != 0);

	dlg.SetWizardMode();
	int result = dlg.DoModal();
	if (result == ID_WIZFINISH) {
		pApp->WriteProfileInt(L"TranscriptionAlignment", L"Gloss", (dlg.init.m_bGloss) ? 1 : 0);
		pApp->WriteProfileInt(L"TranscriptionAlignment", L"GlossNat", (dlg.init.m_bGlossNat) ? 1 : 0);
		pApp->WriteProfileInt(L"TranscriptionAlignment", L"Orthographic", (dlg.init.m_bOrthographic) ? 1 : 0);
		pApp->WriteProfileInt(L"TranscriptionAlignment", L"Phonemic", (dlg.init.m_bPhonemic) ? 1 : 0);
		pApp->WriteProfileInt(L"TranscriptionAlignment", L"Phonetic", (dlg.init.m_bPhonetic) ? 1 : 0);
		pApp->WriteProfileInt(L"TranscriptionAlignment", L"Reference", (dlg.init.m_bReference) ? 1 : 0);
		pApp->WriteProfileInt(L"TranscriptionAlignment", L"UseReference", (dlg.init.m_bUseReference) ? 1 : 0);
	} else if (result == IDCANCEL) {
		pModel->RevertAllTranscriptionChanges();
	}
}

void CSaView::OnUpdateAutoAlign(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	pCmdUI->Enable(pModel->GetDataSize() != 0); // enable if data is available
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaView::OnAdvancedParseWords Parse wave data
/***************************************************************************/
void CSaView::OnAdvancedParseWords() {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	if (m_pDlgAdvancedParseWords == NULL) {
		m_pDlgAdvancedParseWords = new CDlgAdvancedParseWords(pModel);
		if (!m_pDlgAdvancedParseWords->Create()) {
			ErrorMessage(IDS_ERROR_NO_DIALOG);
			delete m_pDlgAdvancedParseWords;
			m_pDlgAdvancedParseWords = NULL;
			return;
		}
	}
	m_pDlgAdvancedParseWords->Show((LPCTSTR)pModel->GetTitle());
}

/***************************************************************************/
// CSaView::OnUpdateAdvancedParseWords Menu update
/***************************************************************************/
void CSaView::OnUpdateAdvancedParseWords(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	pCmdUI->Enable(pModel->GetDataSize() != 0); // enable if data is available
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaView::OnAdvancedParsePhrases Parse wave data
/***************************************************************************/
void CSaView::OnAdvancedParsePhrases() {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	if (m_pDlgAdvancedParsePhrases == NULL) {
		m_pDlgAdvancedParsePhrases = new CDlgAdvancedParsePhrases(pModel);
		if (!m_pDlgAdvancedParsePhrases->Create()) {
			ErrorMessage(IDS_ERROR_NO_DIALOG);
			delete m_pDlgAdvancedParsePhrases;
			m_pDlgAdvancedParsePhrases = NULL;
			return;
		}
	}
	m_pDlgAdvancedParsePhrases->Show((LPCTSTR)pModel->GetTitle());
}

/***************************************************************************/
// CSaView::OnUpdateAdvancedParsePhrases Menu update
/***************************************************************************/
void CSaView::OnUpdateAdvancedParsePhrases(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	pCmdUI->Enable(pModel->GetDataSize() != 0); // enable if data is available
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaView::OnAdvancedSegment Segment wave data
/***************************************************************************/
void CSaView::OnAdvancedSegment() {
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	if (m_pDlgAdvancedSegment == NULL) {
		m_pDlgAdvancedSegment = new CDlgAdvancedSegment(pModel);
		if (!m_pDlgAdvancedSegment->Create()) {
			ErrorMessage(IDS_ERROR_NO_DIALOG);
			delete m_pDlgAdvancedSegment;
			m_pDlgAdvancedSegment = NULL;
			return;
		}
	}
	m_pDlgAdvancedSegment->Show((LPCTSTR)pModel->GetTitle());
}

/***************************************************************************/
// CSaView::OnUpdateAdvancedSegment Menu update
/***************************************************************************/
void CSaView::OnUpdateAdvancedSegment(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	pCmdUI->Enable(pModel->GetDataSize() != 0); // enable if data is available
}

/***************************************************************************/
// CSaView::OnFileSplitFile
// Splits a file based on user defined keys such as reference or gloss
/***************************************************************************/
void CSaView::OnFileSplitFile() {

	CSaApp * pApp = (CSaApp*)AfxGetApp();
	CSaDoc * pModel = GetDocument();
	CString fileName = pModel->GetPathName();
	wchar_t buffer[MAX_PATH];
	swprintf_s(buffer, _countof(buffer), fileName);
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	_wsplitpath_s(buffer, drive, dir, fname, ext);

	CString homeDefault = pApp->GetProfileString(_T(""), _T("DataLocation"));
	if (homeDefault.GetLength() == 0) {
		homeDefault.Format(L"%s%s", drive, dir);
	}
	if (homeDefault.Right(1) != _T("\\")) {
		homeDefault += _T("\\");
	}

	CDlgSplitFile dlg;

	dlg.m_szFolderLocation = pApp->GetProfileString(L"SplitFile", L"Home", (LPCTSTR)homeDefault);
	dlg.m_szFolderName.Format(L"Split-%s", fname);
	dlg.m_szFolderName = FilterName((LPCTSTR)dlg.m_szFolderName).c_str();
	dlg.m_szPhraseFolderName.Format(L"Split-%s-Phrase", fname);
	dlg.m_szPhraseFolderName = FilterName((LPCTSTR)dlg.m_szPhraseFolderName).c_str();
	dlg.m_szGlossFolderName.Format(L"Split-%s-Data", fname);
	dlg.m_szGlossFolderName = FilterName((LPCTSTR)dlg.m_szGlossFolderName).c_str();
	dlg.m_bOverwriteData = (pApp->GetProfileInt(L"SplitFile", L"OverwriteData", 0) != 0);
	dlg.m_bSkipGlossEmpty = (pApp->GetProfileInt(L"SplitFile", L"SkipEmptyGloss", 1) != 0);
	dlg.SetWordFilenameConvention(pApp->GetProfileIntW(L"SplitFile", L"WordConvention", 2));
	dlg.SetPhraseFilenameConvention(pApp->GetProfileIntW(L"SplitFile", L"PhraseConvention", 2));

	bool retry = false;

	wstring newPath;
	wstring glossPath;
	wstring phrasePath;
	int offsetSize;
	bool hasGloss;

	do {
		retry = false;

		if (dlg.DoModal() != IDOK) {
			return;
		}

		pApp->WriteProfileInt(L"SplitFile", L"WordConvention", (dlg.GetWordFilenameConvention()));
		pApp->WriteProfileInt(L"SplitFile", L"PhraseConvention", (dlg.GetPhraseFilenameConvention()));
		pApp->WriteProfileInt(L"SplitFile", L"OverwriteData", (dlg.m_bOverwriteData) ? 1 : 0);
		pApp->WriteProfileInt(L"SplitFile", L"SkipEmptyGloss", (dlg.m_bSkipGlossEmpty) ? 1 : 0);
		pApp->WriteProfileString(L"SplitFile", L"Home", dlg.m_szFolderLocation);

		// we need a focused graph!
		if (GetFocusedGraphWnd() == NULL) {
			ErrorMessage(IDS_SPLIT_NO_SELECTION);
			return;
		}

		// key off of gloss for now
		offsetSize = GetAnnotation(GLOSS)->GetOffsetSize();
		hasGloss = (offsetSize != 0);

		newPath = L"";
		glossPath = L"";
		phrasePath = L"";

		newPath.append(dlg.m_szFolderLocation).append(dlg.m_szFolderName);
		glossPath.append(dlg.m_szFolderLocation).append(dlg.m_szFolderName).append(L"\\").append(dlg.m_szGlossFolderName);
		phrasePath.append(dlg.m_szFolderLocation).append(dlg.m_szFolderName).append(L"\\").append(dlg.m_szPhraseFolderName);

		// check the for preexistence of a file with the same name as the folder.
		if (FileUtils::FileExists(glossPath.c_str())) {
			ErrorMessage(IDS_SPLIT_FILE_EXISTS, glossPath.c_str());
			return;
		}

		if (FileUtils::FileExists(phrasePath.c_str())) {
			ErrorMessage(IDS_SPLIT_FILE_EXISTS, phrasePath.c_str());
			return;
		}

		if (!dlg.m_bOverwriteData) {
			// check the for preexistence of the folders.
			if (FileUtils::FolderExists(newPath.c_str())) {
				CString msg;
				msg.FormatMessage(IDS_SPLIT_FOLDER_EXISTS, newPath.c_str());
				int result = AfxMessageBox(msg, MB_ABORTRETRYIGNORE | MB_ICONQUESTION, 0);
				if (result == IDABORT) {
					return;
				}
				if (result == IDRETRY) {
					retry = true;
				}
			} else if (FileUtils::FolderExists(glossPath.c_str())) {
				CString msg;
				msg.FormatMessage(IDS_SPLIT_FOLDER_EXISTS, glossPath.c_str());
				int result = AfxMessageBox(msg, MB_ABORTRETRYIGNORE | MB_ICONQUESTION, 0);
				if (result == IDABORT) {
					return;
				}
				if (result == IDRETRY) {
					retry = true;
				}
			} else if (FileUtils::FolderExists(phrasePath.c_str())) {
				CString msg;
				msg.FormatMessage(IDS_SPLIT_FOLDER_EXISTS, phrasePath.c_str());
				int result = AfxMessageBox(msg, MB_ABORTRETRYIGNORE | MB_ICONQUESTION, 0);
				if (result == IDABORT) {
					return;
				}
				if (result == IDRETRY) {
					retry = true;
				}
			}
		}
	} while (retry);

	// create the folders.
	if (!FileUtils::CreateFolder(newPath.c_str())) {
		ErrorMessage(IDS_SPLIT_BAD_DIRECTORY, newPath.c_str());
		return;
	}
	if (!FileUtils::CreateFolder(glossPath.c_str())) {
		ErrorMessage(IDS_SPLIT_BAD_DIRECTORY, glossPath.c_str());
		return;
	}
	if (!FileUtils::CreateFolder(phrasePath.c_str())) {
		ErrorMessage(IDS_SPLIT_BAD_DIRECTORY, phrasePath.c_str());
		return;
	}

	int dataCount = 0;
	int wavCount = 0;

	CScopedCursor waitCursor(this);

	EWordFilenameConvention wordConvention = dlg.GetWordFilenameConvention();
	EPhraseFilenameConvention phraseConvention = dlg.GetPhraseFilenameConvention();

	if ((hasGloss) || (!dlg.m_bSkipGlossEmpty)) {
		if (!ExportWordSegments(wordConvention, glossPath.c_str(), dlg.m_bSkipGlossEmpty, dataCount, wavCount, dlg.m_szFilenamePrefix, dlg.m_szFilenameSuffix)) {
			return;
		}
	}

	if (!ExportPhraseSegments(MUSIC_PL1, phraseConvention, phrasePath, dataCount, wavCount, dlg.m_szFilenamePrefix, dlg.m_szFilenameSuffix)) {
		return;
	}

	if (!ExportPhraseSegments(MUSIC_PL2, phraseConvention, phrasePath, dataCount, wavCount, dlg.m_szFilenamePrefix, dlg.m_szFilenameSuffix)) {
		return;
	}

	int count = dataCount + wavCount;

	if (count == 0) {
		ErrorMessage(IDS_SPLIT_INCOMPLETE);
	} else {
		CString szText;
		wchar_t szNumber[128];
		swprintf_s(szNumber, _countof(szNumber), L"%d", count);
		pApp->Message(IDS_SPLIT_COMPLETE, szNumber);
	}
}

/***************************************************************************/
// CSaView::OnUpdateFileSplit Menu Update
/***************************************************************************/
void CSaView::OnUpdateFileSplit(CCmdUI * pCmdUI) {
	CSaApp * pApp = (CSaApp*)AfxGetApp();
	pCmdUI->Enable(pApp->GetBatchMode() == 0);
}

/***************************************************************************/
// CSaView::OnUpdateFileSave Menu Update
/***************************************************************************/
void CSaView::OnUpdateFileSave(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	if (!pModel->HasFullName()) {
		// no path name provided
		pCmdUI->Enable(FALSE);
	} else {
		// path name ok, enable if dirty or processed
		pCmdUI->Enable((pModel->IsModified() || (pModel->GetWbProcess())));
	}
}

// SDM 1.5Test8.1
/***************************************************************************/
// CSaDoc::OnFileSaveAs SaveAS file
// Saves file to new name. Sets file read only file status.
// If a workbench process has been selected, wave data comes
// from a data process. The wave temporary file will be deleted and the
// process temporary file will become the new wave temporary file. The
// process data will be invalidated and the process switched back to plain.
/***************************************************************************/
void CSaView::OnFileSaveAs() {

	CSaApp & app = *(CSaApp*)AfxGetApp();
	CSaDoc & doc = *GetDocument();
	const bool stereo = doc.GetNumChannels() > 1;

	// a recording is stored in a temp file with a .tmp extension,
	// but will have a title SAx.  supply both to the saveas dialog for display,
	// but convert the temp file

	// a normal recording has a full path.
	CString path;
	CString docname;
	bool recording = false;
	if (!doc.IsTempWaveEmpty()) {
		path = doc.GetTempWave();
		docname = doc.GetPathName();
		if (docname.IsEmpty()) {
			// get the current view caption string
			docname = doc.GetFilenameFromTitle().c_str();
		}
		docname = FileUtils::ReplaceExtension((LPCTSTR)docname, L".wav").c_str();
		recording = true;
	} else {
		path = doc.GetPathName();
		if (path.IsEmpty()) {
			// get the current view caption string
			path = doc.GetFilenameFromTitle().c_str();
		}
		path = FileUtils::ReplaceExtension((LPCTSTR)path, L".wav").c_str();
		docname = path;
	}

	// retrieve samples per second
	DWORD samplesPerSec = 0;
	{
		DWORD flags = MMIO_READ;
		WORD bitsPerSample = 0;
		WORD formatTag = 0;
		WORD channels = 0;
		WORD blockAlign = 0;
		try {
			CWaveReader reader;
			reader.Read(path, flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign);
		} catch (wave_error e) {
			app.ErrorMessage(IDS_ERROR_CANT_READ_WAVE_FILE, (LPCTSTR)path);
			return;
		}
	}

	CString defaultDir = app.GetDefaultDir();
	CString extension = _T("wav");
	CString filter = _T("WAV Files (*.wav)|*.wav||");
	CDlgSaveAsOptions dlg(extension, docname, defaultDir, OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT, filter, NULL, true, stereo, samplesPerSec);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	{
		CScopedCursor waitCursor(this);

		// is same file only applies when we have a regular document, not a recording
		bool same = (recording) ? false : dlg.IsSameFile();

		if (same) {
			// There is only one file.
			dlg.mShowFiles = showNew;
			if ((stereo) && ((dlg.mFileFormat == formatMono) || (dlg.mFileFormat == formatRight))) {
				app.ErrorMessage(IDS_ERROR_NO_DUPE_FILENAME);
				return;
			}
		} else if (app.IsFileOpened(dlg.GetSelectedPath())) {
			// error file already opened by SA
			app.ErrorMessage(IDS_ERROR_FILEOPENED, dlg.GetSelectedPath());
			return;
		}

		doc.SaveSection(same, path, dlg.GetSelectedPath(), dlg.mSaveArea, dlg.mFileFormat, dlg.mSamplingRate);

		// only downsample if they changed the sampling rate
		if (samplesPerSec != dlg.mSamplingRate) {
			CScopedStatusBar scopedStatusBar(IDS_RESAMPLE_WAVE);
			CWaveResampler resampler;
			CWaveResampler::ECONVERT result = resampler.Resample(dlg.GetSelectedPath(), dlg.GetSelectedPath(), dlg.mSamplingRate, scopedStatusBar);
			if (result != CWaveResampler::EC_SUCCESS) {
				app.ErrorMessage(IDS_ERROR_DOWNSAMPLE, dlg.GetSelectedPath());
				return;
			}
		}

		switch (dlg.mShowFiles) {
		case showBoth:
			// Open new document
			app.OpenDocumentFile(dlg.GetSelectedPath());
			break;
		case showNew:
			doc.CloseAutoSave(path);
			doc.OnCloseDocument();
			// Open new document
			app.OpenDocumentFile(dlg.GetSelectedPath());
			break;
		case showOriginal:
			// show nothing
			break;
		}
	}
}

/***************************************************************************/
// CSaView::OnUpdateFileSaveAs Menu Update
/***************************************************************************/
void CSaView::OnUpdateFileSaveAs(CCmdUI * pCmdUI) {
	CSaApp * pApp = (CSaApp*)AfxGetApp();
	pCmdUI->Enable(pApp->GetBatchMode() == 0); // SDM 1.5Test8.2
}

/**
* automatically generate CV data
*/
void CSaView::OnGenerateCVData() {

	// verify markup exists for phonetic
	CSaDoc * pModel = GetDocument();

	pModel->CheckPoint();

	// we will assume for now that the user will provide the phonetic segmentation..
	CSegment * pTone = pModel->GetSegment(TONE);
	if (pTone->GetOffsetSize() != 0) {
		bool empty = true;
		for (int i = 0; i < pTone->GetOffsetSize(); i++) {
			CString text = pTone->GetText(i).Trim();
			if (text.GetLength() > 0) empty = false;
		}
		if (!empty) {
			ErrorMessage(IDS_ERROR_GENERATE_CV_TONE);
			return;
		}
	}

	{
		CScopedCursor cursor(this);
		// automatically create CV data for the Tone Segment
		pModel->GenerateCVData(*this);
	}

	// show tone on the .. graph
	int i = GetGraphIndexForIDD(IDD_RAWDATA);
	if ((i != -1) && (m_apGraphs[i] != NULL)) {
		m_apGraphs[i]->ShowAnnotation(TONE, (!m_apGraphs[i]->HaveAnnotation(TONE)), TRUE);
	}

	RedrawGraphs(TRUE);
}

void CSaView::OnExportCVData() {
	CSaDoc * pModel = GetDocument();
	CDlgExportSFM dlg(pModel->GetTitle());
	dlg.DoModal();
}

void CSaView::ErrorMessage(int msg) {
	CSaApp * pApp = (CSaApp *)AfxGetApp();
	pApp->ErrorMessage(msg);
}

void CSaView::ErrorMessage(int msg, LPCTSTR param) {
	CSaApp * pApp = (CSaApp *)AfxGetApp();
	pApp->ErrorMessage(msg, param);
}

void CSaView::ErrorMessage(CSaString & msg) {
	CSaApp * pApp = (CSaApp *)AfxGetApp();
	pApp->ErrorMessage(msg);
}

/**
* Returns true if any segment within this selected segments
* range contains data.
*/
bool CSaView::AnySegmentHasData(CSegment * pSeg, int sel) {
	if (sel == -1) return false;
	if (pSeg == NULL) return false;
	DWORD offset = pSeg->GetOffset(sel);
	DWORD stop = pSeg->GetStop(sel);
	for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
		CSegment * pOther = GetAnnotation(i);
		if (pOther->ContainsText(offset, stop)) {
			return true;
		}
	}
	return false;
}

CMainFrame & CSaView::GetMainFrame() {
	return *((CMainFrame*)AfxGetMainWnd());
}

CSaApp & CSaView::GetApp() {
	return *((CSaApp*)AfxGetApp());
}

/**
* Selects the gloss segment at the present playback cursor position
*/
void CSaView::SelectSegment() {
	return;
}

/**
* Returns the graph position according the current mouse location
* Returns -1 if we can't determine it
* We either accept input from the graph, or the transcription bars
*/
DWORD CSaView::CalculatePositionFromMouse() {

	// prevent crash
	if (m_pFocusedGraph == NULL) {
		TRACE("no graph in focus\n");
		return -1;
	}

	CPlotWnd * pPlot = m_pFocusedGraph->GetPlot();
	if (pPlot == NULL) {
		TRACE("no plot in focus\n");
		return -1;
	}

	// set the new positions
	// get pointer to document
	CSaDoc * pModel = GetDocument();

	CPoint point = m_pFocusedGraph->GetPopupMenuPosition();
	if ((point.x == UNDEFINED_OFFSET) && (point.y == UNDEFINED_OFFSET)) {
		return -1;
	}

	// calculate plot client coordinates
	CRect rWnd;
	pPlot->GetClientRect(rWnd);
	int nWidth = rWnd.Width();

	// get actual data position, frame and data size and alignment
	double fDataPos = 0;
	DWORD dwDataFrame = 0;
	// check if area graph type
	if (m_pFocusedGraph->IsAreaGraph()) {
		// get necessary data from area plot
		fDataPos = pPlot->GetAreaPosition();
		dwDataFrame = pPlot->GetAreaLength();
	} else {
		// get necessary data from document and from view
		// data index of first sample to display
		fDataPos = GetDataPosition(nWidth);
		// number of data points to display
		dwDataFrame = CalcDataFrame(nWidth);
	}

	DWORD dwDataSize = pModel->GetDataSize();
	DWORD nSmpSize = pModel->GetSampleSize();
	// calculate data samples per pixel
	ASSERT(rWnd.Width());
	double fSamplesPerPix = (double)dwDataFrame / (double)(nWidth*nSmpSize);

	// calculate the start cursor position
	DWORD position = (DWORD)round2Int(fDataPos / nSmpSize + ((double)point.x) * fSamplesPerPix);
	position = position*nSmpSize;
	// check the range
	if (position < (DWORD)nSmpSize) {
		position = (DWORD)nSmpSize;
	}
	// check the range
	if (position >= (dwDataSize - (DWORD)nSmpSize)) {
		position = dwDataSize - (DWORD)nSmpSize;
	}

	return position;
}

bool CSaView::CanMoveDataLeft(CSegment * pSeg, bool discrete) {

	CSaDoc * pModel = GetDocument();
	if (pSeg == NULL) {
		return false;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		return false;
	}

	// we will be creating a new segment in combined mode
	// so we don't need to check
	if (discrete) {
		if (AnySegmentHasData(pSeg, sel)) {
			TRACE("segment contains data\n");
			return false;
		}
	}

	// standard case
	if (sel == pSeg->GetOffsetSize() - 1) {
		return false;
	}
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	bool segmental = pModel->IsSegmental(pPhonetic, sel);
	return !segmental;
}

bool CSaView::CanMoveDataLeftAt(CSaDoc * pModel, CPhoneticSegment * pSeg, DWORD position, bool discrete) {

	if (pSeg == NULL) {
		return false;
	}
	int sel = pSeg->FindWithin(position);
	if (sel == -1) {
		return false;
	}

	// we will be creating a new segment in combined mode
	// so we don't need to check
	if (discrete) {
		if (AnySegmentHasData(pSeg, sel)) {
			return false;
		}
	}

	// standard case
	if (sel == pSeg->GetOffsetSize() - 1) {
		return false;
	}
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	bool segmental = pModel->IsSegmental(pPhonetic, sel);
	if (segmental) {
		return false;
	}
	return true;
}

bool CSaView::CanMoveDataRight(CSegment * pSeg) {
	if (pSeg == NULL) {
		return false;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		return false;
	}

	CSaDoc * pModel = GetDocument();

	// standard case
	// allow move right on last segment
	// in sab mode, data is just swapped
	// in non-sab mode, segment is added
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	bool segmental = pModel->IsSegmental(pPhonetic, sel);
	return !segmental;
}

bool CSaView::CanMoveDataRightNext(CSegment * pSeg) {
	TRACE("CanMoveDataRightNext\n");
	if (pSeg == NULL) {
		TRACE("no segment\n");
		return false;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		TRACE("no selection\n");
		return false;
	}

	sel += 1;
	if (sel >= pSeg->GetOffsetSize()) {
		TRACE("at end\n");
		return false;
	}

	CSaDoc * pModel = GetDocument();

	// standard case
	// allow move right on last segment
	// in sab mode, data is just swapped
	// in non-sab mode, segment is added
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	bool segmental = pModel->IsSegmental(pPhonetic, sel);
	return !segmental;
}

bool CSaView::CanMoveDataRightSel(CSegment * pSeg, int sel) {

	if (pSeg == NULL) {
		return false;
	}
	if (sel == -1) {
		return false;
	}
	if (sel >= pSeg->GetOffsetSize()) {
		return false;
	}

	CSaDoc * pModel = GetDocument();

	// standard case
	// allow move right on last segment
	// in sab mode, data is just swapped
	// in non-sab mode, segment is added
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	bool segmental = pModel->IsSegmental(pPhonetic, sel);
	return !segmental;
}

bool CSaView::CanMoveDataRightAt(CPhoneticSegment * pSeg, DWORD position) {
	if (pSeg == NULL) {
		return false;
	}
	int sel = pSeg->FindWithin(position);
	if (sel == -1) {
		return false;
	}
	CSaDoc * pModel = GetDocument();

	// standard case
	// allow move right on last segment
	// in sab mode, data is just swapped
	// in non-sab mode, segment is added
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	bool segmental = pModel->IsSegmental(pPhonetic, sel);
	return !segmental;
}

bool CSaView::CanMoveDataRightNextAt(CPhoneticSegment * pSeg, DWORD position) {
	if (pSeg == NULL) {
		return false;
	}
	int sel = pSeg->FindWithin(position);
	if (sel == -1) {
		return false;
	}
	sel += 1;
	if (sel >= pSeg->GetOffsetSize()) {
		return false;
	}
	CSaDoc * pModel = GetDocument();

	// standard case
	// allow move right on last segment
	// in sab mode, data is just swapped
	// in non-sab mode, segment is added
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	bool segmental = pModel->IsSegmental(pPhonetic, sel);
	return !segmental;
}

bool CSaView::CanSplit(CSegment * pSeg) {

	if (pSeg == NULL) {
		return false;
	}
	// no data
	if (pSeg->GetOffsetSize() == 0) {
		return false;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		// can we find the position from the mouse?
		DWORD pos = CalculatePositionFromMouse();
		if (pos == -1) {
			return false;
		}
	}

	CSaDoc * pModel = GetDocument();
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)pSeg;
	if (pModel->IsSegmental(pPhonetic, sel)) return false;
	return true;
}

bool CSaView::CanSplitAt(CSaDoc * pModel, CPhoneticSegment * pSeg, DWORD position) {

	if (pSeg == NULL) {
		return false;
	}
	// no data
	if (pSeg->GetOffsetSize() == 0) {
		return false;
	}

	int sel = pSeg->FindWithin(position);
	if (sel == -1) {
		return false;
	}

	// if it's too near a start or stop cursor - ignore it
	DWORD start = pSeg->GetOffset(sel);
	DWORD stop = pSeg->GetStop(sel);
	//TRACE("start=%d position=%d stop=%d\n",start,position,stop);
	DWORD error = ((start > position) ? (start - position) : (position - start));
	if (error<3) {
		return false;
	}
	error = ((stop>position) ? (stop - position) : (position - stop));
	if (error < 3) {
		return false;
	}

	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)pSeg;
	if (pModel->IsSegmental(pPhonetic, sel)) {
		return false;
	}
	return true;
}

bool CSaView::CanMerge(CSegment * pSeg) {

	if (pSeg == NULL) {
		return false;
	}
	// no data
	if (pSeg->GetOffsetSize() == 0) {
		return false;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		return false;
	}
	// can't merge first segment
	if (sel == 0) {
		return false;
	}

	CSaDoc * pModel = GetDocument();
	// phonetic not selected
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	// if we are in a segmental transcription, we can't merge left
	if ((pModel->IsBoundary(pPhonetic, sel)) && (pModel->IsSegmental(pPhonetic, sel))) {
		//TRACE("current segment is boundary-segmental\n");
		return false;
	}
	// if we are not segmental, but the segment to our
	// left is segmental, we cannot merge.
	if ((pModel->IsBoundary(pPhonetic, sel)) && (pModel->IsSegmental(pPhonetic, sel - 1))) {
		//TRACE("adjacent segment is boundary-segmental\n");
		return false;
	}
	return true;
}

bool CSaView::CanMergeNext(CSegment * pSeg) {

	if (pSeg == NULL) {
		return false;
	}
	// no data
	if (pSeg->GetOffsetSize() == 0) {
		return false;
	}
	int sel = pSeg->GetSelection();
	if (sel == -1) {
		return false;
	}

	sel += 1;
	if (sel >= pSeg->GetOffsetSize()) {
		return false;
	}

	CSaDoc * pModel = GetDocument();
	// phonetic not selected
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	// if we are in a segmental transcription, we can't merge left
	if ((pModel->IsBoundary(pPhonetic, sel)) && (pModel->IsSegmental(pPhonetic, sel))) {
		//TRACE("current segment is boundary-segmental\n");
		return false;
	}
	// if we are not segmental, but the segment to our
	// left is segmental, we cannot merge.
	if ((pModel->IsBoundary(pPhonetic, sel)) && (pModel->IsSegmental(pPhonetic, sel - 1))) {
		//TRACE("adjacent segment is boundary-segmental\n");
		return false;
	}
	return true;
}

bool CSaView::CanMergeAt(CSaDoc * pModel, CPhoneticSegment * pSeg, DWORD position) {

	if (pSeg == NULL) {
		return false;
	}
	// no data
	if (pSeg->GetOffsetSize() == 0) {
		return false;
	}
	int sel = pSeg->FindWithin(position);
	if (sel == -1) {
		return false;
	}
	// can't merge first segment
	if (sel == 0) {
		return false;
	}

	// phonetic not selected
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	// if we are in a segmental transcription, we can't merge left
	if ((pModel->IsBoundary(pPhonetic, sel)) && (pModel->IsSegmental(pPhonetic, sel))) {
		//TRACE("current segment is boundary-segmental\n");
		return false;
	}
	// if we are not segmental, but the segment to our
	// left is segmental, we cannot merge.
	if ((pModel->IsBoundary(pPhonetic, sel)) && (pModel->IsSegmental(pPhonetic, sel - 1))) {
		//TRACE("adjacent segment is boundary-segmental\n");
		return false;
	}
	return true;
}

bool CSaView::CanMergeAtNext(CSaDoc * pModel, CPhoneticSegment * pSeg, DWORD position) {

	if (pSeg == NULL) {
		return false;
	}
	// no data
	if (pSeg->GetOffsetSize() == 0) {
		return false;
	}
	int sel = pSeg->FindWithin(position);
	if (sel == -1) {
		return false;
	}

	sel += 1;

	// can't merge first segment
	if (sel >= pSeg->GetOffsetSize()) {
		return false;
	}

	// phonetic not selected
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	// if we are in a segmental transcription, we can't merge left
	if ((pModel->IsBoundary(pPhonetic, sel)) && (pModel->IsSegmental(pPhonetic, sel))) {
		//TRACE("current segment is boundary-segmental\n");
		return false;
	}
	// if we are not segmental, but the segment to our
	// left is segmental, we cannot merge.
	if ((pModel->IsBoundary(pPhonetic, sel)) && (pModel->IsSegmental(pPhonetic, sel - 1))) {
		//TRACE("adjacent segment is boundary-segmental\n");
		return false;
	}
	return true;
}

bool CSaView::CanMergeSel(CPhoneticSegment * pSeg, int sel) {
	if (pSeg == NULL) {
		return false;
	}
	// no data
	if (pSeg->GetOffsetSize() == 0) {
		return false;
	}
	if (sel == -1) {
		return false;
	}
	// can't merge first segment
	if (sel == 0) {
		return false;
	}

	CSaDoc * pModel = GetDocument();

	// phonetic not selected
	if (!pSeg->Is(PHONETIC)) {
		return false;
	}
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)pSeg;
	// if we are in a segmental transcription, we can't merge left
	if ((pModel->IsBoundary(pPhonetic, sel)) && (pModel->IsSegmental(pPhonetic, sel))) {
		//TRACE("current segment is boundary-segmental\n");
		return false;
	}
	// if we are not segmental, but the segment to our
	// left is segmental, we cannot merge.
	if ((pModel->IsBoundary(pPhonetic, sel)) && (pModel->IsSegmental(pPhonetic, sel - 1))) {
		//TRACE("adjacent segment is boundary-segmental\n");
		return false;
	}
	return true;
}

void CSaView::DeselectAnnotations() {
	m_advancedSelection.DeselectAnnotations(this);
}
void CSaView::OnNextError() {
}
void CSaView::OnPreviousError() {
}

// split and join operations based on begin cursor
void CSaView::OnSplitMoveLeftAt() {
	DWORD position = m_dwStartCursor;
	if (position == -1) {
		return;
	}
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	m_advancedSelection.SelectFromPosition(this, PHONETIC, position, false);
	DWORD newPosition = EditSplitAt(position);
	EditMoveLeftAt(pModel, newPosition + 1);
}

void CSaView::OnUpdateSplitMoveLeftAt(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)pModel->GetSegment(PHONETIC);
	BOOL enable = FALSE;
	DWORD position = m_dwStartCursor;
	if (position != -1) {
		if (CanSplitAt(pModel, pPhonetic, position)) {
			if (CanMoveDataLeftAt(pModel, pPhonetic, position, false)) {
				enable = TRUE;
			}
		}
	}
	pCmdUI->Enable(enable);
}

void CSaView::OnUpdateMoveRightMergePrevAt(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pSeg = (CPhoneticSegment*)pModel->GetSegment(PHONETIC);
	DWORD position = m_dwStartCursor;
	BOOL enable = FALSE;
	int sel = pSeg->FindWithin(position);
	if (sel != -1) {
		if (CanMoveDataRightSel(pSeg, sel)) {
			if (CanMergeSel(pSeg, sel)) {
				enable = TRUE;
			}
		}
	}
	pCmdUI->Enable(enable);
}

void CSaView::OnMoveRightMergePrevAt() {
	DWORD position = m_dwStartCursor;
	if (position == -1) {
		return;
	}
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	m_advancedSelection.SelectFromPosition(this, PHONETIC, position, false);
	EditMoveRight();
	EditMerge();
}

void CSaView::OnUpdateMoveRightMergeNextAt(CCmdUI * pCmdUI) {
	CSaDoc * pModel = GetDocument();
	CPhoneticSegment * pSeg = (CPhoneticSegment*)pModel->GetSegment(PHONETIC);
	BOOL enable = FALSE;
	DWORD position = m_dwStartCursor;
	int sel = pSeg->FindWithin(position);
	if (sel != -1) {
		sel += 1;
		if (CanMoveDataRightSel(pSeg, sel)) {
			if (CanMergeSel(pSeg, sel)) {
				enable = TRUE;
			}
		}
	}
	pCmdUI->Enable(enable);
}

void CSaView::OnMoveRightMergeNextAt() {
	DWORD position = m_dwStartCursor;
	if (position == -1) {
		return;
	}
	CSaDoc * pModel = GetDocument();
	pModel->CheckPoint();
	m_advancedSelection.SelectFromPosition(this, PHONETIC, position, false);
	EditMoveRightNext();
	EditMerge();
}

