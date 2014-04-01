#include "Stdafx.h"
#include "StringUtils.h"
#include "SaString.h"

string Utf8( LPCTSTR val)
{
	CSaString temp = val;
	string result = temp.utf8().c_str();
	return result;
}

string Utf8( CString val)
{
	CSaString temp = (LPCTSTR)val;
	string result = temp.utf8().c_str();
	return result;
}

