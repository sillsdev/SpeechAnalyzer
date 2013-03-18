/////////////////////////////////////////////////////////////////////////////
// sa_proc.cpp:
// Implementation of the CProcess
//                       CAreaDataProcess classes
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"
#include "mainfrm.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "FileUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcess
// Base class for all data processing classes. Does all jobs, common to all
// data processing derived classes.

/////////////////////////////////////////////////////////////////////////////
// CProcess construction/destruction/creation

/***************************************************************************/
// CProcess::CProcess Constructor
/***************************************************************************/
CProcess::CProcess() {
    memset(this, 0, sizeof(CProcess));
    m_pFile = NULL;
    DeleteProcessFileName(); // no file name
    m_lpBuffer = NULL;
    m_nMaxValue = 0;
    m_nMinValue = 0;
    m_dwBufferOffset = UNDEFINED_OFFSET; // buffer undefined, force buffer reload
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    if (pMain) {
        m_dwBufferSize = GetBufferSize();
    } else {
        m_dwBufferSize = 0;
    }
    SetStatus(PROCESS_IDLE);
}

/***************************************************************************/
// CProcess::~CProcess Destructor
/***************************************************************************/
CProcess::~CProcess() {
    // delete the temporary file
    DeleteTempFile();
    if (m_pFile) {
        delete m_pFile;
    }
    // free the buffer memory
    if (m_lpBuffer) {
        delete [] m_lpBuffer;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CProcess helper functions

CProgressStatusBar * CProcess::pGetStatusBar() {
    // get pointer to status bar
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    CProgressStatusBar * pStatusBar = (CProgressStatusBar *)pMainFrame->GetProgressStatusBar();
    return pStatusBar;
}

/***************************************************************************/
// CProcess::StartProcess Starting the process
// Called by the derived classes to launch the process. The process status
// bar will be made visible instead of the data status bar. The status bar
// is placed and filled up (if it's not already there) and the data buffer
// for a processed data block is allocated (if not yet done). The parameter
// nProcessID is a text ID, which identifies the running process and enables
// the process status bar to display the right text for this process. If the
// ID is -1, a default text is displayed. A buffer of dwBufferSize bytes is
// allocated.
/***************************************************************************/
bool CProcess::StartProcess(void * pCaller, int nProcessID,
                            DWORD dwBufferSize) {
    TRACE(_T("CProcess::StartProcess ID:%d\n"), nProcessID);

    // get pointer to status bar
    CProgressStatusBar * pStatusBar = pGetStatusBar();
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();

    if (pMain) {
        m_dwBufferSize = GetBufferSize();
    } else {
        m_dwBufferSize = 0;
    }

    // no previous process owner? you're the first, show to progress bar
    if (!pStatusBar->GetProcessOwner()) {
        pMain->ShowDataStatusBar(FALSE);    // show the progress status bar
    }

    pStatusBar->SetProcessOwner(this, pCaller, nProcessID); // set the process owner
    if (dwBufferSize) {
        // allocate global buffer for the processed data
        if (!m_lpBuffer) { // not yet allocated
            m_lpBuffer = new char[dwBufferSize];
            if (!m_lpBuffer) {
                // memory lock error
                ErrorMessage(IDS_ERROR_MEMALLOC);
                return false;
            }
        }
    }
    return true;
}

bool CProcess::StartProcess(void * pCaller, int nProcessID, BOOL bBuffer) {
    DWORD dwBufferSize = bBuffer ? GetBufferSize():0;
    return StartProcess(pCaller, nProcessID, dwBufferSize);
}

/***************************************************************************/
// CProcess::SetProgress
/***************************************************************************/
void CProcess::SetProgress(int nPercent) {
    CProgressStatusBar * pStatusBar = pGetStatusBar();

    pStatusBar->SetProgress(nPercent);
}

/***************************************************************************/
// CProcess::ErrorMessage
/***************************************************************************/
void CProcess::ErrorMessage(UINT nTextID, LPCTSTR pszText1, LPCTSTR pszText2) {
    CSaApp * pApp = (CSaApp *) AfxGetApp();

    pApp->ErrorMessage(nTextID, pszText1, pszText2);
}

/***************************************************************************/
// CProcess::EndProcess Ending the process
// Called by the derived classes to end the process. If bProcessBar is TRUE,
// the data status bar will be shown again instead of the process status bar.
/***************************************************************************/
void CProcess::EndProcess(BOOL bProcessBar) {
    TRACE(_T("CProcess::EndProcess\n"));
    if (bProcessBar) {
        CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
        pMainFrame->ShowDataStatusBar(TRUE); // show the data status bar
        CProgressStatusBar * pStatusBar = pGetStatusBar();
        pStatusBar->SetProcessOwner(NULL, NULL); // reset the process owner
    }
}

/***************************************************************************/
// CProcess::GetProcessedData Read processed data from temporary file
// Reads a block of processed data from the temporary file into the processed
// data buffer and returns the pointer to the data. The user gives the data
// offset (data number) in the file to tell what data he needs. If bBlockBegin
// is set, this given offset data will always be at the top of the data block
// (where the returned pointer points to). If bBlockBegin is not set, the data
// block is not reread, if the given offset data is already somewhere in the
// buffer, and only the actual pointer to the data block will be returned.
// Processed data is always 16bit data (dwOffset is a word index) but the
// buffer offset contains a byte index! pCaller is for further use.
/***************************************************************************/
void * CProcess::GetProcessedData(DWORD dwOffset, BOOL bBlockBegin) {
    if (dwOffset == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }

    DWORD dwByteOffset = dwOffset * 2;

    if (((!bBlockBegin) && ((dwByteOffset >= m_dwBufferOffset) && (dwByteOffset < (m_dwBufferOffset + GetProcessBufferSize()))))
            || ((bBlockBegin) && (m_dwBufferOffset == dwByteOffset))) {
        // this data is actually in buffer
        return m_lpBuffer; // return pointer to data
    } else { // new data block has to be read
        if (bBlockBegin) {
            m_dwBufferOffset = dwByteOffset; // given offset is the first sample in data block
        } else {
            m_dwBufferOffset = dwByteOffset - (dwByteOffset % GetProcessBufferSize()); // new block offset
        }
        // open the temporary file
        if (!Open(GetProcessFileName(), CFile::modeRead | CFile::shareExclusive)) {
            // error opening file
            ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
            return NULL;
        }
        // find the right position in the data
        if (m_dwBufferOffset != 0L) {
            try {
                m_pFile->Seek(m_dwBufferOffset, CFile::begin);
            } catch (CFileException e) {
                // error seeking file
                ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
                SetDataInvalid();// close the temporary file
                return NULL;
            }
        }
        // read the processed data block
        try {
            m_pFile->Read((HPSTR)m_lpBuffer, GetProcessBufferSize());
        } catch (CFileException e) {
            // error reading file
            ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
            SetDataInvalid();// close the temporary file
            return NULL;
        }
        CloseTempFile(FALSE); // close the temporary file
        // return the new data pointer
        return m_lpBuffer;
    }
}

/***************************************************************************/
// CProcess::GetProcessedData Read processed data from temporary file
// Reads one data of processed data from the temporary file and returns it.
// The user gives the data offset (data number) in the file to tell what
// data he needs. He also gives a pointer to a BOOLEAN result variable. If
// this will be FALSE after the operation, an error occured.
// Processed data is always 16bit data (dwOffset is a word index) but the
// buffer offset contains a byte index! pCaller is for further use.
/***************************************************************************/
int CProcess::GetProcessedData(DWORD dwOffset, BOOL * pbRes) {
    if (dwOffset == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }

    DWORD dwByteOffset = dwOffset * 2;
    if ((dwByteOffset >= m_dwBufferOffset) && (dwByteOffset < (m_dwBufferOffset + GetProcessBufferSize()))) {
        // this data is actually in buffer
        *pbRes = TRUE;  // set operation result
        return *reinterpret_cast<short int *>(m_lpBuffer + (dwByteOffset - m_dwBufferOffset)); // return data
    } else { // new data block has to be read
        m_dwBufferOffset = dwByteOffset - (dwByteOffset % GetProcessBufferSize()); // new block offset
        // open the temporary file
        if (!Open(GetProcessFileName(), CFile::modeRead | CFile::shareExclusive)) {
            // error opening file
            ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
            *pbRes = FALSE; // set operation result
            m_dwBufferOffset = UNDEFINED_OFFSET;
            return 0;
        }
        // find the right position in the data
        if (m_dwBufferOffset != 0L) {
            try {
                m_pFile->Seek(m_dwBufferOffset, CFile::begin);
            } catch (CFileException e) {
                // error seeking file
                ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
                *pbRes = FALSE; // set operation result
                m_dwBufferOffset = UNDEFINED_OFFSET;
                CloseTempFile(FALSE);
                return 0;
            }
        }
        // read the processed data block
        try {
            m_pFile->Read((HPSTR)m_lpBuffer, GetProcessBufferSize());
        } catch (CFileException e) {
            // error reading file
            ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
            SetDataInvalid();// close the temporary file
            *pbRes = FALSE; // set operation result
            m_dwBufferOffset = UNDEFINED_OFFSET;
            return 0;
        }
        CloseTempFile(FALSE); // close the temporary file
        // return the data
        short int * lpData = (short int *)m_lpBuffer; // cast buffer pointer
        *pbRes = TRUE;  // set operation result
        return *(lpData + (dwOffset - (m_dwBufferOffset / 2))); // return data
    }
}

/***************************************************************************/
// CProcess::GetProcessedWaveData return pointer to processed wave source
// Reads a block of processed data from the temporary file into the processed
// data buffer and returns the pointer to the data. The user gives the data
// offset (data number) in the file to tell what data he needs. If bBlockBegin
// is set, this given offset data will always be at the top of the data block
// (where the returned pointer points to). If bBlockBegin is not set, the data
// block is not reread, if the given offset data is already somewhere in the
// buffer, and only the actual pointer to the data block will be returned.
// The data offset contains a byte index.
/***************************************************************************/
HPSTR CProcess::GetProcessedWaveData(DWORD dwOffset, BOOL bBlockBegin) {
    if (GetProcessFileName() == NULL || GetProcessFileName()[0] == 0) {
        return NULL;
    }

    if (dwOffset == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }

    if (((!bBlockBegin) && ((dwOffset >= m_dwBufferOffset) && (dwOffset < m_dwBufferOffset + GetProcessBufferSize())))
            || ((bBlockBegin) && (m_dwBufferOffset == dwOffset))) {
        // this data is actually in buffer
        return m_lpBuffer; // return pointer to data
    } else { // new data block has to be read
        if (bBlockBegin) {
            m_dwBufferOffset = dwOffset;    // given offset ist first sample in data block
        } else {
            m_dwBufferOffset = dwOffset - (dwOffset % GetProcessBufferSize());    // new block offset
        }
        // open the temporary file
        if (!Open(GetProcessFileName(), CFile::modeRead | CFile::shareExclusive)) {
            // error opening file
            ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
            return NULL;
        }
        // find the right position in the data
        if (m_dwBufferOffset != 0L) {
            try {
                m_pFile->Seek(m_dwBufferOffset, CFile::begin);
            } catch (CFileException e) {
                // error seeking file
                ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
                SetDataInvalid();// close the temporary file
                return NULL;
            }
        }
        // read the processed data block
        try {
            m_pFile->Read((HPSTR)m_lpBuffer, GetProcessBufferSize());
        } catch (CFileException e) {
            // error reading file
            ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
            SetDataInvalid();// close the temporary file
            return NULL;
        }
        CloseTempFile(FALSE); // close the temporary file
        // return the new data pointer
        return m_lpBuffer;
    }
}

/***************************************************************************/
// CProcess::GetProcessedObject returns a pointer to requested data object
// uses data buffer to optimize requests
/***************************************************************************/
void * CProcess::GetProcessedObject(DWORD dwIndex, size_t sObjectSize, BOOL bReverse) {
    if (dwIndex == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }
    DWORD dwByteOffset = dwIndex * sObjectSize;
    return GetProcessedDataBlock(dwByteOffset, sObjectSize, bReverse);
}

/***************************************************************************/
// CProcess::GetProcessedDataBlock returns a pointer to requested data object
// uses data buffer to optimize requests
/***************************************************************************/
void * CProcess::GetProcessedDataBlock(DWORD dwByteOffset, size_t sObjectSize, BOOL bReverse) {
    //TRACE("GetProcessedDataBlock %s[%d] %d %d %d\n",__FILE__,__LINE__,dwByteOffset, sObjectSize, bReverse);

    if (dwByteOffset == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }

    if ((dwByteOffset >= m_dwBufferOffset) && ((dwByteOffset+sObjectSize) < (m_dwBufferOffset + GetProcessBufferSize()))) {
        // this data is actually in buffer
        return m_lpBuffer + (dwByteOffset - m_dwBufferOffset); // return pointer to data
    } else { // new data block has to be read
        m_dwBufferOffset = dwByteOffset; // given offset is the first sample in data block
        if (bReverse) {
            // since we are traversing the file in reverse
            // load buffer so that object is biased to end of buffer
            m_dwBufferOffset = dwByteOffset + sObjectSize;
            if (m_dwBufferOffset > GetProcessBufferSize()) {
                m_dwBufferOffset -= GetProcessBufferSize();
            } else {
                m_dwBufferOffset = 0;
            }
        }

        // open the temporary file
        if (!Open(GetProcessFileName(), CFile::modeRead | CFile::shareExclusive)) {
            // error opening file
            ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
            return NULL;
        }
        // find the right position in the data
        if (m_dwBufferOffset != 0L) {
            try {
                m_pFile->Seek(m_dwBufferOffset, CFile::begin);
            } catch (...) {
                // error seeking file
                ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
                SetDataInvalid();// close the temporary file
                return NULL;
            }
        }
        // read the processed data block
        try {
            m_pFile->Read((HPSTR)m_lpBuffer, GetProcessBufferSize());
        } catch (...) {
            // error reading file
            ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
            SetDataInvalid();// close the temporary file
            return NULL;
        }
        CloseTempFile(FALSE); // close the temporary file
        // return the new data pointer
        return m_lpBuffer + (dwByteOffset - m_dwBufferOffset); // return pointer to data
    }
}

/***************************************************************************/
// CProcess::PropertiesDialog Calls the properties dialog for this process
/***************************************************************************/
int CProcess::PropertiesDialog() {
    return AfxMessageBox(IDS_DEFAULTPROPERTIES, MB_OK, 0);
}

/***************************************************************************/
// CProcess::CreateTempFile Create a temporary file
// Since the processed data will be stored in a temporary file, the file has
// to be created first.
/***************************************************************************/
BOOL CProcess::CreateTempFile(TCHAR * szName) {
    DeleteTempFile();

    if (!CreateTempFile(szName, &m_fileStatus)) {
        // error
        SetDataInvalid();// close the temporary file
        return FALSE;
    }
    m_dwBufferOffset = UNDEFINED_OFFSET; // buffer undefined, force buffer reload
    return TRUE;
}

BOOL CProcess::CreateTempFile(TCHAR * szName, CFileStatus * pFileStatus) {
    TCHAR szTempPath[_MAX_PATH];
    GetTempFileName(szName, szTempPath, _countof(szTempPath));
    // create and open the file
    if (!Open(szTempPath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive)) {
        // error opening file
        ErrorMessage(IDS_ERROR_OPENTEMPFILE, szTempPath);
        return FALSE;
    }

    // read file status
    if (!CFile::GetStatus(szTempPath, *pFileStatus)) {
        // error reading file status
        ErrorMessage(IDS_ERROR_OPENTEMPFILE, szTempPath);
        return FALSE;
    }

    return TRUE;
}

BOOL CProcess::Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException * pError) {
    ASSERT(m_pFile == NULL);
    ASSERT(lpszFileName);
    ASSERT(lpszFileName[0] != 0);

    if (m_pFile) {
        delete m_pFile;    // We shouldn't really get here...
    }

    m_pFile = new CFile;

    return m_pFile->Open(lpszFileName, nOpenFlags, pError);

}

/***************************************************************************/
// CProcess::CreateAuxTempFile Create an auxilliary temporary file
// This is used for temp files whose contents are not the primary
// data for this process.
/***************************************************************************/
BOOL CProcess::CreateAuxTempFile(TCHAR * szName, CFile * pFile, CFileStatus * pFileStatus) {
    if (!pFile) {
        pFile = new CFile();
    }
    if (!pFileStatus) {
        pFileStatus = new CFileStatus;
    }

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    TCHAR szTempPath[_MAX_PATH];
    GetTempFileName(szName, szTempPath, _countof(szTempPath));
    // create and open the file
    if (!pFile->Open(szTempPath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive)) {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, (LPTSTR)szTempPath);
        return Exit(PROCESS_ERROR);
    }
    // read file status
    if (!CFile::GetStatus(szTempPath, *pFileStatus)) {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, (LPTSTR)szTempPath);
        return Exit(PROCESS_ERROR);
    }

    return TRUE;
}

