/////////////////////////////////////////////////////////////////////////////
// sa_p_fmt.cpp:
// Implementation of the CProcessFormants class.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
//
// Revision History
//   06/15/2000
//       RLJ Changed Formant Frequency data for male speakers from that of
//           Peterson & Barney (1952) to Hillenbrand, Getty, Clark, Wheeler (1995).
//           Old data was for F1, F2, and F3. New data is for F1, F2, F3, and F4.
//           F4 is not currently used, but is included for future use.
//   06/16/2000
//       RLJ Added /e/ and /o/ to F1-F2-F3 chart
//       RLJ Inter/extra-polated to derive hypothetical formant frequency data
//           for F1-F3 for /e/ and /o/ for Women and Children.
//   12/04/2001
//       SDM moved formant tables to param.cpp as part of enabling user definable
//           formant sets
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Process.h"
#include "sa_p_spu.h"
#include "sa_p_fmt.h"

#include "resource.h"
#include "isa_doc.h"
#include "sa_gz3d.h"
#include "math.h"
#include "dsp\spectgrm.h"
#include "dsp\signal.h"
#include "dsp\spectrum.h"
#include "dsp\scale.h"
#include "DlgVowelFormants.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static const SRange MenRange[4] =      {{200,1000},{650,2500},{1500,3200}, {3300,3700}};
static const SRange WomenRange[4] =    {{200,1000},{700,3000},{1850,3500}, {3900,4400}};
static const SRange ChildrenRange[4] = {{350,1100},{900,3400},{1950,3900}, {3700,4700}};

//###########################################################################
// CProcessFormants
// class to calculate formants for wave data. The class creates an object
// of the class Spectrogram that does the calculation. The results are stored
// in a temporary file. Spectrogram needs the whole raw data to calculate in
// memory, so the class CProcessFormants creates a temporary second buffer,
// into which it copies all the raw data needed for the calculation.

/////////////////////////////////////////////////////////////////////////////
// CProcessFormants construction/destruction/creation

/***************************************************************************/
// CProcessFormants::CProcessFormants Constructor
/***************************************************************************/
CProcessFormants::CProcessFormants()
{
    m_bTrack = FALSE;
    m_dwValidFrameCount = 0;
    m_dwFormantFrameCount = 0;
    m_dwFrameStart = UNDEFINED_OFFSET;   // to force processing first time
    m_dwProcStart = UNDEFINED_OFFSET;
    m_dwProcEnd = 0;
    for (int n = 0; n <= MAX_NUM_FORMANTS; n++)
    {
        m_F[n].Mean = m_F[n].StdDev = (float)UNDEFINED_DATA;    // initialize formant statistics
    }
}

/***************************************************************************/
// CProcessFormants::~CProcessFormants Destructor
/***************************************************************************/
CProcessFormants::~CProcessFormants()
{

}

/////////////////////////////////////////////////////////////////////////////
// CProcessFormants helper functions


/***************************************************************************/
// CProcessFormants::Exit Exit on Error
// Standard exit function if an error occured.
/***************************************************************************/
long CProcessFormants::Exit(int nError)
{
    return CProcess::Exit(nError);
}

