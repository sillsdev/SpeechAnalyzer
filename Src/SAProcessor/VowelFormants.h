#pragma once

class CVowelFormants {

public:
    CVowelFormants(const string& szVowel, double inF1, double inF2, double inF3, double inF4 = UNDEFINED_DATA);
    string m_szVowel;
    double F1;
    double F2;
    double F3;
    double F4;

    void WriteProperties(ObjectOStream& obs) const;
    BOOL ReadProperties(ObjectIStream& obs);
    void Init(const string& szVowel, double inF1, double inF2, double inF3, double inF4 = UNDEFINED_DATA);
};

typedef std::vector<CVowelFormants> CVowelFormantsVector;

class CVowelFormantSet {
public:
    CVowelFormantSet(const string& szSetName = "", BOOL bUser = TRUE) : m_szSetName(szSetName) {
        m_bUser = bUser;
    };
    CVowelFormantSet(const string& szSetName, const CVowelFormantsVector vowels[3], BOOL bUser = TRUE);

    void WriteProperties(ObjectOStream& obs) const;
    BOOL ReadProperties(ObjectIStream& obs);

    const string& GetName() const {
        return m_szSetName;
    };
    void SetName(const string& szName) {
        m_szSetName = szName;
    };
    const CVowelFormantsVector& GetVowelFormants(int gender) const {
        return m_vowels[gender];
    };
    void SetVowelFormants(int gender, const CVowelFormantsVector& cVector) {
        m_vowels[gender] = cVector;
    };

    BOOL IsUser() const {
        return m_bUser;
    }

    enum gender { male = 0, female = 1, child = 2 };

private:
    BOOL m_bUser;
    string m_szSetName;
    CVowelFormantsVector m_vowels[3];
};
