#include "stdafx.h"
#include "Import.h"
#include "Settings\OBSTREAM.H"
#include "MainFrm.h"
#include "SA_View.h"
#include "TranscriptionDataSettings.h"
#include "RESULT.H"
#include "Segment.h"
#include "Sa_graph.h"
#include "DlgImport.h"
#include "GlossSegment.h"
#include "PhoneticSegment.h"

using std::ifstream;
using std::ios;
using std::streampos;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static const char* psz_Phonemic = "pm";
static const char* psz_Gloss = "gl";
static const char* psz_Phonetic = "ph";
static const char* psz_Orthographic = "or";
static const char* psz_ImportEnd = "import";

static const char* psz_FreeTranslation = "ft"; // Free Translation
static const char* psz_Language ="ln"; // Language Name
static const char* psz_Dialect = "dlct"; // Dialect
static const char* psz_Family = "fam"; // Family
static const char* psz_Ethno = "id"; // Ethnologue ID number
static const char* psz_Country = "cnt"; // Country
static const char* psz_Region = "reg"; // Region
static const char* psz_Speaker = "spkr"; // Speaker Name
static const char* psz_Gender = "gen"; // Gender
static const char* psz_NotebookReference = "nbr"; // Notebook Reference
static const char* psz_Transcriber = "tr"; // Transcriber
static const char* psz_Comments = "cmnt"; // Comments
static const char* psz_Description = "desc"; // Description
static const char* psz_Table = "table";

/***************************************************************************/
// CImport::Import read import file
/***************************************************************************/
BOOL CImport::Import(int nMode)
{
	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	pDoc->CheckPoint();
	pDoc->SetModifiedFlag(TRUE);
	pDoc->SetTransModifiedFlag(TRUE); // transcription has been modified

	BOOL ret = TRUE;
	CSaString String;
	CSaString Imported("Imported...\r\n");
	CSaString Skipped("Skipped... \r\n");
	CSaString *pPhonetic = NULL;
	CSaString *pPhonemic = NULL;
	CSaString *pOrtho = NULL;
	CSaString *pGloss = NULL;
	BOOL bTable = FALSE;
	const CSaString CrLf("\r\n");
	try
	{
		Object_istream obs(m_szPath);
		if ( !obs.bAtBackslash() )
		{
			obs.SkipBOM();
			if ( !obs.bAtBackslash() )
				return FALSE;
		}
		while ( !obs.bAtEnd() )
		{
			streampos pos = obs.getIos().tellg();
			const char* pszMarkerRead;
			CSaString szStringRead;
			obs.peekMarkedString(&pszMarkerRead, &szStringRead);

			if ( obs.bReadString(psz_Table, &String) )
			{
				obs.getIos().seekg(pos);  // start before marker
				obs.getIos().clear();
				BOOL result = ReadTable(obs, nMode);
				if (result)
				{
					Imported += "\\" + CSaString(pszMarkerRead) + " (Entire Table)" + CrLf;
					bTable = TRUE;
				}
				else
					Skipped += "\\" + CSaString(pszMarkerRead) + " (Entire Table)" + CrLf;
				break;  // this must be last marker
			}
			else if ( obs.bReadString(psz_FreeTranslation, &String) )
				pDoc->GetSourceParm()->szFreeTranslation = String;
			else if ( obs.bReadString(psz_Language, &String) )
				pDoc->GetSourceParm()->szLanguage = String;
			else if ( obs.bReadString(psz_Dialect, &String) )
				pDoc->GetSourceParm()->szDialect = String;
			else if ( obs.bReadString(psz_Family, &String) )
				pDoc->GetSourceParm()->szFamily = String;
			else if ( obs.bReadString(psz_Ethno, &String) )
				pDoc->GetSourceParm()->szEthnoID = String;
			else if ( obs.bReadString(psz_Country, &String) )
				pDoc->GetSourceParm()->szCountry = String;
			else if ( obs.bReadString(psz_Region, &String) )
				pDoc->GetSourceParm()->szRegion = String;
			else if ( obs.bReadString(psz_Speaker, &String) )
				pDoc->GetSourceParm()->szSpeaker = String;
			else if ( obs.bReadString(psz_Gender, &String) )
			{
				int nGender = pDoc->GetSourceParm()->nGender;

				String.MakeUpper();

				if (String == "ADULT MALE") nGender = 0;         // male
				else if (String == "ADULT FEMALE") nGender = 1;  // female
				else if (String == "CHILD") nGender = 2;  // child

				pDoc->GetSourceParm()->nGender = nGender;
			}
			else if ( obs.bReadString(psz_NotebookReference, &String) )
				pDoc->GetSourceParm()->szReference = String;
			else if ( obs.bReadString(psz_Transcriber, &String) )
				pDoc->GetSourceParm()->szTranscriber = String;
			else if ( obs.bReadString(psz_Comments, &String) )
				pDoc->GetSaParm()->szDescription = String;
			else if ( obs.bReadString(psz_Description, &String) )
				pDoc->GetSaParm()->szDescription = String;
			else if ( obs.bReadString(psz_Phonetic, &String) )
			{
				if (!pPhonetic)
					pPhonetic = new CSaString;
				*pPhonetic = String;
				continue;
			}
			else if ( obs.bReadString(psz_Phonemic, &String) )
			{
				if (!pPhonemic)
					pPhonemic = new CSaString;
				*pPhonemic = String;
				continue;
			}
			else if ( obs.bReadString(psz_Orthographic, &String) )
			{
				if (!pOrtho)
					pOrtho = new CSaString;
				*pOrtho = String;
				continue;
			}
			else if ( obs.bReadString(psz_Gloss, &String) )
			{
				if (!pGloss)
					pGloss = new CSaString;
				*pGloss = " " + String;


				int nSpace;
				while((nSpace = pGloss->Find(_T(" "))) != -1)
				{
					pGloss->SetAt(nSpace,'#');
				}
				continue;
			}
			else if ( obs.bEnd(psz_ImportEnd) )
				break;
			else
			{
				Skipped += "\\" + CSaString(pszMarkerRead) + " " + szStringRead + CrLf;
				continue;
			}
			Imported += "\\" + CSaString(pszMarkerRead) + " " + szStringRead + CrLf;
		}
	}
	catch(...)
	{
	}

	if (pPhonetic || pPhonemic || pOrtho || pGloss)
	{
		if (!bTable) 
		{
			AutoAlign( pDoc, pPhonetic, pPhonemic, pOrtho, pGloss);
		}

		CSaString Report;

		if (pPhonetic)
		{
			Report += "\\" + CSaString(psz_Phonetic) + " " + *pPhonetic + CrLf;
			delete pPhonetic;
		}
		if (pPhonemic)
		{
			Report += "\\" + CSaString(psz_Phonemic) + " " + *pPhonemic + CrLf;
			delete pPhonemic;
		}
		if (pOrtho)
		{
			Report += "\\" + CSaString(psz_Orthographic) + " " + *pOrtho + CrLf;
			delete pOrtho;
		}
		if (pGloss)
		{
			Report += "\\" + CSaString(psz_Gloss) + " " + *pGloss + CrLf;
			delete pGloss;
		}

		if (bTable)
			Skipped += Report;
		else
			Imported += Report;
	}

	if (!m_bBatch)
		CResult result(Imported + CrLf + Skipped, AfxGetMainWnd());

	return ret;
}

