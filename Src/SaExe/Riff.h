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

#if !defined(_SA_RIFF_H)
#define _SA_RIFF_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CRiff  
{
public:
	CRiff();
	virtual ~CRiff();

	static BOOL NewWav(const TCHAR* szPathName, PCMWAVEFORMAT &pcm, const TCHAR* szRawDataPath);
};

#endif // _SA_RIFF_H
