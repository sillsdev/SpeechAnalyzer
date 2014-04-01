/////////////////////////////////////////////////////////////////////////////
// sa_p_spg.cpp:
// Implementation of the CProcessSpectrogram class.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.5Test11.1A
//        RLJ Changed code for CProcessSpectrogram::GetNextIndex to get
//            default Spectrogram parameter values from either
//            m_spectroAParmDefaults (for SpectrogramA) or from
//            m_spectroBParmDefaults (for SpectrogramB), rather than from
//            hard-coded values (as previously).
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Process.h"
#include "sa_p_spg.h"
#include "sa_p_sfmt.h"
#include "isa_doc.h"
#include "sa_view.h"
#include "math.h"
#include "SpectroParm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessSpectrogram
// class to calculate spectrogram for wave data. The class creates an object
// of the class Spectrogram that does the calculation. The results are stored
// in different temporary files (see base class). Spectrogram needs the whole
// raw data to calculate in memory, so the class CProcessSpectrogram creates
// a temporary second buffer, into which it copies all the raw data needed
// for the calculation.

/***************************************************************************/
// CProcessSpectrogram::CProcessSpectrogram Constructor
/***************************************************************************/
CProcessSpectrogram::CProcessSpectrogram(const CSpectroParm & cSpectroParm, ISaDoc * pDoc, BOOL bRealTime) :
    m_bRealTime(bRealTime), m_pDoc(pDoc)
{
    // create the spectrogram parameter arrays
    m_nWindowWidth = 0;
    m_nWindowHeight = 0;
    m_pSpectroFormants = new CProcessSpectroFormants;
    SetSpectroParm(cSpectroParm);
}

