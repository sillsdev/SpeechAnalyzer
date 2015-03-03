#include "stdafx.h"
#include "WaveformGeneratorSettings.h"
#include "math.h"
#include "mainfrm.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "objectostream.h"
#include "objectistream.h"
#include "riff.h"
#include "WaveformGenerator.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CWaveformGeneratorSettings::CWaveformGeneratorSettings() {

    m_dAmplitude[0] = 100;
    m_dFrequency[0] = 220;
    m_dPhase[0] = 0;
    m_dAmplitude[1] = 100;
    m_dFrequency[1] = 440;
    m_dPhase[1] = 0;
    m_dAmplitude[2] = 100;
    m_dFrequency[2] = 660;
    m_dPhase[2] = 0;
    m_dAmplitude[3] = 100;
    m_dFrequency[3] = 220;
    m_dPhase[3] = 0;
    m_dAmplitude[4] = 100;
    m_dFrequency[4] = 220;
    m_dPhase[4] = 0;
    m_dAmplitude[5] = 100;
    m_dFrequency[5] = 220;
    m_dPhase[5] = 0;
    m_dAmplitude[6] = 100;
    m_dFrequency[6] = 220;
    m_dPhase[6] = 0;
    m_bSinusoid1 = TRUE;
    m_bSinusoid2 = FALSE;
    m_bSinusoid3 = FALSE;
    m_bComb = FALSE;
    m_bSquareWave = FALSE;
    m_bTriangle = FALSE;
    m_bSawtooth = FALSE;
    m_nHandleDiscontinuities = 0;
    m_nHarmonics = 10;

    m_fFileLength = 2;
    pcm.wBitsPerSample = 16;
    pcm.wf.nChannels = 1;
    pcm.wf.nBlockAlign = 2;
    pcm.wf.nSamplesPerSec = 22050;  // Already set
    pcm.wf.wFormatTag = 1;  // PCM

    pcm.wf.nAvgBytesPerSec = pcm.wf.nSamplesPerSec*pcm.wf.nBlockAlign;
    pcm.wf.nBlockAlign = (unsigned short)(pcm.wf.nChannels*(pcm.wBitsPerSample/8));
}

