#pragma once
#ifndef _VOWELFORMANTSETS_H
#define _VOWELFORMANTSETS_H

#include "VowelFormantSet.h"

class CVowelFormantSets : private std::vector<CVowelFormantSet> {
public:
    CVowelFormantSets(App * pApp);
    CVowelFormantSets(const string& szFilename);

    BOOL Load(const string& szFilename);
    BOOL Save(const string& szFilename) const;
    BOOL Load();
    BOOL Save() const;
    string SetFilename(const string& szFilename);
    int SetDefaultSet(int nSet);  // Error checked
    int GetDefaultSet() const;
    static CVowelFormantSet None();
    static CVowelFormantSet HillenbrandEtAl95();
    static CVowelFormantSet Ladefoged93();
    static CVowelFormantSet PetersonBarney52();
    static CVowelFormantSet Whitley();
    static CVowelFormantSet DanielJones();
    static CVowelFormantSet SynthesisLadefoged();

    const CVowelFormantSet& operator[](size_type _P) const;
    CVowelFormantSet& operator[](size_type _P);
    size_type size() const;

private:
    App* pApp;
    string m_szFilename;
    int m_nDefaultSet;
};

#endif
