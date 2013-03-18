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
//        SDM changed OnEditAdd to Join only if segments closer than MAX_ADD_JOIN_TIME
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
//        SDM replaced GetTexts()->GetSize() with GetSize()
// 1.5Test11.3
//         SDM changed OnEditAdd to support new Gloss positions
//         SDM changed CSegment::AdjustPositionAll to CSegment::Adjust
// 03/29/2000
//        RLJ Modified File-->Export to support "Export MIDI"
// 06/01/2000
//        RLJ Added ChangeDisplayedGraphs(int OpenMode)
//        RLJ In order to reduce Sa_view.obj size enough for LINK/DEBUG to work:
//              Added pSaApp, pViewMainFrame, and pSourceParm.
//              Eliminated unused pDoc definitions.
//              For any function in which pDoc was defined but only used once,
//                eliminated the definition and replaced the reference with
//                a direct GetDocument() call.
//              For any function in which GetDocument() was directly called
//                more than once, defined pDoc and replaced original
//                GetDocument() calls with pDoc references.
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
#include "sa_g_raw.h"
#include "sa_g_rec.h"
#include "dsp\dspTypes.h"
#include "Process\Process.h"
#include "fileInformation.h"
#include "playerRecorder.h"
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
#include "settings\obstream.h"
#include "DlgExportFW.h"
#include "sa_g_stf.h"
#include "Segment.h"
#include "graphsTypes.h"
#include "resource.h"
#include "DlgExportXML.h"
#include "Import.h"
#include "DlgExportSFM.h"
#include "DlgExportTable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

Object_istream * CSaView::s_pobsAutoload = NULL;
static const char * psz_sagraph      = "sagraph";


/***************************************************************************/
// 09/28/2000 - DDO This function will create a graph object and read its
//                  properties depending on how it should be created. I
//                  wrote this to get it out of the CreateGraphs functions
//                  which was too bloated.
/***************************************************************************/
void CSaView::CreateOneGraphStepOne(UINT nID, CGraphWnd ** pGraph, CREATE_HOW how,
                                    CGraphWnd * pFromGraph, Object_istream * pObs) {
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

        case CREATE_FROMSCRATCH:  // Use program defaults
            if (*pGraph) {
                (*pGraph)->bSetProperties(nID);
                m_WeJustReadTheProperties = TRUE;
            }
            break;
        default:
            ;  // Use current user settings
        }
        CreateOneGraph(&nID, pGraph);
        if (!nID) {
            return;
        }
    }

    (*pGraph)->SetCaptionStyle(pViewMainFrame->GetCaptionStyle());     // set caption style
    (*pGraph)->SetMagnify(m_fMagnify);                                 // set the magnify factor

    switch (nID) {
        //****************************************************
        // Set properties for raw data graphs
        //****************************************************
    case IDD_RAWDATA:
        (*pGraph)->ShowGrid(TRUE);
        (*pGraph)->ShowBoundaries(!m_bBoundariesNone);
        if (!m_WeJustReadTheProperties) {
            (*pGraph)->ShowLegend(!m_bLegendNone);
            (*pGraph)->ShowXScale(!m_bXScaleNone);
            for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
                (*pGraph)->ShowAnnotation(i, !m_abAnnNone[i]);
            }
        }
        break;

        //****************************************************
        // Do this for all graphs but RAW and POA.
        //****************************************************
    default:
        if (nID != IDD_POA) {
            (*pGraph)->ShowGrid(TRUE);
            (*pGraph)->ShowBoundaries(m_bBoundariesAll);

            if (!m_WeJustReadTheProperties) {
                (*pGraph)->ShowLegend(m_bLegendAll);
                (*pGraph)->ShowXScale(m_bXScaleAll);
                for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
                    (*pGraph)->ShowAnnotation(i, m_abAnnAll[i]);
                }
            }
        }
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CSaView message handlers, Part I

/***************************************************************************/
// CSaView::SendPlayMessage Send specified IDC_PLAY message to player.
/***************************************************************************/
void CSaView::SendPlayMessage(WORD Int1, WORD Int2) {
    //    pViewMainFrame->SendMessage(WM_USER_PLAYER, IDC_PLAY, MAKELONG(Int1, Int2)); // send message to start player
    DWORD lParam = MAKELONG(Int1, Int2);
    pViewMainFrame->SendMessage(WM_USER_PLAYER, IDC_PLAY, lParam); // send message to start player
}

/***************************************************************************/
// CSaView::OnPlaybackCursors Playback between cursors
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackCursors() {
    SendPlayMessage(ID_PLAYBACK_CURSORS, FALSE); // send message to start player
}

