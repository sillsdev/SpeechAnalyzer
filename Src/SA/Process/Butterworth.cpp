// Butterworth.cpp: implementation of the CProcessButterworth class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Butterworth.h"
#include "isa_doc.h"
#include "sa_w_adj.h"
#include "StringUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const double pi = 3.14159265358979323846264338327950288419716939937511;

CProcessButterworth::CProcessButterworth(BOOL bWorkbenchOutput) : CProcessIIRFilter(bWorkbenchOutput) {
    m_bFilterFilter = false;
    m_bReverse = FALSE;
    m_nOrder = 0;
    m_ftFilterType = kftUndefined;
    m_dSampling = 0.;
    m_dFrequency = 0.;
    m_dBandwidth = 0.;
    m_dScale = 1;
}

CProcessButterworth::~CProcessButterworth() {
}

// ButterworthPole()
// The poles of a normalized butterworth filter are equally spaced
// around the unit circle in the complex plane.  All poles are in the
// left plane.  The poles are symmetric about the Real axis, and
// are not placed on the imaginary axis.  Function returns a pole.
// The poles are numbered counter clockwise (90-270 degrees)
inline static std::complex<double> ButterworthPole(int nOrder, int nPole) {
    double dSpacing = pi/ nOrder;
    double dAngle = (pi + dSpacing)/2. + dSpacing*nPole;
    return std::complex<double>(cos(dAngle), sin(dAngle));
}

void CProcessButterworth::CascadeLowPass(CZTransform & zTransform, int nOrder, double dFilterCutoffFreq, double dSamplingFreq, double & tau) {
    double numerator[3];
    double denominator[3];

    double dDigitalCutoffFreq = (dFilterCutoffFreq/dSamplingFreq)*2.*pi;
    double dAnalogCutoffFreq = 2.*tan(dDigitalCutoffFreq/2.);  // warp to analog frequency

    double tauSq = tau*tau;

    for (int i = 0; i < (nOrder+1)/2; i++) {
        std::complex<double> pole = ButterworthPole(nOrder, i);

        // Since the Butterworth pole is normalized to 1
        // it must be scaled to the desired analog filter cutoff
        pole *= dAnalogCutoffFreq;

        // Generate digital filter
        if ((i * 2 + 1) == nOrder) { // last one is single real pole
            // each first order analog filter takes the general form
            //
            //               -pole/(s - pole)
            //
            // Substituting the bilinear transformation
            //                          -1          -1
            //              s = 2 (1 - z  ) / (1 + z  )
            //
            // yields the following coefficients
            denominator[0] = 1.;
            denominator[1] = (2 + pole.real())/(pole.real()-2.);
            numerator[0] = pole.real()/(pole.real() - 2);
            numerator[1] = numerator[0];
            zTransform *= CZTransform(1, numerator, denominator);

            double lp = fabs(denominator[1]) > 1e-6 ? log(fabs(denominator[1])) : log(1e-6);
            tauSq += 1/(lp*lp) + 1;
            break;
        } else { // conjugate pole pairs
            // each second order analog filter takes the general form
            //
            //               -pole      -pole*
            //              --------   ---------  , where * denotes conjugate
            //             (s - pole) (s - pole*)
            //
            //                           2
            //                     |pole|
            //           = --------------------------
            //              2                        2
            //             s  - 2 s Re(pole) + |pole|
            //
            // Substituting the bilinear transformation
            //                          -1          -1
            //              s = 2 (1 - z  ) / (1 + z  )
            //
            // yields the following coefficients

            double PoleMagSquared = (pole.real()*pole.real() + pole.imag()*pole.imag());
            double NormFactor = 4. - 4.* pole.real() + PoleMagSquared;
            denominator[0] = 1.;
            denominator[1] = (2.*PoleMagSquared - 8.)/NormFactor;
            denominator[2] = (4. + 4.*pole.real() + PoleMagSquared) / NormFactor;
            numerator[0] = PoleMagSquared / NormFactor;
            numerator[1] = 2. * numerator[0];
            numerator[2] = numerator[0];
            zTransform *= CZTransform(2, numerator, denominator);

            double lp = fabs(denominator[2]) > 1e-6 ? log(fabs(denominator[2]))/2 : log(1e-6)/2;
            tauSq += 2/(lp*lp) + 2;
        }
    }
    tau = sqrt(tauSq);
}

