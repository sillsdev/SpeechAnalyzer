/////////////////////////////////////////////////////////////////////////////
// sa_wave.cpp:
// Implementation of the CWave
//                       CWaveInDevice
//                       CWaveOutDevice classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
//
// Revision History
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_wave.h"
#include "Process\sa_proc.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_dur.h"
#include "Process\sa_p_fra.h"

#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "mainfrm.h"
#include "mixer.h"

#ifdef PLAY_DUMP
#include <stdio.h>
static FILE *hPlayData;
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


//###########################################################################
// CWave
// This class plays and records wave files in the background. For playback it
// uses two buffers and while one buffer is playing in background it processes
// the data for the other buffer.

/////////////////////////////////////////////////////////////////////////////
// CWave construction/destruction/creation

/***************************************************************************/
// CWave::CWave Constructor
/***************************************************************************/
CWave::CWave()
{
  m_bRecording = false;
  m_bBackgroundEnabled = FALSE;
  m_pNotifyObj = NULL;
  m_pView = NULL;
  m_pFmtParm = NULL;
  m_pWaveWarp = NULL;
  m_nActiveBlock = 0;
  m_nNextBlock = 0;
  m_nMaxLevel = 0;
  m_hmmioFile = NULL;
  for(int i=0;i<m_kBuffers;i++)
  {
    m_lpData[i] = NULL;
    m_dwBufferSize[i] = 0;
    m_dwProcessedSize[i] = 0;
  }
  // create device objects
  m_pOutDev = new CWaveOutDevice;
  m_pInDev = new CWaveInDevice;
}

/***************************************************************************/
// CWave::~CWave Destructor
/***************************************************************************/
CWave::~CWave()
{
  for(int i = 0; i < m_kBuffers; i++)
  {
    if (m_lpData[i])
      delete [] m_lpData[i];
  }
  // delete device objects
  if (m_pOutDev) delete m_pOutDev;
  if (m_pInDev) delete m_pInDev;
}

/////////////////////////////////////////////////////////////////////////////
// CWave helper functions

/***************************************************************************/
// CWave::AllocateBuffers Allocate wave buffers
// Allocates the wave buffers for playback and recording. In case of an
// error the function returns FALSE, otherwise TRUE.
/***************************************************************************/
BOOL CWave::AllocateBuffers()
{
  CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application
  for(int i = 0; i < m_kBuffers; i++)
  {
    if (!m_lpData[i])
      m_lpData[i] = new char[MMIO_BUFFER_SIZE * (DWORD)10];

    if (!m_lpData[i])
    {
      // memory lock error
      pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
      return FALSE;
    }
  }
  return TRUE;
}

/***************************************************************************/
// CWave::AdjustToParameters Adjust size to current parameters
// Adjusts the wave buffer size to a size according to the current sampling
// rate and the sample size.
/***************************************************************************/
DWORD CWave::AdjustToParameters()
{
  DWORD dwFactor = 44100 / m_pFmtParm->dwSamplesPerSec;
  if (m_pFmtParm->wBlockAlign == 1) dwFactor *= 2;
  if (dwFactor > 4) dwFactor = 4;
  if (dwFactor < 1) dwFactor = 1;
  return MMIO_BUFFER_SIZE / dwFactor;
}

/***************************************************************************/
// CWave::ProcessData Processing wave data
// Here the wave data ready to play will be processed before playing, for
// instance to play it reverse or faster or slower.
/***************************************************************************/
BOOL CWave::ProcessData(int nBuffer)
{

  // Set playback buffer pointer and buffer size.
  HPSTR pTarget = GetBufferPointer(nBuffer); // pointer to target buffer
  DWORD dwBufferSize = AdjustToParameters();
  UINT wSmpSize = m_pFmtParm->wBlockAlign/m_pFmtParm->wChannels;
  int nMaxValue = 0, nMinValue = 0;

  // If wavewarp object not constructed, simply copy the sample data to the play buffer
  if (!m_pWaveWarp)
  {
    DWORD dwDataSize = min(m_dwEnd - m_dwPlayPosition, dwBufferSize);
    if (dwDataSize)
    {
      HPSTR pData = m_pNotifyObj->GetWaveData(m_pView, m_dwPlayPosition, dwDataSize);
      if(!pData) return FALSE;
      CopyBuffer(pData, pTarget, dwDataSize, (wSmpSize == 1), &nMaxValue, &nMinValue);
      m_dwPlayPosition += dwDataSize;
      if (m_dwPlayPosition >= m_dwEnd) m_bProcessDone = TRUE;
    }

    SetBufferSize(nBuffer, dwDataSize, dwDataSize);      // set member variables for playback
  }

  //  Otherwise, pass sample data to warping object
  else
  {
    if (m_pWaveWarp->SetPlayBuffer((void *)pTarget, dwBufferSize/wSmpSize) != DONE)
      return FALSE;

    // Load playback buffer with waveform fragments from start to stop cursors.
    DWORD dwDataSize=0, dwPlayLength = 0;
    DWORD dwProcess=0;
    dspError_t Status;
    do
    {
      if (m_stCallData.dwOffset * wSmpSize >= m_dwEnd)
      {
        Status = DONE;
        break;
      }
      dwProcess = m_stCallData.dwOffset * wSmpSize;  // convert to byte offset
      DWORD dwProcessEnd = max(dwProcess + m_stCallData.wLength*wSmpSize, m_dwEnd);  // get endpoint of last fragment
      dwDataSize = min(dwProcessEnd - dwProcess, dwBufferSize);  // resize block to buffer size or to end of last fragment
//      dwDataSize = min(dwDataSize, dwBufferSize/wSmpSize*m_nSpeed/100*wSmpSize);  // resize block to buffer size or to end of last fragment
      HPSTR pData = m_pNotifyObj->GetWaveData(m_pView, dwProcess, dwDataSize);   // retrieve sample data
      if (m_pWaveWarp->SetWaveBuffer((void *)pData) != DONE) return FALSE;   // pass waveform buffer pointer
      Status = m_pWaveWarp->FillPlayBuffer(m_stCallData.dwOffset, dwDataSize/wSmpSize,
        &m_stCallData, &dwPlayLength);
    }while(Status == OUTSIDE_WAVE_BUFFER);

    if (Status < DONE)
    {
      CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application
      pApp->ErrorMessage(IDS_ERROR_WAVEWARP, _T("playback")); // send error message
      return FALSE;
    }
    else if (Status == DONE)
    {
      m_stCallData.dwOffset = dwProcess  / wSmpSize;
      m_stCallData.wLength = (WORD)(dwDataSize / wSmpSize);
      m_bProcessDone = TRUE;
    }

    // Set playback member variable to update timer
    // use play length to avoid snapping to fragment boundaries
    m_dwPlayPosition += (dwPlayLength*m_nSpeed+50)/100*wSmpSize; 

    // Use requested fragment to limit position (avoid runout errors)
    if(m_dwPlayPosition < m_stCallData.dwOffset * wSmpSize)
      m_dwPlayPosition = m_stCallData.dwOffset * wSmpSize;
    else if(m_dwPlayPosition > (m_stCallData.dwOffset + m_stCallData.wLength) * wSmpSize)
      m_dwPlayPosition = (m_stCallData.dwOffset + m_stCallData.wLength) * wSmpSize;

    // Don't go beyond end
    if (m_dwPlayPosition >= m_dwEnd)
      m_dwPlayPosition = m_dwEnd;

    // Set playback size
    DWORD dwPlaySize = dwPlayLength * wSmpSize;
    SetBufferSize(nBuffer, dwDataSize, dwPlaySize);

    // Set find min and max values.
    FindMinMax((HPSTR)pTarget, dwPlaySize, (wSmpSize == 1),  &nMaxValue, &nMinValue);
#ifdef PLAY_DUMP
    fwrite(pTarget, wSmpSize, (size_t)dwPlayLength, hPlayData);
#endif
  }

  // Set peak level for VU meter.
  m_nMaxLevel = max((UINT)abs(nMaxValue), (UINT)abs(nMinValue));
  if (wSmpSize == 1) m_nMaxLevel = 100 * m_nMaxLevel / 128;
  else m_nMaxLevel = (UINT)(100 * (long)m_nMaxLevel / 32768);


  return TRUE;
}

