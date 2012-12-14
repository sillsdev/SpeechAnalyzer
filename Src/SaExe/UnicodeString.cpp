/////////////////////////////////////////////////////////////////////////////
// UnicodeString.cpp:
// Implementation of various unicode functions
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnicodeString.h"

CUtf32String CUnicodeString::operator+(const CUnicodeString &szString) const
{
	CUtf32String szResult;

	szResult += *this;
	szResult += szString;

	return szResult;
}

CUtf16String CUnicodeString::Decode(CFont *pFont, const std::string* szStringEncoded)
{
	ASSERT(pFont);

	const char* szEncoded = szStringEncoded->c_str();

	if(!pFont)
		return CUtf8String(szEncoded);

	int nCharset = 0;
	if (pFont)
	{
		LOGFONT lf;
		pFont->GetLogFont(&lf);

		nCharset = lf.lfCharSet;
	}

	CHARSETINFO charsetinfo;

	if (TranslateCharsetInfo(reinterpret_cast<DWORD*>(nCharset),&charsetinfo, TCI_SRCCHARSET))
	{  
		int nCodePage = charsetinfo.ciACP;

		int nLength = MultiByteToWideChar(nCodePage,0,szEncoded,-1,NULL,0);

		if(nLength)
		{
			DWORD dwError = GetLastError();
			UNUSED_ALWAYS(dwError);

			LPWSTR pStr = new WCHAR[nLength+1];

			MultiByteToWideChar(nCodePage,0,szEncoded,-1,pStr,nLength+1);

			CUtf16String szTextU(pStr);

			delete [] pStr;

			return szTextU;
		}
	}

	{
		CUtf32String szString;
		int nLength = strlen(szEncoded);
		for(int i=0;i<nLength;i++)
		{
			szString += CUtf32String(0xf000+(unsigned char)(szEncoded[i])); // encode as private use as in symbol font
		}

		return szString;
	}
}

std::string CUnicodeString::Encode(CFont *pFont) const
{
	return CUtf16String(*this).Encode(pFont);
}

std::string CUtf16String::Encode(CFont *pFont) const
{
	ASSERT(pFont);

	if(!pFont)
		return CUtf8String(*this).getUtf8();

	int nCharset = 0;
	if (pFont)
	{
		LOGFONT lf;
		pFont->GetLogFont(&lf);

		nCharset = lf.lfCharSet;
	}

	CHARSETINFO charsetinfo;

	if(TranslateCharsetInfo(reinterpret_cast<DWORD*>(nCharset),&charsetinfo, TCI_SRCCHARSET))
	{  
		int nCodePage = charsetinfo.ciACP;

		int nLength = WideCharToMultiByte(nCodePage,0,getUtf16().c_str(),-1,NULL,0,NULL,NULL);

		LPSTR pStr = new CHAR[nLength+1];

		WideCharToMultiByte(nCodePage,0,getUtf16().c_str(),-1,pStr,nLength+1,NULL,NULL);

		std::string szText(pStr);

		delete [] pStr;

		return szText;
	}

	{
		std::string szText;

		for(int i=0;m_szContents[i];i++)
		{
			int nChar = m_szContents[i];

			if(nChar & 0xFFFFFF00)
				nChar = (nChar & 0xFFFFFF00) == 0xf000 ? nChar & 0xff : '?';

			szText += CHAR(nChar);
		}
		return szText;
	}
}

CUtf32String CUnicodeString::Left(int nChars) const
{
	return CUtf32String(*this).Left(nChars);
}

CUtf32String::operator CUtf8String() const
{
	CUtf8String szResult;

	const unsigned long *pLong = m_szContents.c_str();

	for(int i=0; pLong[i]; i++)
	{
		szResult += pLong[i];
	}

	return szResult;
}

CUtf32String::operator CUtf16String() const
{
	CUtf16String szResult;

	const unsigned long *pLong = m_szContents.c_str();

	for(int i=0; pLong[i]; i++)
	{
		szResult += pLong[i];
	}

	return szResult;
}

CUtf32String CUtf32String::Left(int nChars) const
{
	CUtf32String szResult;

	szResult.m_szContents = m_szContents.substr(0, nChars);

	return szResult;
}

CUtf32String CUtf32String::Mid(int nChars) const
{
	CUtf32String szResult;

	szResult.m_szContents = m_szContents.substr(nChars);

	return szResult;
}

