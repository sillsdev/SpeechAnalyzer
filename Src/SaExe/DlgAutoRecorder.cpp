//////////////////////////////////////////////////////////////////////////////
// autoRecorder.cpp:
// Implementation of the CDlgAutoRecorder (dialog)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg2.cpp
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "DlgAutoRecorder.h"
#include "fileInformation.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "doclist.h"
#include "sa_mplot.h"
#include "sa.h"
#include "FileUtils.h"
#include "WaveOutDevice.h"
#include "WaveInDevice.h"
#include "FmtParm.h"
#include "Process\Process.h"
#include "Process\sa_p_fra.h"
#include "settings\obstream.h"

//###########################################################################
// CDlgAutoRecorder dialog
// Displays the recorder that allows the user to control the wave data
// recording process, done by the CWave class.

/////////////////////////////////////////////////////////////////////////////
// CDlgAutoRecorder message map

BEGIN_MESSAGE_MAP(CDlgAutoRecorder, CDialog)
    ON_BN_CLICKED(IDC_STOP, OnStop)
    ON_WM_CLOSE()
    ON_COMMAND(IDC_VOLUMESLIDER, OnVolumeSlide)
    ON_COMMAND(IDC_VOLUMESCROLL, OnVolumeScroll)
    ON_EN_KILLFOCUS(IDC_VOLUMEEDIT, OnKillfocusVolumeEdit)
    ON_COMMAND(IDC_RECVOLUMESLIDER, OnRecVolumeSlide)
    ON_COMMAND(IDC_RECVOLUMESCROLL, OnRecVolumeScroll)
    ON_EN_KILLFOCUS(IDC_RECVOLUMEEDIT, OnKillfocusRecVolumeEdit)
    ON_MESSAGE(MM_MIXM_CONTROL_CHANGE, OnMixerControlChange)
    ON_WM_SHOWWINDOW()
    ON_WM_DESTROY()
    ON_MESSAGE(WM_USER_AUTO_RESTART, OnAutoRestart)
    ON_BN_CLICKED(IDC_PLAY, OnPlay)
    ON_COMMAND(IDHELP, OnHelpAutoRecorder)
END_MESSAGE_MAP()

/***************************************************************************/
// CDlgAutoRecorder::CDlgAutoRecorder Constructor
/***************************************************************************/
CDlgAutoRecorder::CDlgAutoRecorder(CSaDoc * pDoc, CSaView * pView, CSaView * pTarget, CAlignInfo & alignInfo) :
    CDialog(IDD)
{
    m_hmmioFile = NULL;
    m_szFileName[0] = 0; // no file name

    m_bStopPending = false;
    m_pDoc = pDoc;
    m_pView = pView;
    m_pTargetUntested = pTarget;

    m_eMode = Disabled;
    m_eOldMode = Disabled;

    m_AlignInfo = alignInfo;

    ChangeState(WaitForSilence);

    m_NotifyObj.Attach(this); // attach notify object
    m_pWave = new CWave;      // create CWave object
    BOOL bResult = FALSE;
    m_nVolume = m_pWave->GetVolume(bResult);
    m_dwRecordSize = 0;
    m_dwPlayPosition = 0;
    m_bFileReady = TRUE;
    m_bFileApplied = FALSE;

    // allocate wave data buffer
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    m_hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, MMIO_BUFFER_SIZE); // allocate memory
    if (!m_hData)
    {
        // memory allocation error
        pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
        return;
    }
    m_lpRecData = (HPSTR)::GlobalLock(m_hData); // lock memory
    if (m_lpRecData==NULL)
    {
        // memory lock error
        pApp->ErrorMessage(IDS_ERROR_MEMLOCK);
        ::GlobalFree(m_hData);
        return;
    }
}

CDlgAutoRecorder::~CDlgAutoRecorder()
{
    CleanUp();
};

/////////////////////////////////////////////////////////////////////////////
// CDlgAutoRecorder helper functions

/***************************************************************************/
// CDlgAutoRecorder::DoDataExchange Data exchange
/***************************************************************************/
void CDlgAutoRecorder::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
}

/***************************************************************************/
// CDlgAutoRecorder::SetTotalTime Set total time display
/***************************************************************************/
void CDlgAutoRecorder::SetTotalTime()
{
    double fDataSec = m_pDoc->GetTimeFromBytes(m_dwRecordSize); // calculate time
    m_LEDTotalTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
}

/***************************************************************************/
// CDlgAutoRecorder::SetPositionTime Set position time display
/***************************************************************************/
void CDlgAutoRecorder::SetPositionTime()
{
    if ((m_eMode == Record) || ((m_eMode == Monitor) && (m_eOldMode == Record)))
    {
        double fDataSec = m_pDoc->GetTimeFromBytes(m_dwRecordSize); // calculate time
        m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
    }
}

/***************************************************************************/
// CDlgAutoRecorder::BlockStored Recording Block finished
// The actually recording data block has been finished. Update the dialog
// display controls.

