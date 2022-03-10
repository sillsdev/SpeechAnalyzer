/////////////////////////////////////////////////////////////////////////////
// ProcessDoc.h:
// Interface of the CProcessDoc
//
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_DOC_H
#define _SA_P_DOC_H

using std::wstring;

__interface ISaDoc;
__interface CmdTarget;

class CProcessDoc : public IProcess {

public:
    CProcessDoc(App & app, Model & model);
    CProcessDoc() = delete;
    virtual ~CProcessDoc();

    void SetDataInvalid();
    // return pointer to block of processed wave source
    BPTR GetProcessedWaveData( LPCTSTR szName, int selectedChannel, int numChannels, int sampleSize, DWORD dwOffset, BOOL bBlockBegin = FALSE);
    // return processed data pointer to object staring at dwOffset
    void * GetProcessedDataBlock(LPCTSTR szName, int selectedChannel, int numChannels, int sampleSize, DWORD dwOffset, size_t sObjectSize, BOOL bReverse=FALSE);
    DWORD GetProcessBufferIndex(size_t nSize = 1);
    DWORD GetBufferSize();

    // interface methods
    virtual long Process(void * pCaller, int nProgress = 0, int nLevel = 1);
    // return processed data pointer to object staring at dwOffset
    virtual void * GetProcessedObject(LPCTSTR szName, int selectedChannel, int numChannels, int sampleSize, DWORD dwIndex, size_t sObjectSize, BOOL bReverse=FALSE);
    // return the total number of samples in the data for a single channel
    virtual DWORD GetNumSamples() const;                                
    // return the byte count of the data for a single channel
    virtual DWORD GetProcessedModelWaveDataSize();

private:
    void LoadBuffer(char * buffer, size_t size, int sampleSize, int selectedChannel, int numChannels, std::streamsize bytesRead);

    App & app;
    Model& model;
    char m_Buffer[0x10000];     // processed data buffer
    DWORD m_dwBufferOffset;     // actual buffer offset in bytes
};

#endif
