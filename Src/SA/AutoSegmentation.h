#ifndef AUTOSEGMENTATION_H
#define AUTOSEGMENTATION_H

using std::list;

class Phonetic {
public:
	DWORD start;
	DWORD stop;
};

class Phrase {
public:
	wstring GetPhonesAsText();
	vector<Phonetic> SplitPhonetics(int pos);
	void Merge(Phrase right);
	void FilterPhones();

	DWORD offset;
	DWORD duration;
	vector<Phonetic> phones;
};

class Verse {
public:
	wstring text;
	wstring ref;
};

typedef vector<Verse>::iterator verse_iterator;
typedef vector<Verse>::reverse_iterator verse_reverse_iterator;
typedef vector<Phrase>::iterator phrase_iterator;
typedef vector<Phrase>::reverse_iterator phrase_reverse_iterator;
typedef vector<Phonetic>::iterator phone_iterator;
typedef vector<vector<Phrase>> phrase_path;

class CTranscriptionData;
class CSaView;
class CGlossSegment;
class CReferenceSegment;
class CPhoneticSegment;

class CAutoSegmentation {
public:
	bool PhoneticMatching( CSaDoc & doc, CTranscriptionData & td, int skipCount, bool usingGL);
	bool DivideAndConquer( CSaDoc & doc, CSaView & view, DWORD goal, int skipCount);

private:
	void JoinSegmentBoundaries( CSaDoc & doc);
	vector<Verse> GetVerses( CTranscriptionData & td, bool usingGL);
	vector<Phonetic> GetPhones( CSaDoc & doc, wofstream & ofs );
	vector<Phrase> GetPhrases( CSaDoc & doc, wofstream & ofs, int goal);
	vector<Phrase> GetPhrases( CSaDoc & doc, wofstream & ofs);
	void DumpSegments( wofstream & ofs, vector<Verse> & verses, vector<Phrase> & phrases, int to);
	int GetNonVoicedCharacterCount( wstring text);
	phrase_path ConsiderPaths( wofstream & ofs, vector<Verse> verses, size_t v, vector<Phrase> phrases, size_t p, size_t depth);
	float CalculateScore( Verse & verse, Phrase & phrase);
	float CalculateScore( vector<Verse> & verse, vector<Phrase> & phrase, size_t depth);
	float FindMinError( vector<Verse> & verse, vector<Phrase> & phrase, size_t depth);
	float FindMaxError( vector<Verse> & verse, vector<Phrase> & phrase, size_t depth);

	string GetKey( size_t v, DWORD o, DWORD d);

	//map<string,phrase_path> cache;
};

#endif
