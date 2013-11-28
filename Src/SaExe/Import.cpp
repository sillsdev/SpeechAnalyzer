#include "stdafx.h"
#include "Import.h"
#include "Settings\OBSTREAM.H"
#include "MainFrm.h"
#include "SA_View.h"
#include "TranscriptionDataSettings.h"
#include "DlgResult.h"
#include "Segment.h"
#include "Sa_graph.h"
#include "DlgImport.h"
#include "GlossSegment.h"
#include "PhoneticSegment.h"
#include "TextHelper.h"
#include "SFMHelper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static LPCSTR psz_Ref = "ref";
static LPCSTR psz_Phonemic = "pm";
static LPCSTR psz_Gloss = "gl";
static LPCSTR psz_Phonetic = "ph";
static LPCSTR psz_Orthographic = "or";
static LPCSTR psz_ImportEnd = "import";

static LPCSTR psz_FreeTranslation = "ft"; // Free Translation
static LPCSTR psz_Language ="ln"; // Language Name
static LPCSTR psz_Dialect = "dlct"; // Dialect
static LPCSTR psz_Family = "fam"; // Family
static LPCSTR psz_Ethno = "id"; // Ethnologue ID number
static LPCSTR psz_Country = "cnt"; // Country
static LPCSTR psz_Region = "reg"; // Region
static LPCSTR psz_Speaker = "spkr"; // Speaker Name
static LPCSTR psz_Gender = "gen"; // Gender
static LPCSTR psz_NotebookReference = "nbr"; // Notebook Reference
static LPCSTR psz_Transcriber = "tr"; // Transcriber
static LPCSTR psz_Comments = "cmnt"; // Comments
static LPCSTR psz_Description = "desc"; // Description
static LPCSTR psz_Table = "table";

CImport::CImport( const CSaString & szFileName, BOOL batch)
{
    m_szPath = szFileName;
    m_bBatch = batch;
}

/***************************************************************************/
// CImport::Import read import file
/***************************************************************************/
BOOL CImport::Import( EImportMode nMode)
{
    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();
    pDoc->CheckPoint();
    pDoc->SetModifiedFlag(TRUE);
    pDoc->SetTransModifiedFlag(TRUE); // transcription has been modified

    BOOL ret = TRUE;

	wstring result;

	if (CSFMHelper::IsColumnarSFM(m_szPath)) {
		ProcessTable( result);
	} else {
		if (!ProcessNormal(nMode,result))
		{
			return FALSE;
		}
	}

    if (!m_bBatch)
    {
		CResult dlg( result.c_str(), AfxGetMainWnd());
		dlg.DoModal();
    }

    return ret;
}