/***************************************************************************/
// CProcess::OpenFileToAppend  Open temp file and set up for appending
/***************************************************************************/
BOOL CProcess::OpenFileToAppend() {
    if (!Open(GetProcessFileName(), CFile::modeReadWrite | CFile::shareExclusive)) {
        // error opening file

        ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
        return FALSE;
    }

    try {
        m_pFile->SeekToEnd();
    } catch (CFileException e) {
        // error writing file
        ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
        SetDataInvalid();// close the temporary file
        return FALSE;
    }

    return TRUE;
}

/***************************************************************************/
// CProcess::CloseTempFile Close the temporary file
/***************************************************************************/
void CProcess::CloseTempFile(BOOL bUpdateStatus) {
    // close the temporary file
    if (m_pFile) {
        delete m_pFile;
        m_pFile = NULL;
    }
    if (bUpdateStatus) {
        CSaString path = GetProcessFileName();
        CFile::GetStatus(path, m_fileStatus); // read the status
    }
}

/***************************************************************************/
// CProcess::DeleteTempFile Delete the temporary file
/***************************************************************************/
void CProcess::DeleteTempFile() {
    CloseTempFile(FALSE); // close the temporary file
    RemoveFile(GetProcessFileName());
    DeleteProcessFileName();
    m_fileStatus.m_size = 0;
}

