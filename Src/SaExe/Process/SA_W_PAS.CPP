/////////////////////////////////////////////////////////////////////////////
// sa_w_pas.cpp:
// Implementation of the CProcessWbHighpass
//                       CProcessWbLowpass
//                       CProcessWbBandpass classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"

#include "resource.h"
#include "mainfrm.h"
#include "isa_doc.h"
#include "sa_w_pas.h"
#include "sa_w_dlg.h"
#include "settings\obstream.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessWbHighpass
// Workbench process class to calculate new raw data with a highpass function.

/////////////////////////////////////////////////////////////////////////////
// CProcessWbHighpass construction/destruction/creation

/***************************************************************************/
// CProcessWbHighpass::CProcessWbHighpass Constructor
/***************************************************************************/
CProcessWbHighpass::CProcessWbHighpass() {
    m_nCutoff = 150;
    m_nOrder = 3;
    m_bFilterFilter = TRUE;
}

/***************************************************************************/
// CProcessWbHighpass::~CProcessWbHighpass Destructor
/***************************************************************************/
CProcessWbHighpass::~CProcessWbHighpass() {
}

/////////////////////////////////////////////////////////////////////////////
// CProcessWbHighpass helper functions

/***************************************************************************/
// CProcessWbHighpass::Process Processing new raw data with a highpass function
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
long CProcessWbHighpass::Process(void * pCaller, ISaDoc * pDoc, int nProgress, int nLevel) {
    // check if nested workbench processes
    CProcess * pLowerProcess = NULL;
    if (nLevel < MAX_FILTER_NUMBER) {
        CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
        pLowerProcess = ((CProcess *)pMain->GetWbProcess(pDoc->GetWbProcess() - 1, nLevel));
    }
    SetSourceProcess(pLowerProcess);
    SetFilterFilter(m_bFilterFilter);
    HighPass(m_nOrder, m_nCutoff);
    return CButterworth::Process(pCaller, pDoc, nProgress, nLevel);
}

/***************************************************************************/
// CProcessWbHighpass::PropertiesDialog Calls the properties dialog for this process
/***************************************************************************/
int CProcessWbHighpass::PropertiesDialog() {
    CWbDlgFilterPass dlgPass;
    dlgPass.m_bBandPass = FALSE;
    dlgPass.m_bLoPass = FALSE;
    dlgPass.m_nLowerFreq = m_nCutoff;
    dlgPass.m_nOrder = m_nOrder;
    dlgPass.m_bFilterFilter = m_bFilterFilter;
    if (dlgPass.DoModal() == IDOK) {
        m_nCutoff = dlgPass.m_nLowerFreq;
        m_nOrder = dlgPass.m_nOrder;
        m_bFilterFilter = dlgPass.m_bFilterFilter;
        SetDataInvalid();
        return IDOK;
    }
    return IDCANCEL;
}

static const char * psz_highpass = "highpass";
static const char * psz_lowpass = "lowpass";
static const char * psz_bandpass = "bandpass";
static const char * psz_upper = "uppercutoff";
static const char * psz_lower = "lowercutoff";
static const char * psz_order = "filterorder";
static const char * psz_filter = "filterfilter";

/***************************************************************************/
// CProcessWbHighpass::WriteProperties Write highpass properties
/***************************************************************************/
void CProcessWbHighpass::WriteProperties(Object_ostream & obs) {
    obs.WriteBeginMarker(psz_highpass);
    obs.WriteUInt(psz_lower, m_nCutoff);
    obs.WriteUInt(psz_order, m_nOrder);
    obs.WriteBool(psz_filter, m_bFilterFilter);
    obs.WriteEndMarker(psz_highpass);
}

/***************************************************************************/
// CProcessWbHighpass::ReadProperties Read highpass properties
/***************************************************************************/
BOOL CProcessWbHighpass::ReadProperties(Object_istream & obs) {
    UINT nCutoff = m_nCutoff;
    UINT nOrder = m_nOrder;
    BOOL bFilter = m_bFilterFilter;
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_highpass)) {
        return FALSE;
    }
    while (!obs.bAtEnd()) {
        if (obs.bReadUInt(psz_lower, m_nCutoff));
        else if (obs.bReadUInt(psz_order, m_nOrder));
        else if (obs.bReadBool(psz_filter, m_bFilterFilter));
        else if (obs.bEnd(psz_highpass)) {
            break;
        }
    }
    // check if properties changed
    if (bFilter != m_bFilterFilter || nCutoff != m_nCutoff || nOrder != m_nOrder) {
        SetDataInvalid();
    }
    return TRUE;
}

