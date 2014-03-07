#include "stddsp.h"
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include "qpksrc.h"

// Fit a parabola p[0] + p[1] x + p[2] x^2 to the points
// (-1,y[0]), (0,y[1]), (1,y[2]).
static void FitParabola(float p[3], const float y[3])
{
    // y[1] == p[0]
    // y[2] == p[0] + p[1] + p[2]
    // y[0] == p[0] - p[1] + p[2]
    p[0] = y[1];
    p[1] = (y[2]-y[0])/2.0f;
    p[2] = (y[2]+y[0])/2.0f - p[0];
}

// Look for the next peak, searching [first..last).
// Return NULL if there is no peak to be found.
// first is set to where it stopped looking so you can call this
// again.
static const float *
FindNextPeak(const float *& first, const float * const last,
             const float concavityThreshold)
{
    assert(first != NULL);
    assert(last != NULL);
    assert(first < last);

    // Handle degenerate cases:

    // If only one or two points, not enough for a peak.
    if (last - first < 2)
    {
        return NULL;
    }

    // Handle usual cases.
    float bestDerivative = FLT_MAX;
    const float * bestLocation = NULL;
    const float * y;
    float p[3];

    // For each triple of points
    // go until we first get to a reasonably concave portion.
    y = first;

    while (y+2 < last)
    {
        // Fit a parabola to the points at y, y+1, y+2
        FitParabola(p, y);

        // The second derivative at y+1 is given by p[2]*2
        if (p[2]*2.0f < concavityThreshold)
        {
            break;
        }

        y++;
    }

    // Now that we're on the peak, go until we fall off.
    // Record the point whose derivative is closest to 0.
    while (y < last)
    {
        FitParabola(p, y);
        // If we've passed to a region of convexity, stop
        if (p[2]*2.0f > concavityThreshold)
        {
            break;
        }

        // The derivative at y+1 is given by p[1].
        // If we've found a flatter spot, save it.
        if (float(fabs(p[1])) < bestDerivative)
        {
            bestDerivative = float(fabs(p[1]));
            bestLocation = y+1;
        }

        y++;
    }

    if (y+2 < last)
    {
        // Means we found a complete peak
        assert(bestDerivative < FLT_MAX);
        first = y+1;
        return bestLocation;
    }
    else
    {
        // Ran out of peaks
        first = last;
        return NULL;
    }
}

CQuadraticPeakSource::CQuadraticPeakSource(float threshold)
    : myThreshold(threshold)
{
}

void
CQuadraticPeakSource::Search(const float * start, const float * end)
{
    assert(start);
    assert(end);
    assert(start <= end);

    myStart = start;
    myEnd = end;
    myMark = start;

    if (start == end)
    {
        imDone = 1;
    }
    else
    {
        imDone = 0;
        Next();
    }
}

void
CQuadraticPeakSource::Next()
{
    assert(myStart);
    assert(myMark);
    assert(myEnd);
    assert(myMark <= myEnd);
    assert(!IsDone());

    const float * peakLocation = FindNextPeak(myMark, myEnd, myThreshold);

    if (peakLocation)
    {
        myLocation = float(peakLocation - myStart);
        myValue = *peakLocation;
    }
    else
    {
        imDone = 1;
    }
}

void
CQuadraticPeakSource::Get(float & location, float & value) const
{
    assert(!IsDone());

    location = myLocation;
    value = myValue;
}

int32
CQuadraticPeakSource::IsDone() const
{
    return imDone;
}