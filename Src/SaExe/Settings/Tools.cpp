#include "stdSettings.h"
#include "tools.h"
#include <io.h>  // _access


bool bFileExists(const char * pszFile)
{
    return _access(pszFile, 0) == 0; // _access returns 0 on success
}