/***************************************************************************/
// CWave::CopyBuffer Copy source into target buffer and find min/max values
/***************************************************************************/
void CWave::CopyBuffer(HPSTR pSource, HPSTR pTarget, DWORD dwLength, BOOL bIs8Bit, int* piMax, int* piMin)
{
  int nData;
  BYTE bData;
  do
  {
    // read data
    if (bIs8Bit) // 8 bit per sample
    {
      *pTarget++ = *pSource; // copy data
      bData = *pSource++; // data range is 0...255 (128 is center)
      nData = bData - 128;
    }
    else // 16 bit data
    {
      *pTarget++ = *pSource; // copy data
      nData = *((short int*)pSource++);
      *pTarget++ = *pSource; // copy data
      pSource++;
      dwLength--;
    }
    // process data
    if (nData > *piMax) *piMax = nData; // store higher value
    else
    {
      if (nData < *piMin) *piMin = nData; // store lower value
    }
  } while(--dwLength > 0);
}

/***************************************************************************/
// CWave::FindMinMax Find min/max values
/***************************************************************************/
void CWave::FindMinMax(HPSTR pSource, DWORD dwLength, BOOL bIs8Bit, int* piMax, int* piMin)
{
  int nData=0;
  BYTE bData=0;
  if (dwLength == 0)
  {
    return;
  }
  do
  {
    if (bIs8Bit) // 8 bit per sample
    {
      bData = *pSource++; // data range is 0...255 (128 is center)
      nData = bData - 128;
    }
    else                  // 16 bit data
    {
      nData = *((short int*)pSource++);
      pSource++;
      dwLength--;
    }
    // process data
    if (nData > *piMax) *piMax = nData; // store higher value
    else
    {
      if (nData < *piMin) *piMin = nData; // store lower value
    }
  } while(--dwLength > 0);
}

/***************************************************************************/
// CWave::GetBufferPointer Get the pointer to the actual buffer
/***************************************************************************/
char* CWave::GetBufferPointer(int nBuffer)
{
  if (nBuffer < 0 || nBuffer >= m_kBuffers) 
    return NULL;
  else 
    return m_lpData[nBuffer];
}

/***************************************************************************/
// CWave::GetPlaybackPointer Get pointer to the first sample to play
/***************************************************************************/
char* CWave::GetPlaybackPointer(int nBuffer)
{
  char *pPlayback = GetBufferPointer(nBuffer);
  if (m_pWaveWarp)
  {
    DWORD dwProcess = m_stCallData.dwOffset *
      (m_pFmtParm->wBlockAlign / m_pFmtParm->wChannels);
    if (dwProcess < m_dwStart)
      return (pPlayback + m_dwStart - dwProcess);  // playback to start in middle of fragment
  }
  return pPlayback;
}

/***************************************************************************/
// CWave::GetBufferSize Get the size of the actual buffer
/***************************************************************************/
DWORD CWave::GetBufferSize(int nBuffer)
{
  if (nBuffer < 0 || nBuffer >= m_kBuffers) 
    return 0;
  else 
    return m_dwBufferSize[nBuffer];
}

/***************************************************************************/
// CWave::GetPlaybackSize Get the size to play back
/***************************************************************************/
DWORD CWave::GetPlaybackSize(int nBuffer)
{
  DWORD dwBufferSize = GetBufferSize(nBuffer);
  if (m_pWaveWarp)
  {
    WORD wSmpSize = (WORD)(m_pFmtParm->wBlockAlign / m_pFmtParm->wChannels);
    DWORD dwFragment = m_stCallData.dwOffset * wSmpSize;
    WORD wFragmentSize = (WORD)(m_stCallData.wLength * wSmpSize);

    if (dwFragment < m_dwStart)
      dwBufferSize -= (m_dwStart - dwFragment);  // reduce if playback starts in middle of fragment
    else if (dwFragment + wFragmentSize > m_dwEnd)
      dwBufferSize -= (dwFragment + wFragmentSize - m_dwEnd); // trim last fragment to end
  }
  return dwBufferSize;
}

/***************************************************************************/
// CWave::GetProcessedSize Get the size of the proc. data for the act. buffer
/***************************************************************************/
DWORD CWave::GetProcessedSize(int nBuffer)
{
  if (nBuffer < 0 || nBuffer >= m_kBuffers) 
    return 0;
  else 
    return m_dwProcessedSize[nBuffer];
}

/***************************************************************************/
// CWave::SetBufferSize Set the size of the actual buffer
// The parameter dwProcessedSize sets the size of the raw data block that
// has been used to get the size of the data in the buffer given by the
// parameter dwBufferSize. They are both the same for normal speed recording
// and playback.
/***************************************************************************/
void CWave::SetBufferSize(int nBuffer, DWORD dwProcessedSize, DWORD dwBufferSize)
{
  if (nBuffer < 0 || nBuffer >= m_kBuffers) 
    return;

  m_dwProcessedSize[nBuffer] = dwProcessedSize;
  m_dwBufferSize[nBuffer] = dwBufferSize;
}

