#include "stdafx.h"
#include "MusicPhraseSegment.h"
#include "Sa_Doc.h"

CMusicPhraseSegment::CMusicPhraseSegment(EAnnotation index, int master) :
    CIndependentSegment(index,master) {
}

/***************************************************************************/
// CMusicPhraseSegment::CheckPosition Check positions for annotation window
// Checks the positions for validation. If they are ok for a new annotation
// segment it returns the index, where to put it in the annotation array
// (0 based), otherwise -1. The function allows an overlap of 50% of
// existing annotation segments at both ends.
/***************************************************************************/
int CMusicPhraseSegment::CheckPosition(ISaDoc * pDoc, DWORD dwStart, DWORD dwStop, EMode nMode, BOOL bOverlap) const {
    int nLength = GetOffsetSize();
    if (nLength == 0) {
		// no character yet, ok
        return 0;
    }

	// segment selected (edit)
    if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(m_nSelection != -1)) { 
        return CIndependentSegment::CheckPosition(pDoc, dwStart, dwStop, nMode, bOverlap);
    } else if ((nMode==MODE_ADD)||(nMode==MODE_AUTOMATIC)) {
        // if segment is less than 20 ms, we can't add
        DWORD dwSize = dwStop-dwStart;
        if ((dwSize)  < pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) {
			// segment too small
            return -1;
        }

        int nLoop;
        for (nLoop = 0; nLoop < nLength; nLoop++) {
            DWORD dwOffset = GetOffset(nLoop);
            // are we before this segment?
            if (dwStart <= dwOffset) {
				// this offset
                // did our stop overlap it's start?
                if (dwStop > dwOffset) {
                    return -1;
                }
                // is our start less than the previous stop?
                if ((nLoop > 0)&&(dwStart < GetStop(nLoop - 1))) {
                    return -1;
                }
				// ok
                return nLoop;
            }
        }
        // we didn't find anything - is our start overlapping the last stop?
        if ((nLoop>0)&&(dwStart < GetStop(nLoop-1))) {
            return -1;
        }
		// append at the end
        return nLoop;
    } else {
        return -1;
    }
}

CFontTable * CMusicPhraseSegment::NewFontTable() const {
    return new CFontTableANSI;
}
