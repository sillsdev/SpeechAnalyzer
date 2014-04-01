#include "stdafx.h"
#include "stpwatch.h"


/***************************************************************************/
// CStopwatch::CreateObject
/***************************************************************************/
BOOL CStopwatch::CreateObject(CStopwatch ** ppStopwatch)
{
    if (!ppStopwatch)
    {
        return FALSE;    // bad pointer supplied
    }
    ASSERT(*ppStopwatch == NULL);
    *ppStopwatch = NULL;             // initialize to null in case of error
    if (clock() < 0)
    {
        return FALSE;    // couldn't get a clock resource
    }
    *ppStopwatch = new CStopwatch;   // try to construct a stopwatch object
    if (!ppStopwatch)
    {
        return FALSE;    // no memory available
    }
    return TRUE;                     // success
}

/***************************************************************************/
// CStopwatch::CStopwatch  constructor
/***************************************************************************/
CStopwatch::CStopwatch()
{
    m_fOverhead = 0.F;
    Reset();
    m_fOverhead = GetElapsedTime();  // compute calling overhead
}

/***************************************************************************/
// CStopwatch::CStopwatch  destructor
/***************************************************************************/
CStopwatch::~CStopwatch()
{
}

/***************************************************************************/
// CStopwatch::Reset  resets the stopwatch
/***************************************************************************/
void CStopwatch::Reset(void)
{
    m_ClockStart = clock();
}

/***************************************************************************/
// CStopwatch::GetElapsedTime  gets elapsed time in sec since starting clock
/***************************************************************************/
float CStopwatch::GetElapsedTime()
{
    float fElapsedTime = ((float)(clock() - m_ClockStart)/CLOCKS_PER_SEC - m_fOverhead);
    return fElapsedTime;
}

/***************************************************************************/
// CStopwatch::Wait  waits for specified time
/***************************************************************************/
void CStopwatch::Wait(float fTime)
{
    Reset();
    while (GetElapsedTime() < fTime); // loop until requested time has been reached
    // overhead compensation may be a little off here
}

