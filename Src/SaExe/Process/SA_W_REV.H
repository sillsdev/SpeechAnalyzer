/////////////////////////////////////////////////////////////////////////////
// sa_w_rev.h:
// Interface of the CProcessWbEcho
//          CProcessWbReverb classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_REV_H
#define _SA_W_REV_H

//###########################################################################
// CProcessWbEcho data processing

class CProcessWbEcho : public CDataProcess
{
// Construction/destruction/creation
public:
  CProcessWbEcho();
  virtual ~CProcessWbEcho();

  // Attributes
private:
  UINT m_nDelay; // delay in ms
  int  m_nGain;  // gain in dB

  // Operations
public:
  long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);
  virtual int PropertiesDialog(); // calls the properties dialog for this process
  virtual void WriteProperties(Object_ostream& obs);
  virtual BOOL ReadProperties(Object_istream& obs);
};

//###########################################################################
// CProcessWbReverb data processing

class CProcessWbReverb : public CDataProcess
{
// Construction/destruction/creation
public:
  CProcessWbReverb();
  virtual ~CProcessWbReverb();

  // Attributes
private:
  UINT m_nDelay; // delay in ms
  int  m_nGain;  // gain in dB

  // Operations
public:
  long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);
  virtual int PropertiesDialog(); // calls the properties dialog for this process
  virtual void WriteProperties(Object_ostream& obs);
  virtual BOOL ReadProperties(Object_istream& obs);
};

#endif //_SA_W_REV_H
