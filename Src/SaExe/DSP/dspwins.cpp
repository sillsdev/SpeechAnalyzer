#include "stddsp.h"
#define DSPWINS_CPP
#include "MathX.h"
#include "DspWins.h"

void CWindowSettings::Init()
{
    m_nType = kGaussian;
    m_nLengthMode = kBetweenCursors;
    m_dTime = 20;
    m_dBandwidth = 300;
    m_nFragments = 3;
    m_bEquivalentLength = true;
    m_bCenter = true;
    m_nReplication = kNone;
}

bool CWindowSettings::operator==(const CWindowSettings & a) const
{
    bool sameLength =
        (m_nLengthMode == kBetweenCursors) ||
        (m_nLengthMode == kTime) && (m_dTime == a.m_dTime) ||
        (m_nLengthMode == kBandwidth) && (m_dBandwidth == a.m_dBandwidth) ||
        (m_nLengthMode == kFragments) && (m_nFragments == a. m_nFragments);
    bool result =
        m_nType == a.m_nType &&
        m_nLengthMode == a.m_nLengthMode &&
        sameLength &&
        m_bEquivalentLength == a. m_bEquivalentLength &&
        m_bCenter == a. m_bCenter &&
        m_nReplication == a.m_nReplication;

    return result;
}

CWindowSettings::CWindowSettings()
{
    Init();
}

bool CWindowSettings::operator!=(const CWindowSettings & a) const
{
    return !operator==(a);
}

DspWin::CWindowParms RectParms()
{
    double bandwidthProduct = 2.416228*0.576000; // 1.39
    static const double coefficients[] =
    {
        1.
    };

    return DspWin::CWindowParms(bandwidthProduct, sizeof(coefficients)/sizeof(double), coefficients);
}

DspWin::CWindowParms HannParms()   // Also known as Hanning
{
    double bandwidthProduct = 1.414790*1.599393; // 2.26
    static const double coefficients[] =
    {
        0.5,
        0.5
    };

    return DspWin::CWindowParms(bandwidthProduct, sizeof(coefficients)/sizeof(double), coefficients);
}

DspWin::CWindowParms HammingParms()
{
    double bandwidthProduct = 1.358215*1.506903; // 2.05
    static const double coefficients[] =
    {
        0.54,
        0.46
    };

    return DspWin::CWindowParms(bandwidthProduct, sizeof(coefficients)/sizeof(double), coefficients);
}

DspWin::CWindowParms BlackmanParms()
{
    double bandwidthProduct = 1.309175*1.969723; // 2.58
    static const double coefficients[] =
    {
        0.42,
        0.50,
        0.08
    };

    return DspWin::CWindowParms(bandwidthProduct, sizeof(coefficients)/sizeof(double), coefficients);
}

DspWin::CWindowParms BlackmanHarrisParms()
{
    double bandwidthProduct = 1.277152*2.334273; // 2.98
    static const double coefficients[] =
    {
        0.35875,
        0.48829,
        0.14128,
        0.01168
    };

    return DspWin::CWindowParms(bandwidthProduct, sizeof(coefficients)/sizeof(double), coefficients);
}

DspWin::CWindowParms GaussianParms()
{
    double bandwidthProduct = 1.222838*3.354341; // 4.10
    static const double coefficients[] =
    {
        1.434719036600488E-1,
        2.341893831304000E-1,
        1.273062422805212E-1,
        4.610257144132060E-2,
        1.111993920779460E-2,
        1.786810773395950E-3,
        1.910492117385130E-4,
        1.036901498651440E-5
    };

    return DspWin::CWindowParms(bandwidthProduct, sizeof(coefficients)/sizeof(double), coefficients);
}

DspWin::CWindowParms DspWin::WindowParms(int32 windowType)
{
    DspWin::CWindowParms(*parms[])(void) =
    {
        RectParms,
        HannParms,
        HammingParms,
        BlackmanParms,
        BlackmanHarrisParms,
        GaussianParms,
    };

    return parms[windowType]();
}

void DspWin::Build()
{
    if (m_bWindowBuilt)
    {
        return;
    }

    CWindowParms window = WindowParms(m_windowType);
    double RadAngle = 2*PI/(m_nLength);
    double scale = HannParms().m_bandwidthProduct/window.m_bandwidthProduct/window.m_coefficients[0];

    m_cDWindow.resize(m_nLength);
    m_cFWindow.resize(m_nLength);

    for (int32 i=0; i<m_nLength; i++)
    {
        double w = 0;
        double sign = 1;
        for (int32 term=0; term<window.m_terms; term++)
        {
            w+= sign*window.m_coefficients[term] * cos(term*(i+0.5)*RadAngle);
            sign *= -1;
        }
        w *= scale;

        m_cDWindow[i] = w;
        m_cFWindow[i] = (float)w;
    }
    m_bWindowBuilt = true;
}

