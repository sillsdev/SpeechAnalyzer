#include "stdafx.h"
#include "DlgExportXML.h"
#include "sa_doc.h"
#include "Sa_segm.h"
#include "MainFrm.h"
#include "SA_View.h"
#include "GlossSegment.h"
#include "TextSegment.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExportXML dialog

BEGIN_MESSAGE_MAP(CDlgExportXML, CExportBasicDialog)
END_MESSAGE_MAP()

// TRE 01-31-2001 Broke the header in two for size restrictions
const char CDlgExportXML::XML_HEADER1[] =
"<?xml version = \"1.0\"?>\r\n"
"\r\n"
#ifndef XML_EXCLUDE_DOCUMENT_DEFINITION
"<!DOCTYPE SAData [\r\n"
"  <!ELEMENT SAdoc (FileInfo?,LinguisticInfo?,SpeakerInfo?)>\r\n"
"\r\n"
"  <!ELEMENT FileInfo (FileDescription?,AudioFile?,CreationTime?,LastEdit?,FileSize?,OrigFormat?,DatabaseName?)>\r\n"
"    <!ELEMENT FileDescription (#PCDATA)>\r\n"
"    <!ELEMENT AudioFile (#PCDATA)>\r\n"
"    <!ELEMENT CreationTime (#PCDATA)>\r\n"
"    <!ELEMENT LastEdit (#PCDATA)>\r\n"
"    <!ELEMENT FileSize (#PCDATA)>\r\n"
"    <!ELEMENT OrigFormat (#PCDATA)>\r\n"
"    <!ELEMENT DatabaseName (#PCDATA)>\r\n"
"\r\n"
"  <!ELEMENT LinguisticInfo (PhoneticText?,Tone?,PhonemicText?,Orthographic?,Gloss?,PartOfSpeech?,Reference?,PhraseLevel1?,PhraseLevel2?,PhraseLevel3?,PhraseLevel4?,FreeTranslation?,NumPhones?,NumWords?)>\r\n"
"    <!ELEMENT PhoneticText (#PCDATA)>\r\n"
"    <!ELEMENT Tone (#PCDATA)>\r\n"
"    <!ELEMENT PhonemicText (#PCDATA)>\r\n"
"    <!ELEMENT Orthographic (#PCDATA)>\r\n"
"    <!ELEMENT Gloss (#PCDATA)>\r\n"
"    <!ELEMENT PartOfSpeech (#PCDATA)>\r\n"
"    <!ELEMENT Reference (#PCDATA)>\r\n"
"    <!ELEMENT PhraseLevel1 (#PCDATA)>\r\n"
"    <!ELEMENT PhraseLevel2 (#PCDATA)>\r\n"
"    <!ELEMENT PhraseLevel3 (#PCDATA)>\r\n"
"    <!ELEMENT PhraseLevel4 (#PCDATA)>\r\n"
"    <!ELEMENT FreeTranslation (#PCDATA)>\r\n"
"    <!ELEMENT NumPhones (#PCDATA)>\r\n"
"    <!ELEMENT NumWords (#PCDATA)>\r\n"
"\r\n";
#endif
const char CDlgExportXML::XML_HEADER2[] =
#ifndef XML_EXCLUDE_DOCUMENT_DEFINITION
"  <!ELEMENT DataInfo (NumSamples?,Length?,SamplingFreq?,Bandwidth?,HighPassFiltered?,StorageFormat?,QuantizSize?)>\r\n"
"    <!ELEMENT NumSamples (#PCDATA)>\r\n"
"    <!ELEMENT Length (#PCDATA)>\r\n"
"    <!ELEMENT SamplingFreq (#PCDATA)>\r\n"
"    <!ELEMENT Bandwidth (#PCDATA)>\r\n"
"    <!ELEMENT HighPassFiltered (#PCDATA)>\r\n"
"    <!ELEMENT StorageFormat (#PCDATA)>\r\n"
"    <!ELEMENT QuantizSize (#PCDATA)>\r\n"
"\r\n"
"  <!ELEMENT SourceInfo (Language?,Dialect?,Country?,family?,Region?,EthnologueID?,Speaker?,Gender?,NotebookReference?,Transcriber?)>\r\n"
"    <!ELEMENT Language (#PCDATA)>\r\n"
"    <!ELEMENT Dialect (#PCDATA)>\r\n"
"    <!ELEMENT Family (#PCDATA)>\r\n"
"    <!ELEMENT Country (#PCDATA)>\r\n"
"    <!ELEMENT Region (#PCDATA)>\r\n"
"    <!ELEMENT EthnologueID (#PCDATA)>\r\n"
"    <!ELEMENT Speaker (#PCDATA)>\r\n"
"    <!ELEMENT Gender (#PCDATA)>\r\n"
"    <!ELEMENT NotebookReference (#PCDATA)>\r\n"
"    <!ELEMENT Transcriber (#PCDATA)>\r\n"
"  <!ELEMENT Data (Staff?)>\r\n"
"    <!ELEMENT Staff (#PCDATA)>\r\n"
"]>\r\n"
"\r\n"
#endif
"<SAdoc>\r\n"
"\r\n";

