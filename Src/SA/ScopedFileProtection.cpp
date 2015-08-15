#include "StdAfx.h"
#include "ScopedFileProtection.h"

/**
* A class to retrieve, remove and restore the readonly file attribute
* on the named file
*/
CScopedFileProtection::CScopedFileProtection(LPCTSTR _filename) :
filename(_filename),
success(FALSE) {
	TRACE("retrieving file status\n");
	success = CFile::GetStatus( filename,save);
	if ((success) && (save.m_attribute&CFile::readOnly)) {
		TRACE("clearing readonly flag\n");
		CFileStatus newstatus = save;
		newstatus.m_attribute &= ~CFile::readOnly;
		CFile::SetStatus( filename, newstatus);
	}
}

CScopedFileProtection::~CScopedFileProtection(void) {
	if ((success) && (save.m_attribute&CFile::readOnly)) {
		TRACE("restoring readonly flag\n");
		CFile::SetStatus(filename,save);
	}
}