/***************************************************************************/
// CSaView::OnPlaybackSegment Playback current selected segment
/***************************************************************************/
DWORD CSaView::OnPlaybackSegment() {
    DWORD dwStart = GetStartCursorPosition();
    DWORD dwStop = GetStopCursorPosition();

    CSegment * pSelected = FindSelectedAnnotation();

    if (pSelected) { // Adjust Cursors to Current Boundaries
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
    CSaDoc * pDoc = GetDocument();
    if (!pDoc) {
        return 0;    //no document
    }
    CSegment * pSegment = pDoc->GetSegment(GLOSS);
    CSegment * pSelected = FindSelectedAnnotation();
    int nActualGloss = -1;
    int nSelection = -1;
    DWORD dwStart = GetStartCursorPosition();
    DWORD dwStop = GetStopCursorPosition();

    if (pSelected) {
        nSelection = pSelected->GetSelection();
        nActualGloss = pSegment->FindFromPosition(pSelected->GetOffset(nSelection), TRUE);
    }
    if (nActualGloss == -1) {
        // nothing within, check if there is gloss
        if (pSegment->GetOffsetSize() > 0) {
            // there is gloss, so the segment must be below the first gloss
            pSegment->SelectSegment(*pDoc, 0); // select first gloss
            // playback below start cursor
            SetStartCursorPosition(0);
            SetStopCursorPosition(pSegment->GetOffset(0));
        } else {
            // there is no gloss, playback the whole file
            OnPlaybackFile();
            return pDoc->GetDataSize();
        }
    } else {
        if (pSegment->GetSelection()==nActualGloss) {
            pSegment->SelectSegment(*pDoc, -1);
        }
        pSegment->SelectSegment(*pDoc, nActualGloss);
    }

    DWORD dwReturn = GetStopCursorPosition() - GetStartCursorPosition();

    // playback
    OnPlaybackCursors();

    pSegment->SelectSegment(*pDoc, -1);

    // set back actual segment
    if (nSelection != -1) {
        // Select segment (do not toggle off.)
        if (pSelected->GetSelection()!=nSelection) {
            pSelected->SelectSegment(*pDoc, nSelection);
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
    CSaDoc * pDoc = GetDocument();
    if (!pDoc) {
        return 0;    //no document
    }
    CSegment * pSegment = pDoc->GetSegment(MUSIC_PL1);
    CSegment * pSelected = FindSelectedAnnotation();
    int nActualPhrase = -1;
    int nSelection = -1;
    DWORD dwStart = GetStartCursorPosition();
    DWORD dwStop = GetStopCursorPosition();

    if (pSelected) {
        nSelection = pSelected->GetSelection();
        nActualPhrase = pSegment->FindFromPosition(pSelected->GetOffset(nSelection), TRUE);
    }
    if (nActualPhrase == -1) {
        // nothing within, check if there is phrase
        if (pSegment->GetOffsetSize() > 0) {
            // there is phrase, so the segment must be below the first phrase
            pSegment->SelectSegment(*pDoc, 0); // select first phrase
            // playback below start cursor
            SetStartCursorPosition(0);
            SetStopCursorPosition(pSegment->GetOffset(0));
        } else {
            // there is no gloss, playback the whole file
            OnPlaybackFile();
            return pDoc->GetDataSize();
        }
    } else {
        if (pSegment->GetSelection()==nActualPhrase) {
            pSegment->SelectSegment(*pDoc, -1);
        }
        pSegment->SelectSegment(*pDoc, nActualPhrase);
    }

    DWORD dwReturn = GetStopCursorPosition() - GetStartCursorPosition();

    // playback
    OnPlaybackCursors();

    pSegment->SelectSegment(*pDoc, -1);

    // set back actual segment
    if (nSelection != -1) {
        // Select segment (do not toggle off.)
        if (pSelected->GetSelection()!=nSelection) {
            pSelected->SelectSegment(*pDoc, nSelection);
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

    FnKeys * pKeys = ((CMainFrame *)pViewMainFrame)->GetFnKeys(0);
    pKeys->bRepeat[Player_Slow] = FALSE;     // TRUE, if playback repeat enabled
    pKeys->nDelay[Player_Slow] = 100;        // repeat delay time in ms
    pKeys->nMode[Player_Slow] = ID_PLAYBACK_CURSORS;       // replay mode
    pKeys->nSpeed[Player_Slow] = 50;        // default replay speed in %
    pKeys->nVolume[Player_Slow] = 50;       // default play volume in %

    pViewMainFrame->PostMessage(WM_USER_PLAYER, IDC_PLAY, MAKELONG(Player_Slow, -1));
}

/***************************************************************************/
// CSaView::OnPlaybackFile Playback the hole wave file
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackFile() {
    SendPlayMessage(ID_PLAYBACK_FILE, FALSE); // send message to start player
}

/***************************************************************************/
// CSaView::OnPlaybackWindow Playback the displayed data
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackWindow() {
    SendPlayMessage(ID_PLAYBACK_WINDOW, FALSE); // send message to start player
}

/***************************************************************************/
// CSaView::OnPlaybackLtostart Playback left window border to start cursor
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackLtostart() {
    SendPlayMessage(ID_PLAYBACK_LTOSTART, FALSE); // send message to start player
}

/***************************************************************************/
// CSaView::OnPlaybackStarttor Playback start cursor to right window border
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackStarttor() {
    SendPlayMessage(ID_PLAYBACK_STARTTOR, FALSE); // send message to start player
}

/***************************************************************************/
// CSaView::OnPlaybackLtoStop Playback left window border to stop cursor
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackLtoStop() {
    SendPlayMessage(ID_PLAYBACK_LTOSTOP, FALSE); // send message to start player
}

/***************************************************************************/
// CSaView::OnPlaybackStopToR Playback stop cursor to right window border
// The mainframe is informed and it will launch the player. The player
// message takes as wParam the player mode, in the lower word of lParam the
// submode and in the higher word if it will be launched in full size (TRUE)
// or small (FALSE).
/***************************************************************************/
void CSaView::OnPlaybackStopToR() {
    SendPlayMessage(ID_PLAYBACK_STOPTOR, FALSE); // send message to start player
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
    pViewMainFrame->SendMessage(WM_USER_PLAYER, IDC_STOP, MAKELONG(-1, FALSE)); // send message to stop player
}

/***************************************************************************/
// CSaView::OnUpdatePlayerStop Menu update
/***************************************************************************/
void CSaView::OnUpdatePlayerStop(CCmdUI * pCmdUI) {
    pCmdUI->Enable(pViewMainFrame->IsPlayerPlaying()); // enable if player is playing
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
    pViewMainFrame->SendMessage(WM_USER_PLAYER, IDC_STOP, MAKELONG(-1, TRUE)); // send message to start player
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
// CSaView::OnExportFW Export wave file data using Standard Format Markers
//
// Modified on 07/27/2000
/***************************************************************************/
void CSaView::OnExportFW() {
    CSaDoc * pDoc = GetDocument();
    int count = pDoc->GetSegmentSize(REFERENCE);
    if (count==0) {
        CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
        pApp->ErrorMessage(IDS_ERROR_NO_REFERENCE);
        return;
    }

    CSaString title = pDoc->GetTitle();
    BOOL gloss = pDoc->HasSegmentData(GLOSS);
    BOOL ortho = pDoc->HasSegmentData(ORTHO);
    BOOL phonemic = pDoc->HasSegmentData(PHONEMIC);
    BOOL phonetic = pDoc->HasSegmentData(PHONETIC);
    BOOL pos = FALSE;
    BOOL reference = pDoc->HasSegmentData(REFERENCE);
    BOOL phrase = pDoc->HasSegmentData(MUSIC_PL1)|pDoc->HasSegmentData(MUSIC_PL1);

    CDlgExportFW dlg(title, gloss, ortho, phonemic, phonetic, pos, reference, phrase);
    if (dlg.DoModal()==IDOK) {
        pDoc->DoExportFieldWorks(dlg.settings);
    }
}

/***************************************************************************/
// CSaView::OnExportSFM Export wave file data using Standard Format Markers
//
// Modified on 07/27/2000
/***************************************************************************/
void CSaView::OnExportSFM() {
    CDlgExportSFM dlg(((CSaDoc *)GetDocument())->GetTitle());
    dlg.DoModal();
}

/***************************************************************************/
// CSaView::OnExportTimeTable Export wave file data as time table
/***************************************************************************/
void CSaView::OnExportTimeTable() {
    CDlgExportTable dlg(((CSaDoc *)GetDocument())->GetTitle());
    dlg.DoModal();
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
    CSaDoc* pDoc = (CSaDoc*)GetDocument();
    CSaString szTitle;
    szTitle = pDoc->GetTitle();                                  // load file name
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

    szString = "Wave file is " + pDoc->GetPathName() + szCrLf;
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
    CSaString szTitle;
    szTitle = ((CSaDoc *)GetDocument())->GetTitle();                                 // load file name
    int nFind = szTitle.Find(':');
    if (nFind != -1) {
        szTitle = szTitle.Left(nFind);    // extract part left of :
    }
    nFind = szTitle.ReverseFind('.');
    if (nFind >= ((szTitle.GetLength() > 3) ? (szTitle.GetLength()-4) : 0)) {
        szTitle = szTitle.Left(nFind);    // remove extension
    }
    CSaString szFilter = "Standard Format (*.sfm) |*.sfm|All Files (*.*) |*.*||";
    CFileDialog dlgFile(TRUE,_T("sfm"),szTitle,OFN_HIDEREADONLY,szFilter,NULL);
    if (dlgFile.DoModal()!=IDOK) {
        return;
    }

    CSaString szPath = dlgFile.GetPathName();
    CImport import(szPath);
    import.Import();
}

/***************************************************************************/
// CSaView::ImportSFT Import wave file data
/***************************************************************************/
void CSaView::OnImportSFT() {
    // Get Export File Type and Name
    CSaString szTitle;
    szTitle = ((CSaDoc *)GetDocument())->GetTitle();                                 // load file name
    int nFind = szTitle.Find(':');
    if (nFind != -1) {
        szTitle = szTitle.Left(nFind);    // extract part left of :
    }
    nFind = szTitle.ReverseFind('.');
    if (nFind >= ((szTitle.GetLength() > 3) ? (szTitle.GetLength()-4) : 0)) {
        szTitle = szTitle.Left(nFind);    // remove extension
    }
    CSaString szFilter = "Standard Format (*.sft) |*.sft||";
    CFileDialog dlgFile(TRUE,_T("sft"),szTitle,OFN_HIDEREADONLY,szFilter,NULL);
    if (dlgFile.DoModal()!=IDOK) {
        return;
    }

    CSaString szPath = dlgFile.GetPathName();

    CImport import(szPath);
    import.Import();
}

void CSaView::OnFilePhonologyAssistant() {
    if (AfxMessageBox(IDS_ENABLE_PA, MB_OKCANCEL, 0)!=IDOK) {
        return;
    }

    OSVERSIONINFO OSInfo;
    ZeroMemory(&OSInfo, sizeof(OSVERSIONINFO));
    OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    ::GetVersionEx(&OSInfo);
    bool vista = (OSInfo.dwMajorVersion>=6);

    /*
    WOW64 is the x86 emulator that allows 32-bit Windows-based applications to run seamlessly on 64-bit Windows.
    WOW64 is provided with the operating system and does not have to be explicitly enabled.
    For more information, see WOW64 Implementation Details. http://msdn.microsoft.com/en-us/library/aa384274%28v=vs.85%29.aspx
    */
    BOOL wow64 = FALSE;
    ::IsWow64Process(GetCurrentProcess(),&wow64);

    TRACE("running Vista or Greater = %d\n",vista);
    TRACE("running on x64 = %d\n",wow64);

    // obtain the location of the currently running app.
    CString exeName = L"";
    exeName.Append(AfxGetApp()->m_pszExeName);
    exeName.Append(L".exe");
    HMODULE hmod = GetModuleHandle(exeName);

    TCHAR fullPath[_MAX_PATH + 1];
    ::GetModuleFileName(hmod, fullPath, MAX_PATH);

    // write to both the 32-bit and 64-bit hives...

    // KEY_WOW64_64KEY allows us to write to the 64-bit hive
    // wow64 is true if we are a 32-bit app on a 64-bit system.
    // we now write to the 64-bit hive
    if (wow64) {
        REGSAM sam = KEY_ALL_ACCESS|KEY_WOW64_64KEY;
        HKEY hKey = 0;
        DWORD disposition = 0;
        LONG lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\SIL\\Speech Analyzer"), 0, NULL, 0, sam, NULL, &hKey, &disposition);
        if (lResult!=ERROR_SUCCESS) {
            AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK|MB_ICONEXCLAMATION, 0);
            return;
        }
        DWORD len = (wcslen(fullPath)+1)*sizeof(TCHAR);
        lResult = RegSetValueEx(hKey, L"Location", 0, REG_SZ, (const BYTE *)fullPath, len);
        if (lResult!=ERROR_SUCCESS) {
            RegCloseKey(hKey);
            AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK|MB_ICONEXCLAMATION, 0);
            return;
        }
        lResult = RegCloseKey(hKey);
        if (lResult!=ERROR_SUCCESS) {
            AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK|MB_ICONEXCLAMATION, 0);
            return;
        }
    }

    // KEY_WOW64_64KEY allows us to write to the 64-bit hive
    // wow64 is true if we are a 32-bit app on a 64-bit system.
    // we now write to the 32-bit hive
    REGSAM sam = KEY_ALL_ACCESS;
    HKEY hKey = 0;
    DWORD disposition = 0;
    LONG lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\SIL\\Speech Analyzer"), 0, NULL, 0, sam, NULL, &hKey, &disposition);
    if (lResult!=ERROR_SUCCESS) {
        AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK|MB_ICONEXCLAMATION, 0);
        return;
    }
    DWORD len = (wcslen(fullPath)+1)*sizeof(TCHAR);
    lResult = RegSetValueEx(hKey, L"Location", 0, REG_SZ, (const BYTE *)fullPath, len);
    if (lResult!=ERROR_SUCCESS) {
        RegCloseKey(hKey);
        AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK|MB_ICONEXCLAMATION, 0);
        return;
    }
    lResult = RegCloseKey(hKey);
    if (lResult!=ERROR_SUCCESS) {
        AfxMessageBox(IDS_ERROR_REGISTRY, MB_OK|MB_ICONEXCLAMATION, 0);
        return;
    }
}

/***************************************************************************/
// CSaView::OnFileInformation File information
/***************************************************************************/

void CSaView::OnFileInformation() {
    CSaDoc * pDoc = (CSaDoc *)GetDocument();
    SourceParm * pSourceParm = pDoc->GetSourceParm();
    CSaString szCaption, szTitle;
    szCaption.LoadString(IDS_DLGTITLE_FILEINFO);                // load caption string
    szTitle = pDoc->GetTitle();                                 // load file name
    int nFind = szTitle.Find(':');
    if (nFind != -1) {
        szTitle = szTitle.Left(nFind);                          // extract part left of :
    }
    szCaption += " - " + szTitle;                               // build new caption string

    CDlgFileInformation dlg(szCaption, NULL, 0);                // file information dialog
    // set file description string
    dlg.m_dlgUserPage.m_szFileDesc = pDoc->GetSaParm()->szDescription;
    dlg.m_dlgUserPage.m_szFreeTranslation = pSourceParm->szFreeTranslation;
    if (dlg.DoModal() == IDOK) {
        // get new file description string
        BOOL modified = FALSE;
        if (pDoc->GetSaParm()->szDescription != dlg.m_dlgUserPage.m_szFileDesc) {
            modified = TRUE;
        }
        if (pSourceParm->szFreeTranslation != dlg.m_dlgUserPage.m_szFreeTranslation) {
            modified = TRUE;
        }

        BOOL bCountryChanged     = (pSourceParm->szCountry != dlg.m_dlgSourcePage.m_szCountry);
        BOOL bDailectChanged     = (pSourceParm->szDialect != dlg.m_dlgSourcePage.m_szDialect);
        BOOL bEthnoIDChanged     = (pSourceParm->szEthnoID != dlg.m_dlgSourcePage.m_szEthnoID);
        BOOL bFamilyChanged      = (pSourceParm->szFamily != dlg.m_dlgSourcePage.m_szFamily);
        BOOL bLanguageChanged    = (pSourceParm->szLanguage != dlg.m_dlgSourcePage.m_szLanguage);
        BOOL bGenderChanged      = (pSourceParm->nGender != dlg.m_dlgSourcePage.m_nGender);
        BOOL bRegionChanged      = (pSourceParm->szRegion != dlg.m_dlgSourcePage.m_szRegion);
        BOOL bSpeakerChanged     = (pSourceParm->szSpeaker != dlg.m_dlgSourcePage.m_szSpeaker);
        BOOL bReferenceChanged   = (pSourceParm->szReference != dlg.m_dlgSourcePage.m_szReference);
        BOOL bTranscriberChanged = (pSourceParm->szTranscriber != dlg.m_dlgSourcePage.m_szTranscriber);
        if (bCountryChanged || bDailectChanged || bEthnoIDChanged || bFamilyChanged || bLanguageChanged ||
                bGenderChanged  || bRegionChanged  || bSpeakerChanged  || bReferenceChanged || bTranscriberChanged) {
            modified = TRUE;
        }

        if (modified) {
            pDoc->CheckPoint();
        }


        if (pDoc->GetSaParm()->szDescription != dlg.m_dlgUserPage.m_szFileDesc) {
            pDoc->GetSaParm()->szDescription = dlg.m_dlgUserPage.m_szFileDesc;
            pDoc->SetModifiedFlag(TRUE);                        // document has been modified
            pDoc->SetTransModifiedFlag(TRUE);                   // transcription data has been modified
        }
        if (pSourceParm->szFreeTranslation != dlg.m_dlgUserPage.m_szFreeTranslation) {
            pSourceParm->szFreeTranslation = dlg.m_dlgUserPage.m_szFreeTranslation;
            pDoc->SetModifiedFlag(TRUE);                        // document has been modified
            pDoc->SetTransModifiedFlag(TRUE);                   // transcription data has been modified
        }


        if (bCountryChanged || bDailectChanged || bEthnoIDChanged || bFamilyChanged || bLanguageChanged ||
                bGenderChanged  || bRegionChanged  || bSpeakerChanged  || bReferenceChanged || bTranscriberChanged) {
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
            pDoc->SetModifiedFlag(TRUE);                        // document has been modified
            pDoc->SetTransModifiedFlag(TRUE);                   // transcription data has been modified
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
    ArrangeIconicWindows();    // arrange icons
    WINDOWPLACEMENT wpl;       // find zoomed (maximized) window and set it to normal state

    wpl.length = sizeof(WINDOWPLACEMENT);
    for (int i=0; i < MAX_GRAPHS_NUMBER; i++) {
        if ((m_apGraphs[i]) && (m_apGraphs[i]->IsZoomed())) {
            // window is maximized
            pMaxWnd = m_apGraphs[i];
            pMaxWnd->GetWindowPlacement(&wpl);  // get window placement information
            wpl.showCmd = SW_SHOWNORMAL;        // set to normal
            pMaxWnd->SetWindowPlacement(&wpl);  // set new window placement
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

        if (m_apGraphs[i] && !m_apGraphs[i]->IsIconic() && m_anGraphID[i] != IDD_TWC && m_anGraphID[i] != IDD_MAGNITUDE) {

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
    double dHalfHeight  = pWndRect->Height() / 2.;
    int nHalfWidth   = int(pWndRect->Width() / 2. + 0.5);
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
            double dHeightOfStaff = min(dThirdHeight,MAXIMUM_STAFF_HEIGHT);

            ((nPos == 0) ? pSubRect->bottom : pSubRect->top) = pSubRect->bottom - int(dHeightOfStaff + 0.5);
        } else if (STAFF_IS_FIRST) {
            double dHeightOfStaff = min(dThirdHeight,MAXIMUM_STAFF_HEIGHT);

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
            double dHeightOfStaff = min(dThirdHeight * 2/3, MAXIMUM_STAFF_HEIGHT);

            if (nPos == 0) {
                pSubRect->bottom = pSubRect->top + int(dThirdHeight + 0.5);
            } else if (nPos == 1) {
                pSubRect->bottom -= int(dHeightOfStaff + 0.5);
                pSubRect->top += int(dThirdHeight + 0.5);
            } else {
                pSubRect->top = pSubRect->bottom - int(dHeightOfStaff + 0.5);
            }
        } else if (STAFF_IS_SECOND) {
            double dHeightOfStaff = min(dThirdHeight * 2/3, MAXIMUM_STAFF_HEIGHT);

            if (nPos == 0) {
                pSubRect->bottom = pSubRect->top + int((dThirdHeight * 2) - dHeightOfStaff + 0.5);
            } else if (nPos == 1) {
                pSubRect->bottom = pSubRect->top + int(dThirdHeight * 2 + 0.5);
                pSubRect->top = pSubRect->top + int((dThirdHeight * 2) - dHeightOfStaff + 0.5);
            } else {
                pSubRect->top += int(dThirdHeight * 2 + 0.5);
            }
        } else if (STAFF_IS_FIRST) {
            double dHeightOfStaff = min(dThirdHeight * 2/3, MAXIMUM_STAFF_HEIGHT);

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
    m_bXScaleAll = TRUE; // show x-scale windows in all the graphs
    m_bXScaleNone = FALSE;
    // show x-scale windows
    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
        if (m_apGraphs[nLoop]) {
            m_apGraphs[nLoop]->ShowXScale(m_apGraphs[nLoop]->HaveCursors(), m_apGraphs[nLoop]->HaveCursors());
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
    m_bXScaleAll = FALSE; // show x-scale windows only in raw data graph
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
    m_bXScaleNone = TRUE; // hide x-scale windows in all the graphs
    m_bXScaleAll = FALSE;
    // hide x-scale windows
    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
        if (m_apGraphs[nLoop]) {
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
        m_abAnnAll[nAnnot]  = bShow;  //Show annotation in all graphs
        m_abAnnNone[nAnnot] = !bShow;

        //**************************************************
        // Show annotation window in all but music graphs.
        //**************************************************
        for (int i = 0; i < MAX_GRAPHS_NUMBER; i++) {
            if (m_apGraphs[i]) {
                m_apGraphs[i]->ShowAnnotation(nAnnot, bShow, TRUE);
            }
        }
    } else {
        m_abAnnAll[nAnnot]  = FALSE; // show annotation only in raw data graph
        m_abAnnNone[nAnnot] = FALSE;

        //**************************************************
        // Hide annotation window in all graphs.
        //**************************************************
        for (int i = 0; i < MAX_GRAPHS_NUMBER; i++)
            if (m_apGraphs[i]) {
                m_apGraphs[i]->ShowAnnotation(nAnnot, m_anGraphID[i] == IDD_RAWDATA, TRUE);
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
    int nAnnotationID = nID - ID_POPUPGRAPH_PHONETIC;
    ShowAnnotation(nAnnotationID);
}


/***************************************************************************/
// CSaView::OnUpdatePopupgraphAnnotation Menu update
/***************************************************************************/
void CSaView::OnUpdatePopupgraphAnnotation(CCmdUI * pCmdUI) {
    int nAnnotationID = pCmdUI->m_nID - ID_POPUPGRAPH_PHONETIC;
    BOOL bEnable = (m_pFocusedGraph && GetDocument()->GetDataSize() != 0 && (m_pFocusedGraph->HaveAnnotation(REFERENCE) || !m_pFocusedGraph->DisableAnnotation(nAnnotationID)) && m_nFocusedID != IDD_TWC);
    pCmdUI->Enable(bEnable);
    if (m_pFocusedGraph) {
        pCmdUI->SetCheck(m_pFocusedGraph->HaveAnnotation(nAnnotationID));    // check if graph has reference window
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

/***************************************************************************/
// CSaView::OnBoundariesAll Show the boundaries on all graphs
/***************************************************************************/
void CSaView::OnBoundariesAll() {
    m_bBoundariesAll = TRUE; // show boundaries in all the graphs
    m_bBoundariesNone = FALSE;
    // show boundaries
    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
        if (m_apGraphs[nLoop]) {
            m_apGraphs[nLoop]->ShowBoundaries(TRUE, TRUE);
        }
    }
}

/***************************************************************************/
// CSaView::OnUpdateBoundariesAll Menu update
/***************************************************************************/
void CSaView::OnUpdateBoundariesAll(CCmdUI * pCmdUI) {
    pCmdUI->SetCheck(m_bBoundariesAll);
}

/***************************************************************************/
// CSaView::OnBoundariesRawdata Show the boundaries on raw data graph only
/***************************************************************************/
void CSaView::OnBoundariesRawdata() {
    m_bBoundariesAll = FALSE; // show boundaries only in raw data graph
    m_bBoundariesNone = FALSE;
    // hide boundaries except for raw data
    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
        if (m_apGraphs[nLoop]) {
            m_apGraphs[nLoop]->ShowBoundaries(m_anGraphID[nLoop] == IDD_RAWDATA, TRUE);
        }
}

/***************************************************************************/
// CSaView::OnUpdateBoundariesRawdata Menu update
/***************************************************************************/
void CSaView::OnUpdateBoundariesRawdata(CCmdUI * pCmdUI) {
    pCmdUI->SetCheck(!(m_bBoundariesAll || m_bBoundariesNone));
}

/***************************************************************************/
// CSaView::OnBoundariesNone Hide the boundaries on all graphs
/***************************************************************************/
void CSaView::OnBoundariesNone() {
    m_bBoundariesNone = TRUE; // hide boundaries in all the graphs
    m_bBoundariesAll = FALSE;
    // hide boundaries
    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
        if (m_apGraphs[nLoop]) {
            m_apGraphs[nLoop]->ShowBoundaries(FALSE, TRUE);
        }
    }
}

/***************************************************************************/
// CSaView::OnUpdateBoundariesNone Menu update
/***************************************************************************/
void CSaView::OnUpdateBoundariesNone(CCmdUI * pCmdUI) {
    pCmdUI->SetCheck(m_bBoundariesNone);
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
    DWORD start = ((startReq)/2);
    DWORD stop = ((stopReq+1)/2);
    DWORD offset;

    ASSERT(stop > start);
    ASSERT(percent <= 100);

    offset = (DWORD)((stop - start)*(100-percent)/100.0/2.0);

    // zoom cursors to fill % of screen
    if (start > offset) {
        start = (start - offset)*2;
    } else {
        start = 0;
    }

    stop = (stop + offset)*2;
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
    if ((GetDocument()->GetDataSize() == 0) // nothing to zoom
            || (m_fZoom >= m_fMaxZoom) // zoom limit
            || (m_dwStopCursor == m_dwStartCursor)) { // zoom limit
        enable = FALSE;
    } else {
        // divide by two to guarantee result even
        DWORD start = ((m_dwStartCursor)/2);
        DWORD stop = ((m_dwStopCursor+1)/2);
        DWORD frameStart;
        DWORD frameWidth;
        DWORD offset;

        if (stop <= start) {
            enable = FALSE;
        } else {
            offset = (DWORD)((stop - start)*(100-97)/100.0/2.0);

            // zoom cursors to fill 97% of screen
            if (start > offset) {
                start = (start - offset)*2;
            } else {
                start = 0;
            }

            stop = (stop + offset)*2;
            if (stop > GetDocument()->GetDataSize()) {
                stop = GetDocument()->GetDataSize();
            }

            GetDataFrame(frameStart, frameWidth);
            if ((frameStart == start)&&(frameWidth == (stop-start))) {
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
    m_fZoom = 1.0; // no zoom
    ZoomIn(0, TRUE);  // Handle Zoom
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
    ZoomIn(m_fZoom); // double zooming
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
    TRACE("OnHScroll %d %d %d %d\n",nSBCode,nPos,m_dwDataPosition,m_dwScrollLine);
    CSaDoc * pDoc = GetDocument();  // get pointer to document
    if (m_fZoom > 1.0) {            // zooming is enabled
        DWORD dwOldDataPosition = m_dwDataPosition; // save actual data position
        switch (nSBCode) {
        case SB_LEFT: // scroll to the leftmost position
            m_dwDataPosition = 0;
            break;
        case SB_LINELEFT: // scroll one line left
            if (m_dwDataPosition >= m_dwScrollLine) {
                m_dwDataPosition -= m_dwScrollLine;
            } else {
                m_dwDataPosition = 0;
            }
            break;
        case SB_RIGHT: // scroll to the rightmost position
            m_dwDataPosition = pDoc->GetDataSize() - GetDataFrame();
            break;
        case SB_LINERIGHT: // scroll one line right
            if ((m_dwDataPosition <= (pDoc->GetDataSize() - GetDataFrame() - m_dwScrollLine)) &&
                    (pDoc->GetDataSize() >= (GetDataFrame() + m_dwScrollLine))) {
                m_dwDataPosition += m_dwScrollLine;
            } else {
                m_dwDataPosition = pDoc->GetDataSize() - GetDataFrame();
            }
            break;
        case SB_PAGELEFT: // scroll one page left
            if (m_dwDataPosition >= GetDataFrame()) {
                m_dwDataPosition -= GetDataFrame();
            } else {
                m_dwDataPosition = 0;
            }
            break;
        case SB_PAGERIGHT: // scroll one page right
            if ((m_dwDataPosition <= (pDoc->GetDataSize() - 2 * GetDataFrame())) &&
                    (pDoc->GetDataSize() >= (2 * GetDataFrame()))) {
                m_dwDataPosition += GetDataFrame();
            } else {
                m_dwDataPosition = pDoc->GetDataSize() - GetDataFrame();
            }
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION: // scroll to position
            m_dwDataPosition = nPos * m_dwHScrollFactor;
            if (m_dwDataPosition > (pDoc->GetDataSize() - GetDataFrame())) {
                m_dwDataPosition = pDoc->GetDataSize() - GetDataFrame();
            }
            break;
        case SB_ENDSCROLL:
        default:
            break;
        }
        // for 16 bit data value must be even
        FmtParm * pFmtParm = pDoc->GetFmtParm();
        if (pFmtParm->wBlockAlign > 1) {
            m_dwDataPosition &= ~1;
        }
        // is scrolling necessary?
        if (dwOldDataPosition != m_dwDataPosition) { // scroll
            // set scroll bar
            SetScrollPos(SB_HORZ, (int)(m_dwDataPosition / m_dwHScrollFactor), TRUE);
            // scroll all graph windows (only if the have cursors visible)
            for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
                if (m_apGraphs[nLoop]) {
                    m_apGraphs[nLoop]->ScrollGraph(this, m_dwDataPosition, dwOldDataPosition);
                }
            }

            if (GraphIDtoPtr(IDD_RECORDING)) {
                GraphIDtoPtr(IDD_RECORDING)->GetPlot()->RedrawWindow(NULL,NULL,RDW_INTERNALPAINT|RDW_UPDATENOW);
            }
        }
    }

    CView::OnHScroll(nSBCode, nPos, pScrollBar);

    pViewMainFrame->SetPlayerTimes();
}

/***************************************************************************/
// CSaView::OnVScroll Vertical scrolling
/***************************************************************************/
void CSaView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar) {
    TRACE("OnVScroll %d\n",nPos);
    double fZoom = m_fZoom;
    double fActualPos = m_fVScrollSteps / m_fZoom; // actual position
    switch (nSBCode) {
    case SB_BOTTOM: // zoom maximum
        fZoom = m_fMaxZoom;
        break;
    case SB_LINEDOWN: // zoom one step more
        fZoom = m_fVScrollSteps / (fActualPos - 1);
        break;
    case SB_TOP:    // no zoom
        fZoom = (double)0.5;
        break;      // to be sure it will be set to 1.0 (rounding errors)
    case SB_LINEUP: // zoom one step less
        fZoom = m_fVScrollSteps / (fActualPos + 1);
        break;
    case SB_PAGEDOWN: // double zoom
        fZoom = 2.*m_fZoom;
        break;
    case SB_PAGEUP: // divide zoom by two
        fZoom = 0.5*m_fZoom;
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION: { // zoom from position
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
            ZoomOut(-(fZoom-m_fZoom));
        }
    }

    CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

/***************************************************************************/
// CSaView::OnSize Sizeing the view
/***************************************************************************/
void CSaView::OnSize(UINT nType, int cx, int cy) {
    CSaDoc * pDoc = GetDocument(); // get pointer to document

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
            SetScrollRange(SB_HORZ, 0, (int)((pDoc->GetDataSize() - GetDataFrame()) / m_dwHScrollFactor), FALSE);
            SetScrollPos(SB_HORZ, (int)(m_dwDataPosition / m_dwHScrollFactor), TRUE);
        }
        if (pViewMainFrame->IsScrollZoom() && (pDoc->GetDataSize() > 0)) {
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

    DestroyGraph(&(m_apGraphs[index]));

    //************************************************
    // if melogram is being destroyed then make
    // sure the TWC and magnitude are also destroyed.
    //************************************************
    if (m_anGraphID[index] == IDD_MELOGRAM) {
        int i = GetGraphIndexForIDD(IDD_TWC);
        if (i >= 0) {
            DestroyGraph(&(m_apGraphs[i]));
            m_anGraphID[i] = 0;
        }

        i = GetGraphIndexForIDD(IDD_MAGNITUDE);
        if (i >= 0) {
            DestroyGraph(&(m_apGraphs[i]));
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
        SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0);    // close view
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
                m_apGraphs[i]  = m_apGraphs[j];
                m_anGraphID[i] = m_anGraphID[j];
                m_apGraphs[j]  = NULL;
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
            DeleteGraphs(nLoop, FALSE);  // 09/26/2000 - DDO Delete graphs without clear IDs.
            CreateGraph(nLoop, m_anGraphID[nLoop]); // delete and recreate all graphs with new caption style
        }
    }
    OnGraphsRetile();         // retile graphs
    return 0;
}

/***************************************************************************/
// CSaView::OnGraphGridChanged Graphs grids have changed
/***************************************************************************/
LRESULT CSaView::OnGraphGridChanged(WPARAM, LPARAM) {
    RefreshGraphs();
    return 0;
}

// 1.5Test8.2
/***************************************************************************/
// CSaView::OnGraphOrderChanged Graphs colors have changed
/***************************************************************************/
LRESULT CSaView::OnGraphOrderChanged(WPARAM, LPARAM) {
    RefreshGraphs(TRUE, TRUE, TRUE);
    return 0;
}

/***************************************************************************/
// CSaView::OnGraphColorChanged Graphs colors have changed
/***************************************************************************/
LRESULT CSaView::OnGraphColorChanged(WPARAM, LPARAM) {
    RefreshGraphs(TRUE, TRUE);
    return 0;
}

/***************************************************************************/
// CSaView::OnGraphFontChanged Graphs font styles have changed
/***************************************************************************/
LRESULT CSaView::OnGraphFontChanged(WPARAM, LPARAM) {
    RefreshGraphs(TRUE, TRUE, TRUE);
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
    CDlgRecorder * pDlgRecorder = new CDlgRecorder;

    if (pDlgRecorder->DoModal() != IDOK) {
        SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L);    // close the file
    }
    delete pDlgRecorder;
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
                   && ((m_nFocusedID == IDD_RAWDATA) || (m_nFocusedID == IDD_GLOTWAVE)|| (m_nFocusedID == IDD_MELOGRAM))); // enable if graph is this type
    pCmdUI->SetCheck(((m_nFocusedID == IDD_RAWDATA) || (m_nFocusedID == IDD_GLOTWAVE)|| (m_nFocusedID == IDD_MELOGRAM))
                     && ((m_pFocusedGraph->GetPlot())->HaveDrawingStyleDots())); // check if graph has style dots
}

/***************************************************************************/
// CSaView::OnKeyDown Keyboard interface
/***************************************************************************/
void CSaView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
    TRACE("keydown %d %d %d\n",nChar, nRepCnt, nFlags);
    CProgressStatusBar * pStatusBar = NULL;
    CProcess * pProcessOwner = NULL;
    CRect rWnd;

    // are these keys already pressed?
    bool ctrlPressed = (GetAsyncKeyState(VK_CONTROL) < 0);
    bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) < 0);
    TRACE("ctrlPressed=%d shiftPressed=%d\n",ctrlPressed,shiftPressed);

    if ((!ctrlPressed)&&(!shiftPressed)) {
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
            OnVScroll(SB_LINEUP, 0, GetScrollBarCtrl(SB_VERT)); // zoom out
            break;
        case VK_DOWN:
            OnVScroll(SB_LINEDOWN, 0, GetScrollBarCtrl(SB_VERT)); // zoom in
            break;
        case VK_NEXT: // page down
            OnHScroll(SB_PAGEUP, 0, GetScrollBarCtrl(SB_HORZ)); // scroll left one page
            break;
        case VK_PRIOR: // page up
            OnHScroll(SB_PAGEDOWN, 0, GetScrollBarCtrl(SB_HORZ)); // scroll right one page
            break;
        case VK_ESCAPE: // process interrupt from user and stop player
            if (pViewMainFrame->IsPlayerPlaying()) {
                pViewMainFrame->SendMessage(WM_COMMAND, ID_PLAYER_STOP, 0L);    // send message to stop player
            } else if (!IsAnimating()) { // Do not cancel processes during animation
                // get pointer to status bar
                pStatusBar = (CProgressStatusBar *)pViewMainFrame->GetProgressStatusBar();
                pProcessOwner = (CProcess *)pStatusBar->GetProcessOwner(); // get the current process owner
                if (pProcessOwner) {
                    pProcessOwner->CancelProcess();    // cancel the process
                } else {
                    DeselectAnnotations();
                }
            }
            break;
        default:
            CView::OnKeyDown(nChar, nRepCnt, nFlags);
            break;
        }

    } else if ((shiftPressed)&&(!ctrlPressed)) {

        CView::OnKeyDown(nChar, nRepCnt, nFlags);

    } else if ((ctrlPressed)&&(!shiftPressed)) {

        switch (nChar) {
        case VK_END:
            OnVScroll(SB_BOTTOM, 0, GetScrollBarCtrl(SB_VERT)); // zoom to maximum
            break;

        default:
            CView::OnKeyDown(nChar, nRepCnt, nFlags);
            break;
        }

    } else {

        // everything sels
        CView::OnKeyDown(nChar, nRepCnt, nFlags);
    }
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
// CSaView::OnActivateView View activating or deactivating
// If the view loses or gains active state, its focused graph has to
// gain or lose focus. The statusbar has to be updated. Also the mainframe
// has to be informed, because of the player or recorder launched, which have
// to be stopped immediately.
/***************************************************************************/
void CSaView::OnActivateView(BOOL bActivate, CView * pActivateView, CView * pDeactivateView) {
    CSaDoc * pDoc = (CSaDoc *)GetDocument(); // get pointer to document
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
        pViewMainFrame->SendMessage(WM_USER_CHANGEVIEW, TRUE, (LONG)this);
        // process workbench if necessary
        if (pDoc->WorkbenchProcess()) {
            RefreshGraphs(TRUE, TRUE);
        };
        // redraw statusbar if data is present
        if (pDoc->GetDataSize() != 0) {
            if (m_pFocusedGraph) {
                m_pFocusedGraph->UpdateStatusBar(GetStartCursorPosition(), GetStopCursorPosition(), TRUE);
            }
        } else { // clear status bar panes
            // get pointer to status bar
            CDataStatusBar * pStat = pViewMainFrame->GetDataStatusBar();
            // turn off symbols
            pStat->SetPaneSymbol(ID_STATUSPANE_1, FALSE);
            pStat->SetPaneSymbol(ID_STATUSPANE_2, FALSE);
            pStat->SetPaneSymbol(ID_STATUSPANE_3, FALSE);
            pStat->SetPaneSymbol(ID_STATUSPANE_4, FALSE);
            // clear the panes
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

    if (pViewMainFrame->ComputeNumberOfViews(-1) == 0) { // last view destroyed?
        pViewMainFrame->SendMessage(WM_USER_CHANGEVIEW, FALSE, (LONG)this);    // inform mainframe
    }
}

/***************************************************************************/
// 09/26/2000 - DDO This function will return TRUE or FALSE for enabling
//                  and disabling graph types. The conditions are standard
//                  conditions for the types.
/***************************************************************************/
BOOL CSaView::GraphTypeEnabled(int nID, BOOL bIncludeCtrlKeyCheck) {
    BOOL bTest = (GetDocument()->GetDataSize() != 0 &&
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
} ;

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

    for (int i=0; kGraphTypeInfo[i].nID; i++) {
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

    pCmdUI->Enable(bEnable && GraphTypeEnabled(pCmdUI->m_nID, pInfo->bIncludeCtrlKeyCheck)); // enable if data is available
    pCmdUI->SetCheck(bSelected); // check if graph selected
}

/***************************************************************************/
// 09/27/2000 - DDO
/***************************************************************************/
void CSaView::ToggleDpGraph(UINT nID) {
    int i = GetGraphIndexForIDD(nID);
    if (i >= 0) {
        DeleteGraphs(i);                                               // delete this graph
        MakeGraphArraysContiguous();                                   // clean up the old arrays
        if (m_apGraphs[0]) {
            m_apGraphs[0]->SendMessage(WM_LBUTTONDOWN, 0, MAKELONG(0, 0));    // change focus SDM 1.5Test10.6
        }
    } else {
        // create new graph
        for (i = 0; i < MAX_GRAPHS_NUMBER; i++) {
            if (m_anGraphID[i] < nID) {
                if (m_apGraphs[i]) {
                    DeleteGraphs(i);
                }
                m_apGraphs[i] = NULL;
                m_anGraphID[i] = 0;
                MakeGraphArraysContiguous();
                CreateGraph(i, nID);
                if (m_apGraphs[i]) {
                    m_apGraphs[i]->SendMessage(WM_LBUTTONDOWN, 0, MAKELONG(0, 0));    // change focus SDM 1.5Test10.6
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
        m_pFocusedGraph->ShowLegend(!m_pFocusedGraph->HaveLegend(), TRUE);
    }

    //*****************************************************
    // 09/29/2000 - DDO When turning off the legend for
    // the TWC graph and the melogram graph isn't showing
    // it's legend, then automatically show the legend for
    // the melogram.
    //*****************************************************
    if (m_nFocusedID == IDD_TWC) {
        int i = GetGraphIndexForIDD(IDD_MELOGRAM);
        if (m_apGraphs[i] && !m_apGraphs[i]->HaveLegend()) {
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
        bEnable &= (!m_pFocusedGraph->DisableLegend() || m_pFocusedGraph->HaveLegend());
    }
    pCmdUI->Enable(bEnable);
    if (m_pFocusedGraph) {
        pCmdUI->SetCheck(m_pFocusedGraph->HaveLegend());    // check if graph has legend window
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
    BOOL bEnable = m_pFocusedGraph && GetDocument()->GetDataSize() != 0 && (m_pFocusedGraph->HaveXScale() || !m_pFocusedGraph->DisableXScale()) && m_nFocusedID != IDD_TWC;
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
void CSaView::ShowAnnotation(int nAnnot) {
    if (m_pFocusedGraph) {
        m_pFocusedGraph->ShowAnnotation(nAnnot, !m_pFocusedGraph->HaveAnnotation(nAnnot), TRUE);
    }

    if (m_nFocusedID == IDD_MELOGRAM) {
        int i = GetGraphIndexForIDD(IDD_TWC);
        if ((i != -1) && m_apGraphs[i]) {
            m_apGraphs[i]->ShowAnnotation(nAnnot, !m_apGraphs[i]->HaveAnnotation(nAnnot), TRUE);
        }
    }
}


/***************************************************************************/
// CSaView::OnDrawingBoundaries Show or hide boundaries
/***************************************************************************/
void CSaView::OnDrawingBoundaries() {
    if (m_pFocusedGraph) {
        m_pFocusedGraph->ShowBoundaries(!m_pFocusedGraph->HaveBoundaries(), TRUE);
    }
}

/***************************************************************************/
// CSaView::OnUpdateDrawingBoundaries Menu update
/***************************************************************************/
void CSaView::OnUpdateDrawingBoundaries(CCmdUI * pCmdUI) {
    pCmdUI->Enable(m_pFocusedGraph && (GetDocument()->GetDataSize() != 0)  // enable if data is available
                   && (m_pFocusedGraph->HaveCursors())); // enable if cursors visible
    if (m_pFocusedGraph) {
        pCmdUI->SetCheck(m_pFocusedGraph->HaveBoundaries());    // check if graph has boundaries
    } else {
        pCmdUI->SetCheck(FALSE);
    }
}

/***************************************************************************/
// CSaView::OnGraphsMagnify1 Set magnify factor
/***************************************************************************/
void CSaView::OnGraphsMagnify1() {
    m_fMagnify = 1.0;
    // set graphs magnify
    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
        if (m_apGraphs[nLoop]) {
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
        if (m_apGraphs[nLoop]) {
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
        if (m_apGraphs[nLoop]) {
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
            if (m_apGraphs[nLoop]) {
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
                   && (m_pFocusedGraph->HaveCursors())); // enable if cursors visible
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
                   && (m_pFocusedGraph->HaveCursors())); // enable if cursors visible
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
                   && (m_pFocusedGraph->HaveCursors())); // enable if cursors visible
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
                   && (m_pFocusedGraph->HaveCursors())); // enable if cursors visible
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
        if (m_apGraphs[nLoop]) {
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
    if (m_pFocusedGraph && m_pFocusedGraph->IsAreaGraph()  && !m_pFocusedGraph->IsPlotID(IDD_RECORDING)) {
        // restart area graph process only if graph has focus
        bEnable = TRUE;
    }
    if (GraphIDtoPtr(IDD_SNAPSHOT)) {
        bEnable = TRUE;
    }

    pCmdUI->Enable(bEnable);
}

/***************************************************************************/
// CSaView::OnEditSelectWaveform Select Waveform data between cursors
/***************************************************************************/
void CSaView::OnEditSelectWaveform() {
    // select/deselect raw data area
    int i = GetGraphIndexForIDD(IDD_RAWDATA);

    if (i >= 0 && m_apGraphs[i]) {
        // check if already area selected
        if (m_apGraphs[i]->GetPlot()->GetHighLightLength()) {
            m_apGraphs[i]->GetPlot()->SetHighLightArea(0, 0);    // deselect
        } else {
            m_apGraphs[i]->GetPlot()->SetHighLightArea(GetStartCursorPosition(), GetStopCursorPosition());    // select
        }
    }
}

/***************************************************************************/
// CSaView::OnUpdateEditSelectWaveform Select Waveform data between cursors
/***************************************************************************/
void CSaView::OnUpdateEditSelectWaveform(CCmdUI * pCmdUI) {
    //pCmdUI->Enable(!GetDocument()->IsMultiChannel());
    pCmdUI->Enable(true);
}

/***************************************************************************/
// CSaView::OnPlayFKey Playback according to function key setting
/***************************************************************************/
void CSaView::OnPlayFKey(UINT nID) {
    SendPlayMessage(WORD(nID - ID_PLAY_F1), WORD(-1)); // send message to start player
}

/***************************************************************************/
// CSaView::OnUpdatePlayback Menu update
/***************************************************************************/
void CSaView::OnUpdatePlayback(CCmdUI * pCmdUI) {
    pCmdUI->Enable(GetDocument()->GetDataSize() != 0); // enable if data is available
}

/***************************************************************************/
// CSaView::OnSetupFnkeys Calls the player and the setup Fn-keys dialog
/***************************************************************************/
void CSaView::OnSetupFnkeys() {
    pViewMainFrame->SendMessage(WM_USER_PLAYER, IDC_STOP, MAKELONG(-1, FALSE));
    pViewMainFrame->SetupFunctionKeys();
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
    CSegment * pSeg = NULL;

    if (annotSetID >= 0 && annotSetID < ANNOT_WND_NUMBER) {
        pSeg = GetDocument()->GetSegment(annotSetID);
    }

    return pSeg;
}


/***************************************************************************/
// CSaView::OnFilePrint
/***************************************************************************/
void CSaView::OnFilePrint() {
    pViewMainFrame->UpdateWindow();      // Repaint window in case a screen shot print is requested
    pViewMainFrame->SetPrintingFlag();
    CView::OnFilePrint();
}

/***************************************************************************/
// CSaView::OnFilePrintPreview
/***************************************************************************/
void CSaView::OnFilePrintPreview() {
    pViewMainFrame->UpdateWindow();
    pViewMainFrame->SetPrintingFlag();
    m_bPrintPreviewInProgress = TRUE;
    pViewMainFrame->SetPreviewFlag();
    CView::OnFilePrintPreview();
}


/***************************************************************************/
/***************************************************************************/
BOOL CSaView::IsFocusGraph(UINT id) {
    if (m_nFocusedID==ID_GRAPHS_OVERLAY) {
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
    for (int nLoop=0; nLoop<MAX_GRAPHS_NUMBER; nLoop++) {
        if (m_anGraphID[nLoop] == id) {
            return m_apGraphs[nLoop];
        }
    }
    return NULL;
}

/***************************************************************************/
/***************************************************************************/
BOOL CSaView::GraphIDincluded(UINT id) {
    for (int nLoop=0; nLoop<MAX_GRAPHS_NUMBER; nLoop++) {
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
        CSaDoc * pDoc = GetDocument(); // get pointer to document
        m_pPickOverlay->ResetGraphsPtr();

        //      m_pPickOverlay->SetGraphsPtr(m_apGraphs, m_pFocusedGraph);
        POSITION position = pDoc->GetTemplate()->GetFirstDocPosition();
        while (position != NULL) {
            CDocument * pNextDoc = pDoc->GetTemplate()->GetNextDoc(position); // get pointer to document
            if (pNextDoc) {
                POSITION pos = pNextDoc->GetFirstViewPosition();
                while (pos != NULL) {
                    CSaView * pView = (CSaView *)(pNextDoc->GetNextView(pos));
                    m_pPickOverlay->SetGraphsPtr(pView->m_apGraphs, m_pFocusedGraph);
                }
            }
        }

        if ((m_pPickOverlay->DoModal()==IDOK) &&
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

        //      m_pPickOverlay->SetGraphsPtr(m_apGraphs, m_pFocusedGraph);
        POSITION position = GetDocument()->GetTemplate()->GetFirstDocPosition();
        while (position != NULL) {
            CDocument * pDoc = GetDocument()->GetTemplate()->GetNextDoc(position); // get pointer to document
            if (pDoc) {
                POSITION pos = pDoc->GetFirstViewPosition();
                while (pos != NULL) {
                    CSaView * pView = (CSaView *)(pDoc->GetNextView(pos));
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

        if (m_pPickOverlay->DoModal()==IDOK) {
            for (int i=(pMPlot->m_List.GetCount()-1); i>=0; i--) {
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
                    ASSERT(numPlots==1);
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
    CMDIChildWnd * pwnd = pwndChildFrame();

    // Minimize this window if it had been when SA was closed.
    if (m_eInitialShowCmd == SW_SHOWMINIMIZED) {
        pwnd->ShowWindow(m_eInitialShowCmd);
    }

    // Find the MDI child window which corresponds to the one --out of those
    // opened so far-- just above this when SA was closed.
    int zThis = z();
    CSaView * pviewAboveThis = NULL;

    CSaView * pview = pSaApp->GetViewBelow(this);
    for (; pview; pview = pSaApp->GetViewBelow(pview))
        if (zThis < pview->z()) {
            pviewAboveThis = pview;
        }

    // Set this window's z-order. (The framework has opened it on top.)
    if (pviewAboveThis) {
        BOOL bSetZ = pwnd->SetWindowPos(pviewAboveThis->pwndChildFrame(),
                                        0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        ASSERT(bSetZ);
    }
}

/***************************************************************************/
/***************************************************************************/
void CSaView::ShowInitialTopState() {
    CMDIChildWnd * pwnd = pwndChildFrame();

    // Make this the active MDI child window, since it had been
    // the top when SA was closed.
    CMainFrame * pwndMainFrame = (CMainFrame *)pSaApp->m_pMainWnd;
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
void CSaView::s_SetObjectStream(Object_istream & obs) {
    ASSERT(!s_pobsAutoload);
    s_pobsAutoload = &obs;
}

/***************************************************************************/
/***************************************************************************/
void CSaView::s_ClearObjectStream() {
    s_pobsAutoload = NULL;
}

/***************************************************************************/
static const char * psz_saview             = "saview";
static const char * psz_placement          = "placement";
static const char * psz_z                  = "z";
static const char * psz_legendall          = "legendall";
static const char * psz_legendnone         = "legendnone";
static const char * psz_xscaleall          = "xscaleall";
static const char * psz_xscalenone         = "xscalenone";
static const char * psz_boundariesall      = "boundariesall";
static const char * psz_boundariesnone     = "boundariesnone";
static const char * psz_drawstyleline      = "drawstyleline";
static const char * psz_updateboundaries   = "updateboundaries";
static const char * psz_graphlist          = "graphlist";
static const char * psz_graphid            = "graphid";
static const char * psz_layout             = "layout2";
static const char * psz_annotallornonelist = "annotallornonelist";
static const char * psz_annotall           = "annotall";
static const char * psz_annotnone          = "annotnone";

/***************************************************************************/
/***************************************************************************/
void CSaView::WriteProperties(Object_ostream & obs) {
    int i = 0;

    //**********************************************************
    // Write the marker beginning the properties for this view.
    //**********************************************************
    obs.WriteBeginMarker(psz_saview);

    //**********************************************************
    // save the window placement.
    //**********************************************************
    if (m_bViewCreated && GetSafeHwnd()) {
        WINDOWPLACEMENT wpl;
        GetParent()->GetWindowPlacement(&wpl);

        obs.WriteWindowPlacement(psz_placement, wpl);
        obs.WriteInteger(psz_z, m_z);
    }

    // TODO - save more data members.

    obs.WriteBeginMarker(psz_graphlist);

    //*************************************************
    // Save the properties for each graph that exists.
    //*************************************************
    for (i = 0; i < MAX_GRAPHS_NUMBER; i++) {
        if (m_apGraphs[i]) {
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

    //**********************************************************
    // Write page layout properties if they exist.
    //**********************************************************
    if (m_pPageLayout) {
        m_pPageLayout->WriteProperties(obs);
    }

    //**********************************************************
    // Write layout, legend and x-scale settings.
    //**********************************************************
    obs.WriteUInt(psz_layout, m_nLayout);
    obs.WriteBool(psz_legendall, m_bLegendAll);
    obs.WriteBool(psz_legendnone, m_bLegendNone);
    obs.WriteBool(psz_xscaleall, m_bXScaleAll);
    obs.WriteBool(psz_xscalenone, m_bXScaleNone);

    //**********************************************************
    // Write the global properties for the annotation windows.
    //**********************************************************
    obs.WriteBeginMarker(psz_annotallornonelist);
    for (i = 0; i < ANNOT_WND_NUMBER; i++) {
        obs.WriteBool(psz_annotall,  m_abAnnAll[i]);
        obs.WriteBool(psz_annotnone, m_abAnnNone[i]);
    }
    obs.WriteEndMarker(psz_annotallornonelist);

    //**********************************************************
    // Write other misc. global properties.
    //**********************************************************
    obs.WriteBool(psz_boundariesall, m_bBoundariesAll);
    obs.WriteBool(psz_boundariesnone, m_bBoundariesNone);
    obs.WriteBool(psz_drawstyleline, m_bDrawStyleLine);
    obs.WriteBool(psz_updateboundaries, m_bUpdateBoundaries);

    // These data members are currently not saved.
    //    DWORD m_dwDataPosition;              // current start position of displayed data
    //    double m_fMagnify;                    // magnify factor
    //    double m_fZoom;                         // current zoom factor
    //    double m_fMaxZoom;                  // max. zoom factor
    //    DWORD m_dwHScrollFactor;             // factor to represent scroll position on horizontal scroll bar
    //    double m_fVScrollSteps;               // number of vertical scroll steps
    //    DWORD m_dwScrollLine;              // number of samples to scroll one line
    //    DWORD m_dwStartCursor;               // start cursor position
    //    DWORD m_dwStopCursor;              // stop cursor position
    //    BOOL  m_bViewIsActive;               // TRUE = view is activated
    // Printing data members are not saved - no need.

    obs.WriteEndMarker(psz_saview);
}

/***************************************************************************/
/***************************************************************************/
BOOL CSaView::ReadProperties(Object_istream & obs, BOOL bCreateGraphs) {
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
        if (pwndFrame &&
                obs.bReadWindowPlacement(psz_placement, wpl)) {
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
        else if (obs.bReadBool(psz_boundariesall, m_bBoundariesAll));
        else if (obs.bReadBool(psz_boundariesnone, m_bBoundariesNone));
        else if (obs.bReadBool(psz_drawstyleline, m_bDrawStyleLine));
        else if (obs.bReadBool(psz_updateboundaries, m_bUpdateBoundaries));

        else if (obs.bReadBeginMarker(psz_annotallornonelist)) {
            for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
                obs.bReadBool(psz_annotall,  m_abAnnAll[i]);
                obs.bReadBool(psz_annotnone, m_abAnnNone[i]);
            }
            obs.SkipToEndMarker(psz_annotallornonelist);
        } else if (obs.bReadEndMarker(psz_saview)) {
            break;
        } else {
            obs.ReadMarkedString();    // Skip unexpected field
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
BOOL CSaView::ReadGraphListProperties(Object_istream & obs, BOOL bCreateGraphs) {
    if (!obs.bReadBeginMarker(psz_graphlist)) {
        return FALSE;
    }

    int  i  = 0;
    UINT id = 0;
    UINT OpenAsID = pSaApp->GetOpenAsID();

    memset(&m_apGraphs[0],  0, sizeof(CGraphWnd *) * MAX_GRAPHS_NUMBER);
    memset(&m_anGraphID[0], 0, sizeof(UINT) * MAX_GRAPHS_NUMBER);

    while (!obs.bAtEnd()) {
        if (obs.bReadUInt(psz_graphid, id)) {

            //*********************************************************
            // If we're suppposed to create visible graphs then create
            // the appropriate graph (i.e. a position view or other).
            //*********************************************************
            if (bCreateGraphs && OpenAsID == ID_FILE_OPEN) {
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
                ASSERT(m_apGraphs[i] == NULL);  // we shouldn't be destroying a pointer to a graph
                m_apGraphs[i] = new CGraphWnd(id);
                if ((m_apGraphs[i])) {
                    m_anGraphID[i] = id;
                    m_WeJustReadTheProperties = m_apGraphs[i]->ReadProperties(obs);
                    bSkipToEnd = FALSE;
                }

                //*******************************************************
                // If for some reason the construction of graphs failed
                // then skip to the end marker for this section.
                //*******************************************************
                if (bSkipToEnd) {
                    obs.SkipToEndMarker(psz_sagraph);    // move past this graph record
                }
            }

            if (m_apGraphs[i]) {
                i++;    // successfully filled in a space, move to next space.
            }
        } else if (obs.bReadEndMarker(psz_graphlist)) {
            break;
        }
    }



    //************************************************************
    // RLJ 06/06/2000 - If OpenAsID = "Phonetic/Music Analysis",
    //                  then create corresponding graphs.
    //************************************************************
    if (bCreateGraphs && OpenAsID != ID_FILE_OPEN) {
        CreateOpenAsGraphs(OpenAsID);
    }

    return TRUE;
}

/***************************************************************************/
// This function to read graph properties is used when the graphs are being
// created from a template view.
/***************************************************************************/
BOOL CSaView::ReadGraphListProperties(const CSaView & pTemplateView) {
    int OpenAsID = pSaApp->GetOpenAsID();

    if (OpenAsID == ID_FILE_OPEN) {
        for (int i = 0, nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
            if (pTemplateView.m_apGraphs[nLoop])
                CreateGraph(i, pTemplateView.m_anGraphID[nLoop], CREATE_FROMGRAPH,
                            NULL, pTemplateView.m_apGraphs[nLoop]);

            if (m_apGraphs[i]) {
                i++;
            }
        }
    }
    /*****************************************************************/
    // RLJ 06/06/2000 - If OpenAsID = "Phonetic/Music Analysis",
    //                  then create corresponding graphs.
    /*****************************************************************/
    else {
        CreateOpenAsGraphs(OpenAsID);
    }

    return TRUE;
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnEditInplace() {
    m_advancedSelection.Update(this);
    int nAnnotationIndex = m_advancedSelection.GetSelection().nAnnotationIndex;

    if ((nAnnotationIndex != -1) && GetFocusedGraphWnd() &&
            (GetFocusedGraphWnd()->HaveAnnotation(nAnnotationIndex))) { // Selected annotation is visible
        CAnnotationWnd * pWnd = GetFocusedGraphWnd()->GetAnnotationWnd(nAnnotationIndex);
        pWnd->OnCreateEdit();
    }
}

/***************************************************************************/
/***************************************************************************/
void CSaView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
    m_advancedSelection.Update(this);
    int nAnnotationIndex = m_advancedSelection.GetSelection().nAnnotationIndex;

    if (nChar < 32) {
        CView::OnChar(nChar, nRepCnt, nFlags);
        return;
    }

    if ((nAnnotationIndex != -1) && GetFocusedGraphWnd() &&
            (GetFocusedGraphWnd()->HaveAnnotation(nAnnotationIndex))) { // Selected annotation is visible
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
                ASSERT(numPlots==1);
                UINT plotID = pMPlot->GetBasePlotID();
                int i2;
                for (i2 = 0; i2 < MAX_GRAPHS_NUMBER; i2++) {
                    if (m_anGraphID[i2] == plotID) {
                        break; // graph exists already
                    }
                }
                if (i2 >= MAX_GRAPHS_NUMBER) {
                    ChangeGraph(nLoop,plotID);
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