/***************************************************************************/
// CProcess::Exit Exit on Error
// Standard exit function if an error occured.
/***************************************************************************/
long CProcess::Exit(int nError) {
    SetDataInvalid();
    EndProcess(); // end data processing
    EndWaitCursor();
    return MAKELONG(nError, 100);
}

/***************************************************************************/
// CProcess::WriteDataBlock Write data block into temporary file
// Writes one block of processed data into the temporary file (overwrite,
// append). The caller has to deliver the start position where to store the
// data in the file, a pointer to the data buffer with the data to store and
// the size of the data block, he wants to store.
// Position and length are array indices with object size nElementSize
/***************************************************************************/
BOOL CProcess::WriteDataBlock(DWORD dwPosition, HPSTR lpData, DWORD dwDataLength, size_t nElementSize) {
    // open the temporary file
    BOOL bClose = (m_pFile == NULL);
    if (!m_pFile && !Open(GetProcessFileName(), CFile::modeReadWrite | CFile::shareExclusive)) {
        // error opening file
        ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
        return FALSE;
    }
    // find the right position in the data
    try {
        m_pFile->Seek(dwPosition * nElementSize, CFile::begin);
    } catch (CFileException e) {
        // error seeking file
        ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
        SetDataInvalid();// close the temporary file
        return FALSE;
    }
    // write the data block from the buffer
    try {
        Write((HPSTR)lpData, dwDataLength * nElementSize);
    } catch (CFileException e) {
        // error writing file
        ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
        SetDataInvalid();// close the temporary file
        return FALSE;
    }
    if (bClose) {
        CloseTempFile();    // close the temporary file
    }
    return TRUE;
}

