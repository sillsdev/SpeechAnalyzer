/////////////////////////////////////////////////////////////////////////////
// DlgAutoReferenceData.cpp:
// Implementation of the CDlgAutoReferenceData
//
// Author: Kent Gorham
// Copyright 2014 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgAutoReferenceData.h"
#include "Sa_Doc.h"
#include "TextHelper.h"
#include "FileEncodingHelper.h"

using std::find;

// CDlgAutoReferenceData dialog
IMPLEMENT_DYNAMIC(CDlgAutoReferenceData, CDialog)

CDlgAutoReferenceData::CDlgAutoReferenceData(CSaDoc * pSaDoc, int numWords) :
    CDialog(CDlgAutoReferenceData::IDD, NULL),
    mLastImport(L""),
    mSaDoc(pSaDoc),
    mNumWords(numWords),
    mGlossSelected(false),
    mUsingNumbers(false),
    mUsingFirstGloss(false),
    mBeginRef(L""),
    mEndRef(L""),
    mBegin(0),
    mEnd(0),
    MIN_NUM_VALUE(0),
    MAX_NUM_VALUE(10000)
{
}

CDlgAutoReferenceData::~CDlgAutoReferenceData()
{
}

BOOL CDlgAutoReferenceData::OnInitDialog()
{

    // if the file is no longer available, clear the entry
    struct _stat buffer;
    if (_wstat(mLastImport, &buffer)!=0)
    {
        mLastImport = L"";
        mUsingNumbers = true;
    }

    CDialog::OnInitDialog();

    if (mUsingFirstGloss)
    {
        CheckDlgButton(IDC_FIRST_GLOSS_RADIO,BST_CHECKED);
    }
    else
    {
        if (mGlossSelected)
        {
            CheckDlgButton(IDC_SELECTED_GLOSS_RADIO,BST_CHECKED);
        }
        else
        {
            CheckDlgButton(IDC_FIRST_GLOSS_RADIO,BST_CHECKED);
        }
    }

    mRadioSelectedGloss.EnableWindow(mGlossSelected);

    if (mUsingNumbers)
    {
        CheckDlgButton(IDC_RADIO_NUMBERS,BST_CHECKED);
    }
    else
    {
        CheckDlgButton(IDC_RADIO_FILE,BST_CHECKED);
    }

    OnRadio();

    mSpinBegin.SetRange(0,10000);
    mSpinBegin.SetPos(mBegin);
    mSpinEnd.SetRange(1,10000);
    mSpinEnd.SetPos(mEnd);

    CRect cbSize;           // current size of combo box
    mComboBegin.GetClientRect(cbSize);
    int height = mComboBegin.GetItemHeight(-1);
    height += mComboBegin.GetItemHeight(0) * 20;
    height += GetSystemMetrics(SM_CYEDGE) * 2;
    height += GetSystemMetrics(SM_CYEDGE) * 2;
    mComboBegin.SetWindowPos(NULL,0, 0,cbSize.right, height,SWP_NOMOVE|SWP_NOZORDER);

    mComboEnd.GetClientRect(cbSize);
    height = mComboEnd.GetItemHeight(-1);
    height += mComboEnd.GetItemHeight(0) * 20;
    height += GetSystemMetrics(SM_CYEDGE) * 2;
    height += GetSystemMetrics(SM_CYEDGE) * 2;
    mComboEnd.SetWindowPos(NULL,0, 0,cbSize.right, height,SWP_NOMOVE|SWP_NOZORDER);

    UpdateData(FALSE);

    return TRUE;
}


