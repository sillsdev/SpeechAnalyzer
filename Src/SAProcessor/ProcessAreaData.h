/////////////////////////////////////////////////////////////////////////////
// ProcessAreaData.h:
// Interface of the CProcess
//                  CAreaDataProcess classes
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef CPROCESSAREADATA_H
#define CPROCESSAREADATA_H

#include "sa_process.h"

class CProcessAreaData : public CProcess {

public:
    CProcessAreaData(Context& context);
    CProcessAreaData() = delete;

    virtual void UpdateArea();
    // m_dwAreaPos = pView->GetStartCursorPosition();
    // DWORD wSmpSize = model.GetSampleSize();
    // m_dwAreaLength = pView->GetStopCursorPosition() - m_dwAreaPos + wSmpSize;
    BOOL SetAreaFromView(DWORD startCursorPosition, DWORD stopCursorPosition);
    DWORD GetAreaPosition();
    DWORD GetAreaLength();

protected:
    BOOL SetArea(DWORD dwAreaPos, DWORD dwAreaLength);

private:
    DWORD  m_dwAreaPos;      // array of graph area positions
    DWORD  m_dwAreaLength;   // array of graph area lengths
};

class CProcessSDP : public CProcessAreaData {
public:
    CProcessSDP(Context& context) : CProcessAreaData(context) {};
    CProcessSDP() = delete;
};

#endif
