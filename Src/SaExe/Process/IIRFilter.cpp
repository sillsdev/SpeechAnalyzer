// Butterworth.cpp: implementation of the CProcessButterworth class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AppDefs.h"
#include "isa_doc.h"
#include "sa_w_adj.h"
#include "StringUtils.h"
#include "Butterworth.h"
#include "Hilbert.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const double pi = 3.14159265358979323846264338327950288419716939937511;

CProcessIIRFilter::CProcessIIRFilter(BOOL bDstWBench)
{
    m_pSourceProcess = NULL;
    m_bSrcWBenchProcess = TRUE;
    m_bDstWBenchProcess = bDstWBench;
    m_bFilterFilter = false;
    m_nFilterFilterSilence = DEFAULT_FILTER_FILTER_SILENCE_SAMPLES;
}

CProcessIIRFilter::~CProcessIIRFilter()
{
}

static void StoreWaveData(int nData, int wSmpSize, void * pTargetData)
{
    // save data
    if (wSmpSize == 1)   // 8 bit data
    {
        if (nData > 127)
        {
            nData = 127;
        }
        if (nData < -128)
        {
            nData = -128;
        }
        BYTE bData = (BYTE)(nData + 128);
        *((BYTE *)pTargetData) = bData;
    }
    else                  // 16 bit data
    {
        if (nData > 32767)
        {
            nData = 32767;
        }
        if (nData < -32768)
        {
            nData = -32768;
        }
        *((unsigned short *)pTargetData) = (unsigned short)nData;
    }
}

long CProcessIIRFilter::Process(void * pCaller, ISaDoc * pDoc, int nProgress, int nLevel)
{
    TRACE("IIRFilter::Process %d %d\n",nProgress,nLevel);

    if (IsCanceled())
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }

    // check if nested workbench processes
    int nOldLevel = nLevel; // save original level
    IProcess * pLowerProcess = m_pSourceProcess;
    if (pLowerProcess!=NULL)
    {
        TRACE("process lower\n");
        // there is at least one source processes to process first
        long lResult = pLowerProcess->Process(pCaller, pDoc, nProgress, ++nLevel);
        nLevel = (short int)LOWORD(lResult);
        nProgress = HIWORD(lResult);
    }

    if ((nLevel == nOldLevel) && (IsDataReady()))
    {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    else
    {
        SetDataInvalid();
    }

    if (nLevel < 0)   // memory allocation failed or previous processing error
    {
        if ((nLevel == PROCESS_CANCELED))
        {
            CancelProcess();    // set your own cancel flag
        }
        return MAKELONG(nLevel, nProgress);
    }

    TRACE("start process\n");
    // start process
    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSWBLP))	// start data processing
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // create the temporary file
    if (!CreateTempFile(L"IIR"))							
    {
		// creation error - end processing
        EndProcess();
        EndWaitCursor();
        SetDataReady(FALSE);
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    m_bReverse = (!m_zReverseTransform.IsIdentity());

    // start processing loop
    if (!m_bReverse)
    {
		ProcessForward( pDoc, pLowerProcess, nProgress, nLevel);
    }
    else
    {
		ProcessReverse( pCaller, pDoc, nProgress, nLevel);
    }

	// calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel); 
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    EndProcess((nProgress >= 95)); // end data processing
    EndWaitCursor();
    SetDataReady();

	TRACE("end iifilter\n");
    //Dump("iirfilter end process");

    return MAKELONG( nLevel, nProgress);
}

