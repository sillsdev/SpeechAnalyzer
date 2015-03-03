#include "Stdafx.h"
#include "FileEncodingHelper.h"
#include <iostream>
#include "SaString.h"

using std::vector;
using std::ifstream;
using std::streampos;
using std::wstringstream;

CFileEncodingHelper::CFileEncodingHelper(LPCTSTR filenamearg) :
    filename(filenamearg) {
}


CFileEncodingHelper::~CFileEncodingHelper(void) {
}

bool CFileEncodingHelper::CheckEncoding(bool display) {
    string unknown;
    if (!ReadFileIntoBuffer(unknown)) {
        if (display) {
            CString msg;
            msg.FormatMessage(IDS_ERROR_FILENAME,filename);
            AfxMessageBox(msg, MB_OK|MB_ICONEXCLAMATION, 0);
        }
        return false;
    }

    if (IsUTF32(unknown)) {
        if (display) {
            AfxMessageBox(IDS_NO_CONVERT, MB_OK|MB_ICONEXCLAMATION);
        }
        return false;
    } else if (IsUTF8(unknown)) {
        return true;
    } else if (IsUTF16BE(unknown)) {
        if (display) {
            AfxMessageBox(IDS_WARN_CONVERT, MB_OK|MB_ICONEXCLAMATION);
        }
        return true;
    } else if (IsUTF16LE(unknown)) {
        if (display) {
            AfxMessageBox(IDS_WARN_CONVERT, MB_OK|MB_ICONEXCLAMATION);
        }
        return true;
    } else if (IsASCII(unknown)) {
        return true;
    }
    return true;
}

bool CFileEncodingHelper::ConvertFileToUTF8(istringstream & result) {

    wstring buffer;
    if (!ConvertFileToUTF16(buffer)) {
        return false;
    }
    return ConvertUTF16ToUTF8(buffer, result);
}

bool CFileEncodingHelper::ConvertFileToUTF16(wistringstream & result) {
    result.str(L"");
    wstring buffer;
    if (!ConvertFileToUTF16(buffer)) {
        return false;
    }
    result.str(buffer);
    return true;
}

bool CFileEncodingHelper::ConvertFileToUTF16(wstring & result) {

    string unknown;
    if (!ReadFileIntoBuffer(unknown)) {
        return false;
    }
    if (IsUTF32(unknown)) {
        return false;
    } else if (IsUTF8(unknown)) {
        return ConvertUTF8ToUTF16(unknown, result, true);
    } else if (IsUTF16BE(unknown)) {
        return ConvertUTF16BEToUTF16(unknown, result);
    } else if (IsUTF16LE(unknown)) {
        return ConvertUTF16LEToUTF16(unknown, result);
    } else if (IsASCII(unknown)) {
        return ConvertASCIIToUTF16(unknown, result);
    }
    // unknown or no BOM - don't remove it
    return ConvertUTF8ToUTF16(unknown, result, false);
}

bool CFileEncodingHelper::ConvertUTF16ToUTF8(wstring & unknown, istringstream & result) {
    // remove the BOM
    int length2 = WideCharToMultiByte(CP_UTF8,0,unknown.data(),unknown.length(),NULL,0,NULL,NULL);
    if (length2==0) {
        return false;
    }
    result.str()="";
    string buffer;
    buffer.resize(length2);
    length2 = WideCharToMultiByte(CP_UTF8,0,unknown.data(),unknown.length(),&buffer[0],buffer.length(),NULL,NULL);
    if (length2==0) {
        return false;
    }
    result.str(buffer);
    return true;
}

bool CFileEncodingHelper::ReadFileIntoBuffer(string & unknown) {
    CSaString temp;
    temp = filename.c_str();

    ifstream file(temp.utf8().c_str(), std::ios::binary);
    if (!file) {
        // file failed to open
        return false;
    }

    // determine size
    file.seekg(0, std::ios::end);
    streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    unknown = "";
    char c;
    while (file.get(c)) {
        unknown.push_back(c);
    }

    TRACE("file length=%d\n",unknown.length());
    file.close();
    return true;
}

/**
* checks for LE or BE UTF32.
* we don't support either
*/
bool CFileEncodingHelper::IsUTF32(string & unknown) {
    //00 00 FE FF   UTF-32, big-endian
    //FF FE 00 00   UTF-32, little-endian
    if (unknown.length()<4) {
        return false;
    }
    unsigned char c1 = unknown[0];
    unsigned char c2 = unknown[1];
    unsigned char c3 = unknown[2];
    unsigned char c4 = unknown[3];
    if ((c1==0x00)&&(c2==0x00)&&(c3==0xfe)&&(c4==0xff)) {
        TRACE("stream is big-endian UTF32\n");
        return true;
    }
    if ((c1==0xff)&&(c2==0xfe)&&(c3==0x00)&&(c4==0x00)) {
        TRACE("stream is little-endian UTF32\n");
        return true;
    }
    TRACE("stream is not UTF32\n");
    return false;
}

