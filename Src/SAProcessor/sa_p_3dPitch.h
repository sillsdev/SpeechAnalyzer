#ifndef SA_P_3DPITCH_H
#define SA_P_3DPITCH_H

#include "sa_process.h"

__interface Process;

class CProcess3dPitch : public CProcess {
public:
    CProcess3dPitch(Context& context);
    CProcess3dPitch() = delete;

    virtual long Process(void * pCaller, int nProgress = 0, int nLevel = 1);

private:
    double m_dFilterUpperFrequency;
    double m_dFilterLowerFrequency;
    int m_nFilterOrder;
};

#endif