// Cascades a highpass nOrder butterworth filter to current working filter
void CProcessButterworth::CascadeHighPass(CZTransform & zTransform, int nOrder, double dFilterCutoffFreq, double dSamplingFreq, double & tau) {
    double numerator[3];
    double denominator[3];

    double dDigitalCutoffFreq = (dFilterCutoffFreq/dSamplingFreq)*2.*pi;
    double dAnalogCutoffFreq = 2.*tan(dDigitalCutoffFreq/2.);  // warp to analog frequency

    double tauSq = tau*tau;

    for (int i = 0; i < (nOrder+1)/2; i++) {
        std::complex<double> pole = ButterworthPole(nOrder, i);

        // Since the Butterworth pole is normalized to 1
        // it must be scaled to the desired analog filter cutoff
        pole /= dAnalogCutoffFreq;

        // Generate digital filter.
        // The low pass stages are converted to high pass stages using the tranformation
        //     s -> 1/s
        //
        // It suffices to invert the bilinear transform
        //                            -1          -1
        //              s = 0.5 (1 + z  ) / (1 - z  )
        //
        // and substitute into the low pass equations, yielding the following coefficients
        if ((i * 2 + 1) == nOrder) { // last one is single real pole
            // first order section
            denominator[0] = 1.;
            denominator[1] = (1. + 2.*pole.real()) / (1. - 2.*pole.real());
            numerator[0] = 2.*pole.real() / (2.*pole.real() - 1.);
            numerator[1] = -numerator[0];
            zTransform *= CZTransform(1, numerator, denominator);

            double lp = fabs(denominator[1]) > 1e-6 ? log(fabs(denominator[1])) : log(1e-6);
            tauSq += 1/(lp*lp) + 1;
            break;
        } else {
            // second order section, taking complex conjugate pole pairs
            double dPoleMagSquared = (pole.real()*pole.real() + pole.imag()*pole.imag());
            double dNormFactor = 1. - 4.* pole.real() + 4.* dPoleMagSquared;
            denominator[0] = 1.;
            denominator[1] = (2. - 8.*dPoleMagSquared) / dNormFactor;
            denominator[2] = (1. + 4.*pole.real() + 4.*dPoleMagSquared) / dNormFactor;
            numerator[0] = (4.*dPoleMagSquared) / dNormFactor;
            numerator[1] = -2.*numerator[0];
            numerator[2] = numerator[0];
            zTransform *= CZTransform(2, numerator, denominator);

            double lp = fabs(denominator[2]) > 1e-6 ? log(fabs(denominator[2]))/2 : log(1e-6)/2;
            tauSq += 2/(lp*lp) + 2;
        }
    }
    tau = sqrt(tauSq);
}

// Cascades a scale term to scale output
void CProcessButterworth::CascadeScale(CZTransform & zTransform, double dScale) {
    zTransform *= CZTransform(0, &dScale, NULL);
}


void CProcessButterworth::ClearFilter() {
    m_zForwardTransform = CZTransform(0, NULL, NULL);
    m_zReverseTransform = CZTransform(0, NULL, NULL);
}

double CProcessButterworth::FilterFilterNorm(int nOrder) const {
    if (m_bFilterFilter) {
        return exp(log(sqrt(2.) - 1.)/(2*nOrder));
    } else {
        return 1.;
    }
}

/***************************************************************************/
// CProcessButterworth::Process
/***************************************************************************/
/***************************************************************************/
// CProcessButterworth::Process Processing new raw data with a lowpass function
// The processed change data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CProcessButterworth::Process(void * pCaller, ISaDoc * pDoc, int nProgress, int nLevel) {
    if (IsCanceled()) {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }
    // check if nested workbench processes

    if (IsDataReady()) {
        return MAKELONG(--nLevel, nProgress);    // data is already ready
    }

    ConfigureProcess(pDoc->GetSamplesPerSec());
    return CProcessIIRFilter::Process(pCaller, pDoc, nProgress, nLevel);
}