/***************************************************************************/
// CProcessSpectrogram::~CProcessSpectrogram Destructor
/***************************************************************************/
CProcessSpectrogram::~CProcessSpectrogram()
{
    if (m_pSpectroFormants)
    {
        delete m_pSpectroFormants;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CProcessSpectrogram helper functions

/***************************************************************************/
// CProcessSpectrogram::SetSpectroParm Set spectrogram parameters
/***************************************************************************/
void CProcessSpectrogram::SetSpectroParm(const CSpectroParm & cSpectroParm)
{
    m_SpectroParm = cSpectroParm;
    ISaDoc * pDoc = GetDocument();

    // Clip frequency limit to Nyquist limit
    if (m_SpectroParm.nFrequency >= int(pDoc->GetSamplesPerSec()/2))
    {
        m_SpectroParm.nFrequency = pDoc->GetSamplesPerSec()/2 - 1;
    }
}

/***************************************************************************/
// CProcessSpectrogram::Exit Exit on Error
// Standard exit function if an error occurred.
/***************************************************************************/
long CProcessSpectrogram::Exit(int nError)
{
    return CProcess::Exit(nError);
}

/***************************************************************************/
// CProcessSpectrogram::Process Processing spectrogram data
// The processed spectrogram data is stored in a temporary file. To create
// it helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level throughout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word. This
// function needs a pointer to the view instead the pointer to the document
// like other process calls. It calculates spectrogram data.
/***************************************************************************/
long CProcessSpectrogram::Process(void * pCaller, ISaDoc * pDoc, CSaView * pView, int nWidth, int /*nHeight*/, int nProgress, int nLevel)
{

    DWORD wSmpSize = (DWORD)(pDoc->GetSampleSize());
    // check canceled
    if (IsCanceled())
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    // get area boundaries
    BOOL bRealTime = m_bRealTime;
    DWORD dwDataStart, dwDataLength;
    if (IsStatusFlag(KEEP_AREA))
    {
        // old boundaries are to keep
        SetStatusFlag(KEEP_AREA, FALSE);
        dwDataStart = GetAreaPosition();
        dwDataLength = GetAreaLength();
    }
    else
    {
        // get new area boundaries from window position
        dwDataStart = (DWORD) pView->GetDataPosition(0);	// data index of first sample to display
        dwDataLength = pView->GetDataFrame();				// number of data points to display
    }

    // check if data ready
    if (IsDataReady())
    {
        if (!bRealTime)
        {
            return MAKELONG(--nLevel, nProgress);
        }

        if (IsStatusFlag(MAX_RESOLUTION))
        {
            if (GetAreaPosition() <= dwDataStart)
                if (GetAreaPosition() + GetAreaLength() >= dwDataStart + dwDataLength)
                {
                    return MAKELONG(--nLevel, nProgress);
                }
        }
    }

    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSSPG))   // memory allocation failed
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    if (!CreateTempFile(_T("SPG")))   // creating error
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        SetDataInvalid();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }
    // set up spectrogram parameters
    const CSpectroParm * pSpectroParm = & GetSpectroParm(); // get pointer to spectrogram parameters
    
	UINT nBlockAlign = pDoc->GetBlockAlign(true);
    SSpectrogramSettings SpgmSetting;
    SSigParms Signal;
    dspError_t Err;
    SpgmSetting.LwrFreq = (float)0;
    SpgmSetting.UprFreq = (float)(pDoc->GetSamplesPerSec()/2.0);
    SpgmSetting.preEmphSw = true;
    SpgmSetting.Bandwidth = pSpectroParm->Bandwidth();
    Signal.SmpRate = pDoc->GetSamplesPerSec();
    SpgmSetting.FFTLength = (USHORT)(2 << USHORT(ceil(log(float( CDspWin::CalcLength( SpgmSetting.Bandwidth, Signal.SmpRate, ResearchSettings.m_cWindow.m_nType))/log(2.0) + 0.0))));

    {
        int minSpectraInterval = wSmpSize*(NyquistSpectraInterval(pDoc->GetSamplesPerSec())/2 + 1);
        BOOL bRealTime = m_bRealTime;
        if (!bRealTime)
        {
            // Increase Resolution for high quality snapshot
            int maxWidth = (dwDataLength / minSpectraInterval + 1)*4;
            nWidth = nWidth > maxWidth ? maxWidth : nWidth;
            SpgmSetting.FFTLength *= 4;
        }
        else
        {
            // Just do the whole file
            dwDataStart = 0;
            dwDataLength = pDoc->GetDataSize();
            nWidth = dwDataLength / minSpectraInterval + 1;
        }
        SetStatusFlag(MAX_RESOLUTION);
    }

    // save the frame
    SetArea(dwDataStart, dwDataLength);

    SpgmSetting.FreqCnt = USHORT(SpgmSetting.FFTLength/2 + 1);
    int nHeight = SpgmSetting.FreqCnt;
    SpgmSetting.fmntTrackSw = false;      // will use LPC formant tracking later

    // calculate start and end positions
    // dwDataStart contains the offset to the first displayed byte and dwDataLength
    // the number of displayed bytes in the raw data graph. wHalfCalcWindow is the
    // half of the size of the calculation window in the spectrogram calculation.
    // dwBufferStart is the offset to the first raw data byte in the special raw data
    // buffer. It is normally dwDataStart - wHalfCalcWindow. dwBufferLength is the
    // length in bytes of the special raw data buffer. It is normally dwDataLength +
    // twice wHalfCalcWindow. The wide band version of this calculation has also
    // to be done, because if formants tracks are on, we need to check also on the
    // wide band results, if there is enough data to calculate (spectrogram doesn't
    // do that).

    WORD wHalfCalcWindow = (WORD)(nBlockAlign * ((WORD)CDspWin::CalcLength(SpgmSetting.Bandwidth, pDoc->GetSamplesPerSec(), ResearchSettings.m_cWindow.m_nType) / 2));

    double fSpectraInterval = (dwDataLength/wSmpSize)/double(nWidth);

    // Spectrogram class will choke if it gets too much data in one batch.
    DWORD dwWidth = (DWORD) floor((GetProcessBufferSize() - 2*wHalfCalcWindow)/ wSmpSize / fSpectraInterval);
    if (dwWidth > ((DWORD)nWidth / 5))
    {
        dwWidth = nWidth / 5;
    }

    SpgmSetting.SpectBatchLength = (USHORT)(dwWidth & ~1);  // must be even ?

    // create the spectrogram object and initialize spectrogram parameters
    CSpectrogram * pSpectrogram = NULL;
    BOOL bAliased = TRUE;

    Signal.SmpDataFmt = char((!pDoc->Is16Bit()) ? PCM_UBYTE: PCM_2SSHORT);
    SetStatusFlag(~MAX_RESOLUTION, FALSE);					// reset status

    UINT nSpectSize = sizeof(uint8) * (UINT)nHeight;

    // Generate spectrogram.
    for (int wLoop = 0; wLoop < nWidth; wLoop = (wLoop + SpgmSetting.SpectBatchLength))
    {
        // now fill up the special raw data buffer
        DWORD dwDataPos = dwDataStart + round(wLoop*fSpectraInterval)*wSmpSize;
        DWORD dwBufferStart = 0;

        if (dwDataPos > (DWORD)wHalfCalcWindow)
        {
            dwBufferStart = dwDataPos - (DWORD)wHalfCalcWindow;
        }

        SpgmSetting.SigBlkOffset = (dwDataPos - dwBufferStart) / nBlockAlign;
        Signal.Start = pDoc->GetWaveData(dwBufferStart, TRUE);

        DWORD dwBufferLength = (DWORD) ceil((SpgmSetting.SpectBatchLength)*wSmpSize*fSpectraInterval) + (DWORD)2*wHalfCalcWindow + wSmpSize;

        if ((dwBufferStart + dwBufferLength) > pDoc->GetDataSize())
        {
            dwBufferLength = pDoc->GetDataSize() - dwBufferStart;
            SpgmSetting.SpectBatchLength = USHORT(nWidth - wLoop);
        }

        Signal.Length = dwBufferLength / nBlockAlign;
        SpgmSetting.SigBlkLength = (DWORD) floor(SpgmSetting.SpectBatchLength*fSpectraInterval);
        SpgmSetting.SpectCnt = SpgmSetting.SpectBatchLength;

        Err = CSpectrogram::CreateObject(&pSpectrogram, SpgmSetting, Signal);
        if (Err)
        {
            return Exit(PROCESS_ERROR);    // error, setup failed
        }

        int nXIndex = 0;

        dspError_t Status = pSpectrogram->Generate();
        if (Status < DONE)
        {
            delete pSpectrogram; // delete the spectrogram object
            return Exit(PROCESS_ERROR); // error, process failed
        }
        // write the processed spectrogram data block into the temporary file
        int nBatchWidth = SpgmSetting.SpectBatchLength;
        if ((nWidth - wLoop) < nBatchWidth)
        {
            nBatchWidth = (nWidth - wLoop) & ~1;    // must be even
        }
        // set progress bar
        SetProgress(nProgress + (int)(100 * (DWORD)(wLoop + nBatchWidth) / nWidth / (DWORD)nLevel));
        if (IsCanceled())
        {
            // spectrogram canceled
            delete pSpectrogram; // delete the spectrogram object
            return Exit(PROCESS_CANCELED); // process canceled
        }
        try
        {
            for (int nLoop = 0; nLoop < nBatchWidth; nLoop++)
            {
                dspError_t err;
                uint8 * pPower = pSpectrogram->ReadPowerSlice(&err, (WORD)nXIndex++);
                if (!pPower)
                {
                    delete pSpectrogram; // delete the spectrogram object
                    return Exit(PROCESS_ERROR); // error, writing failed
                }
                // save powers in the file
                Write(pPower, (DWORD)nSpectSize);
            }
        }
        catch (CFileException e)
        {
            // error writing file
            ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
            delete pSpectrogram; // delete the spectrogram object
            return Exit(PROCESS_ERROR); // error, writing failed
        }
        bAliased = pSpectrogram->IsAliased();
        delete pSpectrogram; // delete the spectrogram object
    }

    // save aliased flag
    SetStatusFlag(DATA_ALIASED, bAliased);

    nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
    EndProcess((nProgress >= 95)); // end data processing
    EndWaitCursor();
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    m_nWindowWidth = nWidth;   // save window width
    m_nWindowHeight = nHeight; // save window height
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcessSpectrogram::GetSpectroSlice Read spectrogram slice data
// Reads a slice of processed data from the temporary file into the processed
// data buffer and returns the pointer to the data. The returned pointer
// points to a slice of power data (top first). pCaller is a pointer to the
// calling plot and enables this function to get the process index of the
// plot. nIndex is the horizontal index in the spectrogram data (pixel
// number) and formants is a SFormant structure and it puts in there the
// formant data of this slice. The function returns NULL on error.
/***************************************************************************/
void * CProcessSpectrogram::GetSpectroSlice(DWORD dwIndex)
{
    // get window height
    int nHeight = GetWindowHeight();
    // read the data
    size_t sSize = nHeight * sizeof(uint8);
    uint8 * pData = (uint8 *) GetProcessedObject(dwIndex, sSize);
    return pData;
}

