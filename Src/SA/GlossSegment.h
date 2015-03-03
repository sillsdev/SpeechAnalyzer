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
    virtual void Remove(CSaDoc * pDoc, int index, BOOL bCheck);     // remove a segment
    void CorrectGlossDurations(ISaDoc * pSaDoc);
    virtual void Serialize(CArchive & ar);

protected:
    long Exit(int nError);                                          // exit processing on error

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

