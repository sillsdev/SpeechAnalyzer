//
// SelfTest.cpp
//
// Implementation of the CSASelfTest class
//
// This class will run through a test routine when instantiated, logging results to an xml file
//
// 08/27/2001 TRE - original coding
//

#include "SelfTest.h"
#include <windows.h>
#include <mapiwin.h>
#include <process.h>
#include "stdafx.h"
#include "sa_g_spg.h"
#include "sa_plot.h"
#include "SpectroParm.h"
#include "Process\Process.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_pit.h"
#include "Process\sa_p_cpi.h"
#include "Process\sa_p_spi.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_mel.h"
#include "Process\sa_p_cha.h"
#include "Process\sa_p_raw.h"
#include "Process\sa_p_spg.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_fmt.h"
#include "Process\sa_p_dur.h"
#include "Process\sa_p_glo.h"
#include "Process\sa_p_poa.h"
#include "Process\sa_p_rat.h"
#include "Process\sa_p_twc.h"
#include "sa_doc.h"
#include "sa_view.h"

//
// file location of messages that get updated regularly
#define STATUS_MESSAGE 0x3A
#define N_TESTS_PASSED 0x95
#define N_TESTS_FAILED 0xB7


//
// Main testing routine
//
// Each test should begin with a call to StartTest() and end with a call to EndTest()
// The test should be executed only if StartTest() returns TRUE.
//
void CSASelfTest::SelfTest()
{

    BOOL bFileOpened = FALSE;
    BOOL bTestSuccess = FALSE;
    CSaDoc * pDoc = NULL;
    CSaView * pView = NULL;
    const UttParm * pUP = NULL;
    FmtParm * pFP = NULL;

    /*
    // This test has problems because it involves a modal dialog box.
    // The modal dialog box does not allow any processing to be done
    // in the SA thread while it is waiting for OK or Cancel to be
    // pressed. That means we can't send messages to the dialog while
    // it's open.

    if (StartTest("CHFrench UI open","Open CHFRENCH.WAV thru UI"))
    {
    CString szTestFileName = m_szTestFolderPath + "chfrench.wav";
    if (FileExists(szTestFileName))
    {
    // THIS CODE USES DDOBATCH TO SEND KEYSTROKES
    //char szProgramFileName[_MAX_PATH];
    //char szScriptFileName[_MAX_PATH];
    //GetShortPathName(m_szTestFolderPath + "UIOpen.ini",szScriptFileName,_MAX_PATH);
    //GetShortPathName(m_szTestFolderPath + "DDOBatch.exe",szProgramFileName,_MAX_PATH);
    //_spawnl(_P_NOWAIT ,szProgramFileName, szProgramFileName, szScriptFileName, NULL);
    //MessageLoop(10000);

    m_pMain->PostMessage(WM_COMMAND,ID_FILE_OPEN,0);
    ::PostMessage(GetFocus(),WM_CHAR,50,0);
    MessageLoop(3000);
    CMDIChildWnd *Baby = m_pMain->MDIGetActive();
    if (Baby)
    {
    pDoc = (CSaDoc*) Baby->GetActiveDocument();
    pView = (CSaView*) Baby->GetActiveView();
    }
    }
    bFileOpened = (pDoc !=NULL);
    EndTest(bFileOpened);
    bTestSuccess = m_pMain->SendMessage(WM_COMMAND,ID_FILE_CLOSE,0);
    pDoc = NULL;
    bFileOpened = FALSE;
    }
    */

    if (StartTest("CHFrench open","Open file CHFRENCH.WAV"))
    {
        CString szTestFileName = m_szTestFolderPath + "chfrench.wav";
        if (FileExists(szTestFileName))
        {
            pDoc = (CSaDoc *) m_pApp->OpenDocumentFile(szTestFileName);
            pDoc->EnableBackgroundProcessing(FALSE);
            pView = (CSaView *) m_pMain->MDIGetActive()->GetActiveView();
        }
        bFileOpened = (pDoc !=NULL);
        EndTest(bFileOpened);
    }

    if (bFileOpened && StartTest("CHFrench Set Cursor Alignment","Set cursor alignment to sample."))
    {
        // manually set the cursor alignment to sample alignment
        pView->ChangeCursorAlignment(ALIGN_AT_SAMPLE);
        EndTest(pView->GetCursorAlignment() == ALIGN_AT_SAMPLE);
    }

    //
    // Test Utterance Parameters of CHFRENCH.WAV file
    //
    if (bFileOpened && StartTest("CHFrench UttParm","Test utterance parameters"))
    {
        bTestSuccess = TRUE;

        pUP = pDoc->GetUttParm();
        if (pUP->nCritLoud != 6912)
        {
            bTestSuccess = FALSE;
            LogEntry("Critical loudness incorrectly set.");
        }
        if (pUP->nMaxChange != 11)
        {
            bTestSuccess = FALSE;
            LogEntry("Maximum change incorrectly set.");
        }
        if (pUP->nMaxFreq != 260)
        {
            bTestSuccess = FALSE;
            LogEntry("Maximum frequency incorrectly set.");
        }
        if (pUP->nMaxInterp != 7)
        {
            bTestSuccess = FALSE;
            LogEntry("Maximum interpolation incorrectly set.");
        }
        if (pUP->nMinFreq != 70)
        {
            bTestSuccess = FALSE;
            LogEntry("Minimum frequency incorrectly set.");
        }
        if (pUP->nMinGroup != 6)
        {
            bTestSuccess = FALSE;
            LogEntry("Minimum group incorrectly set.");
        }

        EndTest(bTestSuccess);
    }

    //
    // Test Format Parameters of CHFRENCH.WAV file
    //
    if (bFileOpened && StartTest("CHFrench FmtParm","Test format parameters"))
    {
        bTestSuccess = TRUE;

        pFP = pDoc->GetFmtParm();
        if (pFP->dwAvgBytesPerSec != 44100)
        {
            bTestSuccess = FALSE;
            LogEntry("Average bytes per second incorrectly set.");
        }
        if (pFP->dwSamplesPerSec != 22050)
        {
            bTestSuccess = FALSE;
            LogEntry("Samples per second incorrectly set.");
        }
        if (pFP->wBitsPerSample != 16)
        {
            bTestSuccess = FALSE;
            LogEntry("Bits per sample incorrectly set.");
        }
        if (pFP->wBlockAlign != 2)
        {
            bTestSuccess = FALSE;
            LogEntry("BlockAlign incorrectly set.");
        }
        if (pFP->wChannels != 1)
        {
            bTestSuccess = FALSE;
            LogEntry("Channels incorrectly set.");
        }
        if (pFP->wTag != 1)
        {
            bTestSuccess = FALSE;
            LogEntry("Tag incorrectly set.");
        }


        EndTest(bTestSuccess);
    }

    //
    // Process integrity checks using CHFRENCH.WAV file
    //
    if (bFileOpened && StartTest("CHFrench CursorPositioning","Position the cursors in voiced region"))
    {
        pView->SetStartCursorPosition(30714,SNAP_LEFT);
        pView->SetStopCursorPosition(35046,SNAP_RIGHT);

        DWORD dwStart = pView->GetStartCursorPosition();
        DWORD dwStop  = pView->GetStopCursorPosition();
        if (dwStart==30714 && dwStop==35046)
        {
            EndTest();
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened && StartTest("CHFrench ZCrossing","Test zero crossing process."))
    {
        CProcessZCross * pZCross = pDoc->GetZCross();
        short int nResult = LOWORD(pZCross->Process(this, pDoc)); // process data
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pZCross->GetProcessFileName()));
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened && StartTest("CHFrench RawPitch","Test raw pitch process."))
    {
        CProcessPitch * pPitch = pDoc->GetPitch();
        short int nResult = LOWORD(pPitch->Process(this, pDoc)); // process data
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pPitch->GetProcessFileName()));
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened && StartTest("CHFrench CustomPitch","Test custom pitch process."))
    {
        CProcessCustomPitch * pCustomPitch = pDoc->GetCustomPitch();
        short int nResult = LOWORD(pCustomPitch->Process(this, pDoc)); // process data
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pCustomPitch->GetProcessFileName()));
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened && StartTest("CHFrench SmoothedPitch","Test smoothed pitch process."))
    {
        CProcessSmoothedPitch * pSmoothedPitch = pDoc->GetSmoothedPitch();
        short int nResult = LOWORD(pSmoothedPitch->Process(this, pDoc)); // process data
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pSmoothedPitch->GetProcessFileName()));
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened && StartTest("CHFrench AutoPitch","Test auto pitch process."))
    {
        CProcessGrappl * pAutoPitch = pDoc->GetGrappl();
        short int nResult = LOWORD(pAutoPitch->Process(this, pDoc)); // process data
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pAutoPitch->GetProcessFileName()));
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened)
    {
        UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_GRAPITCH,0,0,0,0,0,0,0,0};
        pView->OnGraphsTypesPostProcess(GrphIDs,-1);
        MessageLoop(3000);
    }

    if (bFileOpened && StartTest("CHFrench Fragment","Test fragment process."))
    {
        CProcessFragments * pFragment = pDoc->GetFragments();
        pFragment->SetDataInvalid();
        pDoc->EnableBackgroundProcessing(FALSE);
        pFragment->RestartProcess();
        pFragment->Process(this, (CSaDoc *)pDoc); // process data
        // wait for idle loop to finish
        while (!pFragment->IsDataReady())
        {
            MessageLoop(10);
        }
        EndTest(!FileCompare(pFragment->GetProcessFileName()));
    }

    if (bFileOpened && StartTest("CHFrench Loudness","Test loudness process."))
    {
        CProcessLoudness * pLoudness = pDoc->GetLoudness();
        short int nResult = LOWORD(pLoudness->Process(this, pDoc)); // process data
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pLoudness->GetProcessFileName()));
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened && StartTest("CHFrench Melogram","Test melogram process."))
    {
        CProcessMelogram * pMelogram = pDoc->GetMelogram();
        short int nResult = LOWORD(pMelogram->Process(this, pDoc)); // process data
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pMelogram->GetProcessFileName()));
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened && StartTest("CHFrench Change","Test change process."))
    {
        CProcessChange * pChange = pDoc->GetChange();
        short int nResult = LOWORD(pChange->Process(this, pDoc)); // process data
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pChange->GetProcessFileName()));
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened && StartTest("CHFrench Raw","Test raw data process."))
    {
        CProcessRaw * pRaw = pDoc->GetRaw();
        short int nResult = LOWORD(pRaw->Process(this, pDoc)); // process data
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pRaw->GetProcessFileName()));
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened && StartTest("CHFrench Spectrogram","Test spectrogram process."))
    {
        CProcessSpectrogram * pSpectrogram =
            (CProcessSpectrogram *)pDoc->GetSpectrogram(TRUE); // get pointer to spectrogram object

        pSpectrogram->SetDataInvalid();
        // set spectrogram parameters
        CSpectroParm cSpectroParm = pSpectrogram->GetSpectroParm();
        CSpectroParm * pSpectroParm = &cSpectroParm;
        pSpectroParm->nResolution = 2;
        pSpectroParm->nColor = 0;
        pSpectroParm->nOverlay = 0;
        pSpectroParm->bShowPitch = 1;
        pSpectroParm->bShowF1 = 1;
        pSpectroParm->bShowF2 = 1;
        pSpectroParm->bShowF3 = 1;
        pSpectroParm->bShowF4 = 1;
        pSpectroParm->bShowF5andUp = 0;
        pSpectroParm->bSmoothFormantTracks = 1;
        pSpectroParm->nFrequency = 8000;
        pSpectroParm->nMinThreshold = 1;
        pSpectroParm->nMaxThreshold = 233;
        pSpectroParm->nNumberFormants = 3;
        pSpectroParm->bSmoothSpectra = 1;
        pSpectrogram->SetSpectroParm(*pSpectroParm);
        // process spectrogram
        int nResult = pSpectrogram->Process(this,
                                            pDoc, pView, 752, // some arbitrary plot width
                                            500, // some arbitrary plot height (ignored)
                                            0, 1);
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pSpectrogram->GetProcessFileName()/*,7*/));
        }
        else
        {
            EndTest(FALSE);
        }

    }

    //
    // this process does not create a temp file, so we have to create one
    //
    if (bFileOpened && StartTest("CHFrench Spectrum","Test spectrum process."))
    {
        CProcessSpectrum * pSpectrum = pDoc->GetSpectrum();

        SpectrumParm * stParmSpec = pSpectrum->GetSpectrumParms();
        stParmSpec->nSmoothLevel = 3;
        stParmSpec->nPeakSharpFac = 3;
        pSpectrum->SetSpectrumParms(stParmSpec);

        SPECT_PROC_SELECT SpectraSelected;
        SpectraSelected.bCepstralSpectrum = TRUE;    // turn off to reduce processing time
        SpectraSelected.bLpcSpectrum = TRUE;          // use Lpc method for estimating formants

        short int nResult = LOWORD(pSpectrum->Process(this, pDoc,30870,4410,SpectraSelected)); // process data

        DWORD dwDataSize = pSpectrum->GetDataSize();
        void * pData = (char *) pSpectrum->GetProcessedData(0);
        CString szFileName = this->m_szTempPath + "Spectrum.tmp";

        CFile OutputFile(szFileName,CFile::modeCreate|CFile::modeWrite);
        OutputFile.Write(pData,dwDataSize);
        OutputFile.Flush();
        OutputFile.Close();

        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(szFileName/*,1037*/));
        }
        else
        {
            EndTest(FALSE);
        }


        _tunlink(szFileName); // delete the file now
    }

    if (bFileOpened && StartTest("CHFrench Formants","Test formants process."))
    {
        CProcessFormants * pFormants = pDoc->GetFormants();

        SPECT_PROC_SELECT SpectraSelected;
        SpectraSelected.bCepstralSpectrum = TRUE;    // turn off to reduce processing time
        SpectraSelected.bLpcSpectrum = TRUE;          // use Lpc method for estimating formants

        short int nResult = LOWORD(pFormants->Process(this, pDoc,TRUE,30870,4410,SpectraSelected)); // process data
        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(pFormants->GetProcessFileName()/*,20*/));
        }
        else
        {
            EndTest(FALSE);
        }
    }

    //
    // this process does not create a temp file, so we have to create one
    //
    if (bFileOpened && StartTest("CHFrench PointOfArticulation","Test point of articulation process."))
    {
        CProcessPOA * pPOA = pDoc->GetPOA();
        short int nResult = LOWORD(pPOA->Process(this, pDoc,(DWORD)30870,(DWORD)35280)); // process data

        VOCAL_TRACT_MODEL * pData = (VOCAL_TRACT_MODEL *) pPOA->GetProcessedData(0);
        CString szFileName = this->m_szTempPath + "PointOfArticulation.tmp";

        CFile OutputFile(szFileName,CFile::modeCreate|CFile::modeWrite);
        OutputFile.Write(&pData->dErrorRatio,sizeof(pData->dErrorRatio));
        OutputFile.Write(&pData->nNormCrossSectAreas,sizeof(pData->nNormCrossSectAreas));
        for (register int i=0; i<pData->nNormCrossSectAreas; ++i)
        {
            OutputFile.Write(&pData->dNormCrossSectArea[i],sizeof(double));
        }
        OutputFile.Flush();
        OutputFile.Close();

        if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
        {
            EndTest(!FileCompare(szFileName/*,33*/));
        }
        else
        {
            EndTest(FALSE);
        }

        _tunlink(szFileName);
    }

    if (bFileOpened && StartTest("CHFrench TWC","Test tonal weighting chart process."))
    {
        CProcessTonalWeightChart * pTWC = pDoc->GetTonalWeightChart();
        CProcessMelogram * pMelogram = (CProcessMelogram *)pDoc->GetMelogram(); // get pointer to melogram object
        int nLevel = 0, nProgress = 0;
        long lResult = pMelogram->Process(this, pDoc, nProgress, ++nLevel); // process data
        DWORD dwMelDataSize = pMelogram->GetDataSize() * 2; // size of melogram data
        nLevel = (short int)LOWORD(lResult);
        if ((nLevel == PROCESS_CANCELED) || !dwMelDataSize)
        {
            EndTest(FALSE);
        }
        else
        {
            FmtParm * pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
            UINT nBlockAlign = pFmtParm->wBlockAlign;
            WORD wSmpSize = WORD(nBlockAlign / pFmtParm->wChannels);

            DWORD dwRawDataSize = pDoc->GetDataSize(); // size of raw data
            double fScaleFactor = (double)dwRawDataSize / (double)dwMelDataSize;
            DWORD dwFrameStart = (DWORD)((double)pView->GetStartCursorPosition() / fScaleFactor) & ~1; // must be multiple of two
            DWORD dwFrameSize  = ((DWORD)((double)pView->GetStopCursorPosition() / fScaleFactor) & ~1) - dwFrameStart + wSmpSize;
            if (pView->GetStaticTWC())
            {
                dwFrameStart = 0;
                dwFrameSize  = dwMelDataSize;
            }

            short int nResult = LOWORD(pTWC->Process(this, pDoc,dwFrameStart,dwFrameSize, 37,59));  // process data
            if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
            {
                EndTest(!FileCompare(pTWC->GetProcessFileName()));
            }
            else
            {
                EndTest(FALSE);
            }
        }
    }

    /* This one causes an exception
    if (bFileOpened && StartTest("CHFrench GlottalWave","Test glottal wave process."))
    {
    CProcessGlottis * pGlottalWave = pDoc->GetGlottalWave();
    short int nResult = LOWORD(pGlottalWave->Process(this, pDoc)); // process data
    if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
    EndTest(!FileCompare(pGlottalWave->GetProcessFileName()));
    else EndTest(FALSE);
    }*/

    /* This one causes an exception
    if (bFileOpened && StartTest("CHFrench Ratio","Test ratio process."))
    {
    CProcessRatio * pRatio = pDoc->GetRatio();
    short int nResult = LOWORD(pRatio->Process(this, pDoc)); // process data
    if (nResult != PROCESS_ERROR && nResult != PROCESS_CANCELED)
    EndTest(!FileCompare(pRatio->GetProcessFileName()));
    else EndTest(FALSE);
    }*/


    if (bFileOpened && m_pMain->MessageBox(_T("Include visual tests?"),_T("SA Self Test"),MB_YESNO)==IDYES)
    {
        if (StartTest(_T("CHFrench visual ZCrossing"),_T("Visual check of zero crossing graph")))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_ZCROSS,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
        if (StartTest(_T("CHFrench visual RawPitch"),_T("Visual check of raw pitch graph")))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_PITCH,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
        if (StartTest("CHFrench visual CustomPitch","Visual check of custom pitch graph"))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_CHPITCH,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
        if (StartTest("CHFrench visual SmoothedPitch","Visual check of smoothed pitch graph"))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_SMPITCH,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
        if (StartTest("CHFrench visual AutoPitch","Visual check of autopitch graph"))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_GRAPITCH,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
        if (StartTest("CHFrench visual Loudness","Visual check of loudness graph."))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_LOUDNESS,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
        if (StartTest("CHFrench visual Melogram","Visual check of melogram graph."))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_MELOGRAM,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
        if (StartTest("CHFrench visual Change","Visual check of change graph."))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_CHANGE,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
        if (StartTest("CHFrench visual Spectrogram","Visual check of spectrogram graph."))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_SPECTROGRAM,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
        if (StartTest("CHFrench visual Spectrum","Visual check of spectrum graph."))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_SPECTRUM,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
        if (StartTest("CHFrench visual POA","Visual check of point of articulation graph."))
        {
            UINT GrphIDs[MAX_GRAPHS_NUMBER] = {IDD_RAWDATA,IDD_POA,0,0,0,0,0,0,0,0};
            pView->OnGraphsTypesPostProcess(GrphIDs,-1);
            if (m_pMain->MessageBox(_T("Does this graph seem correct?"),m_szTestNumber,MB_YESNO)==IDYES)
            {
                EndTest();
            }
            else
            {
                EndTest(FALSE);
            }
        }
    }

    if (bFileOpened && StartTest("CHFrench Screen Capture","Test File|Screen Capture|Copy Screen with CHFRENCH.WAV"))
    {
        // It would be nice if we could send the keys to the menu (Alt+F to pull down file menu, etc.)
        // bTestSuccess = m_pMain->SendMessage(WM_CHAR, 'F', 0x20000000);

        // ...for now we can just send the message
        EmptyClipboard();
        m_pMain->SendMessage(WM_COMMAND,ID_GRAPHS_SCREENCOPY,0);
        EndTest(CheckClipboard(CF_BITMAP));
    }
    if (bFileOpened && StartTest("CHFrench Graphs Capture","Test File|Screen Capture|Copy Graphs with CHFRENCH.WAV"))
    {
        EmptyClipboard();
        m_pMain->SendMessage(WM_COMMAND,ID_GRAPHS_GRAPHSCOPY,0);
        EndTest(CheckClipboard(CF_BITMAP));
    }
    if (bFileOpened && StartTest("CHFrench Window Capture","Test File|Screen Capture|Copy Window with CHFRENCH.WAV"))
    {
        EmptyClipboard();
        m_pMain->SendMessage(WM_COMMAND,ID_GRAPHS_WINDOWCOPY,0);
        EndTest(CheckClipboard(CF_BITMAP));

        EmptyClipboard(); // why keep it on the clipboard?
    }

    if (bFileOpened && StartTest("CHFrench CutPaste","Cut and paste portions of the file"))
    {
        MessageLoop(1);
        m_pMain->SendMessage(WM_COMMAND,ID_EDIT_SELECTWAVEFORM,0);
        MessageLoop(1);
        m_pMain->SendMessage(WM_COMMAND,ID_EDIT_CUT,0);
        MessageLoop(1);
        m_pMain->SendMessage(WM_COMMAND,ID_EDIT_PASTE,0);
        MessageLoop(1);
        m_pMain->SendMessage(WM_COMMAND,ID_EDIT_UNDO,0);
        MessageLoop(1);
        EndTest();
    }

    if (bFileOpened && StartTest("CHFrench CursorPositioning","Position the cursors at ends of file"))
    {
        pView->SetStartCursorPosition(0,SNAP_LEFT);
        pView->SetStopCursorPosition(pDoc->GetDataSize(),SNAP_RIGHT);

        DWORD dwStart = pView->GetStartCursorPosition();
        DWORD dwStop  = pView->GetStopCursorPosition();
        DWORD dwSize  = pDoc->GetDataSize();
        if (!dwStart    && dwStop==dwSize - (pFP->wBitsPerSample/8))
        {
            EndTest();
        }
        else
        {
            EndTest(FALSE);
        }
    }

    if (bFileOpened && StartTest("CHFrench close","Close active document"))
    {
        bTestSuccess = m_pMain->SendMessage(WM_COMMAND,ID_FILE_CLOSE,0);
        pDoc = NULL;
        bFileOpened = FALSE;
        EndTest(bTestSuccess);
    }
}//SelfTest


