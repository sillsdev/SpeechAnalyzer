// DlgAudio.h: interface for the CDlgAudio class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

class CDlgAudio : public CDialog  
{
public:
	CDlgAudio();
	CDlgAudio(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL) : CDialog(lpszTemplateName,pParentWnd)
	{
	};
	CDlgAudio(UINT nIDTemplate, CWnd* pParentWnd = NULL) : CDialog(nIDTemplate,pParentWnd)
	{
	};
	virtual ~CDlgAudio();
	virtual void BlockFinished(UINT nLevel, DWORD dwPosition, UINT nSpeed=100) = 0;
	virtual void BlockStored(UINT nLevel, DWORD dwPosition, BOOL *bSaveOverride) = 0;
	virtual void StoreFailed() = 0;
	virtual void EndPlayback() = 0;
	virtual HPSTR GetWaveData(DWORD dwPlayPosition, DWORD dwDataSize) = 0;

};

