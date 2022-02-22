#pragma once

class CObjectOStream;
class CObjectIStream;

class CVowelFormants {

public:
    CVowelFormants(const string& szVowel, double inF1, double inF2, double inF3, double inF4 = UNDEFINED_DATA);
    CVowelFormants(const wstring& szVowel, double inF1, double inF2, double inF3, double inF4 = UNDEFINED_DATA);

    void Init(const wstring& szVowel, double inF1, double inF2, double inF3, double inF4 = UNDEFINED_DATA);
    void WriteProperties(CObjectOStream& obs) const;
    BOOL ReadProperties(CObjectIStream& obs);

    wstring m_szVowel;
    double F1;
    double F2;
    double F3;
    double F4;

};
