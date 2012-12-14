#pragma once

// CDlgAlignTranscriptionDataFinishPage
#include "resource.h"

class CDlgAlignTranscriptionDataFinishPage : public CPropertyPage {

public:
	CDlgAlignTranscriptionDataFinishPage();
	virtual ~CDlgAlignTranscriptionDataFinishPage();

	enum { IDD = IDD_ANNOTATION_FINISH_PAGE };

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnWizardFinish();
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardBack();
};


