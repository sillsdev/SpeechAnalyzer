#ifndef DLGEXPORTSFM_H
#define DLGEXPORTSFM_H

#include "CSaString.h"
#include "AppDefs.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgExportSFM dialog

class CDlgExportSFM : public CDialog
{
    // Construction
public:
    CDlgExportSFM(const CSaString & szDocTitle, CWnd * pParent = NULL);

protected:
    virtual void OnOK();
    virtual void DoDataExchange(CDataExchange * pDX);
    virtual BOOL OnInitDialog();
    void ExportStandard();
    void ExportMultiRec();
    void ExportFile(CSaDoc * pDoc, CFile & file);
    void ExportCounts(CSaDoc * pDoc, CFile & file);
    void ExportAllFileInformation(CSaDoc * pDoc, CFile & file);
    void ExportAllParameters(CSaDoc * pDoc, CFile & file);
    void ExportAllSource(CSaDoc * pDoc, CFile & file);
    bool TryExportSegmentsBy(Annotations master, CSaDoc * pDoc, CFile & file);
    CSaString BuildRecord(Annotations target, DWORD dwStart, DWORD dwStop, CSaDoc * pDoc);
    CSaString BuildPhrase(Annotations target, DWORD dwStart, DWORD dwStop, CSaDoc * pDoc);
    Annotations GetAnnotation(int val);
    BOOL GetFlag(Annotations val);
    int GetIndex(Annotations val);
    LPCTSTR GetTag(Annotations val);
    void SetEnable(int nItem, BOOL bEnable);
    void SetCheck(int nItem, BOOL bCheck);
    void WriteFileUtf8(CFile * pFile, const CSaString szString);

    afx_msg void OnClickedExSfmInterlinear();
    afx_msg void OnClickedExSfmMultirecord();
    afx_msg void OnAllSource();
    afx_msg void OnAllFileInfo();
    afx_msg void OnAllParameters();
    afx_msg void OnAllAnnotations();
    afx_msg void OnHelpExportBasic();

    CSaString m_szFileName;
    CSaString m_szDocTitle;
    enum { IDD = IDD_EXPORT_SFM };
    BOOL  m_bAllSource;
    BOOL  m_bBandwidth;
    BOOL  m_bBits;
    BOOL  m_bComments;
    BOOL  m_bDialect;
    BOOL  m_bEthnologue;
    BOOL  m_bFamily;
    BOOL  m_bAllFile;
    BOOL  m_bFileSize;
    BOOL  m_bFileName;
    BOOL  m_bFree;
    BOOL  m_bGender;
    BOOL  m_bHighPass;
    BOOL  m_bInterlinear;
    BOOL  m_bMultiRecord;
    BOOL  m_bLanguage;
    BOOL  m_bLastModified;
    BOOL  m_bLength;
    BOOL  m_bNotebookRef;
    BOOL  m_bNumberSamples;
    BOOL  m_bOriginalDate;
    BOOL  m_bOriginalFormat;
    BOOL  m_bPhones;
    BOOL  m_bSampleRate;
    BOOL  m_bAllParameters;
    BOOL  m_bRegion;
    BOOL  m_bSpeaker;
    BOOL  m_bTranscriber;
    BOOL  m_bWords;
    BOOL  m_bAllAnnotations;
    BOOL  m_bGloss;
    BOOL  m_bOrtho;
    BOOL  m_bPhonemic;
    BOOL  m_bPhonetic;
    BOOL  m_bPOS;
    BOOL  m_bReference;
    BOOL  m_bTone;
    BOOL  m_bCountry;
    BOOL  m_bQuantization;
    BOOL  m_bPhrase;

    DECLARE_MESSAGE_MAP()

};

#endif DLGEXPORTSFM_H
