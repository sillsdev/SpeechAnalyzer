#include "Stdafx.h"
#include "TextHelper.h"
#include "Settings/OBSTREAM.H"
#include "resource.h"
#include "AppDefs.h"
#include <iostream>
#include <fstream>
#include <string>

using std::vector;
using std::string;
using std::ifstream;
using std::wstring;

bool isUTF8( BYTE * buffer, std::ios::pos_type length) {
	//EF BB BF		UTF-8
	if (length<3) return false;
	bool result = ((buffer[0]==0xef)&(buffer[1]==0xbb)&(buffer[2]==0xbf));
	return result;
}

bool isUTF16( BYTE * buffer, std::ios::pos_type length) {
	//FE FF			UTF-16, big-endian
	//FF FE			UTF-16, little-endian
	if (length<2) return false;
	if ((buffer[0]==0xfe)&&(buffer[1]==0xff)) return true;
	if ((buffer[0]==0xff)&&(buffer[1]==0xfe)) return true;
	return false;
}

bool isUTF32( BYTE * buffer, std::ios::pos_type length) {
	//00 00 FE FF		UTF-32, big-endian
	//FF FE 00 00		UTF-32, little-endian
	if (length<4) return false;
	if ((buffer[0]==0)&&(buffer[1]==0)&&(buffer[2]==0xfe)&&(buffer[3]==0xff)) return true;
	if ((buffer[0]==0xff)&&(buffer[1]==0xfe)&&(buffer[2]==0x00)&&(buffer[3]==0x00)) return true;
	return false;
}

bool isASCII( BYTE * buffer, std::ios::pos_type length) {
	for (int i=0;i<length;i++) {
		if (buffer[i]>0x7f) return false;
	}
	return true;
}

bool containsTabs( wchar_t * buffer, int length) {
	for (int i=0;i<length;i++) {
		if (buffer[i]==0x09) return true;
	}
	return false;
}

bool containsPoundSigns( wchar_t * buffer, int length) {
	for (int i=0;i<length;i++) {
		if (buffer[i]==WORD_DELIMITER) return true;
	}
	return false;
}

bool containsUnderscore( wchar_t * buffer, int length) {
	for (int i=0;i<length;i++) {
		if (buffer[i]=='_') return true;
	}
	return false;
}

/**
* attempt to pull in a tab delimited file, the reference column only
*/
TranscriptionDataMap attemptTabDelimitedRefOnly( const vector<wstring> & lines, const MarkerList & markers) {

	TranscriptionDataMap map;

	if (markers.size()!=1) {
		TRACE(L"Skipping TabDelimitedRefOnly because there's more than one column\n");
		return map;
	}

	// now tokenize the lines
	vector<vector<wstring>> list2;
	vector<wstring> tokens;
	wstring token;
	int min = 100000;
	int max = 0;
	for (vector<wstring>::const_iterator it = lines.begin();it!=lines.end();it++) {
		wstring line = *it;
		for (int i=0;i<line.size();i++) {
			wchar_t c = line[i];
			if (line[i]==0x09) {
				tokens.push_back(token);
				token.clear();
			} else {
				token.push_back(c);
			}
		}
		tokens.push_back(token);
		token.clear();

		min = (tokens.size()<min)?tokens.size():min;
		max = (tokens.size()>max)?tokens.size():max;
		list2.push_back(tokens);
		tokens.clear();
	}

	// count the tokens if the lengths are the same then this is a simple list...
	// and the length matches the marker size, this is as easy as it gets!
	if (min!=max) {
		TRACE(L"Skipping TabDelimitedRefOnly because there's a discrepancy in the list\n");
		return map;
	}

	for (vector<vector<wstring>>::iterator it = list2.begin();it!=list2.end();it++) {
		vector<wstring> tokens = *it;
		vector<wstring>::iterator it2 = tokens.begin();
		MarkerList::const_iterator it3 = markers.begin();
		while (it3!=markers.end()) {
			CSaString marker = *it3;
			wstring token = *it2;
			map[marker].push_back(CSaString(token.c_str()));
			it2++;
			it3++;
		}
	}
	return map;
}

