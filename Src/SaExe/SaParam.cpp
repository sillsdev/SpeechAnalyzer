#include "SaParam.h"
#include "AppDefs.h"

CSaParam::CSaParam() {
    wFlags = 0;
    byRecordFileFormat = FILE_FORMAT_WAV;
    dwRecordBandWidth = 0;
    byRecordSmpSize = 0;
    dwNumberOfSamples = 0;
    lSignalMax = 0;
    lSignalMin = 0;
    dwSignalBandWidth = 0;
    byQuantization = 0;
}

CSaParam CSaParam::operator=(const CSaParam & right) {
	szDescription = right.szDescription;
	wFlags = right.wFlags;
	byRecordFileFormat = right.byRecordFileFormat;
	RecordTimeStamp = right.RecordTimeStamp;
	dwRecordBandWidth = right.dwRecordBandWidth;
	byRecordSmpSize = right.byRecordSmpSize;
	dwNumberOfSamples = right.dwNumberOfSamples;
	lSignalMax = right.lSignalMax;
	lSignalMin = right.lSignalMin;
	dwSignalBandWidth = right.dwSignalBandWidth;
	byQuantization = right.byQuantization;
	return *this;
}

