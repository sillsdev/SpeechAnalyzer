/**************************************************************************\
* MODULE:       WAVEWARP.CPP                                               *
* COPYRIGHT:    SUMMER INSTITUTE OF LINGUISTICS, 1997-1999                 *
* AUTHOR:       ALEC K. EPTING                                             *
*                                                                          *
*                                                                          *
* DESCRIPTION:                                                             *
*                                                                          *
* Class to implement speaking rate change in real time.                    *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER VARIABLES:                                                 *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER FUNCTIONS:                                                 *
*                                                                          *
* Note: all arguments are passed using the C calling convention.           *
*                                                                          *
* Copyright                                                                *
*   Function to return character pointer to copyright notice.              *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
* Version                                                                  *
*   Function to return version of class as floating point number.          *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
* CreateObject                                                             *
*   Function to construct waveform warping object.                         *
*                                                                          *
*   Arguments:                                                             *
*     ppoWaveWarp          address to contain pointer to wavewarp object   *
*     dwWaveStart          waveform index for first sample to be played    *
*     wSpeed               speaking rate as a percentage of the original   *
*                          recording                                       *
*     pstFragStart         pointer to parameters for fragment containing   *
*                          starting sample; the waveform buffer should     *
*                          be filled with sample data beginning at         *
*                          this fragment                                   *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                      no fatal errors                            *
*     INVALID_PARM_PTR          fatal error: invalid pointer specified     *
*     INVALID_BLOCK             fatal error: invalid waveform starting     *
*                                  index specified                         *
*     INVALID_WARP_SPEED    fatal error: invalid warp speed specified  *
*     UNSUPP_SMP_DATA_FMT       fatal error: unsupported sample data       *
*                                  format specified                        *
*     FRAGMENT_NOT_FOUND        fatal error: waveform block starting       *
*                                  index not found in any fragments        *
*     OUT_OF_MEMORY             fatal error: insufficient memory to        *
*                                  create object                           *
*                                                                          *
*                                                                          *
* SetWaveBuffer                                                            *
*   Function to set pointer to waveform buffer.                            *
*                                                                          *
*   Arguments:                                                             *
*     pWaveBfr             generic pointer to waveform buffer              *
*   Return value:                                                          *
*     DONE                      no fatal errors                            *
*     INVALID_PARM              fatal error:  pointer is NULL              *
*                                                                          *
*                                                                          *
* SetPlayBuffer                                                            *
*   Function to set play buffer pointer and buffer length.                 *
*                                                                          *
*   Arguments:                                                             *
*     pPlayBfr                generic pointer to waveform buffer           *
*     dwPlayBfrLength      play buffer length in samples                   *
*   Return value:                                                          *
*     DONE                      no fatal errors                            *
*     INVALID_PARM              fatal error:  pointer is NULL or           *
*                                  play buffer length is zero              *
*                                                                          *
* FillPlayBuffer                                                           *
*   Function to fill play buffer with waveform fragments according to      *
*   specified speaking rate.                                               *
*                                                                          *
*   Arguments:                                                             *
*     dwWaveBlock          waveform index for first sample in block to     *
*                          be processed                                    *
*     dwWaveBlockLength    number of samples in block                      *
*     pstCallFragment      pointer to structure which will contain block   *
*                          index to load in order to continue processing   *
*     pdwPlayLength        address of location to contain number of        *
*                          samples in play buffer                          *
*     wNewSpeed            speaking rate as a percentage of the original   *
*                          recording                                       *
*   Return value:                                                          *
*     OUTSIDE_WAVE_BUFFER       status: re-load waveform buffer requested  *
*     PLAY_BUFFER_FULL          status: play buffer ready to play          *
*     DONE                      no fatal errors, last fragment encountered *
*     INVALID_PARM              fatal error: invalid pointer to waveform   *
*                                  buffer or play buffer specified         *
*     INVALID_BLOCK             fatal error: block index does not match    *
*                                  callback index                          *
*                                                                          *
*                                                                          *
* TYPICAL CALLING SEQUENCE:                                                *
*          :                                                               *
*   #include "WaveWarp.h"                                                  *
*          :                                                               *
*   FRAG_PARMS *lpFragment                                                 *
*           :                                                              *
*   (load fragment parameters)                                             *
*           :                                                              *
*   FRAG_PARMS stCallFragment;                                             *
*   CWaveWarp *pWaveWarp;                                                  *
*   dspError_t Err = CWaveWarp::CreateObject(&pWaveWarp, dwStart/wSmpSize,        *
*                                     wSpeed, stCallFragment);             *
*   if (Err) return;                                                       *
*                                                                          *
*   Err = pWaveWarp->SetWaveBuffer((void *)pData);                         *
*   Err = pWaveWarp->SetPlayBuffer((void *)pTarget, dwPlayBfrSize);        *
*   if (Err) return;                                                       *
*                                                                          *
*   DWORD dwPlayLength;                                                    *
*   dspError_t Status;                                                         *
*   do{                                                                    *
*      (load waveform buffer at pData with dwDataSize bytes of data        *
*       starting at stCallFragment.dwOffset in waveform)                   *
*            :                                                             *
*      if (Err) return;                                                    *
*      Status = pWaveWarp->FillPlayBuffer(stCallFragment.dwOffset,         *
*                                         dwDataSize/wSmpSize,             *
*                                         &stCallFragment,                 *
*                                         &dwPlayLength, wSpeed);          *
*     }while(Status == OUTSIDE_WAVE_BUFFER);                               *
*            :                                                             *
*   delete pWaveWarp;                                                      *
*                                                                          *
*                                                                          *
*                                                                          *
* COMPILER:         Microsoft Visual C++ version 1.52                      *
* COMPILE OPTIONS:  /nologo /G3 /W3 /Zi /AL /YX /D "_DEBUG"                *
*                   /I "c:\msvc\dsp" /GA /Fd"SA.PDB" /Fp"SA.PCH"           *
*                                                                          *
* LINK OPTIONS:     /NOLOGO /LIB:"lafxcwd" /LIB:"oldnames" /LIB:"libw"     *
*                   /LIB:"llibcew" /NOD /NOE                               *
*                   /PACKC:61440 /STACK:10120 /SEG:256 /ALIGN:64           *
*                   /ONERROR:NOEXE /CO /LIB:"commdlg.lib"                  *
*                   /LIB:"mmsystem.lib" /LIB:"shell.lib"                   *
*                                                                          *
* TO DO:                                                                   *
*                                                                          *
*                                                                          *
* CHANGE HISTORY:                                                          *
*   Date     By             Description                                    *
* 11/03/97  AKE     Debugged and tested.                                   *
* 06/23/00  AKE     Set default object pointer to NULL.                    *
\**************************************************************************/
#include "stdafx.h"
#define WAVEWARP_CPP
#define COPYRIGHT_DATE  "1997"
#define VERSION_NUMBER  "2.0"

