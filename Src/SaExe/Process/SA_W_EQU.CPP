/////////////////////////////////////////////////////////////////////////////
// sa_w_equ.cpp:
// Implementation of the CProcessWbEquation class.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_proc.h"
#include "sa_w_equ.h"

#include "resource.h"
#include "mainfrm.h"
#include "isa_doc.h"
#include "sa_w_dlg.h"
#include "zgraph\toolkit.h"
#include "settings\obstream.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessWbEquation
// Workbench process class to calculate new raw data after an equation given
// by the user.

/////////////////////////////////////////////////////////////////////////////
// CProcessWbEquation construction/destruction/creation

/***************************************************************************/
// CProcessWbEquation::CProcessWbEquation Constructor
/***************************************************************************/
CProcessWbEquation::CProcessWbEquation()
{
	m_szEquation.Empty();
}

/***************************************************************************/
// CProcessWbEquation::~CProcessWbEquation Destructor
/***************************************************************************/
CProcessWbEquation::~CProcessWbEquation()
{
}

/////////////////////////////////////////////////////////////////////////////
// CProcessWbEquation helper functions

/***************************************************************************/
// CProcessWbEquation::CheckFunction Check validity of equation
/***************************************************************************/
BOOL CProcessWbEquation::CheckFunction(CSaString* pszFunction)
{
	if (pszFunction && !pszFunction->IsEmpty())
	{
		int nType = 0;
		parseUtf8(*pszFunction, &nType, 1.0);
		if (nType) return TRUE;
	}
	return FALSE;
}

/***************************************************************************/
// CProcessWbEquation::Process Processing new raw data after an equation
// The processed change data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level throughout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessWbEquation::Process(void* pCaller, ISaDoc* pDoc, int nProgress,
								 int nLevel)
{
	//  TRACE(_T("Process: CProcessWbEquation\n"));
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
	if ((nLevel == nOldLevel) && (IsDataReady()))
		return MAKELONG(--nLevel, nProgress); // data is already ready
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
	if(!StartProcess(pCaller, IDS_STATTXT_PROCESSWBEQU)) // memory allocation failed
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}
	// create the temporary file
	if (!CreateTempFile(_T("EQU"))) // creating error
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		SetDataInvalid();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}
	// get source data size
	DWORD dwDataSize;
	if (pLowerProcess)
		dwDataSize = pLowerProcess->GetProcessedWaveDataSize();
	else
		dwDataSize = pDoc->GetUnprocessedDataSize(); // size of raw data
	DWORD dwBufferSize = GetBufferSize();
	FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
	WORD wSmpSize = WORD(pFmtParm->wBlockAlign / pFmtParm->wChannels);
	HPSTR pSourceData, pBlockStart; // pointers to source data
	HPSTR pTargetData; // pointers to target data
	DWORD dwDataPos = 0; // data position pointer
	DWORD dwRangeStart = 0;
	DWORD dwRangeEnd = dwDataSize - wSmpSize;

	// start processing loop
	while (dwDataPos < dwDataSize)
	{
		// load source data
		if (pLowerProcess)
			pBlockStart = pLowerProcess->GetProcessedWaveData(dwDataPos);
		else
			pBlockStart = pDoc->GetUnprocessedWaveData(dwDataPos); // get pointer to raw data block
		if (!pBlockStart)
			return Exit(PROCESS_ERROR); // error, reading failed
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
			// process this data point with the function parser from Z-Graph
			if (!m_szEquation.IsEmpty() && (dwDataPos >= dwRangeStart) && (dwDataPos <= dwRangeEnd))
			{
				int nType = 0;
				double dData = (double)nData;
				if (!nData) dData = 1.7E-12; // to protect parser from crashing
				dData = (int)parseUtf8(m_szEquation, &nType, dData);
				if (!nType) return Exit(PROCESS_ERROR); // error, parsing failed
				nData = (int)dData;
				// do boundary check
				if (wSmpSize == 1) // 8 bit data
				{
					if (dData > 127) nData = 127;
					if (dData < -128) nData = -128;
				}
				else                  // 16 bit data
				{
					if (dData > (double)32767) nData = 0x7FFF;
					if (dData < (double)-32767) nData = 0xFFFF;
				}
			}
			// save data
			if (wSmpSize == 1) // 8 bit data
			{
				bData = BYTE(nData + 128);
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
				ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
				return Exit(PROCESS_ERROR); // error, writing failed
			}
		}
	}
	nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
	// close the temporary file and read the status
	CloseTempFile(); // close the file
	EndProcess((nProgress >= 95)); // end data processing
	EndWaitCursor();
	SetDataReady();
	return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcessWbEquation::PropertiesDialog Calls the properties dialog for this process
/***************************************************************************/
int CProcessWbEquation::PropertiesDialog()
{
	CWbDlgFilterEquation dlgEquation;
	dlgEquation.m_szEquation = m_szEquation;
	if (dlgEquation.DoModal() == IDOK)
	{
		m_szEquation = dlgEquation.m_szEquation;
		SetDataInvalid();
		return IDOK;
	}
	return IDCANCEL;
}

static const char* psz_equation = "equation";
static const char* psz_text = "text";

/***************************************************************************/
// CProcessWbEquation::WriteProperties Write equation properties
/***************************************************************************/
void CProcessWbEquation::WriteProperties(Object_ostream& obs)
{
	obs.WriteBeginMarker(psz_equation);
	obs.WriteString(psz_text, m_szEquation);
	obs.WriteEndMarker(psz_equation);
}

/***************************************************************************/
// CProcessWbEquation::bReadProperties Read equation properties
/***************************************************************************/
BOOL CProcessWbEquation::bReadProperties(Object_istream& obs)
{
	CString szSafe = m_szEquation;
	if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_equation)) return FALSE;
	while (!obs.bAtEnd())
	{
		if (obs.bReadString(psz_text, &m_szEquation));
		else if (obs.bEnd(psz_equation))
			break;
	}
	// check if properties changed
	if (szSafe != m_szEquation) SetDataInvalid();
	return TRUE;
}
