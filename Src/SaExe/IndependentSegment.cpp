#include "stdafx.h"
#include "IndependentSegment.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CIndependentSegment
// class to do all the handling for the master annotation segments.

CIndependentSegment::CIndependentSegment(int index, int master) :
    CSegment(index,master)
{
}

/***************************************************************************/
// CIndependentSegment::LimitsPosition Limits positions for annotation window
// Adjusts the position to acceptable position to adjust boundaries
/***************************************************************************/
void CIndependentSegment::LimitPosition(CSaDoc * pSaDoc, DWORD & dwStart,DWORD & dwStop, int nMode) const
{
    if ((m_nSelection != -1)&&!(nMode&LIMIT_NO_OVERLAP))
    {
        // segment selected (edit)
        if (m_nSelection > 0)
        {
            DWORD dwStartMinus = pSaDoc->SnapCursor(START_CURSOR, dwStart,0,dwStart,SNAP_LEFT);
            DWORD dwStopMinus = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStart,dwStop,SNAP_LEFT);

            // check if not more than 40% overlap of start with previous character
            if (dwStartMinus < (GetOffset(m_nSelection - 1) + GetDuration(m_nSelection - 1) *3/5))
            {
                dwStart = GetOffset(m_nSelection - 1) + GetDuration(m_nSelection - 1) *3/5;
                dwStart = pSaDoc->SnapCursor(START_CURSOR, dwStart,dwStart,dwStop-2,SNAP_RIGHT);
                dwStartMinus = dwStart; // snap will not move
            }
            // check if more than 40% overlap of previous stop with current character
            if ((dwStartMinus*3 + dwStopMinus*2)/5 <= GetStop(m_nSelection - 1))
            {
                if (nMode==LIMIT_MOVING_STOP)
                {
                    dwStop = (GetStop(m_nSelection - 1)*5 - dwStart*3)/2 + 2;
                    dwStop = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStop,pSaDoc->GetUnprocessedDataSize(),SNAP_RIGHT);
                }
                else
                {
                    dwStart = (GetStop(m_nSelection - 1)*5 - dwStop*2)/3 + 2;
                    dwStart = pSaDoc->SnapCursor(START_CURSOR, dwStart,dwStart,dwStop-2,SNAP_RIGHT);
                }
            }
        }
        // find offset of next character
        int nNext = GetNext(m_nSelection);
        if (nNext != -1)
        {
            DWORD dwStartPlus = pSaDoc->SnapCursor(START_CURSOR, dwStart,dwStart,pSaDoc->GetUnprocessedDataSize(),SNAP_RIGHT);
            DWORD dwStopPlus = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStop,pSaDoc->GetUnprocessedDataSize(),SNAP_RIGHT);

            // check if not more than 40% overlap with stop
            if (dwStopPlus > (GetOffset(nNext) + GetDuration(nNext) *2/5))
            {
                dwStop = GetOffset(nNext) + GetDuration(nNext) *2/5;
                dwStop = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStart,dwStop,SNAP_LEFT);
                dwStopPlus = dwStop; // snap will not move
            }
            // check if next character overlaps more than 40% of current character
            if ((dwStartPlus*2 + dwStopPlus*3)/5 >= GetOffset(nNext))
            {
                if (nMode==LIMIT_MOVING_START)
                {
                    dwStart = (GetOffset(nNext)*5 - dwStop*3)/2 - 2;
                    dwStart = pSaDoc->SnapCursor(START_CURSOR, dwStart,0,dwStart,SNAP_LEFT);
                }
                else
                {
                    dwStop = (GetOffset(nNext)*5 - dwStart*2)/3 - 2;
                    dwStop = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStart+2,dwStop,SNAP_LEFT);
                }
            }
        }
    }
    else if ((m_nSelection != -1)&&(nMode&LIMIT_NO_OVERLAP))
    {
        // SDM 1.5Test8.1
        DWORD dwStartMin=0;
        DWORD dwStopMax= pSaDoc->GetUnprocessedDataSize();

        if (m_nSelection > 0)
        {
            int nPrevious = GetPrevious(m_nSelection);
            dwStartMin = GetOffset(nPrevious) + pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
            if (nPrevious > 0)
            {
                if (GetOffset(nPrevious) < GetStop(nPrevious-1))
                {
                    // overlap
                    DWORD dwOverlap = GetStop(nPrevious-1) - GetOffset(nPrevious);
                    if (dwStartMin < (GetOffset(nPrevious) + dwOverlap*5/2 + 2))
                    {
                        dwStartMin = (GetOffset(nPrevious) + dwOverlap*5/2 + 2);
                    }
                }
            }
            if ((pSaDoc->Is16Bit())&&(dwStartMin & 1))
            {
                dwStartMin++;
            }
        }
        // find offset of next character
        int nNext = GetNext(m_nSelection);
        if (nNext != -1)
        {
            dwStopMax = GetStop(nNext);
            if (dwStopMax > pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME))
            {
                dwStopMax -= pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
            }
            else
            {
                dwStopMax = 0;
            }

            if (GetNext(nNext) > 0)
            {
                if (GetStop(nNext) > GetOffset(GetNext(nNext)))
                {
                    // overlap
                    DWORD dwOverlap = GetStop(nNext) - GetOffset(GetNext(nNext));
                    DWORD dwStopNext = GetStop(nNext);
                    if (dwStopNext > (dwOverlap*5/2 + 2))
                    {
                        dwStopNext -= (dwOverlap*5/2 + 2);
                    }
                    else
                    {
                        dwStopNext = 0;
                    }
                    if (dwStopMax > dwStopNext)
                    {
                        dwStopMax = dwStopNext;
                    }
                }
            }
            if ((pSaDoc->Is16Bit())&&(dwStopMax & 1))
            {
                dwStopMax --;
            }
        }
        dwStartMin = pSaDoc->SnapCursor(START_CURSOR, dwStartMin,dwStartMin,dwStopMax,SNAP_RIGHT);
        dwStopMax = pSaDoc->SnapCursor(STOP_CURSOR, dwStopMax,dwStartMin,dwStopMax,SNAP_LEFT);
        if ((dwStartMin + pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME))> dwStopMax)
        {
            return;    // no hope ERROR
        }

        if (dwStart < dwStartMin)
        {
            dwStart = dwStartMin;
        }
        if (dwStop > dwStopMax)
        {
            dwStop = dwStopMax;
        }
        if (dwStop < (dwStart + pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME)))
        {
            if (nMode & LIMIT_MOVING_STOP)
            {
                dwStop = dwStart + pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
                if ((pSaDoc->Is16Bit())&&(dwStop & 1))
                {
                    dwStop++;
                }
                dwStop = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStop,dwStopMax+2,SNAP_RIGHT);
                if (dwStop > dwStopMax)
                {
                    dwStop = dwStopMax;
                    dwStart = dwStop - pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
                    if ((pSaDoc->Is16Bit())&&(dwStart & 1))
                    {
                        dwStart--;
                    }
                    dwStart = pSaDoc->SnapCursor(START_CURSOR, dwStart,dwStartMin,dwStart,SNAP_LEFT);
                }
            }
            else
            {
                dwStart = dwStop - pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
                if ((pSaDoc->Is16Bit())&&(dwStart & 1))
                {
                    dwStart--;
                }
                dwStart = pSaDoc->SnapCursor(START_CURSOR, dwStart,0,dwStart,SNAP_LEFT);
                if (dwStart < dwStartMin)
                {
                    dwStart = dwStartMin;
                    dwStop = dwStart + pSaDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME);
                    if ((pSaDoc->Is16Bit())&&(dwStop & 1))
                    {
                        dwStop++;
                    }
                    dwStop = pSaDoc->SnapCursor(STOP_CURSOR, dwStop,dwStop,dwStopMax,SNAP_RIGHT);
                }
            }
        }
    }
}