/***************************************************************************/
// CDlgWaveformGenerator::process::Process Generate wav file
/***************************************************************************/
void CWaveformGeneratorSettings::SynthesizeSamples(HPSTR pTargetData, DWORD dwDataPos, DWORD dwBufferSize) {

    WORD wSmpSize = WORD(pcm.wf.nBlockAlign / pcm.wf.nChannels);

    DWORD dwBlockEnd = dwDataPos + dwBufferSize;

    int nData;
    BYTE bData;
    DWORD nSample = dwDataPos/wSmpSize;
    const double pi = 3.14159265358979323846264338327950288419716939937511;
    const double radiansPerCycle = 2*pi;
    double samplesPerCycle[7];
    double radiansPerSample[7];
    double phaseRadians[7];
    DWORD nHarmonics[7];

    for (int i = 0; i < 7; i++) {
        samplesPerCycle[i] = pcm.wf.nSamplesPerSec/m_dFrequency[i];
        radiansPerSample[i] = radiansPerCycle/samplesPerCycle[i];
        phaseRadians[i] = m_dPhase[i]/360*radiansPerCycle;
        nHarmonics[i] = (m_nHandleDiscontinuities == 2) ? m_nHarmonics : (DWORD)(pcm.wf.nSamplesPerSec/m_dFrequency[i]/2.);
    }

    while (dwDataPos < dwBlockEnd) {
        double intermediate = 0;

        if (m_bSinusoid1) {
            intermediate += m_dAmplitude[0]*sin(nSample*radiansPerSample[0]+phaseRadians[0]);
        }
        if (m_bSinusoid2) {
            intermediate += m_dAmplitude[1]*sin(nSample*radiansPerSample[1]+phaseRadians[1]);
        }
        if (m_bSinusoid3) {
            intermediate += m_dAmplitude[2]*sin(nSample*radiansPerSample[2]+phaseRadians[2]);
        }

        if (m_bComb) {
            double dCycles = nSample/samplesPerCycle[3] + m_dPhase[3]/360;

            if (fmod(dCycles, 1.0)*samplesPerCycle[3] < 1.0) {
                intermediate += m_dAmplitude[3];
            }
        }

        if (m_nHandleDiscontinuities == 0) { // ideal
            if (m_bSquareWave) { // Positive for 180 degrees then negative for 180 degrees
                double dCycles = nSample/samplesPerCycle[4] + m_dPhase[4]/360;
                intermediate += m_dAmplitude[4]*(fmod(dCycles , 1.0) >= 0.5 ? -1. : 1.);
            }
            if (m_bTriangle) { // rising from -90 to 90 then falling from 90 to 270
                double dCycles = nSample/samplesPerCycle[5] + m_dPhase[5]/360;
                intermediate += m_dAmplitude[5]*(1 - 4.* fabs(fmod((dCycles + 0.25) , 1.0)-0.5));
            }
            if (m_bSawtooth) { // falling linearly from -180 to 180 then jump discontinuity and cycle repeats
                double dCycles = nSample/samplesPerCycle[6] + m_dPhase[6]/360;
                intermediate += m_dAmplitude[6]*(1 - 2.*fmod((dCycles + 0.5) , 1.0));
            }
        } else {
            if (m_bSquareWave) { // Positive for 180 degrees then negative for 180 degrees
                double square = 0;

                for (DWORD nHarmonic = 1; nHarmonic <= nHarmonics[4]; nHarmonic +=2) {
                    square += sin((nSample*radiansPerSample[4]+phaseRadians[4])*nHarmonic)/ nHarmonic;
                }

                intermediate += m_dAmplitude[4]*4/pi*square;
            }
            if (m_bTriangle) { // rising from -90 to 90 then falling from 90 to 270
                double triangle = 0;
                double sign = 1;

                for (DWORD nHarmonic = 1; nHarmonic <= nHarmonics[5]; nHarmonic +=2) {
                    triangle += sign*sin((nSample*radiansPerSample[5]+phaseRadians[5])*nHarmonic)/ (nHarmonic*nHarmonic);
                    sign = -sign;
                }

                intermediate += m_dAmplitude[5]*8/pi/pi*triangle;
            }
            if (m_bSawtooth) { // falling linearly from -180 to 180 then jump discontinuity and cycle repeats
                double saw = 0;
                double sign = 1;

                for (DWORD nHarmonic = 1; nHarmonic <= nHarmonics[6]; nHarmonic +=1) {
                    saw += sign*sin((nSample*radiansPerSample[6]+phaseRadians[6])*nHarmonic)/ nHarmonic;
                    sign = -sign;
                }

                intermediate += m_dAmplitude[6]*2/pi*saw;
            }
        }

        intermediate /= 100.0;

        intermediate = intermediate > 1.0 ? 1.0 : (intermediate < -1.0 ? -1.0 : intermediate); // clip result to prevent overflow

        // save data
        if (wSmpSize == 1) { // 8 bit data
            intermediate *= 127.0;
            bData = BYTE(128 + (int)((intermediate > 0)? (intermediate + 0.5):(intermediate - 0.5)));
            *pTargetData++ = bData;
        } else {              // 16 bit data
            intermediate *= 32767.0;
            nData = (int)((intermediate > 0)? (intermediate + 0.5):(intermediate - 0.5));
            *pTargetData++ = (BYTE)nData;
            *pTargetData++ = (BYTE)(nData >> 8);
        }
        dwDataPos+=wSmpSize;
        nSample++;
    }
}

static LPCSTR psz_generator = "generator";
static LPCSTR psz_amplitude = "amplitude";
static LPCSTR psz_frequency = "frequency";
static LPCSTR psz_phase = "phase";
static LPCSTR psz_sinusoid1 = "sinusoid1";
static LPCSTR psz_sinusoid2 = "sinusoid2";
static LPCSTR psz_sinusoid3 = "sinusoid3";
static LPCSTR psz_comb = "comb";
static LPCSTR psz_square = "square";
static LPCSTR psz_triangle = "triangle";
static LPCSTR psz_saw = "saw";
static LPCSTR psz_enable = "enable";
static LPCSTR psz_oscillators[] = {psz_sinusoid1,psz_sinusoid2,psz_sinusoid3,psz_comb,psz_square,psz_triangle,psz_saw};
static LPCSTR psz_discontinuities = "discontinuities";
static LPCSTR psz_harmonics = "harmonics";
static LPCSTR psz_samplingrate = "samplingrate";
static LPCSTR psz_filelength = "filelength";
static LPCSTR psz_bits = "bits";

