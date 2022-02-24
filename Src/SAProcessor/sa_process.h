/////////////////////////////////////////////////////////////////////////////
// sa_proc.h:
// Interface of the CProcess
//                  CAreaDataProcess classes
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef SA_PROCESS_H
#define SA_PROCESS_H

#include "context.h"
#include "ObjectIStream.h"
#include "ObjectOStream.h"

// CProcess completion codes
#define PROCESS_DONE            1  // data processed successfully
#define PROCESS_UNNECESSARY     0  // data returned from buffer, no processing necessary
#define PROCESS_ERROR          -1  // error in process
#define PROCESS_CANCELED       -2  // process canceled
#define PROCESS_NO_DATA        -3  // no data to process
#define PROCESS_DATA_OVERLOAD  -4  // too much data to process
#define PROCESS_UNVOICED       -5  // process region is unvoiced
#define PROCESS_PROCESSING     -6  // process in process message loop, wait

// CProcess status codes
#define DATA_READY          0x0001  // data ready
#define PROCESS_BUSY        0x0002  // processing
#define PROCESS_CANCEL      0x0004  // user canceled process
#define PROCESS_IDLE        0x0008  // awaiting start
#define DATA_ALIASED        0x0010  // data aliased
#define PROCESS_NO_PITCH    0x0020  // No error, no pitch in analysis
#define MAX_RESOLUTION      0x0100  // spectrogram at max resolution
#define KEEP_AREA           0x4000  // keep the area boundaries

__interface ISaDoc;
__interface View;
__interface IProcess;

class CProcess : public IProcess {

public:
    CProcess(Context context);
    CProcess() = delete;
    virtual ~CProcess();

    virtual void CancelProcess();
    virtual void RestartProcess();
    virtual BOOL IsCanceled() const;
    virtual BOOL IsDataReady() const;
    virtual void SetDataInvalid();
    virtual void * GetProcessedData(DWORD dwOffset, BOOL bBlockBegin = FALSE);      // return processed data pointer on given position (offset)
    virtual int GetProcessedData(DWORD dwOffset, BOOL *);                           // return processed data from given position (offset)
    virtual DWORD GetDataSize() const;
    virtual DWORD GetDataSize(size_t nElementSize) const;
    virtual void * GetProcessedDataBlock(DWORD dwOffset, size_t sObjectSize, BOOL bReverse=FALSE);              // return processed data pointer to object staring at dwOffset
    virtual void * GetProcessedObject(DWORD dwIndex, size_t sObjectSize, BOOL bReverse=FALSE);                   // return processed data pointer to object staring at dwOffset
    virtual void * GetProcessedObject(LPCTSTR szName, int selectedChannel, int numChannels, int sampleSize, DWORD dwIndex, size_t sObjectSize, BOOL bReverse=FALSE);  // return processed data pointer to object staring at dwOffset
    virtual int GetMaxValue() const;
    virtual int GetMinValue() const;
    virtual BOOL IsIdle() const;
    virtual BOOL IsAliased() const;                                                 // return TRUE if processed data is aliased
    virtual long Process(void * pCaller, int nProgress = 0, int nLevel = 1);
    virtual DWORD GetProcessedData(int index);
    virtual DWORD GetProcessBufferIndex(size_t nSize = 1);
    virtual LPCTSTR GetProcessFileName();
    // Workbench Operations
    virtual void WriteProperties(CObjectOStream & obs);
    virtual BOOL ReadProperties(CObjectIStream & obs);
    // special workbench helper functions
    // return pointer to block of processed wave source
    virtual BPTR GetProcessedWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE);    
    // return the sample size in bytes for a single channel
    virtual DWORD GetProcessedWaveDataSize();
    // return the sample size in bytes for a single channel
    virtual DWORD GetProcessedModelWaveDataSize();
    // return number of samples for single channel    
    virtual DWORD GetNumSamples() const;

    long IsStatusFlag(long nStatus) const;
    DWORD GetProcessBufferSize();
    void SetProcessBufferSize(DWORD dwSize);
    void DeleteProcessFileName();
    BOOL SmoothData(int nTimes); // smooth data in the temporary file nTimes times
    virtual void Dump(LPCSTR tag);
    DWORD GetProcessedWaveDataBufferSize();
    DWORD GetBufferSize();

