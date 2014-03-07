// Derived from Reson.c from Sensimetrics corporatation
//
// Author steve MacLean
// Copyright JAARS 2001
//
// Modification History
//   2/14/2001 SDM Copied source from Reson.c
//                 Created CResonator class
//                 Changed Function interface to use Float
//                 Added additional checks in InterPolePair() i.e. sqrt(-), 1/0

/*
*      SenSyn - Version 1.0
*
*      Copyright (c) 1991, 1992 by Sensimetrics Corporation
*      All Rights Reserved.
*
*/

/*
*  reson.c -
*    This code implements a second order resonator, which can be used
*    to realize a pole pair or zero pair.
*
*  coded by Andrew Wilson Howitt
*
*  Modification History:
*    26 Sep 1991    E.P.Carlson, added AdvanceAntiResonator and InterPolePair
*/

#include "stdafx.h"
#include <math.h>
#include "reson.h"
#include <limits>

class limit : public std::numeric_limits<Float>
{
public:
    static double & boundsTest(double & value);
    static double & bound(double & value);
    static double & boundAssert(double & value);
};

double & limit::bound(double & value)
{
    if (value == infinity())
    {
        value = max();
    }
    if (value == -infinity())
    {
        value = min();
    }
    if (value == signaling_NaN())
    {
        value = 0;
    }
    return value;
}

double & limit::boundsTest(double & value)
{
    if (value == infinity())
    {
        throw infinity();
    }
    if (value == -infinity())
    {
        throw -infinity();
    }
    if (value == signaling_NaN())
    {
        throw signaling_NaN();
    }
    return value;
}

CResonator::CResonator()
{
    Coef.A = 1;
    Coef.B = 0;
    Coef.C = 0;

    m_pAdjusted = NULL;
    m_nAdjustedBlend = 0;
    m_nAdjustedCycles = 1;

    ClearResonator();  // clears state
}

CResonator::CResonator(CResonator * original)
{
    Coef = original->Coef;
    State = original->State;

    m_pAdjusted = original->m_pAdjusted;
    m_nAdjustedBlend = original->m_nAdjustedBlend;
    m_nAdjustedCycles = original->m_nAdjustedCycles;
}

CResonator::~CResonator()
{
    delete m_pAdjusted;
}

void CResonator::ClearResonator()
{
    State.Z1 = 0.;
    State.Z2 = 0.;

    delete m_pAdjusted;
    m_pAdjusted = NULL;
}

Float CResonator::AdvanceResonator(Float input)
{
    register Float temp1,temp2;

    temp1 = Coef.C * State.Z2;
    State.Z2 = State.Z1;

    temp2 = Coef.B * State.Z1;
    temp1 += temp2;

    temp2 = Coef.A * input;
    State.Z1 = temp2 + temp1;

    Float result = State.Z1;

    if (m_pAdjusted)
    {
        result *= double(m_nAdjustedBlend)/m_nAdjustedCycles;
        result += (m_nAdjustedCycles - double(m_nAdjustedBlend))/m_nAdjustedCycles * m_pAdjusted->AdvanceResonator(input);
        m_nAdjustedBlend++;
        if (m_nAdjustedBlend == m_nAdjustedCycles)
        {
            delete m_pAdjusted;
            m_pAdjusted = 0;
        }
    }

    try
    {
        limit::boundsTest(result);
    }
    catch (...)
    {
        ClearResonator();
        return 0;
    }

    return result;
}

Float CResonator::AdvanceAntiResonator(Float input)
{
    register Float temp1,temp2;

    temp1 = Coef.C * State.Z2;
    State.Z2 = State.Z1;

    temp2 = Coef.B * State.Z1;
    temp1 += temp2;

    temp2 = Coef.A * input;
    State.Z1 = input;

    Float result = temp2 + temp1;

    if (m_pAdjusted)
    {
        result *= double(m_nAdjustedBlend)/m_nAdjustedCycles;
        result += (m_nAdjustedCycles - double(m_nAdjustedBlend))/m_nAdjustedCycles * m_pAdjusted->AdvanceAntiResonator(input);
        m_nAdjustedBlend++;
        if (m_nAdjustedBlend == m_nAdjustedCycles)
        {
            delete m_pAdjusted;
            m_pAdjusted = 0;
        }
    }

    try
    {
        limit::boundsTest(result);
    }
    catch (...)
    {
        ClearResonator();
        return 0;
    }

    return result;
}

void CResonator::SetPolePair(Float CF, Float BW, Float SF)
{
    register double magnitude,angle,PiT;

    PiT = PI / (double)SF;
    magnitude = exp(-PiT * (double)BW);
    angle = 2. * PiT * (double)CF;

    Coef.C = (Float)(-magnitude * magnitude);
    Coef.B = (Float)(magnitude * cos(angle) * 2.);
    Coef.A = 1. - Coef.B - Coef.C;
}

void CResonator::InterPolePair(Float CF, Float BW, Float SF)
{
    if (!m_pAdjusted || m_nAdjustedBlend)
    {
        CResonator * pRes = new CResonator(this);
        m_pAdjusted = pRes;
        m_nAdjustedBlend = 0;
        m_nAdjustedCycles = int(8*SF/(CF > 70 ? CF : 70));
    }

    SetPolePair(CF, BW, SF);
}

void CResonator::SetZeroPair(Float CF, Float BW, Float SF)
{
    SetPolePair(CF,BW,SF);
    Coef.A = 1. / Coef.A;
    Coef.B *= -Coef.A;
    Coef.C *= -Coef.A;
}

void CResonator::InterZeroPair(Float CF, Float BW, Float SF)
{
    if (!m_pAdjusted || m_nAdjustedBlend)
    {
        CResonator * pRes = new CResonator(this);
        m_pAdjusted = pRes;
        m_nAdjustedBlend = 0;
        m_nAdjustedCycles = int(8*SF/(1000));
    }

    SetZeroPair(CF, BW, SF);
}

void CResonator::InterAdjustGain(Float gain)
{
    Coef.A *= gain;
}

