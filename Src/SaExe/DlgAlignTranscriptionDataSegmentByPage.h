#ifndef DLGALIGNTRANSCRIPTIONSEGMENTSBYPAGE_H
#define DLGALIGNTRANSCRIPTIONSEGMENTSBYPAGE_H

#include "resource.h"
#include "CSaString.h"

class CDlgAlignTranscriptionDataSegmentByPage : public CPropertyPage
{
public:
    CDlgAlignTranscriptionDataSegmentByPage();
    virtual ~CDlgAlignTranscriptionDataSegmentByPage();

    enum { IDD = IDD_ANNOTATION_SEGMENT_PAGE };

protected:
    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnSetActive();

public:
    int m_nSegmentBy;

    afx_msg void OnClicked();

    void SetText(int nItem, UINT nIDS);
};

#endif
