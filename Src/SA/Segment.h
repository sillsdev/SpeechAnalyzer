/////////////////////////////////////////////////////////////////////////////
// sa_segm.h:
// Interface of the CSegment
//                  CDependentSegment classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
//
// Revision History
// 1.06.1.2
//         SDM Added function
//               void CTextSegment::AdjustCursorsToPhonetic(CDocument* pSaDoc)
//         SDM Added virtual function
//               virtual int CTextSegment::CheckCursors(CDocument*) const;
// 1.06.3b
//         SDM Added virtual CSegment::GetInputFilter()=0 and embodiments
//         declared helper globalfunction BOOL gInputFilter(CSaString&)
// 1.06.4
//         SDM made GetWindowFont public changed return type
//         SDM added include for CFontTable
// 1.06.5
//         SDM added mode parameter to CheckPosition
//         SDM removed old add & edit functions
//         SDM added new add function
// 1.06.6U2
//         SDM Added function CPhoneticSegment::AdjustPositionAll()
//         SDM Added function CTextSegment::CountWords()
// 1.5Test8.1
//         SDM removed unused constructors/destructors
//         SDM changed alignToSegment to take a segment index to align to
//         SDM added CDependentTextSegment
//         SDM changed CGlossSegment to handle embedded POS and dependentText segments
//         SDM changed CSegment::GetTexts() to return const
//         SDM changed added bOverlap to CheckPosition CheckCursors
// 1.5Test8.2
//         SDM added CSegment::GetSize
//         SDM changed return type on CTextSegment::AdjustPositionToPhonetic
// 1.5Test8.3
//         SDM Changed ReplaceSelectedSegment to keep existing position for
//                dependents except CPhoneticSegment
// 1.5Test11.0
//         SDM added CSegment::GetStop()
//         SDM made m_Durations private
//         SDM added range checking to GetOffset() & GetDuration()
//         SDM added GetStop() & FindStop()
//         SDM added call to updateDurations() to CSegment::GetDurations();
//         SDM added virtual function updateDurations as NULL in CSegment
//         SDM added CTextSegment::updateDurations to create a correct m_Durations as needed
//         SDM changed AlignToSegment() to AlignOffsetToSegment() & AlignStopToSegment()
//         SDM added CDependentSegment::CheckPositionToSegment()
//         SDM changed CDependent::CheckPositon() & CDependentTextSegment::CheckPosition() to use CheckPositionToSegment()
//         SDM added CTextSegment::GetDuration() & CDependentTextSegment::GetDuration()
//         SDM added CTextSegment::AdjustPositionToSegment()
//         SDM derived CTextSegment::AdjustPositionToPhonetic() from CTextSegment::AdjustPositionToSegment()
//         SDM derived CDependentTextSegment::AdjustPositionToGloss() from CTextSegment::AdjustPositionToSegment()
//         SDM removed CDependentTextSegment::LimitPosition()
// 1.5Test11.3
//         SDM replaced changes to Gloss and children (see 11.0)
//         SDM changed CSegment::AdjustPositionAll to CSegment::Adjust
// 1.5Test11.4
//         SDM added support for editing PHONEMIC/TONE/ORTHO to span multiple segments
/////////////////////////////////////////////////////////////////////////////
#ifndef SEGMENT_H
#define SEGMENT_H

#include "sa_ipa.h"  // Support for CFontTable
#include "SaString.h"
#include "process\Process.h"

BOOL CALLBACK EXPORT gIPAInputFilter(CSaString &);

class CSaDoc;
class CSaView;

//###########################################################################
// CSegment data processing

class CSegment : public CProcess
{

public:
    CSegment(EAnnotation index, int master = -1);
    virtual ~CSegment();

