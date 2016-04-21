/////////////////////////////////////////////////////////////////////////////
// sa_p_doc.cpp:
// Implementation of the CProcessDoc
//
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ProcessDoc.h"
#include "mainfrm.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "FileUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CProcessDoc::CProcessDoc() {

    m_dwBufferOffset = UNDEFINED_OFFSET;    // buffer undefined, force buffer reload
}

CProcessDoc::~CProcessDoc() {
}

long CProcessDoc::Process(void * /*pCaller*/, ISaDoc * /*pDoc*/, int /*nProgress*/, int nLevel) {
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
HPSTR CProcessDoc::GetProcessedWaveData(LPCTSTR szName, int selectedChannel, int numChannels, int sampleSize, DWORD dwOffset, BOOL bBlockBegin) {

    if (wcslen(szName)==0) {
        return NULL;
    }

    if (dwOffset == UNDEFINED_OFFSET) {
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }

    //TRACE(L"GetProcessedWaveData %s %d %d %d %d %d %d %d\n", szName, selectedChannel, numChannels, sampleSize, dwOffset, bBlockBegin,m_dwBufferOffset,_countof(m_Buffer));
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

    //TRACE("new BufferOffset %d\n",m_dwBufferOffset);

    CSaApp * pApp = (CSaApp *) AfxGetApp();

    CFile file;
    // open the temporary file
    if (!file.Open(szName, CFile::modeRead | CFile::shareExclusive, NULL)) {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, szName);
        return NULL;
    }

    // find the right position in the data
    if (m_dwBufferOffset != 0L) {
        try {
            DWORD index = m_dwBufferOffset*numChannels;
            //TRACE("seek %d\n",index);
            file.Seek(index, CFile::begin);
        } catch (CFileException * e) {
            // error seeking file
            pApp->ErrorMessage(IDS_ERROR_READTEMPFILE, szName);
            m_dwBufferOffset = UNDEFINED_OFFSET;
            e->Delete();
			return NULL;
        }
    }

    // read the processed data block
    size_t size = _countof(m_Buffer)*numChannels;
    char * buffer = new char[size];
    memset(buffer,0,size);
    ASSERT(sampleSize<3);

    try {
        UINT bytesRead = file.Read(buffer, size);
        LoadBuffer(buffer, size, sampleSize, selectedChannel, numChannels, bytesRead);
    } catch (CFileException * e) {
        delete [] buffer;
        // error reading file
        pApp->ErrorMessage(IDS_ERROR_READTEMPFILE, szName);
        m_dwBufferOffset = UNDEFINED_OFFSET;
        e->Delete();
		return NULL;
    }

    delete [] buffer;

    file.Close();

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

    //TRACE("read %d %d %d %d %d\n",dwByteOffset,m_dwBufferOffset,sObjectSize,_countof(m_Buffer),bReverse);
    //TRACE(L"GetProcessedDataBlock %s\n",szName);

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

    CSaApp * pApp = (CSaApp *) AfxGetApp();

    CFile file;
    if (!file.Open(szName, CFile::modeRead | CFile::shareExclusive, NULL)) {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, szName);
        return NULL;
    }

    // find the right position in the data
    if (m_dwBufferOffset != 0L) {
        try {
            size_t seek = m_dwBufferOffset*numChannels;
            file.Seek(seek, CFile::begin);
        } catch (...) {
            // error seeking file
            pApp->ErrorMessage(IDS_ERROR_READTEMPFILE, szName);
            m_dwBufferOffset = UNDEFINED_OFFSET;
            return NULL;
        }
    }

    // read the processed data block
    size_t size = _countof(m_Buffer)*numChannels;
    char * buffer = new char[size];
    memset(buffer,0,size);
    ASSERT(sampleSize<3);

    // read the processed data block
    try {
        UINT bytesRead = file.Read(buffer, size);
        LoadBuffer(buffer, size, sampleSize, selectedChannel, numChannels, bytesRead);
    } catch (...) {
        delete [] buffer;
        // error reading file
        pApp->ErrorMessage(IDS_ERROR_READTEMPFILE, szName);
        m_dwBufferOffset = UNDEFINED_OFFSET;
        return NULL;
    }
    delete [] buffer;

    file.Close();

    // return the new data pointer
    return &m_Buffer[dwByteOffset - m_dwBufferOffset]; // return pointer to data
}

void CProcessDoc::LoadBuffer(char * buffer, size_t /*size*/, int sampleSize, int selectedChannel, int numChannels, UINT bytesRead) {
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

DWORD CProcessDoc::GetNumSamples(ISaDoc * pDoc) const {
    return pDoc->GetNumSamples();
}

DWORD CProcessDoc::GetProcessedWaveDataSize(ISaDoc * pDoc) {
    return pDoc->GetDataSize();
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