/***************************************************************************/
// CWave::Play Play wave data
// The caller delivers the number of the first data sample to play and the
// size of the data block to play. Then pointers to the actual view and to
// the notify object are needed (to get necessary data and to inform the
// player dialog via the notify object). Two buffers are allocated to hold
// the data to play. Returns FALSE in case of error, else TRUE.
/***************************************************************************/
BOOL CWave::Play(DWORD dwStart, DWORD dwSize, UINT nVolume, UINT nSpeed,
                 CView* pView, CWaveNotifyObj* pNotify)
{
  m_pNotifyObj = pNotify; // set pointer to notify object
  m_pView = pView; // set pointer to view
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
  CProcessFragments* pFragmenter = pDoc->GetFragments();
  m_pFmtParm = pDoc->GetFmtParm(); // set pointer to wave format parameters
  m_dwStart = dwStart;   // set start index
  m_dwPlayPosition = dwStart; // set index of first sample to play
  m_dwEnd = dwStart + dwSize; // set index of last sample to play
  m_nActiveBlock = 0; // use block 0 first
  m_nNextBlock = 0; // use block 0 first
  if (!AllocateBuffers()) return FALSE; // allocate data buffers

  // set up playback volume and speed
  SetVolume(nVolume);
  SetSpeed(nSpeed);
  m_bPlayDone = FALSE;
  m_bProcessDone = FALSE;

  m_bBackgroundEnabled = pDoc->IsBackgroundProcessing();
  if (m_bBackgroundEnabled) pDoc->EnableBackgroundProcessing(FALSE);    // disable background processing during playback
  
  if((nSpeed != 100) && !pFragmenter->IsDataReady())
  {
    // finish fragmenting
    short int nResult = LOWORD(pFragmenter->Process(this, (CSaDoc *)pDoc)); // process data
    if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED) 
    {
      pFragmenter->SetDataInvalid();
      m_bBackgroundEnabled = FALSE;
      pFragmenter->RestartProcess();          
      TRACE(_T("fragmenter failed\n"));
    }
  }

  CMainFrame* pMainWnd = (CMainFrame*)AfxGetMainWnd();
  // check if player is playing
  if ((pMainWnd->IsPlayerPlaying() || pMainWnd->IsPlayerTestRun()) && pFragmenter->IsDataReady())
  {
    // create WaveWarp object
    dspError_t Err = CWaveWarp::CreateObject(&m_pWaveWarp, pDoc, dwStart/m_pFmtParm->wBlockAlign, (USHORT)nSpeed, &m_stCallData);
    if (Err)
    {
      CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application
      pApp->ErrorMessage(IDS_ERROR_WAVEWARP, _T("initialization")); // send error message
      return FALSE;
    }
  }
  else     
    TRACE(_T("CWaveWarp object not created: PlayerPlaying %u, PlayerTest %u, FragmentReady %u\n"),pMainWnd->IsPlayerPlaying(), pMainWnd->IsPlayerTestRun(),  pFragmenter->IsDataReady());
  
#ifdef PLAY_DUMP
  hPlayData = fopen("PlayData.bin", "wb");
#endif
  
	//m_bBackgroundEnabled = pDoc->IsBackgroundProcessing();
  //if (m_bBackgroundEnabled) pDoc->EnableBackgroundProcessing(FALSE);    // disable background processing during playback
  
  for(int i = 0; i < m_kPlayBuffers; i++)
  {
    m_dwPosition[m_nNextBlock] = m_dwPlayPosition;
	// fill up buffer with data
    if (!ProcessData(m_nNextBlock)) return FALSE;
    m_nProcessedSpeed[m_nNextBlock] = m_nSpeed;
    m_nProcessedMax[m_nNextBlock] = m_nMaxLevel;
    // play buffer 0
    if (!m_pOutDev->Play(m_nNextBlock, nVolume, this, !m_bProcessDone && (i + 1) < m_kPlayBuffers)) return FALSE;

    m_nNextBlock = ++m_nNextBlock % m_kPlayBuffers;

    if(m_bProcessDone)
      break;
  }
  if (m_pNotifyObj && !m_bPlayDone)
    m_pNotifyObj->BlockFinished(m_nProcessedMax[m_nActiveBlock], m_dwPosition[m_nActiveBlock], m_nProcessedSpeed[m_nActiveBlock]);  // update player dialog
  return TRUE;
}

/***************************************************************************/
// CWave::Monitor Monitor wave data
// The caller delivers pointers to the actual view and to the notify object.
/***************************************************************************/
BOOL CWave::Monitor(CView* pView, CWaveNotifyObj* pNotify)
{
  return Record(NULL, pView, 0, pNotify, false); // start monitoring
}

/***************************************************************************/
// CWave::Record Record wave data
// If the flag bRecord is FALSE, it will only start monitoring the device,
// and not save the data. The caller delivers a handle to the temporary
// mmio file and pointers to the actual view and to the notify object. The
// notify object informs the player dialog. Two buffers are allocated to
// receive the recorded data (they need to be stretched size for the
// possible slowed replay after record). The parameter dwOffset can contain
// an offset, from where the actual record pointer will add up.
/***************************************************************************/
BOOL CWave::Record(HMMIO hmmioFile, CView* pView, DWORD dwOffset,
                   CWaveNotifyObj* pNotify, 
									 bool bRecord /*=true*/)
{
  m_hmmioFile = hmmioFile; // save handle to file object
  m_bRecording = bRecord;
  m_pNotifyObj = pNotify; // set pointer to notify object
  m_pView = pView; // set pointer to view
  CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
  m_pFmtParm = pDoc->GetFmtParm(); // set pointer to wave format parameters
  m_nMaxLevel = 0;
  m_dwRecordPointer = dwOffset;
  m_dwEnd = 0;

  SaParm *pSaParm = pDoc->GetSaParm();
  if (pSaParm->wFlags & SA_FLAG_HIGHPASS)
    if (!m_pInDev->GetHighPassFilter())
      if (!m_pInDev->AttachHighPassFilter((USHORT)m_pFmtParm->dwSamplesPerSec))
      {
        // if can't construct highpass filter, reset the flag
        pSaParm->wFlags &= ~SA_FLAG_HIGHPASS;
        pDoc->SetSaParm(pSaParm);
        CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application
        pApp->ErrorMessage(IDS_ERROR_RECHPFILTER); // send error message
        return FALSE;
      }

  if(!m_pInDev->GetPreparedBuffers())
  {
    m_nActiveBlock = 0; // use block 0 first
    // We are not already recording/monitoring
    if (!AllocateBuffers()) return FALSE; // allocate data buffers

    for(int i = 1; i < m_kRecordBuffers; i++)
    {
      if (!m_pInDev->Record(i - 1, this)) 
        return FALSE; // record into buffer i
    }
    if (!m_pInDev->Record(m_kRecordBuffers - 1, this, TRUE)) 
      return FALSE; // record into buffer 1
  }

  return TRUE;
}

