#ifndef DLGSELECTTRANSCRIPTIONBARS_H
#define DLGSELECTTRANSCRIPTIONBARS_H

// CDlgSelectTranscriptionBars dialog

class CDlgSelectTranscriptionBars : public CDialog
{
	DECLARE_DYNAMIC(CDlgSelectTranscriptionBars)

public:
	CDlgSelectTranscriptionBars(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSelectTranscriptionBars();

	// Dialog Data
	enum { IDD = IDD_SELECT_TRANSCRIPTION_BARS };

	BOOL bReference;
	BOOL bPhonetic;
	BOOL bTone;
	BOOL bPhonemic;
	BOOL bOrthographic;
	BOOL bGloss;
	BOOL bPhraseList1;
	BOOL bPhraseList2;
	BOOL bPhraseList3;
	BOOL bPhraseList4;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

#endif
