#ifndef MUSICPHRASESEGMENT_H
#define MUSICPHRASESEGMENT_H

#include "IndependentSegment.h"

class CMusicPhraseSegment : public CIndependentSegment {
public:
    CMusicPhraseSegment(EAnnotation index, int master = -1);
    virtual int CheckPosition(ISaDoc *,DWORD dwStart,DWORD dwStop, EMode nMode=MODE_AUTOMATIC,BOOL bOverlap=TRUE) const;

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

