/////////////////////////////////////////////////////////////////////////////
// sa_exprt.cpp:
// Implementation of the CExportFW
//                       CExportTabbed class.
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
// 1.5Test8.5
//         SDM Changed Table headings to use Initial Uppercase ("Emic" & "Etic")
// 1.5Test10.7
//         SDM Changed Import to not pad extra phonetic with spaces
// 1.5Test11.0
//         SDM replaced GetOffset() + GetDuration() with CSegment::GetStop()
//         SDM fixed bug in export which crashed if phonetic segment empty
//         SDM changed export to export AutoPitch (Grappl)
// 1.5Test11.3
//         SDM changed CSegment::AdjustPositionAll to CSegment::Adjust
//
//    07/27/2000
//         DDO Changed so these dialogs display before the user is asked
//             what file to export. Therefore, I had to move the get filename
//             dialog call into this module instead of doing it in the view
//             class when the user picks one of the export menu items.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process\sa_proc.h"
#include "sa_segm.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_pit.h"
#include "Process\sa_p_cpi.h"
#include "Process\sa_p_spi.h"
#include "Process\sa_p_mel.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_spg.h"
#include "dsp\formants.h"
#include "dsp\ztransform.h"
#include "Process\formanttracker.h"
#include "dsp\mathx.h"

#include "sa_wbch.h"
#include "sa.h"
#include "sa_doc.h"
#include "sa_view.h"
#include "sa_graph.h"
#include "mainfrm.h"
#include "settings\obstream.h"
#include "doclist.h"
using std::ifstream;
using std::ios;
using std::streampos;

#include "exportbasicdialog.h"
#include "sa_exprt.h"
#include "result.h"
#include <math.h>
#include "TranscriptionDataSettings.h"
#include "DlgImport.h"
#include "GlossSegment.h"
#include "PhoneticSegment.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

void WriteFileUtf8( CFile *pFile, const CSaString szString)
{
	std::string szUtf8 = szString.utf8();
	pFile->Write(szUtf8.c_str(),szUtf8.size());
}

//****************************************************************************
// Added on 07/27/200 by DDO.
//****************************************************************************
CSaString GetExportFilename( CSaString szTitle, CSaString szFilter, TCHAR *szExtension)
{
	//**************************************
	// Extract what's to left of :
	//**************************************
	int nFind = szTitle.Find(':');
	if (nFind != -1) {
		szTitle = szTitle.Left(nFind);
	}
	nFind = szTitle.ReverseFind('.');

	//**************************************
	// Remove extension if necessary.
	//**************************************
	szTitle.Trim();
	if (nFind >= ((szTitle.GetLength() > 3) ? (szTitle.GetLength() - 4) : 0)) {
		szTitle = szTitle.Left(nFind);
	}

	CFileDialog dlg(FALSE, szExtension, szTitle, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, NULL);
	if (dlg.DoModal() != IDOK) return "";
	return dlg.GetPathName();
}

/////////////////////////////////////////////////////////////////////////////
// CExportFW dialog

BEGIN_MESSAGE_MAP(CExportFW, CExportBasicDialog)
END_MESSAGE_MAP()

/*
\name name of file
\wav  Audio FileName
\ph   Phonetic text
\tn   Tone
\pm   Phonemic text
\or   Orthographic
\gl   Gloss
\pos  Part of Speech
\ref  Reference
\ft   Free Translation
\np   Number of Phones
\nw   Number of Words

\od   Original Date (Creation Time)
\lud  Date Last updated
\size File size in bytes
\of   Original Format
\samp Number of Samples
\len  Length
\freq Sampling Frequency
\bw   Bandwidth
\hpf  HighPass Filter
\bits Storage Format
\qsize Quantization Size

\ln   Language Name
\dlct Dialect
\cnt  Country
\spkr Speaker Name
\gen  Gender
\id   Ethnologue ID number
\fam  Family
\reg  Region
\nbr  Notebook Reference
\tr   Transcriber
\desc Description
*/