/***************************************************************************/
// CProcessFormants::Process Processing formant data
// The processed formant data is stored in the process buffer and in a temporary
// file if formant tracking is enabled.
/***************************************************************************/
long CProcessFormants::Process(void * pCaller, ISaDoc * pDoc, BOOL bTrack,
                               DWORD dwFrameStart, DWORD dwFrameSize,
                               SSpectProcSelect SpectraSelected,
                               int nProgress, int nLevel)
{
    //TRACE(_T("Process: CProcessFormants\n"));
    // Check if canceled.
    if (IsCanceled())
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }

    // Check to see if the spectrum needs to be recalculated.
    if (dwFrameStart != m_dwFrameStart ||
            dwFrameSize != m_dwFrameSize ||
            SpectraSelected.bCepstralSpectrum != m_stSpectraProc.bCepstralSpectrum  ||
            SpectraSelected.bLpcSpectrum != m_stSpectraProc.bLpcSpectrum ||
            bTrack != m_bTrack)
    {
        // must reprocess
        SetDataReady(FALSE);
        m_dwFrameStart = dwFrameStart;
        m_dwFrameSize = dwFrameSize;
        m_stSpectraProc.bCepstralSpectrum = SpectraSelected.bCepstralSpectrum;
        m_stSpectraProc.bLpcSpectrum = SpectraSelected.bLpcSpectrum;
    }

    if (IsDataReady() && !m_bTrack)
    {
        return MAKELONG(PROCESS_UNNECESSARY, 100);    // data is already ready
    }

    DWORD nSmpSize = pDoc->GetSampleSize();

    // Process spectrum to get formants.
    SetDataReady(TRUE);
    CProcessSpectrum * pSpectrum = pDoc->GetSpectrum();
    short int nResult = LOWORD(pSpectrum->Process(this, pDoc, dwFrameStart, dwFrameSize, SpectraSelected, nProgress, nLevel+1)); // process data

    // Exit if error has occurred or the spectrum process has been canceled.
    if (nResult < 0)
    {
        SetDataReady(FALSE);
        if (nResult == PROCESS_CANCELED)
        {
            CancelProcess();
        }
        return Exit(nResult);
    }
    // Return if processing has already been done.
    //    if (nResult == PROCESS_UNNECESSARY)
    //       return MAKELONG(--nLevel, nProgress); // data is already ready

    // Start formant processing, allocating a process buffer for formant frame data.
    BeginWaitCursor(); // wait cursor
    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSFMT))   // memory allocation failed
    {
        SetDataReady(FALSE);
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    SFormantFrame * pFormantFrame = pSpectrum->GetFormants();
    BOOL bValidFrame = (pFormantFrame->Formant[1].Lpc.FrequencyInHertz != UNDEFINED_DATA &&
                        pFormantFrame->Formant[2].Lpc.FrequencyInHertz != UNDEFINED_DATA &&
                        pFormantFrame->Formant[3].Lpc.FrequencyInHertz != UNDEFINED_DATA);

    // Reset the formant frame index and count, and initialize formant statistics.
    if (!bTrack || !m_bTrack)
    {
        m_bTrack = FALSE;
        m_dwProcStart = UNDEFINED_OFFSET;
        m_dwProcEnd = 0;
        m_dwFormantFrame = 0;
        m_dwFormantFrameCount = 0;
        m_dwValidFrameCount = 0;
        if (bValidFrame)
        {
            for (int n = 1; n <= 3; n++)
            {
                // initialize mean with each formant frequency
                m_F[n].Mean = pFormantFrame->Formant[n].Lpc.FrequencyInHertz;
                m_F[n].StdDev = 0;
            }
            m_dwValidFrameCount++;
        }
        else
        {
            // indicate mean is undefined
            for (int n = 1; n <= 3; n++)
            {
                m_F[n].Mean = (float)UNDEFINED_DATA;
                m_F[n].StdDev = (float)UNDEFINED_DATA;
            }
        }
    }

    // Get the formant frame data and calculate the frame size.  Resize the process buffer
    // to handle an integral number of frames.
    //    m_nFormantFrameSize = (USHORT)(pSpectrum->GetFormantCount() * sizeof(SFormant));
    m_nFormantFrameSize = sizeof(SFormantFrame);  //!! assumes fixed size structure
    SetProcessBufferSize((GetProcessBufferSize() / m_nFormantFrameSize) * m_nFormantFrameSize);

    // Store the formant frame data.
    BOOL bUpdate = ((dwFrameStart < m_dwProcStart) || (dwFrameStart + dwFrameSize - nSmpSize > m_dwProcEnd));

    if (bUpdate || m_bTrack)
    {
        // new frame has been processed
        if (m_bTrack)
        {
            m_dwFormantFrame++;
        }
        // store it
        BOOL bWriteThru = bTrack;  // always write formant tracks to disk
        if (!StoreFormantFrame(m_dwFormantFrame, pFormantFrame, bWriteThru))
        {
            SetDataReady(FALSE);
            EndProcess();
            EndWaitCursor();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
        // track processed frame boundaries
        m_dwProcStart = min(dwFrameStart, m_dwProcStart);
        m_dwProcEnd = max(dwFrameStart + dwFrameSize - nSmpSize, m_dwProcEnd);
        // update total number of frames
        m_dwFormantFrameCount = m_dwFormantFrame + 1;
        if (bValidFrame)
        {
            // it is valid
            if (m_bTrack)
            {
                // calculate running statistics
                m_dwValidFrameCount++;
                for (int n = 1; n <= 3; n++)
                {
                    double FormantFreq = pFormantFrame->Formant[n].Lpc.FrequencyInHertz;
                    m_F[n].Mean = (m_F[n].Mean*(m_dwValidFrameCount-1)/m_dwValidFrameCount) + FormantFreq/m_dwValidFrameCount;
                    //!!update standard deviation
                }
            }
        }
    }

    m_bTrack = bTrack;                    // update state of tracking request

    //    m_bDataReady = TRUE;
    nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
    // set progress bar
    SetProgress(nProgress);
    EndProcess((nProgress >= 100)); // end data processing
    EndWaitCursor();

    if (IsCanceled())
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);
    }

    return MAKELONG(nLevel, nProgress);
}


