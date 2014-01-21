#include "stdafx.h"
#include "DlgRecorder.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "fileInformation.h"
#include "sa_view.h"
#include "sa.h"
#include "mainfrm.h"
#include "FileUtils.h"
#include "WaveOutDevice.h"
#include "WaveInDevice.h"
#include "DlgFnKeys.h"
#include "DlgRecorderOptions.h"
#include "DlgPlayer.h"
#include "Process\sa_p_fra.h"
#include "Process\Process.h"
#include "objectostream.h"

//###########################################################################
// CDlgRecorder dialog
// Displays the recorder that allows the user to control the wave data
// recording process, done by the CWave class.

BEGIN_MESSAGE_MAP(CDlgRecorder, CDialog)
    ON_COMMAND(IDC_RECORD, OnRecord)
    ON_COMMAND(IDC_PLAY, OnPlay)
    ON_COMMAND(IDC_STOP, OnStop)
    ON_COMMAND(IDC_PAUSE, OnPause)
    ON_WM_CLOSE()
    ON_BN_CLICKED(ID_CLOSE, OnButtonClose)
    ON_BN_CLICKED(IDC_SETTINGS, OnSettings)
    ON_BN_CLICKED(IDC_APPLY, OnApply)
    ON_COMMAND(IDC_VOLUMESLIDER, OnVolumeSlide)
    ON_COMMAND(IDC_VOLUMESCROLL, OnVolumeScroll)
    ON_EN_KILLFOCUS(IDC_VOLUMEEDIT, OnKillfocusVolumeEdit)
    ON_COMMAND(IDC_RECVOLUMESLIDER, OnRecVolumeSlide)
    ON_COMMAND(IDC_RECVOLUMESCROLL, OnRecVolumeScroll)
    ON_EN_KILLFOCUS(IDC_RECVOLUMEEDIT, OnKillfocusRecVolumeEdit)
    ON_MESSAGE(MM_MIXM_CONTROL_CHANGE, OnMixerControlChange)
    ON_BN_CLICKED(IDC_MIXER, OnMixer)
    ON_COMMAND(IDHELP, OnHelpRecorder)
END_MESSAGE_MAP()

/***************************************************************************/
// CDlgRecorder::CDlgRecorder Constructor
/***************************************************************************/
CDlgRecorder::CDlgRecorder(CWnd * pParent) : CDialog(CDlgRecorder::IDD, pParent)
{
    m_hmmioFile = NULL;
    m_szFileName[0] = 0;        // no file name
    m_pDoc = NULL;
    m_pView = NULL;
    m_NotifyObj.Attach(this);   // attach notify object
    m_pWave = new CWave();      // create CWave object
    BOOL bResult;
    m_nVolume = m_pWave->GetVolume(bResult);
    m_pWave->GetInDevice()->GetVolume(m_nRecVolume);
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
    m_lpPlayData = (HPSTR)::GlobalLock(m_hData); // lock memory
    if (m_lpPlayData==NULL)
    {
        // memory lock error
        pApp->ErrorMessage(IDS_ERROR_MEMLOCK);
        ::GlobalFree(m_hData);
        return;
    }
}

/***************************************************************************/
// CDlgRecorder::DoDataExchange Data exchange
/***************************************************************************/
void CDlgRecorder::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_VOLUMEEDIT, m_nVolume);
    DDX_Text(pDX, IDC_RECVOLUMEEDIT, m_nRecVolume);
}

/***************************************************************************/
// CDlgRecorder::SetTotalTime Set total time display
/***************************************************************************/
void CDlgRecorder::SetTotalTime()
{
    CSaDoc * pDoc = (CSaDoc *)m_pDoc; // cast pointer to document
    double fDataSec = pDoc->GetTimeFromBytes(m_dwRecordSize); // calculate time
    m_LEDTotalTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
}

/***************************************************************************/
// CDlgRecorder::SetPositionTime Set position time display
/***************************************************************************/
void CDlgRecorder::SetPositionTime()
{
    CSaDoc * pDoc = (CSaDoc *)m_pDoc; // cast pointer to document
    if ((m_nMode == RECORDING) || ((m_nMode == PAUSED) && (m_nOldMode == RECORDING)))
    {
        double fDataSec = pDoc->GetTimeFromBytes(m_dwRecordSize); // calculate time
        m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
    }
    else
    {
        double fDataSec = pDoc->GetTimeFromBytes(m_dwPlayPosition); // calculate time
        m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
    }
}

