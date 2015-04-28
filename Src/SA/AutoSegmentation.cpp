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

#define BREAK_WIDTH 300.0f
#define MIN_PHONE_ERROR 0.70f
#define MAX_PHONE_ERROR 1.30f

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
			Verse entry;
			entry.text = (*glit).Trim();
			glit++;
			// insert reference
			if (rit!=rend) {
				entry.ref = *rit;
				rit++;
			}
			result.push_back(entry);
		}
	} else {
		// using gloss national
		while (gnit!=gnend) {
			// insert gloss
			Verse entry;
			entry.text = (*gnit).Trim();
			gnit++;
			// insert reference
			if (rit!=rend) {
				entry.ref = *rit;
				rit++;
			}
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
		DWORD data = index / sizeFactor;
		// get data for this pixel
		// SDM 1.5Test11.0
		int nHere = pPitch->GetProcessedData( data, &bRes); 
		if (nHere > 0) {
			nData += nHere;
			dwSamples++;
		}
		if ((dwSamples>0) && (bRes)) {
			double fData = double(nData) / PRECISION_MULTIPLIER / dwSamples;
			//TRACE("pitch value = %f\n",fData);
		} else {
			//TRACE("pitch value = none\n");
		}
		result.push_back(phonetic);
	}

	ofs << L"there are " << result.size() << L" phones" << "\n";
	return result;
}

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

		nextBreakWidth = (maxBreakWidth+minBreakWidth)/2.0f;

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

