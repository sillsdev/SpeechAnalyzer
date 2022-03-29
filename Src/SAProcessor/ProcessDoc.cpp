/////////////////////////////////////////////////////////////////////////////
// sa_p_doc.cpp:
// Implementation of the CProcessDoc
//
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ProcessDoc.h"
#include "FileUtils.h"

CProcessDoc::CProcessDoc(App * app, Model * model) : app(app), model(model) {
    assert(app != nullptr);
    assert(model != nullptr);
    // buffer undefined, force buffer reload
    m_dwBufferOffset = UNDEFINED_OFFSET;    
}

long CProcessDoc::Process(void * /*pCaller*/, int /*nProgress*/, int nLevel) {
    return MAKELONG(nLevel, 100);
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
BPTR CProcessDoc::GetProcessedWaveData( LPCTSTR szName, int selectedChannel, int numChannels, int sampleSize, DWORD dwOffset, BOOL bBlockBegin) {

    if (wcslen(szName)==0) {
        return NULL;
    }

    if (dwOffset == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }

    if (((!bBlockBegin) && ((dwOffset >= m_dwBufferOffset) && (dwOffset < m_dwBufferOffset + _countof(m_Buffer)))) ||
            ((bBlockBegin) && (m_dwBufferOffset == dwOffset))) {
        // this data is actually in buffer
        // return pointer to data
        return m_Buffer;
    }

    // new data block has to be read
    if (bBlockBegin) {
        m_dwBufferOffset = dwOffset;                                        // given offset 1st first sample in data block
    } else {
        m_dwBufferOffset = dwOffset - (dwOffset % _countof(m_Buffer));      // new block offset
    }

    // open the temporary file
    ifstream file;
    file.open(szName, ifstream::in | ifstream::binary);
    if (!file.is_open() || file.bad() || file.fail()) {
        // error opening file
        app->ErrorMessage(IDS_ERROR_OPENTEMPFILE, szName);
        return NULL;
    }

    // find the right position in the data
    if (m_dwBufferOffset != 0L) {
        DWORD index = m_dwBufferOffset*numChannels;
        file.seekg(index, ifstream::beg);
        if (file.bad() || file.fail()) {
            // error seeking file
            app->ErrorMessage(IDS_ERROR_READTEMPFILE, szName);
            m_dwBufferOffset = UNDEFINED_OFFSET;
			return NULL;
        }
    }

    // read the processed data block
    size_t size = _countof(m_Buffer)*numChannels;
    char * buffer = new char[size];
    memset(buffer,0,size);
    assert(sampleSize<3);

    file.read(buffer, size);
    if (file.bad()) {
        delete [] buffer;
        // error reading file
        app->ErrorMessage(IDS_ERROR_READTEMPFILE, szName);
        m_dwBufferOffset = UNDEFINED_OFFSET;
		return NULL;
    }
    std::streamsize bytesRead = file.gcount();
    LoadBuffer(buffer, size, sampleSize, selectedChannel, numChannels, bytesRead);

    delete [] buffer;

    // return the new data pointer
    return m_Buffer;
}

/***************************************************************************/
// CProcess::GetProcessedObject returns a pointer to requested data object
// uses data buffer to optimize requests
/***************************************************************************/
void * CProcessDoc::GetProcessedObject(LPCTSTR szName, int selectedChannel, int numChannels, int sampleSize, DWORD dwIndex, size_t sObjectSize, BOOL bReverse) {

    if (dwIndex == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }
    DWORD dwByteOffset = dwIndex * sObjectSize;
    return GetProcessedDataBlock(szName, selectedChannel, numChannels, sampleSize, dwByteOffset, sObjectSize, bReverse);
}

/***************************************************************************/
// CProcess::GetProcessedDataBlock returns a pointer to requested data object
// uses data buffer to optimize requests
/***************************************************************************/
void * CProcessDoc::GetProcessedDataBlock(LPCTSTR szName, int selectedChannel, int numChannels, int sampleSize, DWORD dwByteOffset, size_t sObjectSize, BOOL bReverse) {

    if (dwByteOffset == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }

    if ((dwByteOffset >= m_dwBufferOffset) &&
        ((dwByteOffset+sObjectSize) <= (m_dwBufferOffset + _countof(m_Buffer)))) {
        // this data is actually in buffer
        // return pointer to data
        return &m_Buffer[dwByteOffset - m_dwBufferOffset];
    }

    // new data block has to be read
	// given offset is the first sample in data block
    m_dwBufferOffset = dwByteOffset; 
    if (bReverse) {
        // since we are traversing the file in reverse
        // load buffer so that object is biased to end of buffer
        m_dwBufferOffset = dwByteOffset + sObjectSize;
        if (m_dwBufferOffset > _countof(m_Buffer)) {
            m_dwBufferOffset -= _countof(m_Buffer);
        } else {
            m_dwBufferOffset = 0;
        }
    }

    ifstream file;
    file.open(szName, ifstream::in | ifstream::binary);
    if (!file.is_open() || file.bad() || file.fail()) {
        // error opening file
        app->ErrorMessage(IDS_ERROR_OPENTEMPFILE, szName);
        return NULL;
    }

    // find the right position in the data
    if (m_dwBufferOffset != 0L) {
        size_t seek = m_dwBufferOffset*numChannels;
        file.seekg(seek, ifstream::beg);
        if (file.bad() || file.fail()) {
            // error seeking file
            app->ErrorMessage(IDS_ERROR_READTEMPFILE, szName);
            m_dwBufferOffset = UNDEFINED_OFFSET;
            return NULL;
        }
    }

    // read the processed data block
    size_t size = _countof(m_Buffer)*numChannels;
    char * buffer = new char[size];
    memset(buffer,0,size);
    assert(sampleSize<3);

    // read the processed data block
    file.read(buffer, size);
    std::streamsize bytesRead = file.gcount();
    LoadBuffer(buffer, size, sampleSize, selectedChannel, numChannels, bytesRead);
    if (file.bad()) {
        delete [] buffer;
        // error reading file
        app->ErrorMessage(IDS_ERROR_READTEMPFILE, szName);
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }
    delete [] buffer;

    // return the new data pointer
    return &m_Buffer[dwByteOffset - m_dwBufferOffset]; // return pointer to data
}

void CProcessDoc::LoadBuffer(char * buffer, size_t /*size*/, int sampleSize, int selectedChannel, int numChannels, std::streamsize bytesRead) {
    if (sampleSize==1) {
        char * src = buffer;
        char * dest = m_Buffer;
        // 8 bit processing
        int index = selectedChannel;
        int write = 0;
        UINT i = 0;
        while (i<bytesRead) {
            dest[write++] = src[index];
            index += numChannels;
            i += numChannels;
        }
    } else {
        // 16 bit processing
        WORD * src = (WORD *)buffer;
        WORD * dest = (WORD *)m_Buffer;
        // 8 bit processing
        int index = selectedChannel;
        int write = 0;
        UINT i = 0;
        while (i<bytesRead) {
            dest[write++] = src[index];
            index += numChannels;
            i += (numChannels*sampleSize);
        }
    }
}

DWORD CProcessDoc::GetNumSamples() const {
    return model->GetNumSamples();
}

DWORD CProcessDoc::GetProcessedModelWaveDataSize() {
    return model->GetDataSize();
}

DWORD CProcessDoc::GetProcessBufferIndex(size_t nSize) {
    // return index to process buffer
    return (m_dwBufferOffset >= 0x40000000) ? UNDEFINED_OFFSET : m_dwBufferOffset/nSize;
}

DWORD CProcessDoc::GetBufferSize() {
    return _countof(m_Buffer);
}

void CProcessDoc::SetDataInvalid() {
    m_dwBufferOffset = UNDEFINED_OFFSET;
}
