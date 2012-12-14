/////////////////////////////////////////////////////////////////////////////
// sa_segm.h:
// Interface of the CSegment
//                  CDependentSegment
//                  CTextSegment
//                  CPhoneticSegment
//                  CToneSegment
//                  CPhonemicSegment
//                  COrthoSegment
//                  CGlossSegment    classes.
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
//         SDM added CReferenceSegment
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
//         SDM made m_pDurations private
//         SDM added range checking to GetOffset() & GetDuration()
//         SDM added GetStop() & FindStop()
//         SDM added call to updateDurations() to CSegment::GetDurations();
//         SDM added virtual function updateDurations as NULL in CSegment
//         SDM added CTextSegment::updateDurations to create a correct m_pDurations as needed
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
#ifndef _SA_SEGM_H
#define _SA_SEGM_H

#include "sa_ipa.h"  // Support for CFontTable
#include "CSaString.h"
#include "process\sa_proc.h"

BOOL CALLBACK EXPORT gIPAInputFilter(CSaString&);

class CSaDoc;
class CSaView;

//###########################################################################
// CSegment data processing

class CSegment : public CDataProcess {

	// Construction/destruction/creation
public:
	CSegment(int index, int master = -1);
	virtual ~CSegment();
	virtual void Serialize(CArchive& ar);


	// Attributes
private:
	CDWordArray * m_pOffset;            // array of offsets
	CDWordArray * m_pDuration;          // array of durations

protected:
	CSaString * m_pAnnotation;			// annotation string
	int m_nSelection;					// selected segment
	int m_nAnnotationIndex;
	int m_nMasterIndex;


	// Operations
protected:
	// SDM 1.06.3b
	typedef BOOL (CALLBACK EXPORT *TpInputFilterProc)(CSaString&);
public:
	int GetMasterIndex(void) const {return m_nMasterIndex;};
	int GetAnnotationIndex(void) const {return m_nAnnotationIndex;};
	virtual TpInputFilterProc GetInputFilter(void) const {return NULL;}; // filter function for input
	// get copies of internal data.  (const functions)
	virtual CFontTable* NewFontTable() const = 0; // return selected font table
	int GetOffsetSize() const;
	int GetDurationSize() const;
	DWORD GetOffset(const int nIndex) const; // return offset
	DWORD GetDuration(const int nIndex) const; // return duration
	DWORD GetStop(const int nIndex) const; // return stop
	TCHAR GetChar(int nIndex) const {return m_pAnnotation->GetAt(nIndex);} // return annotation character
	int GetSelection() const {return m_nSelection;} // return the index of the selected character
	int GetPrevious(int nIndex = -1) const; // return the index of the previous segment
	int GetNext(int nIndex = -1) const; // return the index of the next segment
	int FindOffset(DWORD dwOffset) const; // return segment with matching offset
	int FindStop(DWORD dwOffset) const; // return segment with matching stop
	int FindFromPosition(DWORD dwPosition, BOOL bWithin = FALSE) const; // get segment index from position
	virtual BOOL Match(int index, const CSaString & strToFind);
	virtual int FindNext(int fromIndex, const CSaString & strToFind, CSaDoc & SaDoc);
	virtual int FindPrev(int fromIndex, const CSaString & strToFind, CSaDoc & SaDoc);
	virtual CSaString GetSegmentString(int nIndex) const; // return segment string
	virtual int GetSegmentLength(int nIndex) const; // return segment length
	int CheckCursors(CSaDoc*, BOOL bOverlap) const; // checks the position of the cursors for new segment
	
	DWORD GetDurationAt(int index) const;
	void SetDurationAt(int index, DWORD duration);
	void InsertAt(int index, DWORD offset, DWORD duration);
	// remove offset and duration
	void RemoveAt(int index, int length);

	enum {
		MODE_AUTOMATIC,
		MODE_EDIT,
		MODE_ADD
	};