void CAutoSegmentation::DumpSegments( wofstream & ofs, vector<Verse> & verses, vector<Phrase> & phrases, int to) {

	ofs << verses.size() << L" verses and " << phrases.size() << L" phrases\n";;
	ofs << L"\tphones\n";
	ofs << L"ref\tactual\texpect\terror\tstat\tstart\tstop\t\n";

	int i = 0;
	int j = 0;
	bool done = false;
	while (!done) {

		wstring ref = L"";
		DWORD ppcount = 0;
		DWORD gpcount = 1;
		float perror = 0.0f; 
		wstring pstat = L"";
		DWORD pstart = 0;
		DWORD pstop = 0;

		if (i<phrases.size()) {
			ppcount = phrases[i].phones.size();
			pstart = phrases[i].offset;
			pstop = phrases[i].offset+phrases[i].duration;
			if (j<verses.size()) {
				ref = verses[j].ref;
				gpcount = GetNonVoicedCharacterCount(verses[j].text);
				perror = ((float)ppcount/(float)gpcount);
				pstat = ((perror<=MIN_PHONE_ERROR)?L"SHORT":((perror>=MAX_PHONE_ERROR)?L"LONG":L""));
				if (to==i) done = true;
			} else {
				pstat = L"NO VERSE";
			}
		} else {
			// no phrase
			if (j<verses.size()) {
				ref = verses[j].ref;
				gpcount = GetNonVoicedCharacterCount(verses[j].text);
			}
			pstat = L"NO PHRASE";
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
		ofs << pstart;
		ofs << L"\t";
		ofs << pstop;
		ofs << L"\n";

		i++;
		j++;
		if ((i>=phrases.size())&&(j>=verses.size())) break;
	}
}

int CAutoSegmentation::GetNonVoicedCharacterCount( wstring text) {
	int result = 0;
	for (size_t i=0;i<text.length();i++) {
		wchar_t c = text[i];
		//gaps show up as a phonetic
		//if (c==0x20) continue;
		//if (c==0x09) continue;
		if (c=='.') continue;
		if (c==',') continue;
		result++;
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

string CAutoSegmentation::GetKey( size_t v, DWORD o, DWORD d) {
	char buffer[512];
	sprintf(buffer,"%d_%d_%d",v,o,d);
	string result = buffer;
	return result;
}

/**
* This is reentrant function the considers all the possible paths for 
* errors that are found between the verse and phrase
* We will walk through each pair.  When we find a scenario that is a possible error,
* we will branch and try both options, and return the results of each
*/
vector<vector<Phrase>> CAutoSegmentation::ConsiderPaths( wofstream & ofs, vector<Verse> verses, size_t v, vector<Phrase> phrases, size_t p, size_t depth) {

	//string key = GetKey(v,phrases[p].offset,phrases[p].duration);
	//map<string,phrase_path>::iterator it = cache.find(key);
	//if (it!=cache.end()) {
		//return it->second;
	//}

	TRACE(" %d/%d-%d",v,verses.size(),p);

	if (v==depth) {
		TRACE(" [%d %d]\n",phrases[p].offset,phrases[p].duration);
		// we are done.
		phrase_path results;
		results.push_back(phrases);
		//cache[key] = results;
		//TRACE("cache size %d\n",cache.size());
		return results;
	}

	if (p==phrases.size()) {
		// we ran out of phrases before we resolved the verses.
		// continue down the tree until the verses are exhausted
		phrase_path results = ConsiderPaths( ofs, verses, v+1, phrases, p, depth);
		//cache[key] = results;
		return results;
	}

	float score = CalculateScore( verses[v], phrases[p]);
	if (score < MIN_PHONE_ERROR) {
		// we need to merge
		// return the results for what happens if we merge, or if we dont'
		// what happens if we don't change?
		TRACE(" NM");
		vector<vector<Phrase>> results = ConsiderPaths( ofs, verses, v+1, phrases, p+1, depth);
		// what happens if we do?
		
		if ((p+1)<phrases.size()) {
			// combine this and the next phrase segment
			phrases[p].Merge(phrases[p+1]);
			phrases.erase(phrases.begin()+p+1);
			TRACE(" M",v,p);
			vector<vector<Phrase>> temp= ConsiderPaths( ofs, verses, v+1, phrases, p+1, depth);
			for (size_t i=0;i<temp.size();i++) {
				results.push_back(temp[i]);
			}
		}
		//string key = GetKey(v,phrases[p].offset,phrases[p].duration);
		//cache[key] = results;
		return results;
	} else if (score > MAX_PHONE_ERROR) {
		// we need to split
		// return the results for what happens if we split, or if we dont'
		// what happens if we don't change?
		TRACE(" NS");
		vector<vector<Phrase>> results = ConsiderPaths( ofs, verses, v+1, phrases, p+1, depth);
		// what happens if we do?
		// we will calculate where to split based on the score.
		float duration = phrases[p].duration;
		duration /= score;
		// split this segment. insert after
		DWORD oldoffset = phrases[p].offset;
		DWORD oldstop = phrases[p].offset+phrases[p].duration;
		DWORD offset1 = oldoffset;
		DWORD duration1 = duration;
		DWORD offset2 = offset1+duration1;
		DWORD duration2 = oldstop-offset2;
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

		// carry on
		TRACE(" S");
		vector<vector<Phrase>> temp = ConsiderPaths( ofs, verses, v+1, phrases, p+1, depth);
		for (size_t i=0;i<temp.size();i++) {
			results.push_back(temp[i]);
		}

		//string key = GetKey(v,phrases[p].offset,phrases[p].duration);
		//cache[key] = results;
		return results;
	}
	
	// no problem continue
	phrase_path results =  ConsiderPaths( ofs, verses, v+1, phrases, p+1, depth);
	//cache[key] = results;
	return results;
}

/**
* calculate the score for a single verse/phrase pair
*/
float CAutoSegmentation::CalculateScore( Verse & verse, Phrase & phrase) {
	float actual = phrase.phones.size();
	float expected = GetNonVoicedCharacterCount(verse.text);
	return (actual/expected);
}

/**
* calculate the average score for the entire verse/phrase set
*/
float CAutoSegmentation::CalculateScore( vector<Verse> & verses, vector<Phrase> & phrases, size_t depth) {
	float sum = 0;
	float count = 0;
	size_t p = 0;
	for (size_t v=0;v<depth;v++) {
		if (p==phrases.size()) break;
		sum += CalculateScore(verses[v],phrases[p]);
		count++;
		p++;
	}
	return sum/count;
}

/**
* Find the lowest score
*/
float CAutoSegmentation::FindMinError( vector<Verse> & verses, vector<Phrase> & phrases, size_t depth) {
	float min = 1000.0;
	size_t p = 0;
	for (size_t v=0;v<depth;v++) {
		if (p==phrases.size()) break;
		float score = CalculateScore(verses[v],phrases[p]);
		if (score<min) {
			min = score;
		}
		p++;
	}
	return min;
}

/**
* Find the highest score
*/
float CAutoSegmentation::FindMaxError( vector<Verse> & verses, vector<Phrase> & phrases, size_t depth) {
	float max = 0.0;
	size_t p = 0;
	for (size_t v=0;v<depth;v++) {
		if (p==phrases.size()) break;
		float score = CalculateScore(verses[v],phrases[p]);
		if (score>max) {
			max = score;
		}
		p++;
	}
	return max;
}

bool CAutoSegmentation::PhoneticMatching( CSaDoc & doc, CTranscriptionData & td, int skipCount, bool usingGL) {

    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	string filename;
	filename.append("c:\\");
	filename.append("sab_import_trace.txt");
	wofstream ofs(filename.c_str());

	ofs<< L"phonetic matching\n";

	// load up the data for the refs and glosses
	vector<Verse> verses = GetVerses(td,usingGL);
	vector<Phonetic> phones = GetPhones(doc,ofs);
	vector<Phrase> phrases = GetPhrases(doc,ofs);

	// group phones into phrases
	// group words into phrases
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

	int last = phrases.size()-1;
	ofs << L"last phrase stop = "<<(phrases[last].offset+phrases[last].duration)<<L"\n";
	ofs << L"there are "<<phones.size()<<" phones to be dropped\n";
	for (size_t j=0;j<phones.size();j++) {
		ofs << L"dropping phone offset:"<<phones[j].start<<L" stop:"<<phones[j].stop<<L"\n";
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

	ofs << L"processing false breaks\n";
	ofs << L"initial state\n";
	DumpSegments(ofs,verses,phrases,-1);

	// the goal is to balance the lengths of the strings in the verses with the 
	// number of phonetics

	size_t depth = 10;
	vector<vector<Phrase>> results = ConsiderPaths( ofs, verses, 0, phrases, 0, depth);
	TRACE("\n");
	ofs << L"\n";
	ofs << L"There are "<<results.size()<< " paths\n";

	// find the phrase set with the list error
	size_t index = results.size();
	float bestScore = 1.0f;
	float bestDist = 10000.0f;
	for (size_t i=0;i<results.size();i++) {
		float score = CalculateScore(verses,results[i], depth);
		float min = FindMinError(verses,results[i], depth);
		float max = FindMaxError(verses,results[i], depth);
		float dista = ::abs(score - 1.0);
		float distb = ::abs(min - 1.0);
		float distc = ::abs(max - 1.0);
		ofs << "score: "<<score<<" min: "<<min<<" max: "<<max<<"\n";
		float adist = (dista+distb+distc)/3.0;
		if (adist<bestDist) {
			index = i;
			bestDist = adist;
			bestScore = score;
		}
	}

	ofs << "best score="<<bestScore<<"\n";
	vector<Phrase> best = results[index];

	ofs << L"final state\n";
	DumpSegments(ofs,verses,best,best.size()-1);

	// when we are done, populate the phonetic and gloss segments
	CPhoneticSegment * pPhonetic = (CPhoneticSegment*)doc.GetSegment(PHONETIC);
	CPhonemicSegment * pPhonemic = (CPhonemicSegment*)doc.GetSegment(PHONEMIC);
	CGlossSegment * pGloss = (CGlossSegment*)doc.GetSegment(GLOSS);
	CGlossNatSegment * pGlossNat = (CGlossNatSegment*)doc.GetSegment(GLOSS_NAT);
	CReferenceSegment * pRef = (CReferenceSegment*)doc.GetSegment(REFERENCE);
	CToneSegment * pTone = (CToneSegment*)doc.GetSegment(TONE);
	pPhonetic->DeleteContents();
	pPhonemic->DeleteContents();
	pGloss->DeleteContents();
	pGlossNat->DeleteContents();
	pRef->DeleteContents();
	pTone->DeleteContents();
	for (size_t i=0;i<best.size();i++) {
		DWORD offset = best[i].offset;
		DWORD duration = best[i].duration;
		pPhonetic->Append(L"",false,offset,duration);
		pPhonemic->Append(L"",false,offset,duration);
		pGloss->Append(L"",false,offset,duration);
		pGlossNat->Append(L"",false,offset,duration);
		pRef->Append(L"",false,offset,duration);
		pTone->Append(L"",false,offset,duration);
	}

	//JoinSegmentBoundaries();
	TRACE("done!\n");
	ofs.flush();
	ofs.close();
	return true;
}

bool CAutoSegmentation::DivideAndConquer( CSaDoc & doc, CSaView & view, DWORD goal, int skipCount) {

	TRACE("divide and conquer\n");

    // get parse parameters document member data
    // store new data
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
	
    doc.RestartAllProcesses();

	view.RefreshGraphs(TRUE);

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

		TRACE("max=%d min=%d next=%d\n",maxBreakWidth,minBreakWidth,nextBreakWidth);

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
		TRACE("found %d segments\n",count);

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

vector<Phonetic> Phrase::SplitPhonetics(int length) {
	vector<Phonetic> result;
	for (size_t j=length;j<phones.size();j++) {
		result.push_back(phones[j]);
	}
	phones.resize(length);
	return result;
}

void Phrase::Merge(Phrase right) {
	duration = right.offset+right.duration-offset;
	for (size_t j=0;j<right.phones.size();j++) {
		phones.push_back(right.phones[j]);
	}
}

