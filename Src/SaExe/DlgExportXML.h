#ifndef DLGEXPORTXML_H
#define DLGEXPORTXML_H

#include "SaString.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgExportXML dialog

class CDlgExportXML : public CDialog
{

public:
    CDlgExportXML(const CSaString & szDocTitle, CWnd * pParent = NULL);

    static const char XML_HEADER1[];
    static const char XML_HEADER2[];
    static const char XML_FOOTER[];

protected:
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void SetEnable(int nItem, BOOL bEnable);
    void SetCheck(int nItem, BOOL bCheck);
    void WriteFileUtf8(CFile * pFile, const CSaString szString);
    void OutputXMLField(CFile * pFile, LPCTSTR szFieldName, const CSaString & szContents);

    afx_msg void OnAllSource();
    afx_msg void OnAllFileInfo();
    afx_msg void OnAllParameters();
    afx_msg void OnAllAnnotations();
    afx_msg void OnClickedExSfmInterlinear();
    afx_msg void OnClickedExSfmMultirecord();
    afx_msg void OnHelpExportBasic();

    enum { IDD = IDD_EXPORT_XML };
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
    BOOL    m_bPhrase;
    CSaString m_szFileName;
    CSaString m_szDocTitle;

    DECLARE_MESSAGE_MAP()
};

#endif DLGEXPORTXML_H