/***************************************************************************/
// CImport::AutoAlign Execute changes by request from batch file
/***************************************************************************/
void CImport::AutoAlign( CSaDoc * pSaDoc, const CSaString * Phonetic, const CSaString * Phonemic, const CSaString * Ortho, const CSaString * Gloss)
{
	CTranscriptionDataSettings settings;

	settings.m_bPhonetic = (Phonetic != NULL);
	settings.m_bPhonemic = (Phonemic != NULL);
	settings.m_bOrthographic = (Ortho != NULL);
	settings.m_bGloss = (Gloss != NULL);

	settings.m_bPhoneticModified = (settings.m_bPhonetic!=FALSE);
	settings.m_bPhonemicModified = (settings.m_bPhonemic!=FALSE);
	settings.m_bOrthographicModified = (settings.m_bOrthographic!=FALSE);
	settings.m_bGlossModified = (settings.m_bGloss!=FALSE);

	if (settings.m_bPhonetic) settings.m_szPhonetic = *(Phonetic);
	if (settings.m_bPhonemic) settings.m_szPhonemic = *(Phonemic);
	if (settings.m_bOrthographic) settings.m_szOrthographic = *(Ortho);
	if (settings.m_bGloss) settings.m_szGloss = *(Gloss);

	settings.m_nAlignBy = IDC_CHARACTER;
	settings.m_nSegmentBy = IDC_KEEP;

	// save state for undo ability
	pSaDoc->CheckPoint();
	pSaDoc->SetModifiedFlag(TRUE); // document has been modified
	pSaDoc->SetTransModifiedFlag(TRUE); // transcription has been modified

	POSITION pos = pSaDoc->GetFirstViewPosition();
	CSaView* pView = (CSaView*)pSaDoc->GetNextView(pos);

	enum { CHARACTER_OFFSETS = 0,CHARACTER_DURATIONS = 1,WORD_OFFSETS = 2};
	CDWordArray pArray[3];

	// clean gloss string
	// remove trailing and leading spaces
	settings.m_szGloss.Trim(EDIT_WORD_DELIMITER);

	if (pSaDoc->GetSegment(GLOSS)->IsEmpty()) {
		// auto parse
		if (!pSaDoc->AdvancedParseAuto()) {
			// process canceled by user
			pSaDoc->Undo(FALSE);
			return;
		}
	}

	CSegment* pSegment=pSaDoc->GetSegment(PHONETIC);
	
	//adjust character segments
	switch (settings.m_nSegmentBy) 
	{
	case IDC_AUTOMATIC:
		{
			if (!pSaDoc->AdvancedSegment()) {
			// SDM 1.5Test8.2
				// process canceled by user
				pSaDoc->Undo(FALSE);
				return;
			}
			for (int i=0;i<pSegment->GetOffsetSize();i++) {
				pArray[CHARACTER_OFFSETS].InsertAt(i,pSegment->GetOffset(i)); // Copy Arrays
			}
			for (int i=0;i<pSegment->GetDurationSize();i++) {
				pArray[CHARACTER_DURATIONS].InsertAt(i,pSegment->GetDuration(i));
			}
			// Copy gloss segments SDM 1.5Test8.2
			for (int i=0;i<pSaDoc->GetSegment(GLOSS)->GetOffsetSize();i++) {
				pArray[WORD_OFFSETS].InsertAt(i,pSaDoc->GetSegment(GLOSS)->GetOffset(i)); 
			}
			// Create a gloss break at initial position SDM 1.5Test8.2
			if (pArray[WORD_OFFSETS][0] != pArray[CHARACTER_OFFSETS][0]) {
				CSaString szEmpty = "";
				pSaDoc->GetSegment(GLOSS)->Insert(0, szEmpty, FALSE, pArray[CHARACTER_OFFSETS][0], pArray[WORD_OFFSETS][0]-pArray[CHARACTER_OFFSETS][0]);
				pArray[WORD_OFFSETS].InsertAt(0,pArray[CHARACTER_OFFSETS][0]);
				settings.m_szGloss = CSaString(EDIT_WORD_DELIMITER) + settings.m_szGloss;
				settings.m_szPhonetic = CSaString(EDIT_WORD_DELIMITER) + settings.m_szPhonetic;
				settings.m_szPhonemic = CSaString(EDIT_WORD_DELIMITER) + settings.m_szPhonemic;
				settings.m_szOrthographic = CSaString(EDIT_WORD_DELIMITER) + settings.m_szOrthographic;
			}
			break;
		}

	case IDC_MANUAL: 
		{
		// SDM 1.5Test8.2
			for (int i=0;i<pSaDoc->GetSegment(GLOSS)->GetOffsetSize();i++) {
				pArray[WORD_OFFSETS].InsertAt(i,pSaDoc->GetSegment(GLOSS)->GetOffset(i)); // Copy gloss segments SDM 1.5Test8.2
			}
			switch(settings.m_nAlignBy) {
			case IDC_NONE:
			case IDC_WORD:
				{
					pArray[CHARACTER_OFFSETS].InsertAt(0,&pArray[WORD_OFFSETS]); // Copy gloss segments
					// build duration list
					int nIndex = 1;

					while(nIndex < pArray[CHARACTER_OFFSETS].GetSize())
					{
						pArray[CHARACTER_DURATIONS].Add(pArray[CHARACTER_OFFSETS][nIndex] - pArray[CHARACTER_OFFSETS][nIndex - 1]);
						nIndex++;
					}
					// Add final duration to fill remainder of waveform
					pArray[CHARACTER_DURATIONS].Add(pSaDoc->GetUnprocessedDataSize() - pArray[CHARACTER_OFFSETS][nIndex - 1]);
					break;
				}
			case IDC_CHARACTER:
				{
					CFontTable* pTable = pSaDoc->GetFont(PHONETIC);
					int nWord = 0;
					int nCharacters;
					CSaString szWord;
					for(int nGlossWordIndex = 0; nGlossWordIndex < pArray[WORD_OFFSETS].GetSize(); nGlossWordIndex++) {
						DWORD dwDuration;
						if ((nGlossWordIndex+1) < pArray[WORD_OFFSETS].GetSize()) {
							dwDuration = pArray[WORD_OFFSETS][nGlossWordIndex+1] - pArray[WORD_OFFSETS][nGlossWordIndex];
							szWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nWord, settings.m_szPhonetic);
							nCharacters = pTable->GetLength(CFontTable::CHARACTER, szWord);
						} else {
							dwDuration = pSaDoc->GetUnprocessedDataSize() - pArray[WORD_OFFSETS][nGlossWordIndex];
							szWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nWord, settings.m_szPhonetic);
							nCharacters = pTable->GetLength(CFontTable::CHARACTER, szWord);
							szWord = pTable->GetRemainder(CFontTable::DELIMITEDWORD, nWord, settings.m_szPhonetic);
							if (szWord.GetLength() != 0) nCharacters++;  // remainder goes into one extra segment
						}
						if (nCharacters == 0) {
							nCharacters++;
						}

						DWORD dwOffset = pArray[WORD_OFFSETS][nGlossWordIndex];
						DWORD dwSize = dwDuration/nCharacters;

						if (pSaDoc->GetFmtParm()->wBlockAlign == 2) {
							dwSize &= ~1;
						}

						for(int nIndex = 0; nIndex < nCharacters; nIndex++) {
							pArray[CHARACTER_OFFSETS].Add(dwOffset);
							dwOffset = pSaDoc->SnapCursor(STOP_CURSOR, dwOffset + dwSize, dwOffset + (dwSize/2) & ~1, dwOffset + dwSize, SNAP_LEFT);
						}
					}
					int nIndex = 1;
					while(nIndex < pArray[CHARACTER_OFFSETS].GetSize()) {
						pArray[CHARACTER_DURATIONS].Add(pArray[CHARACTER_OFFSETS][nIndex] - pArray[CHARACTER_OFFSETS][nIndex - 1]);
						nIndex++;
					}
					// Add final duration to fill remainder of waveform
					pArray[CHARACTER_DURATIONS].Add(pSaDoc->GetUnprocessedDataSize() - pArray[CHARACTER_OFFSETS][nIndex - 1]);
					break;
				}
			}
			break;
		}

	case IDC_KEEP: // SDM 1.5Test8.2
		{
			// Copy gloss segments SDM 1.5Test8.2
			for (int i=0;i<pSaDoc->GetSegment(GLOSS)->GetOffsetSize();i++) {
				DWORD offset = pSaDoc->GetSegment(GLOSS)->GetOffset(i);
				TRACE("word offset %d\n",offset);
				pArray[WORD_OFFSETS].InsertAt(i,offset);
			}
			
			// copy segment locations not character counts
			int nIndex = 0;
			while (nIndex != -1) {
				DWORD offset = pSegment->GetOffset(nIndex);
				pArray[CHARACTER_OFFSETS].Add(offset);
				TRACE("character offset %d\n",offset);
				DWORD duration = pSegment->GetDuration(nIndex);
				pArray[CHARACTER_DURATIONS].Add(duration);
				TRACE("character duration %d\n",offset);
				nIndex = pSegment->GetNext(nIndex);
			}
		}
	}

	CFontTable::tUnit nAlignMode = CFontTable::CHARACTER;
	switch (settings.m_nAlignBy) {
	case IDC_NONE:
		nAlignMode = CFontTable::NONE;
		break;
	case IDC_WORD:
		nAlignMode = CFontTable::DELIMITEDWORD;
		break;
	case IDC_CHARACTER:
	default:
		nAlignMode = CFontTable::CHARACTER;
	}

	// Insert Annotations
	{
		CSegment * pSegment = NULL;
		CFontTable * pTable = NULL;
		CSaString szNext;
		CSaString szNextWord;
		int nWordIndex = 0;
		int nStringIndex = 0;
		int nOffsetSize = 0;
		int nGlossIndex = 0;
		int nIndex = 0;

		// Process phonetic
		// SDM 1.06.8 only change if new segmentation or text changed
		if ((settings.m_bPhonetic) && ((settings.m_nSegmentBy != IDC_KEEP)||(settings.m_bPhoneticModified))) {

			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = pSaDoc->GetSegment(PHONETIC);
			pTable = pSaDoc->GetFont(PHONETIC);
			pSegment->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for (nIndex = 0; nIndex < (nOffsetSize-1);nIndex++) {
				switch(settings.m_nAlignBy) {
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
					if (szNext.GetLength()==0) {
						szNext+=SEGMENT_DEFAULT_CHAR;
					}
					pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_WORD:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) { // No more word breaks continue one character at a time
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
					} else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex]) {
						// Insert default segment character if phonetic offset does not correspond to word boundary
						szNext = SEGMENT_DEFAULT_CHAR;
					} else { // Insert Word on Gloss Boundary
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength()==0) {
						szNext+=SEGMENT_DEFAULT_CHAR;
					}
					pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					// the line is entered one character per segment
					szNext.Empty();
					while (true) 
					{
						CSaString szTemp = pTable->GetNext( nAlignMode, nStringIndex, settings.m_szPhonetic);
						if (szTemp.GetLength()==0) 
						{
							// end of array
							szTemp=SEGMENT_DEFAULT_CHAR;
							break;
						} else if ((szTemp.GetLength()==1)&&(szTemp[0]==EDIT_WORD_DELIMITER)) {
							// time to stop!
							break;
						} else if (szTemp.GetLength()>1)  {
							// in some situations if the trailing character is not a break
							// it will be combined with the space.  we will break it here.
							if (szTemp[0]==EDIT_WORD_DELIMITER) {
								if (szNext.GetLength()==0) {
									// remove space and append
									szTemp.Delete(0,1);
								} else {
									// backup and let the next character go into the next segment
									nStringIndex--;
									break;
								}
							}
						}
						szNext += szTemp;
					}
					pSegment->Insert( pSegment->GetOffsetSize(),szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				}
			}

			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szPhonetic);
			if (szNext.GetLength()==0) {
				szNext+=SEGMENT_DEFAULT_CHAR;
			}
			pSegment->Insert(pSegment->GetOffsetSize(),szNext,FALSE,pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
			
			// SDM 1.06.8 apply input filter to segment
			if (pSegment->GetInputFilter()) {
				(pSegment->GetInputFilter())(*pSegment->GetString());
			}
		}

		// Process phonemic
		// SDM 1.06.8 only change  if new segmentation or text changed
		if ((settings.m_bPhonemic) && ((settings.m_nSegmentBy != IDC_KEEP)||(settings.m_bPhonemicModified))) {

			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = pSaDoc->GetSegment(PHONEMIC);
			pTable = pSaDoc->GetFont(PHONEMIC);
			pSegment->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for (nIndex = 0; nIndex < (nOffsetSize-1);nIndex++) {
				switch(settings.m_nAlignBy) {
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
					if (szNext.GetLength()!=0) {
						// Skip Empty Segments
						pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				case IDC_WORD:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) {
						// No more word breaks continue one character at a time
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
					} else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex]) {
						// Skip if phonetic offset does not correspond to word boundary
						continue;
					} else { // Insert Word on Gloss Boundary
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength()==0) {
						// Skip NULL strings
						continue; 
					}
					pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					// the line is entered one character per segment
					szNext.Empty();
					while (true) {
						CSaString szTemp = pTable->GetNext( nAlignMode, nStringIndex, settings.m_szPhonemic);
						if (szTemp.GetLength()==0) {
							break;
						} else if ((szTemp.GetLength()==1)&&(szTemp[0]==EDIT_WORD_DELIMITER)) {
							// time to stop!
							break;
						} else if (szTemp.GetLength()>1) {
							// in some situations if the trailing character is not a break
							// it will be combined with the space.  we will break it here.
							if (szTemp[0]==EDIT_WORD_DELIMITER) {
								if (szNext.GetLength()==0) {
									// remove space and append
									szTemp.Delete(0,1);
								} else {
									// backup and let the next character go into the next segment
									nStringIndex--;
									break;
								}
							}
						}
						szNext += szTemp;
					}
					if (szNext.GetLength()>0) {
						pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				}
			}

			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szPhonemic);
			// Skip empty segments
			if (szNext.GetLength()!=0) {
				pSegment->Insert(pSegment->GetOffsetSize(),szNext,FALSE, pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
			}
			// SDM 1.06.8 apply input filter to segment
			if (pSegment->GetInputFilter()) {
				(pSegment->GetInputFilter())(*pSegment->GetString());
			}
		}

		// Process tone
		if (settings.m_nSegmentBy != IDC_KEEP) {
			// SDM 1.06.8 only change  if new segmentation or text changed
			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = pSaDoc->GetSegment(TONE);
			pSegment->DeleteContents();
		}

		// Process orthographic
		// SDM 1.06.8 only change  if new segmentation or text changed
		if ((settings.m_bOrthographic) && ((settings.m_nSegmentBy != IDC_KEEP)||(settings.m_bOrthographicModified))) {

			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			pSegment = pSaDoc->GetSegment(ORTHO);
			pTable = pSaDoc->GetFont(ORTHO);
			pSegment->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for (nIndex = 0; nIndex < (nOffsetSize-1);nIndex++) {
				switch(settings.m_nAlignBy) {
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
					if (szNext.GetLength()!=0) {
						// Skip Empty Segments
						pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				case IDC_WORD:
					if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize()) {
						// No more word breaks continue one character at a time
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
					} else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex]) {
						// Skip if character offset does not correspond to word boundary
						continue;
					} else {
						// Insert Word on Boundary
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength()==0) {
						// Skip NULL words
						continue; 
					}
					pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					// the line is entered one character per segment
					szNext.Empty();
					while (true) {
						CSaString szTemp = pTable->GetNext( nAlignMode, nStringIndex, settings.m_szOrthographic);
						if (szTemp.GetLength()==0) {
							break;
						} else if ((szTemp.GetLength()==1)&&(szTemp[0]==EDIT_WORD_DELIMITER)) {
							// time to stop!
							break;
						} else if (szTemp.GetLength()>1) {
							// in some situations if the trailing character is not a break
							// it will be combined with the space.  we will break it here.
							if (szTemp[0]==EDIT_WORD_DELIMITER) {
								if (szNext.GetLength()==0) {
									// remove space and append
									szTemp.Delete(0,1);
								} else {
									// backup and let the next character go into the next segment
									nStringIndex--;
									break;
								}
							}
						}
						szNext += szTemp;
					}
					if (szNext.GetLength()>0) {
						pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				}
			}

			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szOrthographic);
			if (szNext.GetLength()!=0) {
				// Skip empty segments
				pSegment->Insert(pSegment->GetOffsetSize(),szNext,FALSE,pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
			}
			
			// SDM 1.06.8 apply input filter to segment
			if (pSegment->GetInputFilter()) {
				(pSegment->GetInputFilter())(*pSegment->GetString());
			}
		}

		// Process gloss
		// SDM 1.5Test8.2 only change if text changed
		if (settings.m_bGlossModified) {
			nStringIndex = 0;
			pSegment = pSaDoc->GetSegment(GLOSS);
			pTable = pSaDoc->GetFont(GLOSS);

			bool poundDelimited = (settings.m_szGloss.FindOneOf(CSaString(WORD_DELIMITER))!=-1);

			// align gloss by word SDM 1.5Test8.2
			nAlignMode = CFontTable::DELIMITEDWORD;
			nOffsetSize = pArray[WORD_OFFSETS].GetSize();
			// Don't Select this segment SDM 1.5Test8.2
			pSegment->SelectSegment(*pSaDoc,-1);
			// the gloss table uses a space as a delimiter,
			// the normally the text is delimited with a #.
			// if we see a # in the first position, we will continue
			// to scan the segments util we
			for (nIndex = 0; nIndex < (nOffsetSize-1);nIndex++) {

				szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szGloss);
				if (szNext.GetLength()==0) {
					szNext = CSaString(WORD_DELIMITER);
				} else {
					if (szNext[0]==WORD_DELIMITER) {
						// do nothing
					} else {
						szNext = CSaString(WORD_DELIMITER)+szNext;
					}
				}

				// if the user used # delimiters, then if there are
				// embedded spaces, concatenate the lines
				if (poundDelimited) {
					int nTempIndex = nStringIndex;
					bool done = false;
					while (!done) {
						int nLastIndex = nTempIndex;
						CSaString szTemp = pTable->GetNext(nAlignMode, nTempIndex, settings.m_szGloss);
						if (szTemp.GetLength()==0) {
							if (nTempIndex==nLastIndex) {
								// we are at the end of the data
								done = true;
							} else {
								szNext += CSaString(EDIT_WORD_DELIMITER);
							}
						} else {
							if (szTemp[0]==WORD_DELIMITER) {
								// we found the next line
								nStringIndex = nLastIndex;
								done = true;
							} else {
								szNext += CSaString(EDIT_WORD_DELIMITER) + szTemp;
							}
						}
					}
				}

				szNext.Remove(0x0d);
				szNext.Remove(0x0a);
				pSegment->SelectSegment(*pSaDoc,nIndex);
				((CGlossSegment*)pSegment)->ReplaceSelectedSegment(pSaDoc,szNext);
			};

			// take care of remainder
			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szGloss);
			if (szNext.GetLength()==0) {
				szNext = CSaString(WORD_DELIMITER);
			} else {
				if (szNext[0]==WORD_DELIMITER) {
					// do nothing
				} else {
					szNext = CSaString(WORD_DELIMITER)+szNext;
				}
			}
			szNext.Remove(0x0d);
			szNext.Remove(0x0a);
			pSegment->SelectSegment(*pSaDoc,nIndex);
			((CGlossSegment*)pSegment)->ReplaceSelectedSegment(pSaDoc,szNext);
		}
	}

	pView->ChangeAnnotationSelection(pSegment, -1);

	CGraphWnd *pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);
	if (pGraph) {
		if (settings.m_bPhonetic) {		
			pGraph->ShowAnnotation(PHONETIC, TRUE, TRUE);
		}
		if (settings.m_bPhonemic) {
			pGraph->ShowAnnotation(PHONEMIC, TRUE, TRUE);
		}
		if (settings.m_bOrthographic) {
			pGraph->ShowAnnotation(ORTHO, TRUE, TRUE);
		}
		if (settings.m_bGloss) {
			pGraph->ShowAnnotation(GLOSS, TRUE, TRUE);
		}
	}
	pView->RefreshGraphs(); // redraw all graphs without legend window
}


