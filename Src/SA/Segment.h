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

#include "SaString.h"

class CSaDoc;
class CSaView;
class CFontTable;

//###########################################################################
// CSegment data processing
class CSegment : public CProcess {

public:
    CSegment(Context & context, EAnnotation index, int master = -1);
    CSegment() = delete;

    virtual void Serialize(CArchive & ar);

    enum EMode {
        MODE_AUTOMATIC,
        MODE_EDIT,
        MODE_ADD
    };

    enum ELimit {
        LIMIT_MOVING_START=1,
        LIMIT_MOVING_STOP=2,
        LIMIT_MOVING_BOTH=3,
        LIMIT_MOVING_START_NO_OVERLAP=4,
        LIMIT_MOVING_STOP_NO_OVERLAP=5,
        LIMIT_MOVING_BOTH_NO_OVERLAP=6,
    };

    int GetMasterIndex(void) const;
    EAnnotation GetAnnotationIndex(void) const;
	bool Is( EAnnotation type) const;

    // get copies of internal data.  (const functions)
	// return selected font table
    virtual CFontTable * NewFontTable() const = 0;  
    int GetOffsetSize() const;
    int GetDurationSize() const;
	// return offset
    DWORD GetOffset(const int nIndex) const;
	// return duration
    DWORD GetDuration(const int nIndex) const;
	// return text
    CString GetText(const int nIndex) const;
	// return stop
    DWORD GetStop(const int nIndex) const;
	// return the index of the selected character
    int GetSelection() const;
	// return the index of the previous segment
    int GetPrevious(int nIndex = -1) const;
	// return the index of the next segment
    int GetNext(int nIndex = -1) const;
	// return segment with matching offset
    int FindOffset(DWORD dwOffset) const;
	// return segment with matching stop
    int FindStop(DWORD dwOffset) const;
	// get segment index from position
    int FindFromPosition(DWORD dwPosition, BOOL bWithin = FALSE) const; 
    virtual bool Match(int index, LPCTSTR find);
    virtual void Replace(CSaDoc * pModel, int index, LPCTSTR find, LPCTSTR replace);
    int FindIndex(DWORD offset);
    int FindIndex(DWORD offset,DWORD duration);
	int FindWithin(DWORD offset);
    virtual int FindNext(int fromIndex, LPCTSTR strToFind);
    virtual int FindPrev(int fromIndex, LPCTSTR strToFind);
    // checks the position of the cursors for new segment
    int CheckCursors(CSaDoc *, BOOL bOverlap) const;
    DWORD GetDurationAt(int index) const;
    void InsertAt(int index, DWORD offset, DWORD duration);
    void InsertAt(int index, LPCTSTR text, DWORD offset, DWORD duration);
    void AdjustDuration(DWORD offset, DWORD duration);
    virtual void Add(CSaDoc * pModel, CSaView * pView, DWORD dwStart, CSaString & szString, bool bDelimiter, bool bCheck) = 0;
	// insert a new segment
    virtual BOOL SetText(int nIndex, LPCTSTR pszString);   

    virtual int CheckPosition(ISaDoc *,DWORD dwStart,DWORD dwStop, EMode nMode=MODE_AUTOMATIC, BOOL bOverlap=TRUE) const = 0;

    virtual void LimitPosition(CSaDoc *,DWORD & dwStart,DWORD & dwStop, ELimit nMode=LIMIT_MOVING_BOTH) const = 0;
    BOOL NeedToScroll(CSaView & saView, int nIndex) const;
    int FirstVisibleIndex(CSaDoc & saDoc) const;
    int LastVisibleIndex(CSaDoc & saDoc) const;
    /** returns true if there are no offsets */
    BOOL IsEmpty() const;

	// remove a segment
    virtual void Remove(CSaDoc * pModel, int index, BOOL bCheck);
	// remove text, offset and duration
    virtual void RemoveAt(int index);
	// delete all contents of the segment arrays
    virtual void DeleteContents();
	// erase all text entries
	void DeleteText();
	// erase the text for a given element
	void ClearText(int index);

