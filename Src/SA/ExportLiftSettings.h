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
    BOOL bPhrase;

    wstring gloss;
    wstring glossNat;
    wstring ortho;
    wstring phonemic;
    wstring phonetic;
    wstring reference;
    wstring phrase;

    CString szDocTitle;
    CString szPath;
};

#endif