/***************************************************************************/
// CWave::Stop Stop playing or recording wave data
/***************************************************************************/
void CWave::Stop()
{
  if (m_pOutDev) m_pOutDev->Reset();
  if (m_pInDev) m_pInDev->Reset();
  m_pOutDev->Close(); // close sound device
  m_pInDev->Close(); // close sound device
  m_dwPlayPosition = m_dwEnd;
  m_pNotifyObj = NULL; // don't notify
  if (m_pWaveWarp)
  {
    delete m_pWaveWarp;
    m_pWaveWarp = NULL;
  }
  CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
  CSaDoc* pDoc = (CSaDoc*)pMain->GetCurrDoc();
  if (pDoc && m_bBackgroundEnabled) pDoc->EnableBackgroundProcessing(TRUE);
}

/***************************************************************************/
// CWave::NextBlock Next data block needed
// One data block has been played, the next one is needed. At this time,
// the next block already has been started playing and there is now time to
// fill up the finished one with new data and send it to the playing device
// before it finishes playing the actual playing block.
/***************************************************************************/
void CWave::NextBlock()
{
  if (!m_bPlayDone)
  {
    m_nActiveBlock = ++m_nActiveBlock % m_kPlayBuffers;   // switch buffers
    m_bPlayDone = (m_nActiveBlock == m_nNextBlock);

    if (m_pNotifyObj && !m_bPlayDone)
      m_pNotifyObj->BlockFinished(m_nProcessedMax[m_nActiveBlock], m_dwPosition[m_nActiveBlock], m_nProcessedSpeed[m_nActiveBlock]);  // update player dialog

    // fill buffer with data and play
    if (!m_bProcessDone)
    {
      m_dwPosition[m_nNextBlock] = m_dwPlayPosition;
      if (ProcessData(m_nNextBlock))
      {
        m_nProcessedSpeed[m_nNextBlock] = m_nSpeed;
        m_nProcessedMax[m_nNextBlock] = m_nMaxLevel;
        m_pOutDev->Play(m_nNextBlock, GetVolume(), this);   // play it
      }
      else
      {
        // error in processing
        m_pOutDev->Close(); // close sound device
        if (m_pNotifyObj) m_pNotifyObj->EndPlayback(); // inform notify object
        if (m_pWaveWarp)
        {
          delete m_pWaveWarp;
          m_pWaveWarp = NULL;
        }
      }
      m_nNextBlock = ++m_nNextBlock % m_kPlayBuffers;   // switch buffers
    }
  }

  if(m_bPlayDone)
  {
    // shutdown playback device if processing complete
    m_pOutDev->Close(); // close sound device
    if (m_pNotifyObj) m_pNotifyObj->EndPlayback(); // inform notify object
    if (m_pWaveWarp)
    {
      delete m_pWaveWarp;
      m_pWaveWarp = NULL;
    }
    CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
    CSaDoc* pDoc = (CSaDoc*)pMain->GetCurrDoc();
    if (m_bBackgroundEnabled) pDoc->EnableBackgroundProcessing(TRUE);
    
#ifdef PLAY_DUMP
    fclose(hPlayData);
#endif
  }
}

/***************************************************************************/
// CWave::StoreBlock Store recorded data block
// Once data block has been recorded, the data must be stored in the
// temporary file. By this time, the next block has already has started
// recording, and there is now time to store the recorded data. The
// buffer must then be returned to the input device, before it finishes recor-
// ding into the actual block.
/***************************************************************************/
void CWave::StoreBlock()
{
  // find the maximum level in the recorded data
  HPSTR pSource = GetBufferPointer(m_nActiveBlock); // pointer to source buffer
  int nMaxValue = 0;
  int nData, nMinValue = 0;
  BYTE bData;
  UINT wSmpSize = m_pFmtParm->wBlockAlign/m_pFmtParm->wChannels;
  DWORD dwDataSize = GetBufferSize(m_nActiveBlock); // get buffer size
  if (dwDataSize > 0)
  {
    CHighPassFilter70 *pHighPassFilter = m_pInDev->GetHighPassFilter();
    if (pHighPassFilter)
    {
      DWORD dwDataLength = dwDataSize / wSmpSize;
	  if (wSmpSize == 1)  // 8 bits per sample, 0 to 255 range
      {
        pHighPassFilter->ForwardPass((unsigned char *)pSource, dwDataLength);
        nMinValue = pHighPassFilter->GetBlockMin() - 128;
        nMaxValue = pHighPassFilter->GetBlockMax() - 128;
      }
      else // 16 bits per sample, -32768 to 32767 range
      {
        pHighPassFilter->ForwardPass((short *)pSource, dwDataLength);
        nMinValue = pHighPassFilter->GetBlockMin();
        nMaxValue = pHighPassFilter->GetBlockMax();
      }
    }

    else do
    {
      // read data
      if (wSmpSize == 1) // 8 bit per sample
      {
        bData = *pSource++; // data range is 0...255 (128 is center)
        nData = bData - 128;
      }
      else                  // 16 bit data
      {
        nData = *((short int*)pSource++);
        pSource++;
        dwDataSize--;
      }
      // process data
      if (nData > nMaxValue) nMaxValue = nData; // store higher value
      else
      {
        if (nData < nMinValue) nMinValue = nData; // store lower value
      }
    } while(--dwDataSize > 0);
  }
  // set peak level
  if (nMinValue * -1 > nMaxValue) m_nMaxLevel = (UINT)(nMinValue * -1);
  else m_nMaxLevel = (UINT)nMaxValue;
  // norm the level to %
  if (wSmpSize == 1) // 8 bit per sample
    m_nMaxLevel = 100 * m_nMaxLevel / 128;
  else m_nMaxLevel = (UINT)((LONG)100 * (LONG)m_nMaxLevel / 32768);

  // inform notify object
  BOOL bRecord = m_bRecording;
	if (m_pNotifyObj)
  {
    BOOL *pOverride = bRecord ? &bRecord : NULL;
		m_pNotifyObj->BlockStored(m_nMaxLevel, m_dwRecordPointer+GetBufferSize(m_nActiveBlock), pOverride);
  }

  if (bRecord)
  {
    // store recorded data block in temporary file
    if (mmioWrite(m_hmmioFile, GetBufferPointer(m_nActiveBlock), (long)GetBufferSize(m_nActiveBlock)) == -1)
    {
      // error
      if (m_pNotifyObj) m_pNotifyObj->StoreFailed();
    }
    // actualize play pointer
    m_dwRecordPointer += GetBufferSize(m_nActiveBlock);
  }
 
	// inform notify object
	if (m_pNotifyObj)
	{
		m_pInDev->Record(m_nActiveBlock, this); // record again into this buffer
		m_nActiveBlock = ++m_nActiveBlock % m_kRecordBuffers; // next buffer
	}
}

//###########################################################################
// CWaveInDevice input device

/////////////////////////////////////////////////////////////////////////////
// CWaveInDevice message map
// Input device for recording. It uses a notification window to receive no-
// tification messages from the input device.

BEGIN_MESSAGE_MAP(CWaveInDevice, CWnd)
//{{AFX_MSG_MAP(CWaveInDevice)
ON_MESSAGE(MM_WIM_DATA, OnWimData)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaveInDevice construction/destruction/creation

