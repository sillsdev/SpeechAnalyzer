#ifndef ANALYTICLPCANALYSIS_H
#define ANALYTICLPCANALYSIS_H

#include "Process\TrackState.h"
#include "AnalyticLpcAnalysis.h"

class CAnalyticLpcAnalysis
{
public:
    typedef std::complex<double> CDBL;
    typedef std::vector<CDBL> VECTOR_CDBL;

    CAnalyticLpcAnalysis(const VECTOR_CDBL & signal, int nOrder, const CAnalyticLpcAnalysis * base = NULL);
    virtual ~CAnalyticLpcAnalysis();

    const VECTOR_CDBL & GetAutoCorrelation() const;
    const VECTOR_CDBL & GetPredictor() const;
    const VECTOR_CDBL & GetReflection() const;
    const VECTOR_CDBL & GetPoles() const;
    void GetResidual(const VECTOR_CDBL & signal, VECTOR_CDBL & residual) const;

private:
    int m_nOrder;
    mutable CDBL m_error;
    VECTOR_CDBL m_autocorrelation;
    mutable VECTOR_CDBL m_prediction;
    mutable VECTOR_CDBL m_reflection;
    mutable VECTOR_CDBL m_poles;

private:
    void BuildAutoCorrelation(const VECTOR_CDBL & signal, const CAnalyticLpcAnalysis * base = NULL);
    void BuildPredictorReflectionCoefficients(const CAnalyticLpcAnalysis * base = NULL);
};

#endif