#include "WaveWarp.h"
#include "Process\Process.h"
#include "Process\sa_p_fra.h"
#include "isa_doc.h"

#include "MathX.h"



////////////////////////////////////////////////////////////////////////////////////////
// Class function to return copyright notice.                                         //
////////////////////////////////////////////////////////////////////////////////////////
char * CWaveWarp::Copyright(void)
{
    static char Notice[] = {"WaveWarp Version " VERSION_NUMBER "\n"
                            "Copyright (c) " COPYRIGHT_DATE " by Summer Institute of Linguistics. "
                            "All rights reserved.\n"
                           };
    return(Notice);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return version of class.                                         //
////////////////////////////////////////////////////////////////////////////////////////
float CWaveWarp::Version(void)
{
    return((float)atof(VERSION_NUMBER));
}

#undef COPYRIGHT_DATE
#undef VERSION_NUMBER

////////////////////////////////////////////////////////////////////////////////////////
// Class function to construct waveform warping object if parameters are valid.       //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CWaveWarp::CreateObject(CWaveWarp ** ppWaveWarp, ISaDoc * pDoc, ULONG dwWaveStart, USHORT wSpeed,
                                   FRAG_PARMS * pstFragStart)
{
// Validate parameters passed.
    if (!ppWaveWarp || !pDoc)
    {
        return(Code(INVALID_PARM_PTR));
    }
    *ppWaveWarp = NULL;
    FmtParm * pstSmpFormat = pDoc->GetFmtParm();
    ULONG dwWaveLength = pDoc->GetDataSize() / pstSmpFormat->wBlockAlign;
    if (dwWaveStart >= dwWaveLength)
    {
        return(Code(INVALID_BLOCK));
    }
    if (!wSpeed)
    {
        return(Code(INVALID_WARP_SPEED));
    }
    if (pstSmpFormat->wTag != FILE_FORMAT_PCM || pstSmpFormat->wChannels != 1 ||
            (pstSmpFormat->wBitsPerSample != 8 && pstSmpFormat->wBitsPerSample != 16))
    {
        return(Code(UNSUPP_SMP_DATA_FMT));
    }

// Locate fragment containing starting position for waveform warping.
    CProcessFragments * pFragments = pDoc->GetFragments();
    ULONG dwFragCount = pFragments->GetFragmentCount();
    ULONG dwFragBfrLength = pFragments->GetBufferLength();

    ULONG dwFragBlock = 0;
    FRAG_PARMS * pstFragBfr = pFragments->GetFragmentBlock(dwFragBlock);
    if (!pstFragBfr)
    {
        return(Code(FRAGMENT_NOT_FOUND));
    }
    ULONG dwFragBlockLength = min(dwFragBfrLength, dwFragCount - dwFragBlock);
    ULONG dwFragBfrIndex;

// Find fragment containing the wave sample where warping is to start.
    do
    {
        // scan through fragment buffer
        for (dwFragBfrIndex = 0; dwFragBfrIndex < dwFragBlockLength; dwFragBfrIndex++)
            if (dwWaveStart >= pstFragBfr[dwFragBfrIndex].dwOffset &&
                    dwWaveStart < pstFragBfr[dwFragBfrIndex].dwOffset + pstFragBfr[dwFragBfrIndex].wLength)
            {
                break;
            }

        // if not found, load the next fragment block into the fragment buffer
        if (dwFragBfrIndex == dwFragBlockLength)
        {
            dwFragBlock += dwFragBlockLength;
            if (dwFragBlock != dwFragCount)
            {
                // not the last fragment block
                dwFragBlockLength = min(dwFragBfrLength, dwFragCount - dwFragBlock);  // recalc block length
                pFragments->GetFragmentBlock(dwFragBlock);  // load next block
            }
        }
        else
        {
            break;
        }

    }
    while (dwFragBlock < dwFragCount);   // loop through fragment blocks
    if (dwFragBlock == dwFragCount)
    {
        return(Code(FRAGMENT_NOT_FOUND));
    }
    *pstFragStart = pstFragBfr[dwFragBfrIndex];   // return parameters for fragment to load into waveform buffer

// Construct wavewarp object.
    *ppWaveWarp = new CWaveWarp(pDoc, dwWaveStart, wSpeed, dwFragBlock + dwFragBfrIndex);
    if (!*ppWaveWarp)
    {
        return(Code(OUT_OF_MEMORY));
    }

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Wavewarp object constructor.                                                       //
////////////////////////////////////////////////////////////////////////////////////////
CWaveWarp::CWaveWarp(ISaDoc * pDoc, ULONG dwWaveStart, USHORT wSpeed, ULONG dwFragStart)
{

// Initialize member variables.
    m_pDoc = pDoc;
    m_pWaveBfr = NULL;
    m_dwWarpStart = dwWaveStart;
    m_dwWarpIndex = 0;
    m_pPlayBfr = NULL;
    m_dwPlayBfrLength = 0;
    m_dwPlayLength = 0;
    m_dwFragBlock = dwFragStart;
    m_pstFragBfr = pDoc->GetFragments()->GetFragmentBlock(dwFragStart);  // load fragment at beginning of buffer
    m_dwFragBfrIndex = 0;
    m_wSpeed = wSpeed;
    m_sbSmpDataFmt = (char)((pDoc->GetFmtParm()->wBitsPerSample == 8) ? PCM_UBYTE: PCM_2SSHORT);
    m_dJitterFactor = 0.;
    m_dSmpTime = m_pstFragBfr[m_dwFragBfrIndex].dwOffset;
    m_nSegmentIndex = 0;
    /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
    m_hPlay = fopen("Playback.txt", "w");
#endif
    /******************************************************************************/
}

////////////////////////////////////////////////////////////////////////////////////////
// Wavewarp object destructor.                                                        //
////////////////////////////////////////////////////////////////////////////////////////
CWaveWarp::~CWaveWarp()
{
    /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
    fclose(m_hPlay);
#endif
    /******************************************************************************/
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to set waveform buffer.                                            //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CWaveWarp::SetWaveBuffer(void * pWaveBfr)
{
    if (!pWaveBfr)
    {
        return(Code(INVALID_PARM));
    }

    m_pWaveBfr = pWaveBfr;

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to set or change playback buffer.                                  //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CWaveWarp::SetPlayBuffer(void * pPlayBfr, ULONG dwPlayBfrLength)
{
    if (!pPlayBfr || !dwPlayBfrLength)
    {
        return(Code(INVALID_PARM));
    }

    m_pPlayBfr = pPlayBfr;
    m_dwPlayBfrLength = dwPlayBfrLength;
    m_dwPlayLength = 0;

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to fill playback buffer with waveform fragment scaled according    //
// to requested speaking rate speed.                                                  //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CWaveWarp::FillPlayBuffer(ULONG dwWaveBlock, ULONG dwWaveBlockLength,  FRAG_PARMS * pstCallFragment,
                                     ULONG * pdwPlayLength, USHORT wNewSpeed)
{
//return when outbuffer full or beyond input buffer length
//initialize playlength to 0, dwWaveBlock to load offset, pdwFragIndex to 0
//!!DataFmt
//!!PlayLength = 0
//!!reset playindex in SetPlayBuffer(pPlay, wSpeed)
//!!reset playlength in SetPlayBuffer

    if (!m_pWaveBfr || !m_pPlayBfr)
    {
        return(Code(INVALID_PARM));    //!!is this necessary?
    }
    CProcessFragments * pFragments = m_pDoc->GetFragments();
    if (pFragments->GetProcessBufferIndex() != m_dwFragBlock)
    {
        pFragments->GetFragmentBlock(m_dwFragBlock);    // reload block
    }
    if (dwWaveBlock != m_pstFragBfr[m_dwFragBfrIndex].dwOffset)
    {
        return(Code(INVALID_BLOCK));
    }

//CProcessFragments *pFragments = m_pDoc->GetFragments();
    ULONG dwFragCount = pFragments->GetFragmentCount();
    ULONG dwFragBfrLength = pFragments->GetBufferLength();
    do
    {
        // Translate sample position back to unscaled waveform.
        ULONG dwWaveIndex = m_dwWarpStart + (ULONG)((float)m_dwWarpIndex*(float)m_wSpeed/100.F + 0.5F);

        // Find fragment containing this sample.
        ULONG dwPrevFragIndex = m_dwFragBlock + m_dwFragBfrIndex;
        ULONG dwFragBlockLength = min(dwFragBfrLength, dwFragCount - m_dwFragBlock);
        do
        {
            // scan through fragment buffer
            for (; m_dwFragBfrIndex < dwFragBlockLength; m_dwFragBfrIndex++)   // continues from last fragment checked
                if (dwWaveIndex >= m_pstFragBfr[m_dwFragBfrIndex].dwOffset &&
                        dwWaveIndex < m_pstFragBfr[m_dwFragBfrIndex].dwOffset + m_pstFragBfr[m_dwFragBfrIndex].wLength)
                {
                    break;
                }

            // if not found, load the next fragment block into the fragment buffer
            if (m_dwFragBfrIndex == dwFragBlockLength)
            {
                m_dwFragBlock += dwFragBlockLength;
                if (m_dwFragBlock != dwFragCount)
                {
                    // not the last fragment block
                    dwFragBlockLength = min(dwFragBfrLength, dwFragCount - m_dwFragBlock);  // recalc block length
                    pFragments->GetFragmentBlock(m_dwFragBlock);  // load next block
                    m_dwFragBfrIndex = 0;  // reset index into fragment buffer
                }
            }
            else
            {
                break;
            }

        }
        while (m_dwFragBlock < dwFragCount);   // loop through fragment blocks

        if (m_dwFragBlock == dwFragCount)
        {
            // no more fragment blocks
            /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
            if (m_sbSmpDataFmt == 2)
                for (DWORD i =0; i < m_dwPlayLength; i++)
                {
                    fprintf(m_hPlay, "%d ", ((short *)m_pPlayBfr)[i]);
                }
#endif
            /******************************************************************************/
            pstCallFragment->dwOffset = m_pstFragBfr[dwFragBlockLength-1].dwOffset +
                                        m_pstFragBfr[dwFragBlockLength-1].wLength;
            pstCallFragment->wLength = 0;
            *pdwPlayLength =  m_dwPlayLength;
            return(DONE);
        }

        // Change speed as required only when waveform fragment copying is done.
        if (m_wSpeed != wNewSpeed && m_dwFragBlock + m_dwFragBfrIndex != dwPrevFragIndex)
        {
            m_dwWarpStart = m_pstFragBfr[m_dwFragBfrIndex].dwOffset;
            m_dwWarpIndex = 0;
            m_wSpeed = wNewSpeed;
        }

        // If fragment to be copied is not in the waveform buffer, request the buffer be loaded.
        long lWaveBfrIndex = (long)m_pstFragBfr[m_dwFragBfrIndex].dwOffset - (long)dwWaveBlock;
        ULONG dwFragLength =  m_pstFragBfr[m_dwFragBfrIndex].wLength;

        if (lWaveBfrIndex < 0 || (ULONG)lWaveBfrIndex + dwFragLength > dwWaveBlockLength)
        {
            *pstCallFragment = m_pstFragBfr[m_dwFragBfrIndex];
            *pdwPlayLength =  m_dwPlayLength;
            return(OUTSIDE_WAVE_BUFFER);  //!!last playback should be cut short in calling function
        }                             //!!use WAVE_BUFFER_CALLBACK

        // If play buffer is full, return for playback.
        //!! handle partial fragment in calling function by offsetting into play buffer before playback
        if (m_dwPlayLength + dwFragLength > m_dwPlayBfrLength)
        {
            /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
            if (m_sbSmpDataFmt == 2)
                for (DWORD i =0; i < m_dwPlayLength; i++)
                {
                    fprintf(m_hPlay, "%d ", ((short *)m_pPlayBfr)[i]);
                }
#endif
            /******************************************************************************/
            *pstCallFragment = m_pstFragBfr[m_dwFragBfrIndex];
            *pdwPlayLength = m_dwPlayLength;
            m_dwPlayLength = 0;
            return(PLAY_BUFFER_FULL);
        }

        // Copy fragment.
        if (m_sbSmpDataFmt == PCM_UBYTE)
            memcpy((void *)&((uint8 *)m_pPlayBfr)[m_dwPlayLength], (void *)&((uint8 *)m_pWaveBfr)[lWaveBfrIndex],
                   (size_t)dwFragLength*(size_t)abs(m_sbSmpDataFmt));
        else    // PCM_2SSHORT
            memcpy((void *)&((short *)m_pPlayBfr)[m_dwPlayLength], (void *)&((short *)m_pWaveBfr)[lWaveBfrIndex],
                   (size_t)dwFragLength*(size_t)abs(m_sbSmpDataFmt));
        m_dwPlayLength += dwFragLength;    // update playback length
        m_dwWarpIndex += dwFragLength;     // update scaling index

    }
    while (TRUE);
}

dspError_t CWaveWarp::FillPlayBuffer(ULONG dwWaveBlock, ULONG dwWaveBlockLength,  FRAG_PARMS * pstCallFragment,
                                     ULONG * pdwPlayLength)
{
//return when outbuffer full or beyond input buffer length
//initialize playlength to 0, dwWaveBlock to load offset, pdwFragIndex to 0
//!!DataFmt
//!!PlayLength = 0
//!!reset playindex in SetPlayBuffer(pPlay, wSpeed)
//!!reset playlength in SetPlayBuffer
//!!ensure loudness, pitch, duration run in CreateObject

    if (!m_pWaveBfr || !m_pPlayBfr)
    {
        return(Code(INVALID_PARM));
    }
    CProcessFragments * pFragments = m_pDoc->GetFragments();
    if (pFragments->GetProcessBufferIndex() != m_dwFragBlock)
    {
        pFragments->GetFragmentBlock(m_dwFragBlock);    // reload block
    }
    if (dwWaveBlock != m_pstFragBfr[m_dwFragBfrIndex].dwOffset)
    {
        return(Code(INVALID_BLOCK));
    }

    ULONG dwFragCount = pFragments->GetFragmentCount();
    ULONG dwFragBfrLength = pFragments->GetBufferLength();
    do
    {
        // Translate sample position back to unscaled waveform.
        ULONG dwWaveIndex = m_dwWarpStart + (ULONG)((float)m_dwWarpIndex*(float)m_wSpeed/100.F + 0.5F);

        // Find fragment containing this sample.
        ULONG dwFragBlockLength = min(dwFragBfrLength, dwFragCount - m_dwFragBlock);
        do
        {
            // scan through fragment buffer
            for (; m_dwFragBfrIndex < dwFragBlockLength; m_dwFragBfrIndex++)   // continues from last fragment checked
                if (dwWaveIndex >= m_pstFragBfr[m_dwFragBfrIndex].dwOffset &&
                        dwWaveIndex < m_pstFragBfr[m_dwFragBfrIndex].dwOffset + m_pstFragBfr[m_dwFragBfrIndex].wLength)
                {
                    break;
                }

            // if not found, load the next fragment block into the fragment buffer
            if (m_dwFragBfrIndex == dwFragBlockLength)
            {
                m_dwFragBlock += dwFragBlockLength;
                if (m_dwFragBlock != dwFragCount)
                {
                    // not the last fragment block
                    dwFragBlockLength = min(dwFragBfrLength, dwFragCount - m_dwFragBlock);  // recalc block length
                    pFragments->GetFragmentBlock(m_dwFragBlock);  // load next block
                    m_dwFragBfrIndex = 0;  // reset index into fragment buffer
                }
            }
            else
            {
                break;
            }

        }
        while (m_dwFragBlock < dwFragCount);   // loop through fragment blocks

        if (m_dwFragBlock == dwFragCount)
        {
            // no more fragment blocks
            pstCallFragment->dwOffset = m_pstFragBfr[dwFragBlockLength-1].dwOffset +
                                        m_pstFragBfr[dwFragBlockLength-1].wLength;
            pstCallFragment->wLength = 0;
            *pdwPlayLength =  m_dwPlayLength;
            return(DONE);
        }

        long lWaveBfrIndex = (long)m_pstFragBfr[m_dwFragBfrIndex].dwOffset - (long)dwWaveBlock;
        ULONG dwFragLength =  m_pstFragBfr[m_dwFragBfrIndex].wLength;

        if (lWaveBfrIndex < 0 || (ULONG)lWaveBfrIndex + dwFragLength > dwWaveBlockLength)
        {
            *pstCallFragment = m_pstFragBfr[m_dwFragBfrIndex];
            *pdwPlayLength =  m_dwPlayLength;
            return(OUTSIDE_WAVE_BUFFER);  //!!last playback should be cut short in calling function
        }                              //!!use WAVE_BUFFER_CALLBACK

        // If play buffer is full, return for playback.
        //!! handle partial fragment in calling function by offsetting into play buffer before playback
        if (m_dwPlayLength + dwFragLength > m_dwPlayBfrLength)
        {
            *pstCallFragment = m_pstFragBfr[m_dwFragBfrIndex];
            *pdwPlayLength = m_dwPlayLength;
            m_dwPlayLength = 0;
            return(PLAY_BUFFER_FULL);
        }

        // Copy fragment.
        if (m_sbSmpDataFmt == PCM_UBYTE)
        {
            //   memcpy((void *)&((uint8 *)m_pPlayBfr)[m_dwPlayLength], (void *)&((uint8 *)m_pWaveBfr)[lWaveBfrIndex],
            //               (size_t)dwFragLength*(size_t)abs(m_sbSmpDataFmt));
            uint8 * pPlay = &((uint8 *)m_pPlayBfr)[m_dwPlayLength];
            uint8 * pBlock = &((uint8 *)m_pWaveBfr)[lWaveBfrIndex];
            for (DWORD i = 0; i < dwFragLength; i++)
            {
                float fSample = (float)Round((float)((int8)(*pBlock++ - 128)));
                if (fSample > 127.F)
                {
                    fSample = 127.F;
                }
                else if (fSample < -128.F)
                {
                    fSample = -128.F;
                }
                *pPlay++ = (uint8)(fSample + 128.F);
            }
        }
        else     // PCM_2SSHORT
        {
            //   memcpy((void *)&((short *)m_pPlayBfr)[m_dwPlayLength], (void *)&((short *)m_pWaveBfr)[lWaveBfrIndex],
            //               (size_t)dwFragLength*(size_t)abs(m_sbSmpDataFmt));
            short * pPlay = &((short *)m_pPlayBfr)[m_dwPlayLength];
            short * pBlock = &((short *)m_pWaveBfr)[lWaveBfrIndex];
            for (DWORD i = 0; i < dwFragLength; i++)
            {
                float fSample = (float)Round((float)*pBlock++);
                if (fSample > 32767.F)
                {
                    fSample = 32767.F;
                }
                else if (fSample < -32768.F)
                {
                    fSample = -32768.F;
                }
                *pPlay++ = (short)fSample;
            }
        }
        m_dwPlayLength += dwFragLength;    // update playback length
        m_dwWarpIndex += dwFragLength;     // update scaling index

    }
    while (TRUE);
}

dspError_t CWaveWarp::FillPlayBuffer(FRAG_PARMS * pstCallData,
                                     ULONG dwWaveBlockLength,
                                     ULONG * pdwPlayLength, USHORT wNewSpeed)
{
//return when outbuffer full or beyond input buffer length
//initialize playlength to 0, dwWaveBlock to load offset, pdwFragIndex to 0
//!!DataFmt
//!!PlayLength = 0
//!!reset playindex in SetPlayBuffer(pPlay, wSpeed)
//!!reset playlength in SetPlayBuffer
//!!consolidate pdwCallData and dwWaveBlock?
//!!change FragStart in CreateObject to CallDataStart and subtract 1 from offset

// Validate buffer pointers.
    if (!m_pWaveBfr || !m_pPlayBfr)
    {
        return(Code(INVALID_PARM));
    }

    CProcessFragments * pFragments = m_pDoc->GetFragments();
    ULONG dwFragCount = pFragments->GetFragmentCount();
    ULONG dwFragBfrLength = pFragments->GetBufferLength();
    do
    {
        // Translate sample position back to unscaled waveform.
        ULONG dwWaveIndex = m_dwWarpStart + (ULONG)((float)m_dwWarpIndex*(float)m_wSpeed/100.F + 0.5F);

        // Find fragment containing this sample.
        ULONG dwPrevFragIndex = m_dwFragBlock + m_dwFragBfrIndex;
        ULONG dwFragBlockLength = min(dwFragBfrLength, dwFragCount - m_dwFragBlock);
        do
        {
            // scan through fragment buffer
            for (; m_dwFragBfrIndex < dwFragBlockLength; m_dwFragBfrIndex++)   // continues from last fragment checked
                if (dwWaveIndex >= m_pstFragBfr[m_dwFragBfrIndex].dwOffset &&
                        dwWaveIndex < m_pstFragBfr[m_dwFragBfrIndex].dwOffset + m_pstFragBfr[m_dwFragBfrIndex].wLength)
                {
                    break;
                }

            // if not found, load the next fragment block into the fragment buffer
            if (m_dwFragBfrIndex == dwFragBlockLength)
            {
                m_dwFragBlock += dwFragBlockLength;
                if (m_dwFragBlock != dwFragCount)
                {
                    // not the last fragment block
                    dwFragBlockLength = min(dwFragBfrLength, dwFragCount - m_dwFragBlock);  // recalc block length
                    pFragments->GetFragmentBlock(m_dwFragBlock);  // load next block
                    m_dwFragBfrIndex = 0;  // reset index into fragment buffer
                }
            }
            else
            {
                break;
            }

        }
        while (m_dwFragBlock < dwFragCount);   // loop through fragment blocks

        if (m_dwFragBlock == dwFragCount)
        {
            // no more fragment blocks
            pstCallData->dwOffset = m_pstFragBfr[dwFragBlockLength-1].dwOffset +
                                    m_pstFragBfr[dwFragBlockLength-1].wLength;
            pstCallData->wLength = 0;
            *pdwPlayLength =  m_dwPlayLength;
            return(DONE);
        }

        // Request reload if fragment not contained in waveform buffer.
        long lWaveBfrIndex = (long)m_pstFragBfr[m_dwFragBfrIndex].dwOffset - (long)pstCallData->dwOffset;
        ULONG dwFragLength =  m_pstFragBfr[m_dwFragBfrIndex].wLength;
        if (lWaveBfrIndex < 0 || (ULONG)lWaveBfrIndex + dwFragLength > dwWaveBlockLength)
        {
            // load starting at sample before zero crossing
            pstCallData->dwOffset = m_pstFragBfr[m_dwFragBfrIndex].dwOffset - (m_dwFragBfrIndex != 0);
            pstCallData->wLength = (USHORT)(m_pstFragBfr[m_dwFragBfrIndex].wLength + (m_dwFragBfrIndex != 0));
            *pdwPlayLength =  m_dwPlayLength;
            return(OUTSIDE_WAVE_BUFFER);  //!!last playback should be cut short in calling function
        }

        // If new fragment, estimate start and end times.
        if (m_dwFragBfrIndex != dwPrevFragIndex)
        {
            double dSmpDiff;
            if (m_sbSmpDataFmt == PCM_UBYTE)
            {
                // Initialize to first sample location after zero crossing.
                m_dFragStartTime = (double)m_pstFragBfr[m_dwFragBfrIndex].dwOffset;
                // Interpolate between last fragment and this one.
                if (lWaveBfrIndex)
                {
                    // Compute difference in sample values between last sample of previous fragment and first
                    // sample of current.
                    dSmpDiff = (double)((uint8 *)m_pWaveBfr)[lWaveBfrIndex] - (double)((uint8 *)m_pWaveBfr)[lWaveBfrIndex-1];
                    // If no difference, assume zero crossing is halfway between samples.
                    if (dSmpDiff == 0.)
                    {
                        m_dFragStartTime -= 0.5;
                    }
                    // Otherwise, estimate zero crossing using a linear interpolation method.
                    else
                    {
                        m_dFragStartTime -= (double)(((uint8 *)m_pWaveBfr)[lWaveBfrIndex] - 128) / dSmpDiff;
                    }

                }
                // Initialize to first sample location beyond end of fragment.
                m_dFragEndTime = (double)(m_pstFragBfr[m_dwFragBfrIndex].dwOffset + m_pstFragBfr[m_dwFragBfrIndex].wLength);
                // Interpolate between last sample of current fragment and first sample of next.
                lWaveBfrIndex += (long)dwFragLength;
                if (lWaveBfrIndex < (long)dwWaveBlockLength && m_dwFragBfrIndex < dwFragCount)
                {
                    // Compute difference in sample values between last sample of previous fragment and first
                    // sample of current.
                    dSmpDiff = (double)((uint8 *)m_pWaveBfr)[lWaveBfrIndex] - (double)((uint8 *)m_pWaveBfr)[lWaveBfrIndex - 1];
                    // If no difference, assume zero crossing is halfway between samples.
                    if (dSmpDiff == 0.)
                    {
                        m_dFragEndTime -= 0.5;
                    }
                    // Otherwise, estimate zero crossing using a linear interpolation method.
                    else
                    {
                        m_dFragEndTime -= (double)(((uint8 *)m_pWaveBfr)[lWaveBfrIndex] - 128) / dSmpDiff;
                    }
                }
            }
            else     // PCM_2SSHORT
            {
                // Initialize to first sample location after zero crossing.
                m_dFragStartTime = (double)m_pstFragBfr[m_dwFragBfrIndex].dwOffset;
                // Interpolate between last fragment and this one.
                if (lWaveBfrIndex)
                {
                    // Compute difference in sample values between last sample of previous fragment and first
                    // sample of current.
                    dSmpDiff = (double)((short *)m_pWaveBfr)[lWaveBfrIndex] - (double)((short *)m_pWaveBfr)[lWaveBfrIndex-1];
                    // If no difference, assume zero crossing is halfway between samples.
                    if (dSmpDiff == 0.)
                    {
                        m_dFragStartTime -= 0.5;
                    }
                    // Otherwise, estimate zero crossing using a linear interpolation method.
                    else
                    {
                        m_dFragStartTime -= (double)((short *)m_pWaveBfr)[lWaveBfrIndex] / dSmpDiff;
                    }
                }
                // Initialize to first sample location beyond end of fragment.
                m_dFragEndTime = (double)(m_pstFragBfr[m_dwFragBfrIndex].dwOffset + m_pstFragBfr[m_dwFragBfrIndex].wLength);
                // Interpolate between last sample of current fragment and first sample of next.
                lWaveBfrIndex += (long)dwFragLength;
                if (lWaveBfrIndex < (long)dwWaveBlockLength && m_dwFragBfrIndex < dwFragCount)
                {
                    // Compute difference in sample values between last sample of previous fragment and first
                    // sample of current.
                    dSmpDiff = (double)((short *)m_pWaveBfr)[lWaveBfrIndex] - (double)((short *)m_pWaveBfr)[lWaveBfrIndex - 1];
                    // If no difference, assume zero crossing is halfway between samples.
                    if (dSmpDiff == 0.)
                    {
                        m_dFragEndTime -= 0.5;
                    }
                    // Otherwise, estimate zero crossing using a linear interpolation method.
                    else
                    {
                        m_dFragEndTime -= (double)((short *)m_pWaveBfr)[lWaveBfrIndex] / dSmpDiff;
                    }
                }
            }

            if (m_wSpeed != wNewSpeed)
            {
                // change speed
                m_dwWarpStart = m_pstFragBfr[m_dwFragBfrIndex].dwOffset;
                m_dwWarpIndex = 0;
                m_wSpeed = wNewSpeed;
            }
        }

        // Otherwise, resample current fragment.
        else
        {
            m_dSmpTime -= (m_dFragEndTime - m_dFragStartTime);
            m_dJitterFactor = fmod(m_dSmpTime, 1.);     // calculate fraction of sample period
        }

        dwPrevFragIndex = m_dwFragBfrIndex;
        //!! handle partial fragment in calling function by offsetting into play buffer before playback
        ULONG dwSamples = (ULONG)floor(m_dFragEndTime - m_dSmpTime) + 1;
        if (m_dwPlayLength + dwSamples > m_dwPlayBfrLength)
        {
            pstCallData->dwOffset = m_pstFragBfr[m_dwFragBfrIndex].dwOffset - (pstCallData->dwOffset != 0);
            pstCallData->wLength = (USHORT)(m_pstFragBfr[m_dwFragBfrIndex].wLength + (pstCallData->dwOffset != 0));
            *pdwPlayLength = m_dwPlayLength;
            m_dwPlayLength = 0;
            return(PLAY_BUFFER_FULL);
        }

        // Copy fragment.
        ULONG dwSmpIndex = (ULONG)floor(m_dSmpTime) - pstCallData->dwOffset;
        if (m_sbSmpDataFmt == PCM_UBYTE)
        {
            do
            {
                ((uint8 *)m_pPlayBfr)[m_dwPlayLength++] = (uint8)(Round((double)(((uint8 *)m_pWaveBfr)[dwSmpIndex] - 128) + m_dJitterFactor *
                        (double)(short)(((uint8 *)m_pWaveBfr)[dwSmpIndex + 1] -  //!!in buffer?
                                        ((uint8 *)m_pWaveBfr)[dwSmpIndex])) + 128.);
                dwSmpIndex++;
                m_dSmpTime += 1.;
            }
            while (m_dSmpTime < m_dFragEndTime);    //!! <=?
        }
        else     // PCM_2SSHORT
        {
            do
            {
                ((short *)m_pPlayBfr)[m_dwPlayLength++] = (short)Round((double)((short *)m_pWaveBfr)[dwSmpIndex] + m_dJitterFactor *
                        (double)(((short *)m_pWaveBfr)[dwSmpIndex + 1] -  //!!in buffer?
                                 ((short *)m_pWaveBfr)[dwSmpIndex]));
                dwSmpIndex++;
                m_dSmpTime += 1.;
            }
            while (m_dSmpTime < m_dFragEndTime);    //!! <=?
        }
        m_dwWarpIndex += dwSamples;     // update scaling index    //!!check this

    }
    while (TRUE);
}
