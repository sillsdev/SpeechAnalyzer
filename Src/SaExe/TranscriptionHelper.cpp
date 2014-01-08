#include "Stdafx.h"
#include "TranscriptionHelper.h"
#include "TranscriptionData.h"

static const wchar_t * TAB = L"\x09";
static const wchar_t * CRLF = L"\r\n";

CSaString CTranscriptionHelper::Render(CTranscriptionData & td)
{

    // at this point the map lists should be equal size
    // we will create a list of iterators
    CSaString result;
    typedef std::map<CSaString,MarkerList::iterator> IndexMap;
    IndexMap indexes;
    // create a list of iterators for all markers of everything except the sync marker
    for (TranscriptionDataMap::iterator it = td.m_TranscriptionData.begin(); it!=td.m_TranscriptionData.end(); it++)
    {
		if (it->first.CompareNoCase(td.m_szPrimary)==0)
        {
            continue;
        }
        indexes[it->first] = it->second.begin();
    }

    // iterator through the map
    for (MarkerList::iterator it = td.m_TranscriptionData[td.m_szPrimary].begin(); it!=td.m_TranscriptionData[td.m_szPrimary].end(); it++)
    {
        // first write the sync marker data
        result.Append(*it);
        result.Append(TAB);
        for (MarkerList::iterator it2 = td.m_Markers.begin(); it2!=td.m_Markers.end(); it2++)
        {
            if ((*it2).CompareNoCase(td.m_szPrimary)==0)
            {
                continue;
            }
			CSaString value = "?";
			if (indexes.count(*it2)>0)
			{
				value = *indexes[*it2];
	            indexes[*it2]++;
			}
            result.Append(value);
            result.Append(TAB);
        }
        result.Append(CRLF);
    }
    return result;
}
