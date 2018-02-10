#include "StdAfx.h"
#include "AutoSegmentation.h"
#include "MainFrm.h"
#include "PhoneticSegment.h"
#include "GlossSegment.h"
#include "GlossNatSegment.h"
#include "ReferenceSegment.h"
#include "ToneSegment.h"
#include "PhonemicSegment.h"
#include "OrthoSegment.h"
#include "TranscriptionData.h"
#include "Process\SA_P_GRA.H"
#include <iomanip>
#include "Sa_Doc.h"
#include "SA_View.h"
#include "FileUtils.h"

#define BREAK_WIDTH 300.0f

#define MIN_PHONE_ERROR 0.70f
#define MAX_PHONE_ERROR 1.30f

#define MIN_WORD_ERROR 0.50f
#define MAX_WORD_ERROR 2.00f

#define LIMIT_MAX 2.0f
#define LIMIT_MIN 0.5f

vector<Verse> CAutoSegmentation::GetVerses( CTranscriptionData & td, bool usingGL) {

	vector<Verse> result;
	CString ref = td.m_szPrimary;
	MarkerList rf = td.GetMarkerList(REFERENCE);
	MarkerList gl = td.GetMarkerList(GLOSS);
	MarkerList gn = td.GetMarkerList(GLOSS_NAT);
    TranscriptionDataMap & map = td.m_TranscriptionData;
    MarkerList::iterator rit = map[ref].begin();
    MarkerList::iterator rend = map[ref].end();
    MarkerList::iterator glit = gl.begin();
    MarkerList::iterator glend = gl.end();
    MarkerList::iterator gnit = gn.begin();
    MarkerList::iterator gnend = gn.end();
	glit = gl.begin();
	if (usingGL) {
		while (glit!=glend) {
			wstring text = (*glit).Trim();
			glit++;
			// insert reference
			wstring ref2;
			if (rit!=rend) {
				ref2 = *rit;
				rit++;
			}
			size_t wordCount = GetWordCount(text);
			size_t phoneCount = GetNonVoicedCharacterCount(text);
			Verse entry( ref2.c_str(), text.c_str(), wordCount, phoneCount);
			result.push_back(entry);
		}
	} else {
		// using gloss national
		while (gnit!=gnend) {
			// insert gloss
			wstring text = (*gnit).Trim();
			gnit++;
			// insert reference
			wstring ref2;
			if (rit!=rend) {
				ref2 = *rit;
				rit++;
			}
			size_t wordCount = GetWordCount(text);
			size_t phoneCount = GetNonVoicedCharacterCount(text);
			Verse entry( ref2.c_str(), text.c_str(), wordCount, phoneCount);
			result.push_back(entry);
		}
	}
	return result;
}

vector<Phonetic> CAutoSegmentation::GetPhones( CSaDoc & doc, wofstream & ofs ) {

	vector<Phonetic> result;

	CMainFrame * pMainFrame = (CMainFrame*)AfxGetMainWnd();
	pMainFrame->SetSegmentParams(20.0f/1000.0f,16,18);

	// perform phonetic segmentation and create pattern
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)doc.GetSegment(PHONETIC);
	pPhonetic->DeleteContents();

	// for the case of cancelled dependent processes
    doc.RestartAllProcesses(); 
	// for the case of a cancelled process
    pPhonetic->RestartProcess();
	// clear data from previous run SDM 1.06.4
    pPhonetic->SetDataInvalid(); 

	// process data
    short int nResult = LOWORD(pPhonetic->Process(NULL, &doc)); 
    if (nResult == PROCESS_ERROR) {
        // error segmenting
        doc.ErrorMessage(IDS_ERROR_SEGMENT);
        return result;
    }
    if (nResult == PROCESS_CANCELED) {
        // error cancelled segmenting
        doc.ErrorMessage(IDS_CANCELED);
        return result;
    }

	// formants need pitch info
	// SDM 1.5 Test 11.0
    CProcessGrappl * pPitch = doc. GetGrappl(); 
	// process data
    nResult = LOWORD(pPitch->Process( &doc, &doc)); 
    if (nResult == PROCESS_ERROR) {
		doc.ErrorMessage(IDS_ERROR_GENERATE_CV_NOPITCH);
		return result;
    } else if (nResult == PROCESS_CANCELED) {
        doc.ErrorMessage(IDS_CANCELED);
        return result;
	}

	double sizeFactor = (double)doc.GetDataSize() / (double)(pPitch->GetDataSize() - 1);

	/**
	* We index the pitch data using the midpoint of the current segment
	* We need to scale the index as well
	*/
	for (int i=0;i<pPhonetic->GetOffsetSize();i++) {
		DWORD offset = pPhonetic->GetOffset(i);
		DWORD length = pPhonetic->GetDuration(i);
		Phonetic phonetic;
		phonetic.start = pPhonetic->GetOffset(i);
		phonetic.stop = pPhonetic->GetStop(i);
		int dwSamples = 0;
		BOOL bRes = TRUE;
		long nData = 0;
		DWORD index = (offset + (length/2));
		DWORD data = (DWORD)(index / sizeFactor);
		// get data for this pixel
		// SDM 1.5Test11.0
		int nHere = pPitch->GetProcessedData( data, &bRes); 
		if (nHere > 0) {
			nData += nHere;
			dwSamples++;
		}
		//if ((dwSamples>0) && (bRes)) {
			//double fData = double(nData) / PRECISION_MULTIPLIER / dwSamples;
			//TRACE("pitch value = %f\n",fData);
		//} else {
			//TRACE("pitch value = none\n");
		//}
		result.push_back(phonetic);
	}

	ofs << L"there are " << result.size() << L" phones" << "\n";
	return result;
}

