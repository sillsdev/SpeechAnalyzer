// FormantTracker.cpp: implementation of the CFormantTracker class.
//
// Author: Steve MacLean
// copyright 2003 SIL
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "sa_process.h"
#include "FormantTracker.h"

#include "TrackState.h"
#include "AnalyticLpcAnalysis.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CAnalyticLpcAnalysis::CAnalyticLpcAnalysis(const VECTOR_CDBL & signal, int nOrder, const CAnalyticLpcAnalysis * base) : m_nOrder(nOrder) {
    BuildAutoCorrelation(signal, base);
    BuildPredictorReflectionCoefficients(base);
}

void CAnalyticLpcAnalysis::BuildAutoCorrelation(const VECTOR_CDBL & signal, const CAnalyticLpcAnalysis * base) {

    if (base) {
        m_autocorrelation = base->m_autocorrelation;
    } else {
        m_autocorrelation.clear();
    }

    if (m_autocorrelation.size() < unsigned(m_nOrder + 1)) {
        for (int i = m_autocorrelation.size(); i <= m_nOrder; i++) {
            CDBL value = 0;
            for (int j=signal.size() - 1; j >= i; j--) {
                value += signal[j]*std::conj(signal[j-i]);
            }
            m_autocorrelation.push_back(value);
        }
    } else {
        m_autocorrelation.resize(m_nOrder + 1);
    }
}

void CAnalyticLpcAnalysis::BuildPredictorReflectionCoefficients(const CAnalyticLpcAnalysis * base) {

    CDBL & e = m_error;
    VECTOR_CDBL workingPrediction[2];

    UNUSED_ALWAYS(base);

    e = m_autocorrelation[0];

    workingPrediction[0].resize(m_nOrder+1);
    workingPrediction[1].resize(m_nOrder+1);

    workingPrediction[0][0] = -1.;
    workingPrediction[1][0] = -1.;

    m_reflection.resize(m_nOrder);

    for (int i = 1; i<=m_nOrder; i++) {
        CDBL ki = m_autocorrelation[i];
        VECTOR_CDBL & lastPrediction = workingPrediction[(i-1)&1];
        VECTOR_CDBL & Prediction = workingPrediction[(i)&1];

        for (int j = 1; j < i; j++) {
            ki -= lastPrediction[j]*m_autocorrelation[i - j];
        }

        ki /= e;

        m_reflection[i-1] = -ki;  // reflection coefficient is negative of ki (PARCOR)
        Prediction[i] = ki;

        for (int j = 1; j < i; j++) {
            Prediction[j] = lastPrediction[j-1] - ki*lastPrediction[i-j-1];
        }

        e *= (CDBL(1) - ki*ki);
    }

    m_prediction.swap(workingPrediction[(m_nOrder)&1]);
}

CAnalyticLpcAnalysis::~CAnalyticLpcAnalysis() {
}

const VECTOR_CDBL & CAnalyticLpcAnalysis::GetPredictor() const {
    return m_prediction;
}

