// Hilbert.cpp: implementation of the CProcessHilbert class.
//
//////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "Hilbert.h"
#include "sa_w_adj.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const double CProcessHilbert::Pole1000x96dB[] = {
    9.9935401489275E-001,
    9.9795415607235E-001,
    9.9621092946349E-001,
    9.9383204108577E-001,
    9.9042122918253E-001,
    9.8541559170058E-001,
    9.7800017740675E-001,
    9.6699192390373E-001,
    9.5068735052105E-001,
    9.2667756716555E-001,
    8.9165794724631E-001,
    8.4130973059560E-001,
    7.7041566881683E-001,
    6.7347367595098E-001,
    5.4609671365400E-001,
    3.8722723362882E-001,
    2.0147243962615E-001,
    0 // Real Pole
};

CProcessHilbert::CProcessHilbert(Context & context, CProcess * pSourceProcess, BOOL bWBenchProcess) : CProcessIIRFilter( context, Plain) {
    const double * poles = Pole1000x96dB;
    double fTauSq=0;
    double rTauSq=0;
    int poleLast = 0;

    while (poles[poleLast] != 0.) {
        poleLast++;
    }

    for (int i=poleLast & 0x1; i < poleLast; i+=2) {
        double flp;
        m_zForwardTransform *= AllPass(poles[i]);
        flp = log(poles[i]);
        fTauSq += 2/(flp*flp);
    }
    m_zForwardTransform *= DelayHalf(); // Introduce a single clock delay

    for (int i=(poleLast+1)&0x1; i < poleLast; i+=2) {
        double rlp;
        m_zReverseTransform *= AllPass(poles[i]);
        rlp = log(poles[i]);
        rTauSq += 2/(rlp*rlp);
    }

    double fTau = sqrt(fTauSq);
    double rTau = sqrt(rTauSq);
    UNUSED_ALWAYS(rTau);

    SetFilterFilterSilenceSamples(int(fTau*11+1.5)+poleLast);
    SetSourceProcess(pSourceProcess, bWBenchProcess);
}

CZTransform CProcessHilbert::AllPass(double pole) {
    double numerator[3];
    double denominator[3];
    double beta = pole*pole;

    numerator[0] = beta;
    numerator[1] = 0;
    numerator[2] = -1;
    denominator[0] = 1.;
    denominator[1] = 0;
    denominator[2] = -beta;

    CZTransform result(2, numerator, denominator);
    return result;
}

CZTransform CProcessHilbert::DelayHalf() {
    double numerator[2];

    numerator[0] = 0;
    numerator[1] = 0.5;

    return CZTransform(1, numerator, NULL);
}

void CProcessHilbert::Dump(LPCSTR ofilename) {
	/*
    return;
    FILE * ofile = NULL;
    errno_t err = fopen_s(&ofile, ofilename, "w");
    fprintf(ofile, "hilbert data\n");
    fprintf(ofile, "m_nFilterFilterSilence=%d\n",m_nFilterFilterSilence);
    fprintf(ofile, "m_bReverse=%d\n",m_bReverse);
    fprintf(ofile, "m_bFilterFilter=%d\n",m_bFilterFilter);
    fprintf(ofile, "m_bSrcWBenchProcess=%d\n",m_bSrcWBenchProcess);
    fprintf(ofile, "m_bDstWBenchProcess=%d\n",m_bDstWBenchProcess);
    fprintf(ofile, "m_pSourceProcess=%lp\n",m_pSourceProcess);

    int count=0;
    {
        string ifilename = Utf8(GetProcessFileName());
        FILE * ifile = NULL;
        errno_t err = fopen_s(&ifile, ifilename.c_str(), "rb");
        unsigned short buffer = 0;
        while (true) {
            int read = fread(&buffer, 1, sizeof(unsigned short), ifile);
            if (read<sizeof(unsigned short)) {
                break;
            }
            fprintf(ofile, "%d ",buffer);
            count++;
        }
        if (!feof(ifile)) {
            fprintf(ofile, "premature termination");
        }
        fflush(ifile);
        fclose(ifile);
    }
    fprintf(ofile, "\n");
    fprintf(ofile, "%d values read\n",count);
    fflush(ofile);
    fclose(ofile);

    string csvfilename = ofilename;
    csvfilename.append(".csv");
    err = fopen_s(&ofile, csvfilename.c_str(), "w");
    count=0;
    {
        string ifilename = Utf8(GetProcessFileName());
        FILE * ifile = NULL;
        errno_t err = fopen_s(&ifile, ifilename.c_str(), "rb");
        unsigned short buffer = 0;
        while (true) {
            int read = fread(&buffer, 1, sizeof(unsigned short), ifile);
            if (read<sizeof(unsigned short)) {
                break;
            }
            fprintf(ofile, "%d,\n",buffer);
            count++;
        }
        if (!feof(ifile)) {
            fprintf(ofile, "premature termination");
        }
        fflush(ifile);
        fclose(ifile);
    }
    fflush(ofile);
    fclose(ofile);
	*/
}
