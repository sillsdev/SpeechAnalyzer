#include "stdafx.h"
#include "SFMHelper.h"
#include <iterator>
#include "TextHelper.h"
#include "StringStream.h"
#include <algorithm>

static LPCTSTR IMPORT_END = L"import";
static LPCTSTR EMPTY = L"";

using std::list;
using std::map;
using std::wstringstream;

wstring CSFMHelper::ToLower(wstring in) {
    std::transform(in.begin(), in.end(), in.begin(), ::towlower);
    return in;
}

bool CSFMHelper::IsSFM(wistringstream & stream) {
    // rewind the stream
    stream.clear();
    stream.seekg(0);
    stream.clear();

    if (stream.fail()) {
        return false;
    }
    return (stream.peek()=='\\');
}

/**
* we will just count markers - if there's multiple of one type
* then it's a multirecord file
*
*/
bool CSFMHelper::IsMultiRecordSFM(wistringstream & stream, CSaString & marker) {
    // rewind the stream
    stream.clear();
    stream.seekg(0);
    stream.clear();

    int count = 0;
    CStringStream cstream(stream.str().c_str());
    if (cstream.bFail()) {
        return false;
    }
    if (!cstream.bAtBackslash()) {
        return false;
    }

    CSaString buffer;
    while (!cstream.bAtEnd()) {
        if (cstream.ReadStreamString(marker, buffer)) {
            count++;
        } else {
            cstream.bEnd(IMPORT_END);
        }
    }
    return (count>1);
}

/**
* Read in a SFM data stream.
* Parse out all types defined in the markers list.
* Whenever syncMarker is encountered, the list will be balanced for consistency
* Return a map using key marker as a key and the list of value for that marker
*/
TranscriptionDataMap CSFMHelper::ImportMultiRecordSFM(wistringstream & stream, CSaString & syncMarker, MarkerList & markers, bool /*addTag*/) {
    // rewind the stream
    stream.clear();
    stream.seekg(0);
    stream.clear();

    TranscriptionDataMap result;

    CStringStream cstream(stream.str().c_str());
    if (cstream.bFail()) {
        return result;
    }
    if (!cstream.bAtBackslash()) {
        return result;
    }

    /**
    * we will build up lists of each of the imported types,
    * then we will build the output after the data
    * has been completely read
    */
    while (!cstream.bAtEnd()) {
        MarkerList::const_iterator it = markers.begin();
        bool found = false;
        for (MarkerList::iterator it = markers.begin(); it != markers.end(); it++) {
            CSaString buffer;
            CSaString marker = *it;
            if (cstream.ReadStreamString(marker, buffer)) {
				buffer = buffer.Trim();
                result[marker.MakeLower()].push_back(buffer);
                // when see the sync marker, balance the other entries.
                if (marker.Compare(syncMarker)==0) {
                    BalanceDataMap(result, syncMarker);
                }
                found=true;
                break;
            }
        }
        if (!found) {
            //we are at string that doesn't match - skip over it
            cstream.bEnd(IMPORT_END);
        }
    }

    BalanceDataMap(result, syncMarker);

    return result;
}

void CSFMHelper::BalanceDataMap(TranscriptionDataMap & map, CSaString & marker) {

    if (map[marker].size()>0) {
        TranscriptionDataMap::size_type length = map[marker].size()-1;
        for (TranscriptionDataMap::iterator pos=map.begin(); pos!=map.end(); pos++) {
            if (marker.Compare(pos->first)!=0) {
                while (pos->second.size()<length) {
                    pos->second.push_back(EMPTY);
                }
            }
        }
    }
}