/***************************************************************************/
// CProcessSpectrogram::NyquistSpectraInterval  Returns number of samples
// between succesive spectra to avoid aliasing using nyquist theorem
/***************************************************************************/
int CProcessSpectrogram::NyquistSpectraInterval(double dSourceSamplesPerSec)
{
    return int(0.5 * double(dSourceSamplesPerSec)/SpectraBandwidth() - 1);
}

/***************************************************************************/
// CProcessSpectrogram::SpectraBandwidth return selected SpectraBandwidth
/***************************************************************************/
int CProcessSpectrogram::SpectraBandwidth()
{
    const CSpectroParm * pSpectroParm =  & GetSpectroParm(); // get pointer to spectrogram parameters
    return (int) pSpectroParm->Bandwidth();
}

ISaDoc * CProcessSpectrogram::GetDocument() const
{
    return m_pDoc;
}

const CSpectroParm & CProcessSpectrogram::GetSpectroParm() const
{
    // return reference to spectrogram parameters structure
    return m_SpectroParm;
}

int CProcessSpectrogram::GetWindowWidth() const
{
    // return processed window width
    return m_nWindowWidth;
}
int CProcessSpectrogram::GetWindowHeight() const
{
    // return processed window height
    return m_nWindowHeight;
}

CProcessSpectroFormants * CProcessSpectrogram::GetFormantProcess()
{
    return m_pSpectroFormants;
}
