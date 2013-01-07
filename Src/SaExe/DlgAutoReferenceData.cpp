/////////////////////////////////////////////////////////////////////////////
// DlgAutoReferenceData.cpp:
// Implementation of the CDlgAutoReferenceData
//
// Author: Kent Gorham
// Copyright 2012 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgAutoReferenceData.h"
#include "Sa_Doc.h"

using std::find;

// CDlgAutoReferenceData dialog
IMPLEMENT_DYNAMIC(CDlgAutoReferenceData, CDialog)

CDlgAutoReferenceData::CDlgAutoReferenceData() : 
CDialog(CDlgAutoReferenceData::IDD, NULL),
mBegin(0),
mEnd(0),
mSize(1),
mLastImport(L""),
mSaDoc(NULL),
mGlossSelected(false),
mUsingNumbers(true),
mUsingFirstGloss(true),
mBeginRef(L""),
mEndRef(L"")
{
}

CDlgAutoReferenceData::~CDlgAutoReferenceData() 
{
}

void CDlgAutoReferenceData::Init( CSaDoc * pSaDoc, int begin, int end, bool glossSelected) 
{
	mSaDoc = pSaDoc;
	mBegin = begin;
	mEnd = end;
	mSize = end-begin+1;
	mGlossSelected = glossSelected;
}

BOOL CDlgAutoReferenceData::OnInitDialog() 
{

	// if the file is no longer available, clear the entry
	struct _stat buffer;
	if (_wstat( mLastImport, &buffer)!=0) 
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
		CheckDlgButton(IDC_NUMBERS_RADIO,BST_CHECKED);
	} 
	else 
	{
		CheckDlgButton(IDC_FILE_RADIO,BST_CHECKED);
	}
	
	OnRadio();

	mSpinBegin.SetRange(0,10000);
	mSpinBegin.SetPos(mBegin);
	mSpinEnd.SetRange(1,10000);
	mSpinEnd.SetPos(mEnd);

	CRect cbSize;			// current size of combo box
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


void CDlgAutoReferenceData::DoDataExchange(CDataExchange* pDX) 
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BEGIN_SPIN, mSpinBegin);
	DDX_Control(pDX, IDC_END_SPIN, mSpinEnd);
	DDX_Control(pDX, IDC_BEGIN_EDIT, mEditBegin);
	DDX_Control(pDX, IDC_END_EDIT, mEditEnd);
	DDX_Control(pDX, IDC_BROWSE_BUTTON, mButtonBrowse);
	DDX_Control(pDX, IDC_FIRST_GLOSS_RADIO, mRadioFirstGloss);
	DDX_Control(pDX, IDC_SELECTED_GLOSS_RADIO, mRadioSelectedGloss);
	DDX_Control(pDX, IDC_NUMBERS_RADIO, mRadioNumbers);
	DDX_Control(pDX, IDC_FILE_RADIO, mRadioFile);
	DDX_Text(pDX, IDC_FILENAME, mLastImport);
	DDV_MaxChars(pDX, mLastImport, 256);
	DDX_Text(pDX, IDC_BEGIN_EDIT, mBegin);
	DDV_MinMaxInt(pDX, mBegin, 0, mEnd+1);
	DDX_Text(pDX, IDC_END_EDIT, mEnd);
	DDV_MinMaxInt(pDX, mEnd, mBegin, 10000);
	DDX_Control(pDX, IDC_FILENAME, mEditFilename);
	DDX_Control(pDX, IDC_BEGIN_COMBO, mComboBegin);
	DDX_Control(pDX, IDC_END_COMBO, mComboEnd);
	DDX_CBString(pDX, IDC_BEGIN_COMBO, mBeginRef);
	DDX_CBString(pDX, IDC_END_COMBO, mEndRef);

	if (!pDX->m_bSaveAndValidate) 
	{

		if (!mUsingNumbers) 
		{
			struct _stat buffer;
			// update the dialog display
			if (mLastImport.GetLength()==0) 
			{
				mComboBegin.ResetContent();
				mComboEnd.ResetContent();
			} 
			else if (_wstat( mLastImport, &buffer)!=0) 
			{
				// the file is missing or bad
				mComboBegin.ResetContent();
				mComboEnd.ResetContent();
				mComboBegin.SetCurSel(-1);
				mComboEnd.SetCurSel(-1);
			} 
			else 
			{
				CTranscriptionData td = mSaDoc->ImportTranscription(CSaString(mLastImport),FALSE,FALSE,FALSE,FALSE);
				CString ref = td.m_szPrimary;
				mComboBegin.ResetContent();
				mComboEnd.ResetContent();
				TranscriptionDataMap & tdm = td.m_TranscriptionData;
				MarkerList refs = tdm[ref];
				for (MarkerList::iterator it = refs.begin();it!=refs.end();it++) 
				{
					mComboBegin.AddString(*it);
					mComboEnd.AddString(*it);
				}
				mComboBegin.SelectString(-1,mBeginRef);
				mComboEnd.SelectString(-1,mEndRef);
			}
		}
	} 
	else 
	{

		// transfer dialog controls to member variables
		mUsingNumbers = (IsDlgButtonChecked(IDC_NUMBERS_RADIO)!=0);
		mUsingFirstGloss = (IsDlgButtonChecked(IDC_FIRST_GLOSS_RADIO)!=0);

		if (!mUsingNumbers) 
		{
			// the import file must be valid!
			if (mLastImport.GetLength()==0) 
			{
				pDX->PrepareEditCtrl(IDC_FILENAME);
				CString msg;
				msg.FormatMessage(IDS_ERROR_FILENAME,(LPCTSTR)mLastImport);
				AfxMessageBox( msg, MB_OK|MB_ICONEXCLAMATION, 0);
				pDX->Fail();
			}

			struct _stat buffer;
			if (_wstat( mLastImport, &buffer)!=0) 
			{
				pDX->PrepareEditCtrl(IDC_FILENAME);
				CString msg;
				msg.FormatMessage(IDS_ERROR_FILENAME,(LPCTSTR)mLastImport);
				AfxMessageBox( msg, MB_OK|MB_ICONEXCLAMATION, 0);
				pDX->Fail();
			}

			CTranscriptionData td = mSaDoc->ImportTranscription(CSaString(mLastImport),FALSE,FALSE,FALSE,FALSE);
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
	}
}