TranscriptionDataMap attemptTabDelimited( vector<wstring> lines, MarkerList markers) {

	TranscriptionDataMap map;

	// now tokenize the lines
	vector<vector<wstring>> list2;
	vector<wstring> tokens;
	wstring token;
	int min = 100000;
	int max = 0;
	for (vector<wstring>::iterator it = lines.begin();it!=lines.end();it++) {
		wstring line = *it;
		for (int i=0;i<line.size();i++) {
			wchar_t c = line[i];
			if (line[i]==0x09) {
				tokens.push_back(token);
				token.clear();
			} else {
				token.push_back(c);
			}
		}
		tokens.push_back(token);
		token.clear();

		min = (tokens.size()<min)?tokens.size():min;
		max = (tokens.size()>max)?tokens.size():max;
		list2.push_back(tokens);
		tokens.clear();
	}

	// count the tokens if the lengths are the same then this is a simple list...
	// and the length matches the marker size, this is as easy as it gets!
	if (min!=max) {
		TRACE(L"Skipping TabDelimited because there's a discrepancy in the list\n");
		return map;
	}
	if (min!=markers.size()) {
		TRACE(L"Skipping TabDelimited because a line is missing data\n");
		return map;
	}

	for (vector<vector<wstring>>::iterator it = list2.begin();it!=list2.end();it++) {
		vector<wstring> tokens = *it;
		vector<wstring>::iterator it2 = tokens.begin();
		MarkerList::const_iterator it3 = markers.begin();
		while (it3!=markers.end()) {
			CSaString marker = *it3;
			wstring token = *it2;
			map[marker].push_back(CSaString(token.c_str()));
			it2++;
			it3++;
		}
	}
	return map;
}

TranscriptionDataMap attemptWhitespaceDelimited( vector<wstring> lines, MarkerList markers) {

	TranscriptionDataMap map;

	// now tokenize the lines
	vector<vector<wstring>> list2;
	vector<wstring> tokens;
	wstring token;
	int min = 100000;
	int max = 0;
	for (vector<wstring>::iterator it = lines.begin();it!=lines.end();it++) {
		wstring line = *it;
		for (int i=0;i<line.size();i++) {
			wchar_t c = line[i];
			if ((line[i]==0x20)||(line[i]==0x09)) {
				tokens.push_back(token);
				token.clear();
			} else {
				token.push_back(c);
			}
		}
		tokens.push_back(token);
		token.clear();

		min = (tokens.size()<min)?tokens.size():min;
		max = (tokens.size()>max)?tokens.size():max;
		list2.push_back(tokens);
		tokens.clear();
	}

	// count the tokens if the lengths are the same then this is a simple list...
	// and the length matches the marker size, this is as easy as it gets!
	if (min!=max) {
		TRACE(L"Skipping TabDelimitedRefOnly because there's missing data\n");
		return map;
	}
	if (min!=markers.size()) {
		TRACE(L"Skipping TabDelimitedRefOnly because there's a discrepancy in the list\n");
		return map;
	}

	for (vector<vector<wstring>>::iterator it = list2.begin();it!=list2.end();it++) {
		vector<wstring> tokens = *it;
		vector<wstring>::iterator it2 = tokens.begin();
		MarkerList::const_iterator it3 = markers.begin();
		while (it3!=markers.end()) {
			CSaString marker = *it3;
			wstring token = *it2;
			map[marker].push_back(CSaString(token.c_str()));
			it2++;
			it3++;
		}
	}
	return map;
}

TranscriptionDataMap attemptTwoMarkerWhitespaceDelimited( vector<wstring> lines, MarkerList markers) {

	TranscriptionDataMap map;

	if (markers.size()!=2) {
		TRACE(L"Skipping AttemptTwoMarkerWhitespaceDelimited because marker size != 2\n");
		return map;
	}

	// now tokenize the lines
	vector<vector<wstring>> list2;
	vector<wstring> tokens;
	wstring token;
	int min = 100000;
	int max = 0;
	// only delimit between the first and second token
	for (vector<wstring>::iterator it = lines.begin();it!=lines.end();it++) {
		bool first = true;
		wstring line = *it;
		for (int i=0;i<line.size();i++) {
			wchar_t c = line[i];
			if ((first)&&((line[i]==0x20)||(line[i]==0x09))) {
				tokens.push_back(token);
				token.clear();
				first = false;
			} else {
				token.push_back(c);
			}
		}
		tokens.push_back(token);
		token.clear();

		min = (tokens.size()<min)?tokens.size():min;
		max = (tokens.size()>max)?tokens.size():max;
		list2.push_back(tokens);
		tokens.clear();
	}

	// we are only expecting two tokens per line
	if (min!=markers.size()) {
		TRACE("Skipping AttemptTwoMarkerWhitespaceDelimited because there's missing data\n");
		return map;
	}
	if (max<markers.size()) {
		return map;
	}

	for (vector<vector<wstring>>::iterator it = list2.begin();it!=list2.end();it++) {
		vector<wstring> tokens = *it;
		vector<wstring>::iterator it2 = tokens.begin();
		MarkerList::const_iterator it3 = markers.begin();
		while (it3!=markers.end()) {
			CSaString marker = *it3;
			wstring token = *it2;
			map[marker].push_back(CSaString(token.c_str()));
			it2++;
			it3++;
		}
	}
	return map;
}

