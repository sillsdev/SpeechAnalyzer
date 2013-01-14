/////////////////////////////////////////////////////////////////////////////
// sa_w_rev.cpp:
// Implementation of the CProcessWbEcho
//             CProcessWbReverb classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_proc.h"
#include "sa_w_rev.h"

#include "math.h"
#include "resource.h"
#include "mainfrm.h"
#include "isa_doc.h"
#include "sa_w_dlg.h"
#include "settings\obstream.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessWbEcho
// Workbench process class to calculate new raw data with a echo function.

/////////////////////////////////////////////////////////////////////////////
// CProcessWbEcho construction/destruction/creation

/***************************************************************************/
// CProcessWbEcho::CProcessWbEcho Constructor
/***************************************************************************/
CProcessWbEcho::CProcessWbEcho()
{
	m_nDelay = 100;
	m_nGain = -20;
}

/***************************************************************************/
// CProcessWbEcho::~CProcessWbEcho Destructor
/***************************************************************************/
CProcessWbEcho::~CProcessWbEcho()
{
}

/////////////////////////////////////////////////////////////////////////////
// CProcessWbEcho helper functions

/***************************************************************************/
// CProcessWbEcho::Process Processing new raw data with a echo function
// The processed change data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessWbEcho::Process(void* pCaller, ISaDoc* pDoc, int nProgress,
							 int nLevel)
{
	//  TRACE(_T("Process: CProcessWbEcho\n"));
	if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
	CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
	// check if nested workbench processes
	int nOldLevel = nLevel; // save original level
	CDataProcess* pLowerProcess = NULL;
	if (nLevel < MAX_FILTER_NUMBER)
	{
		pLowerProcess = ((CDataProcess*)pMain->GetWbProcess(pDoc->GetWbProcess() - 1, nLevel));
		if (pLowerProcess)
		{
			// there is at least one more workbench processes to process first
			long lResult = pLowerProcess->Process(pCaller, pDoc, nProgress, ++nLevel);
			nLevel = (short int)LOWORD(lResult);
			nProgress = HIWORD(lResult);
		}
	}
	if ((nLevel == nOldLevel) && (IsDataReady())) return MAKELONG(--nLevel, nProgress); // data is already ready
	else
	{
		SetDataInvalid();
	}

	if (nLevel < 0) // previous processing error
	{
		if ((nLevel == PROCESS_CANCELED)) 
			CancelProcess(); // set your own cancel flag
		return MAKELONG(nLevel, nProgress);
	}

	// start process
	BeginWaitCursor(); // wait cursor
	if (!StartProcess(pCaller, IDS_STATTXT_PROCESSWBECHO)) // memory allocation failed
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}
	// create the temporary file
	if (!CreateTempFile(_T("ECH"))) // creating error
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		SetDataInvalid();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}
	// get source data size
	DWORD dwDataSize;
	if (pLowerProcess) dwDataSize = pLowerProcess->GetProcessedWaveDataSize();
	else dwDataSize = pDoc->GetUnprocessedDataSize(); // size of raw data
	DWORD dwBufferSize = GetBufferSize();
	FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
	WORD wSmpSize = (WORD)(pFmtParm->wBlockAlign / pFmtParm->wChannels);
	HPSTR pSourceData, pBlockStart; // pointers to source data
	HPSTR pTargetData; // pointers to target data
	DWORD dwDataPos = 0; // data position pointer
	DWORD dwRangeStart = 0;
	DWORD dwRangeEnd = dwDataSize - wSmpSize;

	float fGain = (float)1.0 / (float)pow(10.0, (double)m_nGain / -20.0);
	DWORD dwFifoSize = pFmtParm->dwSamplesPerSec * m_nDelay / 1000;
	// alocate the temporary global buffer for the fifo
	HANDLE hFifo = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwFifoSize * 2); // fifo is 16 bit
	if (!hFifo)
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return Exit(PROCESS_ERROR); // error, memory allocation
	}
	HPSTR lpFifo = (HPSTR)::GlobalLock(hFifo); // lock memory
	if (!lpFifo)
	{
		// memory lock error
		ErrorMessage(IDS_ERROR_MEMLOCK);
		::GlobalFree(hFifo);
		return Exit(PROCESS_ERROR); // error, memory allocation
	}
	// initialize fifo with silence
	short int * pFifo = (short int *)lpFifo;
	for (DWORD dwFifoIndex = 0; dwFifoIndex < dwFifoSize; dwFifoIndex++)
	{
		pFifo[dwFifoIndex] = 0;
	}
	DWORD dwFifoIndex = 0;
	// start processing loop
	while (dwDataPos < dwDataSize)
	{
		// load source data
		if (pLowerProcess) pBlockStart = pLowerProcess->GetProcessedWaveData(dwDataPos);
		else pBlockStart = pDoc->GetUnprocessedWaveData(dwDataPos); // get pointer to raw data block
		if (!pBlockStart)
		{
			::GlobalUnlock(hFifo);
			::GlobalFree(hFifo);
			return Exit(PROCESS_ERROR); // error, reading failed
		}
		pSourceData = pBlockStart;
		pTargetData = m_lpData;
		DWORD dwBlockEnd = dwDataPos + dwBufferSize;
		if (dwBlockEnd > dwDataSize)
		{
			dwBlockEnd = dwDataSize;
			dwBufferSize = dwDataSize - dwDataPos;
		}
		int nData;
		BYTE bData;
		while (dwDataPos < dwBlockEnd)
		{
			// read data
			if (wSmpSize == 1) // 8 bit data
			{
				bData = *pSourceData++; // data range is 0...255 (128 is center)
				nData = bData - 128;
			}
			else                  // 16 bit data
			{
				nData = *((short int*)pSourceData++);
				pSourceData++;
				dwDataPos++;
			}
			dwDataPos++;
			// process data
			if (wSmpSize == 1)
			{
				int nTempData = nData + (int)((float)(pFifo[dwFifoIndex]) * fGain);
				pFifo[dwFifoIndex] = (short)nData;
				if ((dwDataPos >= dwRangeStart) && (dwDataPos <= dwRangeEnd))
				{
					nData = nTempData;
					if (nData > 127) nData = 127;
					if (nData < -128) nData = -128;
				}
			}
			else
			{
				long lData = (long)nData + (long)((float)((long)pFifo[dwFifoIndex]) * fGain);
				pFifo[dwFifoIndex] = (short)nData;
				if ((dwDataPos >= dwRangeStart) && (dwDataPos <= dwRangeEnd))
				{
					if (lData > 32767) lData = 32767;
					if (lData < -32767) lData = -32767;
					nData = (int)lData;
				}
			}
			dwFifoIndex = ++dwFifoIndex % dwFifoSize;
			// save data
			if (wSmpSize == 1) // 8 bit data
			{
				bData = (BYTE)(nData + 128);
				*pTargetData++ = bData;
			}
			else                  // 16 bit data
			{
				*pTargetData++ = (BYTE)nData;
				*pTargetData++ = (BYTE)(nData >> 8);
			}
			// set progress bar
			SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
			if (IsCanceled())
			{
				::GlobalUnlock(hFifo);
				::GlobalFree(hFifo);
				return Exit(PROCESS_CANCELED); // process canceled
			}
		}
		// result block is ready, store it
		if (dwDataPos >= dwBlockEnd)
		{
			// write the processed data block
			try
			{
				Write((HPSTR)m_lpData, dwBufferSize);
			}
			catch (CFileException e)
			{
				// error writing file
				::GlobalUnlock(hFifo);
				::GlobalFree(hFifo);
				ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
				return Exit(PROCESS_ERROR); // error, writing failed
			}
		}
	}
	// free the fifo buffer
	::GlobalUnlock(hFifo);
	::GlobalFree(hFifo);
	nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
	// close the temporary file and read the status
	CloseTempFile(); // close the file
	EndProcess((nProgress >= 95)); // end data processing
	EndWaitCursor();
	SetDataReady();
	return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcessWbEcho::PropertiesDialog Calls the properties dialog for this process
