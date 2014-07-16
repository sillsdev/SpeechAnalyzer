#ifndef EXPORTLIFTSETTINGS_H
#define EXPORTLIFTSETTINGS_H

#include "SaString.h"

class CExportLiftSettings
{
public:
    BOOL bGloss;
    BOOL bOrtho;
    BOOL bPhonemic;
    BOOL bPhonetic;
    BOOL bPOS;
    BOOL bReference;
    BOOL bPhrase;
    CString szDocTitle;
    CString szPath;
};

#endif
