/////////////////////////////////////////////////////////////////////////////
// sa_p_gra.cpp:
// Implementation of the CProcessGrappl class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "sa_process.h"
#include "sa_p_grappl.h"
#include "funcs.h"
#include "AbstractPitchProcess.h"
#include "ScopedCursor.h"

//###########################################################################
// CProcessGrappl
// class to calculate grappl pitch for wave data.

CProcessGrappl::CProcessGrappl(Context& context) : CAbstractPitchProcess(context) {
    // initialize algorithm parameters
    m_dAvgPitch = 0.;
}

/***************************************************************************/
// CProcessGrappl::Process Processing grappl pitch data
// The processed grappl pitch data is stored in a temporary file.
// To create it helper functions of the base class are used.
// While processing a process bar, placed on the status bar, has to be updated.
// The level tells which processing level this process has been called, start
// process start on which processing percentage this process starts (for the
// progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start.
// The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessGrappl::Process(void* pCaller, int nProgress, int nLevel) {
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);   // process canceled
    }
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);           // data is already ready
    }
    //TRACE(_T("Process: CProcessGrappl --------\n"));
    DWORD dwDataSize = model.GetDataSize();             // raw data size for all channels
    if (dwDataSize == 0) {
        return Exit(PROCESS_NO_DATA);                   // error, no valid data
    }

    if (nLevel < 0) {                                   // previous processing error
        if ((nLevel == PROCESS_CANCELED)) {
            // set your own cancel flag
            CancelProcess();
        }
        return MAKELONG(nLevel, nProgress);
    }

    // start grappl process
    CScopedCursor cursor(target);
    if (!StartProcess(pCaller, PROCESSGRA)) { 
        // memory allocation failed
        EndProcess();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    // if file has not been created
    if (wcslen(GetProcessFileName())==0) {
        // create the temporary grappl pitch file
        // creation error?
        if (!CreateTempFile(_T("GRA"))) {
            // end data processing
            EndProcess();
            SetDataInvalid();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
        // initialize parameters
        m_dwDataPos = 0;
        m_nMinValue = SHRT_MAX;
        m_CalcParm.sampfreq = (int32)model.GetSamplesPerSec();
        m_CalcParm.eightbit = (int16)(!model.Is16Bit());
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
        if (GetBufferSize() < (DWORD)nWorkSpace) {
            // buffer too small
            TCHAR szText[6];
            swprintf_s(szText, _countof(szText), _T("%u"), nWorkSpace);
            app.GrapplErrorMessage( szText);
            return Exit(PROCESS_ERROR);                 // error, buffer too small
        }
        // init grappl
        if (!grapplInit(m_lpBuffer, &m_CalcParm)) {
            return Exit(PROCESS_ERROR);                 // error, processing failed
        }
    } else {
        // open file to append data
        if (!OpenFileToAppend()) {
            EndProcess(); // end data processing
            SetDataInvalid();
            return MAKELONG(PROCESS_ERROR, nProgress);
        }
    }

    bool alldone = false;
    bool nomore = false;

    // get block size
    DWORD dwBlockSize = 0x10000 - model.GetBlockAlign(true); // 64k - 1
    if (GetBufferSize() < dwBlockSize) {
        dwBlockSize = GetBufferSize();
    }

    trace("dwBlockSize=%d\n",dwBlockSize);
    trace("dwDataSize=%d\n", dwDataSize);
    trace("block align=%d\n",model.GetBlockAlign(true));

    int iterations = 0;

    BPTR pBlockStart;
    // start processing
    while (m_dwDataPos < dwDataSize) {
        //TRACE("m_dwDataPos=%d\n",m_dwDataPos);

        // get raw data block
        // this should be single channel data
        pBlockStart = model.GetWaveData(m_dwDataPos, TRUE);  // get pointer to data block
        if (pBlockStart==NULL) {
            return Exit(PROCESS_ERROR);                         // error, reading failed
        }

        // increment the position pointer
        m_dwDataPos += dwBlockSize;
        // if it is beyond the end of the data, recalculate the amount of data
        // remaining in dwBlockSize and signal that we will end
        if (m_dwDataPos >= dwDataSize) {
            dwBlockSize -= (m_dwDataPos - dwDataSize);
            nomore = TRUE;
        }

        // set grappl input buffer
        uint16 length = (WORD)(dwBlockSize / model.GetBlockAlign(true));
        //TRACE("grappl length %d\n",length);
        if (!grapplSetInbuff((pGrappl)m_lpBuffer, (pGrappl)pBlockStart, length, nomore)) {
            return Exit(PROCESS_ERROR);             // error, processing failed
        }

        // process
        pGrappl_res pResults;
        int16 nresults;
        DWORD dwPitchCount = 0;
        while (grapplGetResults((pGrappl)m_lpBuffer, &pResults, &nresults, &alldone)) {
            // get max and min values and save the results
            for (int16 nLoop = 0; nLoop < nresults; nLoop++) {
                // find min and max values
                if (pResults->fsmooth16 >= 0) {
                    // adjust result to precision multiplier format
                    pResults->fsmooth16 = (int16)(((long)pResults->fsmooth16 * (long)PRECISION_MULTIPLIER + 8L)/ 16L);
                    if (pResults->fsmooth16 > m_nMaxValue) {
                        m_nMaxValue = pResults->fsmooth16;    // save maximum value
                    }
                    if (pResults->fsmooth16 < m_nMinValue) {
                        m_nMinValue = pResults->fsmooth16;    // save minimum value
                    }
                } else {
                    pResults->fsmooth16 = -1;    // set this point as unset
                }
                // keep a running average of the pitch
                if (pResults->fsmooth16 > 0) {
                    // average in voiced regions only
                    dwPitchCount++;
                    m_dAvgPitch = ((double)(dwPitchCount-1)*m_dAvgPitch + (double)pResults->fsmooth16/PRECISION_MULTIPLIER) / (double)dwPitchCount;
                }
                // write one result of the processed grappl pitch data
                try {
                    Write((BPTR)&pResults->fsmooth16, sizeof(int16));
                } catch (CFileException * e) {
                    // error writing file
                    app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
					// error, writing failed
					e->Delete();
					return Exit(PROCESS_ERROR);
                }
                pResults++;
            }
        }

        // set progress bar
        SetProgress(nProgress + (int)(100 * m_dwDataPos / dwDataSize / (DWORD)nLevel));
        if (IsCanceled()) {
            return Exit(PROCESS_CANCELED);      // process canceled
        }
        if (alldone) {
            break;
        }

        iterations++;
    }

    // calculate the actual progress
    nProgress = nProgress + (int)(100 / nLevel);
    // close the temporary file and read the status
    CloseTempFile(); // close the file
    if (alldone) {
        if (GetDataSize() < 2) {
            return Exit(PROCESS_ERROR);    // error, not enough data
        }
        SetStatusFlag(PROCESS_NO_PITCH, m_nMinValue == SHRT_MAX);
    }
    EndProcess((nProgress >= 95)); // end data processing
    if (nomore || alldone) {
        SetDataReady();
    }
    return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcessGrappl::IsVoiced  Checks if waveform section at offset is voiced.
/***************************************************************************/
BOOL CProcessGrappl::IsVoiced( DWORD dwWaveOffset) {
    DWORD nSmpSize = model.GetSampleSize();
    DWORD dwSmpOffset = dwWaveOffset / nSmpSize;
    DWORD dwPitchBlock = m_dwBufferOffset;
    BOOL bDone;
    Process(this );
    BOOL bVoiced = (GetProcessedData( dwSmpOffset / Grappl_calc_intvl, &bDone) > 0);
    if (!bDone) {
        return FALSE;
    }
    if (dwPitchBlock != UNDEFINED_OFFSET && m_dwBufferOffset != dwPitchBlock) {
        GetProcessedData( dwPitchBlock/sizeof(short), TRUE);    // restore pitch buffer
    }
    return(bVoiced);
}

SGrapplParms & CProcessGrappl::GetCalcParms() {
    return m_CalcParm;
}

double CProcessGrappl::GetAveragePitch() {
    return m_dAvgPitch;
}

void CProcessGrappl::SetDataInvalid() {
    CProcess::SetDataInvalid();
}

void CProcessGrappl::Dump(LPCSTR ofilename) {
    return;
    FILE * ofile = NULL;
    errno_t err = fopen_s(&ofile, ofilename, "w");
    fprintf(ofile, "autopitch data\n");
    fprintf(ofile, "uncertainty = %f\n", GetUncertainty(m_dAvgPitch));
    fprintf(ofile, "average pitch = %f", m_dAvgPitch);
    {
        string ifilename = Utf8(GetProcessFileName());
        FILE * ifile = NULL;
        err = fopen_s(&ifile, ifilename.c_str(), "r");
        int16 buffer = 0;
        while (true) {
            int read = fread(&buffer, 1, sizeof(int16), ifile);
            if (read<sizeof(int16)) {
                break;
            }
            fprintf(ofile, "%d ",buffer);
        }
        if (!feof(ifile)) {
            fprintf(ofile, "premature termination");
        }
        fflush(ifile);
        fclose(ifile);
    }
    fprintf(ofile, "\n");
    fflush(ofile);
    fclose(ofile);
}
