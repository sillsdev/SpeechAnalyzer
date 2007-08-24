/////////////////////////////////////////////////////////////////////////////
// sa_p_pit.cpp:
// Implementation of the CProcessPitch class. 
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "sa_proc.h"
#include "sa_p_pit.h"

#include <limits.h>
#include "isa_doc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessPitch
// class to calculate pitch for wave data. This class stores raw pitch and
// quality data into temporary files.

/////////////////////////////////////////////////////////////////////////////
// CProcessPitch construction/destruction/creation

/***************************************************************************/
// CProcessPitch::CProcessPitch Constructor
/***************************************************************************/
CProcessPitch::CProcessPitch()
{
}

/***************************************************************************/
// CProcessPitch::~CProcessPitch Destructor
/***************************************************************************/
CProcessPitch::~CProcessPitch()
{
}

/////////////////////////////////////////////////////////////////////////////
// CProcessPitch helper functions


/***************************************************************************/
// CProcessPitch::Process Processing pitch data
// The processed pitch data is stored in a temporary file. Also quality data,
// a product of pitch calculating is stored in a different temporary file for
// further use. To create it helper functions of the base class are used.
// While processing a process bar, placed on the status bar, has to be
// updated. The level tells which processing level this process has been
// called, start process start on which processing percentage this process
// starts (for the progress bar). The status bar process bar will be updated
// depending on the level and the progress start. The return value returns
// the highest level througout the calling queue, or -1 in case of an error
// in the lower word of the long value and the end process progress
// percentage in the higher word.  Uses WinCecil 2.2 pitch algorithm.
/***************************************************************************/
long CProcessPitch::Process(void* pCaller, ISaDoc* pDoc, int nProgress,
                int nLevel)
{   
  TRACE(_T("Process: CProcessPitch\n"));
  if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
  if (IsDataReady()) 
    return MAKELONG(--nLevel, nProgress); // data is already ready
  DWORD dwDataSize = pDoc->GetDataSize(); // raw data size
  if (!dwDataSize) return Exit(PROCESS_NO_DATA); // error, no valid data    
  FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data 
  const UttParm* pUttParm = pDoc->GetUttParm(); // get sa parameters utterance member data

  if (nLevel < 0) // previous processing error
  { 
    if ((nLevel == PROCESS_CANCELED)) 
      CancelProcess(); // set your own cancel flag
    return MAKELONG(nLevel, nProgress);
  }

  // start pitch process
  BeginWaitCursor(); // wait cursor
  if(!StartProcess(pCaller, IDS_STATTXT_PROCESSPIT)) // previous processing error
  { 
    EndProcess(); // end data processing
    EndWaitCursor();
    return MAKELONG(PROCESS_ERROR, nProgress);
  }
  // if file has not been created
  Boolean ok = TRUE;
  if (!GetProcessFileName()[0])
  { // create the temporary grappl pitch file
    if (!CreateTempFile(_T("PIT"))) // creating error
    { EndProcess(); // end data processing
      SetDataInvalid();
      return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // initialise parameters
    m_dwDataPos = 0; 
    m_nMinValue = SHRT_MAX;
    m_CalcParm.sampfreq = (int32)pFmtParm->dwSamplesPerSec;
	WORD wSmpSize = WORD(pFmtParm->wBlockAlign/pFmtParm->wChannels);
	m_CalcParm.eightbit = (int16)(wSmpSize == 1);
	m_CalcParm.mode = Grappl_fullpitch;;
	m_CalcParm.smoothfreq = 1000L;
	m_CalcParm.calcint= 100;   //!!this should be based on sampling frequency
	m_CalcParm.minmeanweight = 60;
	m_CalcParm.maxinterp_pc10 = 300;
	m_CalcParm.minpitch = int16(pUttParm->nMinFreq);
	m_CalcParm.maxpitch = int16(pUttParm->nMaxFreq);
	m_CalcParm.minvoiced16 = int16((pUttParm->TruncatedCritLoud(pFmtParm->wBitsPerSample)*16+8)/PRECISION_MULTIPLIER);
	m_CalcParm.maxchange_pc10 = int16(pUttParm->nMaxChange * 10);
	m_CalcParm.minsigpoints = int16(pUttParm->nMinGroup);
	m_CalcParm.reslag = 0;

    // check if enough workspace for grappl
    UINT nWorkSpace = grapplWorkspace();
    if (GetBufferSize() < (DWORD)nWorkSpace)
    { // buffer too small
      TCHAR szText[6];
      _stprintf(szText, _T("%u"), nWorkSpace);
      ErrorMessage(IDS_ERROR_GRAPPLSPACE, szText);
      return Exit(PROCESS_ERROR); // error, buffer too small
    }
    // init grappl
    ok = grapplInit(m_lpData, &m_CalcParm);
  }  
  else
  { // open file to append data
    if (!OpenFileToAppend())
    { EndProcess(); // end data processing
      SetDataInvalid();
      return MAKELONG(PROCESS_ERROR, nProgress);
    }      
  }
  int16 alldone = FALSE;
  int16 nomore = FALSE;
  // get block size
  DWORD dwBlockSize = 0x10000 - pFmtParm->wBlockAlign; // 64k - 1
  if (GetBufferSize() < dwBlockSize) dwBlockSize = GetBufferSize();
  HPSTR pBlockStart;
  // start processing
  while (ok && (m_dwDataPos < dwDataSize))
  { // get raw data block
    pBlockStart = pDoc->GetWaveData(m_dwDataPos, TRUE); // get pointer to data block
    if (!pBlockStart) 
        return Exit(PROCESS_ERROR); // error, reading failed
    m_dwDataPos += dwBlockSize;
    if (m_dwDataPos >= dwDataSize)
    { dwBlockSize -= (m_dwDataPos - dwDataSize);
      nomore = TRUE;
    }
    // set grappl input buffer
    ok = grapplSetInbuff((pGrappl)m_lpData, (pGrappl)pBlockStart, (WORD)(dwBlockSize / pFmtParm->wBlockAlign), nomore);
    if (!ok) break;
    // process
    pGrappl_res pResults;
    int16 nresults;
    while (grapplGetResults((pGrappl)m_lpData, &pResults, &nresults, &alldone))
    { // get max and min values and save the results
      for (int16 nLoop = 0; nLoop < nresults; nLoop++)
      { // find min and max values      
        if (pResults->fcalc16 >= 0)
        { // adjust result to precision multiplier format
          pResults->fcalc16 = (int16)(((long)pResults->fcalc16 * (long)PRECISION_MULTIPLIER + 8L)/ 16L);
          if (pResults->fcalc16 > m_nMaxValue) m_nMaxValue = pResults->fcalc16; // save maximum value
          if (pResults->fcalc16 < m_nMinValue) m_nMinValue = pResults->fcalc16; // save minimum value
        }
        else pResults->fcalc16 = -1; // set this point as unset
        // write one result of the processed grappl pitch data
        try
        { Write((HPSTR)&pResults->fcalc16, sizeof(int16));
        }
        catch (CFileException e)
        { // error writing file
          ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
          return Exit(PROCESS_ERROR); // error, writing failed
        }

        pResults++;
      }
    }
    // set progress bar
    SetProgress(nProgress + (int)(100 * m_dwDataPos / dwDataSize / (DWORD)nLevel));
    if (IsCanceled()) return Exit(PROCESS_CANCELED); // process canceled
    if (alldone) break;
  }
  if (!ok) 
    return Exit(PROCESS_ERROR); // error, processing failed
  // calculate the actual progress
  nProgress = nProgress + (int)(100 / nLevel);
  // close the temporary file and read the status
  CloseTempFile(); // close the file
  if (GetDataSize() < 2) return Exit(PROCESS_ERROR); // error, not enough data

  if (alldone)
  {
    if (GetDataSize() < 2) 
      return Exit(PROCESS_ERROR); // error, not enough data        //!!should be TRUE = close file?
    SetStatusFlag(PROCESS_NO_PITCH, m_nMinValue == SHRT_MAX); 
  }    
  EndProcess((nProgress >= 95)); // end data processing
  if (nomore || alldone) SetDataReady();
  return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcessPitch::GetUncertainty  Returns uncertainty at a specific pitch
/***************************************************************************/
double CProcessPitch::GetUncertainty(double fPitch)
{
  double fFs = m_CalcParm.sampfreq; // sampling frequency
  double fUncertainty = fPitch * fPitch / (2 * fFs - fPitch);
  if (fUncertainty < 0.1)
    fUncertainty = 0.1;
  
  return fUncertainty;
}