long CProcessIIRFilter::ProcessForward( ISaDoc * pDoc, IProcess * pLowerProcess, int & nProgress, int & nLevel)
{
    TRACE("forward\n");

	m_bReverse = FALSE;

    DWORD dwDataPos = 0;	// data position pointer

    WORD wDstSmpSize = WORD(m_bDstWBenchProcess ? pDoc->GetSampleSize() : sizeof(short));
	TRACE("wDstSmpSize=%d\n",wDstSmpSize);

    DWORD dwBufferSize = GetBufferSize();
	TRACE("dwBufferSize=%d\n",dwBufferSize);

    // get source data size
    DWORD dwDataSize;
    if (pLowerProcess!=NULL)
    {
        dwDataSize = pLowerProcess->GetProcessedWaveDataSize(pDoc);
    }
    else
    {
        dwDataSize = pDoc->GetDataSize(); // size of raw data
    }
	TRACE("dwDataSize=%d\n",dwDataSize);

    WORD wSrcSmpSize = WORD(m_bSrcWBenchProcess ? pDoc->GetSampleSize() : sizeof(short));

    if (wDstSmpSize > wSrcSmpSize)
    {
        dwBufferSize /= wDstSmpSize/wSrcSmpSize;
    }

	int count = 0;
    HPSTR pTargetData = NULL;		// pointers to target data
    m_nMinValue = INT_MAX;
    m_nMaxValue = INT_MIN;

    while (dwDataPos < dwDataSize)
    {
        // set progress bar
        if (IsCanceled())
        {
            return Exit(PROCESS_CANCELED);    // process canceled
        }

        pTargetData = m_lpBuffer;
        DWORD dwBlockEnd = dwDataPos + dwBufferSize;
        if (dwBlockEnd > dwDataSize)
        {
            dwBlockEnd = dwDataSize;
        }
        DWORD dwBlockStart = dwDataPos;
		DWORD dwLength = dwBlockEnd-dwBlockStart;
        while (dwDataPos < dwBlockEnd)
        {
            int nData = ReadSourceData( dwDataPos, wSrcSmpSize, pDoc);
            dwDataPos += wSrcSmpSize;
            // process data
            nData = round(m_zForwardTransform.Tick(double(nData)));
            if (nData > m_nMaxValue)
            {
                m_nMaxValue = nData;
            }
            if (nData < m_nMinValue)
            {
                m_nMinValue = nData;
            }
            StoreWaveData(nData, wDstSmpSize, pTargetData);
            pTargetData += wDstSmpSize;
        }
        WriteDataBlock( dwBlockStart, m_lpBuffer, dwLength*wDstSmpSize/wSrcSmpSize, 1);
		count += dwLength*wDstSmpSize/wSrcSmpSize;

        dwDataPos = dwBlockEnd;
    }
	TRACE("wrote %d values\n",count);

    if (m_bFilterFilter)
    {
		TRACE("filtering\n");
        // Append some silence to handle phase lag
        DWORD dwSettlingSize = FilterFilterSilenceSamples()*wSrcSmpSize;
        while (dwDataPos < (dwDataSize + dwSettlingSize))
        {
            pTargetData = m_lpBuffer;
            DWORD dwBlockEnd = dwDataPos + dwBufferSize;
            if (dwBlockEnd > dwDataSize + dwSettlingSize)
            {
                dwBlockEnd = dwDataSize + dwSettlingSize;
            }
            DWORD dwBlockStart = dwDataPos;
			DWORD dwLength = dwBlockEnd-dwBlockStart;
            while (dwDataPos < dwBlockEnd)
            {
                // process data
                dwDataPos+= wSrcSmpSize;
                int nData = round(m_zForwardTransform.Tick(0.0));
                StoreWaveData( nData, wDstSmpSize, pTargetData);
                pTargetData += wDstSmpSize;
            }
            WriteDataBlock(dwBlockStart, m_lpBuffer, dwLength*wDstSmpSize/wSrcSmpSize, 1);
			count += dwLength*wDstSmpSize/wSrcSmpSize;
        }
    }
	TRACE("wrote %d values\n",count);
    return MAKELONG( nLevel, nProgress);
}