vector<Word> CAutoSegmentation::GetWords( CSaDoc & doc, wofstream & ofs ) {

	vector<Word> result;

	CMainFrame * pMainFrame = (CMainFrame*)AfxGetMainWnd();
	pMainFrame->SetSegmentParams(75.0f/1000.0f,5,23);

	// perform phonetic segmentation and create pattern
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)doc.GetSegment(PHONETIC);
	pPhonetic->DeleteContents();

	// for the case of cancelled dependent processes
    doc.RestartAllProcesses(); 
	// for the case of a cancelled process
    pPhonetic->RestartProcess();
	// clear data from previous run SDM 1.06.4
    pPhonetic->SetDataInvalid(); 

	// process data
    short int nResult = LOWORD(pPhonetic->Process(NULL, &doc)); 
    if (nResult == PROCESS_ERROR) {
        // error segmenting
        doc.ErrorMessage(IDS_ERROR_SEGMENT);
        return result;
    }
    if (nResult == PROCESS_CANCELED) {
        // error cancelled segmenting
        doc.ErrorMessage(IDS_CANCELED);
        return result;
    }

	// formants need pitch info
	// SDM 1.5 Test 11.0
    CProcessGrappl * pPitch = doc. GetGrappl(); 
	// process data
    nResult = LOWORD(pPitch->Process( &doc, &doc)); 
    if (nResult == PROCESS_ERROR) {
		doc.ErrorMessage(IDS_ERROR_GENERATE_CV_NOPITCH);
		return result;
    } else if (nResult == PROCESS_CANCELED) {
        doc.ErrorMessage(IDS_CANCELED);
        return result;
	}

	double sizeFactor = (double)doc.GetDataSize() / (double)(pPitch->GetDataSize() - 1);

	/**
	* We index the pitch data using the midpoint of the current segment
	* We need to scale the index as well
	*/
	for (int i=0;i<pPhonetic->GetOffsetSize();i++) {
		DWORD offset = pPhonetic->GetOffset(i);
		DWORD length = pPhonetic->GetDuration(i);
		Word word;
		word.start = pPhonetic->GetOffset(i);
		word.stop = pPhonetic->GetStop(i);
		int dwSamples = 0;
		BOOL bRes = TRUE;
		long nData = 0;
		DWORD index = (offset + (length/2));
		DWORD data = (DWORD)(index / sizeFactor);
		// get data for this pixel
		// SDM 1.5Test11.0
		int nHere = pPitch->GetProcessedData( data, &bRes); 
		if (nHere > 0) {
			nData += nHere;
			dwSamples++;
		}
		result.push_back(word);
	}

	ofs << L"there are " << result.size() << L" words" << "\n";
	return result;
}

