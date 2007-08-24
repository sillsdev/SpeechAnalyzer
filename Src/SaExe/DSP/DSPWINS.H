#ifndef DSPWINS_H

#include "dspTypes.h"

#define DSPWINS_H

using std::vector;

// Graph Parameter.cpp for implementation
class CWindowSettings
{
public:
  CWindowSettings() { Init(); }

  enum Type { kRect=0, kHanning=1, kHann=1, kHamming=2, kBlackman=3, kBlackmanHarris=4, kGaussian=5 };
  enum { kBetweenCursors=0, kFragments=1, kTime=2, kBandwidth=3 };
  enum { kNone = 0 };

  bool operator==(const CWindowSettings &a) const;
  bool operator!=(const CWindowSettings &a) const {return !operator==(a);}

  void Init();
  
  int32 m_nType;
	int32 m_nLengthMode;
	double m_dTime;
	double m_dBandwidth;
  int32  m_nFragments;
	bool  m_bEquivalentLength;
	bool  m_bCenter;
  int32 m_nReplication;
};

class DspWin
{
public:
  DspWin(const DspWin &from) :
      m_nLength(from.m_nLength), m_smpRate(from.m_smpRate), m_windowType(from.m_windowType), m_bWindowBuilt(false) {;}

  DspWin& operator=(const DspWin& from) 
  {
    m_bWindowBuilt = false; m_windowType = from.m_windowType; m_smpRate = from.m_smpRate; m_nLength = from.m_nLength; return *this;
  }

  static DspWin FromLength(int32 nLength, int32 smpRate, int32 windowType) { return DspWin(nLength, smpRate, windowType);}
  static DspWin FromBandwidth(double bandwidth, int32 smpRate, int32 windowType) { return DspWin(CalcLength(bandwidth, smpRate, windowType), smpRate, windowType);}

  const double* WindowDouble() { Build(); return &m_cDWindow[0]; }
  const float* WindowFloat() { Build(); return &m_cFWindow[0]; }
  
  static int32 CalcLength(double bandwidth, int32 SmpRate, int32 windowType);
  static int32 CalcEquivalentLength(int32 length, int32 dstWindowType, int32 srcWindowType = kRect);
  int32 Length() const {return m_nLength;}
  double Bandwidth() const;
  int32 Type() const {return m_windowType;}

public:
  enum { kRect=0, kHanning=1, kHann=1, kHamming=2, kBlackman=3, kBlackmanHarris=4, kGaussian=5 };

  class CWindowParms
  {
  public:
    CWindowParms(double bandwidthProduct, int32 terms, const double * coefficients) : m_bandwidthProduct(bandwidthProduct) , m_terms(terms) , m_coefficients(coefficients) {;}
    
    const double m_bandwidthProduct;
    const int32 m_terms;
    const double * const m_coefficients;
  private:
    // Not used
    const CWindowParms& operator=(const CWindowParms &) {;} 
  };
  
private:
  DspWin(int32 nLength, int32 smpRate, int32 windowType) : 
       m_nLength(nLength), m_smpRate(smpRate), m_windowType(windowType), m_bWindowBuilt(false) { ;}

  int32 m_windowType;
  int32 m_smpRate;
  int32 m_nLength;
  bool m_bWindowBuilt;
  vector<double> m_cDWindow;
  vector<float> m_cFWindow;

  void Build();
  static CWindowParms WindowParms(int32 windowType);
};

class HanningWin
{ 
public:
  static dspError_t FromLength(HanningWin** Hanning, int32 Length, uint16 SmpRate);
  static dspError_t FromBandwidth(HanningWin** Hanning, float Bandwidth, 
    uint16 SmpRate);
  static int32 CalcLength(float Bandwidth, uint16 SmpRate);                         
  int32 Length() {return m_Length;}                         
  float Bandwidth() {return m_Bandwidth;}                         
  float Data(uint32 i) {return m_Data[i];}
  ~HanningWin();                                   
private:  
  HanningWin(float *Data, int32 Length, uint16 SmpRate);
  float *m_Data;
  int32        m_Length;
  float       m_Bandwidth;
};

class KaiserWin{ 
public:
  static dspError_t Setup(KaiserWin** Kaiser, float Bandwidth, 
    float Gain, float Atten, float SmpRate);
  ~KaiserWin();                           
  float CalcCoeff(int32 i);                 
  float Coeff(uint32 i) {return m_Coeff[i];}
  void Fill(void);
private:
  KaiserWin(float *Coeff, bool *CoeffSet, float Bandwidth, float Gain, float Atten,
    float Transition, int32 GroupDelay, float SmpRate);    
  float m_Bandwidth;
  float m_Gain;
  float m_Atten;
  float m_Transition;       
  float m_TradeOff;
  int32  m_GroupDelay;
  int32  m_Length;
  float m_SmpRate;          
  float *m_Coeff;
  bool *m_CoeffSet;
};

#endif

