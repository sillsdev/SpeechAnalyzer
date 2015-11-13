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
    GetTempDir(_MAX_PATH, lpszTempPath);
    wmemset(szFilename,0,len);
    ::GetTempFileName(lpszTempPath, szPrefix, 0, szFilename);
}

void FileUtils::GetTempDir( size_t len, LPTSTR buffer) {

    wmemset( buffer, 0, len);
	// GetTempPath always ends with a directory seperator
    GetTempPath(_MAX_PATH, buffer);
    size_t strlen = wcslen(buffer);
    if (strlen != 0) {
		if (buffer[strlen-1] != '\\') {
		    wcscat_s(buffer,_MAX_PATH,L"\\");
		}
	}
    strlen = wcslen(buffer);
    if (strlen != 0) {
		if (buffer[strlen-1] == '\\') {
		    wcscat_s(buffer,_MAX_PATH,L"SpeechAnalyzer\\");
		}
	}
    if (!FolderExists(buffer)) {
        //create directory ?
		if (CreateFolder(buffer)==0) {
		}
	}
}

wstring FileUtils::GetTempFileName(LPCTSTR szPrefix) {
    wstring result;
    TCHAR szPath[_MAX_PATH];
	GetTempDir(_MAX_PATH,szPath);
    TCHAR szFilename[_MAX_PATH];
    wmemset(szFilename,0,_countof(szFilename));
    ::GetTempFileName(szPath, szPrefix, 0, szFilename);
    result = szFilename;
    return result;
}

void FileUtils::Remove(LPCTSTR path) {
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
* create a non-existent folder
*/
bool FileUtils::CreateFolder(LPCTSTR path) {
    
	if (FolderExists(path)) {
        return true;
    }
    if (FileExists(path)) {
        // it exists, but it's not a folder
        return false;
    }

	// split it and see if the parent exists
	wstring parent = GetParentFolder(path);
	if (FolderExists(parent.c_str())) {
		// the parent is OK, just create this one
		// it doesn't exist - create it!
		// call the Win32 function
		CreateDirectory(path,NULL);
		return true;
	}

	// the parent folder doesn't exist - create that first
	if (!CreateFolder(parent.c_str())) {
		return false;
	}
	if (!CreateFolder(path)) {
		return false;
	}
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

void FileUtils::Rename(LPCTSTR oldname, LPCTSTR newname) {
	// clear the way!
	DeleteFile(newname);
    _wrename(oldname, newname);
}

void FileUtils::Copy(LPCTSTR src, LPCTSTR dst) {
	// clear the way!
	DeleteFile(dst);
	::CopyFileW( src, dst, false);
}

bool FileUtils::IsReadOnly(LPCTSTR filename) {
    DWORD dwFileAttributes = GetFileAttributes(filename);
    // oh no! can't determine attribute
    if (dwFileAttributes==INVALID_FILE_ATTRIBUTES) {
        return true;
    }
    return (dwFileAttributes & FILE_ATTRIBUTE_READONLY);
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

/**
* Replace existing extension
* caller must supply '.' in new extension name
*/
wstring FileUtils::ReplaceExtension(LPCTSTR path, LPCTSTR extension) {
	wstring filename = path;
	size_t index = filename.find_last_of('.');
	if (index != wstring::npos) {
		filename = filename.substr(0,index);
	}
	filename.append(extension);
	return filename;
}

/**
* Remove existing extension
*/
wstring FileUtils::RemoveExtension(LPCTSTR path) {
	wstring filename = path;
	size_t index = filename.find_last_of('.');
	if (index != wstring::npos) {
		filename = filename.substr(0,index);
	}
	return filename;
}

/**
* Remove existing extension
*/
wstring FileUtils::GetExtension(LPCTSTR path) {
	wstring extension = path;
	size_t index = extension.find_last_of('.');
	if (index != wstring::npos) {
		extension = extension.substr(index+1);
	}
	return extension;
}

// remove parent folder
wstring FileUtils::GetFilename(LPCTSTR path) {
	wstring result = path;
    size_t pos = result.rfind('\\');
    if (pos!=wstring::npos) {
        result = result.substr(pos+1,result.length()-pos-1);
    }
	return result;
}

wstring FileUtils::GetParentFolder( LPCTSTR path) {

	wstring temp = path;
	if (temp[temp.size()-1]=='\\') {
		temp = temp.substr(0,temp.size()-1);
	}

	wchar_t buffer[MAX_PATH];
	swprintf_s(buffer,_countof(buffer),temp.c_str());

	wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];
    wchar_t fname[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];
    _wsplitpath_s(buffer, drive, dir, fname, ext);

	wstring result;
	result.append(drive);
	result.append(dir);
    size_t pos = result.rfind('\\');
    if (pos==wstring::npos) {
		result.append(L"\\");
	}
	return result;
}