//###########################################################################
// CProcessWbLowpass
// Workbench process class to calculate new raw data with a lowpass function.

/////////////////////////////////////////////////////////////////////////////
// CProcessWbLowpass construction/destruction/creation

/***************************************************************************/
// CProcessWbLowpass::CProcessWbLowpass Constructor
/***************************************************************************/
CProcessWbLowpass::CProcessWbLowpass() {
    m_nCutoff = 3000;
    m_nOrder = 3;
    m_bFilterFilter = TRUE;
}

/***************************************************************************/
// CProcessWbLowpass::~CProcessWbLowpass Destructor
/***************************************************************************/
CProcessWbLowpass::~CProcessWbLowpass() {
}

/////////////////////////////////////////////////////////////////////////////
// CProcessWbLowpass helper functions

/***************************************************************************/
// CProcessWbLowpass::Process Processing new raw data with a lowpass function
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
long CProcessWbLowpass::Process(void * pCaller, ISaDoc * pDoc, int nProgress,
                                int nLevel) {
    // check if nested workbench processes
    CProcess * pLowerProcess = NULL;
    if (nLevel < MAX_FILTER_NUMBER) {
        CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
        pLowerProcess = ((CProcess *)pMain->GetWbProcess(pDoc->GetWbProcess() - 1, nLevel));
    }
    SetSourceProcess(pLowerProcess);
    SetFilterFilter(m_bFilterFilter);
    LowPass(m_nOrder, m_nCutoff);
    return CButterworth::Process(pCaller, pDoc, nProgress, nLevel);
}

/***************************************************************************/
// CProcessWbLowpass::PropertiesDialog Calls the properties dialog for this process
/***************************************************************************/
int CProcessWbLowpass::PropertiesDialog() {
    CWbDlgFilterPass dlgPass;
    dlgPass.m_bBandPass = FALSE;
    dlgPass.m_nUpperFreq = m_nCutoff;
    dlgPass.m_nOrder = m_nOrder;
    dlgPass.m_bFilterFilter = m_bFilterFilter;
    if (dlgPass.DoModal() == IDOK) {
        m_nCutoff = dlgPass.m_nUpperFreq;
        m_nOrder = dlgPass.m_nOrder;
        m_bFilterFilter = dlgPass.m_bFilterFilter;
        SetDataInvalid();
        return IDOK;
    }
    return IDCANCEL;
}

/***************************************************************************/
// CProcessWbLowpass::WriteProperties Write lowpass properties
/***************************************************************************/
void CProcessWbLowpass::WriteProperties(Object_ostream & obs) {
    obs.WriteBeginMarker(psz_lowpass);
    obs.WriteUInt(psz_upper, m_nCutoff);
    obs.WriteUInt(psz_order, m_nOrder);
    obs.WriteBool(psz_filter, m_bFilterFilter);
    obs.WriteEndMarker(psz_lowpass);
}

/***************************************************************************/
// CProcessWbLowpass::ReadProperties Read lowpass properties
/***************************************************************************/
BOOL CProcessWbLowpass::ReadProperties(Object_istream & obs) {
    UINT nCutoff = m_nCutoff;
    UINT nOrder = m_nOrder;
    BOOL bFilter = m_bFilterFilter;
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_lowpass)) {
        return FALSE;
    }
    while (!obs.bAtEnd()) {
        if (obs.bReadUInt(psz_upper, m_nCutoff));
        else if (obs.bReadUInt(psz_order, m_nOrder));
        else if (obs.bReadBool(psz_filter, m_bFilterFilter));
        else if (obs.bEnd(psz_lowpass)) {
            break;
        }
    }
    // check if properties changed
    if (bFilter != m_bFilterFilter || nCutoff != m_nCutoff || nOrder != m_nOrder) {
        SetDataInvalid();
    }
    return TRUE;
}

//###########################################################################
// CProcessWbBandpass
// Workbench process class to calculate new raw data with a bandpass function.

