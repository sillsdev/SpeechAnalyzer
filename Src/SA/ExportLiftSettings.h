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
};

#endif