//NOTE! this is a "callback" function.  Don't attempt to close the window
//from within this method, or call anything that does.
/***************************************************************************/
void CDlgAutoRecorder::BlockStored(UINT nLevel, DWORD dwPosition, BOOL * bSaveOverride)
{
    // TRACE(_T("Block Stored %g\n"), double(dwPosition));

    double fDataSec = 0.0;
    // update the VU bar
    m_VUBar.SetVU((int)nLevel);

    switch (m_eState)
    {
    case WaitForSilence:
        ASSERT(m_eMode == Monitor);
        if (m_bStopPending)
        {
            ChangeState(Stopping);
        }
        else if (GetTickCount() - m_dwTickCount > 2*1000)
        {
            SetRecorderMode( Record);            // start recording now
            ChangeState(WaitingForVoice);
        }
        else if (nLevel >= MIN_VOICE_LEVEL)
        {
            m_dwTickCount = GetTickCount(); // Restart silence timer
        }
        break;
    case WaitingForVoice:
        ASSERT(m_eMode == Record);
        if (bSaveOverride)
        {
            *bSaveOverride = FALSE;
        }
        if (m_bStopPending)
        {
            ChangeState(Stopping);
        }
        else if (nLevel > MIN_VOICE_LEVEL)
        {
            // start recording
            if (bSaveOverride)
            {
                *bSaveOverride = TRUE;
            }
            ChangeState(Recording);
        }
        break;
    case Recording:
        ASSERT(m_eMode == Record);
        m_dwRecordSize = dwPosition;
        // update the time
        fDataSec = m_pDoc->GetTimeFromBytes(m_dwRecordSize); // get sampled data size in seconds
        m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
        if (m_bStopPending || fDataSec > m_AlignInfo.dTotalLength)
        {
            ChangeState(Stopping);
        }
        else if (nLevel < MAX_SILENCE_LEVEL)
        {
            if (GetTickCount() - m_dwTickCount > 2*1000)
            {
                // shutdown recording
                ChangeState(Stopping);
                TRACE(_T("Stopping\n"));
            }
        }
        else if (nLevel >= MAX_SILENCE_LEVEL)
        {
            m_dwTickCount = GetTickCount();  // Restart silence timer
        }
        break;
    case Stopping:
        m_dwRecordSize = dwPosition;
        // update the time
        fDataSec = m_pDoc->GetTimeFromBytes(m_dwRecordSize); // get sampled data size in seconds
        TRACE(_T("Match %f %f\n"),fDataSec, m_AlignInfo.dTotalLength);
        m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
        if (m_bStopPending)
        {
            SetRecorderMode(Stop);
            ChangeState(Idle);
            StartShutdown();
        }
        else
        {
            ASSERT(m_eMode == Record);
            // we've reached 2 seconds of silence
            // stop recording, but don't delete the wave file.
            if (m_eMode != Stop)
            {
                m_pWave->Stop();    // stop recording
            }
            if (m_pDoc->GetSaParm()->wFlags & SA_FLAG_HIGHPASS)
            {
                HighPassFilter();
            }

            ASSERT(m_hmmioFile);
            m_VUBar.SetVU(0); // reset the VU bar
            m_eMode = Stop;
            SetPositionTime();

            if (((m_bFileReady) && (!m_bFileApplied)) &&
                    (m_dwRecordSize > 0))
            {
                if (!Apply())
                {
                    m_bFileReady = FALSE;
                    CleanUp();
                    break;
                }
                GetDlgItem(IDC_PLAY)->EnableWindow(TRUE);
            }

            // disable the wave device so it can be restarted
            SetRecorderMode(Stop);

            // if the data is applied, restart recording
            // if the apply didn't work, then it's time to stop
            if (!m_bStopPending)
            {
                CSaView * pTarget = GetTarget();
                if (pTarget)
                {
                    LRESULT lResult = OnAssignOverlay(m_pView);
                    if (lResult)
                    {
                        PostMessage(WM_USER_AUTO_RESTART,0,0);
                    }
                    else
                    {
                        StartShutdown();
                    }
                }
                else
                {
                    StartShutdown();
                }
            }
            else
            {
                StartShutdown();
            }
            ChangeState(Idle);
        }
        break;

    case Idle:
        //sit and do nothing
        break;
    }
}

/***************************************************************************/
// CDlgAutoRecorder::StoreFailed Recorded block has been failed storing
/***************************************************************************/
void CDlgAutoRecorder::StoreFailed()
{
    TRACE(_T("StoreFailed\n"));
    // stop the recorder
    SetRecorderMode(Stop);
    // inform the user
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    pApp->ErrorMessage(IDS_ERROR_STOREWAVEBLOCK, m_szFileName);
}