//
// CSASelfTest constructor
//
CSASelfTest::CSASelfTest()
{
    TCHAR szString[_MAX_PATH] = _T("");

    m_pApp = (CSaApp *) AfxGetApp();
    m_pMain = (CMainFrame *) AfxGetMainWnd();

    TCHAR szExeFilename[_MAX_PATH];
    GetModuleFileName(AfxGetInstanceHandle(),szExeFilename,_MAX_PATH);

    GetTempPath(_MAX_PATH,szString);
    m_szTempPath = szString;

    m_szExeFolderPath = szExeFilename;
    int LastBackslash = m_szExeFolderPath.ReverseFind('\\');
    m_szExeFolderPath.Delete(LastBackslash+1,m_szExeFolderPath.GetLength()-(LastBackslash+1));
    m_szTestFolderPath = m_szExeFolderPath + "SelfTest\\";
    m_szLogFileName = m_szTestFolderPath + "SelfTest.xml";
    m_szTestNumber = "";
    m_nTestsPassed=m_nTestsFailed=0;

    CreateDirectory(m_szTestFolderPath,NULL);
    CFile LogFile(m_szLogFileName,CFile::modeWrite|CFile::modeCreate);
    LogFile.Write("<?xml version=\"1.0\" ?>\r\n",24);
    LogFile.Write("<SelfTest>\r\n",12);

    LogFile.Write("\t<Summary>\r\n",12);

    LogFile.Write("\t\t<Status>Completed 000 tests.</Status>\r\n",41);

    LogFile.Write("\t\t<DateTime>",12);
    SYSTEMTIME st;
    GetLocalTime(&st);
    CString szTimeDate;
    szTimeDate.Format(_T("%2.2lu/%2.2lu/%4.4lu %2.2lu:%2.2lu.%3.3lu"),st.wMonth,st.wDay,st.wYear,st.wHour,st.wMinute,st.wMilliseconds);
    LogFile.Write(szTimeDate,szTimeDate.GetLength());
    LogFile.Write("</DateTime>\r\n",13);

    LogFile.Write("\t\t<TestsPassed>0</TestsPassed>  \r\n"
                  "\t\t<TestsFailed>0</TestsFailed>  \r\n"
                  "\t</Summary>\r\n",81);

    LogFile.Write("\t<SysInfo>\r\n",12);

    SYSTEM_INFO si;
    ::GetSystemInfo(&si);

    LogFile.Write("\t\t<User>",8);
    ULONG UserNameLength = _MAX_PATH;
    ::GetUserName(szString,&UserNameLength);
    LogFile.Write(szString,_tcslen(szString));
    LogFile.Write("</User>\r\n",9);

    LogFile.Write("\t\t<Computer>",12);
    UserNameLength = _MAX_PATH;
    ::GetComputerName(szString,&UserNameLength);
    LogFile.Write(szString,_tcslen(szString));
    LogFile.Write("</Computer>\r\n",13);

    LogFile.Write("\t\t<OS>",6);
    OSVERSIONINFO vi;
    CString szVersion;
    vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    ::GetVersionEx(&vi);
    switch (vi.dwPlatformId)
    {
    case VER_PLATFORM_WIN32s:
        szVersion.Format(_T("Windows %lu.%lu (build %u) with Win32s"),vi.dwMajorVersion,vi.dwMinorVersion,(UINT)LOWORD(vi.dwBuildNumber));
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        if (vi.dwMinorVersion==0)
        {
            szVersion.Format(_T("Windows 95 (%lu.%lu, build %u)"),vi.dwMajorVersion,vi.dwMinorVersion,(UINT)LOWORD(vi.dwBuildNumber));
        }
        else
            szVersion.Format(_T("Windows %s (%lu.%lu, build %lu)"),
                             vi.dwMinorVersion>50?_T("ME"):_T("98"),vi.dwMajorVersion,vi.dwMinorVersion,vi.dwBuildNumber);
        break;
    case VER_PLATFORM_WIN32_NT:
    {
        char * szType = "NT";
        if (vi.dwMajorVersion==5 && vi.dwMinorVersion < 50)
        {
            szType = "2000";
        }
        else if (vi.dwMajorVersion>=5)
        {
            szType = "XP";
        }
        szVersion.Format(_T("Windows %s (%lu.%lu, build %lu)"),szType,vi.dwMajorVersion,vi.dwMinorVersion,vi.dwBuildNumber);
    }
    }
    LogFile.Write(szVersion,szVersion.GetLength());
    if (vi.szCSDVersion && *vi.szCSDVersion)
    {
        LogFile.Write(" - ",3);
        LogFile.Write(vi.szCSDVersion,_tcslen(vi.szCSDVersion));
    }
    LogFile.Write("</OS>\r\n",7);

    LogFile.Write("\t\t<Processor>",13);
    switch (si.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_INTEL:
        switch (si.dwProcessorType)
        {
        case PROCESSOR_INTEL_386:
            LogFile.Write("386",3);
            break;
        case PROCESSOR_INTEL_486:
            LogFile.Write("486",3);
            break;
        case PROCESSOR_INTEL_PENTIUM:
            LogFile.Write("Pentium",7);
            break;
        }
        break;
    case PROCESSOR_ARCHITECTURE_MIPS:
        LogFile.Write("MIPS",5);
        break;
    case PROCESSOR_ARCHITECTURE_ALPHA:
        LogFile.Write("ALPHA",5);
        break;
    case PROCESSOR_ARCHITECTURE_PPC:
        LogFile.Write("PPC",5);
        break;
    case PROCESSOR_ARCHITECTURE_UNKNOWN:
        LogFile.Write("UNKNOWN",5);
        break;
    }
    LogFile.Write("</Processor>\r\n",14);

    LogFile.Write("\t\t<SAVersion>",13);
    CString szSAVersion;
    szSAVersion.LoadString(VS_VERSION);
    LogFile.Write(szSAVersion,szSAVersion.GetLength());
    LogFile.Write("</SAVersion>\r\n",14);

    LogFile.Write("\t</SysInfo>\r\n",13);


    LogFile.Write("</SelfTest>\r\n",13);
    LogFile.Flush();
    LogFile.Close();

    SelfTest();
}//CSASelfTest constructor

