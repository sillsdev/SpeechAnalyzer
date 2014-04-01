#ifndef CLIPBOARDHELPER_H
#define CLIPBOARDHELPER_H

class CClipboardHelper
{
public:
    static bool LoadFileFromData(HGLOBAL hGlobal, LPTSTR szFilename, size_t len);
};

#endif