bool CSFMHelper::IsColumnarSFM(wistringstream & stream) {
    // rewind the stream
    stream.clear();
    stream.seekg(0);
    stream.clear();

    vector<wstring> lines = TokenizeBufferToLines(stream);

    lines = CSFMHelper::FilterBlankLines(lines);

    if (lines.size()==0) {
        TRACE("file is empty\n");
        return false;
    }

    size_t start = 0;
    while (true) {
        if (start>=lines.size()) {
            return false;
        }
        if (lines[start].length()>0) {
            break;
        }
        start++;
    }

    // we are now sitting at the first non-blank line
    wstring line = lines[start];
    vector<wstring> tokens = TokenizeLineToTokens(line, 0x09);
    // the first line must be all tags.
    for (size_t i=0; i<tokens.size(); i++) {
        if (!CSFMHelper::IsTag(tokens[i].c_str())) {
            return false;
        }
    }

    // the second line should not have any tags
    start++;
    line = lines[start];
    tokens = TokenizeLineToTokens(line, 0x09);
    for (size_t i=0; i<tokens.size(); i++) {
        if (CSFMHelper::IsTag(tokens[i].c_str())) {
            return false;
        }
    }

    // rewind
    start = 0;
    while (true) {
        if (start>=lines.size()) {
            return false;
        }
        if (lines[start].length()>0) {
            break;
        }
        start++;
    }

    // we are now sitting at the first non-blank line
    wstring tagline = lines[start];
    vector<wstring> tags = TokenizeLineToTokens(tagline, 0x09);
    // the first line must be all tags.
    for (size_t i=0; i<tags.size(); i++) {
        if (!CSFMHelper::IsTag(tags[i].c_str())) {
            TRACE("the first line contains an element that is not a tag '%s'\n",tagline.c_str());
            return false;
        }
    }
    size_t tagCount = tags.size();
    start++;

    // run through all the lines and verify that the counts are no greater
    // than the number of tags
    for (size_t i=start; i<lines.size(); i++) {
        vector<wstring> tokens2 = TokenizeLineToTokens(lines[i], 0x09);
        if (tokens2.size()>tagCount) {
            TRACE("line %d has too many elements '%s'.  The tag count id %d\n",i,lines[i].c_str(),tagCount);
            return false;
        }
    }

    // run through all the lines and verify that there are no more tags
    // on any other lines
    for (size_t i=start; i<lines.size(); i++) {
        vector<wstring> tokens3 = TokenizeLineToTokens(lines[i], 0x09);
        for (size_t j=0; j<tokens3.size(); j++) {
            if (CSFMHelper::IsTag(tokens3[j].c_str())) {
                TRACE("line %d contains a tag '%s'.\n",i,lines[i].c_str());
                return false;
            }
        }
    }

    return true;
}

TranscriptionDataMap CSFMHelper::ImportColumnarSFM(wistringstream & stream) {
    // rewind stream
    stream.clear();
    stream.seekg(0);
    stream.clear();

    TranscriptionDataMap td;

    vector<wstring> lines = TokenizeBufferToLines(stream);

    lines = CSFMHelper::FilterBlankLines(lines);

    if (lines.size()==0) {
        return td;
    }

    size_t start = 0;
    while (true) {
        if (start>=lines.size()) {
            return td;
        }
        if (lines[start].length()>0) {
            break;
        }
        start++;
    }

    // we are now sitting at the first non-blank line
    wstring tagline = lines[start];
    vector<wstring> tags = TokenizeLineToTokens(tagline, 0x09);
    // the first line must be all tags.
    for (size_t i=0; i<tags.size(); i++) {
        if (!CSFMHelper::IsTag(tags[i].c_str())) {
            TRACE("the first line contains an element that is not a tag '%s'\n",tagline.c_str());
            return td;
        }
    }
    size_t tagCount = tags.size();
    start++;

    // run through all the lines and verify that the counts are no greater
    // than the number of tags
    for (size_t i=start; i<lines.size(); i++) {
        vector<wstring> tokens = TokenizeLineToTokens(lines[i], 0x09);
        if (tokens.size()>tagCount) {
            TRACE("line %d has too many elements '%s'.  The tag count id %d\n",i,lines[i].c_str(),tagCount);
            return td;
        }
    }

    // run through all the lines and verify that there are no more tags
    // than the number of tags
    for (size_t i=start; i<lines.size(); i++) {
        vector<wstring> tokens = TokenizeLineToTokens(lines[i], 0x09);
        for (size_t j=0; j<tokens.size(); j++) {
            if (CSFMHelper::IsTag(tokens[j].c_str())) {
                TRACE("line %d contains a tag '%s'.\n",i,lines[i].c_str());
                return td;
            }
        }
    }

    // now append the data to the rows
    for (size_t i=start; i<lines.size(); i++) {
        vector<wstring> tokens = TokenizeLineToTokens(lines[i], 0x09);
        for (size_t j=0; j<tags.size(); j++) {
            wstring tag = tags[j];
            tag = (tag[0]=='\\')?tag.substr(1):tag;
            wstring data = L"";
            if (j<tokens.size()) {
                data.append(tokens[j]);
            } else {
                data = L"";
            }
            td[ToLower(tag).c_str()].push_back(data.c_str());
        }
    }
    return td;
}

