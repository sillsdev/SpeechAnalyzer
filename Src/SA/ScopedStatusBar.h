#ifndef SCOPEDSTATUSBAR_H
#define SCOPEDSTATUSBAR_H
#include "MainFrm.h"
#include "WaveUtils.h"

class CScopedStatusBar : public IProgressUpdate {
public:
	CScopedStatusBar( UINT resourceID);
	~CScopedStatusBar();
	void SetProgress(int progress);
    int GetProgress();
};

#endif
