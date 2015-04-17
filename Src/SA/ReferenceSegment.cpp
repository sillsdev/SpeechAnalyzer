#include "Stdafx.h"
#include "ReferenceSegment.h"
#include "Sa_Doc.h"
#include "SA_View.h"

CReferenceSegment::CReferenceSegment(EAnnotation index, int master) : CTextSegment( index, master) {
}

void CReferenceSegment::LimitPosition(CSaDoc *, DWORD & dwStart, DWORD & dwStop, ELimit /*nMode*/) const {
    dwStart=GetOffset(GetSelection());
    dwStop=GetStop(GetSelection());
}

/***************************************************************************/
// Append append a text segment
// Returns FALSE if an error occurred.
// if pszString is NULL, no string is added, and an error is returned
//
// When inserting beyond the end of the current list, nIndex is the next
// available index.
// For example if we are inserting at the 4 gloss segment but only reference
// segments 0 and 1 exist, then the next index is 2, not 3.
/***************************************************************************/
BOOL CReferenceSegment::Append(LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration) {
    return Insert(GetOffsetSize(), pszString, delimiter, dwStart, dwDuration);
}

/***************************************************************************/
// Insert Insert a text segment
// Returns FALSE if an error occurred.
// if pszString is NULL, no string is added, and an error is returned
//
// When inserting beyond the end of the current list, nIndex is the next
// available index.
// For example if we are inserting at the 4 gloss segment but only reference
// segments 0 and 1 exist, then the next index is 2, not 3.
/***************************************************************************/
BOOL CReferenceSegment::Insert(int nIndex, LPCTSTR pszString, bool /*delimiter*/, DWORD dwStart, DWORD dwDuration) {
    if (pszString==NULL) {
        return FALSE;
    }
    InsertAt(nIndex,pszString,dwStart,dwDuration);
    return TRUE;
}

// SDM 1.5 Test11.0
/***************************************************************************/
// CDependentSegment::CheckPositionToSegment Check positions for dep. segments
// Checks the positions for validation. If they are ok for a new segment
// (depending on master segment positions), it returns the
// index, where to put them in in the arrays (0 based), otherwise -1. The
// start must not be placed in the range of a segment, where already
// another segment is aligned to, but there must be a segment to align to.
/***************************************************************************/
int CReferenceSegment::CheckPositionToMaster(ISaDoc * pSaDoc, DWORD dwAlignedStart, DWORD dwStop, EMode nMode) const {

    int nTextIndex = GetSelection();
    if (((nMode==MODE_EDIT)||(nMode==MODE_AUTOMATIC))&&(nTextIndex != -1)) { // segment selected (edit)
        return -1;
    } else { // Add
        return CDependentSegment::CheckPositionToMaster(pSaDoc, dwAlignedStart, dwStop, nMode);
    }
}

/***************************************************************************/
// Add Add dependent annotation segment
/***************************************************************************/
void CReferenceSegment::Add(CSaDoc * pDoc, CSaView * pView, DWORD dwStart, CSaString & szString, bool /*delimiter*/, bool bCheck) {

    // get the offset and duration from master
    int nSegment = pDoc->GetSegment(GLOSS)->FindOffset(dwStart);
    if (nSegment == -1) {
        return; // return on error
    }

    DWORD dwDuration = pDoc->GetSegment(GLOSS)->GetDuration(nSegment);
    DWORD dwStop = dwStart + dwDuration;

    int nPos = CheckPosition(pDoc, dwStart, dwStop, MODE_ADD); // get the insert position
    if (nPos == -1) {
        return; // return on error
    }

    // save state for undo ability
    if (bCheck) {
        pDoc->CheckPoint();
    }

    // insert or append the new dependent segment
    if (!Insert(nPos, szString, 0, dwStart, dwDuration)) {
		// return on error or not inserted.
        return;
    }

	// document has been modified
    pDoc->SetModifiedFlag(TRUE);
	// transcription data has been modified
    pDoc->SetTransModifiedFlag(TRUE);
	// change the selection
    pView->ChangeAnnotationSelection(this, nPos, dwStart, dwStop);
}

CFontTable * CReferenceSegment::NewFontTable() const {
    return new CFontTableANSI();
}
