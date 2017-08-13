#include "Stdafx.h"
#include "TextHelper.h"
#include "objectostream.h"
#include <iostream>
#include <string>
#include <vector>

using std::vector;
using std::ifstream;
using std::streampos;
using std::stringstream;
using std::wstringstream;
using std::wistringstream;
using std::wstring;

/*
* uses the current position in the stream
*/
vector<wstring> TokenizeBufferToLines(wistringstream & stream) {
    // now tokenize the buffer
    vector<wstring> lines;
    wstring line;

    while (stream.good()) {
        std::wistringstream::int_type c = stream.get();
        if ((c==0x0d)||(c==0x0a)) {
            if (line.size()>0) {
                lines.push_back(line);
            }                     
            line.clear();
        } else {
            line.push_back(c);
        }
    }
    return lines;
}

vector<wstring> TokenizeLineToTokens(wstring & line, wchar_t token) {

    // now tokenize the buffer
    vector<wstring> lines;

    wstring sub;
    for (size_t i=0; i<line.length(); i++) {
        wchar_t c = line[i];
        if (c==token) {
            lines.push_back(sub);
            sub.clear();
        } else {
            sub.push_back(c);
        }
    }
    if (sub.size()>0) {
        lines.push_back(sub);
    }
    return lines;
}

bool ContainsTabs(wchar_t * buffer, int length) {
    for (int i=0; i<length; i++) {
        if (buffer[i]==0x09) {
            return true;
        }
    }
    return false;
}

bool ContainsPoundSigns(wchar_t * buffer, int length) {
    for (int i=0; i<length; i++) {
        if (buffer[i]==WORD_DELIMITER) {
            return true;
        }
    }
    return false;
}

bool ContainsUnderscore(wchar_t * buffer, int length) {
    for (int i=0; i<length; i++) {
        if (buffer[i]=='_') {
            return true;
        }
    }
    return false;
}

