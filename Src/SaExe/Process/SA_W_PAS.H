/////////////////////////////////////////////////////////////////////////////
// sa_w_pas.h:
// Interface of the CProcessWbHighpass
//          CProcessWbLowpass
//          CProcessWbBandpass classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_PAS_H
#define _SA_W_PAS_H

#include "Butterworth.h"

//###########################################################################
// CProcessWbHighpass data processing

class CProcessWbHighpass : public CButterworth
{
// Construction/destruction/creation
public:
  CProcessWbHighpass();
  virtual ~CProcessWbHighpass();

  // Attributes
private:
  UINT m_nCutoff; // cutoff frequency
  UINT m_nOrder;
  BOOL m_bFilterFilter;

  // Operations
public:
  long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);
  virtual int PropertiesDialog(); // calls the properties dialog for this process
  virtual void WriteProperties(Object_ostream& obs);
  virtual BOOL bReadProperties(Object_istream& obs);
};

//###########################################################################
// CProcessWbLowpass data processing

class CProcessWbLowpass : public CButterworth
{
// Construction/destruction/creation
public:
  CProcessWbLowpass();
  virtual ~CProcessWbLowpass();

  // Attributes
private:
  UINT m_nCutoff; // cutoff frequency
  UINT m_nOrder;
  BOOL m_bFilterFilter;

  // Operations
public:
  long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);
  virtual int PropertiesDialog(); // calls the properties dialog for this process
  virtual void WriteProperties(Object_ostream& obs);
  virtual BOOL bReadProperties(Object_istream& obs);
};

//###########################################################################
// CProcessWbBandpass data processing

class CProcessWbBandpass : public CButterworth
{
// Construction/destruction/creation
public:
  CProcessWbBandpass();
  virtual ~CProcessWbBandpass();

  // Attributes
private:
  UINT m_nUpperCutoff; // upper cutoff frequency
  UINT m_nLowerCutoff; // lower cutoff frequency
  UINT m_nOrder;
  BOOL m_bFilterFilter;

  // Operations
public:
  long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);
  virtual int PropertiesDialog(); // calls the properties dialog for this process
  virtual void WriteProperties(Object_ostream& obs);
  virtual BOOL bReadProperties(Object_istream& obs);
};

#endif //_SA_W_PAS_H
