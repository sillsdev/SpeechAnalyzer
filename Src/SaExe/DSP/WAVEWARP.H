#ifndef WAVEWARP_H
#define WAVEWARP_H

#include "dspTypes.h"
#include "Error.h"
#include "Fragment.h"

class ISaDoc;


#define OUTSIDE_WAVE_BUFFER         1
#define PLAY_BUFFER_FULL            2

class CWaveWarp
{
public:
    static char * Copyright(void);
    static float Version(void);
    static dspError_t CreateObject(CWaveWarp ** ppWaveWarp, ISaDoc * pDoc, ULONG dwWaveStart, USHORT wSpeed,
                                   FRAG_PARMS * pstFragStart);
    dspError_t SetWaveBuffer(void * pWaveBfr);
    dspError_t SetPlayBuffer(void * pPlayBfr, ULONG dwPlayBfrLength);
    dspError_t FillPlayBuffer(ULONG dwWaveBlock, ULONG dwWaveBlockLength,
                              FRAG_PARMS * pstCallFragment,
                              ULONG * pdwPlayLength);
    dspError_t FillPlayBuffer(ULONG dwWaveBlock, ULONG dwWaveBlockLength,
                              FRAG_PARMS * pstCallFragment,
                              ULONG * pdwPlayLength, USHORT wNewSpeed);
    dspError_t FillPlayBuffer(FRAG_PARMS * pstCallData,
                              ULONG dwWaveBlockLength,
                              ULONG * pdwPlayLength, USHORT wSpeed);
    ~CWaveWarp(void);

private:
    CWaveWarp(ISaDoc * pDoc, ULONG dwWaveStart, USHORT wSpeed, ULONG dwFragStart);
    ISaDoc      *      m_pDoc;
    int                m_nSegmentIndex;
    void       *       m_pWaveBfr;
    ULONG              m_dwWaveBlockStart;
    ULONG              m_dwWarpStart;
    ULONG              m_dwWarpIndex;
    void       *       m_pPlayBfr;
    ULONG              m_dwPlayBfrLength;
    ULONG              m_dwPlayLength;
    FRAG_PARMS    *    m_pstFragBfr;
    ULONG              m_dwFragBfrLength;
    ULONG              m_dwFragBfrIndex;
    ULONG              m_dwFragBlock;
    double                m_dFragStartTime;
    double                m_dFragEndTime;
    double                m_dSmpPeriod;
    int8              m_sbSmpDataFmt;
    double                m_dSmpTime;
    double                m_dJitterFactor;
    USHORT             m_wSpeed;
    /***************************** DEBUG ONLY *************************************/
#ifdef DUMP
    FILE       *       m_hPlay;
#endif
    /******************************************************************************/

};

#endif