/**
* addTag - true if we should add 'Tag' to the beginning of reference markers
*/
bool CTextHelper::ImportText( const CSaString & filename, 
							  const CSaString & sync, 
							  const MarkerList & markers, 
							  TranscriptionDataMap & map,
							  bool addTag) {

	ifstream file(filename, std::ios::binary);
	if (!file) {
		// file failed to open
		return false;
	}

	// determine size
	file.seekg(0, std::ios::end);
	std::ios::pos_type length = file.tellg();
	file.seekg(0, std::ios::beg);

	// read the file completely
	char * buffer = new char[length];
	int i=0;
	char c;
	while (file.get(c)) {
		buffer[i++] = c;
	}
	file.close();

	wchar_t * obuffer = NULL;
	int length2 = 0;
	if (isUTF8((BYTE*)buffer,length)) {
		length2 = MultiByteToWideChar(CP_UTF8,0,buffer,length,NULL,0);
		obuffer = new wchar_t[length2];
		int result = MultiByteToWideChar(CP_UTF8,0,buffer,length,obuffer,length2);
		delete [] buffer;
		buffer = NULL;
		length = 0;
	} else if (isUTF16((BYTE*)buffer,length)) {
		// just recast the pointer?
		obuffer = (wchar_t*)buffer;
		length2 = length/2;
		buffer = NULL;
		length = 0;
	} else if (isUTF32((BYTE*)buffer,length)) {
		// not supported yet
		return false;
	} else if (isASCII((BYTE*)buffer,length)) {
		// assume ascii
		length2 = MultiByteToWideChar(CP_ACP,0,buffer,length,NULL,0);
		obuffer = new wchar_t[length2];
		int result = MultiByteToWideChar(CP_ACP,0,buffer,length,obuffer,length2);
		delete [] buffer;
		buffer = NULL;
		length = 0;
	} else {
		// assume ascii
		length2 = MultiByteToWideChar(CP_ACP,0,buffer,length,NULL,0);
		obuffer = new wchar_t[length2];
		int result = MultiByteToWideChar(CP_ACP,0,buffer,length,obuffer,length2);
		delete [] buffer;
		buffer = NULL;
		length = 0;
	}

	int start = 0;
	if ((obuffer[0]==0xfeff)||(obuffer[0]==0xfffe)) {
		start = 1;
	}

	// now tokenize the buffer
	vector<wstring> lines;
	wstring line;
	for (int i=start;i<length2;i++) {
		wchar_t c = obuffer[i];
		if ((c==0x0d)||(c==0x0a)) {
			if (line.size()>0) {
				lines.push_back(line);
			}
			line.clear();
		} else {
			line.push_back(c);
		}
	}

	delete [] obuffer;

	CString tag;
	tag.LoadStringW(IDS_AUTO_REF_TAG);

	map = attemptTabDelimitedRefOnly(lines,markers);
	if (map.size()>0) {
		if (addTag) map[sync].push_front(tag);
		return true;
	}
	map = attemptTabDelimited(lines,markers);
	if (map.size()>0) { 
		if (addTag) map[sync].push_front(tag);
		return true;
	}
	map = attemptWhitespaceDelimited(lines,markers);
	if (map.size()>0) {
		if (addTag) map[sync].push_front(tag);
		return true;
	}
	map = attemptTwoMarkerWhitespaceDelimited(lines,markers);
	if (map.size()>0) {
		if (addTag) map[sync].push_front(tag);
		return true;
	}

	for (MarkerList::const_iterator it=markers.begin();it!=markers.end();it++) {
		map[*it].push_back("");
	}

	return false;
}

vector<string> Tokenize( const string & str, const string & delimiters) {

	vector<string> tokens;
	string::size_type delimPos = 0;
	string::size_type tokenPos = 0;
	string::size_type pos = 0;

	if (str.length()<1)  return tokens;
	while(1) {
		delimPos = str.find_first_of(delimiters, pos);
		tokenPos = str.find_first_not_of(delimiters, pos);

		if (string::npos != delimPos) {
			if (string::npos != tokenPos) {
				if (tokenPos<delimPos) {
					tokens.push_back(str.substr(pos,delimPos-pos));
				} else {
					tokens.push_back("");
				}
			} else {
				tokens.push_back("");
			}
			pos = delimPos+1;
		} else {
			if (string::npos != tokenPos) {
				tokens.push_back(str.substr(pos));
			} else {
				tokens.push_back("");
			}
			break;
		}
	}
	return tokens;
}
