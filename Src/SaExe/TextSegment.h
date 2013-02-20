#ifndef TEXTSEGMENT_H
#define TEXTSEGMENT_H

#include "DependentSegment.h"

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
	CStringArray * m_pTexts;             // array of text strings

	// Operations
protected:
	CSaString GetText(int nIndex); // return text string

public:
	virtual const CStringArray* GetTexts(); // return pointer to text string array object
	virtual BOOL SetAt( const CSaString*, bool delimiter, DWORD dwStart, DWORD dwDuration);	// sets a new segment
	virtual BOOL Insert(int nIndex, const CSaString*, bool delimiter, DWORD dwStart, DWORD dwDuration); // insert a new segment
	virtual void DeleteContents(); // delete all contents of the segment arrays
	virtual int  GetSegmentLength(int nIndex) const; // return segment length always 1
	virtual CSaString GetSegmentString(int nIndex) const; // return segment string
	virtual void Add(CSaDoc* pDoc, DWORD dwStart, CSaString & szString, bool bDelimiter = false, bool bCheck = true); // add a segment
	virtual void Remove(CDocument*, BOOL bCheck = TRUE); // remove a segment
	virtual void ReplaceSelectedSegment(CDocument* pSaDoc, const CSaString & str);
	virtual DWORD RemoveNoRefresh(CDocument* pSaDoc = NULL);
	virtual BOOL Match(int index, const CSaString & strToFind);
	virtual int FindNext(int fromIndex, const CSaString & strToFind, CSaDoc & SaDoc);
	virtual int FindPrev(int fromIndex, const CSaString & strToFind, CSaDoc & SaDoc);
	virtual int CheckPosition(CSaDoc*,DWORD dwStart,DWORD dwStop, EMode nMode=MODE_AUTOMATIC, BOOL bUnused = TRUE) const;
	virtual void  LimitPosition(CSaDoc*,DWORD& dwStart,DWORD& dwStop, int nMode=LIMIT_MOVING_BOTH) const;
	int CountWords();
	DWORD CalculateDuration(CSaDoc* pDoc, const int nIndex) const;
	virtual CSaString CTextSegment::GetTextFor( DWORD dwStart, DWORD dwStop);

};

#endif

