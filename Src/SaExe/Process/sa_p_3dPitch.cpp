// sa_p_3dPitch.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "isa_doc.h"
#include "sa_p_3dPitch.h"
#include "Butterworth.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcess3dPitch

CProcess3dPitch::CProcess3dPitch()
{
	m_pSourceProcess = NULL;
	m_dFilterUpperFrequency = 1000.;
	m_dFilterLowerFrequency = 70.;
	m_nFilterOrder = 5;
}

CProcess3dPitch::~CProcess3dPitch()
{
}


BEGIN_MESSAGE_MAP(CProcess3dPitch, CDataProcess)
	//{{AFX_MSG_MAP(CProcess3dPitch)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcess3dPitch message handlers

static int ReadDataBlock( CDataProcess *pSource, DWORD dwStart, DWORD dwStop, DWORD dwPos, int wSmpSize);

/***************************************************************************/
// CProcess3dPitch::Process 
/***************************************************************************/
long CProcess3dPitch::Process(void* pCaller, ISaDoc* pSaDoc, int nProgress, int nLevel)
{
	if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
	ISaDoc* pDoc = (ISaDoc*)pSaDoc; // cast pointer
	// check if nested workbench processes
	int nOldLevel = nLevel; // save original level

	CDataProcess* pLowerProcess = NULL;
	pLowerProcess = m_pSourceProcess;
	if (pLowerProcess)
	{
		// there is at least one source processes to process first
		long lResult = pLowerProcess->Process(pCaller, pSaDoc, nProgress, ++nLevel);
		nLevel = (short int)LOWORD(lResult);
		nProgress = HIWORD(lResult);
	}
	if ((nLevel == nOldLevel) && (IsDataReady())) 
		return MAKELONG(--nLevel, nProgress); // data is already ready
	else
		SetDataInvalid();

	if (nLevel < 0) // previous processing error
	{
		if ((nLevel == PROCESS_CANCELED))
			CancelProcess(); // set your own cancel flag
		return MAKELONG(nLevel, nProgress);
	}

	// start process
	BeginWaitCursor(); // wait cursor
	if(!StartProcess(pCaller, IDS_STATTXT_PROCESSWBLP)) // memory allocation failed or previous processing error
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}
	// create the temporary file
	if (!CreateTempFile(_T("PCC"))) // creating error
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
	FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
	WORD wSmpSize = (WORD)(pFmtParm->wBlockAlign / pFmtParm->wChannels);

	CButterworth *pButterworth = new CButterworth;

	pButterworth->SetSourceProcess(pLowerProcess);
	pButterworth->SetFilterFilter(TRUE);
	//  pButterworth->BandPass( m_nFilterOrder, (m_dFilterUpperFrequency + m_dFilterLowerFrequency)/2, m_dFilterUpperFrequency - m_dFilterLowerFrequency);
	pButterworth->LowPass( m_nFilterOrder, m_dFilterUpperFrequency);

	// first do forward pass
	long lResult = pButterworth->Process(pCaller, pSaDoc, nProgress, ++nLevel);
	nLevel = (short int)LOWORD(lResult);
	if ((nLevel == PROCESS_CANCELED))
	{
		nProgress = HIWORD(lResult);
		CancelProcess(); // set your own cancel flag
		delete pButterworth;
		return MAKELONG(nLevel, nProgress);
	}

	// start processing loop
	for(DWORD dwIntegerPitchPeriod = 22; dwIntegerPitchPeriod < 480; dwIntegerPitchPeriod+= 5 /*dwIntegerPitchPeriod/16*/)
	{
		double sumXnXnt = 0.;
		// initialize to ~ -30dB noise to mask low amplitude signals (& kill divide by 0)
		double sumXnXn = 1024*1024*10;
		double sumXntXnt = 1024*1024*10;

		DWORD dwCorrelationLength = dwIntegerPitchPeriod > 40 ? dwIntegerPitchPeriod : 70;      
		DWORD dwDataSamples = dwDataSize/wSmpSize;

		for(DWORD dwDataPos = 0; dwDataPos < dwDataSamples; dwDataPos+= 4)
		{
			// set progress bar
			//SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
			if (IsCanceled()) 
			{
				delete pButterworth;
				return Exit(PROCESS_CANCELED); // process canceled
			}

			DWORD dwBlockEnd = dwDataPos + dwIntegerPitchPeriod + dwCorrelationLength - 1;
			DWORD dwBlockStart = dwDataPos;

			if (dwBlockEnd < dwDataSamples)
			{
				double Xn = ReadDataBlock( pButterworth, dwBlockStart, dwBlockEnd, dwDataPos + dwCorrelationLength - 1, wSmpSize);
				double Xnt = ReadDataBlock( pButterworth, dwBlockStart, dwBlockEnd, dwDataPos + dwIntegerPitchPeriod + dwCorrelationLength - 1, wSmpSize);

				sumXnXn += Xn*Xn;
				sumXnXnt += Xn*Xnt;
				sumXntXnt += Xnt*Xnt;
			}

			short sResult = 0;

			if(dwDataPos > dwCorrelationLength)
			{
				double crossCorrelation = sumXnXnt/sqrt(sumXnXn*sumXntXnt);

				sResult = crossCorrelation > 0 ? short(crossCorrelation*1000. + 0.5) : short(0);

				double Xn = ReadDataBlock( pButterworth, dwBlockStart, dwBlockEnd, dwDataPos, wSmpSize);
				double Xnt = ReadDataBlock( pButterworth, dwBlockStart, dwBlockEnd, dwDataPos + dwIntegerPitchPeriod, wSmpSize);

				sumXnXn -= Xn*Xn;
				sumXnXnt -= Xn*Xnt;
				sumXntXnt -= Xnt*Xnt;
			}

			Write(&sResult, sizeof(short));
		}
	}

	delete pButterworth;

	nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
	// close the temporary file and read the status
	CloseTempFile(); // close the file
	EndProcess((nProgress >= 95)); // end data processing
	EndWaitCursor();
	SetDataReady();
	return MAKELONG(nLevel, nProgress);
}

static int ReadDataBlock( CDataProcess *pSource, DWORD dwStart, DWORD dwStop, DWORD dwPos, int wSmpSize)
{
	BYTE* pSourceData = (BYTE*) pSource->GetProcessedDataBlock(dwStart*wSmpSize, (dwStop - dwStart + 1)*wSmpSize);

	pSourceData += (dwPos - dwStart)*wSmpSize;

	int nData;
	if (wSmpSize == 1) // 8 bit data
	{
		if (!pSourceData)
		{
			ASSERT(FALSE);
			TRACE(_T("Failed reading source data\n"));
			return 0;
		}
		BYTE bData;
		bData = *((BYTE*)pSourceData); // data range is 0...255 (128 is center)
		nData = bData - 128;
		nData *= 256;
	}
	else                  // 16 bit data
	{
		if (!pSourceData)
		{
			ASSERT(FALSE);
			TRACE(_T("Failed reading source data\n"));
			return 0;
		}
		nData = *((short int*)pSourceData);
	}

	return nData;
}

