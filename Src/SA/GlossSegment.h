#ifndef GLOSS_SEGMENT_H
#define GLOSS_SEGMENT_H

#include "TextSegment.h"

//###########################################################################
// CGlossSegment data processing

class CGlossSegment : public CTextSegment {

public:
    CGlossSegment(EAnnotation index, int master = -1);
    virtual ~CGlossSegment();

    virtual long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
	// remove a segment
    virtual void Remove(CSaDoc * pDoc, int index, BOOL bCheck);     
    void CorrectGlossDurations(ISaDoc * pSaDoc);
    virtual void Serialize(CArchive & ar);
	virtual bool ContainsText( DWORD offset, DWORD stop);

	virtual void AddAt( CSaDoc * pDoc, int index, DWORD offset, DWORD duration);

	virtual CString GetDefaultText() {
		return CString(WORD_DELIMITER);
	}

protected:
	// exit processing on error
    long Exit(int nError);

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

