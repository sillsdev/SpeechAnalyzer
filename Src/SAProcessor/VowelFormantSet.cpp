#include "pch.h"
#include "VowelFormantSet.h"
#include "VowelSetVersion.h"
#include "VowelFormantsVector.h"

static LPCSTR psz_VowelSet = "VowelSet";
static LPCSTR psz_Gender = "Gender";
static LPCSTR psz_User = "UserEditable";

CVowelFormantSet::CVowelFormantSet(const wstring& szSetName, const CVowelFormantsVector vowels[3], BOOL bUser) : m_szSetName(szSetName) {
    m_bUser = bUser;
    m_vowels[0] = vowels[0];
    m_vowels[1] = vowels[1];
    m_vowels[2] = vowels[2];
}

CVowelFormantSet::CVowelFormantSet(const wstring& szSetName, BOOL bUser) : m_szSetName(szSetName) {
    m_bUser = bUser;
}

const wstring& CVowelFormantSet::GetName() const {
    return m_szSetName;
}

void CVowelFormantSet::SetName(const wstring& szName) {
    m_szSetName = szName;
}

const CVowelFormantsVector& CVowelFormantSet::GetVowelFormants(int gender) const {
    return m_vowels[gender];
}

void CVowelFormantSet::SetVowelFormants(int gender, const CVowelFormantsVector& cVector) {
    m_vowels[gender] = cVector;
}

BOOL CVowelFormantSet::IsUser() const {
    return m_bUser;
}

// Write spectrumParm properties to stream
void CVowelFormantSet::WriteProperties(ObjectOStream& obs) const {

    obs.WriteBeginMarker(psz_VowelSet, _to_utf8(m_szSetName).c_str());
    obs.WriteBool(psz_User, m_bUser);
    for (int i = 0; i < 3; i++) {
        char buffer[50];
        sprintf_s(buffer, sizeof(buffer), "%d", i);
        obs.WriteBeginMarker(psz_Gender, buffer);
        m_vowels[i].WriteProperties(obs);
        obs.WriteEndMarker(psz_Gender);
    }
    obs.WriteEndMarker(psz_VowelSet);
}

// Read spectrumParm properties from *.psa file.
BOOL CVowelFormantSet::ReadProperties(ObjectIStream& obs) {
    char buffer[1024];
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_VowelSet, buffer, _countof(buffer))) {
        return FALSE;
    }
    m_szSetName = _to_wstring(buffer);

    CVowelFormants cVowel("", UNDEFINED_DATA, UNDEFINED_DATA, UNDEFINED_DATA, UNDEFINED_DATA);
    // always empty vowel lists
    m_vowels[male].clear();
    m_vowels[female].clear();
    m_vowels[child].clear();

    // pre-reserve space to speed operation
    m_vowels[male].reserve(16);
    m_vowels[female].reserve(16);
    m_vowels[child].reserve(16);

    while (!obs.bAtEnd()) {
        char buffer[1024];
        if (obs.bReadBeginMarker(psz_Gender, buffer, _countof(buffer))) {
            string szGenderID;
            szGenderID = buffer;
            int gender = szGenderID[0] - _T('0');

            if (gender >= 3 || gender < 0) {
                gender = 0;
            }

            while (!obs.bAtEnd()) {
                if (cVowel.ReadProperties(obs)) {
                    m_vowels[gender].push_back(cVowel);
                } else if (obs.bEnd(psz_Gender)) {
                    break;
                }
            }
        } else if (obs.bReadBool(psz_User, m_bUser)) {
        } else if (obs.bEnd(psz_VowelSet)) {
            break;
        }
    }
    return TRUE;
}
