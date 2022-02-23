#ifndef _WINDOWSETTINGS_H
#define _WINDOWSETTINGS_H

#include "dspTypes.h"
#include <vector>

using std::vector;

enum WindowType { kRect = 0, kHanning = 1, kHann = 1, kHamming = 2, kBlackman = 3, kBlackmanHarris = 4, kGaussian = 5 };
enum LengthMode { kBetweenCursors = 0, kFragments = 1, kTime = 2, kBandwidth = 3 };
enum Replication { kNone = 0 };

struct SWindowSettings {
    LengthMode lengthMode;
    double time;
    double bandwidth;
    bool  equivalentLength;
    bool  center;
    int32 fragments;
    Replication replication;
    WindowType type;

    void Init() {
        type = kGaussian;
        lengthMode = kBetweenCursors;
        time = 20;
        bandwidth = 300;
        fragments = 3;
        equivalentLength = true;
        center = true;
        replication = kNone;
    };

    bool operator==(const SWindowSettings& right) const {
        bool sameLength = (lengthMode == kBetweenCursors) ||
            (lengthMode == kTime) && (time == right.time) ||
            (lengthMode == kBandwidth) && (bandwidth == right.bandwidth) ||
            (lengthMode == kFragments) && (fragments == right.fragments);

        bool result = (type == right.type) &&
            (lengthMode == right.lengthMode) &&
            (sameLength) &&
            (equivalentLength == right.equivalentLength) &&
            (center == right.center) &&
            (replication == right.replication);
        return result;
    }

    bool operator!=(const SWindowSettings& right) const {
        return !operator==(right);
    }
};

#endif