/***************************************************************************/
int CProcessWbEcho::PropertiesDialog()
{
	CWbDlgFilterReverb dlgReverb;
	dlgReverb.m_nDelay = m_nDelay;
	dlgReverb.m_nGain = m_nGain;
	if (dlgReverb.DoModal() == IDOK)
	{
		m_nDelay = dlgReverb.m_nDelay;
		m_nGain = dlgReverb.m_nGain;
		SetDataInvalid();
		return IDOK;
	}
	return IDCANCEL;
}

static const char* psz_echo = "echo";
static const char* psz_reverb = "reverb";
static const char* psz_delay = "delay";
static const char* psz_gain = "gain";

/***************************************************************************/
// CProcessWbEcho::WriteProperties Write echo properties
/***************************************************************************/
void CProcessWbEcho::WriteProperties(Object_ostream& obs)
{
	obs.WriteBeginMarker(psz_echo);
	obs.WriteUInt(psz_delay, m_nDelay);
	obs.WriteInteger(psz_gain, m_nGain);
	obs.WriteEndMarker(psz_echo);
}

/***************************************************************************/
// CProcessWbEcho::ReadProperties Read echo properties
/***************************************************************************/
BOOL CProcessWbEcho::ReadProperties(Object_istream& obs)
{
	UINT nDelay = m_nDelay;
	int nGain = m_nGain;
	if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_echo)) return FALSE;
	while (!obs.bAtEnd())
	{
		if (obs.bReadUInt(psz_delay, m_nDelay));
		else if (obs.bReadInteger(psz_gain, m_nGain));
		else if (obs.bEnd(psz_echo))
			break;
	}
	// check if properties changed
	if ((nDelay != m_nDelay) || (nGain != m_nGain)) SetDataInvalid();
	return TRUE;
}

