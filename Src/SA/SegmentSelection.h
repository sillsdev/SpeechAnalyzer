#ifndef SEGMENTSELECTION_H
#define SEGMENTSELECTION_H
/////////////////////////////////////////////////////////////////////////////
// SegmentSelection.h
// Interface of the CSegmentSelection class
// Author: Steve MacLean (SDM)
// copyright 1999 JAARS Inc. SIL
//
// Revision History
//   1.06.5
//       SDM Original Version
//   1.06.6
//       SDM Added BOOL return to SelectFromPosition to note selection change
//   1.5Test11.4
//        SDM added SelectFromStopPosition virtual selections to handle phonemic/ortho/tone spanning multiple segments
/////////////////////////////////////////////////////////////////////////////

class CSaView;
class CSaString;

/***************************************************************************/
// CSegmentSelection
// special class to allow selection of empty dependent segments.  Should also
// provide a interface independent of data storage representation
/***************************************************************************/
class CSegmentSelection
{
public:
    CSegmentSelection();
    BOOL CSegmentSelection::SelectFromPosition(CSaView * pView, int nSegmentIndex, DWORD dwPosition, int nMode = NULL);
    BOOL CSegmentSelection::SelectFromStopPosition(CSaView * pView, int nSegmentIndex, DWORD dwPosition, int nMode = NULL);
    void Update(CSaView * pView, BOOL bClearVirtual = FALSE); //Update Selection Record
    CString GetSelectedAnnotationString(CSaView * pView, BOOL bRemoveDelimiter = TRUE) const;
    BOOL SetSelectedAnnotationString(CSaView * pView, CSaString & szString, BOOL bIncludesDelimiter = FALSE, BOOL bCheck=FALSE);
	DWORD GetSelectionStart();
	DWORD GetSelectionStop();
	int GetSelectionIndex();
	bool IsSelectionVirtual();

    enum
    {
        FIND_EXACT = 1
    };

protected:
    DWORD m_dwStart;
    DWORD m_dwStop;
    DWORD m_dwDuration;
    int m_nAnnotationIndex;
    bool m_bVirtual;

private:
    void RefreshAnnotation(CSaView * pView, int nAnnotationIndex);
};

#endif