void CDlgAutoReferenceData::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_BEGIN_SPIN, mSpinBegin);
    DDX_Control(pDX, IDC_END_SPIN, mSpinEnd);
    DDX_Control(pDX, IDC_BEGIN_EDIT, mEditBegin);
    DDX_Control(pDX, IDC_END_EDIT, mEditEnd);
    DDX_Control(pDX, IDC_BROWSE_BUTTON, mButtonBrowse);
    DDX_Control(pDX, IDC_FIRST_GLOSS_RADIO, mRadioFirstGloss);
    DDX_Control(pDX, IDC_SELECTED_GLOSS_RADIO, mRadioSelectedGloss);
    DDX_Control(pDX, IDC_RADIO_NUMBERS, mRadioNumbers);
    DDX_Control(pDX, IDC_RADIO_FILE, mRadioFile);
    DDX_Text(pDX, IDC_FILENAME, mLastImport);
    DDV_MaxChars(pDX, mLastImport, 256);
    DDX_Text(pDX, IDC_BEGIN_EDIT, mBegin);
    DDX_Text(pDX, IDC_END_EDIT, mEnd);
    DDX_Control(pDX, IDC_FILENAME, mEditFilename);
    DDX_Control(pDX, IDC_BEGIN_COMBO, mComboBegin);
    DDX_Control(pDX, IDC_END_COMBO, mComboEnd);
    DDX_CBString(pDX, IDC_BEGIN_COMBO, mBeginRef);
    DDX_CBString(pDX, IDC_END_COMBO, mEndRef);

    // load up the controls
    if (!pDX->m_bSaveAndValidate)
    {
        // display on screen
        if (!mUsingNumbers)
        {
            struct _stat buffer;
			CFileEncodingHelper feh(mLastImport);
            // update the dialog display
            if (mLastImport.GetLength()==0)
            {
                mComboBegin.ResetContent();
                mComboEnd.ResetContent();
            }
            else if (_wstat(mLastImport, &buffer)!=0)
            {
                // the file is missing or bad
                mComboBegin.ResetContent();
                mComboEnd.ResetContent();
                mComboBegin.SetCurSel(-1);
                mComboEnd.SetCurSel(-1);
            }
			else if (!feh.CheckEncoding(false)) 
			{
                // the encoding is wrong
                mComboBegin.ResetContent();
                mComboEnd.ResetContent();
                mComboBegin.SetCurSel(-1);
                mComboEnd.SetCurSel(-1);
			}
            else
            {
                CTranscriptionData td;
                CSaString temp = mLastImport;

				CFileEncodingHelper feh(temp);
				if (!feh.CheckEncoding(false)) 
				{
                    mComboBegin.ResetContent();
                    mComboEnd.ResetContent();
                    mComboBegin.SetCurSel(-1);
                    mComboEnd.SetCurSel(-1);
				} 
				else 
				{
					wistringstream stream;
					if (!feh.ConvertFileToUTF16(stream)) 
					{
						mComboBegin.ResetContent();
						mComboEnd.ResetContent();
						mComboBegin.SetCurSel(-1);
						mComboEnd.SetCurSel(-1);
					} 
					else 
					{
						if (mSaDoc->ImportTranscription(stream,FALSE,FALSE,FALSE,FALSE,FALSE,td,true,false)) 
						{
							CString ref = td.m_szPrimary;
							mComboBegin.ResetContent();
							mComboEnd.ResetContent();
							TranscriptionDataMap & tdm = td.m_TranscriptionData;
							MarkerList refs = tdm[ref];
							for (MarkerList::iterator it = refs.begin(); it!=refs.end(); it++)
							{
								mComboBegin.AddString(*it);
								mComboEnd.AddString(*it);
							}
							if (refs.size()>1)
							{
								// the table is not empty
								if (mBeginRef.GetLength()==0)
								{
									// the user didn't previously select something
									MarkerList::iterator it = refs.begin();
									it++;
									mBeginRef = *it;
									mComboBegin.SelectString(-1,mBeginRef);
								}
								else
								{
									mComboBegin.SelectString(-1,mBeginRef);
								}
							}
							else
							{
								mComboBegin.SelectString(-1,mBeginRef);
							}
							mComboEnd.SelectString(-1,mEndRef);
						}
						else
						{
							mComboBegin.ResetContent();
							mComboEnd.ResetContent();
							mComboBegin.SetCurSel(-1);
							mComboEnd.SetCurSel(-1);
						}
					}
				}
            }
        }
    }
    else
    {
        // transfer dialog controls to member variables
        mUsingNumbers = (IsDlgButtonChecked(IDC_RADIO_NUMBERS)!=0);
        mUsingFirstGloss = (IsDlgButtonChecked(IDC_FIRST_GLOSS_RADIO)!=0);

        if (!mUsingNumbers)
        {
            // the import file must be valid!
            if (mLastImport.GetLength()==0)
            {
                pDX->PrepareEditCtrl(IDC_FILENAME);
                CString msg;
                msg.FormatMessage(IDS_ERROR_FILENAME,(LPCTSTR)mLastImport);
                AfxMessageBox(msg, MB_OK|MB_ICONEXCLAMATION, 0);
                pDX->Fail();
            }

            struct _stat buffer;
            if (_wstat(mLastImport, &buffer)!=0)
            {
                pDX->PrepareEditCtrl(IDC_FILENAME);
                CString msg;
                msg.FormatMessage(IDS_ERROR_FILENAME,(LPCTSTR)mLastImport);
                AfxMessageBox(msg, MB_OK|MB_ICONEXCLAMATION, 0);
                pDX->Fail();
            }

			CFileEncodingHelper feh(mLastImport);
			if (!feh.CheckEncoding(true)) 
			{
                pDX->PrepareEditCtrl(IDC_FILENAME);
                pDX->Fail();
			}

            CTranscriptionData td;
            CSaString temp = mLastImport;

			wistringstream stream;
			if (!feh.ConvertFileToUTF16(stream)) 
			{
                pDX->PrepareEditCtrl(IDC_FILENAME);
                pDX->Fail();
			}

            if (!mSaDoc->ImportTranscription(stream,FALSE,FALSE,FALSE,FALSE,FALSE,td,true,false))
            {
                pDX->PrepareEditCtrl(IDC_FILENAME);
                CString msg;
                msg.LoadStringW(IDS_AUTO_REF_MAIN_1);
                CString msg2;
                msg2.LoadStringW(IDS_AUTO_REF_MAIN_2);
                msg.Append(msg2);
                msg2.LoadStringW(IDS_AUTO_REF_MAIN_3);
                msg.Append(msg2);
                AfxMessageBox(msg,MB_OK|MB_ICONEXCLAMATION);
                pDX->Fail();
            }

            CString ref = td.m_szPrimary;
            TranscriptionDataMap & tdm = td.m_TranscriptionData;
            MarkerList::iterator begin = find(tdm[ref].begin(),tdm[ref].end(),mBeginRef);
            MarkerList::iterator end = find(tdm[ref].begin(),tdm[ref].end(),mEndRef);
            if ((mBeginRef.GetLength()==0)||(begin==tdm[ref].end()))
            {
                pDX->PrepareEditCtrl(IDC_BEGIN_COMBO);
                AfxMessageBox(IDS_ERROR_BAD_START_REF, MB_OK|MB_ICONEXCLAMATION, 0);
                pDX->Fail();
            }
            if ((mEndRef.GetLength()==0)||(end==tdm[ref].end()))
            {
                pDX->PrepareEditCtrl(IDC_END_COMBO);
                AfxMessageBox(IDS_ERROR_BAD_END_REF, MB_OK|MB_ICONEXCLAMATION, 0);
                pDX->Fail();
            }
            int sindex = std::distance(tdm[ref].begin(), begin);
            int eindex = std::distance(tdm[ref].begin(), end);
            if (eindex<sindex)
            {
                pDX->PrepareEditCtrl(IDC_END_COMBO);
                AfxMessageBox(IDS_ERROR_BAD_START_END_REF, MB_OK|MB_ICONEXCLAMATION, 0);
                pDX->Fail();
            }
        }
        else
        {
            // same regardless of start selection
            // we will start number of first gloss in graph
            int min = MIN_NUM_VALUE;
            int max = MAX_NUM_VALUE;
            ValidateRange(pDX, IDC_BEGIN_EDIT, mBegin, min, max);

            min = mBegin;
            max = MAX_NUM_VALUE;
            ValidateRange(pDX, IDC_END_EDIT, mEnd, min, max);
        }
    }
}

