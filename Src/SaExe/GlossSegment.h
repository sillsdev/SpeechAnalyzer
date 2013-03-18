#ifndef GLOSS_SEGMENT_H
#define GLOSS_SEGMENT_H

#include "TextSegment.h"

//###########################################################################
// CGlossSegment data processing

class CGlossSegment : public CTextSegment {

public:
    CGlossSegment(int index, int master = -1);
    virtual ~CGlossSegment();

    virtual long Process(void * pCaller, CSaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    virtual BOOL SetAt(const CSaString *, bool delimiter, DWORD dwStart, DWORD dwDuration); // insert a new segment
    virtual BOOL Insert(int nIndex, LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration); // insert a new segment
    virtual void DeleteContents(); // delete all contents of the segment arrays
    virtual DWORD RemoveNoRefresh(CDocument * pSaDoc = NULL);
    virtual void Remove(CDocument *, BOOL bCheck = TRUE); // remove a segment
    virtual void ReplaceSelectedSegment(CDocument * pSaDoc, const CSaString & str);
    CStringArray * GetPOSs();
    void CorrectGlossDurations(CSaDoc * pSaDoc);
    virtual void Serialize(CArchive & ar);

protected:
    long Exit(int nError); // exit processing on error

private:
    CStringArray * m_pPOS;                    // array of text strings
    virtual CFontTable * NewFontTable() const;

};

#endif