/***************************************************************************/
// CDlgAutoRecorder::BlockFinished Playing Block finished
// The actually playing data block has been finished playing. Update the
// dialog display controls.
/***************************************************************************/
void CDlgAutoRecorder::BlockFinished(UINT nLevel, DWORD dwPosition, UINT)
{
    TRACE(_T("Block Finished %g\n"), double(dwPosition));
    m_dwPlayPosition = dwPosition;
    // update the VU bar
    m_VUBar.SetVU((int)nLevel);
    // update the time
    CSaDoc * pDoc = (CSaDoc *)m_pDoc;
    double fDataSec = pDoc->GetTimeFromBytes(dwPosition); // get sampled data size in seconds
    m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
}

/***************************************************************************/
// CDlgAutoRecorder::EndPlayback Playback finished
// The playback has been finished. Set recorder mode to stop.
/***************************************************************************/
void CDlgAutoRecorder::EndPlayback()
{
    TRACE(_T("End Playback\n"));
    if (m_eMode == Play)
    {
        SetRecorderMode(Record);            // start recording now
        ChangeState(WaitingForVoice);
    }
}

/***************************************************************************/
// CDlgAutoRecorder::GetWaveData Request for next wave data block to play
// This function delivers the next wave data block to play to the CWave
// object from the recorder internal (just recorded) wave file. The file is
// always open, the file pointer is already in the data subchunk and the
// function just delivers the block requested.
/***************************************************************************/
HPSTR CDlgAutoRecorder::GetWaveData(DWORD dwPlayPosition, DWORD dwDataSize)
{

    //TRACE(_T("GetWaveData %g %g\n"), double(dwPlayPosition), double(dwDataSize));
    CSaDoc * pDoc = (CSaDoc *)m_pDoc;
    DWORD dwWaveBufferSize = pDoc->GetWaveDataBufferSize();
    if (((dwPlayPosition + dwDataSize) > (pDoc->GetWaveBufferIndex() + dwWaveBufferSize)) ||
            ((dwPlayPosition + dwDataSize) > (dwPlayPosition - (dwPlayPosition % dwWaveBufferSize) + dwWaveBufferSize)))
    {
        return pDoc->GetWaveData(dwPlayPosition, TRUE); // get pointer to data block
    }
    else
    {
        HPSTR pData = pDoc->GetWaveData(dwPlayPosition); // get pointer to data block
        if (pData == NULL)
        {
            return NULL;    // error while reading data
        }
        pData += dwPlayPosition - pDoc->GetWaveBufferIndex();
        return pData;
    }
}

/***************************************************************************/
// CWave::HighPassFilter Filter recorded wave file
// If the highpass flag is set, the sample data in the wave file is
// filtered, eliminating frequency components below 70 Hz.  Assumes file
// is already open and positioned at the end of the data chunk.
/***************************************************************************/
void CDlgAutoRecorder::HighPassFilter()
{

    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application

    // get out of 'data' chunk
    if (mmioAscend(m_hmmioFile, &m_mmckinfoSubchunk, 0))
    {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szFileName);
        m_bFileReady = FALSE;
        return;
    }

    // get data sizes
    DWORD wSmpSize = m_pDoc->GetSampleSize();                       // sample size in bytes
    DWORD dwRecordingSize = m_mmckinfoSubchunk.cksize;              // recording size in bytes

    // get out of 'RIFF' chunk, to write RIFF size
    if (mmioAscend(m_hmmioFile, &m_mmckinfoParent, 0))
    {
        // error writing RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, m_szFileName);
        m_bFileReady = FALSE;
        return;
    }

    // set file pointer to begin of file
    mmioSeek(m_hmmioFile, 0, SEEK_SET);

    // descend into 'RIFF' chunk again
    if (mmioDescend(m_hmmioFile, &m_mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
        // error descending into RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WAVECHUNK, m_szFileName);
        m_bFileReady = FALSE;
        return;
    }

    // descend into 'data' chunk
    if (mmioDescend(m_hmmioFile, &m_mmckinfoSubchunk, &m_mmckinfoParent, MMIO_FINDCHUNK))
    {
        // error descending into data chunk
        pApp->ErrorMessage(IDS_ERROR_READDATACHUNK, m_szFileName);
        m_bFileReady = FALSE;
        return;
    }

    // filter recording backwards-in-time
    CWaveInDevice * pRecorder = m_pWave->GetInDevice();
    CHighPassFilter70 * pHighPassFilter = pRecorder->GetHighPassFilter();     //!!check if NULL?

    DWORD dwDataOffset = dwRecordingSize;
    mmioSeek(m_hmmioFile, (long)dwDataOffset, SEEK_CUR);   // go to end of data chunk
    long lDataSize = MMIO_BUFFER_SIZE;   //!!buffer size assumed to be even

    do
    {
        if (dwDataOffset < MMIO_BUFFER_SIZE)
        {
            lDataSize = dwDataOffset;    // last block less than buffer size
        }
        dwDataOffset -= (DWORD)lDataSize;
        mmioSeek(m_hmmioFile, -lDataSize, SEEK_CUR);                    // move to start of block
        if (mmioRead(m_hmmioFile, m_lpRecData, lDataSize) == -1)           // load it
        {
            // error
            pApp->ErrorMessage(IDS_ERROR_READDATACHUNK);
            m_bFileReady = FALSE;
            return;
        }

        if (wSmpSize == 1)
        {
            // 8-bit unsigned
            pHighPassFilter->BackwardPass((unsigned char *)m_lpRecData, (unsigned long)lDataSize);    // filter backwards
            //!!ck. return code?
        }
        else
        {
            // 16-bit signed
            pHighPassFilter->BackwardPass((short *)m_lpRecData, (unsigned long)lDataSize/wSmpSize);   // filter backwards
            //!!ck. return code?
        }

        mmioSeek(m_hmmioFile, -lDataSize, SEEK_CUR);                    // return to start of block
        if (mmioWrite(m_hmmioFile, m_lpRecData, lDataSize) == -1)          // write filtered data
        {
            // error
            pApp->ErrorMessage(IDS_ERROR_RECHPFILTER);
            m_bFileReady = FALSE;
        }
        mmioSeek(m_hmmioFile, -lDataSize, SEEK_CUR);                    // return to start of block
    }
    while (dwDataOffset > 0);

    pRecorder->DetachHighPassFilter();                                 // done, remove filter

    // ascend out of data chunk
    if (mmioAscend(m_hmmioFile, &m_mmckinfoSubchunk, 0))
    {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szFileName);
        m_bFileReady = FALSE;
        return;
    }
}

