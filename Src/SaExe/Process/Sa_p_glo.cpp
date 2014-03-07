/////////////////////////////////////////////////////////////////////////////
// sa_p_glo.cpp:
// Implementation of the CProcessGlottis class.
// Author: Alec Epting
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"
#include "sa_p_glo.h"
#include "isa_doc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessGlottis
// class to calculate glottal waveform for speech data. The class creates an
// object of the class CGlottis that does the calculation.

/////////////////////////////////////////////////////////////////////////////
// CProcessGlottis construction/destruction/creation

/***************************************************************************/
// CProcessGlottis::CProcessGlottis Constructor
/***************************************************************************/
CProcessGlottis::CProcessGlottis()
{
}

/***************************************************************************/
// CProcessGlottis::~CProcessGlottis Destructor
/***************************************************************************/
CProcessGlottis::~CProcessGlottis()
{
}


/***************************************************************************/
// CProcessGlottis::Process Processing glottal waveform data
// The processed glottal waveform is stored in the process data buffer. There is
// no temporary file created and no process bar displayed (just the wait
// curser). The level tells which processing level this process has been
// called, start process start on which processing percentage this process
// starts (for the progress bar of other processed that call this one). The
// return value returns the highest level througout the calling queue, or -1
// in case of an error in the lower word of the long value and the end
// process progress percentage in the higher word. This function needs a
// pointer to the view instead the pointer to the document like other process
// calls. It calculates glottal waveform data.
/***************************************************************************/
#include "lpc.h"

