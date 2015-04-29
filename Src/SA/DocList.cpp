#include "stdafx.h"
#include "DocList.h"
#include "Sa_Doc.h"

CDocList::CDocList() { 
	// Constructor
    m_posTemplate = NULL;
    m_pTemplate = NULL;
    m_posDoc = NULL;
}

CSaDoc * CDocList::pdocFirst() { 
	// return first open document
    m_posTemplate = AfxGetApp()->GetFirstDocTemplatePosition();
    ASSERT(m_posTemplate);
    m_pTemplate = AfxGetApp()->GetNextDocTemplate(m_posTemplate);
    ASSERT(m_pTemplate);
    m_posDoc = m_pTemplate->GetFirstDocPosition();
    return pdocNext();
}

CSaDoc * CDocList::pdocNext() { 
	// return next open document
    ASSERT(m_pTemplate);
    if (!m_posDoc) {
        return NULL;
    }
    CSaDoc * pdoc = (CSaDoc *)m_pTemplate->GetNextDoc(m_posDoc);
    ASSERT(pdoc->IsKindOf(RUNTIME_CLASS(CSaDoc)));
    return pdoc;
}

#ifdef _DEBUG
void CDocList::Dump(CDumpContext & /*dc*/) const {}
#endif