void CDlgAutoReferenceData::ValidateRange(CDataExchange * pDX, UINT field, int value, int min, int max)
{
    if ((value>=min)&&(value<=max))
    {
        return;
    }

    CString param1;
    param1.Format(L"%d",min);
    CString param2;
    param2.Format(L"%d",max);
    CString msg;
    AfxFormatString2(msg,IDS_ERROR_RANGE,param1,param2);
    pDX->PrepareEditCtrl(field);
    AfxMessageBox(msg, MB_OK|MB_ICONEXCLAMATION,0);
    pDX->Fail();
}

BEGIN_MESSAGE_MAP(CDlgAutoReferenceData, CDialog)
    ON_COMMAND(IDC_RADIO_NUMBERS, &CDlgAutoReferenceData::OnRadio)
    ON_COMMAND(IDC_RADIO_FILE, &CDlgAutoReferenceData::OnRadio)
    ON_BN_CLICKED(IDC_BROWSE_BUTTON, &CDlgAutoReferenceData::OnClickedBrowseButton)
    ON_NOTIFY(UDN_DELTAPOS, IDC_BEGIN_SPIN, &CDlgAutoReferenceData::OnDeltaposBeginSpin)
    ON_NOTIFY(UDN_DELTAPOS, IDC_END_SPIN, &CDlgAutoReferenceData::OnDeltaposEndSpin)
    ON_EN_KILLFOCUS(IDC_FILENAME, &CDlgAutoReferenceData::OnKillfocusFilename)
