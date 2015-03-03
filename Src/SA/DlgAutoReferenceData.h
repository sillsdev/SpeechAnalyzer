/////////////////////////////////////////////////////////////////////////////
// DlgAutoReferenceData.h:
// Interface of the CDlgAutoReferenceData
//
// Author: Kent Gorham
// Copyright 2014 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////
#ifndef DLGAUTOREFERENCEDATA_H
#define DLGAUTOREFERENCEDATA_H

#include "sa_dlwnd.h"
#include "SaString.h"
#include "TranscriptionData.h"
#include "sa_doc.h"

class CDlgAutoReferenceData : public CDialog {
    DECLARE_DYNAMIC(CDlgAutoReferenceData)

public:
    CDlgAutoReferenceData(CSaDoc * pSaDoc, int numWords);
    virtual ~CDlgAutoReferenceData();

    enum { IDD = IDD_AUTO_REFERENCE_DATA };

protected:
    virtual void DoDataExchange(CDataExchange * pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnRadio();
    afx_msg void OnClickedBrowseButton();
    afx_msg void OnDeltaposBeginSpin(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnDeltaposEndSpin(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnKillfocusFilename();

private:
    CSpinButtonCtrl mSpinBegin;
    CSpinButtonCtrl mSpinEnd;
    CButton mButtonBrowse;
    CEdit mEditBegin;
    CEdit mEditEnd;
    CEdit mEditFilename;
    CComboBox mComboBegin;
    CComboBox mComboEnd;
    CButton mRadioFirstGloss;
    CButton mRadioSelectedGloss;
    CButton mRadioNumbers;
    CButton mRadioFile;

    class CSaDoc * mSaDoc;

public:
    // radio selection
    bool mUsingFirstGloss;
    bool mUsingNumbers;
    CString mLastImport;
    bool mGlossSelected;

    // for word list selections
    CString mBeginRef;
    CString mEndRef;

    // for numbering
    int mBegin;
    int mEnd;

    const int MIN_NUM_VALUE;
    const int MAX_NUM_VALUE;

private:
    void ValidateRange(CDataExchange * pDX, UINT field, int value, int min, int max);

    int mNumWords;

};

#endif