/**
* Divide and conquer
* Reiteratively determines the proper delay gap to locate the requested number
* of phrases
*/
vector<Phrase> CAutoSegmentation::GetPhrases( CSaDoc & doc, wofstream & ofs, int goal) {

	vector<Phrase> result;

	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)doc.GetSegment(PHONETIC);
	pPhonetic->DeleteContents();
	CGlossSegment * pGloss = (CGlossSegment*)doc.GetSegment(GLOSS);
	pGloss->DeleteContents();

	CMainFrame * pMainFrame = (CMainFrame*)AfxGetMainWnd();

    // get parse parameters document member data
    // store new data
	
    doc.RestartAllProcesses();

	// divide and conquer until we find the answer
	int maxBreakWidth = 999;
	int minBreakWidth = 1;
	int nextBreakWidth = (maxBreakWidth+minBreakWidth)/2;

	// processing is done on the gloss segment, but we will copy it
	// to the gloss or gloss_nat when we are done.

	unsigned int error = 100000;
	int bestWidth = -1;

	bool found = false;

	// search based on least error
	while (true) {

		nextBreakWidth = (maxBreakWidth+minBreakWidth)/2;

		TRACE("max=%d min=%d next=%d\n",maxBreakWidth,minBreakWidth,nextBreakWidth);

		pMainFrame->SetPhraseBreakWidth((float)(nextBreakWidth)/1000.0f);

		doc.DeleteSegmentContents(PHONETIC);
		doc.DeleteSegmentContents(PHONEMIC);
		doc.DeleteSegmentContents(ORTHO);
		doc.DeleteSegmentContents(TONE);
	    doc.DeleteSegmentContents(GLOSS);
		doc.DeleteSegmentContents(GLOSS_NAT);
		doc.DeleteSegmentContents(REFERENCE);

		pGloss->RestartProcess();
		pGloss->SetDataInvalid();

		// segment the data
		short int nResult = LOWORD(pGloss->Process(NULL, &doc));
		if (nResult == PROCESS_ERROR) {
			// error parsing
			doc.ErrorMessage(IDS_ERROR_PARSE);
			return result;
		}
		if (nResult == PROCESS_CANCELED) {
			// error canceled parsing
			doc.ErrorMessage(IDS_CANCELED);
			return result;
		}

		int count = pGloss->GetOffsetSize();
		TRACE("found %d segments\n",count);

		if (count==goal) {
			found = true;
			break;
		}

		// sigh - we must continue
		unsigned int thisError = (goal<count)?count-goal:goal-count;
		if (thisError < error) {
			error = thisError;
			bestWidth = nextBreakWidth;
		}
		if (count>goal) {
			// we found too many, we need to raise the break width
			minBreakWidth = nextBreakWidth;
		} else {
			// we found too few, we need to lower the break width
			maxBreakWidth = nextBreakWidth;
		}
		if ((maxBreakWidth-minBreakWidth)<=1) {
			break;
		}
	}

	if (!found) {
		// we failed
		// run it one more time with the best result
		pMainFrame->SetPhraseBreakWidth((float)(bestWidth)/1000.0f);

		doc.DeleteSegmentContents(PHONETIC);
		doc.DeleteSegmentContents(PHONEMIC);
		doc.DeleteSegmentContents(ORTHO);
		doc.DeleteSegmentContents(TONE);
		doc.DeleteSegmentContents(GLOSS);
		doc.DeleteSegmentContents(GLOSS_NAT);
		doc.DeleteSegmentContents(REFERENCE);

		// for the case of a cancelled process
		pGloss->RestartProcess();
		// SDM 1.5Test10.7
		pGloss->SetDataInvalid(); 
		// segment the data
		short int nResult = LOWORD(pGloss->Process(NULL, &doc));
		if (nResult == PROCESS_ERROR) {
			// error parsing
			doc.ErrorMessage(IDS_ERROR_PARSE);
			return result;
		}
		if (nResult == PROCESS_CANCELED) {
			// error canceled parsing
			doc.ErrorMessage(IDS_CANCELED);
			return result;
		}
		DWORD count = pGloss->GetOffsetSize();
		TRACE("found %d segments\n",count);
	}

	for (int i=0;i<pGloss->GetOffsetSize();i++) {
		Phrase entry;
		entry.offset = pGloss->GetOffset(i);
		entry.duration = pGloss->GetDuration(i);
		result.push_back(entry);
	}

	ofs << L"there are " << result.size() << L" phrases\n";
	return result;
}

vector<Phrase> CAutoSegmentation::GetPhrases( CSaDoc & doc, wofstream & ofs) {

    // get parse parameters document member data
    // store new data
	
    doc.RestartAllProcesses();

	// divide and conquer until we find the answer
	CMainFrame * pMainFrame = (CMainFrame*)AfxGetMainWnd();
	pMainFrame->SetPhraseBreakWidth(BREAK_WIDTH/1000.0f);

	doc.DeleteSegmentContents(PHONETIC);
	doc.DeleteSegmentContents(PHONEMIC);
	doc.DeleteSegmentContents(ORTHO);
	doc.DeleteSegmentContents(TONE);
	doc.DeleteSegmentContents(GLOSS);
	doc.DeleteSegmentContents(GLOSS_NAT);
	doc.DeleteSegmentContents(REFERENCE);

	CGlossSegment * pGloss = (CGlossSegment*)doc.GetSegment(GLOSS);
	pGloss->RestartProcess();
	pGloss->SetDataInvalid();

	vector<Phrase> result;

	// segment the data
	short int nResult = LOWORD(pGloss->Process(NULL, &doc));
	if (nResult == PROCESS_ERROR) {
		// error parsing
		doc.ErrorMessage(IDS_ERROR_PARSE);
		return result;
	}
	if (nResult == PROCESS_CANCELED) {
		// error canceled parsing
		doc.ErrorMessage(IDS_CANCELED);
		return result;
	}

	DWORD count = pGloss->GetOffsetSize();
	TRACE("found %d segments\n",count);

	for (int i=0;i<pGloss->GetOffsetSize();i++) {
		Phrase entry;
		entry.offset = pGloss->GetOffset(i);
		entry.duration = pGloss->GetDuration(i);
		result.push_back(entry);
	}

	ofs << L"there are " << result.size() << L" phrases\n";
	return result;
}

