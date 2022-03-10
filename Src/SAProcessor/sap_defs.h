#pragma once
#ifndef _SAP_DEFS_H
#define _SAP_DEFS_H

#define EXPERIMENTAL_ACCESS true

#define PROGRAMMER_ACCESS ((GetKeyState(VK_CONTROL) & GetKeyState(VK_SHIFT)) < 0)

#define UNDEFINED_DATA                  -1          // data not yet defined
#define UNDEFINED_OFFSET                0xFFFFFFFF  // offset not yet defined
#define PRECISION_MULTIPLIER            10          // multiplier to amount precision
#define MAX_CALCULATION_FREQUENCY       500         // permissible parameter range

#define CALCULATION_INTERVAL(samplingRate)      (100*22050/samplingRate)         // number of raw data samples taken to build a processed data result
#define CALCULATION_DATALENGTH(samplingRate)    (300*22050/samplingRate)         // length of data used for calculation

#define ZCROSS_SAMPLEWINDOWWIDTH(samplingRate)  CALCULATION_INTERVAL(samplingRate)  // number of samples in the calculation window to calculate zero crossing

#define MAX_FILTER_NUMBER               3           // number of filters in workbench processes

#define MEL_MULT                        100         // multiplier for melogram data

// CECIL pitch default settings
#define CECIL_PITCH_MAXFREQ             500
#define CECIL_PITCH_MINFREQ             40
#define CECIL_PITCH_VOICING             32
#define CECIL_PITCH_CHANGE              10
#define CECIL_PITCH_GROUP               6
#define CECIL_PITCH_INTERPOL            7

// RIFF header definitions
#define FILE_FORMAT_PCM                 1           // PCM format in format chunk

#define FILE_FORMAT_UTT                 0           // record file format in sa chunk
#define FILE_FORMAT_WAV                 1           // record file format in sa chunk
#define FILE_FORMAT_MAC                 2           // record file format in sa chunk
#define FILE_FORMAT_TIMIT               3           // record file format in sa chunk
#define FILE_FORMAT_OTHER               4           // record file format in sa chunk

#endif