/***************************************************************************/
// CProcess::SmoothData Smooth data filter
// Filters processed data nTimes times. It does a simple double-pass
// smoothing with a zero-phase low-pass filter. A zero phase filter does not
// affect the phase of the signals frequency components, but modifies only
// their magnitudes. Also, in contrast to a linear phase filter, it intro-
// duces no group delay. However, it does require two passes on data con-
// tained. It would be more efficient to use a linear phase filter,
// adjusting the output buffer pointer at the end of processing to zero out
// the group delay. The filter should also be tied to the sampling frequency.
// For this purpose, a bank of filters should be designed and the appropriate
// one selected for the specified sampling frequency.
// Processed data will be processed block for block and restored in the
// temporary file of the processed data. Processed data is always 16bit!
/***************************************************************************/
BOOL CProcess::SmoothData(int nTimes) {
    // allocate second global buffer for the smoothed data
    HANDLE hSmoothData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, GetProcessBufferSize()); // allocate memory
    if (!hSmoothData) {
        // memory allocation error
        ErrorMessage(IDS_ERROR_MEMALLOC);
        return FALSE;
    }
    HPSTR lpSmoothData = (HPSTR)::GlobalLock(hSmoothData); // lock memory
    if (!lpSmoothData) {
        // memory lock error
        ErrorMessage(IDS_ERROR_MEMLOCK);
        ::GlobalFree(hSmoothData);
        return FALSE;
    }
    LONG dwDataPos; // current data position in unsmoothed data
    int nData; // current data
    UINT nDiv, nRes; // helper variables
    BOOL bRes = TRUE; // operation result
    DWORD dwDataSize = GetDataSize(); // get data size
    while (nTimes--) {
        // first smooth backwards
        DWORD dwSmoothPos = dwDataSize % (GetProcessBufferSize() / 2); // current data position in smoothed data buffer
        DWORD dwBlockSize = dwSmoothPos; // save the block size
        short int * pSmoothData = (short int *)lpSmoothData; // pointer to processed data
        pSmoothData += ((dwDataSize - 1) % (GetProcessBufferSize() / 2)); // set buffer pointer
        for (dwDataPos = dwDataSize - 1; dwDataPos >= 0; dwDataPos--) {
            nData = GetProcessedData(dwDataPos, &bRes);
            if (!bRes) {
                break;
            }
            if (nData >= 0) {
                nDiv = 3;
                nRes = (UINT)(nData * 3);
                if ((dwDataPos - 1) >= 0) {
                    nData = GetProcessedData(dwDataPos - 1, &bRes);
                    if (!bRes) {
                        break;
                    }
                    if (nData >= 0) {
                        nDiv += 2;
                        nRes += (UINT)(nData * 2);
                        if ((dwDataPos - 2) >= 0) {
                            nData = GetProcessedData(dwDataPos - 2, &bRes);
                            if (!bRes) {
                                break;
                            }
                            if (nData >= 0) {
                                nDiv++;
                                nRes += nData;
                            }
                        }
                    }
                }
                // result in normal case: r[dwDataPos]=(3*r[dwDataPos]+2*r[dwDataPos-1]+r[dwDataPos-2])/6
                *pSmoothData-- = (short int)((nRes + (nDiv / 2)) / nDiv); // save result
            } else {
                *pSmoothData-- = (short int)nData;
            }
            if (--dwSmoothPos == 0) { // smoothed buffer is full
                bRes = WriteDataBlock(dwDataPos, lpSmoothData, dwBlockSize); // write the buffer into the temporary file
                if (!bRes) {
                    break;
                }
                dwSmoothPos = GetProcessBufferSize() / 2; // reset buffer position
                dwBlockSize = dwSmoothPos; // save block size
                pSmoothData = (short int *)(lpSmoothData + (GetProcessBufferSize() - 2)); // reset buffer pointer
            }
        }
        m_dwBufferOffset = UNDEFINED_OFFSET;  // buffer undefined, force buffer reload
        if (!bRes) {
            break;
        }
        // and then smooth forwards
        m_nMaxValue = 0;
        m_nMinValue = 0x7FFF;
        dwSmoothPos = 0; // reset buffer position
        pSmoothData = (short int *)lpSmoothData; // reset buffer pointer
        dwBlockSize = 0;
        for (dwDataPos = 0; (DWORD)dwDataPos < dwDataSize; dwDataPos++) {
            nData = GetProcessedData(dwDataPos, &bRes);
            if (!bRes) {
                break;
            }
            if (nData >= 0) {
                nDiv = 3;
                nRes = (UINT)(nData * 3);
                if (((DWORD)dwDataPos + 1) < dwDataSize) {
                    nData = GetProcessedData(dwDataPos + 1, &bRes);
                    if (!bRes) {
                        break;
                    }
                    if (nData >= 0) {
                        nDiv += 2;
                        nRes += (UINT)(nData * 2);
                        if (((DWORD)dwDataPos + 2) < dwDataSize) {
                            nData = GetProcessedData(dwDataPos + 2, &bRes);
                            if (!bRes) {
                                break;
                            }
                            if (nData >= 0) {
                                nDiv++;
                                nRes += nData;
                            }
                        }
                    }
                }
                // result in normal case, r[dwDataPos]=(3*r[dwDataPos]+2*r[dwDataPos+1]+r[dwDataPos+2])/6
                *pSmoothData = (short int)((nRes + (nDiv / 2)) / nDiv); // save result
            } else {
                *pSmoothData = (short int)nData;
            }
            if (*pSmoothData >= 0) {
                // adjust max data level
                if (*pSmoothData > m_nMaxValue) {
                    m_nMaxValue = *pSmoothData;
                }
                // adjust min data level
                if (*pSmoothData < m_nMinValue) {
                    m_nMinValue = *pSmoothData;
                }
            }
            *pSmoothData++;
            if ((++dwSmoothPos == GetProcessBufferSize() / 2) // smoothed buffer is full
                    || ((DWORD)dwDataPos == dwDataSize - 1)) { // last result in loop
                bRes = WriteDataBlock(dwBlockSize, lpSmoothData, dwSmoothPos); // write the buffer into the temporary file
                if (!bRes) {
                    break;
                }
                dwSmoothPos = 0; // reset buffer position
                dwBlockSize = dwDataPos; // save data position
                pSmoothData = (short int *)lpSmoothData; // reset buffer pointer
            }
            // overall smoothing is symmetrical
        }
        m_dwBufferOffset = UNDEFINED_OFFSET; // buffer undefined, force buffer reload
        if (!bRes) {
            break;
        }
    }
    // free the smoothed data buffer
    ::GlobalUnlock(hSmoothData);
    ::GlobalFree(hSmoothData);
    return bRes;
}

