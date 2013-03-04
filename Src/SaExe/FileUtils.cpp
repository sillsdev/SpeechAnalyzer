#ifndef FILEUTILS_H_
#define FILEUTILS_H

#include "stdafx.h"
#include <FileUtils.h>
#include <sys/stat.h>

/**
* return true if the path exists as a folder
*/
bool FileExists( LPCTSTR path) {

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
bool FolderExists( LPCTSTR path) {

	if (wcslen(path)==0) return false;

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
bool CreateFolder( LPCTSTR path) {

	if (FolderExists(path)) return true;
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

void AppendDirSep( LPTSTR szBuffer, size_t size) {

	size_t len = wcslen(szBuffer);
	if (len == 0) return;

	if (szBuffer[len - 1] == '\\') return;

	szBuffer[len] = '\\';
	szBuffer[len+1] = 0;
}

#endif
