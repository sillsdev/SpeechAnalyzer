#pragma once
#ifndef SAPROCESSOR_H
#define SAPROCESSOR_H

#include "sap_defs.h"
#include "context.h"

#include "sa_process.h"
#include "processdoc.h"
#include "Butterworth.h"
#include "FormantTracker.h"
#include "Hilbert.h"
#include "lpc.h"
#include "PitchParm.h"
#include "ResearchSettings.h"
#include "sa_p_3dPitch.h"
#include "sa_p_cha.h"
#include "sa_p_custompitch.h"
#include "sa_p_dur.h"
#include "sa_p_fmt.h"
#include "sa_p_fra.h"
#include "sa_p_glo.h"
#include "sa_p_grappl.h"
#include "sa_p_InstantaneousPower.h"
#include "sa_p_lou.h"
#include "sa_p_melogram.h"
#include "sa_p_pitch.h"
#include "sa_p_poa.h"
#include "sa_p_rat.h"
#include "sa_p_raw.h"
#include "sa_p_sfmt.h"
#include "sa_p_smoothedpitch.h"
#include "sa_p_spg.h"
#include "sa_p_spu.h"
#include "sa_p_twc.h"
#include "sa_p_wavelet.h"
#include "sa_p_zcr.h"
#include "sa_w_equ.h"
#include "SA_W_PAS.H"
#include "SA_W_REV.H"
#include "wbprocess.h"
#include "sa_w_adj.h"

#include "VowelFormants.h"
#include "VowelFormantSet.h"
#include "VowelFormantSets.h"
#include "VowelFormantsVector.h"
#include "VowelSetVersion.h"

#include "funcs.h"
#include "scopedcursor.h"

#endif
