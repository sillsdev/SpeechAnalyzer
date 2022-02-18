#pragma once
#include "VowelFormants.h"

using std::vector;

class CVowelFormantsVector {
public:
    void WriteProperties(ObjectOStream& obs) const {
        for (auto & element : data) {
            element.WriteProperties(obs);
        }
    }

    void clear() {
        data.clear();
    }
    void reserve(const size_t val) {
        data.reserve(val);
    }
    void push_back(CVowelFormants value) {
        data.push_back(value);
    }

private:
    vector<CVowelFormants> data;
};