/////////////////////////////////////////////////////////////////////////////
// CProcessWbBandpass construction/destruction/creation

/***************************************************************************/
// CProcessWbBandpass::CProcessWbBandpass Constructor
/***************************************************************************/
CProcessWbBandpass::CProcessWbBandpass() {
    m_nUpperCutoff = 3000;
    m_nLowerCutoff = 150;
    m_nOrder = 3;
    m_bFilterFilter = TRUE;
}

/***************************************************************************/
// CProcessWbBandpass::~CProcessWbBandpass Destructor
/***************************************************************************/
CProcessWbBandpass::~CProcessWbBandpass() {
}

/////////////////////////////////////////////////////////////////////////////
// CProcessWbBandpass helper functions

/***************************************************************************/
// CProcessWbBandpass::Process Processing new raw data with a bandpass function
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
long CProcessWbBandpass::Process(void * pCaller, ISaDoc * pDoc, int nProgress,
                                 int nLevel) {
    // check if nested workbench processes
    CProcess * pLowerProcess = NULL;
    if (nLevel < MAX_FILTER_NUMBER) {
        CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
        pLowerProcess = ((CProcess *)pMain->GetWbProcess(pDoc->GetWbProcess() - 1, nLevel));
    }
    SetSourceProcess(pLowerProcess);
    SetFilterFilter(m_bFilterFilter);
    BandPass(m_nOrder, (m_nUpperCutoff + m_nLowerCutoff)/2,m_nUpperCutoff - m_nLowerCutoff);
    return CButterworth::Process(pCaller, pDoc, nProgress, nLevel);
}

/***************************************************************************/
// CProcessWbBandpass::PropertiesDialog Calls the properties dialog for this process
/***************************************************************************/
int CProcessWbBandpass::PropertiesDialog() {
    CWbDlgFilterPass dlgPass;
    dlgPass.m_nUpperFreq = m_nUpperCutoff;
    dlgPass.m_nLowerFreq = m_nLowerCutoff;
    dlgPass.m_nOrder = m_nOrder;
    dlgPass.m_bFilterFilter = m_bFilterFilter;
    if (dlgPass.DoModal() == IDOK) {
        m_nUpperCutoff = dlgPass.m_nUpperFreq;
        m_nLowerCutoff = dlgPass.m_nLowerFreq;
        m_nOrder = dlgPass.m_nOrder;
        m_bFilterFilter = dlgPass.m_bFilterFilter;
        SetDataInvalid();
        return IDOK;
    }
    return IDCANCEL;
}

/***************************************************************************/
// CProcessWbBandpass::WriteProperties Write bandpass properties
/***************************************************************************/
void CProcessWbBandpass::WriteProperties(Object_ostream & obs) {
    obs.WriteBeginMarker(psz_bandpass);
    obs.WriteUInt(psz_upper, m_nUpperCutoff);
    obs.WriteUInt(psz_lower, m_nLowerCutoff);
    obs.WriteUInt(psz_order, m_nOrder);
    obs.WriteBool(psz_filter, m_bFilterFilter);
    obs.WriteEndMarker(psz_bandpass);
}

/***************************************************************************/
// CProcessWbBandpass::ReadProperties Read bandpass properties
/***************************************************************************/
BOOL CProcessWbBandpass::ReadProperties(Object_istream & obs) {
    UINT nUpperCutoff = m_nUpperCutoff;
    UINT nLowerCutoff = m_nLowerCutoff;
    UINT nOrder = m_nOrder;
    BOOL bFilter = m_bFilterFilter;
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_bandpass)) {
        return FALSE;
    }
    while (!obs.bAtEnd()) {
        if (obs.bReadUInt(psz_upper, m_nUpperCutoff));
        else if (obs.bReadUInt(psz_lower, m_nLowerCutoff));
        else if (obs.bReadUInt(psz_order, m_nOrder));
        else if (obs.bReadBool(psz_filter, m_bFilterFilter));
        else if (obs.bEnd(psz_bandpass)) {
            break;
        }
    }
    // check if properties changed
    if (bFilter != m_bFilterFilter || nUpperCutoff != m_nUpperCutoff || nLowerCutoff != m_nLowerCutoff || nOrder != m_nOrder) {
        SetDataInvalid();
    }
    return TRUE;
}


