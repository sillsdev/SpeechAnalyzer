#ifndef TEXTSEGMENT_H
#define TEXTSEGMENT_H

#include "DependentSegment.h"

//###########################################################################
// CTextSegment data processing

class CTextSegment : public CDependentSegment
{
public:
    CTextSegment(int index, int master = -1);
    virtual ~CTextSegment();

    virtual void Serialize(CArchive & ar);
    virtual const CStringArray & GetTexts();				// return pointer to text string array object
    virtual BOOL SetAt(const CSaString *, bool delimiter, DWORD dwStart, DWORD dwDuration);				// sets a new segment
    virtual BOOL Insert(int nIndex, LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration);// insert a new segment
    virtual void DeleteContents();							// delete all contents of the segment arrays
    virtual int  GetSegmentLength(int nIndex) const;		// return segment length always 1
    virtual CSaString GetSegmentString(int nIndex) const;	// return segment string
    virtual void Add(CSaDoc * pDoc, DWORD dwStart, CSaString & szString, bool bDelimiter = false, bool bCheck = true); // add a segment
    virtual void Remove(CDocument *, BOOL bCheck = TRUE);	// remove a segment
	virtual void Replace( CSaDoc * pDoc, int index, LPCTSTR find, LPCTSTR replace);
    virtual void ReplaceSelectedSegment( CSaDoc * pSaDoc, LPCTSTR replace);
    virtual DWORD RemoveNoRefresh(CDocument * pSaDoc = NULL);
    virtual BOOL Match(int index, LPCTSTR find);
    virtual int FindNext(int fromIndex, LPCTSTR strToFind);
    virtual int FindPrev(int fromIndex, LPCTSTR strToFind);
    virtual int CheckPosition(ISaDoc *,DWORD dwStart,DWORD dwStop, EMode nMode=MODE_AUTOMATIC, BOOL bUnused = TRUE) const;
    virtual void LimitPosition(CSaDoc *,DWORD & dwStart,DWORD & dwStop, ELimit nMode=LIMIT_MOVING_BOTH) const;
    int CountWords();
    DWORD CalculateDuration( ISaDoc * pDoc, const int nIndex) const;
    virtual CSaString GetContainedText(DWORD dwStart, DWORD dwStop);

protected:
    CSaString GetText(int nIndex);      // return text string

	CStringArray m_Texts;               // array of text strings
};

#endif

