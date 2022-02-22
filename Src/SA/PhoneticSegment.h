#ifndef PHONETICSEGMENT_H
#define PHONETICSEGMENT_H

#include "IndependentSegment.h"
#include "sa_ipa.h"

class CPhoneticSegment : public CIndependentSegment {
public:
    CPhoneticSegment() = delete;

    virtual long Process(void * pCaller, CSaDoc * pModel, int nProgress = 0, int nLevel = 1);
	// remove a segment
    virtual void Remove(CSaDoc * pModel, int index, BOOL bCheck);          
    virtual void ReplaceSelectedSegment(CSaDoc * pSaDoc, LPCTSTR replace, bool noSnap);
    virtual bool Filter();
    virtual bool Filter(CString & text);
	virtual bool ContainsText( DWORD offset, DWORD stop);

	virtual int Add( CSaDoc * pModel, DWORD offset, DWORD duration);
	virtual void AddAt( CSaDoc * pModel, int index, DWORD offset, DWORD duration);

protected:
	// exit processing on error
    virtual long Exit(int nError); 
	virtual CString GetDefaultText() {
		return CString(SEGMENT_DEFAULT_CHAR);
	}

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