/////////////////////////////////////////////////////////////////////////////
// CExportSFM message handlers

void CExportFW::OnOK() {

	if ((m_szFileName = GetExportFilename( m_szDocTitle, _T("Standard Format (*.sfm) |*.sfm||"), _T("sfm"))) == "") return;

	UpdateData(TRUE);

	// process all flags
	if (m_bAllAnnotations) {
		m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = TRUE;
	}

	if (m_bAllFile) {
		m_bOriginalDate = m_bLastModified = m_bOriginalFormat = m_bFileSize = TRUE;
	}

	if (m_bAllParameters) {
		m_bNumberSamples = m_bLength = m_bSampleRate = m_bBandwidth = m_bHighPass = m_bBits = m_bQuantization = TRUE;
	}

	if (m_bAllSource) {
		m_bLanguage = m_bDialect = m_bSpeaker = m_bGender = m_bEthnologue = m_bFamily = m_bRegion = m_bNotebookRef = m_bTranscriber = m_bComments = m_bCountry = TRUE;
	}

	CFile file(m_szFileName, CFile::modeCreate|CFile::modeWrite);
	CSaString szString;
	CSaString szCrLf = "\r\n";

	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	if (!pDoc->GetSegment(PHONETIC)->IsEmpty()) {
		
		int nNumber = 0;
		while (nNumber != -1) {

			// this segment should be matching
			DWORD dwOffset = pDoc->GetSegment(PHONETIC)->GetOffset(nNumber);
			
			// try and determine end based on gloss
			CSaString szTemp = "";
			int nStart = pDoc->GetSegment(GLOSS)->FindOffset(dwOffset);
			if (nStart!=-1) {
				if ((nStart+1)<pDoc->GetSegment(GLOSS)->GetOffsetSize()) {
					int nEnd = pDoc->GetSegment(GLOSS)->GetOffset(nStart+1);
					ASSERT(nEnd!=-1);
					int nNext = nNumber;
					// scroll through phonetic until we find next gloss
					szTemp = pDoc->GetSegment(PHONETIC)->GetSegmentString(nNext);
					while (true) {
						nNext++;
						DWORD dwOffset2 = pDoc->GetSegment(PHONETIC)->GetOffset(nNext);
						if ((dwOffset2<dwOffset)||(dwOffset2>=nEnd)) {
							break;
						} else {
							szTemp += pDoc->GetSegment(PHONETIC)->GetSegmentString(nNext);
						}
					}
				} else {
					// extract everything to end
					int nNext = nNumber;
					// scroll through phonetic until we find next gloss
					szTemp = pDoc->GetSegment(PHONETIC)->GetSegmentString(nNext);
					while (true) {
						nNext++;
						if (nNext>=pDoc->GetSegment(PHONETIC)->GetOffsetSize()) {
							break;
						} else {
							szTemp += pDoc->GetSegment(PHONETIC)->GetSegmentString(nNext);
						}
					}
				}
				if (szTemp.GetLength()>0) {
					szString = "\\lx  " + szTemp + szCrLf;
					WriteFileUtf8( &file, szString);
				}

				int nFind = pDoc->GetSegment(GLOSS)->FindOffset(dwOffset);
				if (nFind != -1) {
					szString = "\\ge  " + pDoc->GetSegment(GLOSS)->GetSegmentString(nFind).Mid(1) + szCrLf;
					WriteFileUtf8( &file, szString);
				}

				nFind = pDoc->GetSegment(REFERENCE)->FindOffset(dwOffset);
				if (nFind != -1) {
					szString = "\\rf " + pDoc->GetSegment(REFERENCE)->GetSegmentString(nFind) + szCrLf;
					WriteFileUtf8( &file, szString);
				}

				WriteFileUtf8( &file, szCrLf);
			}

			nNumber = pDoc->GetSegment(PHONETIC)->GetNext(nNumber);
		}
	}

	if (m_bFileName) { // \wav  Audio FileName
		szString = "\\pf " + pDoc->GetPathName() + szCrLf;
		WriteFileUtf8( &file, szString);
	}

	// \date write current time
	CTime time = CTime::GetCurrentTime();
	szString = "\\dt " + time.Format("%A, %B %d, %Y, %X") + "\r\n";
	WriteFileUtf8( &file, szString);

	file.Close();

	CDialog::OnOK();
}

