#include <sys/stat.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "FileUtils.h"

void FileUtils::GetTempFileName(LPCTSTR szPrefix, LPTSTR szFilename, size_t len) {
    TCHAR lpszTempPath[_MAX_PATH];
    wmemset(lpszTempPath,0,_MAX_PATH);
    GetTempPath(_MAX_PATH, lpszTempPath);
    wmemset(szFilename,0,len);
    ::GetTempFileName(lpszTempPath, szPrefix, 0, szFilename);
}

wstring FileUtils::GetTempFileName(LPCTSTR szPrefix) {
    wstring result;
    TCHAR szPath[_MAX_PATH];
    wmemset(szPath,0,_countof(szPath));
    GetTempPath(_MAX_PATH, szPath);
    TCHAR szFilename[_MAX_PATH];
    wmemset(szFilename,0,_countof(szFilename));
    ::GetTempFileName(szPath, szPrefix, 0, szFilename);
    result = szFilename;
    return result;
}

void FileUtils::RemoveFile(LPCTSTR path) {
    // bad path
    if (path==NULL) {
        return;
    }
    // empty path
    if (wcslen(path)==0) {
        return;
    }
    struct _stat64i32 stat;
    memset(&stat,0,sizeof(stat));
    int result = _wstat(path,&stat);
    // file is not accessible
    if (result!=0) {
        return;
    }
    // perform action
    _wremove(path);
}

/**
* return true if the path exists as a file
*/
bool FileUtils::FileExists(LPCTSTR path) {
    // bad path
    if (path==NULL) {
        return false;
    }
    // empty path
    if (wcslen(path)==0) {
        return false;
    }
    struct _stat64i32 stat;
    memset(&stat,0,sizeof(stat));
    int result = _wstat(path,&stat);
    // file is not accessible
    if (result!=0) {
        return false;
    }
    // is it a file?
    return ((stat.st_mode&_S_IFDIR)==0);
}

/**
* return true if the path exists as a folder
*/
bool FileUtils::FolderExists(LPCTSTR path) {
    // bad path
    if (path==NULL) {
        return false;
    }
    // empty path
    if (wcslen(path)==0) {
        return false;
    }
    wstring temp = path;
    if (temp[temp.size()-1]=='\\') {
        temp[temp.size()-1] = 0;
    }
    struct _stat64i32 stat;
    memset(&stat,0,sizeof(stat));
    int result = _wstat(temp.c_str(),&stat);
    // file is not accessible
    if (result!=0) {
        return false;
    }
    // is it a file?
    return ((stat.st_mode&_S_IFDIR)==_S_IFDIR);
}

/**
* create a non-exitent folder
*/
bool FileUtils::CreateFolder(LPCTSTR path) {
    if (FolderExists(path)) {
        return true;
    }
    if (FileExists(path)) {
        // it exists, but it's not a directory
        return false;
    }
    // it doesn't exist - create it!
    CreateDirectory(path, NULL);
    return true;
}

void FileUtils::AppendDirSep(LPTSTR path, size_t size) {

    size_t len = wcslen(path);
    if (len == 0) {
        return;
    }

    if (path[len - 1] == '\\') {
        return;
    }

    wcscat_s(path,size,L"\\");
}

void FileUtils::AppendDirSep(wstring & path) {

    size_t len = path.length();
    if (len == 0) {
        return;
    }

    if (path[len - 1] == '\\') {
        return;
    }

    path.append(L"\\");
}

wstring FileUtils::Trim(wstring & in) {
    wstring result = in;
    while ((result.size()>0) && (::iswspace(result[0]))) {
        result.erase(0);
    }
    while ((result.size()>0) && (::iswspace(result[result.size()-1]))) {
        result.erase(result.size()-1);
    }
    return result;
}

DWORD FileUtils::GetFileSize(LPCTSTR path) {
    // bad path
    if (path==NULL) {
        return -1;
    }
    // empty path
    if (wcslen(path)==0) {
        return -1;
    }
    struct _stat64i32 stat;
    memset(&stat,0,sizeof(stat));
    int result = _wstat(path,&stat);
    // file is not accessible
    if (result!=0) {
        return -1;
    }
    return stat.st_size;
}

/**
* returns the file size in bytes
*/
DWORD FileUtils::GetFileSize(LPCSTR path) {
    // bad path
    if (path==NULL) {
        return -1;
    }
    // empty path
    if (strlen(path)==0) {
        return -1;
    }
    struct _stat64i32 stat;
    memset(&stat,0,sizeof(stat));
    int result = _stat(path,&stat);
    // file is not accessible
    if (result!=0) {
        return -1;
    }
    return stat.st_size;
}

bool FileUtils::EndsWith(LPCTSTR path, LPCTSTR extension) {
    if (wcslen(path)<wcslen(extension)) {
        return false;
    }
    wstring sub(path);
    sub = sub.substr(wcslen(path) - wcslen(extension), wcslen(extension));
    return (_wcsicmp(sub.c_str(),extension)==0);
}

void FileUtils::RenameFile(LPCTSTR oldname, LPCTSTR newname) {
    _wrename(oldname, newname);
}

bool FileUtils::IsReadOnly(LPCTSTR filename) {
    int result = _waccess(filename,04);
    if (result!=0) {
        // true if file is missing or does not have attribute
        return false;
    }
    return true;
}

wstring FileUtils::NormalizePath(LPCTSTR path) {
    wchar_t buffer[MAX_PATH];
    wmemset(buffer,0,_countof(buffer));
    // we need to normalize the path for this all to work correctly.
    wstring result = path;
    if (GetFullPathName(result.c_str(), _countof(buffer), buffer, NULL)!=0) {
        result = buffer;
    }
    return result;
}

