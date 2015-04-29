// doclist.h

#ifndef Doclist_H
#define Doclist_H

class CSaDoc;

/////////////////////////////////////////////////////////////////////////////
// CDocList

// This class assumes use of a single template in the app

class CDocList { 
	// Hungarian: doclst
    POSITION m_posTemplate;
    CDocTemplate * m_pTemplate;
    POSITION m_posDoc;

public:
    CDocList();
    CSaDoc * pdocFirst();
    CSaDoc * pdocNext();

#ifdef _DEBUG
    virtual void Dump(CDumpContext & /*dc*/) const;
#endif
};

#endif