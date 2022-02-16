#ifndef SA_P_3DPITCH_H
#define SA_P_3DPITCH_H

#include "Process.h"

class CProcess3dPitch : public CProcess {
public:
    CProcess3dPitch(Context * pContext);           // protected constructor used by dynamic creation
    virtual ~CProcess3dPitch();

    void SetSourceProcess(IProcess * pSourceProcess);
    virtual long Process(void * pCaller, Model *, int nProgress = 0, int nLevel = 1);

private:
    double m_dFilterUpperFrequency;
    double m_dFilterLowerFrequency;
    int m_nFilterOrder;
};

#endif
