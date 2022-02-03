#include "Stdafx.h"
#include "GlossNatSegment.h"
#include "Sa_Doc.h"
#include "SA_View.h"
#include "sa_ipa.h"

CGlossNatSegment::CGlossNatSegment(EAnnotation index, int master) : CTextSegment(index, master) {
}

void CGlossNatSegment::LimitPosition(CSaDoc *, DWORD & dwStart, DWORD & dwStop, ELimit /*nMode*/) const {
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
BOOL CGlossNatSegment::Append(LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration) {
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
BOOL CGlossNatSegment::Insert(int nIndex, LPCTSTR pszString, bool /*delimiter*/, DWORD dwStart, DWORD dwDuration) {
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
int CGlossNatSegment::CheckPositionToMaster(ISaDoc * pSaDoc, DWORD dwAlignedStart, DWORD dwStop, EMode nMode) const {

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
void CGlossNatSegment::Add( CSaDoc * pModel, CSaView * pView, DWORD dwStart, CSaString & szString, bool /*delimiter*/, bool bCheck) {

    // get the offset and duration from master
    int nSegment = pModel->GetSegment(GLOSS)->FindOffset(dwStart);
    if (nSegment == -1) {
        return; // return on error
    }

    DWORD dwDuration = pModel->GetSegment(GLOSS)->GetDuration(nSegment);
    DWORD dwStop = dwStart + dwDuration;

    int nPos = CheckPosition( pModel, dwStart, dwStop, MODE_ADD); // get the insert position
    if (nPos == -1) {
        return; // return on error
    }

    // save state for undo ability
    if (bCheck) {
        pModel->CheckPoint();
    }

    // insert or append the new dependent segment
    if (!Insert(nPos, szString, 0, dwStart, dwDuration)) {
        pView->RedrawGraphs(TRUE);                                 // refresh the graphs between cursors
        return;                                                     // return on error or not inserted.
    }

    pModel->SetModifiedFlag(TRUE);                                    // document has been modified
    pModel->SetTransModifiedFlag(TRUE);                               // transcription data has been modified
    pView->ChangeAnnotationSelection(this, nPos, dwStart, dwStop);  // change the selection
}

CFontTable * CGlossNatSegment::NewFontTable() const {
    return new CFontTableANSI();
}
