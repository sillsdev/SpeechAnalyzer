#ifndef STPWATCH_H
#define STPWATCH_H

#include <time.h>

class CStopwatch {
public:
    static BOOL CreateObject(CStopwatch ** ppStopwatch);
    void Reset(void);
    float GetElapsedTime(void);
    void Wait(float fTimeInSec);
    ~CStopwatch();

private:
    CStopwatch();
    time_t m_ClockStart;
    float m_fOverhead;
};


#endif

