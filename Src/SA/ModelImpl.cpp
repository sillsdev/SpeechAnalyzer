#include "stdafx.h"
#include "ModelImpl.h"

int CModelImpl::GetProcessText(ProcessorType processorType) {
    switch (processorType) {
    case PROCESSDUR: return IDS_STATTXT_PROCESSDUR;
    case PROCESSFMT: return IDS_STATTXT_PROCESSFMT;
    case PROCESSWBLP: return  IDS_STATTXT_PROCESSWBLP;
    case PROCESSWBEQU: return IDS_STATTXT_PROCESSWBEQU;
    case PROCESSDFLT: return IDS_STATTXT_PROCESSING;
    case PROCESSZCR: return IDS_STATTXT_PROCESSZCR;
    case PROCESSWVL: return IDS_STATTXT_PROCESSWVL;
    case PROCESSWBECHO: return IDS_STATTXT_PROCESSWBECHO;
    case PROCESSWBREV: return IDS_STATTXT_PROCESSWBREV;
    case PROCESSPIT: return IDS_STATTXT_PROCESSPIT;
    case PROCESSLOU: return IDS_STATTXT_PROCESSLOU;
    case PROCESSGRA: return IDS_STATTXT_PROCESSGRA;
    case BACKGNDFRA: return IDS_STATTXT_BACKGNDFRA;
    case PROCESSFRA: return IDS_STATTXT_PROCESSFRA;
    case PROCESSTWC: return IDS_STATTXT_PROCESSTWC;
    case PROCESSCHA: return IDS_STATTXT_PROCESSCHA;
    case PROCESSCPI: return IDS_STATTXT_PROCESSCPI;
    case BACKGNDGRA: return IDS_STATTXT_BACKGNDGRA;
    case PROCESSSLO: return IDS_STATTXT_PROCESSSLO;
    case PROCESSPOA: return IDS_STATTXT_PROCESSPOA;
    case PROCESSRAT: return IDS_STATTXT_PROCESSRAT;
    case PROCESSSPI: return IDS_STATTXT_PROCESSSPI;
    case PROCESSSPG: return IDS_STATTXT_PROCESSSPG;
    case PROCESSSPU: return IDS_STATTXT_PROCESSSPU;
    case PROCESSMEL: return IDS_STATTXT_PROCESSMEL;
    case PROCESSRAW: return IDS_STATTXT_PROCESSRAW;
    case SEGMENTING: return IDS_STATTXT_SEGMENTING;
    case PARSING: return IDS_STATTXT_PARSING;
    case PROCESSWBGENERATOR: return IDS_STATTXT_PROCESSWBGENERATOR;
    }
}

