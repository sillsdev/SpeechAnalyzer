#include "pch.h"
#include "VowelFormants.h"

CVowelFormantSets& GetVowelSets() {
    CSaString szPath(AfxGetApp()->GetProfileString(_T(""), _T("DataLocation")));
    if (szPath.Right(1) != _T("\\")) {
        szPath += _T("\\");
    }

    szPath = szPath + _T("vowelsUtf8.psa");

    static CVowelFormantSets theVowelSets(szPath);
    return theVowelSets;
}

const CVowelFormantSet& GetDefaultVowelSet() {
    CVowelFormantSets& cSets = GetVowelSets();
    return cSets[cSets.GetDefaultSet()];
}

const std::vector<CVowelFormants>& GetVowelVector(int nGender) {
    return GetDefaultVowelSet().GetVowelFormants(nGender);
}

