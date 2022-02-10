#include "pch.h"
#include "VowelFormants.h"

CVowelFormantSets& GetVowelSets() {
    string szPath(AfxGetApp()->GetProfileString(_T(""), _T("DataLocation")));
    if (szPath.Right(1) != _T("\\")) {
        szPath += "\\";
    }

    szPath = szPath + "vowelsUtf8.psa";

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
    obs.WriteBeginMarker(psz_Vowel, m_szVowel.utf8().c_str());

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

static LPCSTR psz_Version = "Version";

void CVowelSetVersion::WriteProperties(ObjectOStream& obs) const {
    obs.WriteBeginMarker(psz_Version, m_szVersion.utf8().c_str());
    obs.WriteEndMarker(psz_Version);
}

BOOL CVowelSetVersion::ReadProperties(ObjectIStream& obs) {

    char buffer[1024];
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_Version, buffer, _countof(buffer))) {
        return FALSE;
    }
    m_szVersion = buffer;

    while (!obs.bAtEnd()) {
        if (obs.bEnd(psz_Version)) {
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
        string szGenderID;
        szGenderID.Format(_T("%d"), i);
        obs.WriteBeginMarker(psz_Gender, szGenderID.c_str());
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

BOOL CVowelFormantSets::Load(const string& szFilename) {
    m_nDefaultSet = 0;

    reserve(17); // reserve room for 16 sets to minimize copying
    push_back(None());
    push_back(DanielJones());
    push_back(Whitley());
    push_back(HillenbrandEtAl95());
    push_back(Ladefoged93());
    push_back(PetersonBarney52());
    push_back(SynthesisLadefoged());

    const size_type nPredefined = size();

    if (szFilename.length()==0) {
        return TRUE;
    }

    try {
        ObjectIStream obs(szFilename);
        CVowelSetVersion version;
        if (!version.ReadProperties(obs)) {
            version.SetVersion("2.9");
        }

        while (!obs.bAtEnd()) {
            CVowelFormantSet newSet;
            if (newSet.ReadProperties(obs)) {
                if (newSet.IsUser()) { // Do not load predefined sets, saved as templates
                    push_back(newSet);
                }
            } else if (obs.bReadInteger(psz_DefaultSet, m_nDefaultSet)) {
                if (m_nDefaultSet < 0 || m_nDefaultSet >= (int)size()) {
                    m_nDefaultSet = 0;
                }
            }
        }

        // reset the default set to zero from pre-3.0 versions
        if (version.GetVersion() == _T("2.9")) {
            m_nDefaultSet = 0;
            Save();
        }
    } catch (...) {
        Save();
    }

    while (size() - nPredefined < 10) {
        string szCustom;
        szCustom.Format(_T("Custom %d"), size() - nPredefined + 1);
        CVowelFormantSet cCustom(szCustom);
        push_back(cCustom);
    }

    return TRUE;
}

int CVowelFormantSets::Save(const string& szFilename) const {
    if (szFilename.length()==0) {
        return FALSE;
    }

    try {
        ObjectOStream obs(szFilename);
        CVowelSetVersion version;
        version.WriteProperties(obs);

        for (const_iterator index = begin(); index != end(); index++) {
            index->WriteProperties(obs);
        }
        obs.WriteInteger(psz_DefaultSet, m_nDefaultSet);
    } catch (...) {
        return FALSE;
    }

    return TRUE;
}

int CVowelFormantSets::SetDefaultSet(int nSet) {
    int nOldSet = m_nDefaultSet;

    if (nSet != m_nDefaultSet) {
        if (nSet >= 0 && nSet < (int)size()) {
            m_nDefaultSet = nSet;
        }

        Save();
    }

    return nOldSet;
}

#define _AC(a,b) b

static inline string GetVowel(LPCSTR pszVowel) {
    string result;
    result = pszVowel;
    return result;
}

// None - now the default set
CVowelFormantSet CVowelFormantSets::None() {
    const int nVowels = 1;
    LPCSTR szVowels[nVowels] = {
        _AC("",""),
    };


    const short Men[][1] = {
        0
    };
    const short Women[][1] = {
        0
    };
    const short Children[][1] = {
        0
    };

    CVowelFormantsVector vowels[3];

    // pre-reserve space to speed operation
    vowels[CVowelFormantSet::male].reserve(nVowels);
    vowels[CVowelFormantSet::female].reserve(nVowels);
    vowels[CVowelFormantSet::child].reserve(nVowels);

    for (int nIndex = 0; nIndex < nVowels; nIndex++) {
        string szVowel = GetVowel(szVowels[nIndex]);

        vowels[CVowelFormantSet::male].push_back(CVowelFormants(szVowel,
            Men[nIndex][0],
            Men[nIndex][1],
            Men[nIndex][2],
            Men[nIndex][3]));
        vowels[CVowelFormantSet::female].push_back(CVowelFormants(szVowel,
            Women[nIndex][0],
            Women[nIndex][1],
            Women[nIndex][2],
            UNDEFINED_DATA));
        vowels[CVowelFormantSet::child].push_back(CVowelFormants(szVowel,
            Children[nIndex][0],
            Children[nIndex][1],
            Children[nIndex][2],
            UNDEFINED_DATA));
        // no female or child data from this data set
    }

    return CVowelFormantSet("(None)", vowels, FALSE);
}

// Hillenbrand, Getty, Clark, Wheeler (1995)
CVowelFormantSet CVowelFormantSets::HillenbrandEtAl95() {
    const int nVowels = 12;
    LPCSTR szVowels[nVowels] = {
        _AC("i","i"),
        _AC("I","ɪ"),
        _AC("e","e"),
        _AC("E","ɛ"),
        _AC("Q","æ"),
        _AC("A","ɑ"),
        _AC("�","ɔ"),
        _AC("o", "o "),
        _AC("U", "ʊ "),
        _AC("u", "u "),
        _AC("�","ʌ"),
        _AC("��","ɜ˞"),
    };

    const short Men[][4] = {
        342, 2322, 3000, 3657,
        427, 2034, 2684, 3618,
        476, 2089, 2691, 3649,
        580, 1799, 2605, 3677,
        588, 1952, 2601, 3624,
        768, 1333, 2522, 3687,
        652,  997, 2538, 3486,
        497,  910, 2459, 3384,
        469, 1122, 2434, 3400,
        378,  997, 2343, 3357,
        623, 1200, 2550, 3557,
        474, 1379, 1710, 3334
    };

    // Here are F1, F2, and F3 for Women. Based on Hillenbrand et al 1995
    const short Women[][4] = {
        437, 2761, 3372, 4352,
        483, 2365, 3053, 4334,
        536, 2530, 3047, 4319,
        731, 2058, 2979, 4294,
        669, 2349, 2972, 4290,
        936, 1551, 2815, 4299,
        781, 1136, 2824, 3923,
        555, 1035, 2828, 3927,
        519, 1225, 2827, 4052,
        459, 1105, 2735, 4115,
        753, 1426, 2933, 4092,
        523, 1588, 1929, 3914
    };

    // Here are F1, F2, and F3 for Children. Based on Hillenbrand et al 1995
    const short Children[][4] = {
        452, 3081, 3702, 4572,
        511, 2552, 3403, 4575,
        564, 2656, 3323, 4422,
        749, 2267, 3310, 4671,
        717, 2501, 3289, 4409,
        1002, 1688, 2950, 4307,
        803, 1210, 2982, 3919,
        597, 1137, 2987, 4167,
        568, 1490, 3072, 4328,
        494, 1345, 2988, 4276,
        749, 1546, 3145, 4320,
        586, 1719, 2143, 3788
    };


    CVowelFormantsVector vowels[3];

    // pre-reserve space to speed operation
    vowels[CVowelFormantSet::male].reserve(nVowels);
    vowels[CVowelFormantSet::female].reserve(nVowels);
    vowels[CVowelFormantSet::child].reserve(nVowels);

    for (int nIndex = 0; nIndex < nVowels; nIndex++) {
        string szVowel = GetVowel(szVowels[nIndex]);

        vowels[CVowelFormantSet::male].push_back(CVowelFormants(szVowel,
            Men[nIndex][0],
            Men[nIndex][1],
            Men[nIndex][2],
            Men[nIndex][3]));
        vowels[CVowelFormantSet::female].push_back(CVowelFormants(szVowel,
            Women[nIndex][0],
            Women[nIndex][1],
            Women[nIndex][2],
            Women[nIndex][3]));
        vowels[CVowelFormantSet::child].push_back(CVowelFormants(szVowel,
            Children[nIndex][0],
            Children[nIndex][1],
            Children[nIndex][2],
            Children[nIndex][3]));
    }

    return CVowelFormantSet("Am. English - Hillenbrand et. al (1995)", vowels, FALSE);
}

// Peterson and Barney(1952)
CVowelFormantSet CVowelFormantSets::PetersonBarney52() {
    const int nVowels = 10;
    LPCSTR szVowels[nVowels] = {
        _AC("i","i"),
        _AC("I","ɪ"),
        _AC("E","ɛ"),
        _AC("Q","æ"),
        _AC("A","ɑ"),
        _AC("�", "ɔ"),
        _AC("U", "ʊ"),
        _AC("u", "u"),
        _AC("�","ʌ"),
        _AC("��","ɜ˞")
    };

    const short Men[][3] = {
        270, 2300, 3000,
        400, 2000, 2550,
        530, 1850, 2500,
        660, 1700, 2400,
        730, 1100, 2450,
        570,  850, 2400,
        440, 1000, 2250,
        300,  850, 2250,
        640, 1200, 2400,
        490, 1350, 1700
    };

    const short Women[][3] = {
        300, 2800, 3300,
        430, 2500, 3100,
        600, 2350, 3000,
        860, 2050, 2850,
        850, 1200, 2800,
        590,  900, 2700,
        470, 1150, 2700,
        370,  950, 2650,
        760, 1400, 2800,
        500, 1650, 1950
    };

    const short Children[][3] = {
        370, 3200, 3700,
        530, 2750, 3600,
        700, 2600, 3550,
        1000, 2300, 3300,
        1030, 1350, 3200,
        680, 1050, 3200,
        560, 1400, 3300,
        430, 1150, 3250,
        850, 1600, 3350,
        560, 1650, 2150
    };

    CVowelFormantsVector vowels[3];

    // pre-reserve space to speed operation
    vowels[CVowelFormantSet::male].reserve(nVowels);
    vowels[CVowelFormantSet::female].reserve(nVowels);

    for (int nIndex = 0; nIndex < nVowels; nIndex++) {
        string szVowel = GetVowel(szVowels[nIndex]);

        vowels[CVowelFormantSet::male].push_back(CVowelFormants(szVowel,
            Men[nIndex][0],
            Men[nIndex][1],
            Men[nIndex][2],
            UNDEFINED_DATA));
        vowels[CVowelFormantSet::female].push_back(CVowelFormants(szVowel,
            Women[nIndex][0],
            Women[nIndex][1],
            Women[nIndex][2],
            UNDEFINED_DATA));
        vowels[CVowelFormantSet::child].push_back(CVowelFormants(szVowel,
            Children[nIndex][0],
            Children[nIndex][1],
            Children[nIndex][2],
            UNDEFINED_DATA));
    }

    return CVowelFormantSet("Am. English - Peterson & Barney (1952)", vowels, FALSE);
}

// Ladefoged(1993)
CVowelFormantSet CVowelFormantSets::Ladefoged93() {
    const int nVowels = 8;
    LPCSTR szVowels[nVowels] = {
        _AC("i","i"),
        _AC("I","ɪ"),
        _AC("E","ɛ"),
        _AC("Q","æ"),
        _AC("A","ɑ"),
        _AC("�","ɔ"),
        _AC("U","ʊ"),
        _AC("u","u"),
    };

    const short Men[][3] = {
        280, 2250, 2890,
        400, 1920, 2560,
        550, 1770, 2490,
        690, 1660, 2490,
        710, 1100, 2540,
        590, 880,  2540,
        450, 1030, 2360,
        310, 870,  2250
    };

    CVowelFormantsVector vowels[3];

    // pre-reserve space to speed operation
    vowels[CVowelFormantSet::male].reserve(nVowels);

    for (int nIndex = 0; nIndex < nVowels; nIndex++) {
        string szVowel = GetVowel(szVowels[nIndex]);

        vowels[CVowelFormantSet::male].push_back(CVowelFormants(szVowel,
            Men[nIndex][0],
            Men[nIndex][1],
            Men[nIndex][2],
            UNDEFINED_DATA));
        // no female or child data from this data set
    }

    return CVowelFormantSet("Am. English - Ladefoged (1993)", vowels, FALSE);
}

CVowelFormantSet CVowelFormantSets::DanielJones() {
    const int nVowels = 8;
    LPCSTR szVowels[nVowels] = {
        _AC("i","i"),
        _AC("e","e"),
        _AC("E","ɛ"),
        _AC("a","a"),
        _AC("A","ɑ"),
        _AC("�","ɔ"),
        _AC("o","o"),
        _AC("u","u"),
    };


    const short Men[][4] = {
        311, 2536, 3627, 3954,
        373, 2165, 2469, 3584,
        553, 1880, 2270, 3470,
        780, 1609, 2345, 3763,
        681, 1063, 2727, 3572,
        542, 920, 2127, 3027,
        371, 710, 2280, 2955,
        303, 627, 2380, 3607,
    };

    CVowelFormantsVector vowels[3];

    // pre-reserve space to speed operation
    vowels[CVowelFormantSet::male].reserve(nVowels);

    for (int nIndex = 0; nIndex < nVowels; nIndex++) {
        string szVowel = GetVowel(szVowels[nIndex]);

        vowels[CVowelFormantSet::male].push_back(CVowelFormants(szVowel,
            Men[nIndex][0],
            Men[nIndex][1],
            Men[nIndex][2],
            Men[nIndex][3]));
        // no female or child data from this data set
    }

    return CVowelFormantSet("IPA - Daniel Jones (1956)", vowels, FALSE);
}

CVowelFormantSet CVowelFormantSets::Whitley() {
    const int nVowels = 8;
    LPCSTR szVowels[nVowels] = {
        _AC("i","i"),
        _AC("e","e"),
        _AC("E","ɛ"),
        _AC("a","a"),
        _AC("A","ɑ"),
        _AC("�","ɔ"),
        _AC("o","o"),
        _AC("u","u"),
    };


    const short Men[][4] = {
        352, 2496, 3298, 4052,
        420, 2320, 2667, 4064,
        522, 1875, 2639, 3627,
        750, 1409, 2305, 3173,
        691, 977, 2837, 3787,
        485, 778, 2650, 3263,
        388, 728, 2577, 3077,
        345, 627, 2375, 3426,
    };

    CVowelFormantsVector vowels[3];

    // pre-reserve space to speed operation
    vowels[CVowelFormantSet::male].reserve(nVowels);

    for (int nIndex = 0; nIndex < nVowels; nIndex++) {
        string szVowel = GetVowel(szVowels[nIndex]);

        vowels[CVowelFormantSet::male].push_back(CVowelFormants(szVowel,
            Men[nIndex][0],
            Men[nIndex][1],
            Men[nIndex][2],
            Men[nIndex][3]));
        // no female or child data from this data set
    }

    return CVowelFormantSet("IPA - Whitley (2001)", vowels, FALSE);
}

CVowelFormantSet CVowelFormantSets::SynthesisLadefoged() {
    const int nVowels = 8;
    LPCSTR szVowels[nVowels] = {
        _AC("i","i"),
        _AC("e","e"),
        _AC("E","ɛ"),
        _AC("a","a"),
        _AC("A","ɑ"),
        _AC("�","ɔ"),
        _AC("o","o"),
        _AC("u","u"),
    };

    const short Men[][4] = {
        404, 2310, 2808, 3337,
        484, 1967, 2748, 3278,
        605, 1673, 2764, 3236,
        709, 1276, 2668, 3243,
        570, 864, 2773, 3291,
        525, 815, 2613, 3331,
        440, 674, 2673, 3311,
        375, 620, 2673, 3291,
    };

    CVowelFormantsVector vowels[3];

    // pre-reserve space to speed operation
    vowels[CVowelFormantSet::male].reserve(nVowels);

    for (int nIndex = 0; nIndex < nVowels; nIndex++) {
        string szVowel = GetVowel(szVowels[nIndex]);

        vowels[CVowelFormantSet::male].push_back(CVowelFormants(szVowel,
            Men[nIndex][0],
            Men[nIndex][1],
            Men[nIndex][2],
            Men[nIndex][3]));
        // no female or child data from this data set
    }

    return CVowelFormantSet("Synthesis - Ladefoged (2001)", vowels, FALSE);
}