int32 DspWin::CalcLength( double bandwidth, uint32 smpRate, int32 windowType)
{
    CWindowParms window = WindowParms(windowType);
    return (int32)( smpRate/bandwidth * window.m_bandwidthProduct + 0.5);
}

double DspWin::Bandwidth() const
{
    CWindowParms window = WindowParms(m_windowType);
    return double(m_smpRate)/m_nLength*window.m_bandwidthProduct;
}

int32 DspWin::CalcEquivalentLength(int32 length, int32 dstWindowType, int32 srcWindowType)
{
    CWindowParms dstWindow = WindowParms(dstWindowType);
    CWindowParms srcWindow = WindowParms(srcWindowType);
    return (int32)(length*dstWindow.m_bandwidthProduct/srcWindow.m_bandwidthProduct);
}

dspError_t HanningWin::FromLength(HanningWin ** Hanning, int32 Length, uint32 SmpRate)
{
    if (!Hanning)
    {
        return(Code(INVALID_PARM_PTR));
    }
    if (!SmpRate)
    {
        return(Code(UNSUPP_SMP_RATE));
    }
    if (Length <= 0)
    {
        return(Code(INVALID_WIN_LEN));
    }
    float * Data = (float *)malloc(Length * sizeof(float));  //Length can be even or odd
    if (!Data)
    {
        return(Code(OUT_OF_MEMORY));
    }
    *Hanning = new HanningWin(Data,Length,SmpRate);
    if (!*Hanning)
    {
        return(Code(OUT_OF_MEMORY));
    }
    return(DONE);
}

int32 HanningWin::CalcLength( float Bandwidth, uint32 SmpRate)
{
    if (!Bandwidth || (Bandwidth>=(float)SmpRate/2.F))
    {
        return(Code(INVALID_BANDWIDTH));
    }
    if (!SmpRate)
    {
        return(Code(UNSUPP_SMP_RATE));
    }
    // Calculate length in samples from bandwidth.  Bandwidth is assumed to be measured from 0 Hz
    // to frequency where window spectrum is 50% of max (at 0 Hz).
    int32 Length = (int32)floor((float)SmpRate/Bandwidth);
    Length |= 1;  //Force length odd
    return(Length);
}


dspError_t HanningWin::FromBandwidth( HanningWin ** Hanning, float Bandwidth, uint32 SmpRate)
{
    if (!Hanning)
    {
        return(Code(INVALID_PARM_PTR));
    }
    int32 Length = HanningWin::CalcLength( Bandwidth, SmpRate);
    if (Length < 0)
    {
        return(Length);
    }
    float * Data = (float *)malloc(Length * sizeof(float));
    if (!Data)
    {
        return(Code(OUT_OF_MEMORY));
    }

    *Hanning = new HanningWin(Data,Length,SmpRate);
    if (!*Hanning)
    {
        return(Code(OUT_OF_MEMORY));
    }
    return(DONE);
}

HanningWin::HanningWin(float * Data, int32 Length, uint32 SmpRate)
{
    m_Data = Data;
    m_Length = Length;
    m_Bandwidth = (float)SmpRate/(float)Length;
    double RadAngle = 2. * PI / (double)(Length - 1);

    for (int32 i = 0; i < Length; i++)
    {
        m_Data[i] = (float)(0.5 - 0.5 * cos(RadAngle * (double)i));
    }
}

int32 HanningWin::Length()
{
    return m_Length;
}
float HanningWin::Bandwidth()
{
    return m_Bandwidth;
}
float HanningWin::Data(uint32 i)
{
    return m_Data[i];
}

HanningWin::~HanningWin()
{
    free(m_Data);
}


dspError_t KaiserWin::Setup(KaiserWin ** Kaiser, float Bandwidth, float Gain,
                            float Atten, float SmpRate)
{
#define PERCENT_TRANSITION   .10F

    if (!Kaiser)
    {
        return(Code(INVALID_PARM_PTR));
    }
    if (SmpRate == 0.F)
    {
        return(Code(INVALID_SMP_RATE));
    }
    if (Bandwidth == 0.F || Bandwidth >= SmpRate/2.F)
    {
        return(Code(INVALID_BANDWIDTH));
    }
    if (Gain == 0.F)
    {
        return(Code(INVALID_GAIN));
    }
    if (Atten == 0.F)
    {
        return(Code(INVALID_ATTEN));
    }

    float Transition = Bandwidth*PERCENT_TRANSITION;

    //  Calculate Kaiser windowed filter width based on the required
    //  attenuation and transition regions for the upsampled signal.
    int32 Length = (int32)(floor((Atten - 7.95)/(14.36*Transition/SmpRate) + 1.));
    Length += !(Length % 2); // make width odd for integral group delay
    // !! account for 3 dB shift
    if (Length <= 0)
    {
        return(Code(INVALID_WIN_LEN));
    }

    int32 GroupDelay = Length/2;
    bool * CoeffSet = (bool *)malloc((GroupDelay+1) * sizeof(bool));
    if (!CoeffSet)
    {
        return(Code(OUT_OF_MEMORY));
    }
    float * Coeff = (float *)malloc((GroupDelay+1) * sizeof(*Coeff));
    if (!Coeff)
    {
        return(Code(OUT_OF_MEMORY));
    }


    *Kaiser = new KaiserWin(Coeff, CoeffSet, Bandwidth, Gain, Atten, Transition, GroupDelay, SmpRate);
    if (!*Kaiser)
    {
        return(Code(OUT_OF_MEMORY));
    }

    return(DONE);
}

