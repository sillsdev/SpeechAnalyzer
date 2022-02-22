#include "stddsp.h"
#define DSPWINS_CPP
#include "MathX.h"
#include "windowsettings.h"

void CWindowSettings::Init() {
    m_nType = kGaussian;
    m_nLengthMode = kBetweenCursors;
    m_dTime = 20;
    m_dBandwidth = 300;
    m_nFragments = 3;
    m_bEquivalentLength = true;
    m_bCenter = true;
    m_nReplication = kNone;
}

bool CWindowSettings::operator==(const CWindowSettings & a) const {
    bool sameLength = (m_nLengthMode == kBetweenCursors) ||
                      (m_nLengthMode == kTime) && (m_dTime == a.m_dTime) ||
                      (m_nLengthMode == kBandwidth) && (m_dBandwidth == a.m_dBandwidth) ||
                      (m_nLengthMode == kFragments) && (m_nFragments == a. m_nFragments);
    bool result = (m_nType == a.m_nType) &&
                  (m_nLengthMode == a.m_nLengthMode) &&
                  (sameLength) &&
                  (m_bEquivalentLength == a. m_bEquivalentLength) &&
                  (m_bCenter == a. m_bCenter) &&
                  (m_nReplication == a.m_nReplication);
    return result;
}

CWindowSettings::CWindowSettings() {
    Init();
}

bool CWindowSettings::operator!=(const CWindowSettings & a) const {
    return !operator==(a);
}
