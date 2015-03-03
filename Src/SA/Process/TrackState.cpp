// FormantTracker.cpp: implementation of the CFormantTracker class.
//
// Author: Steve MacLean
// copyright 2003 SIL
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_doc.h"
#include "Process.h"
#include "Process/FormantTracker.h"
#include "Process/TrackState.h"
#include "FileUtils.h"
#include "StringUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

void STrackState::DumpWindowed(LPCSTR ofilename) {
    return;
    FILE * ofile = NULL;
    fopen_s(&ofile, ofilename, "w");
    fprintf(ofile, "windowed\n");
    fprintf(ofile,"real,imag\n");
    for (int i=0; i<windowed.size(); i++) {
        fprintf(ofile, "%f,%f\n",windowed[i].real(), windowed[i].imag());
    }
    fflush(ofile);
    fclose(ofile);
}

void STrackState::DumpFiltered(LPCSTR ofilename) {
    return;
    FILE * ofile = NULL;
    fopen_s(&ofile, ofilename, "w");
    fprintf(ofile, "filtered\n");
    fprintf(ofile,"real,imag\n");
    for (int i=0; i<filtered.size(); i++) {
        fprintf(ofile, "%f,%f\n",filtered[i].real(), filtered[i].imag());
    }
    fflush(ofile);
    fclose(ofile);
}

void STrackState::DumpZeroFilterDBL(LPCSTR ofilename) {
    return;
    FILE * ofile = NULL;
    fopen_s(&ofile, ofilename, "w");
    fprintf(ofile,"zerofilter\n");
    fprintf(ofile,"double\n");
    for (int i=0; i<zeroFilterDBL.size(); i++) {
        fprintf(ofile, "%f\n",zeroFilterDBL[i]);
    }
    fflush(ofile);
    fclose(ofile);
}