/***************************************************************************/
// CDlgAutoRecorder::SetRecorderMode Set the recorder mode
/***************************************************************************/
void CDlgAutoRecorder::SetRecorderMode(eRecordMode eMode)
{
    if ((m_eMode == eMode) && (m_eMode != Stop))
    {
        return;    // no change
    }

    SetTotalTime();
    SetPositionTime();

    m_eOldMode = m_eMode;
    switch (eMode)
    {
    case Record:
        DeleteTempFile();
        // reset the file pointer
        m_bFileReady = CreateTempFile(); // create new temporary mmio file
        m_dwRecordSize = 0;
        m_bFileApplied = FALSE;

        if (!m_bFileReady)
        {
            break;
        }

        // enable/disable the buttons for recording
        m_eMode = Record;
        // start or continue recording
        if (m_pWave)
        {
            if (!m_pWave->Record(m_hmmioFile, m_pView, m_dwRecordSize, &m_NotifyObj))   // record
            {
                SetRecorderMode(Stop);  // record not successfull
            }
        }
        break;
    case Monitor:
        StopWave();
        m_eMode = Monitor;
        m_VUBar.SetVU(0);
        // start monitoring again
        if (!m_pWave->Monitor(m_pView, &m_NotifyObj))   // monitor
        {
            SetRecorderMode(Stop);  // record not successfull
        }
        break;

    case Stop:
        StopWave();
        m_eMode = Stop;
        SetPositionTime();
        m_VUBar.SetVU(0);
        break;

    case Play:
        StopWave();
        m_eMode = Play;
        // play back the recorded file
        if (!m_pWave->Play( 0, m_pDoc->GetDataSize(), m_nVolume, 100, m_pView, &m_NotifyObj))
        {
            SetRecorderMode(Record);            // start recording now
            ChangeState(WaitingForVoice);
        }
        break;

    default:
        StopWave();
        m_eMode = Disabled;
        m_LEDTotalTime.SetTime(100, 1000);
        m_LEDPosTime.SetTime(100, 1000);
        m_VUBar.SetVU(0);
        break;
    }

    // display pending error messages
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    pApp->DisplayMessages();
}