/***************************************************************************/
// CImport::AutoAlign Execute changes by request from batch file
/***************************************************************************/
void CImport::AutoAlign(CSaDoc * pSaDoc, LPCTSTR pReference, LPCTSTR pPhonetic, LPCTSTR pPhonemic, LPCTSTR pOrtho, LPCTSTR pGloss)
{
    CTranscriptionDataSettings settings;

    settings.m_bPhonetic = (pPhonetic != NULL);
    settings.m_bPhonemic = (pPhonemic != NULL);
    settings.m_bOrthographic = (pOrtho != NULL);
    settings.m_bGloss = (pGloss != NULL);
	settings.m_bReference = (pReference != NULL);

    settings.m_bPhoneticModified = (settings.m_bPhonetic!=FALSE);
    settings.m_bPhonemicModified = (settings.m_bPhonemic!=FALSE);
    settings.m_bOrthographicModified = (settings.m_bOrthographic!=FALSE);
    settings.m_bGlossModified = (settings.m_bGloss!=FALSE);
	settings.m_bReferenceModified = (settings.m_bReference!=FALSE);

	if (settings.m_bReference)
	{
		settings.m_szReference = pReference;
	}
    if (settings.m_bPhonetic)
    {
        settings.m_szPhonetic = pPhonetic;
    }
    if (settings.m_bPhonemic)
    {
        settings.m_szPhonemic = pPhonemic;
    }
    if (settings.m_bOrthographic)
    {
        settings.m_szOrthographic = pOrtho;
    }
    if (settings.m_bGloss)
    {
        settings.m_szGloss = pGloss;
    }

    // save state for undo ability
    pSaDoc->CheckPoint();
    pSaDoc->SetModifiedFlag(TRUE); // document has been modified
    pSaDoc->SetTransModifiedFlag(TRUE); // transcription has been modified

    POSITION pos = pSaDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pSaDoc->GetNextView(pos);

	CDWordArray wordOffsets;
	CDWordArray charDurations;
	CDWordArray charOffsets;

    // clean gloss string
    // remove trailing and leading spaces
    settings.m_szGloss.Trim(SPACE_DELIMITER);

    if (pSaDoc->GetSegment(GLOSS)->IsEmpty())
    {
        // auto parse
        if (!pSaDoc->AdvancedParseAuto())
        {
            // process canceled by user
            pSaDoc->Undo(FALSE);
            return;
        }
    }

    CSegment * pSegment=pSaDoc->GetSegment(PHONETIC);
    // Copy gloss segments SDM 1.5Test8.2
    for (int i=0; i<pSaDoc->GetSegment(GLOSS)->GetOffsetSize(); i++)
    {
        DWORD offset = pSaDoc->GetSegment(GLOSS)->GetOffset(i);
        TRACE("word offset %d\n",offset);
        wordOffsets.InsertAt(i,offset);
    }

    // copy segment locations not character counts
    int nIndex = 0;
    while (nIndex != -1)
    {
        DWORD offset = pSegment->GetOffset(nIndex);
        charOffsets.Add(offset);
        TRACE("character offset %d\n",offset);
        DWORD duration = pSegment->GetDuration(nIndex);
        charDurations.Add(duration);
        TRACE("character duration %d\n",offset);
        nIndex = pSegment->GetNext(nIndex);
    }

    CFontTable::tUnit nAlignMode = CFontTable::CHARACTER;

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
        if ((settings.m_bPhonetic) && (settings.m_bPhoneticModified))
        {

            nStringIndex = 0;
            nGlossIndex = 0;
            nWordIndex = 0;
            pSegment = pSaDoc->GetSegment(PHONETIC);
            pTable = pSaDoc->GetFont(PHONETIC);
            pSegment->DeleteContents(); // Delete contents and reinsert from scratch

            nOffsetSize = charOffsets.GetSize();
            for (nIndex = 0; nIndex < (nOffsetSize-1); nIndex++)
            {
                // the line is entered one character per segment
                szNext.Empty();
                while (true)
                {
                    CSaString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
                    if (szTemp.GetLength()==0)
                    {
                        // end of array
						if (szNext.GetLength()==0)
						{
							szNext += SEGMENT_DEFAULT_CHAR;
						}
                        break;
                    }
                    else if ((szTemp.GetLength()==1)&&
								(szTemp[0]==SPACE_DELIMITER))
                    {
                        // time to stop!
                        break;
                    }
                    else if (szTemp.GetLength()>1)
                    {
                        // in some situations if the trailing character is not a break
                        // it will be combined with the space.  we will break it here.
                        if (szTemp[0]==SPACE_DELIMITER)
                        {
                            if (szNext.GetLength()==0)
                            {
                                // remove space and append
                                szTemp.Delete(0,1);
                            }
                            else
                            {
                                // backup and let the next character go into the next segment
                                nStringIndex--;
                                break;
                            }
                        }
                    }
                    szNext += szTemp;
                }
                pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE, charOffsets[nIndex], charDurations[nIndex]);
            }

            szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szPhonetic);
            if (szNext.GetLength()==0)
            {
                szNext+=SEGMENT_DEFAULT_CHAR;
            }
            pSegment->Insert(pSegment->GetOffsetSize(),szNext,FALSE,charOffsets[nOffsetSize-1], charDurations[nOffsetSize-1]);

            // SDM 1.06.8 apply input filter to segment
            if (pSegment->GetInputFilter())
            {
                (pSegment->GetInputFilter())(*pSegment->GetString());
            }
        }

        // Process phonemic
        // SDM 1.06.8 only change  if new segmentation or text changed
        if ((settings.m_bPhonemic) && (settings.m_bPhonemicModified))
        {

            nStringIndex = 0;
            nGlossIndex = 0;
            nWordIndex = 0;
            pSegment = pSaDoc->GetSegment(PHONEMIC);
            pTable = pSaDoc->GetFont(PHONEMIC);
            pSegment->DeleteContents(); // Delete contents and reinsert from scratch

            nOffsetSize = charOffsets.GetSize();
            for (nIndex = 0; nIndex < (nOffsetSize-1); nIndex++)
            {
                // the line is entered one character per segment
                szNext.Empty();
                while (true)
                {
                    CSaString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
                    if (szTemp.GetLength()==0)
                    {
                        break;
                    }
                    else if ((szTemp.GetLength()==1)&&(szTemp[0]==SPACE_DELIMITER))
                    {
                        // time to stop!
                        break;
                    }
                    else if (szTemp.GetLength()>1)
                    {
                        // in some situations if the trailing character is not a break
                        // it will be combined with the space.  we will break it here.
                        if (szTemp[0]==SPACE_DELIMITER)
                        {
                            if (szNext.GetLength()==0)
                            {
                                // remove space and append
                                szTemp.Delete(0,1);
                            }
                            else
                            {
                                // backup and let the next character go into the next segment
                                nStringIndex--;
                                break;
                            }
                        }
                    }
                    szNext += szTemp;
                }
                if (szNext.GetLength()>0)
                {
                    pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,charOffsets[nIndex], charDurations[nIndex]);
                }
            }

            szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szPhonemic);
            // Skip empty segments
            if (szNext.GetLength()!=0)
            {
                pSegment->Insert(pSegment->GetOffsetSize(),szNext,FALSE, charOffsets[nOffsetSize-1], charDurations[nOffsetSize-1]);
            }
            // SDM 1.06.8 apply input filter to segment
            if (pSegment->GetInputFilter())
            {
                (pSegment->GetInputFilter())(*pSegment->GetString());
            }
        }

        // Process orthographic
        // SDM 1.06.8 only change  if new segmentation or text changed
        if ((settings.m_bOrthographic) && (settings.m_bOrthographicModified))
        {

            nStringIndex = 0;
            nGlossIndex = 0;
            nWordIndex = 0;
            pSegment = pSaDoc->GetSegment(ORTHO);
            pTable = pSaDoc->GetFont(ORTHO);
            pSegment->DeleteContents(); // Delete contents and reinsert from scratch

            nOffsetSize = charOffsets.GetSize();
            for (nIndex = 0; nIndex < (nOffsetSize-1); nIndex++)
            {
                // the line is entered one character per segment
                szNext.Empty();
                while (true)
                {
                    CSaString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
                    if (szTemp.GetLength()==0)
                    {
                        break;
                    }
                    else if ((szTemp.GetLength()==1)&&(szTemp[0]==SPACE_DELIMITER))
                    {
                        // time to stop!
                        break;
                    }
                    else if (szTemp.GetLength()>1)
                    {
                        // in some situations if the trailing character is not a break
                        // it will be combined with the space.  we will break it here.
                        if (szTemp[0]==SPACE_DELIMITER)
                        {
                            if (szNext.GetLength()==0)
                            {
                                // remove space and append
                                szTemp.Delete(0,1);
                            }
                            else
                            {
                                // backup and let the next character go into the next segment
                                nStringIndex--;
                                break;
                            }
                        }
                    }
                    szNext += szTemp;
                }
                if (szNext.GetLength()>0)
                {
                    pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,charOffsets[nIndex], charDurations[nIndex]);
                }
            }

            szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szOrthographic);
            if (szNext.GetLength()!=0)
            {
                // Skip empty segments
                pSegment->Insert(pSegment->GetOffsetSize(),szNext,FALSE,charOffsets[nOffsetSize-1], charDurations[nOffsetSize-1]);
            }

            // SDM 1.06.8 apply input filter to segment
            if (pSegment->GetInputFilter())
            {
                (pSegment->GetInputFilter())(*pSegment->GetString());
            }
        }

        // Process reference
        // SDM 1.06.8 only change  if new segmentation or text changed
        if ((settings.m_bReference) && (settings.m_bReferenceModified))
        {

            nStringIndex = 0;
            nGlossIndex = 0;
            nWordIndex = 0;
            pSegment = pSaDoc->GetSegment(REFERENCE);
            pTable = pSaDoc->GetFont(REFERENCE);
            pSegment->DeleteContents();		// Delete contents and reinsert from scratch

            nOffsetSize = charOffsets.GetSize();
            for (nIndex = 0; nIndex < (nOffsetSize-1); nIndex++)
            {
                // the line is entered one character per segment
                szNext.Empty();
                while (true)
                {
                    CSaString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szReference);
                    if (szTemp.GetLength()==0)
                    {
                        break;
                    }
                    else if ((szTemp.GetLength()==1)&&(szTemp[0]==SPACE_DELIMITER))
                    {
                        // time to stop!
                        break;
                    }
                    else if (szTemp.GetLength()>1)
                    {
                        // in some situations if the trailing character is not a break
                        // it will be combined with the space.  we will break it here.
                        if (szTemp[0]==SPACE_DELIMITER)
                        {
                            if (szNext.GetLength()==0)
                            {
                                // remove space and append
                                szTemp.Delete(0,1);
                            }
                            else
                            {
                                // backup and let the next character go into the next segment
                                nStringIndex--;
                                break;
                            }
                        }
                    }
                    szNext += szTemp;
                }
                if (szNext.GetLength()>0)
                {
                    pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,charOffsets[nIndex], charDurations[nIndex]);
                }
            }

            szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szReference);
            // Skip empty segments
            if (szNext.GetLength()!=0)
            {
                pSegment->Insert(pSegment->GetOffsetSize(),szNext,FALSE, charOffsets[nOffsetSize-1], charDurations[nOffsetSize-1]);
            }
            // SDM 1.06.8 apply input filter to segment
            if (pSegment->GetInputFilter())
            {
                (pSegment->GetInputFilter())(*pSegment->GetString());
            }
        }

        // Process gloss
        // SDM 1.5Test8.2 only change if text changed
		// we are assuming the the string has the format of ' #string1 #string2 #string3a string3b #string4'
        if (settings.m_bGlossModified)
        {
            nStringIndex = 0;
            pSegment = pSaDoc->GetSegment(GLOSS);
            pTable = pSaDoc->GetFont(GLOSS);

            // align gloss by word SDM 1.5Test8.2
            nAlignMode = CFontTable::DELIMITEDWORD;
            nOffsetSize = wordOffsets.GetSize();
            // Don't Select this segment SDM 1.5Test8.2
            pSegment->SelectSegment(*pSaDoc,-1);

            // the gloss table uses a space as a delimiter,
            // the normally the text is delimited with a #.
            // if we see a # in the first position, we will continue
            // to scan the segments util we find the next #
			bool first = true;
            for (nIndex = 0; nIndex < (nOffsetSize-1); nIndex++)
            {
				szNext = "";
				while (true)
				{
					int lastIndex = nStringIndex;
					CString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szGloss);
					if (szTemp.GetLength()==0)
					{
						if (first)
						{
							// this is just a leading space in the string - ignore it
							first = false;
						}
						else
						{
							// a space only - we are done.
							szNext.AppendChar(WORD_DELIMITER);
							break;
						}
					}
					else
					{
						first = false;
						if (szTemp[0]==WORD_DELIMITER)
						{
							// do nothing this is the beginning of the line
							if (szNext.GetLength()==0)
							{
								// this is the beginning of the line and carry on
								szNext.Append(szTemp);
							}
							else
							{
								// this is the next line.  back up and stop
								nStringIndex = lastIndex;
								break;
							}
						}
						else
						{
							// an embedded space - append
							szNext.AppendChar(SPACE_DELIMITER);
							szNext.Append(szTemp);
						}
					}
				}

                szNext.Remove(0x0d);
                szNext.Remove(0x0a);
                pSegment->SelectSegment(*pSaDoc,nIndex);
                ((CGlossSegment *)pSegment)->ReplaceSelectedSegment(pSaDoc,szNext);
            };

            // take care of remainder
            szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szGloss);
            if (szNext.GetLength()==0)
            {
                szNext.AppendChar(WORD_DELIMITER);
            }
            else
            {
                if (szNext[0]==WORD_DELIMITER)
                {
                    // do nothing
                }
                else
                {
                    szNext.Insert(0,WORD_DELIMITER);
                }
            }
            szNext.Remove(0x0d);
            szNext.Remove(0x0a);
            pSegment->SelectSegment(*pSaDoc,nIndex);
            ((CGlossSegment *)pSegment)->ReplaceSelectedSegment(pSaDoc,szNext);
        }
    }

    pView->ChangeAnnotationSelection(pSegment, -1);

    CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);
    if (pGraph)
    {
        if (settings.m_bPhonetic)
        {
            pGraph->ShowAnnotation(PHONETIC, TRUE, TRUE);
        }
        if (settings.m_bPhonemic)
        {
            pGraph->ShowAnnotation(PHONEMIC, TRUE, TRUE);
        }
        if (settings.m_bOrthographic)
        {
            pGraph->ShowAnnotation(ORTHO, TRUE, TRUE);
        }
        if (settings.m_bGloss)
        {
            pGraph->ShowAnnotation(GLOSS, TRUE, TRUE);
        }
		if (settings.m_bReference)
		{
			pGraph->ShowAnnotation(REFERENCE, TRUE, TRUE);
		}
    }
    pView->RefreshGraphs(); // redraw all graphs without legend window
}

