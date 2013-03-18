#include "stdafx.h"
#include <FileUtils.h>
#include <sys/stat.h>
#include "CSaString.h"

void GetTempFileName(LPCTSTR szPrefix, LPTSTR szFilename, size_t len) {

    TCHAR lpszTempPath[_MAX_PATH];
    wmemset(lpszTempPath,0,_MAX_PATH);
    GetTempPath(_MAX_PATH, lpszTempPath);

    wmemset(szFilename,0,len);
    GetTempFileName(lpszTempPath, szPrefix, 0, szFilename);
}

void RemoveFile(LPCTSTR path) {
    if (path==NULL) {
        return;
    }
    if (wcslen(path)==0) {
        return;
    }
    try {
        CFileStatus status;
        if (CFile::GetStatus(path, status)) {
            CFile::Remove(path);
        }
    } catch (...) {
        TRACE("failed to delete %s\n",path);
    }
}

/**
* return true if the path exists as a folder
*/
bool FileExists(LPCTSTR path) {

    CFileStatus status;
    if (CFile::GetStatus(path,status)) {
        if (!(status.m_attribute & CFile::directory)) {
            return true;
        }
    }
    return false;
}

/**
* return true if the path exists as a folder
*/
bool FolderExists(LPCTSTR path) {

    if (wcslen(path)==0) {
        return false;
    }

    CFileStatus status;
    if (CFile::GetStatus(path,status)) {
        if (status.m_attribute & CFile::directory) {
            return true;
        }
    }
    return false;
}

/**
* create a non-exitent folder
*/
bool CreateFolder(LPCTSTR path) {

    if (FolderExists(path)) {
        return true;
    }
    if (FileExists(path)) {
        // it exists, but it's not a directory
        TRACE1("%s already exists, but it's not a directory\n",path);
        return false;
    }
    TRACE1("creating %s\n",path);
    // it doesn't exist - create it!
    CreateDirectory(path, NULL);
    return true;
}

void AppendDirSep(LPTSTR path, size_t size) {

    size_t len = wcslen(path);
    if (len == 0) {
        return;
    }

    if (path[len - 1] == '\\') {
        return;
    }

    wcscat_s(path,size,L"\\");
}

void AppendDirSep(wstring & path) {

    size_t len = path.length();
    if (len == 0) {
        return;
    }

    if (path[len - 1] == '\\') {
        return;
    }

    path.append(L"\\");
}

int GetSaveAsFilename(LPCTSTR title, LPCTSTR filter, LPCTSTR extension, LPTSTR path, wstring & filename) {

    filename = L"";
    CSaString title2 = title;

    int nFind = title2.Find(':');
    if (nFind != -1) {
        title2 = title2.Left(nFind);
    }
    nFind = title2.ReverseFind('.');

    // remove extension
    title2.Trim();
    if (nFind >= ((title2.GetLength() > 3) ? (title2.GetLength() - 4) : 0)) {
        title2 = title2.Left(nFind);
    }

    CFileDialog dlg(FALSE, extension, title2, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);

    if ((title2.GetLength()>0)&&(path!=NULL)) {
        TCHAR temp[MAX_PATH];
        wmemset(temp,0,MAX_PATH);
        wcscat_s(temp,MAX_PATH,path);
        AppendDirSep(temp,MAX_PATH);
        wcscat_s(temp,MAX_PATH,title2);
        wcscat_s(temp,MAX_PATH,L".");
        wcscat_s(temp,MAX_PATH,extension);
        dlg.m_ofn.lpstrFile = temp;
        dlg.m_ofn.lpstrInitialDir = path;
    }

    int result = dlg.DoModal();
    if (result == IDOK) {
        // return the dialog result
        filename = dlg.GetPathName();
    }
    return result;
}
