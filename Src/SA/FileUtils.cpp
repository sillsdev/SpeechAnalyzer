#include "stdafx.h"
#include <FileUtils.h>
#include <sys/stat.h>
#include "SaString.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "FileUtils.h"

void FileUtils::GetTempFileName(LPCTSTR szPrefix, LPTSTR szFilename, size_t len) 
{

    TCHAR lpszTempPath[_MAX_PATH];
    wmemset(lpszTempPath,0,_MAX_PATH);
    GetTempPath(_MAX_PATH, lpszTempPath);

    wmemset(szFilename,0,len);
    ::GetTempFileName(lpszTempPath, szPrefix, 0, szFilename);
}

wstring FileUtils::GetTempFileName(LPCTSTR szPrefix) 
{

	wstring result;
    TCHAR szPath[_MAX_PATH];
    wmemset(szPath,0,_countof(szPath));
    GetTempPath(_MAX_PATH, szPath);
	TCHAR szFilename[_MAX_PATH];
    wmemset(szFilename,0,_countof(szFilename));
    ::GetTempFileName( szPath, szPrefix, 0, szFilename);
	result = szFilename;
	return result;
}

void FileUtils::RemoveFile(LPCTSTR path) 
{
    
	if (path==NULL) 
	{
        return;
    }
    if (wcslen(path)==0) 
	{
        return;
    }
    try 
	{
        CFileStatus status;
        if (CFile::GetStatus(path, status)) 
		{
            CFile::Remove(path);
        }
    } 
	catch (...) 
	{
        TRACE(L"failed to delete %s\n",path);
    }
}

/**
* return true if the path exists as a file
*/
bool FileUtils::FileExists( LPCTSTR path) 
{
    CFileStatus status;
    if (CFile::GetStatus(path,status)) 
	{
        if (!(status.m_attribute & CFile::directory)) 
		{
            return true;
        }
    }
    return false;
}

/**
* return true if the path exists as a folder
*/
bool FileUtils::FolderExists(LPCTSTR path) 
{

    if (wcslen(path)==0) 
	{
        return false;
    }

    CFileStatus status;
    if (CFile::GetStatus(path,status)) 
	{
        if (status.m_attribute & CFile::directory) 
		{
            return true;
        }
    }
    return false;
}

/**
* create a non-exitent folder
*/
bool FileUtils::CreateFolder(LPCTSTR path) 
{

    if (FolderExists(path)) 
	{
        return true;
    }
    if (FileExists(path)) 
	{
        // it exists, but it's not a directory
        TRACE1("%s already exists, but it's not a directory\n",path);
        return false;
    }
    TRACE1("creating %s\n",path);
    // it doesn't exist - create it!
    CreateDirectory(path, NULL);
    return true;
}

void FileUtils::AppendDirSep(LPTSTR path, size_t size) 
{

    size_t len = wcslen(path);
    if (len == 0) 
	{
        return;
    }

    if (path[len - 1] == '\\') 
	{
        return;
    }

    wcscat_s(path,size,L"\\");
}

void FileUtils::AppendDirSep( wstring & path) 
{

    size_t len = path.length();
    if (len == 0) 
	{
        return;
    }

    if (path[len - 1] == '\\') 
	{
        return;
    }

    path.append(L"\\");
}

int FileUtils::GetSaveAsFilename(LPCTSTR title, LPCTSTR filter, LPCTSTR extension, LPTSTR path, wstring & filename) 
{

    filename = L"";
    CSaString title2 = title;
    int nFind = title2.Find(':');
    if (nFind != -1) 
	{
        title2 = title2.Left(nFind);
		title2 = title2.Trim();
    }
    nFind = title2.ReverseFind('.');

    // remove extension
    title2.Trim();
    if (nFind >= ((title2.GetLength() > 3) ? (title2.GetLength() - 4) : 0)) 
	{
        title2 = title2.Left(nFind);
    }

    CFileDialog dlg(FALSE, extension, title2, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);

    if ((title2.GetLength()>0)&&(path!=NULL)) 
	{
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
    if (result == IDOK) 
	{
        // return the dialog result
        filename = dlg.GetPathName();
    }
    return result;
}

DWORD FileUtils::GetFileSize(LPCTSTR szFile) 
{
	CFileStatus stat;
    if (!CFile::GetStatus(szFile, stat)) 
	{
        return -1;
    }
    return stat.m_size;
}

/**
* returns the file size in bytes
*/
DWORD FileUtils::GetFileSize( LPCSTR filename) 
{
   DWORD size = 0;
   int fd = _sopen( filename, _O_RDONLY, _SH_DENYNO, 0);
   if ( fd != -1 ) {
	   struct _stat stat;
	   memset(&stat,0,sizeof(stat));
	   int result = _fstat( fd, &stat);
	   if (result==0) {
		   size = stat.st_size;
	   }
	   _close(fd);
	}
	return size;
}

bool FileUtils::EndsWith(LPCTSTR path, LPCTSTR extension) 
{
	if (wcslen(path)<wcslen(extension)) 
	{
        return false;
    }
    wstring sub(path);
    sub = sub.substr(wcslen(path) - wcslen(extension), wcslen(extension));
    return (_wcsicmp(sub.c_str(),extension)==0);
}

void FileUtils::RenameFile(LPCTSTR oldname, LPCTSTR newname) 
{
	CFile::Rename( oldname, newname);
}

bool FileUtils::IsReadOnly( LPCTSTR filename) 
{
    CFileStatus stat;
    if (!CFile::GetStatus(filename, stat)) 
	{
        return -1;
    }
	return ((stat.m_attribute&CFile::readOnly)==CFile::readOnly);
}

/**
* performs validation on a filename for the following characters.
**/
void FileUtils::DDX_Filename(CDataExchange* pDX, int nIDC, CString& value) 
{

	//  \ / : * ? “ < > |
   HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (pDX->m_bSaveAndValidate) 
	{
		CString temp;
        int nLen = ::GetWindowTextLength( hWndCtrl);
        ::GetWindowText( hWndCtrl, temp.GetBufferSetLength(nLen), nLen+1);
        temp.ReleaseBuffer();
		if (temp.FindOneOf(L"/\\:*?\"<>|")!=-1)
		{
            pDX->PrepareEditCtrl( nIDC);
            CString msg;
            msg.FormatMessage(IDS_ERROR_BADFILENAME_CHARS);
            AfxMessageBox(msg, MB_OK|MB_ICONEXCLAMATION, 0);
            pDX->Fail();
		}
    }
	DDX_Text(pDX, nIDC, value);
}

wstring FileUtils::NormalizePath( LPCTSTR path)
{
	wchar_t buffer[MAX_PATH];
	wmemset(buffer,0,_countof(buffer));
	// we need to normalize the path for this all to work correctly.
	wstring result = path;
	if (GetFullPathName( result.c_str(), _countof(buffer), buffer, NULL)!=0)
	{
		result = buffer;
	}
	return result;
}