/***************************************************************************/
// CDlgAutoRecorder::CreateTempFile Create the temporary wave file
// Creates the temporary wave file, opens it and creates the RIFF chunk and
// the Data subchunk. The file stays open with the file pointer ready
// to write.
/***************************************************************************/
BOOL CDlgAutoRecorder::CreateTempFile()
{
    ASSERT(m_hmmioFile == NULL);

    // create the temporary file
    TCHAR lpszTempPath[_MAX_PATH];
    GetTempPath(_MAX_PATH, lpszTempPath);
    GetTempFileName(lpszTempPath, _T("WAV"), 0, m_szFileName);

    // create and open the file
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    m_hmmioFile = mmioOpen(m_szFileName, NULL, MMIO_CREATE | MMIO_READWRITE | MMIO_EXCLUSIVE);
    if (!(m_hmmioFile))
    {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_FILEOPEN, m_szFileName);
        return FALSE;
    }
    // create a 'RIFF' chunk with a 'WAVE' form type
    m_mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
    // set chunk size
    m_mmckinfoParent.cksize = 0;
    if (mmioCreateChunk(m_hmmioFile, &m_mmckinfoParent, MMIO_CREATERIFF))   // create the 'RIFF' chunk
    {
        // error creating RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, m_szFileName);
        return FALSE;
    }
    // create the 'fmt ' subchunk
    m_mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    // set chunk size
    m_mmckinfoSubchunk.cksize = 16;
    if (mmioCreateChunk(m_hmmioFile, &m_mmckinfoSubchunk, 0))   // create the 'data' chunk
    {
        // error creating format chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szFileName);
        return FALSE;
    }
    // write data into 'fmt ' chunk
    if (mmioWrite(m_hmmioFile, (HPSTR)m_szFileName, 16) == -1)   // fill up fmt chunk
    {
        // error writing format chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szFileName);
        return FALSE;
    }
    // get out of 'fmt ' chunk
    if (mmioAscend(m_hmmioFile, &m_mmckinfoSubchunk, 0))
    {
        // error writing format chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szFileName);
        return FALSE;
    }
    // create the 'data' subchunk
    m_mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    // set chunk size
    m_mmckinfoSubchunk.cksize = 0;
    if (mmioCreateChunk(m_hmmioFile, &m_mmckinfoSubchunk, 0))   // create the 'data' chunk
    {
        // error creating data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szFileName);
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************/
// CDlgAutoRecorder::DeleteTempFile Close and delete the temporary wave file
/***************************************************************************/
void CDlgAutoRecorder::DeleteTempFile()
{
    if (m_szFileName[0] != 0)
    {
        // close and delete the temporary wave file
        if (m_hmmioFile)
        {
            mmioClose(m_hmmioFile, 0);
            m_hmmioFile = NULL;
        }
        RemoveFile(m_szFileName);
        wmemset(m_szFileName,0,_countof(m_szFileName));
    }
}

/***************************************************************************/
// CDlgAutoRecorder::CleanUp Clean up memory and delete the temporary file
/***************************************************************************/
void CDlgAutoRecorder::CleanUp()
{
    if (m_pWave)
    {
        m_pWave->Stop();
        delete m_pWave; // delete the CWave object
        m_pWave = NULL;
    }
    DeleteTempFile();
    if (m_hData)
    {
        ::GlobalUnlock(m_hData);
        ::GlobalFree(m_hData);
        m_hData = 0;

    }
}

/***************************************************************************/
// CDlgAutoRecorder::Apply Apply the wave file to a document
// This function closes the wavefile properly (writes the format parameters
// into the format chunk). Then it sets up the sa parameters and then tells
// the document the current filename (with path) of the temporary file and
// the document will prompt the user to save it. After that the recorder will
// be closed.
/***************************************************************************/
BOOL CDlgAutoRecorder::Apply()
{

    ASSERT(m_hmmioFile);

    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    // set file pointer to end of file (also end of 'data' chunk)
    mmioSeek(m_hmmioFile, 0, SEEK_END);

    // get out of 'data' chunk
    if (mmioAscend(m_hmmioFile, &m_mmckinfoSubchunk, 0))
    {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, m_szFileName);
        return FALSE;
    }

    m_dwRecordSize = m_mmckinfoSubchunk.cksize; // get recorded data size
    // get out of 'RIFF' chunk, to write RIFF size
    if (mmioAscend(m_hmmioFile, &m_mmckinfoParent, 0))
    {
        // error writing RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, m_szFileName);
        return FALSE;
    }
    // now rewrite the correct format parameters
    mmioSeek(m_hmmioFile, 0, SEEK_SET); // set file pointer to begin of file
    // descend into 'RIFF' chunk again
    if (mmioDescend(m_hmmioFile, &m_mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
        // error descending into RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, m_szFileName);
        return FALSE;
    }
    // descend into 'fmt ' chunk
    MMCKINFO mmckinfoSubchunk;
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(m_hmmioFile, &mmckinfoSubchunk, &m_mmckinfoParent, MMIO_FINDCHUNK))
    {
        // error descending into format chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szFileName);
        return FALSE;
    }
    // write the format parameters into 'fmt ' chunk
    // get pointer to format parameters
    CFmtParm fmtParm;
    m_pDoc->GetFmtParm(fmtParm,false);

    long lError = mmioWrite(m_hmmioFile, (HPSTR)&fmtParm.wTag, sizeof(WORD));
    if (lError != -1)
    {
        lError = mmioWrite(m_hmmioFile, (HPSTR)&fmtParm.wChannels, sizeof(WORD));
    }
    if (lError != -1)
    {
        lError = mmioWrite(m_hmmioFile, (HPSTR)&fmtParm.dwSamplesPerSec, sizeof(DWORD));
    }
    if (lError != -1)
    {
        lError = mmioWrite(m_hmmioFile, (HPSTR)&fmtParm.dwAvgBytesPerSec, sizeof(DWORD));
    }
    if (lError != -1)
    {
        lError = mmioWrite(m_hmmioFile, (HPSTR)&fmtParm.wBlockAlign, sizeof(WORD));
    }
    if (lError != -1)
    {
        lError = mmioWrite(m_hmmioFile, (HPSTR)&fmtParm.wBitsPerSample, sizeof(WORD));
    }
    if (lError == -1)
    {
        // error writing format chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEFORMATCHUNK, m_szFileName);
        return FALSE;
    }

    // file is handed off to document
    mmioClose(m_hmmioFile, 0); // close file
    m_hmmioFile = NULL;


    {
        // get sa parameters
        SaParm saParm;
        m_pDoc->GetSaParm(&saParm);
        // set the sa parameters
        saParm.RecordTimeStamp = CTime::GetCurrentTime();
        saParm.dwRecordBandWidth = fmtParm.dwSamplesPerSec / 2;
        if (saParm.wFlags & SA_FLAG_HIGHPASS)
        {
            saParm.dwRecordBandWidth -= 70;
        }
        saParm.byRecordSmpSize = (BYTE)fmtParm.wBitsPerSample;
        saParm.dwNumberOfSamples = m_dwRecordSize / fmtParm.wBlockAlign;
        saParm.dwSignalBandWidth = saParm.dwRecordBandWidth;
        saParm.byQuantization = (BYTE)fmtParm.wBitsPerSample;
        m_pDoc->SetSaParm(&saParm);
        m_pDoc->RestartAllProcesses();
        m_pDoc->InvalidateAllProcesses();

        // tell the document to apply the file
        m_pDoc->ApplyWaveFile(m_szFileName, m_dwRecordSize, m_AlignInfo);
        m_pDoc->InvalidateAllProcesses();

    }

    m_bFileApplied = TRUE;
    m_szFileName[0] = 0;

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAutoRecorder message handlers

/***************************************************************************/
// CDlgAutoRecorder::OnInitDialog Dialog initialization
// All the necessary special control windows have to be created and placed
// over their placeholders in the dialog. The dialog is centered over the
// mainframe window.
/***************************************************************************/
BOOL CDlgAutoRecorder::OnInitDialog()
{
    ASSERT(m_pDoc);
    ASSERT(m_pView);

    // load and update for title bar
    GetWindowText(m_szTitle);
    ChangeState(WaitForSilence);

    CDialog::OnInitDialog();
    m_pWave->GetOutDevice()->ConnectMixer(this);
    m_pWave->GetInDevice()->ConnectMixer(this);

    m_LEDTotalTime.Init(IDC_TOTALTIME, this);
    m_LEDTotalTime.SetTime(100, 1000);
    // build and place the position time LED window
    m_LEDPosTime.Init(IDC_POSITIONTIME, this);
    m_LEDPosTime.SetTime(0, 0);
    // build and place the VUbar window
    m_VUBar.Init(IDC_VUBAR, this);
    m_VUBar.SetVU(0);

    // build and place the volume slider windows
    m_SliderVolume.Init(IDC_VOLUMESLIDER, this);
    m_SliderRecVolume.Init(IDC_RECVOLUMESLIDER, this);
    m_SpinVolume.Init(IDC_VOLUMESCROLL, this);
    m_SpinRecVolume.Init(IDC_RECVOLUMESCROLL, this);

    OnMixerControlChange(0,0);

    SetRecorderMode(Monitor);       // initialize the buttons

    return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgAutoRecorder::OnStop Button stop hit
/***************************************************************************/
void CDlgAutoRecorder::OnStop()
{
    m_bStopPending = true;
    if (m_eState == Idle || m_eState == Playing)
    {
        StartShutdown();
    }
    else
    {
        SetRecorderMode(Stop);
        ChangeState(Idle);
        StartShutdown();
    }
}

/***************************************************************************/
// CDlgAutoRecorder::OnClose Close the dialog
/***************************************************************************/
void CDlgAutoRecorder::OnClose()
{
    OnStop();
}

/***************************************************************************/
// CDlgAutoRecorder::OnCancel ESC key hit (there is no cancel key.)
// When the ESC key is hit, this does the same as if the CLOSE button was hit.
// (If OnButtonClose is not called here, a General Protection Fault results.)
/***************************************************************************/
void CDlgAutoRecorder::OnCancel()
{
    OnStop();
    CDialog::OnCancel();
}

/***************************************************************************/
// CDlgAutoRecorder::OnHelpAutoRecorder Call Auto Recorder help
/***************************************************************************/
void CDlgAutoRecorder::OnHelpAutoRecorder()
{
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Graphs/Record_overlay.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

LRESULT CDlgAutoRecorder::OnAutoRestart(WPARAM, LPARAM)
{
    ASSERT(m_eState == Idle);
    ASSERT(m_pDoc);
    ASSERT(m_pView);

    // load and update for title bar
    ChangeState(WaitForSilence);

    m_LEDTotalTime.SetTime(100, 1000);
    // build and place the position time LED window
    m_LEDPosTime.SetTime(0, 0);
    // build and place the VUbar window
    m_VUBar.SetVU(0);

    SetRecorderMode(Monitor);       // initialize the buttons

    return 0;
}

void CDlgAutoRecorder::ChangeState(eRecordState eState)
{
    if (m_hWnd)
    {
        CString str;
        switch (eState)
        {
        case WaitForSilence:
            str.LoadString(IDS_AUTO_WAITING_SILENCE);
            break;
        case WaitingForVoice:
            str.LoadString(IDS_AUTO_WAITING);
            break;
        case Recording:
            str.LoadString(IDS_AUTO_RECORD);
            break;
        case Stopping:
            str.LoadString((m_bStopPending)?IDS_AUTO_STOPPING:IDS_AUTO_PROCESSING);
            break;
        case Playing:
            break;
        case Idle:
            break;
        default:
            ASSERT(FALSE);
            break;

        }
        CString szTitle;
        szTitle = m_szTitle;
        if (!str.IsEmpty())
        {
            szTitle += " - ";
            szTitle += str;
        }
        SetWindowText(szTitle);
    }
    m_eState = eState;
    m_dwTickCount = GetTickCount();
}

void CDlgAutoRecorder::StartShutdown()
{
    // tell the host the user requested a stop, cancel or esc
    // the host will destroy this window and destroy the document
    TRACE(_T("\nRecorder stopping recorder\n\n"));

    // destroy the recorder..
    CSaView * pTarget = GetTarget();

    // go to the target view and destroy it's rt overlay plots
    TRACE(_T("Remove Rt Plots\n"));
    if (pTarget)
    {
        pTarget->RemoveRtPlots();
    }

    //Destroy the overlay document
    CDocList docList;
    CSaDoc * pDoc = docList.pdocFirst();
    while (pDoc)
    {
        if (pDoc->IsTempOverlay())
        {
            POSITION pos = pDoc->GetFirstViewPosition();
            CView * pView = pDoc->GetNextView(pos);
            if (pView)
            {
                TRACE(_T("Found overlay document %lp\n"),pDoc);
                pView->PostMessage(WM_COMMAND,ID_FILE_CLOSE,0);
                break;
            }
        }
        pDoc = docList.pdocNext();
    }

    EndDialog(IDOK);
}

void CDlgAutoRecorder::StopWave()
{
    if (m_pWave)
    {
        m_pWave->Stop();
    }
}

CSaView * CDlgAutoRecorder::GetTarget()
{
    CDocList docList;
    CSaDoc * pDoc = docList.pdocFirst();
    while (pDoc)
    {
        if (m_pTargetUntested)
        {
            POSITION pos = pDoc->GetFirstViewPosition();
            if (pos)
            {
                CView * pView = (CView *)pDoc->GetNextView(pos);
                if (pView == m_pTargetUntested)
                {
                    return m_pTargetUntested;
                }
            }
        }

        pDoc = docList.pdocNext();
    }

    if (m_pTargetUntested)
    {
        // target view destroyed close dialog
        m_pTargetUntested = NULL;
        StartShutdown();
    }

    return m_pTargetUntested;
}

/***************************************************************************/
// CDlgAutoRecorder::OnVolumeSlide Volume slider position changed
/***************************************************************************/
void CDlgAutoRecorder::OnVolumeSlide()
{
    m_nVolume = m_SliderVolume.GetPosition();
    SetDlgItemInt(IDC_VOLUMEEDIT, m_SliderVolume.GetPosition(), TRUE);
    m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgAutoRecorder::OnVolumeScroll Volume spin control hit
/***************************************************************************/
void CDlgAutoRecorder::OnVolumeScroll()
{
    m_nVolume = GetDlgItemInt(IDC_VOLUMEEDIT, NULL, TRUE);
    if (m_SpinVolume.UpperButtonClicked())
    {
        m_nVolume++;
    }
    else
    {
        m_nVolume--;
    }
    if ((int)m_nVolume < 0)
    {
        m_nVolume = 0;
    }
    if (m_nVolume > 100)
    {
        m_nVolume = 100;
    }
    SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
    m_SliderVolume.SetPosition(m_nVolume);
    m_pWave->SetVolume(m_nVolume);
}


/***************************************************************************/
// CDlgAutoRecorder::OnKillfocusVolumeEdit Volume edited
/***************************************************************************/
void CDlgAutoRecorder::OnKillfocusVolumeEdit()
{
    m_nVolume = GetDlgItemInt(IDC_VOLUMEEDIT, NULL, TRUE);
    if ((int)m_nVolume < 0)
    {
        m_nVolume = 0;
    }
    if (m_nVolume > 100)
    {
        m_nVolume = 100;
    }
    SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
    m_SliderVolume.SetPosition(m_nVolume);
    m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgAutoRecorder::OnRecVolumeSlide Volume slider position changed
/***************************************************************************/
void CDlgAutoRecorder::OnRecVolumeSlide()
{
    m_nRecVolume = m_SliderRecVolume.GetPosition();
    SetRecVolume(m_nRecVolume);
}

/***************************************************************************/
// CDlgAutoRecorder::OnRecVolumeScroll Volume spin control hit
/***************************************************************************/
void CDlgAutoRecorder::OnRecVolumeScroll()
{
    m_nRecVolume = GetDlgItemInt(IDC_RECVOLUMEEDIT, NULL, TRUE);
    if (m_SpinRecVolume.UpperButtonClicked())
    {
        m_nRecVolume++;
    }
    else
    {
        m_nRecVolume--;
    }
    if ((int)m_nRecVolume < 0)
    {
        m_nRecVolume = 0;
    }
    if (m_nRecVolume > 100)
    {
        m_nRecVolume = 100;
    }
    SetRecVolume(m_nRecVolume);
}


/***************************************************************************/
// CDlgAutoRecorder::OnKillfocusVolumeEdit Volume edited
/***************************************************************************/
void CDlgAutoRecorder::OnKillfocusRecVolumeEdit()
{
    m_nRecVolume = GetDlgItemInt(IDC_RECVOLUMEEDIT, NULL, TRUE);
    if ((int)m_nRecVolume < 0)
    {
        m_nRecVolume = 0;
    }
    if (m_nRecVolume > 100)
    {
        m_nRecVolume = 100;
    }
    SetRecVolume(m_nRecVolume);
}

void CDlgAutoRecorder::SetRecVolume(int nVolume)
{
    BOOL bResult;
    m_pWave->GetInDevice()->SetVolume(nVolume, &bResult);
    if (!bResult)
    {
        EnableRecVolume(FALSE);
        nVolume = 0;
    }
    SetDlgItemInt(IDC_RECVOLUMEEDIT, nVolume, TRUE);
    m_SliderRecVolume.SetPosition(nVolume);
}

void CDlgAutoRecorder::EnableRecVolume(BOOL bEnable)
{
    CWnd * pWnd = GetDlgItem(IDC_RECVOLUMEEDIT);
    if (pWnd)
    {
        pWnd->EnableWindow(bEnable);
    }
    m_SliderRecVolume.EnableWindow(bEnable);
}

UINT CDlgAutoRecorder::GetRecVolume()
{

    UINT nVolume;
    BOOL result = (m_pWave->GetInDevice()->GetVolume(nVolume)==MMSYSERR_NOERROR);
    EnableRecVolume(result);
    return nVolume;
}

/***************************************************************************/
// CDlgAutoRecorder::OnMixerControlChange Mixer has changed volume settings
/***************************************************************************/
LRESULT CDlgAutoRecorder::OnMixerControlChange(WPARAM, LPARAM)
{

    BOOL bResult = FALSE;
    m_nVolume = m_pWave->GetVolume(bResult);
    if (bResult)
    {
        SetDlgItemInt(IDC_VOLUMEEDIT, m_nVolume, TRUE);
        m_SliderVolume.SetPosition(m_nVolume);
    }

    m_nRecVolume = GetRecVolume();
    SetDlgItemInt(IDC_RECVOLUMEEDIT, m_nRecVolume, TRUE);
    m_SliderRecVolume.SetPosition(m_nRecVolume);

    return 0;
}

/******************************************************************************
******************************************************************************/
BOOL CDlgAutoRecorder::OnAssignOverlay(CSaView * pSourceView)
{
    // if the focused graph is still mergeable - apply the overlay
    CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
    CSaView * pView = pFrame->GetCurrSaView();

    UINT graphIDs[MAX_GRAPHS_NUMBER];
    for (int i=0; i<MAX_GRAPHS_NUMBER; i++)
    {
        CGraphWnd * pGraph = pView->GetGraph(i);

        graphIDs[i] = 0;

        if (pGraph)
        {
            graphIDs[i] = pGraph->GetPlotID();
            if (graphIDs[i] == ID_GRAPHS_OVERLAY)
            {
                CMultiPlotWnd * pPlot = (CMultiPlotWnd *)pGraph->GetPlot();
                graphIDs[i] = pPlot->GetBasePlotID();
            }
        }
        else
        {
            graphIDs[i] = 0;
        }
    }
    pSourceView->OnGraphsTypesPostProcess(graphIDs);

    BOOL bFound = FALSE;
    for (int i=0; i<MAX_GRAPHS_NUMBER; i++)
    {
        CGraphWnd * pGraph = pView->GetGraph(i);
        if (pGraph && CGraphWnd::IsMergeableGraph(pGraph))
        {
            bFound |= pView->AssignOverlay(pGraph,pSourceView);
        }
    }

    pView->RefreshGraphs();
    return bFound;
}




void CDlgAutoRecorder::OnPlay()
{
    if (m_eMode != Play)
    {
        SetRecorderMode(Play);
        ChangeState(Playing);
    }
}
