#ifndef IMPORT_H
#define IMPORT_H

#include "CSaString.h"
#include "Sa_Doc.h"
#include <string>

using std::wstring;

class CImport
{
public:
    enum EImportMode { KEEP = 0, AUTO = 1, MANUAL = 2, QUERY = 3 };

    CImport(const CSaString & szFileName, BOOL batch=FALSE);
    BOOL Import( EImportMode nMode = QUERY);
    void AutoAlign( CSaDoc * pSaDoc, LPCTSTR pPhonetic, LPCTSTR pPhonemic, LPCTSTR pOrtho, LPCTSTR pGloss);

    CSaString m_szPath;
    BOOL m_bBatch;

private:
    BOOL ReadTable(Object_istream & ios, int nMode = QUERY);
	BOOL ProcessNormal( EImportMode nMode, wstring & result);
	BOOL ProcessTable( wstring & result);
};

#endif