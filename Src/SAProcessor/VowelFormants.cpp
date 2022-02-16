#include "pch.h"
#include "VowelFormants.h"
#include "VowelSetVersion.h"

CVowelFormants::CVowelFormants(const string& szVowel, double inF1, double inF2, double inF3, double inF4) {
    Init(szVowel, inF1, inF2, inF3, inF4);
}

void CVowelFormants::Init(const string& szVowel, double inF1, double inF2, double inF3, double inF4) {
    m_szVowel = szVowel;
    F1 = inF1;
    F2 = inF2;
    F3 = inF3;
    F4 = inF4;
}

static LPCSTR psz_Vowel = "Vowel";
static LPCSTR psz_F1 = "F1";
static LPCSTR psz_F2 = "F2";
static LPCSTR psz_F3 = "F3";
static LPCSTR psz_F4 = "F4";

// Write spectrumParm properties to stream
void CVowelFormants::WriteProperties(ObjectOStream& obs) const {
    obs.WriteBeginMarker(psz_Vowel, m_szVowel.c_str());

    // write out properties
    obs.WriteDouble(psz_F1, F1);
    obs.WriteDouble(psz_F2, F2);
    obs.WriteDouble(psz_F3, F3);
    obs.WriteDouble(psz_F4, F4);

    obs.WriteEndMarker(psz_Vowel);
}

// Read spectrumParm properties from *.psa file.
BOOL CVowelFormants::ReadProperties(ObjectIStream& obs) {

    char buffer[1024];
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_Vowel, buffer, _countof(buffer))) {
        return FALSE;
    }
    m_szVowel = buffer;

    Init(m_szVowel, -1, -1, -1);

    while (!obs.bAtEnd()) {
        if (obs.bReadDouble(psz_F1, F1));
        else if (obs.bReadDouble(psz_F2, F2));
        else if (obs.bReadDouble(psz_F3, F3));
        else if (obs.bReadDouble(psz_F4, F4));
        else if (obs.bEnd(psz_Vowel)) {
            break;
        }
    }
    return TRUE;
}

CVowelFormantSet::CVowelFormantSet(const string& szSetName, const CVowelFormantsVector vowels[3], BOOL bUser)
    : m_szSetName(szSetName) {
    m_bUser = bUser;
    m_vowels[0] = vowels[0];
    m_vowels[1] = vowels[1];
    m_vowels[2] = vowels[2];
}

static LPCSTR psz_VowelSet = "VowelSet";
static LPCSTR psz_Gender = "Gender";
static LPCSTR psz_User = "UserEditable";
static LPCSTR psz_DefaultSet = "DefaultSet";


// Write spectrumParm properties to stream
void CVowelFormantSet::WriteProperties(ObjectOStream& obs) const {

    obs.WriteBeginMarker(psz_VowelSet, m_szSetName.c_str());
    obs.WriteBool(psz_User, m_bUser);
    for (int i = 0; i < 3; i++) {
        char buffer[50];
        sprintf_s(buffer, sizeof(buffer), "%d", i);
        obs.WriteBeginMarker(psz_Gender, buffer);
        for (CVowelFormantsVector::const_iterator index = m_vowels[i].begin(); index != m_vowels[i].end(); index++) {
            index->WriteProperties(obs);
        }
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
    m_szSetName = buffer;

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
