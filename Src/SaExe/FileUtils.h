#ifndef FILEUTILS_H
#define FILEUTILS_H

#define DIRECTORY_SEPARATOR '\\'

#include <string>
using std::wstring;

void GetTempFileName(LPCTSTR szPrefix, LPTSTR szTempFileName, size_t len);
extern void RemoveFile(LPCTSTR path);
extern void RenameFile(LPCTSTR oldname, LPCTSTR newname);
extern bool FileExists(LPCTSTR path);
extern bool FolderExists(LPCTSTR path);
extern bool CreateFolder(LPCTSTR path);
extern void AppendDirSep(LPTSTR path, size_t size);
extern void AppendDirSep(wstring & ref);
extern int GetSaveAsFilename(LPCTSTR title, LPCTSTR filter, LPCTSTR extension, LPTSTR path, wstring & result);
extern bool EndsWith(LPCTSTR path, LPCTSTR extension);

#endif