	virtual int CheckPosition(CSaDoc*,DWORD dwStart,DWORD dwStop, int nMode=MODE_AUTOMATIC,BOOL bOverlap=TRUE) const = 0;
	
	enum {
		LIMIT_MOVING_START=1,
		LIMIT_MOVING_STOP=2,
		LIMIT_MOVING_BOTH=3,
		LIMIT_NO_OVERLAP=4
	};

	virtual void LimitPosition(CSaDoc*,DWORD& dwStart,DWORD& dwStop, int nMode=LIMIT_MOVING_BOTH) const = 0;
	virtual CSaString GetText(int /*nIndex*/) const {return *m_pAnnotation;} // return text string
	BOOL NeedToScroll(CSaView & saView, int nIndex) const;
	int FirstVisibleIndex(CSaDoc & saDoc) const;
	int LastVisibleIndex(CSaDoc & saDoc) const;
	/** returns true if there are no offsets */
	BOOL IsEmpty() const;

	// give pointers to internal data, allowing caller to modify data.
	CSaString * GetString(); // return pointer to annotation string
	virtual const CStringArray* GetTexts(); // return pointer to text string array object

	// modify internal data
	void SelectSegment(CSaDoc & pSaDoc, int segIdx);
	void AdjustCursorsToSnap(CDocument* pDoc); // adjust cursors to apropriate snap position
	void SetSelection(int nIndex); // set selection
	virtual void Remove(CDocument*, BOOL bCheck = TRUE); // remove a segment
	virtual DWORD RemoveNoRefresh(CDocument*); // remove a segment
	virtual void ReplaceSelectedSegment(CDocument* pSaDoc, const CSaString & str);
	virtual void DeleteContents(); // delete all contents of the segment arrays
	virtual void Adjust(CSaDoc* saDoc, int nIndex, DWORD dwOffset, DWORD dwDuration = 0); // adjust position of segment
	virtual BOOL Insert(int nIndex, const CSaString*, bool delimiter, DWORD dwStart, DWORD dwDuration); // insert a new segment
	virtual long Process(void* /*pCaller*/, CSaDoc* /*pDoc*/, int /*nProgress*/ = 0, int /*Level*/ = 1) { return PROCESS_ERROR; };
};

//###########################################################################
// CDependentSegment data processing

class CDependentSegment : public CSegment
{
public:
	CDependentSegment(int index, int master = -1): CSegment(index,master) {};

	// Attributes
private:

	// Operations
protected:
	int AlignOffsetToMaster(CDocument*, DWORD* pdwOffset) const; // find the nearest master segment to the given position
	int AlignStopToMaster(CDocument*, DWORD* pdwStop) const; // find the nearest master segment to the given position
public:
	void AdjustCursorsToMaster(CDocument* pSaDoc, BOOL bAdjust = TRUE, DWORD* pdwOffset = NULL, DWORD* pdwStop = NULL) const; // adjust cursors to the nearest phonetic segment to the given position SDM 1.06.1.2 Added function
	int AdjustPositionToMaster(CDocument* pSaDoc, DWORD& pdwOffset, DWORD& pdwStop) const;
	int CheckPositionToMaster(CSaDoc*,DWORD dwStart,DWORD dwStop, int nMode) const;
	virtual int  CheckPosition(CSaDoc* pDoc, DWORD dwStart, DWORD dwStop, int nMode=MODE_AUTOMATIC, BOOL /*bUnused*/=TRUE) const
		{ return CheckPositionToMaster(pDoc, dwStart, dwStop, nMode);};
	virtual void LimitPosition(CSaDoc*,DWORD& dwStart,DWORD& dwStop, int nMode=LIMIT_MOVING_BOTH) const;
	virtual void Add(CSaDoc* pDoc, DWORD dwStart, CSaString& szString, BOOL bDelimiter = FALSE, BOOL bCheck = TRUE); // add a segment
};

//###########################################################################
// CTextSegment data processing

class CTextSegment : public CDependentSegment
{

	// Construction/destruction/creation
public:
	CTextSegment(int index, int master = -1);