    virtual void Serialize(CArchive & ar);
    int GetMasterIndex(void) const;
    EAnnotation GetAnnotationIndex(void) const;
    // get copies of internal data.  (const functions)
    virtual CFontTable * NewFontTable() const = 0;  // return selected font table
    int GetOffsetSize() const;
    int GetDurationSize() const;
    DWORD GetOffset(const int nIndex) const;        // return offset
    DWORD GetDuration(const int nIndex) const;      // return duration
    DWORD GetStop(const int nIndex) const;          // return stop
    TCHAR GetChar(int nIndex) const;                // return annotation character
    int GetSelection() const;						// return the index of the selected character
    int GetPrevious(int nIndex = -1) const;         // return the index of the previous segment
    int GetNext(int nIndex = -1) const;             // return the index of the next segment
    int FindOffset(DWORD dwOffset) const;           // return segment with matching offset
    int FindStop(DWORD dwOffset) const;             // return segment with matching stop
    int FindFromPosition(DWORD dwPosition, BOOL bWithin = FALSE) const; // get segment index from position
    virtual BOOL Match(int index, LPCTSTR find);
	virtual void Replace( CSaDoc * pDoc, int index, LPCTSTR find, LPCTSTR replace);
	int FindIndex( DWORD offset);
    virtual int FindNext(int fromIndex, LPCTSTR strToFind);
    virtual int FindPrev(int fromIndex, LPCTSTR strToFind);
    int CheckCursors(CSaDoc *, BOOL bOverlap) const;        // checks the position of the cursors for new segment
    DWORD GetDurationAt(int index) const;
    void SetAt(int index, DWORD offset, DWORD duration);
    void InsertAt(int index, DWORD offset, DWORD duration);
    // remove offset and duration
    void RemoveAt(int index, int length);
	virtual void Split( CSaDoc * pDoc, CSaView * pView, DWORD start, DWORD newStopStart, DWORD newDuration);
	virtual void Merge( CSaDoc * pDoc, CSaView * pView, DWORD thisOffset, DWORD prevOffset, DWORD thisStop);
	virtual void MoveDataLeft( DWORD offset);
	virtual void MoveDataRight( DWORD offset);
	void AdjustDuration( DWORD offset, DWORD duration);
	virtual void Add( CSaDoc * pDoc, CSaView * pView, DWORD dwStart, CSaString & szString, bool bDelimiter, bool bCheck) = 0;
	void Add( DWORD offset, DWORD duration);
	void Remove( int sel);

    enum EMode
    {
        MODE_AUTOMATIC,
        MODE_EDIT,
        MODE_ADD
    };

    virtual int CheckPosition(ISaDoc *,DWORD dwStart,DWORD dwStop, EMode nMode=MODE_AUTOMATIC, BOOL bOverlap=TRUE) const = 0;
	
	enum ELimit
	{
        LIMIT_MOVING_START=1,
        LIMIT_MOVING_STOP=2,
        LIMIT_MOVING_BOTH=3,
        LIMIT_MOVING_START_NO_OVERLAP=4,
        LIMIT_MOVING_STOP_NO_OVERLAP=5,
        LIMIT_MOVING_BOTH_NO_OVERLAP=6,
    };

    virtual void LimitPosition(CSaDoc *,DWORD & dwStart,DWORD & dwStop, ELimit nMode=LIMIT_MOVING_BOTH) const = 0;
    BOOL NeedToScroll(CSaView & saView, int nIndex) const;
    int FirstVisibleIndex(CSaDoc & saDoc) const;
    int LastVisibleIndex(CSaDoc & saDoc) const;
    /** returns true if there are no offsets */
    BOOL IsEmpty() const;

    // modify internal data
    void AdjustCursorsToSnap(CDocument * pDoc); // adjust cursors to appropriate snap position
    void SetSelection(int nIndex); // set selection
    virtual void Remove(CDocument *, BOOL bCheck = TRUE); // remove a segment
    virtual DWORD RemoveNoRefresh(CDocument *); // remove a segment
    virtual void ReplaceSelectedSegment(CSaDoc * pSaDoc, LPCTSTR replace);
    virtual void DeleteContents(); // delete all contents of the segment arrays
    virtual void Adjust(ISaDoc * saDoc, int nIndex, DWORD dwOffset, DWORD dwDuration = 0, bool segmental = false); // adjust position of segment
    virtual BOOL SetAt(const CSaString *, bool delimiter, DWORD dwStart, DWORD dwDuration);  // sets a new segment
    virtual BOOL Insert(int nIndex, LPCTSTR szText, bool delimiter, DWORD dwStart, DWORD dwDuration); // insert a new segment
    virtual long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    virtual CSaString GetContainedText(DWORD dwStart, DWORD dwStop);
    virtual CSaString GetOverlappingText(DWORD dwStart, DWORD dwStop);

    // annotation functions
    virtual CSaString GetSegmentString(int nIndex) const;   // return segment string
    virtual int GetSegmentLength(int nIndex) const;         // return segment length
    virtual CSaString GetText(int nIndex) const;            // return text string
    CSaString * GetString();                                // return pointer to annotation string
    size_t GetStringLength();                               // return the text string length
    void SetString(LPCTSTR val);                             // sets the annotation string

	virtual CSaString GetDefaultChar();

	void Validate();

protected:
    typedef BOOL (CALLBACK EXPORT * TpInputFilterProc)(CSaString &);

    CSaString * m_pAnnotation;          // annotation string
    int m_nSelection;                   // selected segment
    EAnnotation m_nAnnotationIndex;
    int m_nMasterIndex;

private:
    CDWordArray m_Offset;               // array of offsets
    CDWordArray m_Duration;             // array of durations

public:
    virtual TpInputFilterProc GetInputFilter(void) const; // filter function for input
};

#endif