//
// CSASelfTest destructor
//
CSASelfTest::~CSASelfTest()
{
    CString szMessage = "";
    szMessage.Format(_T("%u tests passed of %u."),m_nTestsPassed,m_nTestsPassed+m_nTestsFailed);
    m_pMain->MessageBox(szMessage,_T("SA self test complete"));

    // open selftest.xml if some tests failed
    if (m_nTestsFailed)
    {
        HKEY hXML = NULL;
        if (RegCreateKey(HKEY_CLASSES_ROOT,_T(".xml"),&hXML)==ERROR_SUCCESS)
        {
            TCHAR szXMLClassName[256] = _T("");
            long Length = 256;
            if (RegQueryValue(hXML,NULL,szXMLClassName,&Length)==ERROR_SUCCESS)
            {
                HKEY hXMLClass = NULL;
                if (RegCreateKey(HKEY_CLASSES_ROOT,szXMLClassName,&hXMLClass)==ERROR_SUCCESS)
                {
                    HKEY hXMLShellOpenCommand = NULL;
                    if (RegCreateKey(hXMLClass,_T("shell\\Open\\command"),&hXMLShellOpenCommand)==ERROR_SUCCESS)
                    {
                        TCHAR szXMLCommand[_MAX_PATH];
                        Length = _MAX_PATH;
                        if (RegQueryValue(hXMLShellOpenCommand,NULL,szXMLCommand,&Length)==ERROR_SUCCESS)
                        {
                            CSaString szCommand = szXMLCommand;
                            CSaString szRestOfCommand = " ";
                            if (*szXMLCommand=='\"')
                            {
                                szCommand = szCommand.Right(szCommand.GetLength()-1);
                                int nEnd = szCommand.FindOneOf(_T("\""));
                                if (nEnd)
                                {
                                    szRestOfCommand = szCommand.Right(szCommand.GetLength()-nEnd-1);
                                    szRestOfCommand.TrimLeft();
                                    szCommand = szCommand.Left(nEnd);
                                }

                            }
                            else
                            {
                                int nEnd = szCommand.FindOneOf(_T(" /"));
                                if (nEnd)
                                {
                                    szRestOfCommand = szCommand.Right(szCommand.GetLength()-nEnd-1);
                                    szCommand = szCommand.Left(nEnd);
                                }
                            }
                            _wspawnl(_P_NOWAIT, (const TCHAR *)szCommand, (const TCHAR *)szRestOfCommand, (const TCHAR *)m_szLogFileName, NULL);
                            //"file:////" + m_szTestFolderPath + "selftest.xml", NULL);
                        }
                    }
                }
            }
        }
    }
}//CSASelfTest destructor


