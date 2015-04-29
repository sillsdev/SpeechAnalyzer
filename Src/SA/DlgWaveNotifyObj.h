#ifndef DLGWAVENOTIFYOBJ_H
#define DLGWAVENOTIFYOBJ_H

#include "sa_dlwnd.h"
#include "sa_wave.h"
#include "fnkeys.h"
#include "IWaveNotifiable.h"

class CSaDoc;

class CDlgWaveNotifyObj : public CWaveNotifyObj {
public:
    CDlgWaveNotifyObj();
    ~CDlgWaveNotifyObj();

public:
    void Attach(IWaveNotifiable * pClient);
    virtual void BlockFinished(UINT nLevel, DWORD dwPosition, UINT = 100);
    virtual void BlockStored(UINT nLevel, DWORD dwPosition, BOOL * bSaveOverride = NULL);
    virtual void StoreFailed();
    virtual void EndPlayback();
    virtual HPSTR GetWaveData(CView * pView, DWORD dwPlayPosition, DWORD dwDataSize);

private:
    IWaveNotifiable * m_pClient;
};

#endif