CUtf32String CUtf32String::Mid(int nStart, int nChars) const
{
	CUtf32String szResult;

	szResult.m_szContents = m_szContents.substr(nStart, nChars);

	return szResult;
}

CUtf16String::operator CUtf8String() const
{
	return CUtf32String(*this);
}

CUtf16String::operator CUtf32String() const
{
	unsigned short* pShort = (unsigned short*) m_szContents.c_str();
	CUtf32String szResult;
	const unsigned long kReplacement = 0x00fffd;

	for (int i = 0; pShort[i]; i++)
	{
		Kef16 kef16 = classifyChar(pShort[i]);// find next lead bytepChar[i] 

		switch(kef16)
		{
		case kef16BMP:
			{
				unsigned long wChar = pShort[i];

				BOOL bIsValid = !(wChar >= 0xd800 && wChar < 0xe000);
				// if properly encode output entered character
				szResult += bIsValid ? wChar : kReplacement;
			}
			break;
		case kef16HighSurrogate: // uuuuuxxxxxxyyyyyyyyyy <> 110110wwwwxxxxxx 110111yyyyyyyyyy
			if(classifyChar(pShort[i+1]) == kef16LowSurrogate)
			{
				unsigned long wChar = pShort[i] & 0x3ffL;
				wChar = (wChar << 10) + (pShort[i+1] & 0x3ffL);

				wChar += 0x10000;

				BOOL bIsValid = !(wChar >= 0x10000 && wChar < 0x110000);
				// if properly encode output entered character
				szResult += bIsValid ? wChar : kReplacement;
				i+= 1; // consumed one trail 
			}
			else
				szResult += kReplacement;
			break;
		case kef16BOM:
			break;
		default:
			szResult += kReplacement;
		}
	}
	return szResult;
}

CUtf16String::Kef16 CUtf16String::classifyChar(unsigned short usValue) const
{
	if((usValue & 0xFC00) == 0xd800) // 0xxxxxxx
		return kef16HighSurrogate;
	if((usValue & 0xFC00) == 0xdC00) // 0xxxxxxx
		return kef16LowSurrogate;
	if(usValue == 0xFEFF)
		return kef16BOM;
	if(usValue == 0xFFFE)
		return kef16Error;
	return kef16BMP;
}

CUtf16String& CUtf16String::operator+=(const unsigned long lChar)
{
	if(lChar < 0x10000)
	{
		m_szContents += (unsigned short) lChar;
	}
	else if(lChar <0x110000)
	{
		unsigned short wHighSurrogate = unsigned short(0xd800 + ((lChar - 0x10000) >> 10));
		unsigned short wLowSurrogate = unsigned short(0xdc00 + (lChar & 0x3FF));

		m_szContents += wHighSurrogate;
		m_szContents += wLowSurrogate;
	}
	else
	{
		const unsigned long kReplacement = 0x00fffd;

		return operator+=(kReplacement);
	}


	return *this;
}

CUtf8String::operator CUtf16String() const
{
	return CUtf32String(*this);
}

