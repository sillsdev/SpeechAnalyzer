#include "stddsp.h"
#include "cspksrc.h"
#include <assert.h>

CountedSortedPeakSource::CountedSortedPeakSource(PeakSource & ps,
  uint32 maxNumPeaks)
  : myPS(ps), myQ(0), myMaxNumPeaks(maxNumPeaks)
{
}

CountedSortedPeakSource::~CountedSortedPeakSource()
{
  // I want to see how good the memory checker is :-)
  delete myQ;
}

void
CountedSortedPeakSource::Search(const float * start, const float * end)
{   
  uint32 numPeaks = 0;
  rpair_float_float point;

  // This time, we are sorting by frequency so point.first is location
  // and point.second is value.  This is backwards from EnergyPeakSource.
    
  delete myQ;
  myQ = new pq_rpair_float_float;
    
  for(myPS.Search(start, end);
    !myPS.IsDone() && numPeaks < myMaxNumPeaks;
    myPS.Next())
  {
    // Pull another peak, push it on the queue
    myPS.Get(point.first, point.second);
    myQ->push(point);
    numPeaks++;
  }  
}

void
CountedSortedPeakSource::Next()
{
  assert(!IsDone());
  myQ->pop();
}

void
CountedSortedPeakSource::Get(float & location, float & value) const
{
  assert(!IsDone());
  location = myQ->top().first;
  value = myQ->top().second;
}

int32 CountedSortedPeakSource::IsDone() const
{
  return myQ->empty();
}