//
// StartTest()
//
// If resuming, checks to see if this test should be skipped; if so, returns FALSE
//
// Otherwise, logs the beginning of the test and returns TRUE
//
BOOL CSASelfTest::StartTest(const CString szTestNumber, const CString szDescription)
{
    m_szTestNumber = szTestNumber;

    CFile LogFile(m_szLogFileName,CFile::modeWrite);

    LogFile.Seek(STATUS_MESSAGE,CFile::begin);
    CString szStatusMessage;
    szStatusMessage.Format(_T("TEST %3.3d IN PROGRESS"),m_nTestsPassed+m_nTestsFailed+1);
    LogFile.Write(szStatusMessage,szStatusMessage.GetLength());

    LogFile.Seek(-13,CFile::end);
    LogFile.Write("\t<Test ID=\"",11);
    LogFile.Write(m_szTestNumber,m_szTestNumber.GetLength());
    LogFile.Write("\">\r\n",4);
    if (szDescription.GetLength())
    {
        LogFile.Write("\t\t<Description>",15);
        LogFile.Write(szDescription,szDescription.GetLength());
        LogFile.Write("</Description>\r\n",16);
    }
    LogFile.Write("\t</Test>\r\n",10);
    LogFile.Write("</SelfTest>\r\n",13);
    LogFile.Flush();
    LogFile.Close();
    return TRUE;
}//StartTest

