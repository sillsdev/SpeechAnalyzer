#ifndef IWAVENOTIFIABLE
#define IWAVENOTIFIABLE

__interface IWaveNotifiable
{
    void BlockStored(UINT nLevel, DWORD dwPosition, BOOL * bSaveOverride = NULL);
    void BlockFinished(UINT nLevel, DWORD dwPosition, UINT = 100);
    void StoreFailed();
    void EndPlayback();
    HPSTR GetWaveData(DWORD dwPlayPosition, DWORD dwDataSize);
};

#endif
