#ifndef MUSICPHRASESEGMENT_H
#define MUSICPHRASESEGMENT_H

#include "IndependentSegment.h"

//###########################################################################
// CMusicPhraseSegment data processing

class CMusicPhraseSegment : public CIndependentSegment
{
	// Operations
public:
	CMusicPhraseSegment(int index, int master = -1);
	virtual int CheckPosition(CSaDoc*,DWORD dwStart,DWORD dwStop, EMode nMode=MODE_AUTOMATIC,BOOL bOverlap=TRUE) const;

	// Attributes
private:
	virtual CFontTable* NewFontTable() const;
};

#endif