bool CFileEncodingHelper::IsUTF8(string & unknown) {
    //EF BB BF      UTF-8
    if (unknown.length()<3) {
        return false;
    }
    unsigned char c1 = unknown[0];
    unsigned char c2 = unknown[1];
    unsigned char c3 = unknown[2];
    bool result = ((c1==0xef) && (c2==0xbb) && (c3==0xbf));
    TRACE("stream %s UTF8\n",((result)?"is":"is not"));
    return result;
}

bool CFileEncodingHelper::IsUTF16BE(string & unknown) {
    //FE FF         UTF-16, big-endian
    if (unknown.length()<2) {
        return false;
    }
    unsigned char c1 = unknown[0];
    unsigned char c2 = unknown[1];
    if ((c1==0xfe)&&(c2==0xff)) {
        TRACE("stream is big-endian UTF16\n");
        return true;
    }
    TRACE("stream is not big-endian UTF16\n");
    return false;
}

bool CFileEncodingHelper::IsUTF16LE(string & unknown) {
    //FF FE         UTF-16, little-endian
    if (unknown.length()<2) {
        return false;
    }
    unsigned char c1 = unknown[0];
    unsigned char c2 = unknown[1];
    if ((c1==0xff)&&(c2==0xfe)) {
        TRACE("stream is little-endian UTF16\n");
        return true;
    }
    TRACE("stream is not little-endian UTF16\n");
    return false;
}

bool CFileEncodingHelper::IsASCII(string & unknown) {
    for (size_t i=0; i<unknown.length(); i++) {
        unsigned char c = unknown[i];
        if (c>0x7f) {
            TRACE("stream is not ASCII\n");
            return false;
        }
    }
    TRACE("stream is ASCII\n");
    return true;
}

/**
* converts UTF16 big endian buffer to a UTF16 array
* if the function returns true, obuffer will be a pointer to the newly created array
* the caller is responsible for deleting buffer.
*/
bool CFileEncodingHelper::ConvertUTF16BEToUTF16(string & unknown, wstring & obuffer) {

    // remove BOM
    unknown.erase(unknown.begin());
    unknown.erase(unknown.begin());

    // swap the bytes
    for (size_t i=0; i<unknown.length();) {
        char c1 = unknown[i];
        char c2 = unknown[i+1];
        unknown[i] = c2;
        unknown[i+1] = c1;
        i+=2;
    }

    obuffer = L"";
    obuffer.resize(unknown.length()/2);
    memcpy(&obuffer[0],unknown.data(),unknown.length());

    return (obuffer.length()!=0);
}

/**
* converts UTF16 little endian buffer to a UTF16 array
* if the function returns true, obuffer will be a pointer to the newly created array
* the caller is responsible for deleting buffer.
*/
bool CFileEncodingHelper::ConvertUTF16LEToUTF16(string & unknown, wstring & obuffer) {

    // remove BOM
    unknown.erase(unknown.begin());
    unknown.erase(unknown.begin());

    obuffer = L"";
    obuffer.resize(unknown.length()/2);
    memcpy(&obuffer[0],unknown.data(),unknown.length());

    return (obuffer.length()!=0);
}

/**
* converts buffer to a UTF16 array
* if the function returns true, obuffer will be a pointer to the newly created array
* the caller is responsible for deleting buffer.
*/
bool CFileEncodingHelper::ConvertUTF8ToUTF16(string & unknown, wstring & obuffer, bool removeBOM) {

    // remove the BOM
    if (removeBOM) {
        unknown.erase(unknown.begin());
        unknown.erase(unknown.begin());
        unknown.erase(unknown.begin());
    }
    obuffer=L"";
    int length2 = MultiByteToWideChar(CP_UTF8,0,unknown.data(),unknown.length(),NULL,0);
    if (length2==0) {
        return false;
    }
    obuffer.resize(length2);
    length2 = MultiByteToWideChar(CP_UTF8,0,unknown.data(),unknown.length(),&obuffer[0],obuffer.length());
    return (length2!=0);
}

/**
* converts buffer to a UTF16 array
* if the function returns true, obuffer will be a pointer to the newly created array
* the caller is responsible for deleting buffer.
*/
bool CFileEncodingHelper::ConvertASCIIToUTF16(string & unknown, wstring & obuffer) {

    obuffer=L"";
    int length2 = MultiByteToWideChar(CP_ACP,0,unknown.data(),unknown.length(),NULL,0);
    if (length2==0) {
        return false;
    }
    obuffer.resize(length2);
    length2 = MultiByteToWideChar(CP_ACP,0,unknown.data(),unknown.length(),&obuffer[0],obuffer.length());
    return (length2!=0);
}

