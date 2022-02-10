/////////////////////////////////////////////////////////////////////////////
// ProcessAreaData.h:
// Interface of the CProcess
//                  CAreaDataProcess classes
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef CPROCESSAREADATA_H
#define CPROCESSAREADATA_H

#include "Process.h"

class CProcessAreaData : public CProcess {

public:
    CProcessAreaData();
    virtual void UpdateArea();
    BOOL SetArea(View* pView);
    DWORD GetAreaPosition();
    DWORD GetAreaLength();

protected:
    BOOL SetArea(DWORD dwAreaPos, DWORD dwAreaLength);

private:
    DWORD  m_dwAreaPos;      // array of graph area positions
    DWORD  m_dwAreaLength;   // array of graph area lengths
};

class CProcessSDP : public CProcessAreaData {};

#endif
