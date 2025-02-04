#ifndef IMPORT_ELAN_H
#define IMPORT_ELAN_H

#include "SaString.h"
#include "StringStream.h"

class CSaDoc;

class CImportELAN {
public:
    enum EImportMode { KEEP = 0, AUTO = 1, MANUAL = 2, QUERY = 3 };

    CImportELAN(const CSaString & szFileName, BOOL batch=FALSE);
    BOOL Import(EImportMode nMode = QUERY);
    void AutoAlign(CSaDoc * pSaDoc, LPCTSTR pRef, LPCTSTR pPhonetic, LPCTSTR pPhonemic, LPCTSTR pOrtho, LPCTSTR pGloss, LPCTSTR pGlossNat);

    CSaString m_szPath;
    BOOL m_bBatch;

private:
    BOOL ReadTable(CStringStream & ios, int nMode = QUERY);
    bool ProcessNormal(wistringstream & stream, EImportMode nMode, wstring & result);
    BOOL ProcessColumnar(wistringstream & stream, wstring & result);
};

#endif
