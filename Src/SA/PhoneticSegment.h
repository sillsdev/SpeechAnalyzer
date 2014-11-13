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

protected:
    virtual long Exit(int nError); // exit processing on error

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