//
// EndTest()
//
// Logs the end of the current test
//
void CSASelfTest::EndTest(BOOL bSuccess)
{
    CFile LogFile(m_szLogFileName,CFile::modeWrite);
    LogFile.Seek(-23,CFile::end);

    LogFile.Write("\t\t<Success>",11);
    if (bSuccess)
    {
        ++m_nTestsPassed;
        LogFile.Write("true",4);
    }
    else
    {
        ++m_nTestsFailed;
        LogFile.Write("false",5);
    }
    LogFile.Write("</Success>\r\n",12);

    LogFile.Write("\t</Test>\r\n",10);
    LogFile.Write("</SelfTest>\r\n",13);

    TCHAR nNumber[256] = _T("");

    LogFile.Seek(STATUS_MESSAGE,CFile::begin);
    CString szStatusMessage;
    szStatusMessage.Format(_T("Completed %3.3d tests."),m_nTestsPassed+m_nTestsFailed);
    LogFile.Write(szStatusMessage,szStatusMessage.GetLength());

    if (bSuccess)
    {
        LogFile.Seek(N_TESTS_PASSED,CFile::begin);
        _itow_s(m_nTestsPassed,nNumber,_countof(nNumber),10);
        LogFile.Write(nNumber,_tcslen(nNumber));
        LogFile.Write("</TestsPassed>",14);
    }
    else
    {
        LogFile.Seek(N_TESTS_FAILED,CFile::begin);
        _itow_s(m_nTestsFailed,nNumber,_countof(nNumber),10);
        LogFile.Write(nNumber,_tcslen(nNumber));
        LogFile.Write("</TestsFailed>",14);
    }

    LogFile.Flush();
    LogFile.Close();
}//EndTest

