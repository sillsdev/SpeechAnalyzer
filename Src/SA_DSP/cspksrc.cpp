#include "stddsp.h"
#include "cspksrc.h"
#include <assert.h>

CCountedSortedPeakSource::CCountedSortedPeakSource(CPeakSource & ps,
        uint32 maxNumPeaks)
    : myPS(ps), myQ(0), myMaxNumPeaks(maxNumPeaks)
{
}

CCountedSortedPeakSource::~CCountedSortedPeakSource()
{
    // I want to see how good the memory checker is :-)
    delete myQ;
}

void CCountedSortedPeakSource::Search(const float * start, const float * end)
{
    uint32 numPeaks = 0;
    rpair_float_float point;

    // This time, we are sorting by frequency so point.first is location
    // and point.second is value.  This is backwards from CEnergyPeakSource.

    delete myQ;
    myQ = new pq_rpair_float_float;

    for (myPS.Search(start, end);
            !myPS.IsDone() && numPeaks < myMaxNumPeaks;
            myPS.Next())
    {
        // Pull another peak, push it on the queue
        myPS.Get(point.first, point.second);
        myQ->push(point);
        numPeaks++;
    }
}

void CCountedSortedPeakSource::Next()
{
    assert(!IsDone());
    myQ->pop();
}

void CCountedSortedPeakSource::Get(float & location, float & value) const
{
    assert(!IsDone());
    location = myQ->top().first;
    value = myQ->top().second;
}

int32 CCountedSortedPeakSource::IsDone() const
{
    return myQ->empty();
}