const char CDlgExportXML::XML_FOOTER[] = "\r\n"
"\r\n"
"</SAdoc>";

CDlgExportXML::CDlgExportXML( const CSaString & szDocTitle, CWnd* pParent) : CExportBasicDialog( szDocTitle, pParent) 
{
}

void CDlgExportXML::OutputXMLField( CFile* pFile, const TCHAR *szFieldName, const CSaString &szContents)
{
	if (!szContents || !*szContents) return;

	CSaString szString;
	szString = "\t<";
	szString += szFieldName;
	szString += ">";
	for(register int i=0;i<szContents.GetLength();++i)
	{
		if (szContents[i]=='<')
			szString += "&#60;";
		else if (szContents[i]=='>')
			szString += "&#62;";
		else if (szContents[i]=='&')
			szString += "&#38;";
		else if (szContents[i]<0)
		{
			CSaString szS;
			swprintf_s(szS.GetBuffer(25),25,_T("&#%ld;"),(unsigned char) szContents[i]);
			szString += szS;
		}
		else szString += szContents[i];
	}
	szString += "</";
	szString += szFieldName;
	szString += ">\r\n";
	WriteFileUtf8(pFile, szString);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgExportXML message handlers

void CDlgExportXML::OnOK()
{
	if ((m_szFileName = ::GetExportFilename( m_szDocTitle, _T("Extensible Markup (*.xml) |*.xml||"), _T("xml"))) == "") return;

	UpdateData(TRUE);

	// process all flags
	if (m_bAllAnnotations)
		m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bPOS = m_bPhrase = TRUE;

	if (m_bAllFile)
		m_bOriginalDate = m_bLastModified = m_bOriginalFormat = m_bFileSize = TRUE;

	if (m_bAllParameters)
		m_bNumberSamples = m_bLength = m_bSampleRate = m_bBandwidth = m_bHighPass = m_bBits = m_bQuantization = TRUE;

	if (m_bAllSource)
		m_bLanguage = m_bDialect = m_bSpeaker = m_bGender = m_bEthnologue = m_bFamily = m_bRegion = m_bNotebookRef =
		m_bTranscriber = m_bComments = m_bCountry = TRUE;

	CFile* pFile = new CFile(m_szFileName, CFile::modeCreate|CFile::modeWrite);
	CSaString szString;

	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	pFile->Write(XML_HEADER1,sizeof(XML_HEADER1)-1);
	pFile->Write(XML_HEADER2,sizeof(XML_HEADER2)-1);

	pFile->Write("<FileInfo>\r\n",12);

	if (m_bComments) // %%% 1 %%%
	{
		OutputXMLField(pFile,_T("Comments"),pDoc->GetSaParm()->szDescription);
	}
	if (m_bFileName) // %%% 2 %%%
	{
		OutputXMLField(pFile,_T("AudioFile"),pDoc->GetPathName());
	}

	CFileStatus* pFileStatus = pDoc->GetFileStatus();
	if (pFileStatus->m_szFullName[0] != 0) // file name is defined
	{
		if (m_bOriginalDate) // %%% 3 %%%
		{
			OutputXMLField(pFile,_T("CreationTime"),pFileStatus->m_ctime.Format("%A, %B %d, %Y, %X"));
		}
		if (m_bLastModified) // %%% 4 %%%
		{
			OutputXMLField(pFile,_T("LastEdit"),pFileStatus->m_mtime.Format("%A, %B %d, %Y, %X"));
		}
		if (m_bFileSize) // %%% 5 %%%
		{
			swprintf_s(szString.GetBuffer(25),25,_T("%ld Bytes"),pFileStatus->m_size);
			szString.ReleaseBuffer();
			OutputXMLField(pFile,_T("FileSize"),szString);
		}
		if (m_bOriginalFormat) // %%% 6 %%%
		{
			SaParm* pSaParm = pDoc->GetSaParm();
			if (pSaParm->byRecordFileFormat <= FILE_FORMAT_TIMIT)
			{
				szString.LoadString((UINT)pSaParm->byRecordFileFormat + IDS_FILETYPE_UTT);
				OutputXMLField(pFile,_T("OrigFormat"),szString);
			}
		}
	}

	pFile->Write("</FileInfo>\r\n",13);

	pFile->Write("<LinguisticInfo>\r\n",18);
	if (!pDoc->GetSegment(PHONETIC)->IsEmpty())
	{
		CSaString szAnnotation[ANNOT_WND_NUMBER];
		CSaString szPOS;
		int nMaxLength = 0;
		int nNumber = 0;
		DWORD dwOffset;

		while (nNumber != -1)
		{
			BOOL bBreak = FALSE;

			dwOffset = pDoc->GetSegment(PHONETIC)->GetOffset(nNumber);

			int nFind = pDoc->GetSegment(GLOSS)->FindOffset(dwOffset);
			if ((nNumber > 0) && (nFind != -1))
				bBreak = TRUE;
			if (!m_bInterlinear && bBreak)
			{
				for (int nLoop = PHONETIC; nLoop < ANNOT_WND_NUMBER; nLoop++)
				{
					szAnnotation[nLoop] += " ";
				}
				szPOS +=" ";
			}
			else if (bBreak)
			{
				for (int nLoop = PHONETIC; nLoop < ANNOT_WND_NUMBER; nLoop++)
				{
					while (szAnnotation[nLoop].GetLength() <= nMaxLength)
						szAnnotation[nLoop] += " ";
				}
				while (szPOS.GetLength() <= nMaxLength)
					szPOS +=" ";
			}

			szAnnotation[PHONETIC] += pDoc->GetSegment(PHONETIC)->GetSegmentString(nNumber);

			for (int nLoop = PHONETIC+1; nLoop < ANNOT_WND_NUMBER; nLoop++)
			{
				nFind = pDoc->GetSegment(nLoop)->FindOffset(dwOffset);
				if (nFind != -1)
				{
					if (nLoop == GLOSS)
					{
						szAnnotation[nLoop] += pDoc->GetSegment(nLoop)->GetSegmentString(nFind).Mid(1);
						szPOS += ((CGlossSegment*)pDoc->GetSegment(GLOSS))->GetPOSs()->GetAt(nFind);
						if (szPOS.GetLength() > nMaxLength) nMaxLength = szPOS.GetLength();
					}
					else
					{
						szAnnotation[nLoop] += pDoc->GetSegment(nLoop)->GetSegmentString(nFind);
					}
				}
				if (szAnnotation[nLoop].GetLength() > nMaxLength) nMaxLength = szAnnotation[nLoop].GetLength();
			}

			nNumber = pDoc->GetSegment(PHONETIC)->GetNext(nNumber);
		}
		// write out results
		if (m_bPhonetic) // %%% 8 %%%
		{
			OutputXMLField(pFile,_T("PhoneticText"),szAnnotation[PHONETIC]);
		}
		if (m_bTone) // %%% 9 %%%
		{
			OutputXMLField(pFile,_T("Tone"),szAnnotation[TONE]);
		}
		if (m_bPhonemic) // %%% 10 %%%
		{
			OutputXMLField(pFile,_T("PhonemicText"),szAnnotation[PHONEMIC]);
		}
		if (m_bOrtho) // %%% 11 %%%
		{
			OutputXMLField(pFile,_T("Orthographic"),szAnnotation[ORTHO]);
		}
		if (m_bGloss) // %%% 12 %%%
		{
			OutputXMLField(pFile,_T("Gloss"),szAnnotation[GLOSS]);
		}
		if (m_bPOS) // %%% 13 %%%
		{
			OutputXMLField(pFile,_T("PartOfSpeech"),szPOS);
		}
		if (m_bReference) // %%% 14 %%%
		{
			OutputXMLField(pFile,_T("Reference"),szAnnotation[REFERENCE]);
		}
	}

	for (int nPhrase = MUSIC_PL1; nPhrase <= MUSIC_PL4; nPhrase++)
	{
		CSaString szPhrase;

		szString.Format(_T("PhraseLevel%d "), nPhrase - MUSIC_PL1 + 1);

		if (m_bPhrase && !pDoc->GetSegment(nPhrase)->IsEmpty())
		{
			int nNumber = 0;

			while (nNumber != -1)
			{
				szPhrase += pDoc->GetSegment(nPhrase)->GetSegmentString(nNumber);

				nNumber = pDoc->GetSegment(nPhrase)->GetNext(nNumber);
			}
		}
		if (m_bPhrase)// %%% 15 %%% - %%% 18 %%%
		{
			OutputXMLField(pFile,szString,szPhrase);
		}
	}

	if (m_bFree) // %%% 15 %%%
	{
		OutputXMLField(pFile,_T("FreeTranslation"),pDoc->GetSourceParm()->szFreeTranslation);
	}
	if (m_bPhones) // %%% 16 %%%
	{
		// create and write number of phones text
		int nNumber = 0;
		int nLoop = 0;
		if (pDoc->GetSegment(PHONETIC)->GetString()->GetLength() > 0)
		{
			// find number of phones
			while(TRUE)
			{ nLoop++;
			nNumber = pDoc->GetSegment(PHONETIC)->GetNext(nNumber);
			if (nNumber < 0) break;
			}
		}
		swprintf_s(szString.GetBuffer(25),25,_T("%u"), nLoop);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("NumPhones"),szString);
	}
	if (m_bWords) // %%% 17 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%u"), ((CTextSegment*)pDoc->GetSegment(GLOSS))->CountWords());
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("NumWords"),szString);
	}
	pFile->Write("</LinguisticInfo>\r\n",19);

	pFile->Write("<DataInfo>\r\n",12);
	if (m_bNumberSamples) // %%% 18 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%ld"), pDoc->GetDataSize() / pDoc->GetFmtParm()->wBlockAlign);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("NumSamples"),szString);
	}
	if (m_bLength) // %%% 19 %%%
	{
		// create and write length text
		double fDataSec = pDoc->GetTimeFromBytes(pDoc->GetUnprocessedDataSize()); // get sampled data size in seconds
		int nMinutes = (int)fDataSec / 60;

		if (nMinutes == 0) // length is less than one minute
			swprintf_s(szString.GetBuffer(25),25,_T("%5.3f Seconds"), fDataSec);
		else // length is equal or more than one minute
		{
			fDataSec = fDataSec - (float)(nMinutes * 60);
			swprintf_s(szString.GetBuffer(25),25,_T("%i:%5.3f (Min:Sec)"), nMinutes, fDataSec);
		}
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("Length"),szString);
	}
	if (m_bSampleRate) // %%% 20 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%lu Hz"),pDoc->GetFmtParm()->dwSamplesPerSec);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("SamplingFreq"),szString);
	}
	if (m_bBandwidth) // %%% 21 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%lu Hz"),pDoc->GetSaParm()->dwRecordBandWidth);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("Bandwidth"),szString);
	}
	if (m_bHighPass) // %%% 22 %%%
	{
		szString = pDoc->GetSaParm()->wFlags & SA_FLAG_HIGHPASS ? "Yes":"No";
		OutputXMLField(pFile,_T("HighPassFiltered"),szString);
	}
	if (m_bBits) // %%% 23 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%d Bits"),pDoc->GetFmtParm()->wBitsPerSample);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("StorageFormat"),szString);
	}
	if (m_bQuantization) // %%% 24 %%%
	{
		swprintf_s(szString.GetBuffer(25),25,_T("%d Bits"),(int)pDoc->GetSaParm()->byRecordSmpSize);
		szString.ReleaseBuffer();
		OutputXMLField(pFile,_T("QuantizSize"),szString);
	}
	pFile->Write("</DataInfo>\r\n",13);

	pFile->Write("<SourceInfo>\r\n",14);
	if (m_bLanguage) // %%% 25 %%%
	{
		OutputXMLField(pFile,_T("Language"),pDoc->GetSourceParm()->szLanguage);
	}

	if (m_bDialect) // %%% 26 %%%
	{
		OutputXMLField(pFile,_T("Dialect"),pDoc->GetSourceParm()->szDialect);
	}
	if (m_bFamily) // %%% 27 %%%
	{
		OutputXMLField(pFile,_T("Family"),pDoc->GetSourceParm()->szFamily);
	}
	if (m_bCountry) // %%% 28 %%%
	{
		OutputXMLField(pFile,_T("Country"),pDoc->GetSourceParm()->szCountry);
	}
	if (m_bRegion) // %%% 29 %%%
	{
		OutputXMLField(pFile,_T("Region"),pDoc->GetSourceParm()->szRegion);
	}
	if (m_bEthnologue) // %%% 30 %%%
	{
		OutputXMLField(pFile,_T("EthnologueID"),pDoc->GetSourceParm()->szEthnoID);
	}
	if (m_bSpeaker) // %%% 31 %%%
	{
		OutputXMLField(pFile,_T("SpeakerName"),pDoc->GetSourceParm()->szSpeaker);
	}
	if (m_bGender) // %%% 32 %%%
	{
		switch(pDoc->GetSourceParm()->nGender)
		{
		case 0:  szString = "Adult Male";   break;
		case 1:  szString = "Adult Female"; break;
		case 2:  szString = "Child";        break;
		default: szString = "";             break;
		}
		OutputXMLField(pFile,_T("Gender"),szString);
	}
	if (m_bNotebookRef) // %%% 33 %%%
	{
		OutputXMLField(pFile,_T("NotebookReference"),pDoc->GetSourceParm()->szReference);
	}
	if (m_bTranscriber) // %%% 34 %%%
	{
		OutputXMLField(pFile,_T("Transcriber"),pDoc->GetSourceParm()->szTranscriber);
	}
	pFile->Write("</SourceInfo>\r\n",14);

	pFile->Write("<Data>\r\n",8);
	pFile->Write("</Data>\r\n",9);

	pFile->Write(XML_FOOTER,sizeof(XML_FOOTER)-1);

	if (pFile) delete pFile;
	CDialog::OnOK();
}

void CDlgExportXML::InitializeDialog() 
{
	SetEnable(IDC_EX_SFM_INTERLINEAR, FALSE);
}
