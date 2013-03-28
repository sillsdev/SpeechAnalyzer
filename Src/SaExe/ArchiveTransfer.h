#ifndef ARCHIVE_TRANSFER_H
#define ARCHIVE_TRANSFER_H

class ArchiveTransfer
{
public:
    static int tInt(CArchive & ar, int x=0);
    static short int tShortInt(CArchive & ar, short int x=0);
    static DWORD tDWORD(CArchive & ar, DWORD x=0);
};

#endif

