#ifndef _SA_ANSEL_H
#define _SA_ANSEL_H
/////////////////////////////////////////////////////////////////////////////
// sa_ansel.h
// Interface of the CASegmentSelection class
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
// CASegmentSelection
// special class to allow selection of empty dependent segments.  Should also
// provide a interface independent of data storage representation
/***************************************************************************/
class CASegmentSelection
{
    //Constructor/Destructor
public:
    CASegmentSelection();

    //Types
public:
    class CASelection
    {
    public:
        DWORD dwStart;
        DWORD dwStop;
        DWORD dwDuration;
        int nAnnotationIndex;
        BOOL bVirtual;
    protected:
        //      int nInsertAtIndex;
        //      int nPhoneticRelativeIndex;
    };
    enum
    {
        FIND_EXACT = 1
    };

    //Attributes
protected:
    CASelection m_Selection;

    // Operations
public:
    BOOL CASegmentSelection::SelectFromPosition(CSaView * pView, int nSegmentIndex, DWORD dwPosition, int nMode = NULL);
    BOOL CASegmentSelection::SelectFromStopPosition(CSaView * pView, int nSegmentIndex, DWORD dwPosition, int nMode = NULL);
    const CASelection & GetSelection() const
    {
        return m_Selection;
    }; //Access Selection Record

    void Update(CSaView * pView, BOOL bClearVirtual = FALSE); //Update Selection Record
    CString GetSelectedAnnotationString(CSaView * pView, BOOL bRemoveDelimiter = TRUE) const;
    BOOL SetSelectedAnnotationString(CSaView * pView, CSaString & szString, BOOL bIncludesDelimiter = FALSE, BOOL bCheck=FALSE);

private:
    void RefreshAnnotation(CSaView * pView, int nAnnotationIndex);
};



#endif //_SA_ANSEL_H