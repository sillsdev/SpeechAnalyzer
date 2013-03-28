#include "stdafx.h"
#include "MusicPhraseSegment.h"
#include "Sa_Doc.h"

CMusicPhraseSegment::CMusicPhraseSegment(int index, int master) :
    CIndependentSegment(index,master)
{
}

/***************************************************************************/
// CMusicPhraseSegment::CheckPosition Check positions for annotation window
// Checks the positions for validation. If they are ok for a new annotation
// segment it returns the index, where to put it in the annotation array
// (0 based), otherwise -1. The function allows an overlap of 50% of
// existing annotation segments at both ends.
/***************************************************************************/
int CMusicPhraseSegment::CheckPosition(CSaDoc * pDoc, DWORD dwStart,DWORD dwStop, EMode nMode, BOOL bOverlap) const
{
    int nLength = GetOffsetSize();
    if (nLength == 0)
    {
        return 0;    // no character yet, ok
    }

    if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(m_nSelection != -1))   // segment selected (edit)
    {
        return CIndependentSegment::CheckPosition(pDoc, dwStart, dwStop, nMode, bOverlap);
    }
    else if ((nMode==MODE_ADD)||(nMode==MODE_AUTOMATIC))
    {
        if ((dwStop - dwStart)  < pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME))
        {
            return -1;    // segment too small
        }
        int nLoop;
        for (nLoop = 0; nLoop < nLength; nLoop++)
        {
            DWORD dwOffset = GetOffset(nLoop);
            if (dwStart <= dwOffset)   // this offset
            {
                if (dwStop > dwOffset)
                {
                    return -1;
                }
                if ((nLoop > 0)&&(dwStart < GetStop(nLoop - 1)))
                {
                    return -1;
                }
                return nLoop; // ok
            }
        }
        if ((nLoop >0)&&(dwStart < GetStop(nLoop-1)))
        {
            return -1;
        }
        return nLoop; // append at the end
    }
    else
    {
        return -1;
    }
}

CFontTable * CMusicPhraseSegment::NewFontTable() const
{
    return new CFontTableANSI;
}