/***************************************************************************/
// CWaveInDevice::CWaveInDevice Constructor
/***************************************************************************/
CWaveInDevice::CWaveInDevice()
{
  m_hInDev = NULL;
  m_hWnd = NULL;
  m_pHighPassFilter = NULL;
  m_pMixer = new CRecMixer;
  m_nBuffers = 0;
}

/***************************************************************************/
// CWaveInDevice::~CWaveInDevice Destructor
/***************************************************************************/
CWaveInDevice::~CWaveInDevice()
{
  if (m_pMixer)
    delete m_pMixer;
  if(m_pHighPassFilter)
    delete m_pHighPassFilter;
}

/***************************************************************************/
// CWaveInDevice::Create Creation
// Creates a child window with the given parameters as notification window.
/***************************************************************************/
BOOL CWaveInDevice::Create()
{
  // create notification window with size 0
  if (!CreateEx(0, AfxRegisterWndClass(0), _T("Wave Wnd"), WS_POPUP, 0, 0, 0, 0, NULL, NULL))
  {
    TRACE(_T("Failed to create wave notification window"));
    return FALSE;
  }
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWaveInDevice helper functions

/***************************************************************************/
// MMERR Error processing
// Public function to process device error results.
/***************************************************************************/
void MMERR(MMRESULT mmr)
{
  CSaApp* pApp = (CSaApp*)AfxGetApp(); // get pointer to application
  switch (mmr)
  {
    case WAVERR_BADFORMAT:
      pApp->ErrorMessage(IDS_ERROR_MMIO_BADFORMAT);
      break;
    case MMSYSERR_NOMEM:
      pApp->ErrorMessage(IDS_ERROR_MMIO_NOMEMORY);
      break;
    case MMSYSERR_ALLOCATED:
      pApp->ErrorMessage(IDS_ERROR_MMIO_ALRALLOC);
      break;
    case MMSYSERR_BADDEVICEID:
			if (::GetWindowsVersion()<6)
				pApp->ErrorMessage(IDS_ERROR_MMIO_BADID);
			else
				pApp->ErrorMessage(IDS_ERROR_MMIO_BADID_VISTA);
      break;
    case MMSYSERR_INVALHANDLE:
      pApp->ErrorMessage(IDS_ERROR_MMIO_INVHANDLE);
      break;
    case WAVERR_UNPREPARED:
      pApp->ErrorMessage(IDS_ERROR_MMIO_NOHEADER);
      break;
    case MMSYSERR_INVALPARAM:
      pApp->ErrorMessage(IDS_ERROR_MMIO_INVALIDPARAM);
      break;
    default:
      pApp->ErrorMessage(IDS_ERROR_MMIO_UNKNOWN);
      TRACE(_T("Unknown MMSYSERR 0x%X\n"), mmr);
      break;
  }
}

/***************************************************************************/
// CWaveInDevice::Open Open the input device
// The device receives a handle to the notification window.
/***************************************************************************/
BOOL CWaveInDevice::Open(WAVEFORMATEX* pFormat)
{
  MMRESULT mmr;
  // make sure we have a callback window
  if (!m_hWnd)
  {
    Create(); // create the callback window
    ASSERT(m_hWnd);
  }
  // see if input device already open for this format
  if (IsOpen())
  {
#ifdef _DEBUG
    // see if it can handle this format
    if (CanDoFormat(pFormat)) return TRUE; // OK
    else // already open, but for a different format
    {
      // error opening device
      CSaApp* pApp = (CSaApp*)AfxGetApp();
      pApp->ErrorMessage(IDS_ERROR_MMIO_OPENFORMAT);
      return FALSE;
    }
#else
    // We opened this file using this format we don't need to check it
    return TRUE;
#endif
  }
  mmr = waveInOpen(&m_hInDev, WAVE_MAPPER, pFormat, (UINT)GetSafeHwnd(), 0, CALLBACK_WINDOW);
  if (mmr != 0)
  {
    MMERR(mmr); // display error message
    return FALSE;
  }
  return TRUE;
}

/***************************************************************************/
// CWaveInDevice::CanDoFormat Check if the device could open for this format
/***************************************************************************/
BOOL CWaveInDevice::CanDoFormat(WAVEFORMATEX* pFormat)
{
  MMRESULT mmr;
  // device already opened?
  if (!IsOpen())
  {
    TRACE(_T("Not open"));
    return FALSE;
  }
  mmr = waveInOpen(NULL, WAVE_MAPPER, pFormat, NULL, 0, WAVE_FORMAT_QUERY);
  if (mmr != 0)
  {
    MMERR(mmr); // display error message
    return FALSE;
  }
  return TRUE;
}

/***************************************************************************/
// CWaveInDevice::Close Close the input device
/***************************************************************************/
BOOL CWaveInDevice::Close()
{
  if (m_hInDev)
  {
    // close the device
    waveInReset(m_hInDev);
    MSG msg;
    while (::PeekMessage(&msg, NULL, MM_WIM_DATA, MM_WIM_DATA, PM_REMOVE))
    {
      // There are two headers in the queue which are released by waveInReset
      // They must be freed before we destroy the object
      // Since the OnWimData starts new block we need to only call WaveInDone
      WAVEHDR *pHdr = (WAVEHDR *)msg.lParam; // get pointer to wave header
      CWave *pWave = (CWave *)(pHdr->dwUser); // get pointer to CWave object
      WaveInDone(pWave, pHdr); // wave block done
      waveInReset(m_hInDev);
    }
    MMRESULT mmr = waveInClose(m_hInDev);
    if (mmr != 0) MMERR(mmr); // display error message
    m_hInDev = NULL;
  }
  // destroy the window
  if (m_hWnd) DestroyWindow();
  ASSERT(m_hWnd == NULL);
  return TRUE;
}

/***************************************************************************/
// CWaveInDevice::AttachHighPassFilter  Construct a highpass filter
/***************************************************************************/
BOOL CWaveInDevice::AttachHighPassFilter(USHORT wSmpRate)
{
  ASSERT(m_pHighPassFilter == NULL);
	return(CHighPassFilter70::CreateObject(&m_pHighPassFilter, wSmpRate) == DONE);
}

/***************************************************************************/
// CWaveInDevice::Record Start recording
/***************************************************************************/
BOOL CWaveInDevice::Record(int nBuffer, CWave *pWave, BOOL bStart)
{
  // device has to be opened
  if (!Open(pWave->GetFormat())) return FALSE;
  // allocate a header
  WAVEHDR* pHdr = (WAVEHDR*) new WAVEHDR;
  ASSERT(pHdr);
  // fill out the wave header
  memset(pHdr, 0, sizeof(WAVEHDR));
  pHdr->lpData = (char*) pWave->GetBufferPointer(nBuffer); // pointer to the data buffer
  // initialize buffer size
  pWave->SetBufferSize(nBuffer, pWave->AdjustToParameters(), pWave->AdjustToParameters());
  pHdr->dwBufferLength = pWave->GetBufferSize(nBuffer); // size of the data buffer
  pHdr->dwUser = (DWORD)(void*)pWave;    // so we can find the object
  
	// prepare the header
  MMRESULT mmr = waveInPrepareHeader(m_hInDev, pHdr, sizeof(WAVEHDR));
  if (mmr)
  {
    MMERR(mmr); // display error message
    return FALSE;
  }
  m_nBuffers++;
  
	// send it to the driver
  mmr = waveInAddBuffer(m_hInDev, pHdr, sizeof(WAVEHDR));
  if (mmr)
  {
    MMERR(mmr); // display error message
    return FALSE;
  }
  
  if(bStart)
  {
    // start the recording
    mmr = waveInStart(m_hInDev);
    if (mmr)
    {
      MMERR(mmr);
      return FALSE;
    }
  }

  return TRUE;
}

/***************************************************************************/
// CWaveInDevice::GetVolume Get the playback volume
/***************************************************************************/
UINT CWaveInDevice::GetVolume(BOOL *pResult)
{
  DWORD dwVolume = 0xbfff; // Value in case of error
  BOOL bWasOpen = IsOpen();
  MMRESULT mmr = 0;
  if (!bWasOpen)
  {
    // device has to be opened first
    CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
    CView* pView = pMDIFrameWnd->GetCurrSaView();
    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    FmtParm* pFmtParm = pDoc->GetFmtParm(); // get pointer to wave format parameters
    mmr = waveInOpen (&m_hInDev , WAVE_MAPPER, (WAVEFORMATEX*)pFmtParm, NULL, 0, CALLBACK_NULL);
  }
  if(!mmr) mmr = m_pMixer->GetVolume(m_hInDev, &dwVolume);

  if (!bWasOpen)
  {
    // close it again
    if (m_hInDev)
    {
      waveInReset(m_hInDev);
      mmr = waveInClose(m_hInDev);
      m_hInDev = NULL;
    }
  }
  if (pResult)
    *pResult = (mmr == MMSYSERR_NOERROR);

  return (UINT)(((float)(LOWORD(dwVolume)) * (float)100 / (float)0x0FFFF) + 0.5);
}

/***************************************************************************/
// CWaveInDevice::SetVolume Set the playback volume
/***************************************************************************/
void CWaveInDevice::SetVolume(UINT nVolume, BOOL *pResult)
{
  DWORD dwVolume= DWORD(((float)0x0FFFF * (float)nVolume / (float)100) + 0.5);
  BOOL bWasOpen = IsOpen();
  MMRESULT mmr = 0;
  if (!bWasOpen)
  {
    // device has to be opened first

    CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
    CView* pView = pMDIFrameWnd->GetCurrSaView();

    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    FmtParm* pFmtParm = pDoc->GetFmtParm(); // get pointer to wave format parameters
    mmr = waveInOpen (&m_hInDev , WAVE_MAPPER, (WAVEFORMATEX*)pFmtParm, NULL, 0, CALLBACK_NULL);
  }

  if(!mmr) mmr = m_pMixer->SetVolume(m_hInDev, dwVolume);

  if (!bWasOpen)
  {
    // close it again
    if (m_hInDev)
    {
      waveInReset(m_hInDev);
      mmr = waveInClose(m_hInDev);
      m_hInDev = NULL;
    }
  }
  if (pResult)
    *pResult = (mmr == MMSYSERR_NOERROR);
}


/***************************************************************************/
// CWaveOutDevice::ShowMixer
/***************************************************************************/
BOOL CWaveInDevice::ShowMixer(BOOL bShow)
{
  BOOL bResult = FALSE;
  BOOL bWasOpen = IsOpen();
  MMRESULT mmr = 0;
  if (!bWasOpen)
  {
    // device has to be opened first
    CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
    ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CView* pView = pMDIFrameWnd->GetCurrSaView();
    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    FmtParm* pFmtParm = pDoc->GetFmtParm(); // get pointer to wave format parameters
    mmr = waveInOpen (&m_hInDev , WAVE_MAPPER, (WAVEFORMATEX*)pFmtParm, NULL, 0, CALLBACK_NULL);
  }

  if(bShow && !mmr) bResult = m_pMixer->ShowMixerControls(m_hInDev);
  if(!bShow && !mmr) bResult = m_pMixer->CanShowMixerControls(m_hInDev);

  if (!bWasOpen)
  {
    // close it again
    if (m_hInDev)
    {
      waveInReset(m_hInDev);
      mmr = waveInClose(m_hInDev);
      m_hInDev = NULL;
    }
  }
  return bResult;
}


/***************************************************************************/
// CWaveInDevice::ConnectMixer Connect window to receive mixer callback msg's
/***************************************************************************/
BOOL CWaveInDevice::ConnectMixer(CWnd* pCallback)
{
  BOOL bWasOpen = IsOpen();
  BOOL bConnected = FALSE;
  MMRESULT mmr = 0;
  if (!bWasOpen)
  {
    // device has to be opened first
    CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
    CView* pView = pMDIFrameWnd->GetCurrSaView();
    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    FmtParm* pFmtParm = pDoc->GetFmtParm(); // get pointer to wave format parameters
    mmr = waveInOpen(&m_hInDev, WAVE_MAPPER, (WAVEFORMATEX*)pFmtParm, NULL, 0, CALLBACK_NULL);
  }

  if(!mmr) bConnected = m_pMixer->Connect(m_hInDev, pCallback->GetSafeHwnd());

  if (!bWasOpen)
  {
    // close it again
    if (m_hInDev)
    {
      waveInReset(m_hInDev);
      mmr = waveInClose(m_hInDev);
      m_hInDev = NULL;
    }
  }
  if (mmr)
  {
    MMERR(mmr); // display error message
    return FALSE;
  }
  return bConnected;
}

/***************************************************************************/
// CWaveInDevice::Reset Reset the device
/***************************************************************************/
void CWaveInDevice::Reset()
{
  if (!m_hInDev) return;
  MMRESULT mmr = waveInReset(m_hInDev);
  if (mmr) MMERR(mmr);
}

/***************************************************************************/
// CWaveInDevice::WaveInDone Block finished recording
/***************************************************************************/
void CWaveInDevice::WaveInDone(CWave* /*pWave*/, WAVEHDR* pHdr)
{
  // unprepare the header
  MMRESULT mmr = waveInUnprepareHeader(m_hInDev, pHdr, sizeof(WAVEHDR));
  m_nBuffers--;
  if (mmr) MMERR(mmr); // display error message
  // free the header
  delete (pHdr);
}

/////////////////////////////////////////////////////////////////////////////
// CWaveInDevice message handlers

/***************************************************************************/
// CWaveInDevice::OnWimData
/***************************************************************************/
LRESULT CWaveInDevice::OnWimData(WPARAM /*wParam*/, LPARAM lParam)
{
  WAVEHDR *pHdr = (WAVEHDR *)lParam; // get pointer to wave header
  CWave *pWave = (CWave *)(pHdr->dwUser); // get pointer to CWave object
  CWaveInDevice* pInDev = pWave->GetInDevice(); // get pointer to input device
  ASSERT(pInDev);
  pWave->SetBufferSize(pWave->GetActiveBlock(), pHdr->dwBytesRecorded, pHdr->dwBytesRecorded);
  
	pInDev->WaveInDone(pWave, pHdr); // wave block done
  pWave->StoreBlock(); // store data block
  return 0;
}

//###########################################################################
// CWaveOutDevice output device

/////////////////////////////////////////////////////////////////////////////
// CWaveOutDevice message map
// Output device for playing. It uses a notification window to receive no-
// tification messages from the output device.

BEGIN_MESSAGE_MAP(CWaveOutDevice, CWnd)
//{{AFX_MSG_MAP(CWaveOutDevice)
ON_MESSAGE(MM_WOM_DONE, OnWomDone)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaveOutDevice construction/destruction/creation

/***************************************************************************/
// CWaveOutDevice::CWaveOutDevice Constructor
/***************************************************************************/
CWaveOutDevice::CWaveOutDevice()
{
  m_hOutDev = NULL;
  m_hWnd = NULL;
  m_pMixer = new CPlayMixer;
}

/***************************************************************************/
// CWaveOutDevice::~CWaveOutDevice Destructor
/***************************************************************************/
CWaveOutDevice::~CWaveOutDevice()
{
  ASSERT(m_hOutDev == NULL);
  if(m_pMixer)
    delete m_pMixer;
}

/***************************************************************************/
// CWaveOutDevice::Create Creation
// Creates a child window with the given parameters as notification window.
/***************************************************************************/
BOOL CWaveOutDevice::Create()
{
  // create notification window with size 0
  if (!CreateEx(0, AfxRegisterWndClass(0), _T("Wave Wnd"), WS_POPUP, 0, 0, 0, 0, NULL, NULL))
  {
    TRACE(_T("Failed to create wave notification window"));
    return FALSE;
  }
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWaveOutDevice helper functions

/***************************************************************************/
// CWaveOutDevice::Open Open the output device
// The device receives a handle to the notification window.
/***************************************************************************/
BOOL CWaveOutDevice::Open(WAVEFORMATEX *pFormat)
{
  MMRESULT mmr;
  // make sure we have a callback window
  if (!m_hWnd)
  {
    Create(); // create the callback window
    ASSERT(m_hWnd);
  }
  // see if output device already open for this format
  if (IsOpen())
  {
    // see if it can handle this format
    if (CanDoFormat(pFormat)) return TRUE; // OK
    else // already open, but for a different format
    {
      // error opening device
      CSaApp* pApp = (CSaApp*)AfxGetApp();
      pApp->ErrorMessage(IDS_ERROR_MMIO_OPENFORMAT);
      return FALSE;
    }
  }
  mmr = waveOutOpen(&m_hOutDev, WAVE_MAPPER, pFormat, (UINT)GetSafeHwnd(), 0, CALLBACK_WINDOW);
  if (mmr != 0)
  {
    MMERR(mmr); // display error message
    return FALSE;
  }
  return TRUE;
}

/***************************************************************************/
// CWaveOutDevice::CanDoFormat Check if the device could open for this format
/***************************************************************************/
BOOL CWaveOutDevice::CanDoFormat(WAVEFORMATEX *pFormat)
{
  MMRESULT mmr;
  // device already opened?
  if (!IsOpen())
  {
    TRACE(_T("Not open"));
    return FALSE;
  }
  HWAVEOUT hDev = NULL;
  mmr = waveOutOpen(&hDev, WAVE_MAPPER, pFormat, NULL, 0, WAVE_FORMAT_QUERY);
  if (mmr != 0)
  {
    MMERR(mmr); // display error message
    return FALSE;
  }
  return TRUE;
}

/***************************************************************************/
// CWaveOutDevice::Close Close the output device
/***************************************************************************/
BOOL CWaveOutDevice::Close()
{
  if (m_hOutDev)
  {
    // close the device
    waveOutReset(m_hOutDev);
    MSG msg;
    while (::PeekMessage(&msg, NULL, MM_WOM_DONE, MM_WOM_DONE, PM_REMOVE))
    {
      // There are two headers in the queue which are released by waveOutReset
      // They must be freed before we destroy the object
      // Since the OnWomDone starts new block we need to only call WaveOutDone
      WAVEHDR *pHdr = (WAVEHDR *)msg.lParam; // get pointer to wave header
      CWave *pWave = (CWave *)(pHdr->dwUser); // get pointer to CWave object
      WaveOutDone(pWave, pHdr); // wave block done
    }
    MMRESULT mmr = waveOutClose(m_hOutDev);
    if (mmr != 0) MMERR(mmr); // display error message
    m_hOutDev = NULL;
  }
  // destroy the window
  if (m_hWnd) DestroyWindow();
  ASSERT(m_hWnd == NULL);
  return TRUE;
}

/***************************************************************************/
// CWaveOutDevice::Play Start playing
/***************************************************************************/
BOOL CWaveOutDevice::Play(int nBuffer, UINT nVolume, CWave *pWave, BOOL bPause)
{
  if (pWave->GetPlaybackSize(nBuffer))
  {
    // device has to be opened
    if (!Open(pWave->GetFormat())) return FALSE;
    // allocate a header
    WAVEHDR* pHdr = new WAVEHDR;
    ASSERT(pHdr);
    // fill out the wave header
    memset(pHdr, 0, sizeof(WAVEHDR));
    pHdr->lpData = (char*) pWave->GetPlaybackPointer(nBuffer); // pointer to the data buffer
    pHdr->dwBufferLength = pWave->GetPlaybackSize(nBuffer); // size of the data buffer
    pHdr->dwUser = (DWORD)(void*)pWave;    // so we can find the object
    // prepare the header
    MMRESULT mmr = waveOutPrepareHeader(m_hOutDev, pHdr, sizeof(WAVEHDR));
    if (mmr)
    {
      MMERR(mmr); // display error message
      return FALSE;
    }
    // set the volume
    SetVolume(nVolume);
    if(bPause)
    {
      mmr = waveOutPause(m_hOutDev);
      if (mmr)
      {
        MMERR(mmr); // display error message
        return FALSE;
      }
    }
    // start it playing
    mmr = waveOutWrite(m_hOutDev, pHdr, sizeof(WAVEHDR));
    if (mmr)
    {
      MMERR(mmr); // display error message
      return FALSE;
    }

    if(!bPause)
    {
      mmr = waveOutRestart(m_hOutDev);
      if (mmr)
      {
        MMERR(mmr); // display error message
        return FALSE;
      }
    }
  }

  return TRUE;
}

/***************************************************************************/
// CWaveOutDevice::GetVolume Get the playback volume
/***************************************************************************/
UINT CWaveOutDevice::GetVolume(BOOL *pResult)
{
  DWORD dwVolume = 0xbfff; // Value in case of error
  BOOL bWasOpen = IsOpen();
  MMRESULT mmr = 0;
  if (!bWasOpen)
  {
    // device has to be opened first
    CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
    CView* pView = pMDIFrameWnd->GetCurrSaView();

    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    FmtParm* pFmtParm = pDoc->GetFmtParm(); // get pointer to wave format parameters
    mmr = waveOutOpen(&m_hOutDev, WAVE_MAPPER, (WAVEFORMATEX*)pFmtParm, NULL, 0, CALLBACK_NULL);
  }
  if(!mmr) mmr = m_pMixer->GetVolume(m_hOutDev, &dwVolume);

  if (!bWasOpen)
  {
    // close it again
    if (m_hOutDev)
    {
      waveOutReset(m_hOutDev);
      mmr = waveOutClose(m_hOutDev);
      m_hOutDev = NULL;
    }
  }
  if (pResult)
    *pResult = (mmr == MMSYSERR_NOERROR);
  return (UINT)(((float)(LOWORD(dwVolume)) * (float)100 / (float)0x0FFFF) + 0.5);
}

/***************************************************************************/
// CWaveOutDevice::SetVolume Set the playback volume
/***************************************************************************/
void CWaveOutDevice::SetVolume(UINT nVolume, BOOL *pResult)
{
  DWORD dwVolume= DWORD(((float)0x0FFFF * (float)nVolume / (float)100) + 0.5);
  BOOL bWasOpen = IsOpen();
  MMRESULT mmr = 0;
  if (!bWasOpen)
  {
    // device has to be opened first
    CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
    CView* pView = pMDIFrameWnd->GetCurrSaView();

    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    FmtParm* pFmtParm = pDoc->GetFmtParm(); // get pointer to wave format parameters
    mmr = waveOutOpen(&m_hOutDev, WAVE_MAPPER, (WAVEFORMATEX*)pFmtParm, NULL, 0, CALLBACK_NULL);
  }

  if(!mmr) mmr = m_pMixer->SetVolume(m_hOutDev, dwVolume);

  if (!bWasOpen)
  {
    // close it again
    if (m_hOutDev)
    {
      waveOutReset(m_hOutDev);
      mmr = waveOutClose(m_hOutDev);
      m_hOutDev = NULL;
    }
  }
  if (pResult)
    *pResult = (mmr == MMSYSERR_NOERROR);
}


/***************************************************************************/
// CWaveOutDevice::ShowMixer
/***************************************************************************/
BOOL CWaveOutDevice::ShowMixer(BOOL bShow)
{
  BOOL bWasOpen = IsOpen();
  BOOL bResult = FALSE;
  MMRESULT mmr = 0;
  if (!bWasOpen)
  {
    // device has to be opened first
    CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
    CView* pView = pMDIFrameWnd->GetCurrSaView();

    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    FmtParm* pFmtParm = pDoc->GetFmtParm(); // get pointer to wave format parameters
    mmr = waveOutOpen(&m_hOutDev, WAVE_MAPPER, (WAVEFORMATEX*)pFmtParm, NULL, 0, CALLBACK_NULL);
  }

  if(bShow && !mmr) bResult = m_pMixer->ShowMixerControls(m_hOutDev);
  if(!bShow && !mmr) bResult = m_pMixer->CanShowMixerControls(m_hOutDev);

  if (!bWasOpen)
  {
    // close it again
    if (m_hOutDev)
    {
      waveOutReset(m_hOutDev);
      mmr = waveOutClose(m_hOutDev);
      m_hOutDev = NULL;
    }
  }
  return bResult;
}


/***************************************************************************/
// CWaveOutDevice::ConnectMixer Connect window to receive mixer callback msg's
/***************************************************************************/
BOOL CWaveOutDevice::ConnectMixer(CWnd* pCallback)
{
  BOOL bWasOpen = IsOpen();
  BOOL bConnected = FALSE;
  MMRESULT mmr = 0;
  if (!bWasOpen)
  {
    // device has to be opened first
    CMainFrame* pMDIFrameWnd = (CMainFrame*)AfxGetMainWnd();
    CView* pView = pMDIFrameWnd->GetCurrSaView();

    CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();
    FmtParm* pFmtParm = pDoc->GetFmtParm(); // get pointer to wave format parameters
    mmr = waveOutOpen(&m_hOutDev, WAVE_MAPPER, (WAVEFORMATEX*)pFmtParm, NULL, 0, CALLBACK_NULL);
  }

  if(!mmr) bConnected = m_pMixer->Connect(m_hOutDev, pCallback->GetSafeHwnd());

  if (!bWasOpen)
  {
    // close it again
    if (m_hOutDev)
    {
      waveOutReset(m_hOutDev);
      mmr = waveOutClose(m_hOutDev);
      m_hOutDev = NULL;
    }
  }
  if (mmr)
  {
    MMERR(mmr); // display error message
    return FALSE;
  }

  return bConnected;
}


/***************************************************************************/
// CWaveOutDevice::Reset Reset the device
/***************************************************************************/
void CWaveOutDevice::Reset()
{
  if (!m_hOutDev) return;
  MMRESULT mmr = waveOutReset(m_hOutDev);
  if (mmr) MMERR(mmr);
}

/***************************************************************************/
// CWaveOutDevice::WaveOutDone Playing finished
/***************************************************************************/
void CWaveOutDevice::WaveOutDone(CWave * /*pWave*/, WAVEHDR *pHdr)
{
  // unprepare the header
  MMRESULT mmr = waveOutUnprepareHeader(m_hOutDev, pHdr, sizeof(WAVEHDR));
  if (mmr) MMERR(mmr); // display error message
  // free the header
  if (pHdr) delete pHdr;
}

/////////////////////////////////////////////////////////////////////////////
// CWaveOutDevice message handlers

/***************************************************************************/
// CWaveOutDevice::OnWomDone
/***************************************************************************/
LRESULT CWaveOutDevice::OnWomDone(WPARAM /*wParam*/, LPARAM lParam)
{
  WAVEHDR *pHdr = (WAVEHDR *)lParam; // get pointer to wave header
  CWave *pWave = (CWave *)(pHdr->dwUser); // get pointer to CWave object
  CWaveOutDevice *pOutDev = pWave->GetOutDevice(); // get pointer to output device
  ASSERT(pOutDev);
  pOutDev->WaveOutDone(pWave, pHdr); // wave block done
  pWave->NextBlock(); // prepare next data block
  return 0;
}


