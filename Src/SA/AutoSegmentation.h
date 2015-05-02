#ifndef AUTOSEGMENTATION_H
#define AUTOSEGMENTATION_H

using std::list;

class Phonetic {
public:
	DWORD start;
	DWORD stop;
};

class Word {
public:
	DWORD start;
	DWORD stop;
};

class Phrase {
public:
	wstring GetPhonesAsText();
	void Merge(Phrase right);
	void FilterPhones();
	void FilterWords();

	DWORD offset;
	DWORD duration;
	vector<Phonetic> phones;
	vector<Word> words;
};

class Verse {
public:
	Verse( LPCTSTR ref, LPCTSTR text, size_t aWordCount, size_t aPhoneCount);
	Verse( const Verse & right);
	wstring ref;
	wstring GetText();
	size_t GetWordCount();
	size_t GetPhoneCount();
private:
	wstring text;
	size_t wordCount;
	size_t phoneCount;
};

typedef vector<Verse>::iterator verse_iterator;
typedef vector<Phrase>::iterator phrase_iterator;
typedef vector<Phonetic>::iterator phone_iterator;

class CTranscriptionData;
class CSaView;
class CGlossSegment;
class CReferenceSegment;
class CPhoneticSegment;

class CAutoSegmentation {
public:
	bool DoPhoneticMatching( LPCTSTR filename, CSaDoc & doc, CTranscriptionData & td, int skipCount, bool usingGL);
	bool DoDivideAndConquer( CSaDoc & doc, CSaView & view, DWORD goal, int skipCount);

private:
	void JoinSegmentBoundaries( CSaDoc & doc);
	vector<Verse> GetVerses( CTranscriptionData & td, bool usingGL);
	vector<Phonetic> GetPhones( CSaDoc & doc, wofstream & ofs );
	vector<Word> GetWords( CSaDoc & doc, wofstream & ofs );
	vector<Phrase> GetPhrases( CSaDoc & doc, wofstream & ofs, int goal);
	vector<Phrase> GetPhrases( CSaDoc & doc, wofstream & ofs);
	void DumpSegments( wofstream & ofs, vector<Verse> & verses, vector<Phrase> & phrases);
	size_t GetNonVoicedCharacterCount( wstring text);
	size_t GetWordCount( wstring text);
	vector<Phrase> ConsiderPaths( wofstream & ofs, vector<Verse> & verses, vector<Phrase> phrases, size_t v, size_t p, size_t indent);
	vector<Phrase> ConsiderPathsImpl( wofstream & ofs, vector<Verse> & verses, vector<Phrase> phrases, size_t v, size_t p, size_t indent);
	float ScorePhone( Verse & verse, Phrase & phrase);
	float ScoreWord( Verse & verse, Phrase & phrase);
	float ScorePath(  wofstream & ofs, wstring tabs, vector<Verse> & verse, vector<Phrase> & phrase, size_t v, size_t p, LPCTSTR tag);

	string GetKey( size_t v, DWORD o, DWORD d);
	wstring GetTabs(size_t indent);

	bool NeedsSplit( Verse & v, Phrase & p);
	bool NeedsMerge( Verse & v, Phrase & p);

	map<string,vector<Phrase>> cache;
};

#endif
