#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>
#include <fstream.h>
#include <windows.h>

struct TableEntry
{
	char *m_Filename;
	char *m_User;
	char *m_Date;
	char *m_Time;
	char *m_Comment;

	TableEntry(){m_Filename=m_User=m_Date=m_Time=m_Comment=NULL;}
	BOOL operator==(TableEntry &te);
	TableEntry &operator<<(TableEntry &te);
	BOOL Input(ifstream &infile,BOOL bCmdLineVersion);
	void AppendFilename(const char *fn);
	void Output(ofstream &outfile);
	void Clear();
};//TableEntry

class TableOfEntries
{
	TableEntry m_Entry[2];
	BOOL bCmdLineVersion;

	void Process(const char *infilename,const char *outfilename);
public:

	TableOfEntries(const char *infilename,const char *outfilename)
	{
		Process(infilename,outfilename);
	}
};//TableOfEntries

#endif // TABLE.H