BEGIN_MESSAGE_MAP(CDlgAutoReferenceData, CDialog)
	ON_COMMAND(IDC_NUMBERS_RADIO, &CDlgAutoReferenceData::OnRadio)
	ON_COMMAND(IDC_FILE_RADIO, &CDlgAutoReferenceData::OnRadio)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, &CDlgAutoReferenceData::OnClickedBrowseButton)
	ON_NOTIFY(UDN_DELTAPOS, IDC_BEGIN_SPIN, &CDlgAutoReferenceData::OnDeltaposBeginSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_END_SPIN, &CDlgAutoReferenceData::OnDeltaposEndSpin)
	ON_EN_KILLFOCUS(IDC_FILENAME, &CDlgAutoReferenceData::OnKillfocusFilename)
END_MESSAGE_MAP()

void CDlgAutoReferenceData::OnRadio() 
{

	mUsingNumbers  = (IsDlgButtonChecked(IDC_NUMBERS_RADIO)!=0);
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
	}
}

void CDlgAutoReferenceData::OnDeltaposBeginSpin(NMHDR *pNMHDR, LRESULT *pResult) 
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	mBegin = GetDlgItemInt(IDC_BEGIN_EDIT, NULL, TRUE);
	mBegin += pNMUpDown->iDelta;
	if (mBegin>10000) mBegin = 10000;
	if (mBegin<0) mBegin = 0;
	SetDlgItemInt( IDC_BEGIN_EDIT, mBegin, TRUE);

	mEnd = GetDlgItemInt(IDC_END_EDIT, NULL, TRUE);
	if (mBegin>=mEnd) mEnd=mBegin+1;
	SetDlgItemInt( IDC_END_EDIT, mEnd, TRUE);

	*pResult = 0;
}

void CDlgAutoReferenceData::OnDeltaposEndSpin(NMHDR *pNMHDR, LRESULT *pResult) 
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	mEnd = GetDlgItemInt(IDC_END_EDIT, NULL, TRUE);
	mEnd += pNMUpDown->iDelta;
	if (mEnd>10000) mEnd = 10000;
	if (mEnd<1) mEnd = 1;
	SetDlgItemInt( IDC_END_EDIT, mEnd, TRUE);

	mBegin = GetDlgItemInt(IDC_BEGIN_EDIT, NULL, TRUE);
	if (mEnd<=mBegin) mBegin=mEnd-1;
	SetDlgItemInt( IDC_BEGIN_EDIT, mBegin, TRUE);

	*pResult = 0;
}

void CDlgAutoReferenceData::OnClickedBrowseButton() 
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Text Files (*.txt) |*.txt| All Files (*.*) |*.*||"), this );
	if (dlg.DoModal() != IDOK) 
	{
		return;
	}
	mLastImport = dlg.GetPathName();
	UpdateData(FALSE);
}

void CDlgAutoReferenceData::OnKillfocusFilename() 
{
	GetDlgItemTextW(IDC_FILENAME,mLastImport);
	UpdateData(FALSE);
}

