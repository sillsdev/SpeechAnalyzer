/////////////////////////////////////////////////////////////////////////////
// sa_p_zcr.h:
// Interface of the CProcessZCross class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_ZCR_H
#define _SA_P_ZCR_H


//###########################################################################
// CProcessZCross data processing

class CProcessZCross : public CDataProcess
{
// Construction/destruction/creation
public:
  CProcessZCross();
  virtual ~CProcessZCross();

// Attributes
private:

// Operations
public:
  long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);
};

#endif //_SA_P_ZCR_H