CExportFW::CExportFW( const CSaString & szDocTitle, CWnd* pParent) : CExportBasicDialog( szDocTitle, pParent) 
{
	m_bAllParameters = FALSE;
	m_bAllSource = FALSE;
}

void CExportFW::InitializeDialog() 
{
	SetEnable(IDC_EX_SFM_RECORD_DATA, FALSE);
	SetEnable(IDC_EX_SFM_BANDWIDTH, FALSE);
	SetEnable(IDC_EX_SFM_BITS, FALSE);
	SetEnable(IDC_EX_SFM_QUANTIZATION, FALSE);
	SetEnable(IDC_EX_SFM_HIGHPASS, FALSE);
	SetEnable(IDC_EX_SFM_LENGTH, FALSE);
	SetEnable(IDC_EX_SFM_NUMBER_OF_SAMPLES, FALSE);
	SetEnable(IDC_EX_SFM_RATE, FALSE);

	SetEnable(IDC_EX_SFM_ALL_SOURCE, FALSE);
	SetEnable(IDC_EX_SFM_LANGUAGE, FALSE);
	SetEnable(IDC_EX_SFM_DIALECT, FALSE);
	SetEnable(IDC_EX_SFM_SPEAKER, FALSE);
	SetEnable(IDC_EX_SFM_GENDER, FALSE);
	SetEnable(IDC_EX_SFM_ETHNOLOGUE_ID, FALSE);
	SetEnable(IDC_EX_SFM_FAMILY, FALSE);
	SetEnable(IDC_EX_SFM_REGION, FALSE);
	SetEnable(IDC_EX_SFM_COUNTRY, FALSE);
	SetEnable(IDC_EX_SFM_NOTEBOOKREF, FALSE);
	SetEnable(IDC_EX_SFM_TRANSCRIBER, FALSE);
	SetEnable(IDC_EX_SFM_COMMENTS, FALSE);

	SetEnable(IDC_EX_SFM_FREE, FALSE);
	SetCheck(IDC_EX_SFM_FREE, FALSE);

	SetEnable(IDC_EX_SFM_PHONES, FALSE);
	SetCheck(IDC_EX_SFM_PHONES, FALSE);

	SetEnable(IDC_EX_SFM_WORDS, FALSE);
	SetCheck(IDC_EX_SFM_WORDS, FALSE);

	SetEnable(IDC_EX_SFM_FILENAME, FALSE);
	SetCheck(IDC_EX_SFM_FILENAME, TRUE);

	SetEnable(IDC_EX_SFM_FILE_INFO, FALSE);
	SetCheck(IDC_EX_SFM_FILE_INFO, TRUE);

	SetEnable(IDC_EXTAB_ANNOTATIONS, FALSE);
	SetCheck(IDC_EXTAB_ANNOTATIONS, TRUE);
	SetCheck(IDC_EXTAB_POS, FALSE);
	SetCheck(IDC_EXTAB_TONE, FALSE);
	SetCheck(IDC_EXTAB_PHRASE, FALSE);
	SetCheck(IDC_EXTAB_PHONEMIC, FALSE);
	SetCheck(IDC_EXTAB_ORTHO, FALSE);

	SetEnable(IDC_EX_SFM_MULTIRECORD, FALSE);
	SetEnable(IDC_EX_SFM_INTERLINEAR, FALSE);
	SetCheck(IDC_EX_SFM_MULTIRECORD, TRUE);
}
