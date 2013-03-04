#ifndef PHONETICSEGMENT_H
#define PHONETICSEGMENT_H

#include "IndependentSegment.h"
#include "sa_ipa.h"

//###########################################################################
// CPhoneticSegment data processing

class CPhoneticSegment : public CIndependentSegment {
	// Operations
public:
	CPhoneticSegment(int index, int master = -1);
protected:
	virtual long Exit(int nError); // exit processing on error

public:
	virtual CSegment::TpInputFilterProc GetInputFilter(void) const; // filter function for input
	virtual long Process(void* pCaller, CSaDoc* pDoc, int nProgress = 0, int nLevel = 1);
	virtual void Remove(CDocument*, BOOL bCheck = TRUE); // remove a segment
	virtual DWORD RemoveNoRefresh(CDocument* pSaDoc);
	virtual void ReplaceSelectedSegment(CDocument* pSaDoc, const CSaString & str);

	// Attributes
private:
	virtual CFontTable* NewFontTable() const;
};

#endif
