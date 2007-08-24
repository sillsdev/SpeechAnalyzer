#if !defined(AFX_SA_P_3DPITCH_H__DEDA9BC4_3A24_11D5_9FE4_00E098784E13__INCLUDED_)
#define AFX_SA_P_3DPITCH_H__DEDA9BC4_3A24_11D5_9FE4_00E098784E13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// sa_p_3dPitch.h : header file
//

#include "Sa_proc.h"


/////////////////////////////////////////////////////////////////////////////
// CProcess3dPitch command target

class CProcess3dPitch : public CDataProcess
{
public:
	CProcess3dPitch();           // protected constructor used by dynamic creation
	virtual ~CProcess3dPitch();

// Attributes
public:

// Operations
public:

    void SetSourceProcess(CDataProcess * pSourceProcess);
    virtual long Process(void* pCaller, ISaDoc*, int nProgress = 0, int nLevel = 1);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcess3dPitch)
	//}}AFX_VIRTUAL

// Implementation
private:
  double m_dFilterUpperFrequency;
  double m_dFilterLowerFrequency;
  int m_nFilterOrder;
  CDataProcess * m_pSourceProcess;

protected:

	// Generated message map functions
	//{{AFX_MSG(CProcess3dPitch)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SA_P_3DPITCH_H__DEDA9BC4_3A24_11D5_9FE4_00E098784E13__INCLUDED_)
