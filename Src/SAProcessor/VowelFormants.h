#pragma once

class CVowelFormants {

public:
    CVowelFormants(const string& szVowel, double inF1, double inF2, double inF3, double inF4 = UNDEFINED_DATA);

    void Init(const string& szVowel, double inF1, double inF2, double inF3, double inF4 = UNDEFINED_DATA);
    void WriteProperties(ObjectOStream& obs) const;
    BOOL ReadProperties(ObjectIStream& obs);

    wstring m_szVowel;
    double F1;
    double F2;
    double F3;
    double F4;

};