TranscriptionDataMap CSFMHelper::ImportSFM(CSaString & /*filename*/) {
    TranscriptionDataMap map;
    return map;
}

bool CSFMHelper::IsTag(LPCTSTR text) {
    if (wcslen(text)==0) {
        return false;
    }
    if (text[0] != '\\') {
        return false;
    }
    return true;
}

vector<wstring> CSFMHelper::FilterBlankLines(vector<wstring> & input) {
    vector<wstring> result;
    for (size_t i=0; i<input.size(); i++) {
        if (input[i].length()>0) {
            result.push_back(input[i].c_str());
        }
    }
    return result;
}

bool CSFMHelper::IsPhonemic(LPCTSTR text, size_t length) {
    if (length<3) {
        return false;
    }
    if (::tolower(text[0])!='\\') {
        return false;
    }
    if (::tolower(text[1])!='p') {
        return false;
    }
    if (::tolower(text[2])!='m') {
        return false;
    }
    return true;
}

bool CSFMHelper::IsPhonetic(LPCTSTR text, size_t length) {
    if (length<3) {
        return false;
    }
    if (::tolower(text[0])!='\\') {
        return false;
    }
    if (::tolower(text[1])!='p') {
        return false;
    }
    if (::tolower(text[2])!='h') {
        return false;
    }
    return true;
}

bool CSFMHelper::IsOrthographic(LPCTSTR text, size_t length) {
    if (length<3) {
        return false;
    }
    if (::tolower(text[0])!='\\') {
        return false;
    }
    if (::tolower(text[1])!='o') {
        return false;
    }
    if (::tolower(text[2])!='r') {
        return false;
    }
    return true;
}

bool CSFMHelper::IsGloss(LPCTSTR text, size_t length) {
    if (length<3) {
        return false;
    }
    if (::tolower(text[0])!='\\') {
        return false;
    }
    if (::tolower(text[1])!='g') {
        return false;
    }
    if (::tolower(text[2])!='l') {
        return false;
    }
    return true;
}

bool CSFMHelper::IsGlossNat(LPCTSTR text, size_t length) {
    if (length<3) {
        return false;
    }
    if (::tolower(text[0])!='\\') {
        return false;
    }
    if (::tolower(text[1])!='g') {
        return false;
    }
    if (::tolower(text[2])!='n') {
        return false;
    }
    return true;
}

bool CSFMHelper::IsRef(LPCTSTR text, size_t length) {
    if (length<4) {
        return false;
    }
    if (::tolower(text[0])!='\\') {
        return false;
    }
    if (::tolower(text[1])!='r') {
        return false;
    }
    if (::tolower(text[2])!='e') {
        return false;
    }
    if (::tolower(text[3])!='f') {
        return false;
    }
    return true;
}

/***************************************************************************/
// extractTabField local helper function to get field from tab delimited string
/***************************************************************************/
const CSaString CSFMHelper::ExtractTabField(const CSaString & szLine, const int nField) {
    int nCount = 0;
    int nLoop = 0;

    if (nField < 0) {
        return "";    // SDM 1.5Test10.1
    }

    while ((nLoop < szLine.GetLength()) && (nCount < nField)) {
        if (szLine[nLoop] == '\t') {
            nCount++;
        }
        nLoop++;
    }
    int nBegin = nLoop;
    while ((nLoop < szLine.GetLength()) && (szLine[nLoop] != '\t')) {
        nLoop++;
    }
    return szLine.Mid(nBegin, nLoop-nBegin);
}