//###########################################################################
// CProcessWbReverb
// Workbench process class to calculate new raw data with a reverb function.

/////////////////////////////////////////////////////////////////////////////
// CProcessWbReverb construction/destruction/creation

/***************************************************************************/
// CProcessWbReverb::CProcessWbReverb Constructor
/***************************************************************************/
CProcessWbReverb::CProcessWbReverb()
{
	m_nDelay = 100;
	m_nGain = -20;
}

/***************************************************************************/
// CProcessWbReverb::~CProcessWbReverb Destructor
/***************************************************************************/
CProcessWbReverb::~CProcessWbReverb()
{
}

/////////////////////////////////////////////////////////////////////////////
// CProcessWbReverb helper functions

/***************************************************************************/
// CProcessWbReverb::Process Processing new raw data with a reverb function
// The processed change data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessWbReverb::Process(void* pCaller, ISaDoc* pDoc, int nProgress,
							   int nLevel)
{
	//  TRACE(_T("Process: CProcessWbReverb\n"));
	if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
	CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
	// check if nested workbench processes
	int nOldLevel = nLevel; // save original level
	CDataProcess* pLowerProcess = NULL;
	if (nLevel < MAX_FILTER_NUMBER)
	{
		pLowerProcess = ((CDataProcess*)pMain->GetWbProcess(pDoc->GetWbProcess() - 1, nLevel));
		if (pLowerProcess)
		{
			// there is at least one more workbench processes to process first
			long lResult = pLowerProcess->Process(pCaller, pDoc, nProgress, ++nLevel);
			nLevel = (short int)LOWORD(lResult);
			nProgress = HIWORD(lResult);
		}
	}
	if ((nLevel == nOldLevel) && (IsDataReady())) return MAKELONG(--nLevel, nProgress); // data is already ready
	else
	{
		if ((nLevel == PROCESS_CANCELED)) CancelProcess(); // set your own cancel flag
		SetDataInvalid();
	}

	if (nLevel < 0) // previous processing error
	{
		if ((nLevel == PROCESS_CANCELED)) 
			CancelProcess(); // set your own cancel flag
		return MAKELONG(nLevel, nProgress);
	}

	// start process
	BeginWaitCursor(); // wait cursor
	if (!StartProcess(pCaller, IDS_STATTXT_PROCESSWBREV)) // memory allocation failed
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}
	// create the temporary file
	if (!CreateTempFile(_T("REV"))) // creating error
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		SetDataInvalid();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}
	// get source data size
	DWORD dwDataSize;
	if (pLowerProcess) dwDataSize = pLowerProcess->GetProcessedWaveDataSize();
	else dwDataSize = pDoc->GetUnprocessedDataSize(); // size of raw data
	DWORD dwBufferSize = GetBufferSize();
	FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
	WORD wSmpSize = (WORD)(pFmtParm->wBlockAlign / pFmtParm->wChannels);
	HPSTR pSourceData, pBlockStart; // pointers to source data
	HPSTR pTargetData; // pointers to target data
	DWORD dwDataPos = 0; // data position pointer
	DWORD dwRangeStart = 0;
	DWORD dwRangeEnd = dwDataSize - wSmpSize;

	float fGain = (float)1.0 / (float)pow(10.0, (double)m_nGain / -20.0);
	DWORD dwFifoSize = pFmtParm->dwSamplesPerSec * m_nDelay / 1000;
	// alocate the temporary global buffer for the fifo
	HANDLE hFifo = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwFifoSize * 2); // fifo is 16 bit
	if (!hFifo)
	{
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		return Exit(PROCESS_ERROR); // error, memory allocation
	}
	HPSTR lpFifo = (HPSTR)::GlobalLock(hFifo); // lock memory
	if (!lpFifo)
	{
		// memory lock error
		ErrorMessage(IDS_ERROR_MEMLOCK);
		::GlobalFree(hFifo);
		return Exit(PROCESS_ERROR); // error, memory allocation
	}
	// initialize fifo with silence
	short int * pFifo = (short int *)lpFifo;
	DWORD dwFifoIndex;
	for (dwFifoIndex = 0; dwFifoIndex < dwFifoSize; dwFifoIndex++)
	{
		pFifo[dwFifoIndex] = 0;
	}
	dwFifoIndex = 0;
	// start processing loop
	while (dwDataPos < dwDataSize)
	{
		// load source data
		if (pLowerProcess) pBlockStart = pLowerProcess->GetProcessedWaveData(dwDataPos);
		else pBlockStart = pDoc->GetUnprocessedWaveData(dwDataPos); // get pointer to raw data block
		if (!pBlockStart)
		{
			::GlobalUnlock(hFifo);
			::GlobalFree(hFifo);
			return Exit(PROCESS_ERROR); // error, reading failed
		}
		pSourceData = pBlockStart;
		pTargetData = m_lpData;
		DWORD dwBlockEnd = dwDataPos + dwBufferSize;
		if (dwBlockEnd > dwDataSize)
		{
			dwBlockEnd = dwDataSize;
			dwBufferSize = dwDataSize - dwDataPos;
		}
		int nData;
		BYTE bData;
		while (dwDataPos < dwBlockEnd)
		{
			// read data
			if (wSmpSize == 1) // 8 bit data
			{
				bData = *pSourceData++; // data range is 0...255 (128 is center)
				nData = bData - 128;
			}
			else                  // 16 bit data
			{
				nData = *((short int*)pSourceData++);
				pSourceData++;
				dwDataPos++;
			}
			dwDataPos++;
			// process data
			if (wSmpSize == 1)
			{
				int nTempData = nData + (int)((float)(pFifo[dwFifoIndex]) * fGain);
				if ((dwDataPos >= dwRangeStart) && (dwDataPos <= dwRangeEnd))
				{
					nData = nTempData;
					if (nData > 127) nData = 127;
					if (nData < -128) nData = -128;
				}
				pFifo[dwFifoIndex] = (short)nData;
			}
			else
			{
				long lData = (long)nData + (long)((float)((long)pFifo[dwFifoIndex]) * fGain);
				if ((dwDataPos >= dwRangeStart) && (dwDataPos <= dwRangeEnd))
				{
					if (lData > 32767) lData = 32767;
					if (lData < -32767) lData = -32767;
					nData = (int)lData;
				}
				pFifo[dwFifoIndex] = (short)nData;
			}
			dwFifoIndex = ++dwFifoIndex % dwFifoSize;
			// save data
			if (wSmpSize == 1) // 8 bit data
			{
				bData = (BYTE)(nData + 128);
				*pTargetData++ = bData;
			}
			else                  // 16 bit data
			{
				*pTargetData++ = (BYTE)nData;
				*pTargetData++ = (BYTE)(nData >> 8);
			}
			// set progress bar
			SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
			if (IsCanceled())
			{
				::GlobalUnlock(hFifo);
				::GlobalFree(hFifo);
				return Exit(PROCESS_CANCELED); // process canceled
			}
		}
		// result block is ready, store it
		if (dwDataPos >= dwBlockEnd)
		{
			// write the processed data block
			try
			{
				Write((HPSTR)m_lpData, dwBufferSize);
			}
			catch (CFileException e)
			{
				// error writing file
				::GlobalUnlock(hFifo);
				::GlobalFree(hFifo);
				ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
				return Exit(PROCESS_ERROR); // error, writing failed
			}
		}
	}
	// free the fifo buffer
	::GlobalUnlock(hFifo);
	::GlobalFree(hFifo);
	nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
	// close the temporary file and read the status
	CloseTempFile(); // close the file
	EndProcess((nProgress >= 95)); // end data processing
	EndWaitCursor();
	SetDataReady();
	return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcessWbReverb::PropertiesDialog Calls the properties dialog for this process