void CProcessButterworth::HighPass(int nOrder, double dFrequency, double dScale) {
    if (m_nOrder != nOrder || m_dFrequency != dFrequency || m_dScale != dScale || m_ftFilterType != kftHighPass) {
        SetDataInvalid();
    }

    m_nOrder = nOrder;
    m_dFrequency = dFrequency;
    m_ftFilterType = kftHighPass;
    m_dScale = dScale;
}

void CProcessButterworth::LowPass(int nOrder, double dFrequency, double dScale) {
    if ((m_nOrder != nOrder) || (m_dFrequency != dFrequency) || (m_dScale != dScale) || (m_ftFilterType != kftLowPass)) {
        SetDataInvalid();
    }

    m_nOrder = nOrder;
    m_dFrequency = dFrequency;
    m_ftFilterType = kftLowPass;
    m_dScale = dScale;
}

void CProcessButterworth::BandPass(int nOrder, double dFrequency, double dBandwidth, double dScale) {
    if (m_nOrder != nOrder || m_dFrequency != dFrequency || m_dBandwidth != dBandwidth || m_dScale != dScale || m_ftFilterType != kftBandPass) {
        SetDataInvalid();
    }

    m_nOrder = nOrder;
    m_dFrequency = dFrequency;
    m_dBandwidth = dBandwidth;
    m_ftFilterType = kftBandPass;
    m_dScale = dScale;
}

void CProcessButterworth::ConfigureProcess(double dSampling) {
    double tau = 0;
    ClearFilter();
    if (m_bReverse) {
        double rTau = 0;
        switch (m_ftFilterType) {
        case kftHighPass:
            CascadeHighPass(m_zReverseTransform, m_nOrder, m_dFrequency*FilterFilterNorm(m_nOrder), dSampling, rTau);
            break;
        case kftLowPass:
            CascadeLowPass(m_zReverseTransform, m_nOrder, m_dFrequency/FilterFilterNorm(m_nOrder), dSampling, rTau);
            break;
        case kftBandPass:
            // Assume a low pass followed by a high pass will suffice
            //   We should probably adjust the filter for center frequency gain
            //   But this is good enough for now...
            CascadeLowPass(m_zReverseTransform, m_nOrder, (m_dFrequency + m_dBandwidth/2.)/FilterFilterNorm(m_nOrder), dSampling, rTau);
            CascadeHighPass(m_zReverseTransform, m_nOrder, (m_dFrequency - m_dBandwidth/2.)*FilterFilterNorm(m_nOrder), dSampling, rTau);
            break;
        default:
            // null filter... We could assert here
            TRACE(_T("Filter not configured\n"));
        }
    }
    switch (m_ftFilterType) {
    case kftHighPass:
        CascadeHighPass(m_zForwardTransform, m_nOrder, m_dFrequency*FilterFilterNorm(m_nOrder), dSampling, tau);
        CascadeScale(m_zForwardTransform, m_dScale);
        break;
    case kftLowPass:
        CascadeLowPass(m_zForwardTransform, m_nOrder, m_dFrequency/FilterFilterNorm(m_nOrder), dSampling, tau);
        CascadeScale(m_zForwardTransform, m_dScale);
        break;
    case kftBandPass:
        // Assume a low pass followed by a high pass will suffice
        //   We should probably adjust the filter for center frequency gain
        //   But this is good enough for now...
        CascadeLowPass(m_zForwardTransform, m_nOrder, (m_dFrequency + m_dBandwidth/2.)/FilterFilterNorm(m_nOrder), dSampling, tau);
        CascadeHighPass(m_zForwardTransform, m_nOrder, (m_dFrequency - m_dBandwidth/2.)*FilterFilterNorm(m_nOrder), dSampling, tau);
        CascadeScale(m_zForwardTransform, m_dScale);
        break;
    default:
        // null filter... We could assert here
        TRACE(_T("Filter not configured\n"));
    }
    SetFilterFilterSilenceSamples(int(tau*11 + 1));
}

void CProcessButterworth::SetFilterFilter(bool bSet) {
    m_bFilterFilter = bSet;
    SetReverse(bSet);
}

double CProcessButterworth::ForwardTick(double data) {
    return m_zForwardTransform.Tick(data);
}

void CProcessButterworth::SetReverse(BOOL bSet) {
    m_bReverse = bSet;
}

int CProcessButterworth::round2Int(double value) {
    return (value >= 0.0) ? int(value + 0.5) : int(value - 0.5);
}

