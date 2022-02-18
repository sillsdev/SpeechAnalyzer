#include "pch.h"
#include "VowelFormants.h"
#include "VowelSetVersion.h"

CVowelFormants::CVowelFormants(const string& szVowel, double inF1, double inF2, double inF3, double inF4) {
    Init(szVowel, inF1, inF2, inF3, inF4);
}

void CVowelFormants::Init(const string& szVowel, double inF1, double inF2, double inF3, double inF4) {
    m_szVowel = _to_wstring(szVowel);
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
    obs.WriteBeginMarker(psz_Vowel, _to_utf8(m_szVowel).c_str());

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

    Init(buffer, -1, -1, -1);

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