/***************************************************************************/
int CProcessWbReverb::PropertiesDialog()
{
	CWbDlgFilterReverb dlgReverb;
	dlgReverb.m_nDelay = m_nDelay;
	dlgReverb.m_nGain = m_nGain;
	dlgReverb.m_bEcho = FALSE;
	if (dlgReverb.DoModal() == IDOK)
	{
		m_nDelay = dlgReverb.m_nDelay;
		m_nGain = dlgReverb.m_nGain;
		SetDataInvalid();
		return IDOK;
	}
	return IDCANCEL;
}

/***************************************************************************/
// CProcessWbReverb::WriteProperties Write reverb properties
/***************************************************************************/
void CProcessWbReverb::WriteProperties(Object_ostream& obs)
{
	obs.WriteBeginMarker(psz_reverb);
	obs.WriteUInt(psz_delay, m_nDelay);
	obs.WriteInteger(psz_gain, m_nGain);
	obs.WriteEndMarker(psz_reverb);
}

/***************************************************************************/
// CProcessWbReverb::ReadProperties Read reverb properties
/***************************************************************************/
BOOL CProcessWbReverb::ReadProperties(Object_istream& obs)
{
	UINT nDelay = m_nDelay;
	int nGain = m_nGain;
	if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_reverb)) return FALSE;
	while (!obs.bAtEnd())
	{
		if (obs.bReadUInt(psz_delay, m_nDelay));
		else if (obs.bReadInteger(psz_gain, m_nGain));
		else if (obs.bEnd(psz_reverb))
			break;
	}
	// check if properties changed
	if ((nDelay != m_nDelay) || (nGain != m_nGain)) SetDataInvalid();
	return TRUE;
}



