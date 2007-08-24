/////////////////////////////////////////////////////////////////////////////
// sa_p_doc.cpp:
// Implementation of the CProcessDoc
//
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_proc.h"

#include "isa_doc.h"
#include "sa_p_doc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CProcessDoc::CProcessDoc(ISaDoc *pDoc) : m_pDoc(pDoc)
{ 
  m_lpData = new char[GetProcessBufferSize()];
  SetDataSize(pDoc->GetUnprocessedDataSize());
  SetDataReady();
}

long CProcessDoc::Process(void* pCaller, ISaDoc* pDoc, int nProgress, int nLevel)
{
  UNUSED_ALWAYS(nProgress);
  UNUSED_ALWAYS(pCaller);
  UNUSED_ALWAYS(pDoc);
  
  SetDataSize(pDoc->GetUnprocessedDataSize());
  SetDataReady();
  return MAKELONG(nLevel, 100);    
}
