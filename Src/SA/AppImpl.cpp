#include "stdafx.h"
#include "AppImpl.h"

CVowelFormantSets & CAppImpl::GetVowelSets() {
    string szPath(GetProfileString(_T(""), _T("DataLocation")));
    if (szPath.Right(1) != _T("\\")) {
        szPath += "\\";
    }
    szPath = szPath + "vowelsUtf8.psa";
    static CVowelFormantSets theVowelSets(szPath);
    return theVowelSets;
}

const CVowelFormantSet& CAppImpl::GetDefaultVowelSet() {
    CVowelFormantSets& cSets = GetVowelSets();
    return cSets[cSets.GetDefaultSet()];
}

const CVowelFormantsVector& CAppImpl::GetVowelVector(int nGender) {
    return GetDefaultVowelSet().GetVowelFormants(nGender);
}
