// AcousticTube.cpp: implementation of the CAcousticTube class.
//
//////////////////////////////////////////////////////////////////////

#include "stddsp.h"
//#include "sa.h"
#include "AcousticTube.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAcousticTube::CAcousticTube(const std::vector<double> & dDiameters, const std::vector<double> & dReflections) {
    m_nOrder = -1;
    SetTransform(dDiameters, dReflections);
}

CAcousticTube::~CAcousticTube() {
}

void CAcousticTube::SetTransform(const std::vector<double> & dDiameters, const std::vector<double> & dReflection) {
    ASSERT(dDiameters.size() == dReflection.size() + 1);

    int32 nOrder = dReflection.size();

    if (m_nOrder != nOrder) {
        m_nOrder = nOrder;

        // clear the state
        m_State.assign(m_nOrder + 1,0);
    }

    m_Diameters = dDiameters;
    m_Reflection = dReflection;

    // calculate reflection coeffiecients
    m_RCoefficientsFromDiameters.resize(m_nOrder);
    for (int32 i=0; i < m_nOrder; i++) {
        m_RCoefficientsFromDiameters[i] = (m_Diameters[i+1] - m_Diameters[i])/(m_Diameters[i+1] + m_Diameters[i]);
    }
}

double CAcousticTube::Tick(double dInput, bool bSourceReflect, bool bLattice) {
    std::vector<double> & RCoefficients = bSourceReflect ? m_Reflection : m_RCoefficientsFromDiameters;
    // These are two synthesis structures should be numerically identical
    // Their behavior may be different when interpolating the vocal tract.
    if (bLattice) {
        // All pole lattice
        double dForward = dInput;
        double ki;

        for (int32 i = m_nOrder - 1; i >= 0; i--) {
            ki = -RCoefficients[i]; // Parcor = -ri

            dForward += ki * m_State[i];
            m_State[i+1] = -ki * dForward + m_State[i];
        }

        m_State[0] = dForward;

        return dForward;
    } else {
        // signal flow from lossless tube model
        double dForward = dInput;

        dForward += m_State[0];

        for (int32 i = 0; i < m_nOrder; i++) {
            m_State[i] = m_State[i+1] * (1 + RCoefficients[i]) * (1 - RCoefficients[i]);
            m_State[i] -= dForward * RCoefficients[i];

            dForward += RCoefficients[i] * m_State[i+1];
        }

        return dForward;
    }
}