/***************************************************************************/
// CProcessFormants::Smooth  Smooth formant tracks
// For each frame, this function selects formant candidates (pole frequencies)
// within empirically-derived frequency ranges which are closest to the running
// 3-point median value for each formant track.  For a particular frame,
// if there are no formant candidates within the specified frequency range,
// the formant for the previous frame is repeated.
//
// The formant values are smoothed by the median filter as the candidates are
// evaluated.
//
// This function should only be called for voiced sections
// of speech after formants have been processed with formant tracking requested.
/***************************************************************************/
void CProcessFormants::SmoothMedian(ISaDoc * pDoc)
{

    // Try to find pole frequency within valid formant frequency range nearest current median value
    if (m_dwFormantFrameCount >= 3)
    {
        int nGender = pDoc->GetGender();

        SFormant * FormantFrame[3];
        SFormant MedianFreq[MAX_NUM_FORMANTS+1];
        m_dwValidFrameCount = 0;

        DWORD dwFrame = 0;
        for (; dwFrame < m_dwFormantFrameCount - 2; dwFrame++)
        {
            // Set pointers to each frame.
            FormantFrame[0] = GetFormantFrame(dwFrame)->Formant;     // frame to update with median
            FormantFrame[1] = GetFormantFrame(dwFrame+1)->Formant;   // frame to median smooth
            FormantFrame[2] = GetFormantFrame(dwFrame+2)->Formant;   // frame to select pole frequencies within valid formant ranges

            unsigned short nFormant = 1;
            unsigned short nPole = 1;
            for (; nFormant < MAX_NUM_FORMANTS; nFormant++)
            {
                // look for nearest formant from previous frame
                if (dwFrame == 0)
                {
                    MedianFreq[nFormant].Lpc.FrequencyInHertz = FormantFrame[0][nFormant].Lpc.FrequencyInHertz;
                }
                float fMinDistance = FLT_MAX;
				for (; nPole < MAX_NUM_FORMANTS; nPole++)   //!!number of formants
                {
                    if (FormantFrame[2][nPole].Lpc.FrequencyInHertz == (float)NA)
                    {
                        continue;
                    }
                    if (!CFormantPicker::BandwidthInRange(nGender, nFormant, FormantFrame[2][nPole].Lpc.BandwidthInHertz))
                    {
                        continue;
                    }
                    if (FormantFrame[2][nPole].Lpc.FrequencyInHertz > CFormantPicker::FormantRange(nGender, nFormant).High)
                    {
                        break;
                    }

                    if (FormantFrame[2][nPole].Lpc.FrequencyInHertz >= CFormantPicker::FormantRange(nGender, nFormant).Low)
                    {
                        float fDistance = (float)fabs(FormantFrame[2][nPole].Lpc.FrequencyInHertz - MedianFreq[nFormant].Lpc.FrequencyInHertz);
                        if (fDistance < fMinDistance)
                        {
                            fMinDistance = fDistance;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                if (fMinDistance != FLT_MAX)
                {
                    FormantFrame[2][nFormant].Lpc.FrequencyInHertz = FormantFrame[2][nPole-1].Lpc.FrequencyInHertz;
                }
                else
                {
                    // none found
                    // shift pole frequencies to allow formant to be inserted
                    for (UINT n = MAX_NUM_FORMANTS; n > nFormant; n--)
                    {
                        FormantFrame[2][n].Lpc.FrequencyInHertz = FormantFrame[2][n-1].Lpc.FrequencyInHertz;
                    }
                    // repeat formant from last frame
                    FormantFrame[2][nFormant].Lpc.FrequencyInHertz = FormantFrame[1][nFormant].Lpc.FrequencyInHertz;
                    //!!bandwidth and power?
                }

                // Perform median smoothing
                if (FormantFrame[0][nFormant].Lpc.FrequencyInHertz != (float)NA &&
                        FormantFrame[1][nFormant].Lpc.FrequencyInHertz != (float)NA &&
                        FormantFrame[2][nFormant].Lpc.FrequencyInHertz != (float)NA)
                {
                    if ((FormantFrame[0][nFormant].Lpc.FrequencyInHertz <= FormantFrame[1][nFormant].Lpc.FrequencyInHertz &&
                            FormantFrame[0][nFormant].Lpc.FrequencyInHertz >= FormantFrame[2][nFormant].Lpc.FrequencyInHertz) ||
                            (FormantFrame[0][nFormant].Lpc.FrequencyInHertz <= FormantFrame[2][nFormant].Lpc.FrequencyInHertz &&
                             FormantFrame[0][nFormant].Lpc.FrequencyInHertz >= FormantFrame[1][nFormant].Lpc.FrequencyInHertz))
                    {
                        // previous frame contains median value
                        float CurrMedian = FormantFrame[0][nFormant].Lpc.FrequencyInHertz;    // temp placeholder to keep from obliterating FormantFrame[0]
                        FormantFrame[0][nFormant].Lpc.FrequencyInHertz = MedianFreq[nFormant].Lpc.FrequencyInHertz;  // set previous frame's formant frequency to median value
                        MedianFreq[nFormant].Lpc.FrequencyInHertz = CurrMedian;  // update median value for current frame
                    }

                    else if ((FormantFrame[1][nFormant].Lpc.FrequencyInHertz <= FormantFrame[0][nFormant].Lpc.FrequencyInHertz &&
                              FormantFrame[1][nFormant].Lpc.FrequencyInHertz >= FormantFrame[2][nFormant].Lpc.FrequencyInHertz) ||
                             (FormantFrame[1][nFormant].Lpc.FrequencyInHertz <= FormantFrame[2][nFormant].Lpc.FrequencyInHertz &&
                              FormantFrame[1][nFormant].Lpc.FrequencyInHertz >= FormantFrame[0][nFormant].Lpc.FrequencyInHertz))
                    {
                        // current frame contains median value
                        FormantFrame[0][nFormant].Lpc.FrequencyInHertz = MedianFreq[nFormant].Lpc.FrequencyInHertz; // set previous frame's formant frequency to median value
                        MedianFreq[nFormant].Lpc.FrequencyInHertz = FormantFrame[1][nFormant].Lpc.FrequencyInHertz; // update median value for current frame
                    }

                    else if ((FormantFrame[2][nFormant].Lpc.FrequencyInHertz <= FormantFrame[0][nFormant].Lpc.FrequencyInHertz &&
                              FormantFrame[2][nFormant].Lpc.FrequencyInHertz >= FormantFrame[1][nFormant].Lpc.FrequencyInHertz) ||
                             (FormantFrame[2][nFormant].Lpc.FrequencyInHertz <= FormantFrame[1][nFormant].Lpc.FrequencyInHertz &&
                              FormantFrame[2][nFormant].Lpc.FrequencyInHertz >= FormantFrame[0][nFormant].Lpc.FrequencyInHertz))
                    {
                        // next frame contains median value
                        FormantFrame[0][nFormant].Lpc.FrequencyInHertz = MedianFreq[nFormant].Lpc.FrequencyInHertz; // set previous frame's formant frequency to median value
                        MedianFreq[nFormant].Lpc.FrequencyInHertz = FormantFrame[2][nFormant].Lpc.FrequencyInHertz; // update median value for current frame
                    }
                }
            }
            BOOL bValidF1F2F3 = (FormantFrame[0][1].Lpc.FrequencyInHertz != (float)NA &&
                                 FormantFrame[0][2].Lpc.FrequencyInHertz != (float)NA &&
                                 FormantFrame[0][3].Lpc.FrequencyInHertz != (float)NA);
            if (bValidF1F2F3)
            {
                m_dwValidFrameCount++;
                if (dwFrame == 0)
                    for (nFormant = 1; nFormant <= 3; nFormant++)
                    {
                        m_F[nFormant].Mean = FormantFrame[0][nFormant].Lpc.FrequencyInHertz;
                        m_F[nFormant].StdDev = 0.;  //!!update
                    }
                else
                    for (nFormant = 1; nFormant <= 3; nFormant++)
                    {
                        double FormantFreq = FormantFrame[0][nFormant].Lpc.FrequencyInHertz;
                        m_F[nFormant].Mean = (m_F[nFormant].Mean*(m_dwValidFrameCount-1)/m_dwValidFrameCount) + FormantFreq/m_dwValidFrameCount;
                        //!!update standard deviation
                    }
            }

            // Manage buffer.
            DWORD dwBlockEnd = (dwFrame + 3) * m_nFormantFrameSize;
            if (dwBlockEnd >= m_dwBufferOffset + GetProcessBufferSize())
            {
                // all 3 frames not in buffer
                DWORD dwBlockStart = (dwFrame + 1) * m_nFormantFrameSize;
                SaveBuffer((UINT)(dwBlockStart - m_dwBufferOffset));  // save buffer
                LoadBuffer(dwBlockStart);                      // load next block
            }
        }

        // Update next to last frame.
        FormantFrame[0] = GetFormantFrame(dwFrame)->Formant;
        UINT nFormant = 1;
        for (; nFormant <= MAX_NUM_FORMANTS; nFormant++)
        {
            FormantFrame[0][nFormant].Lpc.FrequencyInHertz = MedianFreq[nFormant].Lpc.FrequencyInHertz;
        }

        BOOL bValidF1F2F3 = (FormantFrame[0][1].Lpc.FrequencyInHertz != (float)NA &&
                             FormantFrame[0][2].Lpc.FrequencyInHertz != (float)NA &&
                             FormantFrame[0][3].Lpc.FrequencyInHertz != (float)NA);
        if (bValidF1F2F3)
        {
            m_dwValidFrameCount++;
            for (nFormant = 1; nFormant <= 3; nFormant++)
            {
                double FormantFreq = FormantFrame[0][nFormant].Lpc.FrequencyInHertz;
                m_F[nFormant].Mean = (m_F[nFormant].Mean*(m_dwValidFrameCount-1)/m_dwValidFrameCount) +
                                     FormantFreq/m_dwValidFrameCount;
                //!!update standard deviation
            }
        }
        //!!blockend beyond end of buffer?
        // Store last block.
        DWORD dwBlockEnd = (dwFrame + 2) * m_nFormantFrameSize;
        SaveBuffer((UINT)(dwBlockEnd - m_dwBufferOffset));
    }

}

/***************************************************************************/
// CProcessFormants::StoreFormantFrame  Store formant frame data
// The formant frame data is stored in the process buffer.  If WriteThru
// is TRUE, the frame data is also written to a temporary file.  The frames
// are stored in the process buffer in order, but are written to disk in the
// order they are received.  Thus, formant frames should be passed to this function
// sequentially, starting with 0 and proceeding to the highest frame index.
/***************************************************************************/
BOOL CProcessFormants::StoreFormantFrame(DWORD dwFormantFrame, SFormantFrame * pFormant, BOOL bWriteThru)
{
    // Calculate beginning and end of the frame in bytes.
    DWORD dwFormantFrameStart = dwFormantFrame * m_nFormantFrameSize;
    DWORD dwFormantFrameEnd = dwFormantFrameStart + m_nFormantFrameSize;

    // Reset the data buffer byte offset into the frame data.
    if (dwFormantFrame == 0)
    {
        m_dwBufferOffset = 0;    // to zero if first frame
    }
    if (dwFormantFrameStart < m_dwBufferOffset || dwFormantFrameEnd > m_dwBufferOffset + GetProcessBufferSize())
    {
        m_dwBufferOffset = dwFormantFrameStart;    // to beginning of the frame if any frame data would fall outside buffer
    }

    // Copy the frame data to the buffer.
    HPSTR pFormantFrame = m_lpBuffer + dwFormantFrameStart - m_dwBufferOffset;
    memcpy(pFormantFrame, (HPSTR)pFormant, m_nFormantFrameSize);  // copy frame into buffer

    // If requested, write the frame data to disk.
    if (bWriteThru)
    {
        BOOL bAppend = (dwFormantFrame != 0);
        if (!WriteData(pFormantFrame, m_nFormantFrameSize, bAppend))
        {
            return FALSE;
        };
    }
    return TRUE;
}


/***************************************************************************/
// CProcessFormants::WriteData  Write data to a temporary disk file
// If the append flag is true, the data is appended to the file; otherwise,
// a new file is created.
/***************************************************************************/
BOOL CProcessFormants::WriteData(HPSTR pData, UINT nDataSize, BOOL bAppend)
{
    // If not appending, delete the current file and create a new one.
    if (!bAppend)
    {
        DWORD dwBufferOffset = m_dwBufferOffset;   // save buffer offset (destroyed by CreateTempFile)
        if (!CreateTempFile(_T("FMT")))   // creating error
        {
            return FALSE;
        }
        m_dwBufferOffset = dwBufferOffset;  // restore buffer offset
    }

    // Otherwise, open the existing file containing formant frame data.
    else if (!OpenFileToAppend())
    {
        // error opening file
        ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
        return FALSE;
    }

    // Write the data to the file.
    try
    {
        Write(pData, nDataSize);
    }
    catch (CFileException e)
    {
        // error writing file
        ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
        return FALSE; // error, writing failed
    }

    // Close the temporary file and read the status.
    CloseTempFile(); // close the file
    return TRUE;
}


/***************************************************************************/
// CProcessFormants::SaveBuffer  Save buffer contents to disk
/***************************************************************************/
BOOL CProcessFormants::SaveBuffer(UINT nDataSize)
{
    return WriteDataBlock(m_dwBufferOffset, m_lpBuffer, nDataSize, sizeof(char));
}

/***************************************************************************/
// CProcessFormants::GetFormantFrame Read formant frequency data from temp file
// Reads processed data from the process buffer if available or if not, from the
// temporary file, into the buffer, returning a pointer to formant frame at the
// specified index.
/***************************************************************************/
SFormantFrame * CProcessFormants::GetFormantFrame(DWORD dwIndex)
{
    // Check to see if frame data is in process buffer.
    DWORD dwFormantFrameStart = dwIndex * m_nFormantFrameSize;    // calculate byte offset
    if (dwFormantFrameStart >= m_dwBufferOffset && dwFormantFrameStart + m_nFormantFrameSize <= m_dwBufferOffset + GetProcessBufferSize())
    {
        return (SFormantFrame *)(m_lpBuffer + dwFormantFrameStart - m_dwBufferOffset);    // return pointer to frame data
    }

    // Return pointer to the frame data.
    return LoadBuffer(dwFormantFrameStart);
}


/***************************************************************************/
// CProcessFormants::LoadBuffer  Loads buffer with formant data
/***************************************************************************/
SFormantFrame * CProcessFormants::LoadBuffer(DWORD dwFormantBlockStart)
{
    return (SFormantFrame *)GetProcessedWaveData(dwFormantBlockStart, TRUE);
}

/***************************************************************************/
// CProcessFormants::GetVowelFreq Return vowel frequencies
// The frequencies of F1, F2, and F3 are returned in the formants structure.
// The frequencies are returned in Hertz.
/***************************************************************************/
SFormantFreq CProcessFormants::GetVowelFreq(int nIndex, int nGender, BOOL bMelScale) const
{
    const CVowelFormantsVector & vowelVector = GetVowelVector(nGender);

    // get frequency range
    SFormantFreq formants;

    if (nIndex >= 0 && nIndex < (int)vowelVector.size())
    {
        const CVowelFormants & vowel = vowelVector[nIndex];
        formants.F[1] = (float)vowel.F1;
        formants.F[2] = (float)vowel.F2;
        formants.F[3] = (float)vowel.F3;
        formants.F[4] = (float)vowel.F4;
    }
    else
    {
        formants.F[1] = (float)UNDEFINED_DATA;
        formants.F[2] = (float)UNDEFINED_DATA;
        formants.F[3] = (float)UNDEFINED_DATA;
        formants.F[4] = (float)UNDEFINED_DATA;
    }

    // convert to mel scale, if requested
    if (bMelScale)
    {
        if (formants.F[1] != (float)UNDEFINED_DATA)
        {
            formants.F[1] = (float)Scale::ToMels((double)formants.F[1]);
        }
        if (formants.F[2] != (float)UNDEFINED_DATA)
        {
            formants.F[2] = (float)Scale::ToMels((double)formants.F[2]);
        }
        if (formants.F[3] != (float)UNDEFINED_DATA)
        {
            formants.F[3] = (float)Scale::ToMels((double)formants.F[3]);
        }
        if (formants.F[4] != (float)UNDEFINED_DATA)
        {
            formants.F[4] = (float)Scale::ToMels((double)formants.F[4]);
        }
    }

    // clear out rest of formant array
    formants.F[0] = (float)UNDEFINED_DATA;
    for (int n = 5; n < sizeof(formants)/sizeof(formants.F[0]); n++)
    {
        formants.F[n] = (float)UNDEFINED_DATA;
    }

    return formants;
}

/***************************************************************************/
// CProcessFormants::GetVowel Returns vowel
/***************************************************************************/
CSaString CProcessFormants::GetVowel(int nIndex, int nGender) const
{
    const CVowelFormantsVector & vowelVector = GetVowelVector(nGender);

    if (nIndex >= 0 && nIndex < (int)vowelVector.size())
    {
        const CVowelFormants & vowel = vowelVector[nIndex];
        return vowel.m_szVowel;
    }
    else
    {
        return CSaString(_T(""));
    }
}

/***************************************************************************/
// CProcessFormants::GetVowelCount Returns number of vowels for gender
/***************************************************************************/
int CProcessFormants::GetVowelCount(int nGender) const
{
    const CVowelFormantsVector & vowelVector = GetVowelVector(nGender);
    return vowelVector.size();
}

/***************************************************************************/
// CProcessFormants::FindNearestVowel Returns vowel closest to F1, F2, and F3
/***************************************************************************/
CSaString CProcessFormants::FindNearestVowel(SFormantFreq FormantFreq, ISaDoc * pDoc) const
{
    int nGender = pDoc->GetGender();
    double dMinDistanceSquared = FLT_MAX;
    int nNearestVowel = -1;

    SRange FormantScale[4];

    for (int nFormant = 1; nFormant <= 3; nFormant++)
    {
        FormantScale[nFormant] = CPlot3D::GetChartRange(nFormant, nGender);
        if (m_FormantParms.bMelScale)
        {
            FormantScale[nFormant].High = (float)Scale::ToMels((double)FormantScale[nFormant].High);
            FormantScale[nFormant].Low = (float)Scale::ToMels((double)FormantScale[nFormant].Low);
        }
    }

    //!! should use F2 + F1 v F2 - F1 normalized to axis ranges to eliminate correlation between F1 and F2
    //!! what about correlation with F3?
    //!! also aspect ratio should be maintained for each chart so as not to confuse user
    for (int nVowel = 0; nVowel < GetVowelCount(nGender); nVowel++)
    {
        double dDistanceSquared = 0.;
        SFormantFreq VowelFreq = GetVowelFreq(nVowel, nGender);
        for (int nFormant = 1; nFormant <= 3; nFormant++)
        {
            // compute Euclidean distance squared
            double dClusterMean = (double)FormantFreq.F[nFormant];
            if (m_FormantParms.bMelScale)
            {
                VowelFreq.F[nFormant] = (float)Scale::ToMels((double)VowelFreq.F[nFormant]);
                dClusterMean = Scale::ToMels(dClusterMean);
            }
            dClusterMean = min(max(dClusterMean, FormantScale[nFormant].Low), FormantScale[nFormant].High);
            double dDifference = (dClusterMean - VowelFreq.F[nFormant])/(FormantScale[nFormant].High - FormantScale[nFormant].Low);
            dDistanceSquared += (dDifference * dDifference);
        }
        if (dDistanceSquared < dMinDistanceSquared)
        {
            nNearestVowel = nVowel;
            dMinDistanceSquared = dDistanceSquared;
        }
    }
    return GetVowel(nNearestVowel, nGender);
}


CSaString CProcessFormants::FindNearestVowel(ISaDoc * pDoc) const
{
    BOOL bValidMeans = (m_F[1].Mean != (float)UNDEFINED_DATA &&
                        m_F[2].Mean != (float)UNDEFINED_DATA &&
                        m_F[3].Mean != (float)UNDEFINED_DATA);
    if (!bValidMeans)
    {
        return 0x20;    // return a blank in the IPA font if no valid means
    }
    SFormantFreq FormantFreq;
    FormantFreq.F[1] = (float)m_F[1].Mean;
    FormantFreq.F[2] = (float)m_F[2].Mean;
    FormantFreq.F[3] = (float)m_F[3].Mean;
    return FindNearestVowel(FormantFreq, pDoc);
}

void CProcessFormants::SetFormantParms(CFormantParm * pFormantParms)
{
    m_FormantParms = *pFormantParms;
}

void CProcessFormants::SetDataInvalid()
{
    SetDataReady(FALSE);
}

CFormantParm * CProcessFormants::GetFormantParms()
{
    return &m_FormantParms;
}

DWORD CProcessFormants::GetFormantFrameCount()
{
    return m_dwFormantFrameCount;
}

unsigned short CProcessFormants::GetFormantFrameSize()
{
    return m_nFormantFrameSize;
}

SStatistic CProcessFormants::GetFormantStats(int nFormant)
{
    return m_F[nFormant];
}

void CProcessFormants::ResetTracking()
{
    m_bTrack = FALSE;
    m_dwFormantFrameCount = 0;
    SetDataReady(FALSE);
}

