#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <windows.h>
#include <string>

using std::string;
using std::wstring;

#define DIRECTORY_SEPARATOR '\\'

namespace FileUtils {

wstring Trim(wstring & in);
wstring GetTempFileName(LPCTSTR szPrefix);
void GetTempFileName(LPCTSTR szPrefix, LPTSTR szTempFileName, size_t len);
extern void Remove(LPCTSTR path);
extern void Rename(LPCTSTR oldname, LPCTSTR newname);
extern void Copy(LPCTSTR src, LPCTSTR dst);
extern bool FileExists(LPCTSTR path);
extern bool FolderExists(LPCTSTR path);
extern bool CreateFolder(LPCTSTR path);
extern void AppendDirSep(LPTSTR path, size_t size);
extern void AppendDirSep(wstring & ref);
extern DWORD GetFileSize(LPCTSTR path);
extern DWORD GetFileSize(LPCSTR path);
extern bool EndsWith(LPCTSTR path, LPCTSTR extension);
extern bool IsReadOnly(LPCTSTR path);
extern wstring NormalizePath(LPCTSTR path);
extern wstring ReplaceExtension(LPCTSTR path, LPCTSTR extension);
extern wstring RemoveExtension(LPCTSTR path);
extern wstring GetExtension(LPCTSTR path);
extern wstring GetFilename(LPCTSTR path);
extern wstring GetParentFolder(LPCTSTR path);
}

#endif
