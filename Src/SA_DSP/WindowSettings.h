#ifndef _WINDOWSETTINGS_H
#define _WINDOWSETTINGS_H

#include "dspTypes.h"
#include <vector>

using std::vector;

// Graph Parameter.cpp for implementation
class CWindowSettings {
public:
    CWindowSettings();

    enum Type { kRect=0, kHanning=1, kHann=1, kHamming=2, kBlackman=3, kBlackmanHarris=4, kGaussian=5 };
    enum { kBetweenCursors=0, kFragments=1, kTime=2, kBandwidth=3 };
    enum { kNone = 0 };

    bool operator==(const CWindowSettings & a) const;
    bool operator!=(const CWindowSettings & a) const;

    void Init();
    int32 getType() {
        return m_nType;
    }

    int32 m_nLengthMode;
    double m_dTime;
    double m_dBandwidth;
    int32  m_nFragments;
    bool  m_bEquivalentLength;
    bool  m_bCenter;
    int32 m_nReplication;

private:
    int32 m_nType;
};

#endif