protected:
    virtual long GetStatus() const;
    virtual bool StartProcess(void * pCaller, ProcessorType nProcessID, DWORD dwBufferSize);          // start processing data
    virtual bool StartProcess(void * pCaller, ProcessorType nProcessID = PROCESSDFLT, BOOL bBuffer = TRUE);
    virtual void SetProgress(int nPercentProgress);
    virtual long Exit(int nError);                                                          // exit processing on error
    virtual BOOL WriteDataBlock(DWORD dwPosition, BPTR lpData, DWORD dwDataLength, size_t nElementSize = 2); // write a block into the temporary file
    virtual void SetDataReady(BOOL bReady = TRUE);
    virtual void Write(const void * lpBuf, UINT nCount);
    // Special case used to bypass file
    virtual void SetDataSize(int nElements, size_t nElementSize = 1);

    void SetStatus(long nStatus);
    void SetStatusFlag(long nStatus, BOOL bValue);
    void EndProcess(BOOL bProcessBar = TRUE);                                               // end processing data
    BOOL CreateTempFile(const wchar_t*);                                                           // create a temporary file
    BOOL CreateTempFile(const wchar_t*, CFileStatus *);                                            // create a temporary file
    BOOL CreateAuxTempFile(const wchar_t* szName, CFile * pFile, CFileStatus & fileStatus); // create auxilliary temp file
    BOOL OpenFileToAppend();                                                                // open temporary file to append data
    void CloseTempFile(BOOL bUpdateStatus = TRUE);                                          // close the temporary file

    BPTR m_lpBuffer;     // pointer to processed data
    DWORD m_dwBufferSize;       // data buffer size
    DWORD m_dwBufferOffset;     // actual buffer offset
    int m_nMaxValue;            // maximum value of processed data
    int m_nMinValue;            // minimum value of processed data
    Context context;
    App & app;
    View & view;
    Model & model;
    MainFrame& main;

private:
    ProgressStatusBar * GetStatusBar();
    virtual void DeleteTempFile();      // delete the temporary file (private use SetDataInvalid)
    BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException * pError = NULL);

    long m_nStatus;
    CFileStatus m_fileStatus;   // file status
    CFile * m_pFile;            // file object
};

template<class _Ty>
class CProcessIterator {
public:
    typedef CProcessIterator<_Ty> CIter;

    CProcessIterator(CProcess & process, unsigned int index = 0, BOOL bReverse=FALSE)
        :  m_process(process), m_pData(0), m_bReverse(bReverse) {
        m_nCurrentIndex = index;
        m_nBeginIndex = m_nEndIndex = 0;
        m_nLength = process.GetProcessBufferSize()/sizeof(_Ty);
    }

    _Ty & operator*() {
        if ((m_nCurrentIndex < m_nBeginIndex) || (m_nCurrentIndex >= m_nEndIndex)) {
            loadData();
        }
        if (m_pData == NULL) {
            throw 0;
        }
        return
            m_pData[m_nCurrentIndex-m_nBeginIndex];
    }

    // prefix operators
    CIter & operator++() {
        m_nCurrentIndex++;
        return *this;
    }
    CIter & operator--() {
        m_nCurrentIndex--;
        return *this;
    }

private:
    CProcess & m_process;
    BOOL m_bReverse;
    _Ty * m_pData;
    unsigned int m_nBeginIndex;
    unsigned int m_nCurrentIndex;
    unsigned int m_nEndIndex;
    unsigned int m_nLength;

    void loadData() {
        if (m_bReverse) {
            m_nBeginIndex = m_nCurrentIndex > m_nLength ? m_nCurrentIndex + 1 - m_nLength : 0;
        } else {
            m_nBeginIndex = m_nCurrentIndex;
        }
        m_nEndIndex = m_nBeginIndex + m_nLength;
        m_pData = (_Ty *) m_process.GetProcessedDataBlock(m_nBeginIndex*sizeof(_Ty), m_nLength*sizeof(_Ty));
    }
};

#endif
