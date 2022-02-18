#pragma once
class CAppImpl : public App {
public:
    CVowelFormantSets & GetVowelSets();
    const CVowelFormantSet & GetDefaultVowelSet();
    const CVowelFormantsVector & GetVowelVector(int nGender);
};

