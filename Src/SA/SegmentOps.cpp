#include "StdAfx.h"
#include "SegmentOps.h"
#include "Sa_Doc.h"
#include "Segment.h"

CSegmentOps::CSegmentOps( CSaDoc & aDocument) : document(aDocument) {
}

CSegmentOps::~CSegmentOps(void) {
}

/***************************************************************************
* CSaDoc::ShrinkSegments Shrink segments to new file size
* The function shrinks the segments to the new file size. 
* The file size changed at the position start by length bytes and it shrunk. 
* The segments in the deleted section will be deleted. 
* The ones that overlap into the section will be adjusted if valid
***************************************************************************/
void CSegmentOps::ShrinkSegments( WAVETIME start, WAVETIME length) {

    DWORD sectionStart = document.ToBytes(start, true);
    DWORD sectionLength = document.ToBytes(length, true);

	for (int nSeg=0;nSeg<ANNOT_WND_NUMBER;nSeg++) {
		document.GetSegment(nSeg)->ShrinkSegment(document,sectionStart,sectionLength);
	}

	document.DeselectAll();
}

/***************************************************************************
* CSaDoc::GrowSegments Grow segments to new file size
* The function adjusts the segments to the new file size. 
* The file size changed at the position start by length bytes and it grew.
* All the segments offsets after start change. 
* The ones that overlap into the section will be adjusted if valid
***************************************************************************/
void CSegmentOps::GrowSegments(WAVETIME start, WAVETIME length) {

	DWORD sectionStart = document.ToBytes(start, true);
    DWORD sectionLength = document.ToBytes(length, true);

	for (int nSeg=0;nSeg<ANNOT_WND_NUMBER;nSeg++) {
		document.GetSegment(nSeg)->GrowSegment(document,sectionStart,sectionLength);
	}

	document.DeselectAll();
}

