#ifndef DLGALIGNTRANSCRIPTIONDATAALIGNBYPAGE_H
#define DLGALIGNTRANSCRIPTIONDATAALIGNBYPAGE_H

#include "resource.h"

class CDlgAlignTranscriptionDataAlignByPage : public CPropertyPage
{
public:
	CDlgAlignTranscriptionDataAlignByPage();
	virtual ~CDlgAlignTranscriptionDataAlignByPage();

	enum { IDD = IDD_ANNOTATION_ALIGN_PAGE };

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnSetActive();

public:
	int m_nAlignBy;
	afx_msg void OnClicked();

	void SetText( int nItem, UINT nIDS);
	virtual LRESULT OnWizardBack();
};

#endif
