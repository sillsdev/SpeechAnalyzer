#ifndef SEGMENT_OPS
#define SEGMENT_OPS

#include "AppDefs.h"

class CSaDoc;

/**
* This class is an encapsulation of segment operations.
* This class contains functions that are used to manipulate all segments.
* Specific implementations are stored in Segment derived classes.
*/
class CSegmentOps {
public:
	CSegmentOps(CSaDoc & document);
	~CSegmentOps();
	void ShrinkSegments( WAVETIME sectionStart, WAVETIME sectionLength);		// remove a section
	void GrowSegments( WAVETIME sectionStart, WAVETIME sectionLength);			// insert a section

private:
	CSaDoc & document;
};

#endif
