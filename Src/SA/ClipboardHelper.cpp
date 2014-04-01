#include "Stdafx.h"
#include "ClipboardHelper.h"
#include "FileUtils.h"

/************************************************************************
* reads data from a HGLOBAL object from the clipboard and
* writes it to a file.
************************************************************************/
bool CClipboardHelper::LoadFileFromData(HGLOBAL hData, LPTSTR szFilename, size_t len)
{

    // because we now use true CF_WAVE we can save as temp then open
    // temporary target file has to be created
    TCHAR szTempPath[_MAX_PATH];
    wmemset(szTempPath,0,_MAX_PATH);
    wmemset(szFilename,0,len);
    GetTempPath(_MAX_PATH, szTempPath);
    GetTempFileName(szTempPath, _T("WAV"), 0, szFilename);

    if ((::GlobalFlags(hData)&~GMEM_LOCKCOUNT)==GMEM_DISCARDED)
    {
        TRACE("global memory is already discarded\n");
        return false;
    }

    HPSTR lpData = (HPSTR)::GlobalLock(hData); // lock memory
    DWORD dwSize = ::GlobalSize(hData);

    {
        CFile file;
        if (!file.Open(szFilename, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive))
        {
            TRACE("unable to open file for writing\n");
            FileUtils::RemoveFile(szFilename);
            ::GlobalUnlock(hData);
            return false;
        }
        file.Write(lpData,dwSize);
        file.Close();
    }

    ::GlobalUnlock(hData);

    return true;
}