/**
* attempt to pull in a tab delimited file, the reference column only
*/
TranscriptionDataMap AttemptTabDelimitedRefOnly(const vector<wstring> & lines, const MarkerList & markers) {

    TranscriptionDataMap map;

    if (markers.size()!=1) {
        TRACE(L"Skipping TabDelimitedRefOnly because there's more than one column\n");
        return map;
    }

    // now tokenize the lines
    vector<vector<wstring>> list2;
    vector<wstring> tokens;
    wstring token;
    size_t min = 100000;
    size_t max = 0;
    for (vector<wstring>::const_iterator it = lines.begin(); it!=lines.end(); it++) {
        wstring line = *it;
        for (size_t i=0; i<line.size(); i++) {
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

    for (vector<vector<wstring>>::iterator it = list2.begin(); it!=list2.end(); it++) {
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

TranscriptionDataMap AttemptTabDelimited(vector<wstring> lines, MarkerList markers) {

    TranscriptionDataMap map;

    // now tokenize the lines
    vector<vector<wstring>> list2;
    vector<wstring> tokens;
    wstring token;
    size_t min = 100000;
    size_t max = 0;
    for (vector<wstring>::iterator it = lines.begin(); it!=lines.end(); it++) {
        wstring line = *it;
        for (size_t i=0; i<line.size(); i++) {
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

    for (vector<vector<wstring>>::iterator it = list2.begin(); it!=list2.end(); it++) {
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

TranscriptionDataMap AttemptWhitespaceDelimited(vector<wstring> lines, MarkerList markers) {

    TranscriptionDataMap map;

    // now tokenize the lines
    vector<vector<wstring>> list2;
    vector<wstring> tokens;
    wstring token;
    size_t min = 100000;
    size_t max = 0;
    for (vector<wstring>::iterator it = lines.begin(); it!=lines.end(); it++) {
        wstring line = *it;
        for (size_t i=0; i<line.size(); i++) {
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

    for (vector<vector<wstring>>::iterator it = list2.begin(); it!=list2.end(); it++) {
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

TranscriptionDataMap AttemptTwoMarkerWhitespaceDelimited(vector<wstring> lines, MarkerList markers) {

    TranscriptionDataMap map;

    if (markers.size()!=2) {
        TRACE(L"Skipping AttemptTwoMarkerWhitespaceDelimited because marker size != 2\n");
        return map;
    }

    // now tokenize the lines
    vector<vector<wstring>> list2;
    vector<wstring> tokens;
    wstring token;
    size_t min = 100000;
    size_t max = 0;
    // only delimit between the first and second token
    for (vector<wstring>::iterator it = lines.begin(); it!=lines.end(); it++) {
        bool first = true;
        wstring line = *it;
        for (size_t i=0; i<line.size(); i++) {
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

    for (vector<vector<wstring>>::iterator it = list2.begin(); it!=list2.end(); it++) {
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
* NOTE - uses the current position of the stream
*/
bool CTextHelper::ImportText(wistringstream & stream,
                             const CSaString & sync,
                             const MarkerList & markers,
                             TranscriptionDataMap & map,
                             bool addTag) {
    vector<wstring> lines = TokenizeBufferToLines(stream);

    CString tag;
    tag.LoadStringW(IDS_AUTO_REF_TAG);

    map = AttemptTabDelimitedRefOnly(lines,markers);
    if (map.size()>0) {
        if (addTag) {
            map[sync].push_front(tag);
        }
        return true;
    }
    map = AttemptTabDelimited(lines,markers);
    if (map.size()>0) {
        if (addTag) {
            map[sync].push_front(tag);
        }
        return true;
    }
    map = AttemptWhitespaceDelimited(lines,markers);
    if (map.size()>0) {
        if (addTag) {
            map[sync].push_front(tag);
        }
        return true;
    }
    map = AttemptTwoMarkerWhitespaceDelimited(lines,markers);
    if (map.size()>0) {
        if (addTag) {
            map[sync].push_front(tag);
        }
        return true;
    }

    for (MarkerList::const_iterator it=markers.begin(); it!=markers.end(); it++) {
        map[*it].push_back("");
    }

    return false;
}

vector<string> CTextHelper::Tokenize(const string & in, const string & delimiters) {

    vector<string> tokens;
    string::size_type delimPos = 0;
    string::size_type tokenPos = 0;
    string::size_type pos = 0;

    if (in.length()<1) {
        return tokens;
    }
    while (1) {
        delimPos = in.find_first_of(delimiters, pos);
        tokenPos = in.find_first_not_of(delimiters, pos);

        if (string::npos != delimPos) {
            if (string::npos != tokenPos) {
                if (tokenPos<delimPos) {
                    tokens.push_back(in.substr(pos,delimPos-pos));
                } else {
                    tokens.push_back("");
                }
            } else {
                tokens.push_back("");
            }
            pos = delimPos+1;
        } else {
            if (string::npos != tokenPos) {
                tokens.push_back(in.substr(pos));
            } else {
                tokens.push_back("");
            }
            break;
        }
    }
    return tokens;
}

vector<string> CTextHelper::Tokenize(const string & in, char delimiter) {

	vector<string> tokens;
	string::size_type delimPos = 0;
	string::size_type tokenPos = 0;
	string::size_type pos = 0;

	if (in.length()<1) {
		return tokens;
	}
	while (1) {
		delimPos = in.find_first_of(delimiter, pos);
		tokenPos = in.find_first_not_of(delimiter, pos);

		if (string::npos != delimPos) {
			if (string::npos != tokenPos) {
				if (tokenPos<delimPos) {
					tokens.push_back(in.substr(pos, delimPos - pos));
				} else {
					tokens.push_back("");
				}
			} else {
				tokens.push_back("");
			}
			pos = delimPos + 1;
		} else {
			if (string::npos != tokenPos) {
				tokens.push_back(in.substr(pos));
			} else {
				tokens.push_back("");
			}
			break;
		}
	}
	return tokens;
}

vector<wstring> CTextHelper::Tokenize(const wstring & in, const wstring & delimiters) {

	vector<wstring> tokens;
	string::size_type delimPos = 0;
	string::size_type tokenPos = 0;
	string::size_type pos = 0;

	if (in.length()<1) {
		return tokens;
	}
	while (1) {
		delimPos = in.find_first_of(delimiters, pos);
		tokenPos = in.find_first_not_of(delimiters, pos);

		if (string::npos != delimPos) {
			if (string::npos != tokenPos) {
				if (tokenPos<delimPos) {
					tokens.push_back(in.substr(pos, delimPos - pos));
				} else {
					tokens.push_back(L"");
				}
			} else {
				tokens.push_back(L"");
			}
			pos = delimPos + 1;
		} else {
			if (string::npos != tokenPos) {
				tokens.push_back(in.substr(pos));
			} else {
				tokens.push_back(L"");
			}
			break;
		}
	}
	return tokens;
}

vector<wstring> CTextHelper::Tokenize(const wstring & in, wchar_t delimiter) {

	vector<wstring> tokens;
	string::size_type delimPos = 0;
	string::size_type tokenPos = 0;
	string::size_type pos = 0;

	if (in.length()<1) {
		return tokens;
	}
	while (1) {
		delimPos = in.find_first_of(delimiter, pos);
		tokenPos = in.find_first_not_of(delimiter, pos);

		if (string::npos != delimPos) {
			if (string::npos != tokenPos) {
				if (tokenPos<delimPos) {
					tokens.push_back(in.substr(pos, delimPos - pos));
				} else {
					tokens.push_back(L"");
				}
			} else {
				tokens.push_back(L"");
			}
			pos = delimPos + 1;
		} else {
			if (string::npos != tokenPos) {
				tokens.push_back(in.substr(pos));
			} else {
				tokens.push_back(L"");
			}
			break;
		}
	}
	return tokens;
}

