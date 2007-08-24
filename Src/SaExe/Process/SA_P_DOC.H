/////////////////////////////////////////////////////////////////////////////
// sa_p_doc.h:
// Interface of the CProcessDoc
//
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_DOC_H

#include "CSaString.h"

#define _SA_P_DOC_H

class ISaDoc;

//###########################################################################
// CProcessDoc data processing

class CProcessDoc : public CDataProcess  
{
// Construction/destruction/creation
public:
  CProcessDoc(ISaDoc *pDoc);
  virtual ~CProcessDoc() {;}

  // Attributes
private:
  ISaDoc *m_pDoc;

  // Operations
public:
  virtual void SetDataInvalid() {m_dwBufferOffset = UNDEFINED_OFFSET;}
  virtual const TCHAR * GetProcessFileName() {return m_pDoc->GetRawDataWrk(0);} // return process temporary file path and name 

  long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);

private:
  // Can you have a virtual private function??? Only friends?
  virtual void DeleteTempFile() {;}      // delete the temporary file (private use SetDataInvalid)
};

#endif //_SA_P_DOC_H
