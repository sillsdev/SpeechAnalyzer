#ifndef QPKSRC_H
#define QPKSRC_H

#include "pksrc.h"

// A QuadraticPeakSource is an iterator over peaks in a function.
// The peaks are returned in order of increasing location.
// See class PeakSource for more documentation.

class QuadraticPeakSource : public PeakSource
{
public:

    // Construct a Quadratic peak source with the given concavity
    // threshold.
    QuadraticPeakSource(float threshold);

    virtual void Search(const float * start, const float * end);
    virtual void Next();
    virtual void Get(float & location, float & value) const;
    virtual int IsDone() const;

private:
    // These should never be called
    QuadraticPeakSource(const QuadraticPeakSource &);
    const QuadraticPeakSource & operator=(const QuadraticPeakSource &);

    const float * myStart;
    const float * myMark;
    const float * myEnd;
    float myThreshold;
    float myLocation, myValue;
    int imDone;
};

#endif
