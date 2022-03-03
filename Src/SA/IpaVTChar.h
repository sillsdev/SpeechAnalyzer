#pragma once
#ifndef _IPAVTCHAR_H
#define _IPAVTCHAR_H

class CIpaVTChar {
public:
    typedef std::vector<double> VocalTract;
    struct CStimulus {
        CStimulus() {
            Pitch = AV = AF = AH = VHX = 0;
        }
        double Pitch;
        double AV;
        double AH;
        double AF;
        double VHX;
    };

    CIpaVTChar() {
        m_duration = m_dVTGain = m_dFrameEnergy = 0;
    }
    CIpaVTChar(const CString& ipa, double duration, double FEnergy, double VTGain, const CStimulus& stimulus, VocalTract& areas, VocalTract& reflection, VocalTract& pred) {
        m_ipa = ipa;
        m_duration = duration;
        m_dFrameEnergy = FEnergy;
        m_stimulus = stimulus;
        m_dVTGain = VTGain;
        m_areas = areas;
        m_reflection = reflection;
        m_pred = pred;
    }

    CString m_ipa;
    double m_duration;
    double m_dFrameEnergy;

    CStimulus m_stimulus;

    double m_dVTGain;

    VocalTract m_areas;
    VocalTract m_reflection;
    VocalTract m_pred;
};

#endif