/***************************************************************************/
// extractTabField local helper function to get field from tab delimited string
/***************************************************************************/
static const CSaString extractTabField(const CSaString& szLine, const int nField)
{
	int nCount = 0;
	int nLoop = 0;

	if (nField < 0) return ""; // SDM 1.5Test10.1

	while((nLoop < szLine.GetLength()) && (nCount < nField))
	{
		if (szLine[nLoop] == '\t')
			nCount++;
		nLoop++;
	}
	int nBegin = nLoop;
	while((nLoop < szLine.GetLength()) && (szLine[nLoop] != '\t'))
	{
		nLoop++;
	}
	return szLine.Mid(nBegin, nLoop-nBegin);
}

static void CreateWordSegments(const int nWord, int& nSegments)
{
	CSaDoc* pDoc = (CSaDoc*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	if (pDoc->GetSegment(GLOSS)->GetOffsetSize() > nWord)
	{
		DWORD dwStart;
		DWORD dwStop;
		int nPhonetic;
		CPhoneticSegment* pPhonetic = (CPhoneticSegment*) pDoc->GetSegment(PHONETIC);

		if (nWord == -1)
		{
			dwStart = 0;
			if (pDoc->GetSegment(GLOSS)->IsEmpty())
			{
				dwStop = pDoc->GetUnprocessedDataSize();
			}
			else
			{
				dwStop = pDoc->GetSegment(GLOSS)->GetOffset(0);
			}
			if (dwStart + pDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME) > dwStop) return;
			nPhonetic = 0;
		}
		else
		{
			ASSERT(nSegments);
			dwStart = pDoc->GetSegment(GLOSS)->GetOffset(nWord);
			dwStop = pDoc->GetSegment(GLOSS)->GetDuration(nWord) + dwStart;
			nPhonetic = pPhonetic->FindOffset(dwStart);
		}
		// Limit number of segments
		if (nSegments*pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME) > (dwStop -dwStart))
		{
			nSegments = (int)((dwStop -dwStart)/pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));
			if (!nSegments) nSegments = 1;
			if (nSegments*pDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME) > (dwStop -dwStart))
				return;
		}
		// remove excess segments
		int nCount = 0;
		int nIndex = nPhonetic;
		while ((nIndex != -1)&&(pPhonetic->GetOffset(nIndex) < dwStop))
		{
			if (nCount >= nSegments)
			{
				pPhonetic->SetSelection(nIndex);
				pPhonetic->Remove(pDoc, FALSE); // no checkpoint
				if (nIndex >= pPhonetic->GetOffsetSize()) break;
			}
			else
			{
				DWORD dwBegin = dwStart + nCount;
				pPhonetic->Adjust(pDoc, nIndex, dwBegin, 1);
				nIndex = pPhonetic->GetNext(nIndex);
				nCount++;
			}
		}
		if (nSegments == 0) return; // done
		// add segments
		while (nCount < nSegments)
		{
			if (nIndex == -1) nIndex = pPhonetic->GetOffsetSize();
			DWORD dwBegin = dwStart + nCount;
			CSaString szEmpty(SEGMENT_DEFAULT_CHAR);
			pPhonetic->Insert(nIndex, szEmpty, FALSE, dwBegin, 1);
			nIndex = pPhonetic->GetNext(nIndex);
			nCount++;
		}
		// adjust segment spacing
		DWORD dwSize = (dwStop - dwStart)/nSegments;
		if (pDoc->GetFmtParm()->wBlockAlign==2)
		{
			dwSize &= ~1;
		};
		dwSize += pDoc->GetFmtParm()->wBlockAlign;
		if (nIndex == -1) nIndex = pPhonetic->GetOffsetSize();
		nIndex = pPhonetic->GetPrevious(nIndex);
		while((nIndex != -1)&&(pPhonetic->GetOffset(nIndex) >= dwStart))
		{
			nCount--;
			DWORD dwBegin = dwStart+nCount*dwSize;
			DWORD dwDuration = dwSize;
			if ((dwBegin + dwDuration) > dwStop) dwDuration = dwStop - dwBegin;
			pPhonetic->Adjust(pDoc, nIndex, dwBegin, dwDuration);
			nIndex = pPhonetic->GetPrevious(nIndex);
		}
	}
	nSegments = 0;
}
/***************************************************************************/
// CImport::ReadTable read table from import file
// file position should be '\' of table marker
/***************************************************************************/
BOOL CImport::ReadTable(Object_istream &obs, int nMode)
{
	CSaView* pView = (CSaView*)((CMainFrame*)AfxGetMainWnd())->GetCurrSaView();

	CSaDoc* pDoc = (CSaDoc*)pView->GetDocument();

	const int MAXLINE = 32000;
	char* pUtf8 = new char[MAXLINE];
	CSaString szLine;

	// eat table marker
	obs.getIos().getline(pUtf8,MAXLINE);
	szLine.setUtf8(pUtf8);
	if (szLine.GetLength() >= (MAXLINE - 1)) // error
		return FALSE;
	if (szLine.Find(CString(psz_Table)) ==-1) // error
		return FALSE;

	// read header
	obs.getIos().getline(pUtf8,MAXLINE);
	szLine.setUtf8(pUtf8);
	if (szLine.GetLength() >= (MAXLINE - 1)) // error
		return FALSE;

	// parse header
	int nAnnotField[ANNOT_WND_NUMBER+1];
	CSaString szField;

	for(int nLoop = 0;nLoop < ANNOT_WND_NUMBER+1; nLoop++) nAnnotField[nLoop] = -1;


	for(int nLoop = 0;nLoop < 20; nLoop++)
	{
		szField = extractTabField(szLine, nLoop);

		if (szField.Find(_T("Time")) != -1)
			return FALSE;  // table is not built on phonetic segment boundaries
		else if (szField.Find(_T("Ref")) != -1)
			nAnnotField[REFERENCE] = nLoop;
		else if (szField.Find(_T("Etic")) != -1)
			nAnnotField[PHONETIC] = nLoop;
		else if (szField.Find(_T("Tone")) != -1)
			nAnnotField[TONE] = nLoop;
		else if (szField.Find(_T("Emic")) != -1)
			nAnnotField[PHONEMIC] = nLoop;
		else if (szField.Find(_T("Ortho")) != -1)
			nAnnotField[ORTHO] = nLoop;
		else if (szField.Find(_T("Gloss")) != -1)
			nAnnotField[GLOSS] = nLoop;
		else if (szField.Find(_T("POS")) != -1)
			nAnnotField[ANNOT_WND_NUMBER] = nLoop;
	}
	// create new segmentation
	if (nMode == QUERY)
	{
		CDlgImport dlg;
		if (dlg.DoModal() != IDOK)
		{
			// process canceled by user
			pDoc->Undo(FALSE);
			return FALSE;
		}
		nMode = dlg.m_nMode;
	}
	if ((pDoc->GetSegment(GLOSS)->IsEmpty())&& (nMode!=KEEP))
	{
		// do equal segmentation (replaces auto parse)
		streampos pos = obs.getIos().tellg();  // save top of file position

		// find number of segments in each word
		int nSegmentCount = 0;
		int nWords = 0;
		int nSegmentToBeginWord[MAX_INT];
		while(obs.getIos().peek() != EOF)
		{
			// read line
			obs.getIos().getline(pUtf8,MAXLINE);
			szLine.setUtf8(pUtf8);
			if (szLine.GetLength() >= (MAXLINE - 1)) // error
				return FALSE;
			if (extractTabField(szLine, nAnnotField[GLOSS]).GetLength()) // gloss found
			{
				nSegmentToBeginWord[nWords] = nSegmentCount;
				nWords++;
			}
			nSegmentCount++;
		}
		obs.getIos().seekg(pos);  // return to top of table
		obs.getIos().clear();  // clear the EOF flag
		if (nSegmentCount == 0) nSegmentCount = 1;
		if (nWords == 0) nWords = 1;
		nSegmentToBeginWord[nWords] = nSegmentCount;

		// add, remove and adjust segments as needed
		CPhoneticSegment* pPhonetic = (CPhoneticSegment*) pDoc->GetSegment(PHONETIC);
		CSaString szEmpty(SEGMENT_DEFAULT_CHAR);
		CSaString szEmptyGloss = "";
		CGlossSegment* pGloss = (CGlossSegment*)pDoc->GetSegment(GLOSS);
		DWORD dwFileLength = pDoc->GetUnprocessedDataSize();
		int nIndex = pPhonetic->FindFromPosition(0);
		int nWordCurr = 0;
		DWORD dwStart = 0;
		DWORD dwDuration = 0;
		while(nWordCurr < nWords || nIndex != -1)
		{
			CSaString szTest = pPhonetic->GetText(nIndex);
			if (nWordCurr < nWords)
			{
				dwDuration = (DWORD)((float)dwFileLength * (float)(nSegmentToBeginWord[nWordCurr + 1] - nSegmentToBeginWord[nWordCurr]) / (float)nSegmentCount);
				if (nIndex != -1) // adjust existing segments
					pPhonetic->Adjust(pDoc, nWordCurr, dwStart, dwDuration);
				else // add segments
					pPhonetic->Insert(nWordCurr, szEmpty, FALSE, dwStart, dwDuration);
				pGloss->Insert(nWordCurr, szEmptyGloss, 0, dwStart, dwDuration);
				nIndex = nWordCurr;
				nWordCurr++;
				dwStart += dwDuration;
			}
			else // remove extra segments
			{
				pPhonetic->SetSelection(nIndex);
				pPhonetic->Remove(pDoc, FALSE);
				nIndex--;
			}
			nIndex = pPhonetic->GetNext(nIndex);
		}
	}

	if ((nMode == MANUAL) && (nAnnotField[GLOSS] == -1))
		nMode = AUTO;

	if (nMode == MANUAL)
	{
		streampos pos = obs.getIos().tellg();  // save top of file position

		int nSegmentCount = 0;
		int nWordCount = -1;
		while(obs.getIos().peek() != EOF)
		{
			// read line
			obs.getIos().getline(pUtf8,MAXLINE);
			szLine.setUtf8(pUtf8);
			if (szLine.GetLength() >= (MAXLINE - 1)) // error
				return FALSE;
			if (extractTabField(szLine, nAnnotField[GLOSS]).GetLength()) // gloss found
			{
				CreateWordSegments(nWordCount, nSegmentCount);
				nWordCount++;
			};
			nSegmentCount++;
		}
		CreateWordSegments(nWordCount, nSegmentCount);
		obs.getIos().seekg(pos);  // return to top of table
		obs.getIos().clear();  // clear the EOF flag
	}
	else if (nMode == AUTO)
	{
		if (!pDoc->AdvancedSegment())
		{
			// process canceled by user
			pDoc->Undo(FALSE);
			return FALSE;
		}
	}

	// clear import fields
	CSegment* pPhonetic = pDoc->GetSegment(PHONETIC);
	CGlossSegment* pGloss = (CGlossSegment*) pDoc->GetSegment(GLOSS);
	CSaString szString = WORD_DELIMITER;
	if (nAnnotField[GLOSS] != -1)
	{
		for(int nIndex = 0; nIndex < pGloss->GetOffsetSize(); nIndex++)
		{
			if (pGloss->GetSelection() != nIndex) pGloss->SetSelection(nIndex);
			pGloss->ReplaceSelectedSegment(pDoc, szString);
		}
	}
	szString = SEGMENT_DEFAULT_CHAR;
	if (nAnnotField[PHONETIC] != -1)
	{
		for(int nIndex = 0; nIndex < pPhonetic->GetOffsetSize(); nIndex++)
		{
			pView->ASelection().SelectFromPosition(pView, PHONETIC, pPhonetic->GetOffset(nIndex), CASegmentSelection::FIND_EXACT);
			pView->ASelection().SetSelectedAnnotationString(pView, szString, TRUE, FALSE);
		}
	}
	for(int nIndex = PHONETIC+1; nIndex < ANNOT_WND_NUMBER; nIndex++)
	{
		if ((nAnnotField[nIndex] != -1)&& (nIndex != GLOSS))
			pDoc->GetSegment(nIndex)->DeleteContents();
	}
	if (nAnnotField[ANNOT_WND_NUMBER/* POS*/] != -1)
	{
		for(int nIndex = 0; nIndex < pDoc->GetSegment(GLOSS)->GetOffsetSize(); nIndex++)
			((CGlossSegment*)pDoc->GetSegment(GLOSS))->GetPOSs()->SetAt(nIndex, "");
	}

	// insert fields into segments
	int nIndexPhonetic = 0;
	int nIndexGloss = -1;
	BOOL bAppendGloss;
	BOOL bAppendPhonetic = FALSE;
	if (pPhonetic->IsEmpty()) return FALSE; // no where to go
	while(obs.getIos().peek() != EOF)
	{
		// read line
		obs.getIos().getline(pUtf8,MAXLINE);
		szLine.setUtf8(pUtf8);
		if (szLine.GetLength() >= (MAXLINE - 1)) // error
			return FALSE;

		szString = extractTabField(szLine, nAnnotField[GLOSS]);
		if (szString.GetLength()) // gloss found
		{
			nIndexGloss++;
			if (nIndexGloss >= pGloss->GetOffsetSize())
			{
				nIndexGloss--;
				if (nIndexPhonetic == pPhonetic->GetPrevious(pPhonetic->GetOffsetSize()))
				{
					bAppendPhonetic = TRUE;
				}
				else
				{
					nIndexPhonetic = pPhonetic->GetPrevious(pPhonetic->GetOffsetSize());
					bAppendPhonetic = FALSE;
				}
				bAppendGloss = TRUE;
			}
			else
			{
				nIndexPhonetic = pPhonetic->FindOffset(pGloss->GetOffset(nIndexGloss));
				bAppendPhonetic = FALSE;
				bAppendGloss = FALSE;
			}
			if (bAppendGloss)
				szString = pGloss->GetSegmentString(nIndexGloss) + " " + szString;
			if (pGloss->GetSelection() != nIndexGloss)
				pGloss->SetSelection(nIndexGloss);
			if ((szString[0] != WORD_DELIMITER) || (szString[0] != TEXT_DELIMITER))
				szString = WORD_DELIMITER + szString;
			pGloss->ReplaceSelectedSegment(pDoc, szString);

			// POS
			szString = extractTabField(szLine, nAnnotField[ANNOT_WND_NUMBER/*POS*/]);
			if (szString.GetLength())
			{
				if (bAppendGloss)
					szString = pGloss->GetPOSs()->GetAt(nIndexGloss) + " " + szString;
				pGloss->GetPOSs()->SetAt(nIndexGloss, szString);
			}
			// Reference
			szString = extractTabField(szLine, nAnnotField[REFERENCE]);
			if (szString.GetLength())
			{
				pView->ASelection().SelectFromPosition(pView, REFERENCE, pGloss->GetOffset(nIndexGloss), CASegmentSelection::FIND_EXACT);
				if (bAppendGloss)
					szString = pView->ASelection().GetSelectedAnnotationString(pView,FALSE) + " " + szString;
				pView->ASelection().SetSelectedAnnotationString(pView, szString, TRUE, FALSE);
			}
		}
		for(int nIndex = PHONETIC; nIndex < GLOSS; nIndex++)
		{
			szString = extractTabField(szLine, nAnnotField[nIndex]);
			if (szString.GetLength())
			{
				pView->ASelection().SelectFromPosition(pView, nIndex, pPhonetic->GetOffset(nIndexPhonetic), CASegmentSelection::FIND_EXACT);
				if (bAppendPhonetic)
					szString = pView->ASelection().GetSelectedAnnotationString(pView,FALSE) + /*" " +*/ szString; // SDM 1.5Test10.7 remove spaces
				pView->ASelection().SetSelectedAnnotationString(pView, szString, TRUE, FALSE);
			}
		}

		nIndexPhonetic = pPhonetic->GetNext(nIndexPhonetic);
		if (nIndexPhonetic == -1)
		{
			nIndexPhonetic = pPhonetic->GetPrevious(pPhonetic->GetOffsetSize());
			bAppendPhonetic = TRUE;
		}
		else if (((nIndexGloss + 1) < pGloss->GetOffsetSize()) && (pPhonetic->GetOffset(nIndexPhonetic) >= pGloss->GetOffset(nIndexGloss + 1)))
		{
			nIndexPhonetic = pPhonetic->GetPrevious(nIndexPhonetic);
			bAppendPhonetic = TRUE;
		}
		else
			bAppendPhonetic = FALSE;
	}
	// deselect everything // SDM 1.5Test10.1
	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
	{ pDoc->GetSegment(nLoop)->SetSelection(-1);
	}

	return TRUE;

}