long CProcessIIRFilter::ProcessReverse( void * pCaller, ISaDoc * pDoc, int & nProgress, int & nLevel)
{
    TRACE("reverse\n");
	int count = 0;

    DWORD dwDataPos = 0;	// data position pointer

    WORD wDstSmpSize = WORD(m_bDstWBenchProcess ? pDoc->GetSampleSize() : sizeof(short));
	TRACE("wDstSmpSize=%d\n",wDstSmpSize);

    DWORD dwBufferSize = GetBufferSize();
	TRACE("dwBufferSize=%d\n",dwBufferSize);

    // process in reverse
    // first do forward pass
    CProcessIIRFilter forwardPass( m_bDstWBenchProcess);
    forwardPass.SetSourceProcess( m_pSourceProcess, m_bSrcWBenchProcess);
    forwardPass.m_zForwardTransform *= m_zForwardTransform;
    forwardPass.SetFilterFilter(true);
    forwardPass.SetFilterFilterSilenceSamples(FilterFilterSilenceSamples());

    long lResult = forwardPass.Process( pCaller, pDoc, nProgress, ++nLevel);

    nLevel = (short int)LOWORD(lResult);
    nProgress = HIWORD(lResult);
    if ((nLevel == PROCESS_CANCELED))
    {
        CancelProcess(); // set your own cancel flag
        return MAKELONG(nLevel, nProgress);
    }

    m_pSourceProcess = &forwardPass;

    WORD wSmpSize = wDstSmpSize;
    DWORD dwDataSize = 0;
    if (forwardPass.m_pSourceProcess!=NULL)
    {
        dwDataSize = forwardPass.m_pSourceProcess->GetNumSamples(pDoc) * wSmpSize;
    }
    else
    {
        dwDataSize = (forwardPass.GetDataSize()-FilterFilterSilenceSamples())*wSmpSize;
    }
	TRACE("dwDataSize=%d\n",dwDataSize);

    dwDataPos = forwardPass.GetDataSize() * wSmpSize;
	TRACE("forwardPass.GetDataSize()=%d\n",forwardPass.GetDataSize());
	TRACE("dwDataPos=%d\n",dwDataPos);

    while (dwDataPos > dwDataSize)
    {
        // process silence
        dwDataPos-= wSmpSize;
        int nData = ReadSourceData(dwDataPos, wSmpSize, pDoc);
        // process data
        nData = round(m_zReverseTransform.Tick(double(nData)));
    }

    HPSTR pTargetData = NULL;		// pointers to target data
    m_nMinValue = INT_MAX;
    m_nMaxValue = INT_MIN;

    // start processing loop
    while (dwDataPos > 0)
    {
        if (IsCanceled())
        {
            m_pSourceProcess = forwardPass.m_pSourceProcess;
            return Exit(PROCESS_CANCELED); // process canceled
        }

        DWORD dwBlockEnd = dwDataPos - dwBufferSize;
        if (dwDataPos < dwBufferSize)
        {
            dwBlockEnd = 0;
            dwBufferSize = dwDataPos;
        }
		TRACE("dwBlockEnd=%d\n",dwBlockEnd);

        DWORD dwBlockStart = dwBlockEnd;
		TRACE("dwBlockStart=%d\n",dwBlockStart);

        pTargetData = m_lpBuffer + dwBufferSize;
            
		//TRACE("dwDataPos=%d\n",dwDataPos);
        while (dwDataPos > dwBlockEnd)
        {
            dwDataPos-= wSmpSize;
            int nData = ReadSourceData( dwDataPos, wSmpSize, pDoc);

            // process data
            nData = round(m_zReverseTransform.Tick(double(nData)));

            if (nData > m_nMaxValue)
            {
                m_nMaxValue = nData;
            }
            if (nData < m_nMinValue)
            {
                m_nMinValue = nData;
            }

            pTargetData -= wSmpSize;
            StoreWaveData( nData, wSmpSize, pTargetData);
        }
        WriteDataBlock( dwBlockStart, m_lpBuffer, dwBufferSize, 1);
		count += dwBufferSize;

        dwDataPos = dwBlockEnd;
    }

    // Preserve source setting so that we can use it
    m_pSourceProcess = forwardPass.m_pSourceProcess;
    //Dump("butterworth end reverse");
	TRACE("wrote %d values\n",count);
    return MAKELONG( nLevel, nProgress);
}

void CProcessIIRFilter::SetSourceProcess(IProcess * pSourceProcess, BOOL bWBenchProcess)
{

    m_bSrcWBenchProcess = bWBenchProcess;
    if (m_pSourceProcess != pSourceProcess)
    {
        SetDataInvalid();
    }
    m_pSourceProcess = pSourceProcess;
}

int CProcessIIRFilter::ReadSourceData(DWORD dwDataPos, int wSmpSize, ISaDoc * pDoc)
{
    IProcess * pLowerProcess = m_pSourceProcess;
    if (pLowerProcess==NULL)
    {
        pLowerProcess = pDoc->GetAdjust();          // This is the default node
    }

    // read data
    int nData;
    if (wSmpSize == 1)
    {
        // 8 bit data
        void * pSourceData = pLowerProcess->GetProcessedObject(  pDoc->GetProcessFilename(),
																 pDoc->GetSelectedChannel(),
																 pDoc->GetNumChannels(),
																 pDoc->GetSampleSize(),
																 dwDataPos,
																 1,
																 m_bReverse);
        if (pSourceData==NULL)
        {
            ASSERT(FALSE);
            TRACE(_T("Failed reading source data\n"));
            return 0;
        }
        BYTE bData;
        bData = *((BYTE *)pSourceData); // data range is 0...255 (128 is center)
        nData = bData - 128;
    }
    else
    {
        // 16 bit data
        void * pSourceData = pLowerProcess->GetProcessedObject( pDoc->GetProcessFilename(),
																pDoc->GetSelectedChannel(),
																pDoc->GetNumChannels(),
																pDoc->GetSampleSize(),
																dwDataPos>>1,
																2,
																m_bReverse);
        if (pSourceData==NULL)
        {
            ASSERT(FALSE);
            TRACE(_T("Failed reading source data\n"));
            return 0;
        }
        nData = *((short int *)pSourceData);
    }

    return nData;
}

void CProcessIIRFilter::SetFilterFilterSilenceSamples(int forwardSamples)
{
    m_nFilterFilterSilence = forwardSamples > 0 ? forwardSamples : 1;
}

int CProcessIIRFilter::FilterFilterSilenceSamples()
{
    return m_nFilterFilterSilence;
}

CZTransform CProcessIIRFilter::GetForward()
{
    return m_zForwardTransform;
}

void CProcessIIRFilter::SetFilterFilter(bool bSet)
{
    m_bFilterFilter = bSet;
}

int CProcessIIRFilter::round(double value)
{
    return (value >= 0.) ? int(value + 0.5) : int(value - 0.5);
}