/***************************************************************************/
// CProcess::Process Dummy process
// This function does a dummy process, that means it just sets the process
// status to processed. It does not create a temporary file and it does not
// process any data.
/***************************************************************************/
long CProcess::Process(void * pCaller, ISaDoc *, int nProgress, int nLevel) {
    UNUSED_ALWAYS(pCaller);
    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }
    // set ready flag
    SetDataReady();
    return MAKELONG(nLevel, nProgress);
}

/***************************************************************************/
// CProcess::WriteProperties Dummy properties writing
/***************************************************************************/
void CProcess::WriteProperties(Object_ostream &) {
}

/***************************************************************************/
// CProcess::ReadProperties Dummy properties reading
/***************************************************************************/
BOOL CProcess::ReadProperties(Object_istream &) {
    return FALSE;
}

/***************************************************************************/
// CProcess::SetStatus
/***************************************************************************/
void CProcess::SetStatus(long nStatus) {
    m_nStatus = nStatus;
}

void CProcess::Dump(const char * tag) {
    TRACE("DataProcess %s size=%d max=%d min=%d status=%d offset=%d\n",tag,m_dwBufferSize, m_nMaxValue, m_nMinValue,m_nStatus,m_dwBufferOffset);
}

//###########################################################################
// CAreaDataProcess
// Base class for all area data processing classes. Does all jobs, common to
// all area data processing derived classes. The area process only processes
// an area in the whole raw data and allows multiple areas to be processed.
// For each area, determined by an index number, there will be a temporary
// file, where the processed data is stored in.