long CProcessGlottis::Process(void * pCaller, ISaDoc * pDoc, int nProgress, int nLevel)
{

    //TRACE(_T("Process: CProcessGlottis\n"));
    if (IsCanceled())
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    if (IsDataReady())
    {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }


    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller))   // memory allocation failed
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // create the temporary file
    if (!CreateTempFile(_T("GLO")))   // creating error
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    DWORD wSmpSize = pDoc->GetSampleSize();   //compute sample size in bytes
    DWORD dwWaveSize = pDoc->GetDataSize();
    HPSTR pBlockStart;
    DWORD dwBlockStart = 0;
    HPSTR pFrame = NULL;
    short * pProcData = (short *)m_lpBuffer;
    DWORD dwProcDataCount = 0;

    // Construct an LPC object for vocal tract modeling.
    SSigParms Signal;
    SLPCSettings LpcSetting;
    CLinPredCoding * pLpcObject = NULL;
    SLPCModel * pLpcModel;
    dspError_t Err;

    if (wSmpSize == 1)
    {
        Signal.SmpDataFmt = PCM_UBYTE;			//unsigned 8-bit
    }
    else if (wSmpSize == 2)
    {
        Signal.SmpDataFmt = PCM_2SSHORT;		//2's complement 16 bit
    }
    Signal.SmpRate = pDoc->GetSamplesPerSec();  //set sample rate
    DWORD dwBufferSize = GetBufferSize();		// data buffer size
    Signal.Length = dwBufferSize / wSmpSize;
    LpcSetting.Process.Flags = PRE_EMPHASIS | REFL_COEFF | PRED_COEFF | MEAN_SQ_ERR | RESIDUAL ;
    LpcSetting.nMethod = LPC_COVAR_LATTICE;     //use covariance LPC analysis
    LpcSetting.nMethod = LPC_AUTOCOR;			//use autocorrelation LPC analysis
    LpcSetting.nOrder = (uint8)(Signal.SmpRate/1000 + 4);  //rule-of-thumb from Markel and Gray
    //!!order for unvoiced speech?
    LpcSetting.nOrder = 44;  // sufficient for modeling unvoiced speech

    LpcSetting.nFrameLen = (unsigned short)((double)Signal.SmpRate*.020 + 0.5); //20 ms
    DWORD dwFrameSize = (DWORD)(LpcSetting.nFrameLen*wSmpSize);

    m_nMaxValue = 0;
    int nFrameIntervalSamples = LpcSetting.nFrameLen - 2*LpcSetting.nOrder;
    DWORD dwFrameInterval = (DWORD)((nFrameIntervalSamples)*wSmpSize);
    DWORD dwLastOffset = ((dwWaveSize-(dwFrameSize-dwFrameInterval))/dwFrameInterval-1)*dwFrameInterval;

    //!!assumes PCM_READ_BUFFER greater than predictor delay
    int nLeadPadding = LpcSetting.nOrder;
    for (unsigned short i = 0; i < nLeadPadding; i++)
    {
        *pProcData++ = 0;    //pad output to account for predictor delay
        //*pProcData++ = (short)((unsigned char)pFrame[i]-128);
        dwProcDataCount++;
    }

    for (DWORD dwWaveOffset = 0; dwWaveOffset <= dwLastOffset; dwWaveOffset += dwFrameInterval)
    {
        if (dwWaveOffset == 0)
        {
            dwBlockStart = dwWaveOffset;
            pBlockStart = pDoc->GetWaveData(dwBlockStart, TRUE); // get pointer to data block
            if (!pBlockStart)   // reading failed
            {
                EndProcess(); // end data processing
                EndWaitCursor();
                SetDataInvalid();
                return MAKELONG(-1, nProgress);
            }
            Signal.Start = pBlockStart;
            Err = CLinPredCoding::CreateObject(&pLpcObject, LpcSetting, Signal);
            pFrame = pBlockStart;
        }
        else if (dwWaveOffset + dwFrameSize + wSmpSize > dwBlockStart + dwBufferSize)
        {
            dwBlockStart = dwWaveOffset - wSmpSize;
            pBlockStart = pDoc->GetWaveData(dwBlockStart, TRUE); // get pointer to data block

            if (!pBlockStart)   // reading failed
            {
                EndProcess(); // end data processing
                EndWaitCursor();
                SetDataInvalid();
                return MAKELONG(-1, nProgress);
            }
            pFrame = pBlockStart + wSmpSize;
        }

        Err = pLpcObject->GetLpcModel(&pLpcModel, (void *)pFrame);
        for (int i = 0; i < nFrameIntervalSamples; i++)
        {
            //dwSum += pLpcModel->pResidual[i];
            //*pProcData++ = dwSum;
            //*pProcData = PrevData + pLpcModel->pResidual[i];
            //PrevData = *pProcData++;
            //*pProcData++ = pLpcModel->pPredValue[i];
            *pProcData++ = (SHORT) pLpcModel->pResidual[i];
            dwProcDataCount++;

            // set progress bar
            SetProgress(nProgress + (int)(100 * dwWaveOffset / dwWaveSize / (DWORD)nLevel));
            if (IsCanceled())
            {
                EndProcess(); // end data processing
                EndWaitCursor();
                SetDataInvalid();
                return MAKELONG(PROCESS_CANCELED, nProgress);
            }
            if ((dwProcDataCount >= dwBufferSize / 2) ||
                    (dwWaveOffset == dwLastOffset && (i+1) == pLpcModel->nResiduals))   // processed data buffer is full or processing finished
            {
                // write the processed data block
                try
                {
                    Write(m_lpBuffer, (UINT)dwProcDataCount * 2);
                }
                catch (CFileException e)
                {
                    AfxMessageBox(IDS_ERROR_WRITETEMPFILE, MB_OK | MB_ICONEXCLAMATION, 0); // display message
                    EndProcess(); // end data processing
                    EndWaitCursor();
                    SetDataInvalid();
                    return MAKELONG(-1, nProgress);
                }
                dwProcDataCount = 0; // reset counter
                pProcData = (short *)m_lpBuffer; // reset pointer to begin of processed data buffer
            }
            m_nMaxValue = max(m_nMaxValue, abs((SHORT)pLpcModel->pResidual[i]));
        }
        pFrame += dwFrameInterval;
    }
    if (pLpcObject)
    {
        delete pLpcObject;
    }

    int nTrailPadding = (dwWaveSize - dwLastOffset - dwFrameInterval)/wSmpSize - nLeadPadding;
    for (int i = 0; i < nTrailPadding; i++)
    {
        *pProcData++ = 0;    //pad output to account for predictor delay
        dwProcDataCount++;
    }
    if (dwProcDataCount)
    {
        try
        {
            Write(m_lpBuffer, (UINT)dwProcDataCount * 2);
        }
        catch (CFileException e)
        {
            AfxMessageBox(IDS_ERROR_WRITETEMPFILE, MB_OK | MB_ICONEXCLAMATION, 0); // display message
            EndProcess(); // end data processing
            EndWaitCursor();
            SetDataInvalid();
            return MAKELONG(-1, nProgress);
        }
    }

    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    EndProcess((nProgress >= 95)); // end data processing
    EndWaitCursor();
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

