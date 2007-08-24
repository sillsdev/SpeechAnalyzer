// doclist.h

#ifndef Doclist_H
#define Doclist_H

/////////////////////////////////////////////////////////////////////////////
// CDocList

// This class assumes use of a single template in the app

class CDocList // Hungarian: doclst
{
  POSITION m_posTemplate;
  CDocTemplate* m_pTemplate;
  POSITION m_posDoc;

public:
  CDocList() // Constructor
  {
    m_posTemplate = NULL;
    m_pTemplate = NULL;
    m_posDoc = NULL;
  }

  CSaDoc* pdocFirst() // return first open document
  {
    m_posTemplate = AfxGetApp()->GetFirstDocTemplatePosition();
    ASSERT(m_posTemplate);
    m_pTemplate = AfxGetApp()->GetNextDocTemplate(m_posTemplate);
    ASSERT(m_pTemplate);
    m_posDoc = m_pTemplate->GetFirstDocPosition();
    return pdocNext();
  }

  CSaDoc* pdocNext() // return next open document
  {
    ASSERT( m_pTemplate );
    if ( !m_posDoc )
      return NULL;
    CSaDoc* pdoc = (CSaDoc*)m_pTemplate->GetNextDoc(m_posDoc);
    ASSERT( pdoc->IsKindOf(RUNTIME_CLASS(CSaDoc)) );
    return pdoc;
  }

#ifdef _DEBUG
  virtual void Dump(CDumpContext& /*dc*/) const {}
#endif  // _DEBUG
};  // class CDocList

#endif // Doclist_H