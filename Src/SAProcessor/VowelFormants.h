#pragma once
typedef std::vector<CVowelFormants> CVowelFormantsVector;

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


class CVowelSetVersion {
public:
    CVowelSetVersion(const string szVersion = "3.0") {
        m_szVersion = szVersion;
    };

    void WriteProperties(ObjectOStream& obs) const;
    BOOL ReadProperties(ObjectIStream& obs);

    string GetVersion() {
        return m_szVersion;
    };
    void SetVersion(const string& szVersion) {
        m_szVersion = szVersion;
    };

private:
    string m_szVersion;
};

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

class CVowelFormantSets : private std::vector<CVowelFormantSet> {
public:
    CVowelFormantSets() {
        Load();
    };
    CVowelFormantSets(const string& szFilename) : m_szFilename(szFilename) {
        Load(szFilename);
    }
    virtual ~CVowelFormantSets() {};

    BOOL Load(const string& szFilename);
    BOOL Save(const string& szFilename) const;
    BOOL Load() {
        return Load(m_szFilename);
    }
    BOOL Save() const {
        return Save(m_szFilename);
    }
    string SetFilename(const string& szFilename) {
        string szOld(m_szFilename);
        m_szFilename = szFilename;
        return szOld;
    }
    int SetDefaultSet(int nSet);  // Error checked
    int GetDefaultSet() const {
        return m_nDefaultSet;
    }
    static CVowelFormantSet None();
    static CVowelFormantSet HillenbrandEtAl95();
    static CVowelFormantSet Ladefoged93();
    static CVowelFormantSet PetersonBarney52();
    static CVowelFormantSet Whitley();
    static CVowelFormantSet DanielJones();
    static CVowelFormantSet SynthesisLadefoged();

    const CVowelFormantSet& operator[](size_type _P) const {
        return static_cast<const std::vector<CVowelFormantSet>*>(this)->operator[](_P);
    }
    CVowelFormantSet& operator[](size_type _P) {
        return static_cast<std::vector<CVowelFormantSet>*>(this)->operator[](_P);
    }
    size_type size() const {
        return static_cast<const std::vector<CVowelFormantSet>*>(this)->size();
    }

private:
    string m_szFilename;
    int m_nDefaultSet;
};