//
// LogEntry()
//
// Writes the message into the log file in the current test's area
//
void CSASelfTest::LogEntry(const CString szMessage)
{
    BOOL bInTest = m_szTestNumber.GetLength();

    CFile LogFile(m_szLogFileName,CFile::modeWrite);
    LogFile.Seek(bInTest?-23:-13,CFile::end);
    if (bInTest)
    {
        LogFile.Write("\t",1);
    }
    LogFile.Write("\t<Entry>",8);
    LogFile.Write(szMessage,szMessage.GetLength());
    LogFile.Write("</Entry>\r\n",10);
    if (bInTest)
    {
        LogFile.Write("\t</Test>\r\n",10);
    }
    LogFile.Write("</SelfTest>\r\n",13);
    LogFile.Flush();
    LogFile.Close();
}//LogEntry

//
// FileCompare()
//
// Compares two files. If they are different, logs the difference
// Returns -1 if one of the files doesn't exist or if the file sizes are different.
// Returns 0 if number of bytes differing is <= nTolerableDifferences
// Otherwise returns the number of bytes in the file which are different.
//
// If FileTwo is NULL, uses the "SelfTest\*.tmp" file, where * corresponds to the test name
//
#define BUFFERSIZE 1024
int CSASelfTest::FileCompare(const CString szFileOne, long nTolerableDifferences, CString szFileTwo)
{
    long nFileDifferences = 0;
    long nFirstDifference = -1;

    if (!szFileTwo.GetLength())
    {
        szFileTwo = m_szTestFolderPath + m_szTestNumber + ".tmp";
    }

    if (!FileExists(szFileOne))
    {
        CString szMessage = "Unable to open file " + szFileOne;
        LogEntry(szMessage);
    }
    if (!FileExists(szFileTwo))
    {
        CString szMessage = "Unable to open file " + szFileTwo;
        CString szBenchMessage = szFileTwo + " does not exist.\r\n\nAre you intending to create the benchmark at this time?";
        if (m_pMain->MessageBox(szBenchMessage,m_szTestNumber,MB_YESNO)==IDYES)
        {
            CFile File1(szFileOne,CFile::modeRead|CFile::typeBinary);
            CFile File2(szFileTwo,CFile::modeWrite|CFile::modeCreate|CFile::typeBinary);
            char Buffer[BUFFERSIZE];

            for (;;)
            {
                UINT nBytesRead = File1.Read(Buffer,BUFFERSIZE);
                if (!nBytesRead)
                {
                    File1.Close();
                    File2.Flush();
                    File2.Close();
                    return 0;
                }
                File2.Write(Buffer,nBytesRead);
            }
        }
        else
        {
            LogEntry(szMessage);
        }
    }
    if (!FileExists(szFileOne)||!FileExists(szFileTwo))
    {
        LogEntry("Neither file exists.");
        return -1;
    }

    CFile File1(szFileOne,CFile::modeRead|CFile::typeBinary);
    CFile File2(szFileTwo,CFile::modeRead|CFile::typeBinary);
    char Buffer1[BUFFERSIZE], Buffer2[BUFFERSIZE];


    DWORD dwFile1Len=File1.GetLength();
    DWORD dwFile2Len=File2.GetLength();
    if (dwFile1Len!=dwFile2Len)   // Files are different length
    {
        CString szLen;
        szLen.Format(_T("%lu"),dwFile1Len);
        LogEntry(szFileOne + " has length of " + szLen);
        szLen.Format(_T("%lu"),dwFile2Len);
        LogEntry(szFileTwo + " has length of " + szLen);

        File1.Close();
        File2.Close();
        return -1;
    }


    for (UINT nChunkNumber = 0;; ++nChunkNumber)
    {
        UINT nRead = File1.Read((void *)Buffer1,BUFFERSIZE);
        nRead = File2.Read((void *)Buffer2,BUFFERSIZE);

        for (register unsigned i=1; i<nRead; ++i)
        {
            if (Buffer1[i-1]!=Buffer2[i-1])   // Files have different content
            {
                if (nFirstDifference == -1)
                {
                    nFirstDifference = (((DWORD)nChunkNumber)*BUFFERSIZE)+i;
                }
                ++nFileDifferences;
            }
        }

        if (nRead!=BUFFERSIZE)
        {
            break;
        }
    }
    if (nFirstDifference != -1 && nFileDifferences > nTolerableDifferences)
    {
        CString szDifferences;
        szDifferences.Format(_T("Files differ in %ld of %lu bytes"),nFileDifferences,File1.GetLength());
        LogEntry(szDifferences);
        LogHexDataCompare(File1,File2,nFirstDifference);

        File1.Close();
        File2.Close();
        return nFileDifferences;
    }
    File1.Close();
    File2.Close();
    return 0;
}//FileCompare

