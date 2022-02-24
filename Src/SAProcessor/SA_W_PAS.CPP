/////////////////////////////////////////////////////////////////////////////
// sa_w_pas.cpp:
// Implementation of the CProcessWbHighpass
//                       CProcessWbLowpass
//                       CProcessWbBandpass classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "sa_process.h"
#include "sa_w_pas.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessWbHighpass
// Workbench process class to calculate new raw data with a highpass function.

CProcessWbHighpass::CProcessWbHighpass(Context context) : CProcessButterworth(context,WbHiPass) {
    m_nCutoff = 150;
    m_nOrder = 3;
    m_bFilterFilter = TRUE;
}

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
long CProcessWbHighpass::Process(void * pCaller, int nProgress, int nLevel) {
    // check if nested workbench processes
    CWbProcess * pLowerProcess = NULL;
    if (nLevel < MAX_FILTER_NUMBER) {
        pLowerProcess = main.GetWbProcess(model.GetWbProcess() - 1, nLevel);
    }
    SetSourceProcess(pLowerProcess);
    SetFilterFilter(m_bFilterFilter!=FALSE);
    HighPass(m_nOrder, m_nCutoff);
    return CProcessButterworth::Process(pCaller, nProgress, nLevel);
}

static LPCSTR psz_highpass = "highpass";
static LPCSTR psz_lowpass = "lowpass";
static LPCSTR psz_bandpass = "bandpass";
static LPCSTR psz_upper = "uppercutoff";
static LPCSTR psz_lower = "lowercutoff";
static LPCSTR psz_order = "filterorder";
static LPCSTR psz_filter = "filterfilter";

/***************************************************************************/
// CProcessWbHighpass::WriteProperties Write highpass properties
/***************************************************************************/
void CProcessWbHighpass::WriteProperties(CObjectOStream & obs) {
    obs.WriteBeginMarker(psz_highpass);
    obs.WriteUInt(psz_lower, m_nCutoff);
    obs.WriteUInt(psz_order, m_nOrder);
    obs.WriteBool(psz_filter, m_bFilterFilter);
    obs.WriteEndMarker(psz_highpass);
}

/***************************************************************************/
// CProcessWbHighpass::ReadProperties Read highpass properties
/***************************************************************************/
BOOL CProcessWbHighpass::ReadProperties(CObjectIStream & obs) {
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

CProcessWbLowpass::CProcessWbLowpass(Context context) : CProcessButterworth(context,WbLoPass) {
    m_nCutoff = 3000;
    m_nOrder = 3;
    m_bFilterFilter = TRUE;
}

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
long CProcessWbLowpass::Process(void * pCaller, int nProgress, int nLevel) {
    // check if nested workbench processes
    CProcess * pLowerProcess = NULL;
    if (nLevel < MAX_FILTER_NUMBER) {
        pLowerProcess = ((CWbProcess *)main.GetWbProcess(model.GetWbProcess() - 1, nLevel));
    }
    SetSourceProcess(pLowerProcess);
    SetFilterFilter(m_bFilterFilter);
    LowPass(m_nOrder, m_nCutoff);
    return CProcessButterworth::Process(pCaller, nProgress, nLevel);
}

/***************************************************************************/
// CProcessWbLowpass::WriteProperties Write lowpass properties
/***************************************************************************/
void CProcessWbLowpass::WriteProperties(CObjectOStream & obs) {
    obs.WriteBeginMarker(psz_lowpass);
    obs.WriteUInt(psz_upper, m_nCutoff);
    obs.WriteUInt(psz_order, m_nOrder);
    obs.WriteBool(psz_filter, m_bFilterFilter);
    obs.WriteEndMarker(psz_lowpass);
}

/***************************************************************************/
// CProcessWbLowpass::ReadProperties Read lowpass properties
/***************************************************************************/
BOOL CProcessWbLowpass::ReadProperties(CObjectIStream & obs) {
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

/***************************************************************************/
// CProcessWbBandpass::CProcessWbBandpass Constructor
/***************************************************************************/
CProcessWbBandpass::CProcessWbBandpass(Context context) : CProcessButterworth(context,WbBandPass) {
    m_nUpperCutoff = 3000;
    m_nLowerCutoff = 150;
    m_nOrder = 3;
    m_bFilterFilter = TRUE;
}

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
long CProcessWbBandpass::Process(void * pCaller, int nProgress, int nLevel) {
    // check if nested workbench processes
    CWbProcess * pLowerProcess = NULL;
    if (nLevel < MAX_FILTER_NUMBER) {
        pLowerProcess = main.GetWbProcess(model.GetWbProcess() - 1, nLevel);
    }
    SetSourceProcess(pLowerProcess);
    SetFilterFilter(m_bFilterFilter);
    BandPass(m_nOrder, (m_nUpperCutoff + m_nLowerCutoff)/2,m_nUpperCutoff - m_nLowerCutoff);
    return CProcessButterworth::Process(pCaller, nProgress, nLevel);
}

/***************************************************************************/
// CProcessWbBandpass::WriteProperties Write bandpass properties
/***************************************************************************/
void CProcessWbBandpass::WriteProperties(CObjectOStream & obs) {
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
BOOL CProcessWbBandpass::ReadProperties(CObjectIStream & obs) {
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


