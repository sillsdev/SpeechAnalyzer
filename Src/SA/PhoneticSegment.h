#ifndef PHONETICSEGMENT_H
#define PHONETICSEGMENT_H

#include "IndependentSegment.h"
#include "sa_ipa.h"

class CPhoneticSegment : public CIndependentSegment
{
public:
    CPhoneticSegment(EAnnotation index, int master = -1);

    virtual long Process(void * pCaller, CSaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    virtual void Remove( CSaDoc * pDoc, int index, BOOL bCheck);			// remove a segment
    virtual void ReplaceSelectedSegment( CSaDoc * pSaDoc, LPCTSTR replace);
	virtual bool Filter();
	virtual bool Filter( CString & text);

protected:
    virtual long Exit(int nError); // exit processing on error
	virtual CSaString GetDefaultChar();

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

