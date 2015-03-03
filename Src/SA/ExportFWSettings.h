#ifndef EXPORTFWSETTINGS_H
#define EXPORTFWSETTINGS_H

#include "SaString.h"

class CExportFWSettings {

public:
    BOOL bGloss;
    BOOL bGlossNat;
    BOOL bOrtho;
    BOOL bTone;
    BOOL bPhonemic;
    BOOL bPhonetic;
    BOOL bReference;
    BOOL bPhrase;
    CString szDocTitle;
    CString szPath;
};

#endif