void CAutoSegmentation::DumpSegments( wofstream & ofs, vector<Verse> & verses, vector<Phrase> & phrases) {

	ofs << verses.size() << L" verses and " << phrases.size() << L" phrases\n";;
	ofs << L"\tphones\t\t\t\twords\n";
	ofs << L"ref\tactual\texpect\terror\tstat\tactual\texpect\terror\tstat\tmod\tstart\tstop\t\n";

	size_t i = 0;
	size_t j = 0;
	bool done = false;
	while (!done) {
		wstring ref = L"";
		DWORD ppcount = 0;
		DWORD gpcount = 1;
		float perror = 0.0f; 
		wstring pstat = L"";
		DWORD pwcount = 0;
		DWORD gwcount = 1;
		float werror = 0.0f; 
		wstring wstat = L"";
		DWORD pstart = 0;
		DWORD pstop = 0;
		bool mod = false;
		if (i<phrases.size()) {
			ppcount = phrases[i].phones.size();
			pwcount = phrases[i].words.size();
			pstart = phrases[i].offset;
			pstop = phrases[i].offset+phrases[i].duration;
			if (j<verses.size()) {
				ref = verses[j].ref;
				gpcount = verses[j].GetPhoneCount();
				perror = ((float)ppcount/(float)gpcount);
				pstat = ((perror<MIN_PHONE_ERROR)?L"SHORT":((perror>MAX_PHONE_ERROR)?L"LONG":L""));
				gwcount = verses[j].GetWordCount();
				werror = ((float)pwcount/(float)gwcount);
				wstat = ((werror<MIN_WORD_ERROR)?L"SHORT":((werror>MAX_WORD_ERROR)?L"LONG":L""));
				mod = ((NeedsMerge(verses[j],phrases[i]))||(NeedsSplit(verses[j],phrases[i])));
			} else {
				pstat = L"NO VERSE";
			}
		} else {
			// no phrase
			if (j<verses.size()) {
				ref = verses[j].ref;
				gpcount = verses[j].GetPhoneCount();
				gwcount = verses[j].GetWordCount();
			}
			pstat = L"NO PHRASE";
			wstat = L"NO PHRASE";
		}
	
		ofs << ref;
		ofs << L"\t";
		ofs << ppcount;
		ofs << L"\t";
		ofs << gpcount;
		ofs << L"\t";
		ofs << (DWORD)(perror*100);
		ofs << L"\t";
		ofs << pstat;
		ofs << L"\t";
		ofs << pwcount;
		ofs << L"\t";
		ofs << gwcount;
		ofs << L"\t";
		ofs << (DWORD)(werror*100);
		ofs << L"\t";
		ofs << wstat;
		ofs << L"\t";
		ofs << ((mod)?"*":" ");
		ofs << L"\t";
		ofs << pstart;
		ofs << L"\t";
		ofs << pstop;
		ofs << L"\n";

		i++;
		j++;
		if ((i>=phrases.size())&&(j>=verses.size())) break;
	}
}

size_t CAutoSegmentation::GetNonVoicedCharacterCount( wstring text) {
	size_t result = 0;
	wchar_t last = 0;
	wchar_t c = 0;
	for (size_t i=0;i<text.length();i++) {
		last = c;
		c = text[i];
		//gaps show up as a phonetic
		//if (c==0x20) continue;
		//if (c==0x09) continue;
		if (c=='.') continue;
		if (c==',') continue;
		// skip duplicate characters
		if (last==c) continue;
		result++;
	}
	return result;
}

/**
* break on the line on white space
*/
size_t CAutoSegmentation::GetWordCount( wstring text) {
	wstring word;
	size_t result = 0;
	for (size_t i=0;i<text.length();i++) {
		wchar_t c = text[i];
		if ((c==0x20)||(c==0x09)||(c==0x0d)||(c==0x0a)||(c=='.')||(c==',')) {
			if (word.length()>0) {
				result++;
				word = L"";
			}
		} else {
			word.push_back(c);
		}
	}
	// handle remainder
	if (word.length()>0) {
		result++;
		word = L"";
	}
	return result;
}

void Phrase::FilterPhones() {
	vector<Phonetic> newphones;
	DWORD stop = offset+duration;
	for (size_t i=0;i<phones.size();i++) {
		if ((phones[i].stop>=offset)&&(phones[i].stop<=stop)) {
			newphones.push_back(phones[i]);
		}
	}
	phones = newphones;
}

void Phrase::FilterWords() {
	vector<Word> newwords;
	DWORD stop = offset+duration;
	for (size_t i=0;i<words.size();i++) {
		if ((words[i].stop>=offset)&&(words[i].stop<=stop)) {
			newwords.push_back(words[i]);
		}
	}
	words = newwords;
}

string CAutoSegmentation::GetKey( size_t v, DWORD o, DWORD d) {
	char buffer[512];
	sprintf_s(buffer,sizeof(buffer),"%d_%d_%d",v,o,d);
	return string(buffer);
}

wstring CAutoSegmentation::GetTabs(size_t indent) {
	wstring result;
	for (size_t i=0;i<indent;i++) {
		result.append(L" ");
	}
	return result;
}