CUtf8String::operator CUtf32String() const
{
	unsigned char* pChar = (unsigned char*) m_szContents.c_str();
	CUtf32String szResult;
	const unsigned long kReplacement = 0x00fffd;

	for(int i = 0; pChar[i]; i++)
	{
		Kef8 kef8 = classifyChar(pChar[i]);// find next lead bytepChar[i] 

		switch(kef8)
		{
		case kef8Lead1: //      000000000yyyyyyy <> 0yyyyyyy
			szResult += pChar[i];
			break;
		case kef8Lead2: //      00000yyyyyxxxxxx <> 110yyyyy 10xxxxxx ->
			if(classifyChar(pChar[i+1]) == kef8Trail)
			{
				WCHAR wChar = WCHAR(pChar[i] & 0x1fL);
				wChar = WCHAR((wChar << 6) + (pChar[i+1] & 0x3fL));

				szResult += wChar >= 0x80 ? wChar : kReplacement;
				i+= 1; // consumed one trail byte
			}
			else
				szResult += kReplacement;
			break;
		case kef8Lead3: //      zzzzyyyyyyxxxxxx <> 1110zzzz 10yyyyyy 10xxxxxx->
			if(classifyChar(pChar[i+1]) == kef8Trail && classifyChar(pChar[i+2]) == kef8Trail)
			{
				WCHAR wChar = WCHAR(pChar[i] & 0x0fL);
				wChar = WCHAR((wChar << 6) + WCHAR(pChar[i+1] & 0x3fL));
				wChar = WCHAR((wChar << 6) + WCHAR(pChar[i+2] & 0x3fL));

				BOOL bIsValid = (wChar >= 0x800) && !(wChar >= 0xd800 && wChar < 0xe000);
				// if properly encode output entered character
				szResult += bIsValid ? wChar : kReplacement;
				i+= 2; // consumed two trail byte
			}
			else
				szResult += kReplacement;
			break;
		case kef8Lead4: // uuuzzzzzzyyyyyyxxxxxx <> 11110uuu 10zzzzzz 10yyyyyy 10xxxxxx ->
			if(classifyChar(pChar[i+1]) == kef8Trail && classifyChar(pChar[i+2]) == kef8Trail && classifyChar(pChar[i+3]) == kef8Trail)
			{
				unsigned long wChar = (pChar[i] & 0x07L);
				wChar = (wChar << 6) + WCHAR(pChar[i+1] & 0x3fL);
				wChar = (wChar << 6) + WCHAR(pChar[i+2] & 0x3fL);
				wChar = (wChar << 6) + WCHAR(pChar[i+3] & 0x3fL);

				BOOL bIsValid = (wChar >= 0x10000) &&  (wChar < 0x110000);
				szResult += bIsValid ? wChar : kReplacement;
				i+= 3; // consumed three trail byte
			}
			else
				szResult += kReplacement;
			break;
		case kef8FE:
			if(classifyChar(pChar[i+1]) == kef8FF)
			{
				// This is not a character, used to mark stream as unicode
				i+= 1; // consumed one trail byte
			}
			else
				szResult += kReplacement;
			break;
		default:
			szResult += kReplacement;
		}
	}
	return szResult;
}

CUtf8String::Kef8 CUtf8String::classifyChar(unsigned char ucValue) const
{
	if((ucValue & 0x80) == 0x00) // 0xxxxxxx
		return kef8Lead1;
	if((ucValue & 0xC0) == 0x80) // 10xxxxxx
		return kef8Trail;
	if((ucValue & 0xE0) == 0xC0) // 110xxxxx
		return kef8Lead2;
	if((ucValue & 0xF0) == 0xE0) // 1110xxxx
		return kef8Lead3;
	if((ucValue & 0xF8) == 0xF0) // 11110xxx
		return kef8Lead4;
	if(ucValue == 0xFF)
		return kef8FF;
	if(ucValue == 0xFE)
		return kef8FE;
	return kef8Other;
}

CUtf8String& CUtf8String::operator+=(const unsigned long lChar)
{
	if(lChar < 0x80)
	{
		m_szContents += (unsigned char) lChar;
	}
	else if(lChar <0x800)
	{
		unsigned char cLead2 = unsigned char(0xc0 + (lChar >> 6));
		unsigned char cTrail1 = unsigned char(0x80 + (lChar &0x3f));

		m_szContents += cLead2;
		m_szContents += cTrail1;
	}
	else if(lChar < 0x10000 && !(lChar >= 0xd800 && lChar < 0xe000))
	{
		unsigned char cLead3 = unsigned char(0xe0 + (lChar >> 12));
		unsigned char cTrail1 = unsigned char(0x80 + ((lChar >> 6) &0x3f));
		unsigned char cTrail2 = unsigned char(0x80 + (lChar &0x3f));

		m_szContents += cLead3;
		m_szContents += cTrail1;
		m_szContents += cTrail2;
	}
	else if(lChar < 0x110000)
	{
		unsigned char cLead4 = unsigned char(0xe0 + (lChar >> 18));
		unsigned char cTrail1 = unsigned char(0x80 + ((lChar >> 12) &0x3f));
		unsigned char cTrail2 = unsigned char(0x80 + ((lChar >> 6) &0x3f));
		unsigned char cTrail3 = unsigned char(0x80 + (lChar &0x3f));

		m_szContents += cLead4;
		m_szContents += cTrail1;
		m_szContents += cTrail2;
		m_szContents += cTrail3;
	}
	else
	{
		const unsigned long kReplacement = 0x00fffd;

		return operator+=(kReplacement);
	}

	return *this;
}


