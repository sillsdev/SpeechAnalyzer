#ifndef EXPORTLIFTSETTINGS_H
#define EXPORTLIFTSETTINGS_H

#include "SaString.h"

class CExportLiftSettings {
public:
    BOOL bGloss;
    BOOL bGlossNat;
    BOOL bOrtho;
    BOOL bPhonemic;
    BOOL bPhonetic;
    BOOL bReference;
    BOOL bPhrase1;
    BOOL bPhrase2;
    BOOL bPhrase3;
    BOOL bPhrase4;

    wstring gloss;
    wstring glossNat;
    wstring ortho;
    wstring phonemic;
    wstring phonetic;
    wstring reference;
    wstring phrase1;
    wstring phrase2;

    CSaString optionalLanguageTag;
    CString szDocTitle;
    CString szPath;

    // Constants for preferences from last Lift export (Gloss, Phonemic, and Phonetic are already hard-coded)
    const LPCWSTR LAST_EXPORT           = L"LastExport";
    const LPCWSTR LAST_EXPORT_GLOSS_NAT = L"LastExport.GlossNat";
    const LPCWSTR LAST_EXPORT_ORTHO     = L"LastExport.Ortho";
    const LPCWSTR LAST_EXPORT_REFERENCE = L"LastExport.Reference";
    const LPCWSTR LAST_EXPORT_PHRASE1   = L"LastExport.Phrase1";
    const LPCWSTR LAST_EXPORT_PHRASE2   = L"LastExport.Phrase2";
    const LPCWSTR LAST_EXPORT_OPTIONAL_LANGUAGE_TAG = L"LastExport.OptionalLanguageTag";
};

#endif
