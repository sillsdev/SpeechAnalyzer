/////////////////////////////////////////////////////////////////////////////
// Riff.cpp:
// Interface for the CRiff class.
//
// Author: Steve MacLean
// copyright 2000-2001 JAARS Inc. SIL
//
// Revision History
//   03/08/2001 SDM Initial version very rudimentary Riff support expect to collect 
//                  various riff functionality from around SA.
/////////////////////////////////////////////////////////////////////////////
#ifndef RIFF_H
#define RIFF_H

class CRiff  {
public:
	CRiff();
	virtual ~CRiff();

	static BOOL NewWav(const TCHAR* szPathName, PCMWAVEFORMAT &pcm, const TCHAR* szRawDataPath);
};

#endif
