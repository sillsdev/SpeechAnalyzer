#pragma once

class CFileEncodingHelper
{
public:
	CFileEncodingHelper( LPCTSTR filename);
	~CFileEncodingHelper(void);

	bool CheckEncoding( bool display);
	bool ConvertFileToUTF16( wistringstream & result);
	bool ConvertFileToUTF16( wstring & result);
	bool ConvertFileToUTF8( istringstream & result);

private:
	bool ReadFileIntoBuffer( string & unknown); 
	bool IsUTF32( string & unknown); 
	bool IsUTF8( string & unknown);
	bool IsUTF16BE( string & unknown);
	bool IsUTF16LE( string & unknown);
	bool IsASCII( string & unknown);
	bool ConvertUTF16BEToUTF16( string & unknown, wstring & result);
	bool ConvertUTF16LEToUTF16( string & unknown, wstring & result);
	bool ConvertUTF8ToUTF16( string & unknown, wstring & result, bool removeBOM);
	bool ConvertASCIIToUTF16( string & unknown, wstring & result);
	bool ConvertUTF16ToUTF8( wstring & unknown, istringstream & result);

	wstring filename;
};
