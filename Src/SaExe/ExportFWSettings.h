#ifndef EXPORTFWSETTINGS_H
#define EXPORTFWSETTINGS_H

#include "CSaString.h"

class CExportFWSettings {

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