static void CreateWordSegments(const int nWord, int & nSegments)
{
    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    if (pDoc->GetSegment(GLOSS)->GetOffsetSize() > nWord)
    {
        DWORD dwStart;
        DWORD dwStop;
        int nPhonetic;
        CPhoneticSegment * pPhonetic = (CPhoneticSegment *) pDoc->GetSegment(PHONETIC);

        if (nWord == -1)
        {
            dwStart = 0;
            if (pDoc->GetSegment(GLOSS)->IsEmpty())
            {
                dwStop = pDoc->GetDataSize();
            }
            else
            {
                dwStop = pDoc->GetSegment(GLOSS)->GetOffset(0);
            }
            if (dwStart + pDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME) > dwStop)
            {
                return;
            }
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
            if (!nSegments)
            {
                nSegments = 1;
            }
            if (nSegments*pDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME) > (dwStop -dwStart))
            {
                return;
            }
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
                if (nIndex >= pPhonetic->GetOffsetSize())
                {
                    break;
                }
            }
            else
            {
                DWORD dwBegin = dwStart + nCount;
                pPhonetic->Adjust(pDoc, nIndex, dwBegin, 1);
                nIndex = pPhonetic->GetNext(nIndex);
                nCount++;
            }
        }
        if (nSegments == 0)
        {
            return;    // done
        }
        // add segments
        while (nCount < nSegments)
        {
            if (nIndex == -1)
            {
                nIndex = pPhonetic->GetOffsetSize();
            }
            DWORD dwBegin = dwStart + nCount;
            CSaString szEmpty(SEGMENT_DEFAULT_CHAR);
            pPhonetic->Insert(nIndex, szEmpty, FALSE, dwBegin, 1);
            nIndex = pPhonetic->GetNext(nIndex);
            nCount++;
        }
        // adjust segment spacing
        DWORD dwSize = (dwStop - dwStart)/nSegments;
        if (pDoc->Is16Bit())
        {
            dwSize &= ~1;
        };
        dwSize += pDoc->GetBlockAlign();
        if (nIndex == -1)
        {
            nIndex = pPhonetic->GetOffsetSize();
        }
        nIndex = pPhonetic->GetPrevious(nIndex);
        while ((nIndex != -1)&&(pPhonetic->GetOffset(nIndex) >= dwStart))
        {
            nCount--;
            DWORD dwBegin = dwStart+nCount*dwSize;
            DWORD dwDuration = dwSize;
            if ((dwBegin + dwDuration) > dwStop)
            {
                dwDuration = dwStop - dwBegin;
            }
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
BOOL CImport::ReadTable( CObjectIStream & ostream, int nMode)
{
    CSaView * pView = (CSaView *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView();

    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

    const int MAXLINE = 32000;
    char * pUtf8 = new char[MAXLINE];
    CSaString szLine;

    // eat table marker
    ostream.getIos().getline(pUtf8,MAXLINE);
    szLine.setUtf8(pUtf8);
    if (szLine.GetLength() >= (MAXLINE - 1))   // error
    {
        return FALSE;
    }
    if (szLine.Find(CString(psz_Table)) ==-1)   // error
    {
        return FALSE;
    }

    // read header
    ostream.getIos().getline(pUtf8,MAXLINE);
    szLine.setUtf8(pUtf8);
    if (szLine.GetLength() >= (MAXLINE - 1))   // error
    {
        return FALSE;
    }

    // parse header
    int nAnnotField[ANNOT_WND_NUMBER+1];
    CSaString szField;

    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER+1; nLoop++)
    {
        nAnnotField[nLoop] = -1;
    }


    for (int nLoop = 0; nLoop < 20; nLoop++)
    {
        szField = CSFMHelper::ExtractTabField(szLine, nLoop);

        if (szField.Find(_T("Time")) != -1)
        {
            return FALSE;    // table is not built on phonetic segment boundaries
        }
        else if (szField.Find(_T("Ref")) != -1)
        {
            nAnnotField[REFERENCE] = nLoop;
        }
        else if (szField.Find(_T("Etic")) != -1)
        {
            nAnnotField[PHONETIC] = nLoop;
        }
        else if (szField.Find(_T("Tone")) != -1)
        {
            nAnnotField[TONE] = nLoop;
        }
        else if (szField.Find(_T("Emic")) != -1)
        {
            nAnnotField[PHONEMIC] = nLoop;
        }
        else if (szField.Find(_T("Ortho")) != -1)
        {
            nAnnotField[ORTHO] = nLoop;
        }
        else if (szField.Find(_T("Gloss")) != -1)
        {
            nAnnotField[GLOSS] = nLoop;
        }
        else if (szField.Find(_T("POS")) != -1)
        {
            nAnnotField[ANNOT_WND_NUMBER] = nLoop;
        }
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
        streampos pos = ostream.getIos().tellg();  // save top of file position

        // find number of segments in each word
        int nSegmentCount = 0;
        int nWords = 0;
        int nSegmentToBeginWord[MAX_INT];
        while (ostream.getIos().peek() != EOF)
        {
            // read line
            ostream.getIos().getline(pUtf8,MAXLINE);
            szLine.setUtf8(pUtf8);
            if (szLine.GetLength() >= (MAXLINE - 1))   // error
            {
                return FALSE;
            }
            if (CSFMHelper::ExtractTabField(szLine, nAnnotField[GLOSS]).GetLength())   // gloss found
            {
                nSegmentToBeginWord[nWords] = nSegmentCount;
                nWords++;
            }
            nSegmentCount++;
        }
        ostream.getIos().seekg(pos);  // return to top of table
        ostream.getIos().clear();  // clear the EOF flag
        if (nSegmentCount == 0)
        {
            nSegmentCount = 1;
        }
        if (nWords == 0)
        {
            nWords = 1;
        }
        nSegmentToBeginWord[nWords] = nSegmentCount;

        // add, remove and adjust segments as needed
        CPhoneticSegment * pPhonetic = (CPhoneticSegment *) pDoc->GetSegment(PHONETIC);
        CSaString szEmpty(SEGMENT_DEFAULT_CHAR);
        CSaString szEmptyGloss = "";
        CGlossSegment * pGloss = (CGlossSegment *)pDoc->GetSegment(GLOSS);
        DWORD dwFileLength = pDoc->GetDataSize();
        int nIndex = pPhonetic->FindFromPosition(0);
        int nWordCurr = 0;
        DWORD dwStart = 0;
        DWORD dwDuration = 0;
        while (nWordCurr < nWords || nIndex != -1)
        {
            CSaString szTest = pPhonetic->GetText(nIndex);
            if (nWordCurr < nWords)
            {
                dwDuration = (DWORD)((float)dwFileLength * (float)(nSegmentToBeginWord[nWordCurr + 1] - nSegmentToBeginWord[nWordCurr]) / (float)nSegmentCount);
                if (nIndex != -1)   // adjust existing segments
                {
                    pPhonetic->Adjust(pDoc, nWordCurr, dwStart, dwDuration);
                }
                else     // add segments
                {
                    pPhonetic->Insert(nWordCurr, szEmpty, FALSE, dwStart, dwDuration);
                }
                pGloss->Insert(nWordCurr, szEmptyGloss, 0, dwStart, dwDuration);
                nIndex = nWordCurr;
                nWordCurr++;
                dwStart += dwDuration;
            }
            else     // remove extra segments
            {
                pPhonetic->SetSelection(nIndex);
                pPhonetic->Remove(pDoc, FALSE);
                nIndex--;
            }
            nIndex = pPhonetic->GetNext(nIndex);
        }
    }

    if ((nMode == MANUAL) && (nAnnotField[GLOSS] == -1))
    {
        nMode = AUTO;
    }

    if (nMode == MANUAL)
    {
        streampos pos = ostream.getIos().tellg();  // save top of file position

        int nSegmentCount = 0;
        int nWordCount = -1;
        while (ostream.getIos().peek() != EOF)
        {
            // read line
            ostream.getIos().getline(pUtf8,MAXLINE);
            szLine.setUtf8(pUtf8);
            if (szLine.GetLength() >= (MAXLINE - 1))   // error
            {
                return FALSE;
            }
            if (CSFMHelper::ExtractTabField(szLine, nAnnotField[GLOSS]).GetLength())   // gloss found
            {
                CreateWordSegments(nWordCount, nSegmentCount);
                nWordCount++;
            };
            nSegmentCount++;
        }
        CreateWordSegments(nWordCount, nSegmentCount);
        ostream.getIos().seekg(pos);  // return to top of table
        ostream.getIos().clear();  // clear the EOF flag
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
    CSegment * pPhonetic = pDoc->GetSegment(PHONETIC);
    CGlossSegment * pGloss = (CGlossSegment *) pDoc->GetSegment(GLOSS);
    CSaString szString = WORD_DELIMITER;
    if (nAnnotField[GLOSS] != -1)
    {
        for (int nIndex = 0; nIndex < pGloss->GetOffsetSize(); nIndex++)
        {
            if (pGloss->GetSelection() != nIndex)
            {
                pGloss->SetSelection(nIndex);
            }
            pGloss->ReplaceSelectedSegment(pDoc, szString);
        }
    }
    szString = SEGMENT_DEFAULT_CHAR;
    if (nAnnotField[PHONETIC] != -1)
    {
        for (int nIndex = 0; nIndex < pPhonetic->GetOffsetSize(); nIndex++)
        {
            pView->SelectFromPosition( PHONETIC, pPhonetic->GetOffset(nIndex), CSegmentSelection::FIND_EXACT);
            pView->SetSelectedAnnotationString( szString, TRUE, FALSE);
        }
    }
    for (int nIndex = PHONETIC+1; nIndex < ANNOT_WND_NUMBER; nIndex++)
    {
        if ((nAnnotField[nIndex] != -1)&& (nIndex != GLOSS))
        {
            pDoc->GetSegment(nIndex)->DeleteContents();
        }
    }
    if (nAnnotField[ANNOT_WND_NUMBER/* POS*/] != -1)
    {
        for (int nIndex = 0; nIndex < pDoc->GetSegment(GLOSS)->GetOffsetSize(); nIndex++)
        {
            ((CGlossSegment *)pDoc->GetSegment(GLOSS))->POSSetAt( nIndex, L"");
        }
    }

    // insert fields into segments
    int nIndexPhonetic = 0;
    int nIndexGloss = -1;
    BOOL bAppendGloss;
    BOOL bAppendPhonetic = FALSE;
    if (pPhonetic->IsEmpty())
    {
        return FALSE;    // no where to go
    }
    while (ostream.getIos().peek() != EOF)
    {
        // read line
        ostream.getIos().getline(pUtf8,MAXLINE);
        szLine.setUtf8(pUtf8);
        if (szLine.GetLength() >= (MAXLINE - 1))   // error
        {
            return FALSE;
        }

        szString = CSFMHelper::ExtractTabField(szLine, nAnnotField[GLOSS]);
        if (szString.GetLength())   // gloss found
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
            {
                szString = pGloss->GetSegmentString(nIndexGloss) + " " + szString;
            }
            if (pGloss->GetSelection() != nIndexGloss)
            {
                pGloss->SetSelection(nIndexGloss);
            }
            if ((szString[0] != WORD_DELIMITER) || (szString[0] != TEXT_DELIMITER))
            {
                szString = CSaString(SPACE_DELIMITER) + CSaString(WORD_DELIMITER) + szString;
            }
            pGloss->ReplaceSelectedSegment(pDoc, szString);

            // POS
            szString = CSFMHelper::ExtractTabField(szLine, nAnnotField[ANNOT_WND_NUMBER/*POS*/]);
            if (szString.GetLength())
            {
                if (bAppendGloss)
                {
                    szString = pGloss->GetPOSAt(nIndexGloss) + " " + szString;
                }
                pGloss->POSSetAt( nIndexGloss, szString);
            }
            // Reference
            szString = CSFMHelper::ExtractTabField(szLine, nAnnotField[REFERENCE]);
            if (szString.GetLength())
            {
                pView->SelectFromPosition( REFERENCE, pGloss->GetOffset(nIndexGloss), CSegmentSelection::FIND_EXACT);
                if (bAppendGloss)
                {
                    szString = pView->GetSelectedAnnotationString(FALSE) + " " + szString;
                }
                pView->SetSelectedAnnotationString( szString, TRUE, FALSE);
            }
        }
        for (int nIndex = PHONETIC; nIndex < GLOSS; nIndex++)
        {
            szString = CSFMHelper::ExtractTabField(szLine, nAnnotField[nIndex]);
            if (szString.GetLength())
            {
                pView->SelectFromPosition( nIndex, pPhonetic->GetOffset(nIndexPhonetic), CSegmentSelection::FIND_EXACT);
                if (bAppendPhonetic)
                {
                    szString = pView->GetSelectedAnnotationString(FALSE) + /*" " +*/ szString;    // SDM 1.5Test10.7 remove spaces
                }
                pView->SetSelectedAnnotationString( szString, TRUE, FALSE);
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
        {
            bAppendPhonetic = FALSE;
        }
    }
    // deselect everything // SDM 1.5Test10.1
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        pDoc->GetSegment(nLoop)->SetSelection(-1);
    }
    return TRUE;
}

/**
* isTabular should have already confirmed that this is a tabular file
*/
BOOL CImport::ProcessTable( wstring & result)
{
	result.clear();

	wstring obuffer;
	if (!ConvertFileToUTF16( m_szPath, obuffer))
	{
		return FALSE;
	}
	
	vector<wstring> lines = TokenizeBufferToLines( obuffer);

	lines = CSFMHelper::FilterBlankLines(lines);

	if (lines.size()==0) return false;

	size_t start = 0;
	while (true) 
	{
		if (start>=lines.size()) return false;
		if (lines[start].length()>0) break;
		start++;
	}

	// we are now sitting at the first non-blank line
	wstring tagline = lines[start];
	vector<wstring> tags = TokenizeLineToTokens( tagline, 0x09);
	// the first line must be all tags.
	for (size_t i=0;i<tags.size();i++) {
		if (!CSFMHelper::IsTag(tags[i].c_str())) {
			TRACE("the first line contains an element that is not a tag '%s'\n",tagline.c_str());
			return false;
		}
	}
	size_t tagCount = tags.size();
	start++;
	
	// run through all the lines and verify that the counts are no greater
	// than the number of tags
	for (size_t i=start;i<lines.size();i++) {
		vector<wstring> tokens = TokenizeLineToTokens( lines[i], 0x09);
		if (tokens.size()>tagCount) {
			TRACE("line %d has too many elements '%s'.  The tag count id %d\n",i,lines[i].c_str(),tagCount);
			return false;
		}
	}

	// run through all the lines and verify that there are no more tags
	// than the number of tags
	for (size_t i=start;i<lines.size();i++) {
		vector<wstring> tokens = TokenizeLineToTokens( lines[i], 0x09);
		for (size_t j=0;j<tokens.size();j++) {
			if (CSFMHelper::IsTag(tokens[j].c_str())) {
				TRACE("line %d contains a tag '%s'.\n",i,lines[i].c_str());
				return false;
			}
		}
	}

	// create as many rows as there are tags
	vector<wstring> rows;
	for (size_t i=0;i<tags.size();i++)
	{
		rows.push_back(tags[i]);
	}

	// now append the data to the rows
	for (size_t i=start;i<lines.size();i++)
	{
		vector<wstring> tokens = TokenizeLineToTokens( lines[i], 0x09);
		for (size_t j=0;j<tags.size();j++)
		{
			bool gloss = false;
			if (CSFMHelper::IsGloss(tags[j].c_str(),tags[j].length())) {
				gloss = true;
			}
			wstring data = L"";
			if (gloss)
			{
				// we want to assure the sequence is SPACE_DELIMITER,WORD_DELIMITER,text
				if (j<tokens.size())
				{
					if (tokens[j].length()>0)
					{
						if (tokens[j][0]==SPACE_DELIMITER)
						{
							if (tokens[j].length()>1)
							{
								if (tokens[j][1]==WORD_DELIMITER)
								{
									// do nothing
								}
								else
								{
									data.push_back(WORD_DELIMITER);
								}
							}
							else
							{
								data.push_back(WORD_DELIMITER);
							}
						}
						else if (tokens[j][0]==WORD_DELIMITER)
						{
							data.push_back(SPACE_DELIMITER);
						}
						else
						{
							data.push_back(SPACE_DELIMITER);
							data.push_back(WORD_DELIMITER);
						}
						data.append(tokens[j]);
					}
					else
					{
						data.push_back(SPACE_DELIMITER);
						data.push_back(WORD_DELIMITER);
						data.append(tokens[j]);
					}
				}
				else
				{
					data.push_back(SPACE_DELIMITER);
					data.push_back(WORD_DELIMITER);
				}
				rows[j].append(data);
			}
			else
			{
				if (j<tokens.size())
				{
					data.append(tokens[j]);
				}
				else
				{
					data = L"";
				}
				rows[j].append(L" ");
				rows[j].append(data);
			}
		}
	}

	wstring ref;
	wstring phonetic;
	wstring phonemic;
	wstring ortho;
	wstring gloss;

	for (size_t i=0;i<rows.size();i++)
	{
		if (CSFMHelper::IsPhonetic(rows[i].c_str(),rows[i].length()))
		{
			phonetic = rows[i].c_str();
		}
		else if (CSFMHelper::IsPhonemic(rows[i].c_str(),rows[i].length()))
		{
			phonemic = rows[i].c_str();
		}
		else if (CSFMHelper::IsOrthographic(rows[i].c_str(),rows[i].length()))
		{
			ortho = rows[i].c_str();
		}
		else if (CSFMHelper::IsGloss(rows[i].c_str(),rows[i].length()))
		{
			gloss = rows[i].c_str();
		}
		else if (CSFMHelper::IsRef(rows[i].c_str(),rows[i].length()))
		{
			ref = rows[i].c_str();
		}
	}

	CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();
	phonetic = (phonetic.length()>4)?phonetic.substr(4):phonetic;
	phonemic = (phonemic.length()>4)?phonemic.substr(4):phonemic;
	ortho = (ortho.length()>4)?ortho.substr(4):ortho;
	gloss = (gloss.length()>4)?gloss.substr(4):gloss;
	ref = (ref.length()>5)?ref.substr(5):ref;

	AutoAlign( pDoc, ref.c_str(), phonetic.c_str(), phonemic.c_str(), ortho.c_str(), gloss.c_str());

	// now build the result string
	for (size_t i=0;i<rows.size();i++)
	{
		result.append(rows[i].c_str());
		result.append(L"\r\n");
	}

	return true;
}

BOOL CImport::ProcessNormal( EImportMode nMode, wstring & result)
{
    CSaString text;
	CSaString ref;
    CSaString phonetic;
    CSaString phonemic;
    CSaString ortho;
    CSaString gloss;

    const CSaString CrLf("\r\n");
    BOOL bTable = FALSE;
    CSaString imported("Imported...\r\n");
    CSaString skipped("Skipped... \r\n");

	result.clear();

	CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();
    try
    {
		CObjectIStream ostream(m_szPath.utf8().c_str());
        if (!ostream.bAtBackslash())
        {
            ostream.SkipBOM();
            if (!ostream.bAtBackslash())
            {
                return FALSE;
            }
        }
        while (!ostream.bAtEnd())
        {
            streampos pos = ostream.getIos().tellg();
            
			const char * pszMarkerRead = NULL;
			char buffer[1024];
			memset(buffer,0,_countof(buffer));
            ostream.PeekMarkedString( &pszMarkerRead, buffer, _countof(buffer));
            CSaString szStringRead;
			szStringRead.setUtf8(buffer);

            if (ReadStreamString( ostream, psz_Table, text))
            {
                ostream.getIos().seekg(pos);  // start before marker
                ostream.getIos().clear();
                BOOL result = ReadTable(ostream, nMode);
                if (result)
                {
					imported += "\\";
					imported += pszMarkerRead;
					imported += " (Entire Table)";
					imported += "\r\n";
                    bTable = TRUE;
                }
                else
                {
                    skipped += "\\";
					skipped += pszMarkerRead;
					skipped += " (Entire Table)";
					skipped += "\r\n";
                }
                break;  // this must be last marker
            }
            else if (ReadStreamString( ostream,psz_FreeTranslation, text))
            {
                pDoc->GetSourceParm()->szFreeTranslation = text;
            }
            else if (ReadStreamString( ostream,psz_Language, text))
            {
                pDoc->GetSourceParm()->szLanguage = text;
            }
            else if (ReadStreamString( ostream,psz_Dialect, text))
            {
                pDoc->GetSourceParm()->szDialect = text;
            }
            else if (ReadStreamString( ostream,psz_Family, text))
            {
                pDoc->GetSourceParm()->szFamily = text;
            }
            else if (ReadStreamString( ostream,psz_Ethno, text))
            {
                pDoc->GetSourceParm()->szEthnoID = text;
            }
            else if (ReadStreamString( ostream,psz_Country, text))
            {
                pDoc->GetSourceParm()->szCountry = text;
            }
            else if (ReadStreamString( ostream,psz_Region, text))
            {
                pDoc->GetSourceParm()->szRegion = text;
            }
            else if (ReadStreamString( ostream,psz_Speaker, text))
            {
                pDoc->GetSourceParm()->szSpeaker = text;
            }
            else if (ReadStreamString( ostream,psz_Gender, text))
            {
                int nGender = pDoc->GetSourceParm()->nGender;

                text.MakeUpper();

                if (text == "ADULT MALE")
                {
                    nGender = 0;    // male
                }
                else if (text == "ADULT FEMALE")
                {
                    nGender = 1;    // female
                }
                else if (text == "CHILD")
                {
                    nGender = 2;    // child
                }

                pDoc->GetSourceParm()->nGender = nGender;
            }
            else if (ReadStreamString( ostream,psz_NotebookReference, text))
            {
                pDoc->GetSourceParm()->szReference = text;
            }
            else if (ReadStreamString( ostream,psz_Transcriber, text))
            {
                pDoc->GetSourceParm()->szTranscriber = text;
            }
            else if (ReadStreamString( ostream,psz_Comments, text))
            {
                pDoc->GetSaParm()->szDescription = text;
            }
            else if (ReadStreamString( ostream,psz_Description, text))
            {
                pDoc->GetSaParm()->szDescription = text;
            }
            else if (ReadStreamString( ostream,psz_Phonetic, text))
            {
                phonetic = text;
                continue;
            }
            else if (ReadStreamString( ostream,psz_Phonemic, text))
            {
                phonemic = text;
                continue;
            }
            else if (ReadStreamString( ostream,psz_Orthographic, text))
            {
                ortho = text;
                continue;
            }
            else if (ReadStreamString( ostream,psz_Gloss, text))
            {
				bool first = true;
				bool hasSpaces = (text.Find(SPACE_DELIMITER)!=-1);
				bool hasPounds = (text.Find(WORD_DELIMITER)!=-1);
				if (hasPounds)
				{
					// if it has pounds, then make sure that a space preceeds each pound.
					// spaces without pounds are treated as embedded spaces
					for (int i=0;i<text.GetLength();)
					{
						if (text[i]==SPACE_DELIMITER)
						{
							if ((i+1)<text.GetLength())
							{
								if (text[i+1]==WORD_DELIMITER)
								{
									gloss += text[i++];
									gloss += text[i++];
								}
								else
								{
									// space followed by something else - treat as embedded space
									gloss += text[i++];
								}
							}
							else
							{
								// we are done.
								gloss += text[i++];
							}
						}
						else if (text[i]==WORD_DELIMITER)
						{
							// if we are here, we didn't see a preceeding space - add one
							gloss += CSaString(SPACE_DELIMITER);
							gloss += text[i++];
						}
						else
						{
							gloss += text[i++];
						}
					}
				}
				else if (hasSpaces)
				{
					// if it doesn't have pounds then convert spaces to space/pounds sequences
					for (int i=0;i<text.GetLength();)
					{
						if (text[i]==SPACE_DELIMITER)
						{
							gloss += text[i++];
							gloss += CSaString(WORD_DELIMITER);
						}
						else
						{
							if (first)
							{
								gloss += CSaString(SPACE_DELIMITER);
								gloss += CSaString(WORD_DELIMITER);
								first=false;
							}
							gloss += text[i++];
						}
						first=false;
					}
				}
				else
				{
					// neither? add something in front
					gloss += CSaString(SPACE_DELIMITER);
					gloss += CSaString(WORD_DELIMITER);
					gloss += text;

				}
                continue;
            }
            else if (ostream.bEnd(psz_ImportEnd))
            {
                break;
            }
            else
            {
                skipped += "\\";
				skipped += pszMarkerRead;
				skipped += " ";
				skipped += szStringRead;
				skipped += "\r\n";
                continue;
            }
            imported += "\\";
			imported += pszMarkerRead;
			imported += " ";
			imported += szStringRead;
			imported += L"\r\n";
        }
    }
    catch (...)
    {
    }

	if ((phonetic.GetLength()!=0) || 
		(phonemic.GetLength()!=0) || 
		(ortho.GetLength()!=0) || 
		(gloss.GetLength()!=0) ||
		(ref.GetLength()!=0))
    {
        if (!bTable)
        {
            AutoAlign(pDoc, ref, phonetic, phonemic, ortho, gloss);
        }

        CSaString Report;

        if (phonetic.GetLength()!=0)
        {
            Report += "\\" + CSaString(psz_Phonetic) + " " + phonetic + CrLf;
			phonetic = "";
        }
        if (phonemic.GetLength()!=0)
        {
            Report += "\\" + CSaString(psz_Phonemic) + " " + phonemic + CrLf;
			phonemic = "";
        }
        if (ortho.GetLength()!=0)
        {
            Report += "\\" + CSaString(psz_Orthographic) + " " + ortho + CrLf;
			ortho = "";
        }
        if (gloss.GetLength()!=0)
        {
            Report += "\\" + CSaString(psz_Gloss) + gloss + CrLf;
			gloss = "";
        }

        if (bTable)
        {
            skipped += Report;
        }
        else
        {
            imported += Report;
        }
	}
	result.append(imported);
	result.append(L"\r\n");
	result.append(skipped);
	return TRUE;
}