/***************************************************************************/
// CDlgRecorder::SetSettingsText Set settings text
/***************************************************************************/
void CDlgRecorder::SetSettingsText()
{
    CSaDoc * pDoc = (CSaDoc *)m_pDoc; // cast pointer to document

    TCHAR szBuffer[60];
    swprintf_s(szBuffer, _T("%u Hz"), pDoc->GetSamplesPerSec());
	if (pDoc->GetNumChannels()==1)
	{
        swprintf_s(szBuffer, _T("%s, %u-bit\nMono"), szBuffer, pDoc->GetBitsPerSample());
	}
	else if (pDoc->GetNumChannels()==2)
	{
        swprintf_s(szBuffer, _T("%s, %u-bit\nStereo"), szBuffer, pDoc->GetBitsPerSample());
	}
    else
    {
		swprintf_s(szBuffer, _T("%s, %u-bit\n%d Channels"), szBuffer, pDoc->GetBitsPerSample(), pDoc->GetNumChannels());
    }

	if (pDoc->IsUsingHighPassFilter())
    {
        swprintf_s(szBuffer, _T("%s, Highpass"), szBuffer);
    }
    if (GetStaticSourceInfo().bEnable)
    {
        swprintf_s(szBuffer, _T("%s\nAdd file information"), szBuffer);
    }
    SetDlgItemText(IDC_SETTINGSTEXT, szBuffer);
}

/***************************************************************************/
// CDlgRecorder::BlockStored Recording Block finished
// The actually recording data block has been finished. Update the dialog
// display controls.
/***************************************************************************/
void CDlgRecorder::BlockStored(UINT nLevel, DWORD dwPosition, BOOL *)
{
    // update the VU bar
    m_VUBar.SetVU((int)nLevel);
    // save the position
    if (m_nMode == RECORDING)
    {
        m_dwRecordSize = dwPosition;
        // update the time
        CSaDoc * pDoc = (CSaDoc *)m_pDoc;
        double fDataSec = pDoc->GetTimeFromBytes(m_dwRecordSize); // get sampled data size in seconds
        m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
    }
}

/***************************************************************************/
// CDlgRecorder::StoreFailed Recorded block has been failed storing
/***************************************************************************/
void CDlgRecorder::StoreFailed()
{
    // stop the recorder
    SetRecorderMode( STOPPED);
    // inform the user
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    pApp->ErrorMessage(IDS_ERROR_STOREWAVEBLOCK, m_szFileName);
}

/***************************************************************************/
// CDlgRecorder::BlockFinished Playing Block finished
// The actually playing data block has been finished playing. Update the
// dialog display controls.
/***************************************************************************/
void CDlgRecorder::BlockFinished(UINT nLevel, DWORD dwPosition, UINT)
{
    m_dwPlayPosition = dwPosition;
    // update the VU bar
    m_VUBar.SetVU((int)nLevel);
    // update the time
    CSaDoc * pDoc = (CSaDoc *)m_pDoc;
    double fDataSec = pDoc->GetTimeFromBytes(dwPosition); // get sampled data size in seconds
    m_LEDPosTime.SetTime((int)fDataSec / 60, (int)(fDataSec * 10) % 600);
}

/***************************************************************************/
// CDlgRecorder::EndPlayback Playback finished
// The playback has been finished. Set recorder mode to stop.
/***************************************************************************/
void CDlgRecorder::EndPlayback()
{
    if (m_nMode != IDLE)
    {
        SetRecorderMode( STOPPED);
    }
}

/***************************************************************************/
// CDlgRecorder::GetWaveData Request for next wave data block to play
// This function delivers the next wave data block to play to the CWave
// object from the recorder internal (just recorded) wave file. The file is
// always open, the file pointer is already in the data subchunk and the
// function just delivers the block requested.
/***************************************************************************/
HPSTR CDlgRecorder::GetWaveData(DWORD dwPlayPosition, DWORD dwDataSize)
{
    // find the right position in the data
    if (mmioSeek(m_hmmioFile, dwPlayPosition + 40, SEEK_SET) == -1)
    {
        return NULL;
    }
    // read the waveform data block
    if (mmioRead(m_hmmioFile, (HPSTR)m_lpPlayData, dwDataSize) == -1)
    {
        return NULL;
    }
    return m_lpPlayData;
}

