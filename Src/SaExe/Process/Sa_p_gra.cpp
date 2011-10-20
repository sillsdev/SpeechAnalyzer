/////////////////////////////////////////////////////////////////////////////
// sa_p_gra.cpp:
// Implementation of the CProcessGrappl class. 
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "sa_proc.h"
#include "sa_p_gra.h"

#include "isa_doc.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessGrappl
// class to calculate grappl pitch for wave data.

/////////////////////////////////////////////////////////////////////////////
// CProcessGrappl construction/destruction/creation

/***************************************************************************/
// CProcessGrappl::CProcessGrappl Constructor
/***************************************************************************/
CProcessGrappl::CProcessGrappl()
{
	// initialize algorithm parameters
	m_dAvgPitch = 0.;
}

/***************************************************************************/
// CProcessGrappl::~CProcessGrappl Destructor
/***************************************************************************/
CProcessGrappl::~CProcessGrappl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CProcessGrappl helper functions

/***************************************************************************/
// CProcessGrappl::Process Processing grappl pitch data
// The processed grappl pitch data is stored in a temporary file. To create
// it helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessGrappl::Process(void* pCaller, ISaDoc* pDoc, int nProgress, int nLevel)
{   
	if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
	if (IsDataReady()) 
		return MAKELONG(--nLevel, nProgress); // data is already ready
	TRACE(_T("Process: CProcessGrappl\n"));
	DWORD dwDataSize = pDoc->GetDataSize(); // raw data size
	if (!dwDataSize) return Exit(PROCESS_NO_DATA); // error, no valid data    
	FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data 
	// determine if process to be run in background    
	BOOL bBackground = FALSE; //pDoc->IsBackgroundProcessing();    

	if (nLevel < 0) // previous processing error
	{ 
		if ((nLevel == PROCESS_CANCELED)) 
			CancelProcess(); // set your own cancel flag
		return MAKELONG(nLevel, nProgress);
	}

	// start grappl process
	if (!bBackground) BeginWaitCursor(); // wait cursor
	if(!(bBackground?StartProcess(pCaller, IDS_STATTXT_BACKGNDGRA):
		StartProcess(pCaller, IDS_STATTXT_PROCESSGRA))) // memory allocation failed
	{ 
		EndProcess(); // end data processing
		if (!bBackground) EndWaitCursor();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}
	// if file has not been created
	Boolean ok = TRUE;
	if (!GetProcessFileName()[0])
	{ 
		// create the temporary grappl pitch file
		if (!CreateTempFile(_T("GRA"))) // creating error
		{ EndProcess(); // end data processing
		if (!bBackground) EndWaitCursor();
		SetDataInvalid();
		return MAKELONG(PROCESS_ERROR, nProgress);
		}
		// initialise parameters
		m_dwDataPos = 0; 
		m_nMinValue = SHRT_MAX;
		m_CalcParm.sampfreq = (int32)pFmtParm->dwSamplesPerSec;
		m_CalcParm.eightbit = (int16)(pFmtParm->wBlockAlign == 1);
		m_CalcParm.mode = Grappl_fullpitch;
		m_CalcParm.smoothfreq = 1000;                                                 
		m_CalcParm.minpitch = 40;
		m_CalcParm.maxpitch = 500;
		m_CalcParm.calcint = 100;
		m_CalcParm.minvoiced16 = 32;
		m_CalcParm.maxchange_pc10 = 200;
		m_CalcParm.minsigpoints  = 6;
		m_CalcParm.minmeanweight = 60;
		m_CalcParm.maxinterp_pc10 = 300;
		m_CalcParm.reslag = 0;      
		// check if enough workspace for grappl
		UINT nWorkSpace = grapplWorkspace();
		if (GetBufferSize() < (DWORD)nWorkSpace)
		{ // buffer too small
			TCHAR szText[6];
			swprintf_s(szText, _countof(szText), _T("%u"), nWorkSpace);
			ErrorMessage(IDS_ERROR_GRAPPLSPACE, szText);
			return Exit(PROCESS_ERROR); // error, buffer too small
		}
		// init grappl
		ok = grapplInit(m_lpData, &m_CalcParm);
	}  
	else
	{ // open file to append data
		if (!OpenFileToAppend())
		{ 
			EndProcess(); // end data processing
			if (!bBackground) EndWaitCursor();
			SetDataInvalid();
			return MAKELONG(PROCESS_ERROR, nProgress);
		}      
	}
	int16 alldone = FALSE;
	int16 nomore = FALSE;
	// get block size
	DWORD dwBlockSize = 0x10000 - pFmtParm->wBlockAlign; // 64k - 1
	if (GetBufferSize() < dwBlockSize) 
		dwBlockSize = GetBufferSize();
	HPSTR pBlockStart;
	// start processing
	while (ok && (m_dwDataPos < dwDataSize))
	{ // get raw data block
		pBlockStart = pDoc->GetWaveData(m_dwDataPos, TRUE); // get pointer to data block
		if (!pBlockStart) 
			return Exit(PROCESS_ERROR); // error, reading failed
		m_dwDataPos += dwBlockSize;
		if (m_dwDataPos >= dwDataSize)
		{ 
			dwBlockSize -= (m_dwDataPos - dwDataSize);
			nomore = TRUE;
		}
		// set grappl input buffer
		ok = grapplSetInbuff((pGrappl)m_lpData, (pGrappl)pBlockStart, (WORD)(dwBlockSize / pFmtParm->wBlockAlign), nomore);
		if (!ok) 
			break;
		// process
		pGrappl_res pResults;
		int16 nresults;
		/***************************** DEBUG ONLY *************************************/     
#ifdef DUMP     
		FILE *hPitchData = fopen("Grappl.txt", "w");    
#endif
		/******************************************************************************/     
		DWORD dwPitchCount = 0;
		while (grapplGetResults((pGrappl)m_lpData, &pResults, &nresults, &alldone))
		{ // get max and min values and save the results
			for (int16 nLoop = 0; nLoop < nresults; nLoop++)
			{ // find min and max values      
				if (pResults->fsmooth16 >= 0)
				{ // adjust result to precision multiplier format
					pResults->fsmooth16 = (int16)(((long)pResults->fsmooth16 * (long)PRECISION_MULTIPLIER + 8L)/ 16L);
					if (pResults->fsmooth16 > m_nMaxValue) m_nMaxValue = pResults->fsmooth16; // save maximum value
					if (pResults->fsmooth16 < m_nMinValue) m_nMinValue = pResults->fsmooth16; // save minimum value
				}
				else pResults->fsmooth16 = -1; // set this point as unset
				/***************************** DEBUG ONLY *************************************/          
#ifdef DUMP               
				fprintf(hPitchData, "%5d ", pResults->fsmooth16);
#endif                                                                           
				/******************************************************************************/
				// keep a running average of the pitch
				if (pResults->fsmooth16 > 0)
				{ // average in voiced regions only
					dwPitchCount++;
					m_dAvgPitch = ((double)(dwPitchCount-1)*m_dAvgPitch + (double)pResults->fsmooth16/PRECISION_MULTIPLIER) / 
						(double)dwPitchCount;
				}                
				// write one result of the processed grappl pitch data
				try
				{ Write((HPSTR)&pResults->fsmooth16, sizeof(int16));
				}
				catch (CFileException e)
				{ // error writing file
					ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
					return Exit(PROCESS_ERROR); // error, writing failed
				}
				pResults++;
			}
		}
		/***************************** DEBUG ONLY *************************************/        
#ifdef DUMP             
		fclose(hPitchData);
#endif
		/******************************************************************************/
		// set progress bar
		SetProgress(nProgress + (int)(100 * m_dwDataPos / dwDataSize / (DWORD)nLevel));
		if (IsCanceled()) return Exit(PROCESS_CANCELED); // process canceled
		if (bBackground || alldone) break;
	}
	if (!ok) 
		return Exit(PROCESS_ERROR); // error, processing failed
	// calculate the actual progress
	nProgress = nProgress + (int)(100 / nLevel);
	// close the temporary file and read the status
	CloseTempFile(); // close the file
	if (alldone)
	{
		if (GetDataSize() < 2) 
			return Exit(PROCESS_ERROR); // error, not enough data
		SetStatusFlag(PROCESS_NO_PITCH, m_nMinValue == SHRT_MAX); 
	}    
	EndProcess((nProgress >= 95)); // end data processing
	if (!bBackground) EndWaitCursor();
	if (nomore || alldone) 
		SetDataReady();
	return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcessGrappl::IsVoiced  Checks if waveform section at offset is voiced.
/***************************************************************************/
BOOL CProcessGrappl::IsVoiced(ISaDoc *pDoc, DWORD dwWaveOffset)
{                                                    
	FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
	UINT nSmpSize = pFmtParm->wBlockAlign / pFmtParm->wChannels;
	DWORD dwSmpOffset = dwWaveOffset / nSmpSize;
	DWORD dwPitchBlock = m_dwBufferOffset; 
	BOOL bDone;
	Process(this, (ISaDoc *)pDoc);
	BOOL bVoiced = (GetProcessedData(dwSmpOffset / Grappl_calc_intvl, &bDone) > 0);
	if (!bDone) return FALSE; 
	if (dwPitchBlock != UNDEFINED_OFFSET && m_dwBufferOffset != dwPitchBlock) 
		GetProcessedData(dwPitchBlock/sizeof(short), TRUE);  // restore pitch buffer
	return(bVoiced);
}

/***************************************************************************/
// CProcessGrappl::GetUncertainty  Returns uncertainty at a specific pitch
/***************************************************************************/
double CProcessGrappl::GetUncertainty(double fPitch)
{
	double fFs = m_CalcParm.sampfreq; // sampling frequency
	double fUncertainty = fPitch * fPitch / (2 * fFs - fPitch);
	if (fUncertainty < 0.1)
		fUncertainty = 0.1;

	return fUncertainty;
}

