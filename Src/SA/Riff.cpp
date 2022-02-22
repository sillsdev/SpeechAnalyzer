/////////////////////////////////////////////////////////////////////////////
// Riff.cpp:
// Implementation of the CRiff class.
//
// Author: Steve MacLean
// copyright 2000-2001 JAARS Inc. SIL
//
// Revision History
//   03/08/2001 SDM Initial version very rudimentary Riff support expect to collect
//                  various riff functionality from around SA.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa.h"
#include "Riff.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BOOL CRiff::NewWav(LPCTSTR pszPathName, PCMWAVEFORMAT & pcm, LPCTSTR szRawDataPath) {
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application

    // open file
    HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READWRITE | MMIO_EXCLUSIVE);
    if (!hmmioFile) {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_FILEOPEN, pszPathName);
        return FALSE;
    }

    /* Create the output file RIFF chunk of form type 'WAVE'.
    */
    MMCKINFO riffChunk;  // chunk info. for output RIFF chunk
    riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioCreateChunk(hmmioFile, &riffChunk, MMIO_CREATERIFF) != 0) {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    /* We are now descended into the 'RIFF' chunk we just created.
    * Now create the 'fmt ' chunk. Since we know the size of this chunk,
    * specify it in the MMCKINFO structure so MMIO doesn't have to seek
    * back and set the chunk size after ascending from the chunk.
    */
    MMCKINFO fmtChunk;      // info. for a chunk in output file
    fmtChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    fmtChunk.cksize = sizeof(PCMWAVEFORMAT);  // we know the size of this ck.
    if (mmioCreateChunk(hmmioFile, &fmtChunk, 0) != 0) {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    /* Write the PCMWAVEFORMAT structure to the 'fmt ' chunk.
    */
    if (mmioWrite(hmmioFile, (BPTR) &pcm, sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT)) {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    /* Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk.
    */
    if (mmioAscend(hmmioFile, &fmtChunk, 0) != 0) {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    /* Create the 'data' chunk that holds the waveform samples.
    */
    MMCKINFO dataChunk;     // info. for a chunk in output file
    dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioCreateChunk(hmmioFile, &dataChunk, 0) != 0) {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    dataChunk.dwFlags = MMIO_DIRTY; // force ascend to update the chunk length
    riffChunk.dwFlags = MMIO_DIRTY;

    {
        CFile * pFile = NULL;
        char * pCopyBuf = NULL;
        try {
            pFile = new CFile(szRawDataPath, CFile::modeRead);
            const int BUFFER_SIZE =4096;
            pCopyBuf = new char [BUFFER_SIZE];

            long nBytesRead = 0;
            do {
                nBytesRead = long(pFile->Read(pCopyBuf, BUFFER_SIZE));
                if (nBytesRead && (mmioWrite(hmmioFile, pCopyBuf, nBytesRead) != nBytesRead)) {
                    throw IDS_ERROR_WRITEDATACHUNK;
                }
            } while (nBytesRead == BUFFER_SIZE);

            delete pFile;
            delete [] pCopyBuf;

        } catch (...) {
            if (pFile) {
                delete pFile;
            }
            if (pCopyBuf) {
                delete [] pCopyBuf;
            }
            // error writing data chunk
            pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
            mmioClose(hmmioFile, 0);
            return FALSE;
        }
    }

    // At file end what is file position
    MMIOINFO mmioinfo;
    if (mmioGetInfo(hmmioFile,&mmioinfo,0)) {
        // error writing RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    // get out of 'data' chunk
    if (mmioAscend(hmmioFile, &dataChunk, 0)) {
        // error writing data chunk
        pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    // get out of 'RIFF' chunk, to write RIFF size
    if (mmioAscend(hmmioFile, &riffChunk, 0)) {
        // error writing RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    if (!mmioClose(hmmioFile, 0)) { // close file
        // Set File Length ...
        // SDM 1.5Test10.2
        CFile WaveFile(pszPathName,CFile::modeReadWrite);
        WaveFile.SetLength(mmioinfo.lDiskOffset+2);
    }

    return TRUE;
}