// SDM Split function 1.06.1.2
// SDM 1.06.6U2 require not more than 40% overlap, require MIN_EDIT_SEGMENT_TIME on edit
/***************************************************************************/
// CIndependentSegment::CheckPosition Check positions for annotation window
// Checks the positions for validation. If they are ok for a new annotation
// segment it returns the index, where to put it in the annotation array
// (0 based), otherwise -1. The function allows an overlap of 50% of
// existing annotation segments at both ends.
/***************************************************************************/
int CIndependentSegment::CheckPosition(CSaDoc * pDoc, DWORD dwStart,DWORD dwStop, EMode nMode, BOOL bOverlap) const
{
    if (IsEmpty())
    {
        return 0; // no character yet, ok
    }

    if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(m_nSelection != -1))   // segment selected (edit)
    {
        if (bOverlap)   //SDM 1.5Test8.1
        {
            if (m_nSelection > 0)
            {
                // check if not more than 40% overlap of start with previous character
                if (dwStart < (GetOffset(m_nSelection - 1) + GetDuration(m_nSelection - 1) *3/5))
                {
                    return -1;    // error
                }
                // check if more than 40% overlap of previous stop with current character
                if ((dwStart+(dwStop-dwStart)*2/5) <= GetStop(m_nSelection - 1))
                {
                    return -1;    // error
                }
            }
            // find offset of next character
            int nNext = GetNext(m_nSelection);
            if (nNext != -1)
            {
                // check if not more than 40% overlap with stop
                if (dwStop > (GetOffset(nNext) + GetDuration(nNext) *2/5))
                {
                    return -1;    // error
                }
                // check if next character overlaps more than 40% of current character
                if ((dwStart+(dwStop-dwStart)*3/5) >= GetOffset(nNext))
                {
                    return -1;    // error
                }
            }
            if (dwStop < dwStart)
            {
                return -1;
            }
            if (pDoc->GetTimeFromBytes(dwStop-dwStart) < MIN_EDIT_SEGMENT_TIME)
            {
                return -1;
            }
            return m_nSelection;
        }
        else     // SDM 1.5Test8.1
        {
            int nPrevious = GetPrevious(m_nSelection);
            if (nPrevious > 0)
            {
                if (GetOffset(nPrevious) < GetStop(nPrevious-1))   // overlap
                {
                    DWORD dwOverlap = GetStop(nPrevious-1) - GetOffset(nPrevious);
                    if (dwStart < (GetOffset(nPrevious) + dwOverlap*5/2 + 2))
                    {
                        return -1;
                    }
                }
            }
            // find offset of next character
            int nNext = GetNext(m_nSelection);
            if (GetNext(nNext) != -1)
            {
                if (GetStop(nNext) > GetOffset(GetNext(nNext)))   // overlap
                {
                    DWORD dwOverlap = GetStop(nNext) - GetOffset(GetNext(nNext));
                    DWORD dwStopNext = GetStop(nNext);
                    if (dwStopNext > (dwOverlap*5/2 + 2))
                    {
                        dwStopNext -= (dwOverlap*5/2 + 2);
                    }
                    else
                    {
                        dwStopNext = 0;
                    }
                    if (dwStop > dwStopNext)
                    {
                        return -1;
                    }
                }
            }
            if (dwStop < dwStart)
            {
                return -1;
            }
            if (pDoc->GetTimeFromBytes(dwStop-dwStart) < MIN_EDIT_SEGMENT_TIME)
            {
                return -1;
            }
            return m_nSelection;
        }
    }
    else if ((nMode==MODE_ADD)||(nMode==MODE_AUTOMATIC))
    {
        //SDM 1.06.5 add will make room by shifting adjacent segments
        // check the positions
        if ((dwStop - dwStart)  < pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME))
        {
            return -1;    // segment too small
        }
        int nLength = GetOffsetSize();
        int nLoop;
        for (nLoop = 0; nLoop < nLength; nLoop++)
        {
            DWORD dwOffset = GetOffset(nLoop);
            if (dwStart <= dwOffset)   // this offset
            {
                // Check if insertion will leave at least minimum segment length
                if ((dwStop+pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) > (dwOffset + GetDuration(nLoop)))
                {
                    return -1;
                }
                if ((nLoop > 0)&&
                        ((dwStart-pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)) < GetOffset(nLoop - 1)))
                {
                    return -1;
                }
                int nNext = GetNext(nLoop);
                if (nNext != -1)
                {
                    // check if next has more than 40% overlap
                    if ((dwStop+(dwOffset+GetDuration(nLoop)-dwStop)*3/5)  > GetOffset(nNext))
                    {
                        return -1;    // error
                    }
                }
                int nPrevious = GetPrevious(nLoop);
                if (nPrevious > 0)
                {
                    // check if segment before previous will overlap 40%
                    if ((GetOffset(nPrevious) + (dwStart-GetOffset(nPrevious))*2/5) < GetStop(nPrevious - 1))
                    {
                        return -1;    // error
                    }
                }
                return nLoop; // ok
            }
        }
        if ((nLoop >0)&&((dwStart - GetOffset(nLoop-1)) < pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME)))
        {
            return -1;
        }
        int nPrevious = GetPrevious(nLoop);
        if (nPrevious > 0)
        {
            // check if segment before previous will overlap 40%
            if ((dwStart + GetOffset(nPrevious))/2 < GetStop(nPrevious - 1))
            {
                return -1;    // error
            }
        }
        return nLoop; // append at the end
    }
    return -1;
}
