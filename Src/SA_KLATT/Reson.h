// Derived from Reson.h from Sensimetrics corporatation
//
// Author steve MacLean
// Copyright JAARS 2001
//
// Modification History
//   2/14/2001 SDM Copied source from Reson.h
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

/*  reson.h - resonator declarations
*
*  coded by Eric P. Carlson, based upon reson.c by A.W. Howitt
*
*  Modification History:
*    22 Jul 1992    changed type Float back to float
*    26 Sep 1991    added AdvanceAntiResonator and InterPolePair
*    14 Nov 1991    changed type Float double
*/

class CResonator
{
public:
    CResonator();
    CResonator(CResonator * original);
    ~CResonator();

public:
    void ClearResonator();
    Float AdvanceResonator(Float input);
    Float AdvanceAntiResonator(Float input);
    void InterPolePair(Float CF, Float BW, Float SF);
    void InterZeroPair(Float CF, Float BW, Float SF);
    void InterAdjustGain(Float gain);
private:
    void SetPolePair(Float CF, Float BW, Float SF);
    void SetZeroPair(Float CF, Float BW, Float SF);
    void AdjustGain(Float gain)
    {
        Coef.A *= gain;
    }

    struct
    {
        Float A, B, C;
    } Coef;
    struct
    {
        Float Z1, Z2;
    } State;
    CResonator * m_pAdjusted;
    int m_nAdjustedCycles;
    int m_nAdjustedBlend;
} ;