	virtual ~CTextSegment();
	virtual void Serialize(CArchive& ar);

	// Attributes
protected:
	CStringArray* m_pTexts;             // array of text strings

	// Operations
protected:
	CSaString GetText(int nIndex) {return m_pTexts->GetAt(nIndex);} // return text string
public:
	virtual const CStringArray* GetTexts() {return m_pTexts;} // return pointer to text string array object
	virtual BOOL Insert(int nIndex, const CSaString*, bool delimiter, DWORD dwStart, DWORD dwDuration); // insert a new segment
	virtual void DeleteContents(); // delete all contents of the segment arrays
	virtual int  GetSegmentLength(int /*nIndex*/) const {return 1;} // return segment length always 1
	virtual CSaString GetSegmentString(int nIndex) const {return m_pTexts->GetAt(nIndex);} // return segment string
	virtual void Add(CSaDoc* pDoc, DWORD dwStart, CSaString & szString, bool bDelimiter = false, bool bCheck = true); // add a segment
	virtual void Remove(CDocument*, BOOL bCheck = TRUE); // remove a segment
	virtual void ReplaceSelectedSegment(CDocument* pSaDoc, const CSaString & str);
	virtual DWORD RemoveNoRefresh(CDocument* pSaDoc = NULL);
	virtual BOOL Match(int index, const CSaString & strToFind);
	virtual int FindNext(int fromIndex, const CSaString & strToFind, CSaDoc & SaDoc);
	virtual int FindPrev(int fromIndex, const CSaString & strToFind, CSaDoc & SaDoc);
	virtual int CheckPosition(CSaDoc*,DWORD dwStart,DWORD dwStop, int nMode=MODE_AUTOMATIC, BOOL bUnused = TRUE) const;
	virtual void  LimitPosition(CSaDoc*,DWORD& dwStart,DWORD& dwStop, int nMode=LIMIT_MOVING_BOTH) const;
	int CountWords();
	DWORD CalculateDuration(CSaDoc* pDoc, const int nIndex) const;
};

class CDependentTextSegment : public CTextSegment
{
public:
	CDependentTextSegment(int index, int master = -1): CTextSegment(index,master) {};

	virtual void LimitPosition(CSaDoc*,DWORD& dwStart,DWORD& dwStop, int /*nMode*/=LIMIT_MOVING_BOTH) const {dwStart=GetOffset(GetSelection());dwStop=GetStop(GetSelection());return;};
	virtual BOOL Insert(int nIndex, const CSaString*, bool delimiter, DWORD dwStart, DWORD dwDuration); // insert a new segment
	virtual BOOL SetText(int nIndex, const CSaString*, int nDelimiter, DWORD dwStart, DWORD dwDuration); // insert a new segment
	int CheckPositionToMaster(CSaDoc*,DWORD dwStart,DWORD dwStop, int nMode) const;
	virtual void Add(CSaDoc* pDoc, DWORD dwStart, CSaString& szString, BOOL bDelimiter = FALSE, BOOL bCheck = TRUE); // add a segment
};

//###########################################################################
// CIndependentSegment data processing

class CIndependentSegment : public CSegment
{
	// Operations
public:
	CIndependentSegment(int index, int master = -1): CSegment(index,master) {};
	virtual int CheckPosition(CSaDoc*,DWORD dwStart,DWORD dwStop, int nMode=MODE_AUTOMATIC,BOOL bOverlap=TRUE) const;
	virtual void LimitPosition(CSaDoc*,DWORD& dwStart,DWORD& dwStop, int nMode=LIMIT_MOVING_BOTH) const;
};

//###########################################################################
// CPhoneticSegment data processing

