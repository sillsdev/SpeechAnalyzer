#ifndef PHONETICSEGMENT_H
#define PHONETICSEGMENT_H

#include "IndependentSegment.h"
#include "sa_ipa.h"

class CPhoneticSegment : public CIndependentSegment
{
public:
    CPhoneticSegment(EAnnotation index, int master = -1);

    virtual CSegment::TpInputFilterProc GetInputFilter(void) const; // filter function for input
    virtual long Process(void * pCaller, CSaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    virtual void Remove(CDocument *, BOOL bCheck = TRUE);			// remove a segment
    virtual DWORD RemoveNoRefresh(CDocument * pSaDoc);
    virtual void ReplaceSelectedSegment( CSaDoc * pSaDoc, LPCTSTR replace);
	virtual void Split( CSaDoc * pDoc, CSaView * pView, DWORD start, DWORD newStopStart);
	virtual void Merge( CSaDoc * pDoc, CSaView * pView, DWORD thisOffset, DWORD prevOffset, DWORD thisStop);

protected:
    virtual long Exit(int nError); // exit processing on error
	virtual CSaString GetDefaultChar();

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