END_MESSAGE_MAP()

void CDlgAutoReferenceData::OnRadio()
{

    mUsingNumbers  = (IsDlgButtonChecked(IDC_RADIO_NUMBERS)!=0);
    if (mUsingNumbers)
    {
        mEditBegin.EnableWindow(TRUE);
        mSpinBegin.EnableWindow(TRUE);
        mEditEnd.EnableWindow(TRUE);
        mSpinEnd.EnableWindow(TRUE);
        mEditBegin.ShowWindow(SW_SHOW);
        mSpinBegin.ShowWindow(SW_SHOW);
        mEditEnd.ShowWindow(SW_SHOW);
        mSpinEnd.ShowWindow(SW_SHOW);

        mComboBegin.EnableWindow(FALSE);
        mComboEnd.EnableWindow(FALSE);
        mComboBegin.ShowWindow(SW_HIDE);
        mComboEnd.ShowWindow(SW_HIDE);

        mEditFilename.EnableWindow(FALSE);
        mButtonBrowse.EnableWindow(FALSE);
    }
    else
    {
        mEditBegin.EnableWindow(FALSE);
        mSpinBegin.EnableWindow(FALSE);
        mEditEnd.EnableWindow(FALSE);
        mSpinEnd.EnableWindow(FALSE);
        mEditBegin.ShowWindow(SW_HIDE);
        mSpinBegin.ShowWindow(SW_HIDE);
        mEditEnd.ShowWindow(SW_HIDE);
        mSpinEnd.ShowWindow(SW_HIDE);

        mComboBegin.EnableWindow(TRUE);
        mComboEnd.EnableWindow(TRUE);
        mComboBegin.ShowWindow(SW_SHOW);
        mComboEnd.ShowWindow(SW_SHOW);

        mEditFilename.EnableWindow(TRUE);
        mButtonBrowse.EnableWindow(TRUE);

        if (mLastImport.GetLength()>0)
        {
            UpdateData(FALSE);
        }
    }
}

void CDlgAutoReferenceData::OnDeltaposBeginSpin(NMHDR * pNMHDR, LRESULT * pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    mBegin = GetDlgItemInt(IDC_BEGIN_EDIT, NULL, TRUE);
    mBegin += pNMUpDown->iDelta;
    if (mBegin>MAX_NUM_VALUE)
    {
        mBegin = MAX_NUM_VALUE;
    }
    if (mBegin<0)
    {
        mBegin = 0;
    }
    SetDlgItemInt(IDC_BEGIN_EDIT, mBegin, TRUE);

    mEnd = GetDlgItemInt(IDC_END_EDIT, NULL, TRUE);
    if (mBegin>=mEnd)
    {
        mEnd=mBegin+1;
    }
    SetDlgItemInt(IDC_END_EDIT, mEnd, TRUE);

    *pResult = 0;
}

void CDlgAutoReferenceData::OnDeltaposEndSpin(NMHDR * pNMHDR, LRESULT * pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    mEnd = GetDlgItemInt(IDC_END_EDIT, NULL, TRUE);
    mEnd += pNMUpDown->iDelta;
    if (mEnd>10000)
    {
        mEnd = 10000;
    }
    if (mEnd<1)
    {
        mEnd = 1;
    }
    SetDlgItemInt(IDC_END_EDIT, mEnd, TRUE);

    mBegin = GetDlgItemInt(IDC_BEGIN_EDIT, NULL, TRUE);
    if (mEnd<=mBegin)
    {
        mBegin=mEnd-1;
    }
    SetDlgItemInt(IDC_BEGIN_EDIT, mBegin, TRUE);

    *pResult = 0;
}

void CDlgAutoReferenceData::OnClickedBrowseButton()
{
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Standard Format (*.sfm)(*.txt)|*.sfm;*.txt| All Files (*.*) |*.*||"), this);
    if (dlg.DoModal() != IDOK)
    {
        return;
    }
    CString temp = dlg.GetPathName();
	CFileEncodingHelper feh(temp);
	if (!feh.CheckEncoding(true)) 
	{
		return;
	}
	mLastImport = temp;
    UpdateData(FALSE);
}

void CDlgAutoReferenceData::OnKillfocusFilename()
{
    GetDlgItemTextW(IDC_FILENAME,mLastImport);
    UpdateData(FALSE);
}
