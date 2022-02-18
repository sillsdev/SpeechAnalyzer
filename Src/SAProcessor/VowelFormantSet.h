#pragma once
#ifndef _VOWELFORMANTSET_H
#define _VOWELFORMANTSET_H

#include "VowelFormants.h"
#include "VowelFormantsVector.h"

class CVowelFormantSet {
public:
    CVowelFormantSet(const wstring& szSetName, const CVowelFormantsVector vowels[3], BOOL bUser = TRUE);
    CVowelFormantSet(const wstring& szSetName = _T(""), BOOL bUser = TRUE);

    void WriteProperties(ObjectOStream& obs) const;
    BOOL ReadProperties(ObjectIStream& obs);

    const wstring& GetName() const;
    void SetName(const wstring& szName);
    const CVowelFormantsVector& GetVowelFormants(int gender) const;
    void SetVowelFormants(int gender, const CVowelFormantsVector& cVector);
    BOOL IsUser() const;

    enum Gender { male = 0, female = 1, child = 2 };

private:
    BOOL m_bUser;
    wstring m_szSetName;
    CVowelFormantsVector m_vowels[3];
};

#endif