/***************************************************************************/
// CSASelfTest::MessageLoop Do windows message loop
// This function enables key down message to come through while
// processing.
/***************************************************************************/
void CSASelfTest::MessageLoop(DWORD dwMilliSeconds)
{
    DWORD dwStartTime=::GetTickCount();
    BOOL bDoingBackgroundProcessing = TRUE;

    while (bDoingBackgroundProcessing && (dwStartTime+dwMilliSeconds > ::GetTickCount()))
    {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if (!AfxGetApp()->PumpMessage())
            {
                bDoingBackgroundProcessing = FALSE;
                //::PostQuitMessage( );
                break;
            }
        }
        // let MFC do its idle processing
        LONG lIdle = 0;
        while (AfxGetApp()->OnIdle(lIdle++))
            ;
        // Perform some background processing here
        // using another call to OnIdle
    }
}

BOOL CSASelfTest::LogHexDataCompare(CFile & FileOne,CFile & FileTwo,UINT HighlightPosition)
{
    BOOL bInTest = m_szTestNumber.GetLength();
    CString szOutString;
    UINT BeginPosition=(HighlightPosition>7)?HighlightPosition-8:0;
    unsigned char Buffer[16] = "";
    register UINT i=0;

    CFile LogFile(m_szLogFileName,CFile::modeWrite);
    LogFile.Seek(bInTest?-23:-13,CFile::end);

    if (bInTest)
    {
        LogFile.Write("\t",1);
    }
    LogFile.Write("\t<FileDifference ID1=\"",22);
    LogFile.Write(FileOne.GetFileName(),FileOne.GetFileName().GetLength());
    LogFile.Write("\" ID2=\"",7);
    LogFile.Write(FileTwo.GetFileName(),FileTwo.GetFileName().GetLength());
    LogFile.Write("\">\r\n",4);

    CString szNotice;
    szNotice.Format(_T("\t\t<Notice>Files diverge at byte %lu.</Notice>\r\n"),HighlightPosition);
    if (bInTest)
    {
        LogFile.Write("\t",1);
    }
    LogFile.Write(szNotice,szNotice.GetLength());

    if (bInTest)
    {
        LogFile.Write("\t",1);
    }
    LogFile.Write("\t\t<Data1>",9);
    FileOne.Seek(BeginPosition,CFile::begin);
    UINT nRead = FileOne.Read(Buffer,16);
    for (i=0; i<nRead; ++i)
    {
        char szHexValue[2] = "";
        szHexValue[0] = (char)(Buffer[i]/16);
        if (szHexValue[0]<=9)
        {
            szHexValue[0] += '0';
        }
        else
        {
            szHexValue[0]+=('A'-10);
        }
        szHexValue[1] = (char)(Buffer[i]%16);
        if (szHexValue[1]<=9)
        {
            szHexValue[1] += '0';
        }
        else
        {
            szHexValue[1]+=('A'-10);
        }

        if (i+BeginPosition==HighlightPosition-1)
        {
            LogFile.Write("*",1);
        }
        LogFile.Write(szHexValue,2);
        if (i+BeginPosition==HighlightPosition-1)
        {
            LogFile.Write("*",1);
        }
        if (i<nRead-1)
        {
            LogFile.Write(" ",1);
        }
    }
    LogFile.Write("</Data1>\r\n",10);

    if (bInTest)
    {
        LogFile.Write("\t",1);
    }
    LogFile.Write("\t\t<Data2>",9);
    FileTwo.Seek(BeginPosition,CFile::begin);
    nRead = FileTwo.Read(Buffer,16);
    for (i=0; i<nRead; ++i)
    {
        char szHexValue[2] = "";
        szHexValue[0] = (char)(Buffer[i]/16);
        if (szHexValue[0]<=9)
        {
            szHexValue[0] += '0';
        }
        else
        {
            szHexValue[0]+=('A'-10);
        }
        szHexValue[1] = (char)(Buffer[i]%16);
        if (szHexValue[1]<=9)
        {
            szHexValue[1] += '0';
        }
        else
        {
            szHexValue[1]+=('A'-10);
        }

        if (i+BeginPosition==HighlightPosition-1)
        {
            LogFile.Write("*",1);
        }
        LogFile.Write(szHexValue,2);
        if (i+BeginPosition==HighlightPosition-1)
        {
            LogFile.Write("*",1);
        }
        if (i<nRead-1)
        {
            LogFile.Write(" ",1);
        }
    }
    LogFile.Write("</Data2>\r\n",10);

    if (bInTest)
    {
        LogFile.Write("\t",1);
    }
    LogFile.Write("\t</FileDifference>\r\n",20);

    if (bInTest)
    {
        LogFile.Write("\t</Test>\r\n",10);
    }
    LogFile.Write("</SelfTest>\r\n",13);
    LogFile.Flush();
    LogFile.Close();
    return TRUE;
}//LogHexData

void CSASelfTest::EmptyClipboard()
{
    if (::OpenClipboard(*m_pMain))
    {
        ::EmptyClipboard();
        ::CloseClipboard();
    }
}

BOOL CSASelfTest::CheckClipboard(UINT nFormat)   //CF_BITMAP
{
    BOOL bReturn = FALSE;
    if (::OpenClipboard(*m_pMain))
    {
        bReturn = ::IsClipboardFormatAvailable(nFormat);
        ::CloseClipboard();
    }
    return bReturn;
}