/**
* This is reentrant function the considers all the possible paths for 
* errors that are found between the verse and phrase
* We will walk through each pair.  When we find a scenario that is a possible error,
* we will branch and try both options, and return the results of each
*/
vector<Phrase> CAutoSegmentation::ConsiderPaths( wofstream & ofs, vector<Verse> & verses, vector<Phrase> phrases, size_t v, size_t p, size_t indent) {
	
	if ((v>=verses.size())||(p>=phrases.size())) {
		return phrases;
	}

	string key = GetKey(v,phrases[p].offset,phrases[p].duration);
	map<string,vector<Phrase>>::iterator it = cache.find(key);
	if (it!=cache.end()) {
		return it->second;
	}

	vector<Phrase> result = ConsiderPathsImpl( ofs, verses, phrases, v, p, indent);
	cache[key] = result;
	return result;
}

vector<Phrase> CAutoSegmentation::ConsiderPathsImpl( wofstream & ofs, vector<Verse> & verses, vector<Phrase> phrases, size_t v, size_t p, size_t indent) {

	wstring tabs = GetTabs(indent);

	float phoneScore = ScorePhone(verses[v],phrases[p]);
	float wordScore = ScoreWord(verses[v],phrases[p]);
	ofs << tabs << L"["<<verses[v].ref<<","<<phrases[p].offset<<","<<phrases[p].duration<<","<<phoneScore<<","<<wordScore<<"]\n";
	TRACE(L"%s[%s,%d,%d,%f,%f]\n",tabs.c_str(),verses[v].ref.c_str(),phrases[p].offset,phrases[p].duration,phoneScore,wordScore);
	if (NeedsMerge(verses[v],phrases[p])) {
		// we need to merge
		// return the results for what happens if we merge, or if we dont'
		// what happens if we don't change?
		ofs << tabs << L"testing NM "<<verses[v].ref<<L"\n";
		vector<Phrase> resultA = ConsiderPaths( ofs, verses, phrases, v+1, p+1, indent+1);
		float scoreA = ScorePath(ofs,tabs,verses,resultA,v,p,L"NM");

		// what happens if we do merge?
		if ((p+1)>=phrases.size()) {
			return resultA;
		}

		// combine this and the next phrase segment
		phrases[p].Merge(phrases[p+1]);
		phrases.erase(phrases.begin()+p+1);
		// since we've do a merge, we need to reevaluate this node for a split.
		ofs << tabs << "testing M "<<verses[v].ref<<"\n";
		vector<Phrase> resultB = ConsiderPaths( ofs, verses, phrases, v, p, indent);
		float scoreB = ScorePath(ofs,tabs,verses,resultB,v,p,L"M");
		return (scoreA<scoreB)?resultA:resultB;
	} else if (NeedsSplit(verses[v],phrases[p])) {
		// we need to split
		// return the results for what happens if we split, or if we dont'
		// what happens if we don't change?
		ofs << tabs << L"testing NS "<<verses[v].ref<<L"\n";
		vector<Phrase> resultA = ConsiderPaths( ofs, verses, phrases, v+1, p+1, indent+1);
		float scoreA = ScorePath(ofs,tabs,verses,resultA,v,p,L"NS");

		// what happens if we do?
		// we will calculate where to split based on the score.
		float duration = (float)phrases[p].duration;
		float score = ScorePhone(verses[v],phrases[p]);
		duration /= score;
		// split this segment. insert after
		DWORD oldoffset = phrases[p].offset;
		DWORD oldstop = phrases[p].offset+phrases[p].duration;
		DWORD offset1 = oldoffset;
		DWORD duration1 = (DWORD)duration;
		DWORD offset2 = offset1+duration1+1;
		DWORD duration2 = oldstop-offset2-1;
		phrases[p].offset = offset1;
		phrases[p].duration = duration1;

		Phrase entry;
		entry.offset = offset2;
		entry.duration = duration2;
		entry.phones = phrases[p].phones;
		phrases.insert(phrases.begin()+p+1,entry);

		// filter after they are separate
		phrases[p].FilterPhones();
		phrases[p+1].FilterPhones();
		phrases[p].FilterWords();
		phrases[p+1].FilterWords();

		// carry on
		ofs << tabs << L"testing S "<<verses[v].ref<<L"\n";
		// actually restart at current 'v'
		vector<Phrase> resultB = ConsiderPaths( ofs, verses, phrases, v, p, indent);
		float scoreB = ScorePath(ofs,tabs,verses,resultB,v,p,L"S");
		return (scoreA<scoreB)?resultA:resultB;
	} else {
		// no problem continue with verse and phrase
		return ConsiderPaths( ofs, verses, phrases, v+1, p+1, indent+1);
	}
}

