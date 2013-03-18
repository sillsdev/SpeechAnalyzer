#include "stdafx.h"
#include "TextSegment.h"
#include "Process\Process.h"
#include "Segment.h"
#include "Process\sa_p_cha.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_fra.h"

#include "sa_asert.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "ArchiveTransfer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CTextSegment
// class to do all the handling with text annotation segments. This class is
// the base class for segments, that contain independent character strings
// (text) for each segment. They still depend on master annotation and must
// be aligned with it (base class is CDependentSegment class).

/////////////////////////////////////////////////////////////////////////////
// CTextSegment construction/destruction/creation
CSaString CTextSegment::GetText(int nIndex) {
    return m_pTexts->GetAt(nIndex);
}

const CStringArray * CTextSegment::GetTexts() {
    return m_pTexts;
}

int CTextSegment::GetSegmentLength(int /*nIndex*/) const {
    return 1;
}

CSaString CTextSegment::GetSegmentString(int nIndex) const {
    return m_pTexts->GetAt(nIndex);
}

/***************************************************************************/
// CTextSegment::CTextSegment Constructor
/***************************************************************************/
CTextSegment::CTextSegment(int index, int master) : CDependentSegment(index,master) {
    m_pTexts = new CStringArray; // create string array object
}

/***************************************************************************/
// CTextSegment::~CTextSegment Destructor
/***************************************************************************/
CTextSegment::~CTextSegment() {
    if (m_pTexts) {
        delete m_pTexts;
        m_pTexts = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CTextSegment helper functions

/***************************************************************************/
// CTextSegment::Insert Insert/append a text segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CTextSegment::SetAt(const CSaString * pszString, bool delimiter, DWORD dwStart, DWORD dwDuration) {
    // prepare delimiter
    CSaString szDelimiter = WORD_DELIMITER;
    if (delimiter) {
        szDelimiter.SetAt(0, TEXT_DELIMITER);
    }

    try {
        int nIndex = FindOffset(dwStart);
        ASSERT(nIndex>=0);
        if (pszString!=NULL) {
            m_pTexts->SetAtGrow(nIndex, szDelimiter + *pszString);
        } else {
            m_pTexts->SetAtGrow(nIndex, szDelimiter);
        }
        CSegment::SetAt(nIndex, dwStart, dwDuration);
    } catch (CMemoryException e) {
        // memory allocation error
        ErrorMessage(IDS_ERROR_MEMALLOC);
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************/
// CTextSegment::Insert Insert/append a text segment
// Returns FALSE if an error occured. If the pointer to the string is NULL
// there will be no string added.
/***************************************************************************/
BOOL CTextSegment::Insert(int nIndex, LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration) {

    // prepare delimiter
    CSaString szDelimiter = WORD_DELIMITER;
    if (delimiter) {
        szDelimiter.SetAt(0, TEXT_DELIMITER);
    }

    try {
        if (pszString!=NULL) {
            m_pTexts->InsertAt(nIndex, szDelimiter + pszString, 1);
        } else {
            m_pTexts->InsertAt(nIndex, szDelimiter, 1);
        }
        InsertAt(nIndex, dwStart, dwDuration);
    } catch (CMemoryException e) {
        // memory allocation error
        ErrorMessage(IDS_ERROR_MEMALLOC);
        return FALSE;
    }
    return TRUE;
}

/***************************************************************************/
// CTextSegment::CaluculateDuration calculate segment duration from master data
/***************************************************************************/
DWORD CTextSegment::CalculateDuration(CSaDoc * pDoc, const int nIndex) const {

    CSegment * pMaster = (CSegment *) pDoc->GetSegment(m_nMasterIndex);

    if ((nIndex < 0) || (nIndex >= (GetOffsetSize()))) {
        return DWORD(-1);
    }
    if ((nIndex + 1) == GetOffsetSize()) {
        return pMaster->GetStop(pMaster->GetOffsetSize()-1) - GetOffset(nIndex);
    } else {
        DWORD dwStop = pMaster->GetStop(pMaster->FindOffset(GetOffset(GetNext(nIndex)))-1);
        if (dwStop == -1) {
            return DWORD(-1);
        }
        return  dwStop - GetOffset(nIndex);
    }
}

/***************************************************************************/
// CTextSegment::DeleteContents Delete all contents of the segment arrays
/***************************************************************************/
void CTextSegment::DeleteContents() {
    m_pTexts->RemoveAll();
    CSegment::DeleteContents(); // call the base class to delete positions
}

// SDM 1.5 Test11.0 Modified to support new gloss stop position
/***************************************************************************/
// CTextSegment::CheckPositions Check positions for text segments
// Checks the positions for validation. If they are ok for a new segment
// (depending on master segment positions), it returns the
// index, where to put them in in the arrays (0 based), otherwise -1. The
// start cursor must not be placed in the range of a segment, where already
// another segment is aligned to, but there must be a segment to align to.
/***************************************************************************/
void CTextSegment::LimitPosition(CSaDoc * pSaDoc, DWORD & dwStart, DWORD & dwStop, int) const {

    // get pointer to view
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    CSegment * pMaster = pDoc->GetSegment(m_nMasterIndex);

    int nTextIndex = GetSelection();

    if (nTextIndex != -1) {
        int nIndex;

        // Check for obscuring previous segment
        nIndex = GetPrevious(nTextIndex);
        if (nIndex != -1) {
            nIndex = pMaster->FindOffset(GetOffset(nIndex));
            if (nIndex !=-1) { // Properly Aligned to master
                nIndex = pMaster->GetNext(nIndex);
                if ((nIndex !=-1) && (dwStart < pMaster->GetOffset(nIndex))) {
                    dwStart = pMaster->GetOffset(nIndex);
                }
                if ((nIndex !=-1) && (dwStop < pMaster->GetStop(nIndex))) {
                    dwStop = pMaster->GetStop(nIndex);
                }
            }
        }
        // Check for obscuring next segment
        nIndex = GetNext(nTextIndex);
        if (nIndex != -1) {
            nIndex = pMaster->FindStop(GetStop(nIndex));
            if (nIndex !=-1) {
                nIndex = pMaster->GetPrevious(nIndex);
                if ((nIndex !=-1) && (dwStop > pMaster->GetStop(nIndex))) {
                    dwStop = pMaster->GetStop(nIndex);
                }
                if ((nIndex !=-1) && (dwStart > pMaster->GetOffset(nIndex))) {
                    dwStart = pMaster->GetOffset(nIndex);
                }
            } else { // no master segment at stop boundary
                nIndex = GetNext(nTextIndex); // 1.5Test8.1
                DWORD dwOffset = GetStop(nIndex);
                nIndex = AlignStopToMaster(pSaDoc, &dwOffset); // find the nearest master segment to the end of next text segment
                if ((nIndex !=-1) && (dwStop > pMaster->GetStop(nIndex))) {
                    dwStop = pMaster->GetStop(nIndex);
                }
                if ((nIndex !=-1) && (dwStart > pMaster->GetOffset(nIndex))) {
                    dwStart = pMaster->GetOffset(nIndex);
                }
            }
        } else { // last gloss
            dwStop = pMaster->GetStop(pMaster->GetOffsetSize() - 1);
        }

        AdjustPositionToMaster(pDoc, dwStart, dwStop);
    }
}

// SDM Added function 1.06.1.2
/***************************************************************************/
// CTextSegment::CheckPositions Check positions for text segments
// Checks the positions for validation. If they are ok for a new segment
// (depending on master segment positions), it returns the
// index, where to put them in in the arrays (0 based), otherwise -1. The
// start cursor must not be placed in the range of a segment, where already
// another segment is aligned to, but there must be a segment to align to.
/***************************************************************************/
int CTextSegment::CheckPosition(CSaDoc * pSaDoc, DWORD dwStart, DWORD dwStop, EMode nMode, BOOL) const {

    // get pointer to view
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer

    // get the actual aligned position
    DWORD dwAlignedStart = dwStart;
    DWORD dwAlignedStop = dwStop;

    AdjustPositionToMaster(pDoc, dwAlignedStart, dwAlignedStop);

    int nTextIndex = GetSelection();

    if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(nTextIndex != -1)) { // segment selected (edit)
        int nIndex;

        if (dwAlignedStart >= dwAlignedStop) {
            return -1;    // zero duration (or negative)
        }
        if (dwAlignedStart == GetOffset(nTextIndex) && (dwAlignedStop == GetStop(nTextIndex))) {
            return -1;    // no change
        }

        // Check for obscuring previous segment
        nIndex = GetPrevious(nTextIndex);
        if (nIndex != -1) {
            if (GetOffset(nIndex) >= dwAlignedStart) {
                return -1;
            }
        }
        // Check for obscuring present segment
        nIndex = GetNext(nTextIndex);
        if (nIndex != -1) {
            if (GetStop(nIndex) <= dwAlignedStop) {
                return -1;
            }
        }
        return nTextIndex;
    } else { // Add
        if (pDoc->GetSegment(m_nMasterIndex)->IsEmpty()) {
            return -1;
        }
        if (IsEmpty()) {
            return 0;    // No Text in Segment insert at 0 (Start)
        }
        nTextIndex = 0; // GetNext does not work for -1
        while ((nTextIndex != -1) && (GetOffset(nTextIndex) < dwAlignedStart)) {
            nTextIndex = GetNext(nTextIndex);
        }

        if (nTextIndex == -1) {
            return GetOffsetSize();    // Insert at End
        }

        if (GetOffset(nTextIndex) == dwAlignedStart) {
            return -1;
        }

        return nTextIndex;
    }
    return -1;
}

// SDM 1.06.5
/***************************************************************************/
// CTextSegment::Add Add text segment
/***************************************************************************/
void CTextSegment::Add(CSaDoc * pDoc, DWORD dwStart, CSaString & szString, bool bDelimiter, bool bCheck) {

    // add a segment
    // get pointer to view
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    int nPos = FindFromPosition(dwStart,TRUE);
    if (nPos==-1) {
        nPos = FindFromPosition(dwStart,FALSE);
    } else {
        nPos++;
    }

    CSegment * pMaster = pDoc->GetSegment(m_nMasterIndex);
    int nMaster;

    if ((nPos == -1) || (nPos >= GetOffsetSize()) || dwStart > GetStop(nPos)) {
        nMaster = pMaster->GetOffsetSize();
    } else {
        nMaster = pMaster->FindOffset(GetOffset(nPos));
    }

    ASSERT(nMaster > 0); // must be master segment to attach to

    DWORD dwStop = pMaster->GetStop(nMaster-1);

    ASSERT(dwStop > dwStart);

    nPos = CheckPosition(pDoc, dwStart, dwStop, CSegment::MODE_ADD);  // get the insert position
    if (nPos == -1) {
        return;    // return on error
    }

    ASSERT(dwStop > dwStart);

    // save state for undo ability
    if (bCheck) {
        pDoc->CheckPoint();
    }

    // insert or append the new segment
    if (!Insert(nPos, szString, bDelimiter, dwStart, dwStop - dwStart)) {
        return;    // return on error
    }

    // move the end of the previous text segment
    if (nPos > 0) {
        Adjust(pDoc, nPos - 1, GetOffset(nPos - 1), CalculateDuration(pDoc, nPos -1));
    }
    pDoc->SetModifiedFlag(TRUE); // document has been modified
    pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
    pView->ChangeAnnotationSelection(this, nPos); // change the selection
    pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}


// SDM 1.5Test11.3
/***************************************************************************/
// CTextSegment::Remove Remove text segment
/***************************************************************************/
void CTextSegment::Remove(CDocument * pSaDoc, BOOL bCheck) {

    // get pointer to view
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer

    // save state for undo ability
    if (bCheck) {
        pDoc->CheckPoint();
    }

    // move the end of the previous text segment
    if (m_nSelection > 0) {
        SetDurationAt(m_nSelection - 1, GetStop(m_nSelection) - GetOffset(m_nSelection - 1));
    }
    CDependentSegment::Remove(pSaDoc, FALSE);
}

/***************************************************************************/
// CTextSegment::RemoveNoRefresh Remove text segment
/***************************************************************************/
DWORD CTextSegment::RemoveNoRefresh(CDocument *) {

    //SDM 1.5Test8.1 return oldOffset
    DWORD dwOffset = GetOffset(m_nSelection);

    // change the segment arrays
    m_pTexts->RemoveAt(m_nSelection, 1);
    RemoveAt(m_nSelection,1);

    return dwOffset;
}

// SDM 1.5Test8.3
void CTextSegment::ReplaceSelectedSegment(CDocument * pSaDoc, const CSaString & str1) {

    // get pointer to view
    CSaDoc * pDoc = (CSaDoc *)pSaDoc; // cast pointer
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    if (m_nSelection != -1) {
        m_pTexts->SetAt(m_nSelection, str1);
    }

    pDoc->SetModifiedFlag(TRUE); // document has been modified
    pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified

    int nSaveSelection = m_nSelection; // 1.5Test10.2
    pView->ChangeAnnotationSelection(this, m_nSelection); // deselect // 1.5Test10.2
    pView->ChangeAnnotationSelection(this, nSaveSelection); // select again // 1.5Test10.2
    pView->RefreshGraphs(FALSE); // refresh the graphs between cursors
}

/***************************************************************************/
// CSegment::FindNext find next segment matching strToFind and hilite it.
//***************************************************************************/
int CTextSegment::FindNext(int fromIndex, const CSaString & strToFind, CSaDoc &) {

    ASSERT(fromIndex >= -1);
    ASSERT(!IsEmpty());

    int index    = fromIndex + 1;

    for (; index<=m_pTexts->GetUpperBound(); index++) {
        if (m_pTexts->GetAt(index) == strToFind) {
            break;
        }
    }

    return ((index<=m_pTexts->GetUpperBound()) ? index : -1);
}

/***************************************************************************/
// CSegment::Match find next segment matching strToFind and hilite it.
//***************************************************************************/
BOOL CTextSegment::Match(int index, const CSaString & strToFind) {

    ASSERT(index >= -1);
    ASSERT(!IsEmpty());
    BOOL ret = FALSE;

    if (index >= 0 && index <= m_pTexts->GetUpperBound()) {
        ret = (m_pTexts->GetAt(index) == strToFind);
    }

    return ret;
}

/***************************************************************************/
// CTextSegment::FindPrev find next segment matching strToFind and hilite it.
/***************************************************************************/
int CTextSegment::FindPrev(int fromIndex, const CSaString & strToFind, CSaDoc &) {

    ASSERT(fromIndex >= -1);
    ASSERT(!IsEmpty());
    int index = -1;

    if (fromIndex > 0 || fromIndex == -1) {
        index    = (fromIndex == -1) ? m_pTexts->GetUpperBound() : fromIndex - 1;

        for (; index>=0; index--) {
            if (m_pTexts->GetAt(index) == strToFind) {
                break;
            }
        }
    }

    return index;
}


/***************************************************************************/
// CTextSegment::CountWords
//***************************************************************************/i
int CTextSegment::CountWords() {

    int nIndex = 0;
    int nWords = 0;
    if (IsEmpty()) {
        return nWords;
    }
    while (nIndex!=-1) {
        if (GetText(nIndex)[0]==WORD_DELIMITER) {
            nWords++;
        }
        nIndex=GetNext(nIndex);
    }
    return nWords;

}

/***************************************************************************/
// CTextSegment::Serialize
//***************************************************************************/i
void CTextSegment::Serialize(CArchive & ar) {
    CSegment::Serialize(ar);
    if (ar.IsStoring()) {
        ar << CSaString("CTextSegmentDetail tag");
    } else {
        CSaString detailTagCheck;
        ar >> detailTagCheck;
        SA_ASSERT(detailTagCheck == "CTextSegmentDetail tag");
    }
    m_pTexts->Serialize(ar);
}

CSaString CTextSegment::GetContainedText(DWORD dwStart, DWORD dwStop) {

    CSaString szText;
    for (int i=0; i<GetOffsetSize(); i++) {
        DWORD offset = GetOffset(i);
        if ((offset>=dwStart)&&(offset<=dwStop)) {
            szText.Append(m_pTexts->GetAt(i));
        }
    }
    return szText;
}