/***************************************************************************/
// CWave::HighPassFilter Filter recorded wave file
// If the highpass flag is set, the sample data in the wave file is
// filtered, eliminating frequency components below 70 Hz.  Assumes file
// is already open and positioned at the end of the data chunk.
/***************************************************************************/
void CDlgRecorder::HighPassFilter()
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
        if (mmioRead(m_hmmioFile, m_lpPlayData, lDataSize) == -1)           // load it
        {
            // error
            pApp->ErrorMessage(IDS_ERROR_READDATACHUNK);
            m_bFileReady = FALSE;
            return;
        }

        if (wSmpSize == 1)
        {
            // 8-bit unsigned
            pHighPassFilter->BackwardPass((unsigned char *)m_lpPlayData, (unsigned long)lDataSize);    // filter backwards
            //!!ck. return code?
            //UINT nMaxLevel = max(abs(pHighPassFilter->GetMax()-128), abs(pHighPassFilter->GetMin()-128));  // get max level for block
            //m_pWave->SetMaxLevel(100*(long)nMaxLevel/128);                                            // set max level
        }
        else
        {
            // 16-bit signed
            pHighPassFilter->BackwardPass((short *)m_lpPlayData, (unsigned long)lDataSize/wSmpSize);   // filter backwards
            //!!ck. return code?
            //UINT nMaxLevel = max(abs(pHighPassFilter->GetMax()), abs(pHighPassFilter->GetMin()));     // get max level for block
            //m_pWave->SetMaxLevel(100*(long)nMaxLevel/32768);                                          // set max level
        }

        mmioSeek(m_hmmioFile, -lDataSize, SEEK_CUR);                    // return to start of block
        if (mmioWrite(m_hmmioFile, m_lpPlayData, lDataSize) == -1)          // write filtered data
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
// CDlgRecorder::SetRecorderMode Set the recorder mode
/***************************************************************************/
void CDlgRecorder::SetRecorderMode( EMode mode)
{
    if ((m_nMode == mode) && (m_nMode != STOPPED))
    {
        return;    // no change
    }
    TRACE("SetRecorderMode %d\n",mode);

    // stop, whatever the recorder is doing
    if (m_pWave)
    {
        m_pWave->Stop();
    }
    SetTotalTime();
    SetPositionTime();
    CSaDoc * pDoc = (CSaDoc *)m_pDoc; // cast pointer to document
    
	switch (mode)
    {
    case RECORDING:
        GetDlgItem(IDC_APPLY)->EnableWindow(FALSE); // disable apply button
        // reset the file pointer
        if (m_nMode == STOPPED)
        {
            DeleteTempFile(); // delete old temporary mmio file
            m_bFileReady = CreateTempFile(); // create new temporary mmio file
            m_dwRecordSize = 0;
            if (!m_bFileReady)
            {
                break;
            }
        }
        // stop monitoring
        if ((m_nMode == STOPPED) || (m_nMode == PAUSED))
        {
            if (m_pWave!=NULL)
            {
                m_pWave->Stop();
            }
        }
        // enable/disable the buttons for recording
        m_nMode = RECORDING;
        m_record.Flash(FALSE);		// stop flashing Record button
        m_play.Release();			// release Play button
        m_pause.Release();			// release Pause button
        m_play.EnableWindow(FALSE); // disable Play button
        m_pause.EnableWindow(TRUE); // enable Pause button
        GetDlgItem(IDC_SETTINGS)->EnableWindow(FALSE); // disable settings button
        // start or continue recording
        if (m_pWave)
        {
            if (!m_pWave->Record(m_hmmioFile, m_pView, m_dwRecordSize, &m_NotifyObj))   // record
            {
                m_nMode = STOPPED;  // record not successfull
                m_record.Release(); // release Record button
                m_pause.EnableWindow(FALSE); // disable Pause button
                GetDlgItem(IDC_SETTINGS)->EnableWindow(TRUE); // enable settings button
            }
        }
        break;

    case PLAYING:
        m_nMode = PLAYING;
        m_play.Flash(FALSE);	// stop flashing Play button
        m_play.Push();			// push Play button
        m_record.Release();		// release Record button
        m_pause.Release();		// release Pause button
        m_record.EnableWindow(FALSE); // disable Record button
        m_pause.EnableWindow(TRUE); // enable Pause button
        GetDlgItem(IDC_SETTINGS)->EnableWindow(FALSE); // disable settings button
        GetDlgItem(IDC_APPLY)->EnableWindow(FALSE); // disable apply button
        // play back the recorded file
        if (!m_pWave->Play(m_dwPlayPosition, m_dwRecordSize - m_dwPlayPosition, m_nVolume, 100, m_pView, &m_NotifyObj))
        {
            m_nMode = STOPPED;  // play not successfull
            m_play.Release();	// release Play button
            m_pause.EnableWindow(FALSE); // disable Pause button
            GetDlgItem(IDC_SETTINGS)->EnableWindow(TRUE); // enable settings button
        }
        break;

    case PAUSED:
        if (m_pWave)
        {
            m_pWave->Stop();
        }
        m_nOldMode = m_nMode;
        m_nMode = PAUSED;
        // start flashing paused buttons
        if (m_nOldMode == RECORDING)
        {
            m_record.Flash(TRUE);
        }
        else
        {
            m_play.Flash(TRUE);
        }
        // start monitoring again
        if (!m_pWave->Monitor(m_pView, &m_NotifyObj))   // monitor
        {
            m_nMode = STOPPED;	// monitor not successfull
            m_record.Release(); // release Record button
            m_play.Release();	// release Play button
            m_stop.Release();	// release Stop button
            m_pause.Release();	// release Pause button
        }
        m_VUBar.SetVU(0);
        break;

    case STOPPED:
        if ((m_nMode == RECORDING) || (m_nMode == PLAYING) || (m_nMode == PAUSED))
        {
            if (m_pWave)
            {
                m_pWave->Stop();
            }
            if ((m_nMode == RECORDING) || ((m_nMode == PAUSED) && (m_nOldMode == RECORDING)))
				if (pDoc->IsUsingHighPassFilter())
                {
                    HighPassFilter();
                }
        }
        m_nMode = STOPPED;
        m_dwPlayPosition = 0;
        SetPositionTime();
        m_record.Release();		// release Record button
        m_play.Release();		// release Play button
        m_stop.Release();		// release Stop button
        m_pause.Release();		// release Pause button
        m_record.EnableWindow(TRUE);// enable Record button
        m_play.EnableWindow(TRUE);	// enable Play button
        m_pause.EnableWindow(FALSE);// disable Pause button
        GetDlgItem(IDC_SETTINGS)->EnableWindow(TRUE); // enable settings button
        if (m_dwRecordSize > 0)
        {
            m_play.EnableWindow(TRUE); // enable Play button
            GetDlgItem(IDC_APPLY)->EnableWindow(TRUE); // enable apply button
        }
        else
        {
            m_play.EnableWindow(FALSE); // disable Play button
            GetDlgItem(IDC_APPLY)->EnableWindow(FALSE); // disable apply button
        }
        if (!m_pWave->Monitor(m_pView, &m_NotifyObj))   // monitor
        {
            m_nMode = STOPPED;  // monitor not successfull
        }
        m_VUBar.SetVU(0);
        break;

    default:
        m_nMode = IDLE;
        m_record.Release();			// release Record button
        m_play.Release();			// release Play button
        m_stop.Release();			// release Stop button
        m_pause.Release();			// release Pause button
        m_record.EnableWindow(TRUE);// enable Record button
        m_play.EnableWindow(TRUE);	// enable Play button
        m_pause.EnableWindow(FALSE);// disable Pause button
        GetDlgItem(IDC_SETTINGS)->EnableWindow(TRUE); // enable settings button
        GetDlgItem(IDC_APPLY)->EnableWindow(FALSE); // disable apply button
        if (m_pWave)
        {
            m_pWave->Stop();
        }
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
// CDlgRecorder::CreateTempFile Create the temporary wave file
// Creates the temporary wave file, opens it and creates the RIFF chunk and
// the Data subchunk. The file stays open with the file pointer ready
// to write.
/***************************************************************************/
BOOL CDlgRecorder::CreateTempFile()
{
    // create the temporary file
    GetTempFileName(_T("WAV"), m_szFileName, _countof(m_szFileName));
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
// CDlgRecorder::DeleteTempFile Close and delete the temporary wave file
/***************************************************************************/
void CDlgRecorder::DeleteTempFile()
{
    if (m_szFileName[0] != 0)
    {
        // close and delete the temporary wave file
        if (m_hmmioFile)
        {
            mmioClose(m_hmmioFile, 0);
        }
        try
        {
            RemoveFile(m_szFileName);
            m_szFileName[0] = 0;
        }
        catch (CFileException e)
        {
            // error deleting file
            CSaApp * pApp = (CSaApp *)AfxGetApp();
            pApp->ErrorMessage(IDS_ERROR_DELTEMPFILE, m_szFileName);
        }
    }
}

/***************************************************************************/
// CDlgRecorder::CleanUp Clean up memory and delete the temporary file
// If the user allows closing, the function returns TRUE, else FALSE.
/***************************************************************************/
void CDlgRecorder::CleanUp()
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
    }
}

/***************************************************************************/
// CDlgRecorder::CloseRecorder Close the recorder
// If the user allows closing, the function returns TRUE, else FALSE.
/***************************************************************************/
BOOL CDlgRecorder::CloseRecorder()
{
    if (m_nMode != STOPPED)
    {
        SetRecorderMode(STOPPED);    // stop recording
    }
    m_VUBar.SetVU(0); // reset the VU bar
    if ((m_bFileReady && !m_bFileApplied) && (m_dwRecordSize > 0))
    {
        // temporary data not applied, ask user
        int nResponse = AfxMessageBox(IDS_QUESTION_APPLYRECORD, MB_YESNOCANCEL | MB_ICONQUESTION, 0);
        if (nResponse == IDYES)
        {
            // apply the data
            OnApply();
            return FALSE;
        }
        if (nResponse == IDCANCEL)
        {
            // continue with recorder
            SetRecorderMode(STOPPED);
            return FALSE;
        }
    }
    CleanUp();
    if (m_bFileApplied)
    {
        EndDialog(IDOK);
    }
    else
    {
        EndDialog(IDCANCEL);
    }
    return TRUE;
}

/***************************************************************************/
// CDlgRecorder::Apply Apply the wave file to a document
// This function closes the wavefile properly (writes the format parameters
// into the format chunk). Then it sets up the sa parameters and then tells
// the document the current filename (with path) of the temporary file and
// the document will prompt the user to save it. After that the recorder will
// be closed.
/***************************************************************************/
BOOL CDlgRecorder::Apply(CDocument * pDocument)
{

    CSaDoc * pDoc = (CSaDoc *)pDocument; // cast document pointer
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
    CFmtParm fmtParm;
    pDoc->GetFmtParm(fmtParm,false);

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
    mmioClose(m_hmmioFile, 0); // close file
    
    // set the sa parameters
	pDoc->SetRecordTimeStamp(CTime::GetCurrentTime());
	pDoc->SetRecordSampleSize((BYTE)fmtParm.wBitsPerSample);
	pDoc->SetNumberOfSamples(m_dwRecordSize / fmtParm.wBlockAlign);
	pDoc->SetRecordBandWidth(fmtParm.dwSamplesPerSec / 2);
	pDoc->SetSignalBandWidth(fmtParm.dwSamplesPerSec / 2);
	pDoc->SetQuantization((BYTE)fmtParm.wBitsPerSample);

    // tell the document to apply the file
    pDoc->ApplyWaveFile(m_szFileName, m_dwRecordSize);
    // if player is visible, disable the speed slider until required processing is completed
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    CDlgPlayer * pPlayer = pMain->GetPlayer();
    if ((pPlayer!=NULL) && (pPlayer->IsWindowVisible()))
    {
        pPlayer->EnableSpeedSlider(FALSE);
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgRecorder message handlers

/***************************************************************************/
// CDlgRecorder::OnInitDialog Dialog initialization
// All the necessary special control windows have to be created and placed
// over their placeholders in the dialog. The dialog is centered over the
// mainframe window.
/***************************************************************************/
BOOL CDlgRecorder::OnInitDialog()
{

    CDialog::OnInitDialog();
    m_pWave->GetOutDevice()->ConnectMixer(this);
    m_pWave->GetInDevice()->ConnectMixer(this);
    m_nMode = IDLE;

    // get pointer to view and document
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    m_pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
    m_pDoc = (CSaDoc *)m_pView->GetDocument();

    // change the font for the text controls
    CWnd * pWnd = GetDlgItem(IDC_SETTINGSTITLE);
    CFont * pFont = pWnd->GetFont(); // get the standard font
    LOGFONT logFont;
    pFont->GetObject(sizeof(LOGFONT), (void *)&logFont); // fill up logFont
    // modify the logFont
    logFont.lfWeight = FW_NORMAL;                       // not bold
    m_Font.CreateFontIndirect(&logFont);                // create the modified font
    pWnd->SetFont(&m_Font);                             // set the modified font
    GetDlgItem(IDC_SETTINGSTEXT)->SetFont(&m_Font);
    // set file name in recorders caption
    CString szTitle = m_pDoc->GetFilenameFromTitle().c_str(); // load file name
    CString szCaption;
    GetWindowText(szCaption);
    szCaption += " - " + szTitle;
    SetWindowText(szCaption);
    CRect rWnd;
    // build and place the record toggle button
    m_record.Init(IDC_RECORD, "Rec", this);
    // build and place the play toggle button
    m_play.Init(IDC_PLAY, "Play", this);
    m_play.EnableWindow(FALSE); // disable Play button
    // build and place the stop toggle button
    m_stop.Init(IDC_STOP, "Stop", this);
    // build and place the pause toggle button
    m_pause.Init(IDC_PAUSE, "Pause", this);
    // build and place the total time LED window
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
    // build and place the volume spin controls
    m_SpinVolume.Init(IDC_VOLUMESCROLL, this);
    m_SpinRecVolume.Init(IDC_RECVOLUMESCROLL, this);

    pWnd = GetDlgItem(IDC_MIXER);
    if (pWnd)
    {
        pWnd->EnableWindow(m_pWave->GetInDevice()->ShowMixer(FALSE));
    }

    OnMixerControlChange(0,0);

    CenterWindow(); // center dialog on frame window
    // set the settings text
    SetSettingsText();
    OnStop(); // put recorder into stop mode

    return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgRecorder::OnRecord Button record hit
/***************************************************************************/
void CDlgRecorder::OnRecord()
{
    if ((m_nMode != RECORDING) && (m_nMode != PAUSED))
    {
        SetRecorderMode(RECORDING);
    }
}

/***************************************************************************/
// CDlgRecorder::OnPlay Button play hit
/***************************************************************************/
void CDlgRecorder::OnPlay()
{
    if ((m_nMode != PLAYING) && (m_nMode != PAUSED))
    {
        SetRecorderMode(PLAYING);
    }
}

/***************************************************************************/
// CDlgRecorder::OnStop Button stop hit
/***************************************************************************/
void CDlgRecorder::OnStop()
{
    if (m_nMode != IDC_STOP)
    {
        SetRecorderMode(STOPPED);
    }
    else
    {
        m_stop.Release();
    }
}

/***************************************************************************/
// CDlgRecorder::OnPause Button pause hit
/***************************************************************************/
void CDlgRecorder::OnPause()
{
    if (m_nMode != PAUSED)
    {
        SetRecorderMode(PAUSED);
    }
    else
    {
        SetRecorderMode(m_nOldMode);
    }
}

/***************************************************************************/
// CDlgRecorder::OnMixerControlChange Mixer has changed volume settings
/***************************************************************************/
LRESULT CDlgRecorder::OnMixerControlChange(WPARAM, LPARAM)
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

/***************************************************************************/
// CDlgRecorder::OnVolumeSlide Volume slider position changed
/***************************************************************************/
void CDlgRecorder::OnVolumeSlide()
{
    m_nVolume = m_SliderVolume.GetPosition();
    SetDlgItemInt(IDC_VOLUMEEDIT, m_SliderVolume.GetPosition(), TRUE);
    m_pWave->SetVolume(m_nVolume);
}

/***************************************************************************/
// CDlgRecorder::OnVolumeScroll Volume spin control hit
/***************************************************************************/
void CDlgRecorder::OnVolumeScroll()
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
// CDlgRecorder::OnKillfocusVolumeEdit Volume edited
/***************************************************************************/
void CDlgRecorder::OnKillfocusVolumeEdit()
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
// CDlgRecorder::OnRecVolumeSlide Volume slider position changed
/***************************************************************************/
void CDlgRecorder::OnRecVolumeSlide()
{
    m_nRecVolume = m_SliderRecVolume.GetPosition();
    SetRecVolume(m_nRecVolume);
}

/***************************************************************************/
// CDlgRecorder::OnRecVolumeScroll Volume spin control hit
/***************************************************************************/
void CDlgRecorder::OnRecVolumeScroll()
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
// CDlgRecorder::OnKillfocusVolumeEdit Volume edited
/***************************************************************************/
void CDlgRecorder::OnKillfocusRecVolumeEdit()
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

void CDlgRecorder::OnMixer()
{
    m_pWave->GetInDevice()->ShowMixer();
}

void CDlgRecorder::SetRecVolume(int nVolume)
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

void CDlgRecorder::EnableRecVolume(BOOL bEnable)
{
    CWnd * pWnd = GetDlgItem(IDC_RECVOLUMEEDIT);
    if (pWnd)
    {
        pWnd->EnableWindow(bEnable);
    }

    m_SliderRecVolume.EnableWindow(bEnable);
}

UINT CDlgRecorder::GetRecVolume()
{
    UINT nVolume;
    BOOL result = (m_pWave->GetInDevice()->GetVolume(nVolume)==MMSYSERR_NOERROR);
    EnableRecVolume(result);
    return nVolume;
}
/***************************************************************************/
// CDlgRecorder::OnClose Close the dialog
/***************************************************************************/
void CDlgRecorder::OnClose()
{
    CloseRecorder();
}

/***************************************************************************/
// CDlgRecorder::OnButtonClose Button close hit
/***************************************************************************/
void CDlgRecorder::OnButtonClose()
{
    SendMessage(WM_CLOSE);
}

/***************************************************************************/
// CDlgRecorder::OnCancel ESC key hit (there is no cancel key.)
// When the ESC key is hit, this does the same as if the CLOSE button was hit.
// (If OnButtonClose is not called here, a General Protection Fault results.)
/***************************************************************************/
void CDlgRecorder::OnCancel()
{
    CDialog::OnCancel();
    OnButtonClose();
}

/***************************************************************************/
// CDlgRecorder::OnSettings Button settings hit
/***************************************************************************/
void CDlgRecorder::OnSettings()
{
    SetRecorderMode( STOPPED);
    CSaDoc * pDoc = (CSaDoc *)m_pDoc; // cast document pointer

    // get format parameters
    CFmtParm fmtParm;
    pDoc->GetFmtParm(fmtParm,false);

    // create the dialog
    CDlgRecorderOptions dlg(this);

    // init the dialog
    dlg.SetSamplingRate(fmtParm.dwSamplesPerSec);
    dlg.SetBitDepth(fmtParm.wBitsPerSample);
    dlg.SetChannels(fmtParm.wChannels);
	dlg.SetHighpass(pDoc->IsUsingHighPassFilter() ? TRUE : FALSE);

    if (dlg.DoModal() == IDOK)
    {
        if ((m_bFileReady && m_dwRecordSize) &&
            ((dlg.GetSamplingRate() != (fmtParm.dwSamplesPerSec)) ||
             (dlg.GetBitDepth() != fmtParm.wBitsPerSample) ||
             (dlg.GetChannels() != fmtParm.wChannels) ||
             (dlg.GetHighpass() != ((pDoc->IsUsingHighPassFilter()) ? TRUE : FALSE))))
        {
            // ask user to delete recorded file before changing the settings
            int nResponse = AfxMessageBox(IDS_QUESTION_DELRECORD, MB_YESNO | MB_ICONQUESTION, 0);
            if (nResponse == IDYES)
            {
                m_play.EnableWindow(FALSE); // disable Play button
                GetDlgItem(IDC_APPLY)->EnableWindow(FALSE); // disable apply button
                DeleteTempFile(); // delete the temporary mmio file
                m_bFileReady = CreateTempFile(); // create new temporary mmio file
                m_dwRecordSize = 0;
            }
            else
            {
                // continue with recorder
                return;
            }
        }

        fmtParm.dwSamplesPerSec = dlg.GetSamplingRate();
        fmtParm.wBitsPerSample = dlg.GetBitDepth();
        fmtParm.wChannels = dlg.GetChannels();
        fmtParm.wBlockAlign = WORD(fmtParm.wBitsPerSample / 8);
        fmtParm.dwAvgBytesPerSec = fmtParm.dwSamplesPerSec * fmtParm.wBlockAlign;
        if (dlg.GetHighpass())
        {
			pDoc->SetHighPassFilter();
        }
        else
        {
            CWaveInDevice * pRecorder = m_pWave->GetInDevice();
            if (pRecorder->GetHighPassFilter())
            {
                pRecorder->DetachHighPassFilter();
            }
			pDoc->ClearHighPassFilter();
        }
        // set format parameters
        pDoc->SetFmtParm(&fmtParm);

        if (!GetStaticSourceInfo().bEnable)
        {
            CSaDoc * pDoc = (CSaDoc *)m_pDoc; // cast document pointer
            SourceParm * pSourceParm = pDoc->GetSourceParm();

            pSourceParm->szCountry.Empty();
            pSourceParm->szDialect.Empty();
            pSourceParm->szEthnoID = "???";
            pSourceParm->szFamily.Empty();
            pSourceParm->szLanguage.Empty();
            pSourceParm->szRegion.Empty();
            pSourceParm->szTranscriber.Empty();
            pSourceParm->szSpeaker.Empty();
            pSourceParm->szReference.Empty();
            pSourceParm->szFreeTranslation.Empty();
            pSourceParm->szDescription.Empty();
            pSourceParm->nGender = -1;
        }
        SetSettingsText();
    }
    SetRecorderMode( STOPPED);
}

/***************************************************************************/
// CDlgRecorder::OnApply Button apply hit
// This function calls the function to apply the wave file to a document.
// After that, the recorder will be closed.
/***************************************************************************/
void CDlgRecorder::OnApply()
{
    if (m_pWave)
    {
        m_pWave->Stop();    // stop recording
    }
    m_VUBar.SetVU(0); // reset the VU bar
    if (m_bFileReady && !m_bFileApplied)   // file is ready but not yet applied
    {
        if (!Apply(m_pDoc))
        {
            m_bFileReady = FALSE;
            CloseRecorder();
            return;
        }
        m_bFileApplied = TRUE;
    }
    ClearFileName(); // file has been overtaken from document
    CloseRecorder(); // close the recorder
    sourceInfo & m_source = GetStaticSourceInfo();
    if (m_source.bEnable)
    {
        CSaDoc * pDoc = (CSaDoc *)m_pDoc; // cast document pointer
        SourceParm * pSourceParm = pDoc->GetSourceParm();

        pSourceParm->szCountry = m_source.source.szCountry;
        pSourceParm->szDialect = m_source.source.szDialect;
        pSourceParm->szEthnoID = m_source.source.szEthnoID.Left(3);
        pSourceParm->szFamily = m_source.source.szFamily;
        pSourceParm->szLanguage = m_source.source.szLanguage;
        pSourceParm->nGender = m_source.source.nGender;
        pSourceParm->szRegion = m_source.source.szRegion;
        pSourceParm->szSpeaker = m_source.source.szSpeaker;
        pSourceParm->szReference = m_source.source.szReference;
        pSourceParm->szTranscriber = m_source.source.szTranscriber;
        pDoc->SetDescription(m_source.source.szDescription);
        pSourceParm->szFreeTranslation = m_source.source.szFreeTranslation;
    }
    return;
}

/***************************************************************************/
// CDlgRecorder::OnHelpRecorder Call Export Table help
/***************************************************************************/
void CDlgRecorder::OnHelpRecorder()
{
    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/File/Record_New.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

/***************************************************************************/
// CDlgRecorder::GetDocument()
/***************************************************************************/
CSaDoc * CDlgRecorder::GetDocument()
{
    return m_pDoc;
}

/***************************************************************************/
// CDlgRecorder::GetStaticSourceInfo()
/***************************************************************************/
CDlgRecorder::sourceInfo & CDlgRecorder::GetStaticSourceInfo()
{
    static sourceInfo m_source;

    return m_source;
};

static LPCSTR psz_sourceInfo = "sourceInfo";
static LPCSTR psz_enable = "enable";
static LPCSTR psz_country = "country";
static LPCSTR psz_dialect = "dialect";
static LPCSTR psz_ethnoID = "ethnoID";
static LPCSTR psz_family = "family";
static LPCSTR psz_language = "language";
static LPCSTR psz_gender = "gender";
static LPCSTR psz_region = "region";
static LPCSTR psz_speaker = "speaker";
static LPCSTR psz_reference = "reference";
static LPCSTR psz_transcriber = "transcriber";
static LPCSTR psz_freeTranslation = "freeTranslation";
static LPCSTR psz_fileDescription = "fileDescription";

void CDlgRecorder::sourceInfo::WriteProperties(CObjectOStream & obs)
{
    obs.WriteBeginMarker(psz_sourceInfo);

    sourceInfo & m_source = GetStaticSourceInfo();

    // write out properties
    obs.WriteInteger(psz_enable, m_source.bEnable);
    obs.WriteString(psz_country, m_source.source.szCountry.utf8().c_str());
    obs.WriteString(psz_dialect, m_source.source.szDialect.utf8().c_str());
    obs.WriteString(psz_ethnoID, m_source.source.szEthnoID.utf8().c_str());
    obs.WriteString(psz_family, m_source.source.szFamily.utf8().c_str());
    obs.WriteString(psz_language, m_source.source.szLanguage.utf8().c_str());
    CSaString szGender;
    szGender.Format(_T("%d"), m_source.source.nGender);
    obs.WriteString(psz_gender, szGender.utf8().c_str());
    obs.WriteString(psz_region, m_source.source.szRegion.utf8().c_str());
    obs.WriteString(psz_speaker, m_source.source.szSpeaker.utf8().c_str());
    obs.WriteString(psz_reference, m_source.source.szReference.utf8().c_str());
    obs.WriteString(psz_transcriber, m_source.source.szTranscriber.utf8().c_str());
    obs.WriteString(psz_fileDescription, m_source.source.szDescription.utf8().c_str());
    obs.WriteString(psz_freeTranslation, m_source.source.szFreeTranslation.utf8().c_str());

    obs.WriteEndMarker(psz_sourceInfo);
}

BOOL CDlgRecorder::sourceInfo::ReadProperties(CObjectIStream & obs)
{

    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_sourceInfo))
    {
        return FALSE;
    }

    sourceInfo & m_source = GetStaticSourceInfo();

    while (!obs.bAtEnd())
    {
        CSaString szGender;
        szGender.Format(_T("%d"), m_source.source.nGender);
        if (obs.bReadInteger(psz_enable, m_source.bEnable));
        else if (ReadStreamString(obs,psz_country,m_source.source.szCountry));
        else if (ReadStreamString(obs,psz_dialect,m_source.source.szDialect));
        else if (ReadStreamString(obs,psz_ethnoID,m_source.source.szEthnoID));
        else if (ReadStreamString(obs,psz_family,m_source.source.szFamily));
        else if (ReadStreamString(obs,psz_language,m_source.source.szLanguage));
        else if (ReadStreamString(obs,psz_gender,szGender))
        {
            m_source.source.nGender = _ttoi(szGender.GetBuffer(2));
        }
        else if (ReadStreamString(obs,psz_region,m_source.source.szRegion));
        else if (ReadStreamString(obs,psz_speaker,m_source.source.szSpeaker));
        else if (ReadStreamString(obs,psz_reference,m_source.source.szReference));
        else if (ReadStreamString(obs,psz_transcriber,m_source.source.szTranscriber));
        else if (ReadStreamString(obs,psz_fileDescription,m_source.source.szDescription));
        else if (ReadStreamString(obs,psz_freeTranslation,m_source.source.szFreeTranslation));
        else if (obs.bEnd(psz_sourceInfo))
        {
            break;
        }
    }
    return TRUE;
}

CDlgRecorder::~CDlgRecorder()
{
    if (m_pWave)
    {
        delete m_pWave;
    }
}

HMMIO CDlgRecorder::GetFileHandle()
{
    return m_hmmioFile;   // return handle to wave file
}

void CDlgRecorder::ClearFileName()
{
    m_szFileName[0] = 0;
};