bool CAutoSegmentation::DoPhoneticMatching( LPCTSTR path, CSaDoc & doc, CTranscriptionData & td, int skipCount, bool usingGL) {

	TRACE(L"phonetic matching\n");

    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	wstring filename;
	wstring parent = FileUtils::RemoveExtension(path);
	filename.append(parent.c_str());
	filename.append(L".log");
	wofstream ofs(filename.c_str());

	ofs<< L"phonetic matching\n";

	// load up the data for the refs and glosses
	vector<Verse> verses = GetVerses(td,usingGL);
	vector<Phonetic> phones = GetPhones(doc,ofs);
	vector<Word> words = GetWords(doc,ofs);
	// use the divide and conquer method to get the phrases
	int goal = verses.size()+skipCount;
	vector<Phrase> phrases = GetPhrases(doc,ofs,goal);

	// group phones into phrases
	for (size_t p=0;p<phrases.size();p++) {
		DWORD start = phrases[p].offset;
		DWORD stop = start + phrases[p].duration;
		while (true) {
			if (phones.empty()) break;
			DWORD pstop = phones[0].stop;
			if (pstop>stop) break;
			phrases[p].phones.push_back(phones[0]);
			phones.erase(phones.begin());
		}
	}
	// group words into phrases
	for (size_t p=0;p<phrases.size();p++) {
		DWORD start = phrases[p].offset;
		DWORD stop = start + phrases[p].duration;
		while (true) {
			if (words.empty()) break;
			DWORD wstop = words[0].stop;
			if (wstop>stop) break;
			phrases[p].words.push_back(words[0]);
			words.erase(words.begin());
		}
	}

	int last = phrases.size()-1;
	ofs << L"last phrase stop = "<<(phrases[last].offset+phrases[last].duration)<<L"\n";
	ofs << L"there are "<<phones.size()<<" phones to be dropped\n";
	for (size_t j=0;j<phones.size();j++) {
		ofs << L"dropping phone offset:"<<phones[j].start<<L" stop:"<<phones[j].stop<<L"\n";
	}
	ofs << L"there are "<<words.size()<<" words to be dropped\n";
	for (size_t j=0;j<words.size();j++) {
		ofs << L"dropping words offset:"<<words[j].start<<L" stop:"<<words[j].stop<<L"\n";
	}

	ofs << L"there are "<<phrases.size()<<" phrases\n";
	ofs << L"there are "<<verses.size()<<" verses\n";

	ofs << L"skipping "<<skipCount<<L" phrases per user\n";
	for (int i=0;i<skipCount;i++) {
		phrases.erase(phrases.begin());
	}

	// so at this point, we have phonetic characters for the phonetic segments,
	// and the gloss text segmentation
	// we need to walk through and find the segments that are too small.
	// we will always work with gloss, even if it's really GN

	// the goal is to balance the lengths of the strings in the verses with the 
	// number of phonetics
	ofs << L"minimum phone:"<<MIN_PHONE_ERROR<<"\n";
	ofs << L"maximum phone:"<<MAX_PHONE_ERROR<<"\n";
	ofs << L"minimum word:"<<MIN_WORD_ERROR<<"\n";
	ofs << L"maximum word:"<<MAX_WORD_ERROR<<"\n";

	ofs << L"processing false breaks\n";
	ofs << L"initial state\n";
	DumpSegments(ofs,verses,phrases);

	wstring tabs;

	float prescore = ScorePath(ofs, tabs, verses, phrases, 0, 0, L"");
	ofs << L"prescore:"<<prescore<<L"\n";

	vector<Phrase> result = ConsiderPaths( ofs, verses, phrases, 0, 0, 0);

	ofs << L"final state\n";
	float postscore = ScorePath(ofs, tabs, verses, result, 0, 0, L"");
	ofs << L"prescore:"<<prescore<<L"\n";
	ofs << L"postscore:"<<postscore<<L"\n";

	if (postscore>=prescore) {
		ofs << L"no improvement =(\n";
		ofs << L"keeping original path\n";
		result = phrases;
	} else {
		ofs << L"it actually worked!\n";
		DumpSegments(ofs,verses,result);
	}

	// when we are done, populate the phonetic and gloss segments
	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)doc.GetSegment(PHONETIC);
	CGlossSegment * pGloss = (CGlossSegment*)doc.GetSegment(GLOSS);
	CGlossNatSegment * pGlossNat = (CGlossNatSegment*)doc.GetSegment(GLOSS_NAT);
	CReferenceSegment * pRef = (CReferenceSegment*)doc.GetSegment(REFERENCE);
	pPhonetic->DeleteContents();
	pGloss->DeleteContents();
	pGlossNat->DeleteContents();
	pRef->DeleteContents();
	for (size_t i=0;i<result.size();i++) {
		DWORD offset = result[i].offset;
		DWORD duration = result[i].duration;
		pPhonetic->Append(L"",false,offset,duration);
		pGloss->Append(L"",false,offset,duration);
		pGlossNat->Append(L"",false,offset,duration);
		pRef->Append(L"",false,offset,duration);
	}

	//JoinSegmentBoundaries();
	TRACE(L"done!\n");
	ofs.flush();
	ofs.close();
	return true;
}