/***************************************************************************/
// CDlgWaveformGeneratorSettings::WriteProperties Write echo properties
/***************************************************************************/
void CWaveformGeneratorSettings::WriteProperties(CObjectOStream & obs) {

    BOOL enables[] = {m_bSinusoid1,m_bSinusoid2,m_bSinusoid3,m_bComb,m_bSquareWave,m_bTriangle,m_bSawtooth};
    obs.WriteBeginMarker(psz_generator);
    for (int i = 0; i<7; i++) {
        obs.WriteBeginMarker(psz_oscillators[i]);
        obs.WriteBool(psz_enable,enables[i]);
        obs.WriteDouble(psz_amplitude, m_dAmplitude[i]);
        obs.WriteDouble(psz_frequency, m_dFrequency[i]);
        obs.WriteDouble(psz_phase, m_dPhase[i]);
        obs.WriteEndMarker(psz_oscillators[i]);
    }
    obs.WriteInteger(psz_discontinuities, m_nHandleDiscontinuities);
    obs.WriteInteger(psz_harmonics, m_nHarmonics);
    obs.WriteInteger(psz_samplingrate, pcm.wf.nSamplesPerSec);
    obs.WriteDouble(psz_filelength, m_fFileLength);
    obs.WriteInteger(psz_bits, pcm.wBitsPerSample);
    obs.WriteEndMarker(psz_generator);
}

/***************************************************************************/
// CDlgWaveformGenerator::settings::ReadProperties Read echo properties
/***************************************************************************/
BOOL CWaveformGeneratorSettings::ReadProperties(CObjectIStream & obs) {

    BOOL * enables[] = { &m_bSinusoid1,&m_bSinusoid2,&m_bSinusoid3,&m_bComb,&m_bSquareWave,&m_bTriangle,&m_bSawtooth};

    int nValue = 0;

    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_generator)) {
        return FALSE;
    }
    while (!obs.bAtEnd()) {
        BOOL found = TRUE;
        if (obs.bReadDouble(psz_amplitude, m_dAmplitude[0]));
        else if (obs.bReadDouble(psz_frequency, m_dFrequency[0]));
        else if (obs.bReadInteger(psz_discontinuities, m_nHandleDiscontinuities));
        else if (obs.bReadInteger(psz_harmonics, m_nHarmonics));
        else if (obs.bReadInteger(psz_samplingrate, nValue)) {
            pcm.wf.nSamplesPerSec = nValue;
        } else if (obs.bReadDouble(psz_filelength, m_fFileLength));
        else if (obs.bReadInteger(psz_bits, nValue)) {
            pcm.wBitsPerSample = (unsigned short)(nValue);
        } else {
            found = FALSE;

            for (int i = 0; i < 7 ; i++) {
                if (obs.bReadBeginMarker(psz_oscillators[i])) {
                    found = TRUE;
                    while (!obs.bAtEnd()) {
                        if (obs.bReadBool(psz_enable, *(enables[i])));
                        else if (obs.bReadDouble(psz_amplitude, m_dAmplitude[i]));
                        else if (obs.bReadDouble(psz_frequency, m_dFrequency[i]));
                        else if (obs.bReadDouble(psz_phase, m_dPhase[i]));
                        else if (obs.bEnd(psz_oscillators[i])) {
                            break;
                        }
                    }
                    break;
                }
            }
        }

        if (!found && obs.bEnd(psz_generator)) {
            break;
        }
    }
    // assume properties changed
    return TRUE;
}

BOOL CWaveformGeneratorSettings::Synthesize(LPCTSTR szFileName) {

    pcm.wf.nBlockAlign = (unsigned short)(pcm.wf.nChannels*(pcm.wBitsPerSample/8));
    pcm.wf.nAvgBytesPerSec = pcm.wf.nSamplesPerSec*pcm.wf.nBlockAlign;

    CProcessWaveformGenerator * pProcess = new CProcessWaveformGenerator();

    short int nResult = LOWORD(pProcess->Process(*this));
    if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED) {
        delete pProcess;
        return FALSE;
    }

    CRiff::NewWav(szFileName, pcm, pProcess->GetProcessFileName());

    delete pProcess;

    return TRUE;
}

