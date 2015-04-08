#ifndef PHONETICSEGMENT_H
#define PHONETICSEGMENT_H

#include "IndependentSegment.h"
#include "sa_ipa.h"

class CPhoneticSegment : public CIndependentSegment {
public:
    CPhoneticSegment(EAnnotation index, int master = -1);

    virtual long Process(void * pCaller, CSaDoc * pDoc, int nProgress = 0, int nLevel = 1);
	// remove a segment
    virtual void Remove(CSaDoc * pDoc, int index, BOOL bCheck);          
    virtual void ReplaceSelectedSegment(CSaDoc * pSaDoc, LPCTSTR replace);
    virtual bool Filter();
    virtual bool Filter(CString & text);
	virtual bool ContainsText( DWORD offset, DWORD stop);

protected:
	// exit processing on error
    virtual long Exit(int nError); 
    virtual CString GetDefaultChar();

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