    // modify internal data
	// adjust cursors to appropriate snap position
    void AdjustCursorsToSnap(CDocument * pModel);
	// set selection
    void SetSelection(int nIndex);
    virtual void ReplaceSelectedSegment(CSaDoc * pSaDoc, LPCTSTR replace, bool noSnap);
    // adjust position of segment
	virtual void Adjust( ISaDoc * saDoc, int nIndex, DWORD dwNewOffset, DWORD dwNewDuration, bool segmental);
	// adjust position of segment - local only
	void Adjust( int nIndex, DWORD dwNewOffset, DWORD dwNewDuration);
	// sets a new segment
    virtual BOOL SetAt(const CSaString & data, bool delimiter, DWORD dwStart, DWORD dwDuration, bool textSegment);
	// sets a new segment without delimiters
    virtual BOOL SetAt(const CSaString & data, DWORD dwStart, DWORD dwDuration);
	// insert a new segment
    virtual BOOL Insert(int nIndex, LPCTSTR szText, bool delimiter, DWORD dwStart, DWORD dwDuration);
	// insert a new segment
    virtual BOOL Append(LPCTSTR szText, bool delimiter, DWORD dwStart, DWORD dwDuration);
    virtual long Process(void * pCaller, ISaDoc * pModel, int nProgress = 0, int nLevel = 1);
    virtual CSaString GetContainedText(DWORD dwStart, DWORD dwStop);
    virtual CSaString GetOverlappingText(DWORD dwStart, DWORD dwStop);

    // annotation functions
	// return segment string
    virtual CSaString GetSegmentString(int nIndex) const;
	// return segment length
    virtual int GetSegmentLength(int nIndex) const;

	virtual CString GetDefaultText() {
		return CString("");
	}

	// return the contents of the entire segment
    CString GetContent() const;
	// return the text string length
    size_t GetContentLength() const;

    virtual bool Filter();
    virtual bool Filter(CString & text);

    int CountWords();

	// adjust segments to new file size
    void ShrinkSegment( CSaDoc & document, DWORD sectionStart, DWORD sectionLength);  
	// adjust segments to new file size
    void GrowSegment( CSaDoc & document, DWORD sectionStart, DWORD sectionLength);  

	bool IsDependent( CSegment & parent);

	int GetLastNonEmptyValue();

	virtual bool ContainsText( DWORD offset, DWORD duration);

    bool SplitSegment( CSaDoc & document, int sel, DWORD splitPosition);
    virtual bool Split( DWORD start, DWORD newStopStart);
    virtual bool Merge( DWORD thisOffset, DWORD prevOffset, DWORD thisStop);
    virtual bool MoveDataLeftSAB(DWORD offset, CString newText);
    virtual bool MoveDataLeft(DWORD offset);
    virtual bool MoveDataRight(DWORD offset, bool sab);

    void GetString(DWORD offset, CSaString & szString, bool removeDelimiter) {
        int nIndex = FindOffset(offset);
        if (nIndex != -1) {
            szString = GetSegmentString(nIndex);
            if (removeDelimiter) {
                if ((szString.GetLength() > 1) && (szString[0] == WORD_DELIMITER)) {
                    // Remove Word Delimiter
                    szString = szString.Mid(1);
                }
            }
            szString += "\t";
        } else {
            szString = "\t";
        }
    }

protected:
    int GetReferenceCount(CSegment * pSegment, int sel);
	virtual int Add( DWORD offset, DWORD duration);
	virtual int Add( LPCTSTR text, DWORD offset, DWORD duration);

    typedef BOOL (CALLBACK EXPORT * TpInputFilterProc)(CSaString &);
	// selected segment
    int m_nSelection;                   
    EAnnotation m_nAnnotationType;
    int m_nMasterType;

	// array of text strings
    CStringArray m_Text;

private:
	// array of offsets
    CDWordArray m_Offset;
	// array of durations
    CDWordArray m_Duration;
};

#endif