KaiserWin::KaiserWin(float * Coeff, bool * CoeffSet, float Bandwidth, float Gain, float Atten,
                     float Transition, int32 GroupDelay, float SmpRate)
{
    m_GroupDelay = GroupDelay;
    m_Length = 2*GroupDelay+1;
    m_Gain = Gain;
    m_Bandwidth = Bandwidth;
    m_Atten = Atten;
    m_Transition = Transition;
    m_SmpRate = SmpRate;
    m_CoeffSet = CoeffSet;
    m_Coeff = Coeff;

    //  Compute the tradeoff between a sharp transition from passband to
    //  stopband and greater attenuation.
    if (Atten > 50.F)
    {
        m_TradeOff = .1102F*(Atten - 8.7F);
    }
    else if (Atten > 21.F)
    {
        m_TradeOff = .5842F*(float)pow((double)Atten - 21.,.4) + .07886F*(Atten - 21.F);
    }
    else
    {
        m_TradeOff = 0.F;
    }
    m_Atten = Atten;

    return;
}

KaiserWin::~KaiserWin()
{
    free(m_CoeffSet);
    free(m_Coeff);
}

float KaiserWin::CalcCoeff(int32 i)
{
    if (i > m_GroupDelay)
    {
        i = m_Length - 1 - i%m_Length;    //modulo to prevent overflow
    }
    if (!m_CoeffSet[i])
    {
        if (i != m_GroupDelay)
            m_Coeff[i] = (float)Round((double)m_Gain*
                                      (double)BessI0(m_TradeOff*(float)pow(1.-pow((double)(i-m_GroupDelay)/
                                              (double)m_GroupDelay,2.),.5))/(double)BessI0(m_TradeOff)*
                                      (double)sin(2.*PI*(double)(i-m_GroupDelay)*(double)m_Bandwidth/
                                                  (double)m_SmpRate)/(PI*(double)(i-m_GroupDelay)));
        else
        {
            m_Coeff[i] = (float)Round((double)m_Gain*(2.*(double)m_Bandwidth/(double)(m_SmpRate)));
        }
        m_CoeffSet[i] = true;
    }
    return(m_Coeff[i]);
}

void KaiserWin::Fill(void)
{
    for (int32 i = 0; i < m_Length; i++)
    {
        CalcCoeff(i);
    }

    return;
}

DspWin::DspWin( int32 nLength, uint32 smpRate, int32 windowType) :
    m_nLength(nLength), m_smpRate(smpRate), m_windowType(windowType), m_bWindowBuilt(false)
{
}

DspWin::DspWin( const DspWin & from) :
    m_nLength(from.m_nLength), m_smpRate(from.m_smpRate), m_windowType(from.m_windowType), m_bWindowBuilt(false)
{
}

DspWin & DspWin::operator=( const DspWin & from)
{
    m_bWindowBuilt = false;
    m_windowType = from.m_windowType;
    m_smpRate = from.m_smpRate;
    m_nLength = from.m_nLength;
    return *this;
}

DspWin DspWin::FromLength(int32 nLength, uint32 smpRate, int32 windowType)
{
    return DspWin( nLength, smpRate, windowType);
}

DspWin DspWin::FromBandwidth(double bandwidth, uint32 smpRate, int32 windowType)
{
    return DspWin( CalcLength( bandwidth, smpRate, windowType), smpRate, windowType);
}

const double * DspWin::WindowDouble()
{
    Build();
    return &m_cDWindow[0];
}

const float * DspWin::WindowFloat()
{
    Build();
    return &m_cFWindow[0];
}

int32 DspWin::Length() const
{
    return m_nLength;
}

int32 DspWin::Type() const
{
    return m_windowType;
}

float KaiserWin::Coeff(uint32 i)
{
    return m_Coeff[i];
}
