/////////////////////////////////////////////////////////////////////////////
// sa_proc.cpp:
// Implementation of the CProcess
//                       CAreaDataProcess classes
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "sa_process.h"

//###########################################################################
// CProcess
// Base class for all data processing classes. Does all jobs, common to all
// data processing derived classes.
CProcess::CProcess(Context & context) : context(context), app(context.app), model(context.model), frame(context.frame), target(context.target) {
    assert(&app != nullptr);
    assert(&model != nullptr);
    assert(&frame != nullptr);
    assert(&target != nullptr);
    filename.clear();

    m_nStatus = PROCESS_IDLE;
    m_lpBuffer = NULL;
    m_nMaxValue = 0;
    m_nMinValue = 0;
    m_dwBufferOffset = UNDEFINED_OFFSET; // buffer undefined, force buffer reload
    m_dwBufferSize = GetBufferSize();
}

/***************************************************************************/
// CProcess::~CProcess Destructor
/***************************************************************************/
CProcess::~CProcess() {
    // close the temporary file
    if (file.is_open()) {
        file.close();
    }
    // delete the temporary file
    FileUtils::Remove(GetProcessFileName());
    DeleteProcessFileName();
    // free the buffer memory
    if (m_lpBuffer) {
        delete [] m_lpBuffer;
        m_lpBuffer = NULL;
    }
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
bool CProcess::StartProcess(void * pCaller, ProcessorType processorType, DWORD dwBufferSize) {
    
    // get pointer to status bar
    ProgressStatusBar * pStatusBar = frame.GetProgressStatusBar();
    m_dwBufferSize = GetBufferSize();

    // no previous process owner? you're the first, show to progress bar
    if (!pStatusBar->GetProcessOwner()) {
		// show the progress status bar
        frame.ShowDataStatusBar(FALSE);
    }

	// set the process owner
    pStatusBar->SetProcessOwner(this, pCaller, processorType);
    if (dwBufferSize) {
        // allocate global buffer for the processed data
		// not yet allocated
        if (!m_lpBuffer) { 
            m_lpBuffer = new char[dwBufferSize];
            if (!m_lpBuffer) {
                // memory lock error
                app.ErrorMessage(IDS_ERROR_MEMALLOC,NULL,NULL);
                return false;
            }
        }
    }
    return true;
}

bool CProcess::StartProcess(void * pCaller, ProcessorType processorType, BOOL bBuffer) {
    DWORD dwBufferSize = bBuffer ? GetBufferSize():0;
    return StartProcess(pCaller, processorType, dwBufferSize);
}

/***************************************************************************/
// CProcess::SetProgress
/***************************************************************************/
void CProcess::SetProgress(int nPercent) {
    ProgressStatusBar * pStatusBar = frame.GetProgressStatusBar();
    pStatusBar->SetProgress(nPercent);
}

/***************************************************************************/
// CProcess::EndProcess Ending the process
// Called by the derived classes to end the process. If bProcessBar is TRUE,
// the data status bar will be shown again instead of the process status bar.
/***************************************************************************/
void CProcess::EndProcess(BOOL bProcessBar) {
    if (bProcessBar) {
        frame.ShowDataStatusBar(TRUE); // show the data status bar
        ProgressStatusBar * pStatusBar = frame.GetProgressStatusBar();
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

    if (((!bBlockBegin) && ((dwByteOffset >= m_dwBufferOffset) && (dwByteOffset < (m_dwBufferOffset + GetProcessBufferSize())))) ||
            ((bBlockBegin) && (m_dwBufferOffset == dwByteOffset))) {
        // this data is actually in buffer
        // return pointer to data
        return m_lpBuffer;
    }

    // new data block has to be read
    if (bBlockBegin) {
        m_dwBufferOffset = dwByteOffset; // given offset is the first sample in data block
    } else {
        m_dwBufferOffset = dwByteOffset - (dwByteOffset % GetProcessBufferSize()); // new block offset
    }
    // open the temporary file
    ifstream file;
    file.open(GetProcessFileName(), ifstream::in | ifstream::binary);
    if (!file.is_open() || file.bad() || file.fail()) {
        // error opening file
        app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
        return NULL;
    }
    // find the right position in the data
    if (m_dwBufferOffset != 0L) {
        file.seekg(m_dwBufferOffset, ifstream::beg);
        if (file.bad() ||  file.fail()) {
            // error seeking file
            app.ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
			// close the temporary file
			SetDataInvalid();
			return NULL;
        }
    }
    // read the processed data block
    file.read(m_lpBuffer, GetProcessBufferSize());
    if (file.bad() || file.fail()) {
        // error reading file
        app.ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
		SetDataInvalid();
		return NULL;
    }
    // return the new data pointer
    return m_lpBuffer;
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
int CProcess::GetProcessedData( DWORD dwOffset, BOOL * pbRes) {
    if (dwOffset == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }

    DWORD dwByteOffset = dwOffset * 2;
    if ((dwByteOffset >= m_dwBufferOffset) && (dwByteOffset < (m_dwBufferOffset + GetProcessBufferSize()))) {
        // this data is actually in buffer
        // set operation result
        *pbRes = TRUE;
        return *reinterpret_cast<short int*>(m_lpBuffer + (dwByteOffset - m_dwBufferOffset)); // return data
    }

    // new data block has to be read
    m_dwBufferOffset = dwByteOffset - (dwByteOffset % GetProcessBufferSize()); // new block offset

                                                                               // open the temporary file
    ifstream file;
    file.open(GetProcessFileName(), ifstream::in | ifstream::binary);
    if (!file.is_open() || file.bad() || file.fail()) {
        // error opening file
        app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
        *pbRes = FALSE; // set operation result
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return 0;
    }
    // find the right position in the data
    if (m_dwBufferOffset != 0L) {
        file.seekg(m_dwBufferOffset, ifstream::beg);
        if (file.bad() || file.fail()) {
            // error seeking file
            app.ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
			// set operation result
			*pbRes = FALSE;
            m_dwBufferOffset = UNDEFINED_OFFSET;
			return 0;
        }
    }
    // read the processed data block
    file.read(m_lpBuffer, GetProcessBufferSize());
    if (file.bad() || file.fail()) {
        // error reading file
        app.ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
		// close the temporary file
		SetDataInvalid();
		// set operation result
		*pbRes = FALSE;
        m_dwBufferOffset = UNDEFINED_OFFSET;
		return 0;
    }
    // return the data
    short int * lpData = (short int *)m_lpBuffer; // cast buffer pointer
    *pbRes = TRUE;  // set operation result
    return *(lpData + (dwOffset - (m_dwBufferOffset / 2))); // return data
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
BPTR CProcess::GetProcessedWaveData(DWORD dwOffset, BOOL bBlockBegin) {

    if (GetProcessFileName() == NULL || GetProcessFileName()[0] == 0) {
        return NULL;
    }
    if (dwOffset == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }
    if (((!bBlockBegin) && ((dwOffset >= m_dwBufferOffset) && (dwOffset < m_dwBufferOffset + GetProcessBufferSize()))) ||
            ((bBlockBegin) && (m_dwBufferOffset == dwOffset))) {
        // this data is actually in buffer
        // return pointer to data
        return m_lpBuffer;
    }

    // new data block has to be read
    if (bBlockBegin) {
        m_dwBufferOffset = dwOffset;    // given offset ist first sample in data block
    } else {
        m_dwBufferOffset = dwOffset - (dwOffset % GetProcessBufferSize());    // new block offset
    }
    // open the temporary file
    ifstream file;
    file.open(GetProcessFileName(), ifstream::in | ifstream::binary);
    if (!file.is_open() || file.bad() || file.fail()) {
        // error opening file
        app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
        return NULL;
    }
    // find the right position in the data
    if (m_dwBufferOffset != 0L) {
        file.seekg(m_dwBufferOffset, ifstream::beg);
        if (file.bad() || file.fail()) {
            // error seeking file
            app.ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
			SetDataInvalid();
			return NULL;
        }
    }
    // read the processed data block
    file.read(m_lpBuffer, GetProcessBufferSize());
    if (file.bad() || file.fail()) {
        // error reading file
        app.ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
		// close the temporary file
		SetDataInvalid();
		return NULL;
    }
    // return the new data pointer
    return m_lpBuffer;
}

/***************************************************************************/
// CProcess::GetProcessedObject returns a pointer to requested data object
// uses data buffer to optimize requests
// Returns null on failure
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
// CProcess::GetProcessedObject returns a pointer to requested data object
// uses data buffer to optimize requests
/***************************************************************************/
void * CProcess::GetProcessedObject(LPCTSTR /*szName*/, int /*selectedChannel*/, int /*numChannels*/, int /*sampleSize*/, DWORD dwIndex, size_t sObjectSize, BOOL bReverse) {
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
// Returns NULL on failure
/***************************************************************************/
void * CProcess::GetProcessedDataBlock(DWORD dwByteOffset, size_t sObjectSize, BOOL bReverse) {

    if (dwByteOffset == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }

    if ((dwByteOffset >= m_dwBufferOffset) && ((dwByteOffset + sObjectSize) < (m_dwBufferOffset + GetProcessBufferSize()))) {
        // this data is actually in buffer
        // return pointer to data
        return m_lpBuffer + (dwByteOffset - m_dwBufferOffset);
    }

    // new data block has to be read
    // given offset is the first sample in data block
    m_dwBufferOffset = dwByteOffset;                        
    if (bReverse) {
        // since we are traversing the file in reverse, load buffer so that object is biased to end of buffer
        m_dwBufferOffset = dwByteOffset + sObjectSize;
        if (m_dwBufferOffset > GetProcessBufferSize()) {
            m_dwBufferOffset -= GetProcessBufferSize();
        } else {
            m_dwBufferOffset = 0;
        }
    }

    ifstream file;
    file.open(GetProcessFileName(), ifstream::in | ifstream::binary);
    if (!file.is_open() || file.bad() || file.fail()) {
        // error opening file
        app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
        return NULL;
    }
    // find the right position in the data
    if (m_dwBufferOffset != 0L) {
        file.seekg(m_dwBufferOffset, ifstream::beg);
        if (file.bad() || file.fail()) {
            // error seeking file
            app.ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
            // close the temporary file
            SetDataInvalid();
            return NULL;
        }
    }
    // read the processed data block
    file.read(m_lpBuffer, GetProcessBufferSize());
    if (file.bad() || file.fail()) {
        // error reading file
        app.ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
        // close the temporary file
        SetDataInvalid();
        return NULL;
    }
    // return the new data pointer
    return m_lpBuffer + (dwByteOffset - m_dwBufferOffset); 
}

bool CProcess::CreateTempFile(const wchar_t* szName) {
    
    // close the temporary file
    if (file.is_open()) {
        file.close();
    }
    FileUtils::Remove(GetProcessFileName());
    DeleteProcessFileName();
    TCHAR szTempPath[_MAX_PATH];
    FileUtils::GetTempFileName( szName, szTempPath, _countof(szTempPath));
    // create and open the file
    if (!Open(szTempPath, fstream::in | fstream::out | fstream::binary | fstream::trunc)) {
        // error opening file
        app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, szTempPath);
        // close the temporary file
        SetDataInvalid();
        return false;
    }
    // buffer undefined, force buffer reload
    m_dwBufferOffset = UNDEFINED_OFFSET;
    return true;
}

bool CProcess::Open(LPCTSTR lpszFilename, fstream::openmode flags) {
    assert(lpszFilename !=NULL);
    assert(wcslen(lpszFilename)>0);
    if (file.is_open()) {
        file.close();
    }

    file.open(lpszFilename, 0);
    return (file.is_open() && !file.bad() && !file.fail());
}

/***************************************************************************/
// CProcess::OpenFileToAppend  Open temp file and set up for appending
/***************************************************************************/
bool CProcess::OpenFileToAppend() {
    if (!Open(GetProcessFileName(), fstream::in | fstream::out | fstream::binary | fstream::app | fstream::ate)) {
        // error opening file
        app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
        return false;
    }

    file.seekg(0, fstream::end);
    if (file.bad() || file.fail()) {
        // error writing file
        app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
		// close the temporary file
		SetDataInvalid();
		return false;
    }

    return true;
}

/***************************************************************************/
// CProcess::CloseTempFile Close the temporary file
/***************************************************************************/
void CProcess::CloseTempFile() {
    // close the file
    if (file.is_open()) {
        file.close();
    }
}

/***************************************************************************/
// CProcess::Exit Exit on Error
// Standard exit function if an error occured.
/***************************************************************************/
long CProcess::Exit(int nError) {
    SetDataInvalid();
    EndProcess(); // end data processing
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
bool CProcess::WriteDataBlock( DWORD dwPosition, BPTR lpData, DWORD dwDataLength, size_t nElementSize) {
    // open the temporary file
    bool preExisting = file.is_open();
    if (!preExisting) {
        if (!Open(GetProcessFileName(), fstream::in | fstream::out | fstream::binary)) {
            // error opening file
            app.ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
            return false;
        }
    }
    // find the right position in the data
    file.seekg(dwPosition * nElementSize, ofstream::beg);
    if (file.bad() || file.fail()) {
        // error seeking file
        app.ErrorMessage(IDS_ERROR_READTEMPFILE, GetProcessFileName());
		SetDataInvalid();
		return false;
    }
    // write the data block from the buffer
    file.write((BPTR)lpData, dwDataLength * nElementSize);
    if (file.bad() || file.fail()) {
        // error writing file
        app.ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
		// close the temporary file
		SetDataInvalid();
		return false;
    }
    if (!preExisting) {
        CloseTempFile();    // close the temporary file
    }
    return true;
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
        app.ErrorMessage(IDS_ERROR_MEMALLOC);
        return FALSE;
    }
    BPTR lpSmoothData = (BPTR)::GlobalLock(hSmoothData); // lock memory
    if (!lpSmoothData) {
        // memory lock error
        app.ErrorMessage(IDS_ERROR_MEMLOCK);
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
                    nData = GetProcessedData( dwDataPos - 1, &bRes);
                    if (!bRes) {
                        break;
                    }
                    if (nData >= 0) {
                        nDiv += 2;
                        nRes += (UINT)(nData * 2);
                        if ((dwDataPos - 2) >= 0) {
                            nData = GetProcessedData( dwDataPos - 2, &bRes);
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
            nData = GetProcessedData( dwDataPos, &bRes);
            if (!bRes) {
                break;
            }
            if (nData >= 0) {
                nDiv = 3;
                nRes = (UINT)(nData * 3);
                if (((DWORD)dwDataPos + 1) < dwDataSize) {
                    nData = GetProcessedData( dwDataPos + 1, &bRes);
                    if (!bRes) {
                        break;
                    }
                    if (nData >= 0) {
                        nDiv += 2;
                        nRes += (UINT)(nData * 2);
                        if (((DWORD)dwDataPos + 2) < dwDataSize) {
                            nData = GetProcessedData( dwDataPos + 2, &bRes);
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
            if ((++dwSmoothPos == GetProcessBufferSize() / 2) ||    // smoothed buffer is full
                    ((DWORD)dwDataPos == dwDataSize - 1)) {             // last result in loop
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
long CProcess::Process(void * /*pCaller*/, int nProgress, int nLevel) {
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
void CProcess::WriteProperties(CObjectOStream &) {
}

/***************************************************************************/
// CProcess::ReadProperties Dummy properties reading
/***************************************************************************/
BOOL CProcess::ReadProperties(CObjectIStream &) {
    return FALSE;
}

/***************************************************************************/
// CProcess::SetStatus
/***************************************************************************/
void CProcess::SetStatus(long nStatus) {
    m_nStatus = nStatus;
}

long CProcess::GetStatus() const {
    return m_nStatus;
}

// cancel current process
void CProcess::CancelProcess() {
    SetStatusFlag(PROCESS_CANCEL, TRUE);
}

// restart current process
void CProcess::RestartProcess() {
    SetStatusFlag(PROCESS_CANCEL, FALSE);
}

// return the process canceled flag
BOOL CProcess::IsCanceled() const {
    return IsStatusFlag(PROCESS_CANCEL);
}

BOOL CProcess::IsDataReady() const {
    return IsStatusFlag(DATA_READY);
}

void CProcess::SetDataInvalid() {
    SetStatusFlag(DATA_READY, FALSE);
    if (file.is_open()) {
        file.close();
    }
    m_dwBufferOffset = UNDEFINED_OFFSET;
}

// effectively return the number of samples
// return processed data size in words (16 bit)
DWORD CProcess::GetDataSize() const {
    return GetDataSize(2);
}

// return processed data size in words (16 bit)
DWORD CProcess::GetNumSamples() const {
    return GetDataSize(2);
}

// return processed element count
DWORD CProcess::GetDataSize(size_t nElementSize) const {
    ifstream in(filename, ifstream::ate | ifstream::binary);
    std::streampos length = in.tellg();
    return (DWORD)(length / nElementSize);
}

// return maximum value
int CProcess::GetMaxValue() const {
    return m_nMaxValue;
}

// return minimum value
int CProcess::GetMinValue() const {
    return m_nMinValue;
}

long CProcess::IsStatusFlag(long nStatus) const {
    return (m_nStatus & nStatus) == nStatus;
}

// return TRUE if process is idle
BOOL CProcess::IsIdle() const {
    return (IsStatusFlag(PROCESS_IDLE) != 0);
}

// return TRUE if processed data is aliased
BOOL CProcess::IsAliased() const {
    return (IsStatusFlag(DATA_ALIASED) != 0);
};

// provides implementation for Process interface.
// return processed wave source data size
DWORD CProcess::GetProcessedModelWaveDataSize() {
    return GetDataSize(1);
}

// return processed wave source data size
DWORD CProcess::GetProcessedWaveDataSize() {
    return GetDataSize(1);
}

// return index to process buffer
DWORD CProcess::GetProcessBufferIndex(size_t nSize) {
    return m_dwBufferOffset >= 0x40000000 ? UNDEFINED_OFFSET : m_dwBufferOffset/nSize;
}

// return process buffer size
DWORD CProcess::GetProcessBufferSize() {
    return m_dwBufferSize;
}

void CProcess::SetProcessBufferSize(DWORD dwSize) {
    m_dwBufferSize = dwSize;
}

// return process temporary file path and name
LPCTSTR CProcess::GetProcessFileName() {
    return filename.c_str();
}

// delete the temporary file name
void CProcess::DeleteProcessFileName() {
    filename.clear();
}

void CProcess::SetStatusFlag( long nStatus, BOOL bValue) {
    SetStatus(bValue ? m_nStatus | nStatus : m_nStatus & ~nStatus);
}

void CProcess::SetDataReady(BOOL bReady) {
    SetStatusFlag(DATA_READY, bReady);
}

bool CProcess::Write(const void * lpBuf, UINT nCount) {
    file.write( (const char *)lpBuf, nCount);
    return (!file.bad() && !file.fail());
}

DWORD CProcess::GetProcessedWaveDataBufferSize() {
    return GetBufferSize();
}

DWORD CProcess::GetBufferSize() {
    return 0x10000;
}

DWORD CProcess::GetProcessedData(int /*index*/) {
    return 0;
}

void CProcess::Dump(LPCSTR tag) {
}
