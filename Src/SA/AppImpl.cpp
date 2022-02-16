#include "stdafx.h"
#include "AppImpl.h"

VowelFormantSets & CAppImpl::GetVowelSets() {
    string szPath(GetProfileString(_T(""), _T("DataLocation")));
    if (szPath.Right(1) != _T("\\")) {
        szPath += "\\";
    }
    szPath = szPath + "vowelsUtf8.psa";
    static CVowelFormantSets theVowelSets(szPath);
    return theVowelSets;
}

const VowelFormantSet& CAppImpl::GetDefaultVowelSet() {
    CVowelFormantSets& cSets = GetVowelSets();
    return cSets[cSets.GetDefaultSet()];
}

const std::vector<CVowelFormants>& CAppImpl::GetVowelVector(int nGender) {
    return GetDefaultVowelSet().GetVowelFormants(nGender);
}
