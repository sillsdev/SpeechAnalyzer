/////////////////////////////////////////////////////////////////////////////
// sa_w_adj.cpp:
// Implementation of the CProcessAdjust
//
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_proc.h"

#include "isa_doc.h"
#include "sa_p_doc.h"
#include "sa_w_adj.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessAdjust

/////////////////////////////////////////////////////////////////////////////
// CProcessAdjust construction/destruction/creation

/***************************************************************************/
// CProcessAdjust::CProcessAdjust Constructor
/***************************************************************************/
CProcessAdjust::CProcessAdjust(ISaDoc *pDoc)
{
	m_pDoc = pDoc;
	m_bInvert = FALSE;
	m_bZero = FALSE;
	m_bNormalize = FALSE;
}

/***************************************************************************/
// CProcessAdjust::~CProcessAdjust Destructor
/***************************************************************************/
CProcessAdjust::~CProcessAdjust()
{
}

/////////////////////////////////////////////////////////////////////////////
// CProcessAdjust helper functions

/***************************************************************************/
// CProcessAdjust::SetInvert
/***************************************************************************/
void CProcessAdjust::SetInvert(BOOL bNew)
{
	if (bNew != m_bInvert)
	{
		m_bInvert = bNew;
		SetDataInvalid();
	} 
}

/***************************************************************************/
// CProcessAdjust::SetZero
/***************************************************************************/
void CProcessAdjust::SetZero(BOOL bNew)
{
	if (bNew != m_bZero)
	{
		m_bZero = bNew;
		SetDataInvalid();
	} 
}

/***************************************************************************/
// CProcessAdjust::SetNormalize
/***************************************************************************/
void CProcessAdjust::SetNormalize(BOOL bNew)
{
	if (bNew != m_bNormalize)
	{
		m_bNormalize = bNew;
		SetDataInvalid();
	} 
}


/***************************************************************************/
// CProcessAdjust::Process Processing new raw data with a highpass function
// The processed change data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessAdjust::Process(void* pCaller, ISaDoc* pDoc, int nProgress, int nLevel)
{
	if (m_bInvert || m_bZero || m_bNormalize)
	{
		CDataProcess* pLowerProcess = pDoc->GetUnprocessed();

		double dScale = m_bInvert ? -1 : 1;

		if(m_bNormalize)
		{
			CButterworth cNormalize;

			cNormalize.SetSourceProcess(pLowerProcess);
			cNormalize.SetFilterFilter(TRUE);
			cNormalize.HighPass( m_bZero ? 3 : 0, 40., 1.);
			cNormalize.Process(pCaller, pDoc, nProgress, nLevel);

			int nMax = cNormalize.GetMinValue();
			if (cNormalize.GetMaxValue() > nMax)
				nMax = cNormalize.GetMaxValue();

			dScale *= 1.0/2 * (1 << pDoc->GetFmtParm()->wBitsPerSample) / (nMax);
		}

		SetSourceProcess(pLowerProcess);
		SetFilterFilter(TRUE);
		HighPass( m_bZero ? 3 : 0, 40., dScale);
		return CButterworth::Process(pCaller, pDoc, nProgress, nLevel);
	}
	else
	{
		SetDataReady();
		return MAKELONG(nLevel, nProgress);    
	}
}

HPSTR CProcessAdjust::GetProcessedWaveData(DWORD dwOffset, BOOL bBlockBegin)
{
	if (ProcessImpersonate() == NULL)
	{
		return CDataProcess::GetProcessedWaveData(dwOffset, bBlockBegin);
	}
	else
		return ProcessImpersonate()->GetProcessedWaveData(dwOffset, bBlockBegin);
}

void* CProcessAdjust::GetProcessedDataBlock(DWORD dwOffset, size_t sObjectSize, BOOL bReverse)
{
	if (ProcessImpersonate() == NULL)
	{
		return CDataProcess::GetProcessedDataBlock(dwOffset, sObjectSize, bReverse);
	}
	else
		return ProcessImpersonate()->GetProcessedDataBlock(dwOffset, sObjectSize, bReverse);
}


CDataProcess* CProcessAdjust::ProcessImpersonate() const
{
	if (m_bInvert || m_bZero || m_bNormalize)
		return NULL;
	else
		return m_pDoc->GetUnprocessed();
}