class CPhoneticSegment : public CIndependentSegment {
	// Operations
public:
	CPhoneticSegment(int index, int master = -1): CIndependentSegment(index,master) {};
protected:
	virtual long Exit(int nError); // exit processing on error

public:
	virtual TpInputFilterProc GetInputFilter(void) const {return gIPAInputFilter;}; // filter function for input
	virtual long Process(void* pCaller, CSaDoc* pDoc, int nProgress = 0, int nLevel = 1);
	virtual void Remove(CDocument*, BOOL bCheck = TRUE); // remove a segment
	virtual DWORD RemoveNoRefresh(CDocument* pSaDoc);
	virtual void ReplaceSelectedSegment(CDocument* pSaDoc, const CSaString & str);

	// Attributes
private:
	virtual CFontTable* NewFontTable() const {return new CFontTableIPA;};
};

//###########################################################################
// CMusicPhraseSegment data processing

class CMusicPhraseSegment : public CIndependentSegment
{
	// Operations
public:
	CMusicPhraseSegment(int index, int master = -1): CIndependentSegment(index,master) {};
	virtual int CheckPosition(CSaDoc*,DWORD dwStart,DWORD dwStop, int nMode=MODE_AUTOMATIC,BOOL bOverlap=TRUE) const;

	// Attributes
private:
	virtual CFontTable* NewFontTable() const {return new CFontTableANSI;};
};

//###########################################################################
// CToneSegment data processing

class CToneSegment : public CDependentSegment
{
	// Operations
public:
	CToneSegment(int index, int master = -1): CDependentSegment(index,master) {};

	// Attributes
private:
	virtual CFontTable* NewFontTable() const {return new CFontTableANSI;};
};

//###########################################################################
// CPhonemicSegment data processing

class CPhonemicSegment : public CDependentSegment {
	// Operations
public:
	CPhonemicSegment(int index, int master = -1) : CDependentSegment(index,master) {};
	virtual TpInputFilterProc GetInputFilter(void) const {return gIPAInputFilter;}; // filter function for input

	// Attributes
private:
	virtual CFontTable* NewFontTable() const {return new CFontTableIPA;};
};

//###########################################################################
// COrthoSegment data processing

class COrthoSegment : public CDependentSegment
{
public:
	COrthoSegment(int index, int master = -1): CDependentSegment(index,master) {};

	// Attributes
private:
	virtual CFontTable* NewFontTable() const {return new CFontTableANSI;};
};

//###########################################################################
// CGlossSegment data processing

class CGlossSegment : public CTextSegment
{

	// Construction/destruction/creation
public:
	CGlossSegment(int index, int master = -1);
	virtual ~CGlossSegment();

	// Attributes
private:
	CStringArray* m_pPOS;                    // array of text strings
	virtual CFontTable* NewFontTable() const {return new CFontTableANSI;};

	// Operations
protected:
	long Exit(int nError); // exit processing on error

public:
	virtual long Process(void* pCaller, CSaDoc* pDoc, int nProgress = 0, int nLevel = 1);
	virtual BOOL Insert(int nIndex, const CSaString*, bool delimiter, DWORD dwStart, DWORD dwDuration); // insert a new segment
	virtual void DeleteContents(); // delete all contents of the segment arrays
	virtual DWORD RemoveNoRefresh(CDocument* pSaDoc = NULL);
	virtual void Remove(CDocument*, BOOL bCheck = TRUE); // remove a segment
	virtual void ReplaceSelectedSegment(CDocument* pSaDoc, const CSaString & str);
	CStringArray* GetPOSs() { return m_pPOS;}
	void CorrectGlossDurations(CSaDoc* pSaDoc);
	virtual void Serialize(CArchive& ar);

};


class CReferenceSegment : public CDependentTextSegment
{
public:
	CReferenceSegment(int index, int master = -1): CDependentTextSegment(index,master) {};

	// Attributes
private:
	virtual CFontTable* NewFontTable() const {return new CFontTableANSI;};
};

class ArchiveTransfer
{
public:
	static int tInt(CArchive& ar, int x=0);
	static short int tShortInt(CArchive& ar, short int x=0);
	static DWORD tDWORD(CArchive& ar, DWORD x=0);
};

#endif //_SA_SEGM_H

