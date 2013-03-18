#ifndef IMPORT_H
#define IMPORT_H

#include "CSaString.h"
#include "Sa_Doc.h"

class CImport {
public:
    CImport(const CSaString & szFileName, BOOL batch=FALSE) {
        m_szPath = szFileName;
        m_bBatch = batch;
    }; // standard constructor

    CSaString m_szPath;
    BOOL m_bBatch;

    enum { KEEP = 0, AUTO = 1, MANUAL = 2, QUERY = 3 };

    BOOL Import(int nMode = QUERY);
    void AutoAlign(CSaDoc * pSaDoc, const CSaString * Phonetic, const CSaString * Phonemic, const CSaString * Ortho, const CSaString * Gloss);

private:
    BOOL ReadTable(Object_istream & ios, int nMode = QUERY);
};

#endif IMPORT_H
