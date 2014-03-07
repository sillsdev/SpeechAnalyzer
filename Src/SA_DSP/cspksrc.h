#ifndef CSPKSRC_H
#define CSPKSRC_H

#include "pksrc.h"
#include "template.h"

// A CCountedSortedPeakSource reads peaks from another peak source up to
// a specified maximum number.  It returns the peaks in increasing order of location.
// This is an instance of the Decorator design pattern.
class CCountedSortedPeakSource : public CPeakSource
{
public:
    // Construct this peak source as a wrapper around the given peak
    // source.  The maximum number of peaks to find is specified by
    // maxNumPeaks.
    CCountedSortedPeakSource(CPeakSource & ps, unsigned int maxNumPeaks);
    ~CCountedSortedPeakSource();

    virtual void Search(const float * start, const float * end);
    virtual void Next();
    virtual void Get(float & location, float & value) const;
    virtual int IsDone() const;

private:
    // These should never be called
    CCountedSortedPeakSource(const CCountedSortedPeakSource &);
    const CCountedSortedPeakSource & operator=(const CCountedSortedPeakSource &);

    CPeakSource & myPS;
    pq_rpair_float_float * myQ;
    unsigned int myMaxNumPeaks;
};

#endif
