#pragma once
class CAppImpl : public App {
public:
    VowelFormantSets & GetVowelSets();
    const VowelFormantSet & GetDefaultVowelSet();
    const std::vector<CVowelFormants> & GetVowelVector(int nGender);
};