bool CAutoSegmentation::DoDivideAndConquer( CSaDoc & doc, CSaView & view, DWORD goal, int skipCount) {

	TRACE(L"divide and conquer\n");

    // get parse parameters document member data
    // store new data
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	
    doc.RestartAllProcesses();

	view.RedrawGraphs(TRUE);

	// divide and conquer until we find the answer
	int maxBreakWidth = 999;
	int minBreakWidth = 1;
	int nextBreakWidth = (maxBreakWidth+minBreakWidth)/2;

	// processing is done on the gloss segment, but we will copy it
	// to the gloss or gloss_nat when we are done.

    CSegment * pGloss = doc.GetSegment(GLOSS);

	unsigned int error = 100000;
	int bestWidth = -1;

	bool found = false;

	// search based on least error
	while (true) {

		nextBreakWidth = (maxBreakWidth+minBreakWidth)/2;

		TRACE(L"max=%d min=%d next=%d\n",maxBreakWidth,minBreakWidth,nextBreakWidth);

		pMainFrame->SetPhraseBreakWidth((float)(nextBreakWidth)/1000.0f);

		doc.DeleteSegmentContents(PHONETIC);
		doc.DeleteSegmentContents(PHONEMIC);
		doc.DeleteSegmentContents(ORTHO);
		doc.DeleteSegmentContents(TONE);
	    doc.DeleteSegmentContents(GLOSS);
		doc.DeleteSegmentContents(GLOSS_NAT);
		doc.DeleteSegmentContents(REFERENCE);

		pGloss->RestartProcess(); // for the case of a cancelled process
		pGloss->SetDataInvalid(); // SDM 1.5Test10.7

		// segment the data
		short int nResult = LOWORD(pGloss->Process(NULL, &doc));
		if (nResult == PROCESS_ERROR) {
			// error parsing
			doc.ErrorMessage(IDS_ERROR_PARSE);
			return false;
		}
		if (nResult == PROCESS_CANCELED) {
			// error canceled parsing
			doc.ErrorMessage(IDS_CANCELED);
			return false;
		}

		DWORD count = pGloss->GetOffsetSize();
		TRACE("found %d segments\n",count);

		if (count==goal) {
			found = true;
			break;
		}

		// sigh - we must continue
		unsigned int thisError = (goal<count)?count-goal:goal-count;
		if (thisError < error) {
			error = thisError;
			bestWidth = nextBreakWidth;
		}
		if (count>goal) {
			// we found too many, we need to raise the break width
			minBreakWidth = nextBreakWidth;
		} else {
			// we found too few, we need to lower the break width
			maxBreakWidth = nextBreakWidth;
		}
		if ((maxBreakWidth-minBreakWidth)<=1) {
			break;
		}
	}

	if (!found) {
		// we failed
		// run it one more time with the best result
		pMainFrame->SetPhraseBreakWidth((float)(bestWidth)/1000.0f);

		doc.DeleteSegmentContents(PHONETIC);
		doc.DeleteSegmentContents(PHONEMIC);
		doc.DeleteSegmentContents(ORTHO);
		doc.DeleteSegmentContents(TONE);
		doc.DeleteSegmentContents(GLOSS);
		doc.DeleteSegmentContents(GLOSS_NAT);
		doc.DeleteSegmentContents(REFERENCE);

		// for the case of a cancelled process
		pGloss->RestartProcess();
		// SDM 1.5Test10.7
		pGloss->SetDataInvalid(); 
		// segment the data
		short int nResult = LOWORD(pGloss->Process(NULL, &doc));
		if (nResult == PROCESS_ERROR) {
			// error parsing
			doc.ErrorMessage(IDS_ERROR_PARSE);
			return false;
		}
		if (nResult == PROCESS_CANCELED) {
			// error canceled parsing
			doc.ErrorMessage(IDS_CANCELED);
			return false;
		}
		DWORD count = pGloss->GetOffsetSize();
		TRACE(L"found %d segments\n",count);

		CString param1;
		param1.Format(L"%d",goal);
		CString param2;
		param2.Format(L"%d",pGloss->GetOffsetSize());
		CSaString msg;
		AfxFormatString2(msg,IDS_ERROR_SAB_AUTOPARSE_FAIL,param1,param2);
		doc.ErrorMessage(msg);
		return false;
	}

	CReferenceSegment * pRef = (CReferenceSegment*)doc.GetSegment(REFERENCE);
	CGlossNatSegment * pGlossNat = (CGlossNatSegment*)doc.GetSegment(GLOSS_NAT);
	CToneSegment * pTone = (CToneSegment*)doc.GetSegment(TONE);
	COrthographicSegment * pOrtho = (COrthographicSegment*)doc.GetSegment(ORTHO);
	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)doc.GetSegment(PHONETIC);
	CPhonemicSegment * pPhonemic = (CPhonemicSegment*)doc.GetSegment(PHONEMIC);

	// remove the segments the user told us to ignore
	while (skipCount>0) {
		pRef->RemoveAt(0);
		pGlossNat->RemoveAt(0);
		pGloss->RemoveAt(0);
		pTone->RemoveAt(0);
		pOrtho->RemoveAt(0);
		pPhonemic->RemoveAt(0);
		pPhonetic->RemoveAt(0);
		skipCount--;
	}

	// walk through the segments are remove the spacing between segments
	JoinSegmentBoundaries( doc);
	return true;
}

