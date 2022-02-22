#pragma once
#include "VowelFormants.h"

using std::vector;

class CVowelFormantsVector {
public:
    void WriteProperties(CObjectOStream& obs) const {
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
    size_t size() const {
        return data.size();
    }
    CVowelFormants get(size_t val) const {
        return data[val];
    }

private:
    vector<CVowelFormants> data;
};

