#ifndef FORMANTTRACKER_H
#define FORMANTTRACKER_H

#include "..\DSP\ZTransform.h"

class CAnalyticLpcAnalysis {
public:
    typedef std::complex<double> CDBL;
    typedef std::vector<CDBL> VECTOR_CDBL;

    CAnalyticLpcAnalysis(const VECTOR_CDBL & signal, int nOrder, const CAnalyticLpcAnalysis * base = NULL);
    virtual ~CAnalyticLpcAnalysis() {
        ;
    }

    const VECTOR_CDBL & GetAutoCorrelation() const;
    const VECTOR_CDBL & GetPredictor() const {
        return m_prediction;
    }
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

typedef std::complex<double> CDBL;
typedef std::vector<CDBL> VECTOR_CDBL;
typedef std::deque<CDBL> DEQUE_CDBL;
typedef CZTransformGeneric<CDBL> CZTransformCDBL;
typedef double DBL;
typedef std::vector<DBL> VECTOR_DBL;

class CProcessFormantTracker : public CProcess {
public:
    CProcessFormantTracker(CProcess & Real, CProcess & Imag, CProcess & Pitch);
    virtual ~CProcessFormantTracker();

    virtual long Process(void * pCaller, ISaDoc *, int nProgress = 0, int nLevel = 1);

    FORMANT_FREQ * GetFormant(DWORD dwIndex);

    class CTrackState {
    public:
        virtual ~CTrackState() {
            ;
        }

        virtual DEQUE_CDBL & GetData() = 0;
        virtual VECTOR_DBL & GetWindow() = 0;

        // Previous track position
        virtual VECTOR_CDBL & GetTrackIn() = 0;

        // Result track position
        virtual VECTOR_CDBL & GetTrackOut() = 0;

        // Working intermediates to eliminate memory thrashing
        virtual VECTOR_CDBL & GetWindowed() = 0;
        virtual VECTOR_CDBL & GetFiltered() = 0;
        virtual VECTOR_CDBL & GetZeroFilterCDBL() = 0;
        virtual VECTOR_DBL & GetZeroFilterDBL() = 0;
    };

private:
    CProcess * m_pReal;
    CProcess * m_pImag;
    CProcess * m_pPitch;

    void AdvanceData(CTrackState & state, DWORD dwDataPos, int nSamples);
    void WriteTrack(CTrackState & state, double samplingRate, int pitch); // write a block into the temporary file
    void BuildTrack(CTrackState & state, double samplingRate, int pitch);
};

#endif
