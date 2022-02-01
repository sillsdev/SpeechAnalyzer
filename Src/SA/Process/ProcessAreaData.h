/////////////////////////////////////////////////////////////////////////////
// ProcessAreaData.h:
// Interface of the CProcess
//                  CAreaDataProcess classes
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef CPROCESSAREADATA_H
#define CPROCESSAREADATA_H

#include "Process/Process.h"

class CProcessAreaData : public CProcess {

public:
    CProcessAreaData();
    virtual ~CProcessAreaData();

    // Attributes
private:
    DWORD  m_dwAreaPos;      // array of graph area positions
    DWORD  m_dwAreaLength;   // array of graph area lengths

    // Operations
public:
    virtual void UpdateArea();
    BOOL SetArea(CSaView * pView);

    DWORD GetAreaPosition();
    DWORD GetAreaLength();

protected:
    BOOL SetArea(DWORD dwAreaPos, DWORD dwAreaLength);
};

class CProcessSDP : public CProcessAreaData {};

#endif
