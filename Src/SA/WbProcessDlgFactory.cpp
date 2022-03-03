#include "stdafx.h"
#include "WbProcessDlgFactory.h"
#include "sa_w_dlg.h"
#include "wbprocess.h"

int CWbProcessDlgFactory::showDialog(CWbProcess* process) {

    switch (process->GetDialogType()) {
    case WbEquation:
    {
        CProcessWbEquation* pProcess = (CProcessWbEquation*)process;
        CWbDlgFilterEquation dlgEqu;
        dlgEqu.m_szEquation = pProcess->GetEquation();
        if (dlgEqu.DoModal() == IDOK) {
            pProcess->SetEquation(_to_utf8(dlgEqu.m_szEquation).c_str());
            pProcess->SetDataInvalid();
            return IDOK;
        }
        return IDCANCEL;
    }
    case WbEcho:
    {
        CProcessWbEcho* pProcess = (CProcessWbEcho*)process;
        CWbDlgFilterReverb dlgReverb;
        dlgReverb.m_nDelay = pProcess->GetDelay();
        dlgReverb.m_nGain = pProcess->GetGain();
        if (dlgReverb.DoModal() == IDOK) {
            pProcess->SetDelay(dlgReverb.m_nDelay);
            pProcess->SetGain(dlgReverb.m_nGain);
            pProcess->SetDataInvalid();
            return IDOK;
        }
        return IDCANCEL;
    }
    case WbReverb:
    {
        CProcessWbReverb* pProcess = (CProcessWbReverb*)process;
        CWbDlgFilterReverb dlgReverb;
        dlgReverb.m_nDelay = pProcess->GetDelay();
        dlgReverb.m_nGain = pProcess->GetGain();
        dlgReverb.m_bEcho = FALSE;
        if (dlgReverb.DoModal() == IDOK) {
            pProcess->SetDelay(dlgReverb.m_nDelay);
            pProcess->SetGain(dlgReverb.m_nGain);
            pProcess->SetDataInvalid();
            return IDOK;
        }
        return IDCANCEL;
    }
    case WbHiPass:
    {
        CProcessWbHighpass* pProcess = (CProcessWbHighpass*)process;
        CWbDlgFilterPass dlgPass;
        dlgPass.m_bBandPass = FALSE;
        dlgPass.m_bLoPass = FALSE;
        dlgPass.m_nLowerFreq = pProcess->GetCutoff();
        dlgPass.m_nOrder = pProcess->GetOrder();
        dlgPass.m_bFilterFilter = pProcess->GetFilter();
        if (dlgPass.DoModal() == IDOK) {
            pProcess->SetCutoff(dlgPass.m_nLowerFreq);
            pProcess->SetOrder(dlgPass.m_nOrder);
            pProcess->SetFilter(dlgPass.m_bFilterFilter);
            pProcess->SetDataInvalid();
            return IDOK;
        }
        return IDCANCEL;
    }
    case WbLoPass:
    {
        CProcessWbLowpass* pProcess = (CProcessWbLowpass*)process;
        CWbDlgFilterPass dlgPass;
        dlgPass.m_bBandPass = FALSE;
        dlgPass.m_nUpperFreq = pProcess->GetCutoff();
        dlgPass.m_nOrder = pProcess->GetOrder();
        dlgPass.m_bFilterFilter = pProcess->GetFilter();
        if (dlgPass.DoModal() == IDOK) {
            pProcess->SetCutoff(dlgPass.m_nUpperFreq);
            pProcess->SetOrder(dlgPass.m_nOrder);
            pProcess->SetFilter(dlgPass.m_bFilterFilter);
            pProcess->SetDataInvalid();
            return IDOK;
        }
        return IDCANCEL;
    }
    case WbBandPass:
    {
        CProcessWbBandpass* pProcess = (CProcessWbBandpass*)process;
        CWbDlgFilterPass dlgPass;
        dlgPass.m_nUpperFreq = pProcess->GetUpperCutoff();
        dlgPass.m_nLowerFreq = pProcess->GetLowerCutoff();
        dlgPass.m_nOrder = pProcess->GetOrder();
        dlgPass.m_bFilterFilter = pProcess->GetFilter();
        if (dlgPass.DoModal() == IDOK) {
            pProcess->SetUpperCutoff(dlgPass.m_nUpperFreq);
            pProcess->SetLowerCutoff(dlgPass.m_nLowerFreq);
            pProcess->SetOrder(dlgPass.m_nOrder);
            pProcess->SetFilter(dlgPass.m_bFilterFilter);
            pProcess->SetDataInvalid();
            return IDOK;
        }
        return IDCANCEL;
    }
    default:
        return AfxMessageBox(IDS_DEFAULTPROPERTIES, MB_OK, 0);
    }
};