/**
* Walk through and determine the midpoint between any segment pair.
* The midpoint because the new stop/start location
* This is only useful for AudioSync
*/
void CAutoSegmentation::JoinSegmentBoundaries( CSaDoc & doc) {

	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)doc.GetSegment(PHONETIC);
	for (int i=0;i<pPhonetic->GetOffsetSize();i++) {
		if (i==0) continue;
		DWORD lastStart = pPhonetic->GetOffset(i-1);
		DWORD lastStop = pPhonetic->GetStop(i-1);
		DWORD start = pPhonetic->GetOffset(i);
		DWORD stop = pPhonetic->GetStop(i);
		if (lastStop==start) continue;
		DWORD midPoint = (lastStop+start)/2;
		// these calls use adjust dependent segments as well
		pPhonetic->Adjust( &doc, i-1, lastStart, midPoint-lastStart, false);
		pPhonetic->Adjust( &doc, i, midPoint, stop-midPoint, false);
	}
}

void Phrase::Merge(Phrase right) {
	duration = right.offset+right.duration-offset;
	for (size_t j=0;j<right.phones.size();j++) {
		phones.push_back(right.phones[j]);
	}
	for (size_t j=0;j<right.words.size();j++) {
		words.push_back(right.words[j]);
	}
}

/**
* Calculate the score for a single verse/phrase pair
* The closer the output value is to 1.0, the better
*/
float CAutoSegmentation::ScorePhone( Verse & verse, Phrase & phrase) {
	float actual = (float)phrase.phones.size();
	float expected = (float)verse.GetPhoneCount();
	return (actual/expected);
}

/**
* Calculate the score for a single verse/phrase pair
* The closer the output value is to 1.0, the better
*/
float CAutoSegmentation::ScoreWord( Verse & verse, Phrase & phrase) {
	float actual = (float)phrase.words.size();
	float expected = (float)verse.GetWordCount();
	return (actual/expected);
}

/**
* The percentage of errors in the document.
* Lower is better.
*/
float CAutoSegmentation::ScorePath( wofstream & ofs, wstring tabs, vector<Verse> & verses, vector<Phrase> & phrases, size_t v, size_t p, LPCTSTR tag) {

	// if we have extra phrases - count that as well..
	if (phrases.size()>verses.size()) return 1.0;
	// if we are short phrases - that is also bad!
	if (phrases.size()<verses.size()) return 1.0;

	float error = 0;
	size_t p1 = p;
	for (size_t v1=v;v1<verses.size();v1++) {
		if (p1==phrases.size()) break;
		if ((NeedsSplit(verses[v1],phrases[p1]))||(NeedsMerge(verses[v1],phrases[p1]))) {
			error++;
		}
		p1++;
	}

	float score = error/verses.size();
	score = (score>1.0f)?1.0f:score;
	ofs << tabs << L"result "<<verses[v].ref<<L" op:"<<tag<<L" score:"<<score<<"\n";
	return score;
}

bool CAutoSegmentation::NeedsSplit( Verse & v, Phrase & p) {
	float pa = (float)p.phones.size();
	float pe = (float)v.GetPhoneCount();
	float pae = pa/pe;
	float wa = (float)p.words.size();
	float we = (float)v.GetWordCount();
	float wae = wa/we;
	return ((pae>MAX_PHONE_ERROR)&&(wae>MAX_WORD_ERROR));
}

bool CAutoSegmentation::NeedsMerge( Verse & v, Phrase & p) {
	float pe = (float)v.GetPhoneCount();
	float pa = (float)p.phones.size();
	float pae = pa/pe;
	float we = (float)v.GetWordCount();
	float wa = (float)p.words.size();
	float wae = wa/we;
	return ((pae<MIN_PHONE_ERROR)&&(wae<MIN_WORD_ERROR));
}

Verse::Verse( LPCTSTR aref, LPCTSTR atext, size_t aWordCount, size_t aPhoneCount) {
	ref = aref;
	text = atext;
	wordCount = aWordCount;
	phoneCount = aPhoneCount;
}

Verse::Verse( const Verse & right) {
	ref = right.ref.c_str();
	text = right.text.c_str();
	wordCount = right.wordCount;
	phoneCount = right.phoneCount;
}

wstring Verse::GetText() {
	return text;
}

size_t Verse::GetWordCount() {
	return wordCount;
}
size_t Verse::GetPhoneCount() {
	return phoneCount;
}

