/////////////////////////////////////////////////////////////////////////////
// sa_p_rat.cpp:
// Implementation of the CProcessRatio class. 
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_proc.h"
#include "sa_p_rat.h"
#include "sa_p_cha.h"
#include "sa_p_lou.h"


#include "isa_doc.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessRatio
// class to calculate ratio detector values for wave data.

/////////////////////////////////////////////////////////////////////////////
// CProcessRatio construction/destruction/creation

/***************************************************************************/
// CProcessRatio::CProcessRatio Constructor
/***************************************************************************/
CProcessRatio::CProcessRatio()
{
}

/***************************************************************************/
// CProcessRatio::~CProcessRatio Destructor
/***************************************************************************/
CProcessRatio::~CProcessRatio()
{
}

/////////////////////////////////////////////////////////////////////////////
// CProcessRatio helper functions

/***************************************************************************/
// CProcessRatio::Process Processing change data
// The processed ratio data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessRatio::Process(void* pCaller, ISaDoc* pDoc, int nProgress,
               int nLevel)
{   
  TRACE(_T("Process: CProcessRatio\n"));
  if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
  if (IsDataReady()) return MAKELONG(--nLevel, nProgress); // data is already ready
  // create raw change data, its quality and smoothed loudness output is needed to calculate change
  CProcessChange* pChange = (CProcessChange*)pDoc->GetChange(); // get pointer to raw change object
  CProcessSmoothLoudness* pLoudness = (CProcessSmoothLoudness*)pDoc->GetSmoothLoudness(); // get pointer to smoothed loudness object
  long lResult = pChange->Process(pCaller, pDoc, nProgress, ++nLevel); // process data
  nLevel = (short int)LOWORD(lResult);
  nProgress = HIWORD(lResult);

  if (nLevel < 0) // previous processing error
  { 
    if ((nLevel == PROCESS_CANCELED)) 
      CancelProcess(); // set your own cancel flag
    return MAKELONG(nLevel, nProgress);
  }

  // start change process
  BeginWaitCursor(); // wait cursor
  if(!StartProcess(pCaller, IDS_STATTXT_PROCESSRAT)) // memory allocation failed
  { 
    EndProcess(); // end data processing
    EndWaitCursor();
    return MAKELONG(PROCESS_ERROR, nProgress);
  }
  if (!pLoudness->SmoothData(2)) // smooth smoothed loudness two times
  { 
    EndProcess(); // end data processing
    EndWaitCursor();
    return MAKELONG(PROCESS_ERROR, nProgress);
  }
  FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
  const UttParm* pUttParm = pDoc->GetUttParm(); // get sa parameters utterance member data
  UINT nBlockAlign = pFmtParm->wBlockAlign;
  // calculate current processing position (copied but not understood from WINCECIL)
  DWORD dwLoopStart = ((pFmtParm->dwSamplesPerSec / (2 * MAX_CALCULATION_FREQUENCY)
    + pFmtParm->dwSamplesPerSec / pUttParm->nMinFreq) / 2
    + CALCULATION_DATALENGTH(pFmtParm->dwSamplesPerSec) / 2 + 5) / CALCULATION_INTERVAL(pFmtParm->dwSamplesPerSec) + 1;
  DWORD dwDataSize = pLoudness->GetDataSize();
  DWORD dwLoopEnd = dwDataSize - dwLoopStart; // end of loop
  // create the temporary change file
  if (!CreateTempFile(_T("RAT"))) // creating error
  { 
    EndProcess(); // end data processing
    EndWaitCursor();
    SetDataInvalid();
    return MAKELONG(PROCESS_ERROR, nProgress);
  }
  // process smoothed loudness and quality data into change data
  short int * pChangeData = (short int *)m_lpData; // pointer to smoothed change data
  DWORD dwChangeCount = 0; // reset processed data counter
  DWORD dwLoopPos = 0;
  DWORD dwChangeBlock = 0;
  // prepare source data
  short int * pQuality = (short int *)pChange->GetQualityData(dwChangeBlock, TRUE); // read quality data block
  short int * pLoudData = (short int *)pLoudness->GetProcessedData(dwChangeBlock, TRUE); // read smoothed loudness data block
  if ((!pQuality) || (!pLoudData)) return Exit(PROCESS_ERROR); // error, reading failed
  long lMultiplier = 1;
  if (nBlockAlign > 1) lMultiplier = PRECISION_MULTIPLIER; // 16 bit data, quality comes divided by PRECISION_MULTIPLIER
  // start processing
  while (dwLoopPos < dwDataSize)
  { 
    *pChangeData = (short)(((long)*pQuality++ * 200 * lMultiplier) / ((long)*pLoudData++ + 70) / lMultiplier);
    if ((dwLoopPos < dwLoopStart) || (dwLoopPos >= dwLoopEnd)) *pChangeData = 0;
    if (*pChangeData > m_nMaxValue) m_nMaxValue = *pChangeData; // save maximum value
    pChange++;
    dwLoopPos++;
    // set progress bar
    SetProgress(nProgress + (int)(100 * dwLoopPos / dwDataSize / (DWORD)nLevel));
    if (IsCanceled()) return Exit(PROCESS_CANCELED); // process canceled
    if ((++dwChangeCount == GetProcessBufferSize() / 2) || (dwLoopPos >= dwDataSize)) // change buffer is full or processing finished
    { 
      // write the processed change data block
      try
      { 
        Write((HPSTR)m_lpData, dwChangeCount * 2);
      }
      catch (CFileException e)
      { 
        // error writing file
        ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
        return Exit(PROCESS_ERROR); // error, writing failed
      }
      pChangeData = (short int *)m_lpData; // reset pointer to begin of change data buffer
      dwChangeBlock += dwChangeCount;
      dwChangeCount = 0; // reset processed data counter
      if (dwChangeCount < dwDataSize)
      { 
        // read next source data blocks
        pQuality = (short int *)pChange->GetQualityData(dwChangeBlock, TRUE); // read quality data block
        pLoudData = (short int *)pLoudness->GetProcessedData(dwChangeBlock, TRUE); // read smoothed loudness data block
        if ((!pQuality) || (!pLoudData)) return Exit(PROCESS_ERROR); // error, reading failed
      }
    }    
  }
  nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
  // close the temporary change file and read the status
  CloseTempFile(); // close the file
  BOOL bRes = SmoothData(2); // smooth data two times
  if (bRes) EndProcess((nProgress >= 95)); // end data processing
  else EndProcess();
  EndWaitCursor();
  SetDataReady(bRes);
  if (bRes) return MAKELONG(nLevel, nProgress);
  else return MAKELONG(PROCESS_ERROR, nProgress);
}
