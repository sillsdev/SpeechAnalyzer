/////////////////////////////////////////////////////////////////////////////
// DlgMultiChannel.h:
// Implementation of the CDlgMultiChannel
//
// Author: Kent Gorham
// copyright 2014 JAARS Inc. SIL
//
// Revision History
//
/////////////////////////////////////////////////////////////////////////////
#ifndef DLGMULTICHANNEL_H
#define DLGMULTICHANNEL_H

#include "resource.h"
#include "afxwin.h"

class CSaDoc;

class CDlgMultiChannel : public CDialog
{

    DECLARE_DYNAMIC(CDlgMultiChannel)
public:
    CDlgMultiChannel(int nChannels);
    ~CDlgMultiChannel();

    int m_nChannel;

    enum { IDD = IDD_MULTICHANNEL };

protected:
    virtual void DoDataExchange(CDataExchange * pDX);
    virtual BOOL OnInitDialog();

	CComboBox m_MultiChannelCombo;
    int m_nChannels;

    DECLARE_MESSAGE_MAP()

};

#endif