/////////////////////////////////////////////////////////////////////////////
// CAreaDataProcess construction/destruction/creation

/***************************************************************************/
// CAreaDataProcess::CAreaDataProcess Constructor
/***************************************************************************/
CProcessAreaData::CProcessAreaData() {
    // create the area arrays
    m_dwAreaPos = 0;
    m_dwAreaLength = 0;
}

/***************************************************************************/
// CAreaDataProcess::~CAreaDataProcess Destructor
/***************************************************************************/
CProcessAreaData::~CProcessAreaData() {
}

/////////////////////////////////////////////////////////////////////////////
// CAreaDataProcess helper functions

/***************************************************************************/
// CAreaDataProcess::SetArea
/***************************************************************************/
BOOL CProcessAreaData::SetArea(CSaView * pView) {
    if (IsStatusFlag(KEEP_AREA)) {
        return FALSE;    // not a new area
    }

    // get new area boundaries
    m_dwAreaPos = pView->GetStartCursorPosition();

    ISaDoc * pDoc = pView->GetDocument();
    FmtParm * pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
    WORD wSmpSize = (WORD)(pFmtParm->wBlockAlign / pFmtParm->wChannels);
    m_dwAreaLength = pView->GetStopCursorPosition() - m_dwAreaPos + wSmpSize;

    SetStatusFlag(KEEP_AREA);
    return TRUE;
}

/***************************************************************************/
// CAreaDataProcess::SetArea
/***************************************************************************/
BOOL CProcessAreaData::SetArea(DWORD dwAreaPos, DWORD dwAreaLength) {
    // get new area boundaries
    m_dwAreaPos = dwAreaPos;
    m_dwAreaLength = dwAreaLength;

    return TRUE;
}

long CProcess::GetStatus() const {
    return m_nStatus;
}

