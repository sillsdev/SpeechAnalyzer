#pragma once
typedef std::vector<CVowelFormants> CVowelFormantsVector;

extern const CVowelFormantsVector& GetVowelVector(int nGender);
extern const CVowelFormantSet& GetDefaultVowelSet();
extern CVowelFormantSets& GetVowelSets();

