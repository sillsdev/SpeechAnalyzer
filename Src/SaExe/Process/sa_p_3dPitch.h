#ifndef SA_P_3DPITCH_H
#define SA_P_3DPITCH_H

#include "Sa_proc.h"

class CProcess3dPitch : public CDataProcess {
public:
    CProcess3dPitch();           // protected constructor used by dynamic creation
    virtual ~CProcess3dPitch();

public:
    void SetSourceProcess(CDataProcess * pSourceProcess);
    virtual long Process(void * pCaller, ISaDoc *, int nProgress = 0, int nLevel = 1);

private:
    double m_dFilterUpperFrequency;
    double m_dFilterLowerFrequency;
    int m_nFilterOrder;
    CDataProcess * m_pSourceProcess;

protected:

    DECLARE_MESSAGE_MAP()
};

#endif
