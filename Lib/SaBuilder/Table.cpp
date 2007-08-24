#include "Table.h"

const char *RTF_HEADER =
"{\\rtf1\\ansi\\ftnbj\n"
"{\\fonttbl{\\f0 \\fmodern Courier New;}}\n"
"{\\colortbl ;\\red255\\green255\\blue255 ;\\red0\\green0\\blue128 ;\\red0\\green0\\blue0 ;}\n"
"{\\stylesheet\n"
"{\\f0\\fs24\\cf3\\cb1 Normal;}\n"
"{\\cs1\\cf3\\cb1 Default Paragraph Font;}\n"
"}\n"
"\\sectd\\marglsxn1133\\margrsxn1133\\margtsxn1133\\margbsxn1133\n"
"\\headery720\\footery720\\sbkpage\\pgncont\\f0\\cf0\\fs28\\qc\\fs20\n";


const char *TABLE_HEADER =
"\\par\\par\n"
"\\trowd\\trgaph60\\trleft0\n"
"\\clvertalt\\clbrdrt\\brdrs\\brdrw2\\clbrdrb\\brdrs\\brdrw2\\clbrdrl\\brdrs\\brdrw2\\clbrdrr\\brdrs\\brdrw2\\cellx2175\n"
"\\clvertalt\\clbrdrt\\brdrs\\brdrw2\\clbrdrb\\brdrs\\brdrw2\\clbrdrl\\brdrs\\brdrw2\\clbrdrr\\brdrs\\brdrw2\\cellx2950\n"
"\\clvertalt\\clbrdrt\\brdrs\\brdrw2\\clbrdrb\\brdrs\\brdrw2\\clbrdrl\\brdrs\\brdrw2\\clbrdrr\\brdrs\\brdrw2\\cellx3950\n"
"\\clvertalt\\clbrdrt\\brdrs\\brdrw2\\clbrdrb\\brdrs\\brdrw2\\clbrdrl\\brdrs\\brdrw2\\clbrdrr\\brdrs\\brdrw2\\cellx4825\n"
"\\clvertalt\\clbrdrt\\brdrs\\brdrw2\\clbrdrb\\brdrs\\brdrw2\\clbrdrl\\brdrs\\brdrw2\\clbrdrr\\brdrs\\brdrw2\\cellx9925\n"
"\\pard\\intbl\\s0\\ql\\cf2\\fs20\n"
"Filename\n"
"\\cell\\pard\\intbl\n"
"User\n"
"\\cell\\pard\\intbl\n"
"Date\n"
"\\cell\\pard\\intbl\n"
"Time\n"
"\\cell\\pard\\intbl\n"
"Comment\n"
"\\cell\\pard\\intbl\n"
"\\row\\pard\\intbl\\s0\\ql\\fs18\\f0\\cf0\n";


const char *CELL_TRAILER = "\n\\cell\\pard\\intbl\n";

const char *ROW_SEPARATOR = "\\row\\pard\\intbl\n";

const char *FINAL_ROW_TRAILER = "\\row\\pard\n\\par}\n";

int strrncmp( const char *string1, const char *string2, size_t count )
{
	size_t length1 = strlen(string1);
	size_t length2 = strlen(string2);
	if(length1<count || length2<count) return -1;

	return strcmp(string1+(length1-count),string2+(length2-count));
}

BOOL TableEntry::Input(ifstream &infile, BOOL bCmdLineVersion)
{
	if(infile.eof()||infile.bad()) return FALSE;

	char szLine[_MAX_PATH];

	// skip lines until we get to one that begins with *****
	do
	{
		infile.getline(szLine,_MAX_PATH);
		if(infile.eof()||infile.bad()) return FALSE;
	}
	while(strncmp(szLine,"*****",5));

	// grab m_Filename
	for(register i=0;szLine[i]=='*';++i); // skip *'s
	while(szLine[i]==' ')++i; // skip spaces
	if(szLine[i])
	{
		for(register j=strlen(szLine)-1;szLine[j]=='*';--j); // skip ending *'s
		while(szLine[j]==' ')--j; // skip trailing spaces
		szLine[j+1]='\0';
	}
	m_Filename = strdup(szLine+i);

	// read next line & grab m_User, m_Date & m_Time
	infile.getline(szLine,_MAX_PATH);
	if(infile.eof()||infile.bad()) return FALSE;

	// skip if this line is a "Version" line
	if(!strncmp(szLine,"Version",7))
		infile.getline(szLine,_MAX_PATH);

	// for labels, there's an inserted line with the labeling info
	if(!strncmp(szLine,"Label: ",7))
	{
		delete(m_Filename);
		m_Filename = new char[strlen(szLine+7)+13];
		strcpy(m_Filename,"\n\\cf2 ");
		strcat(m_Filename,szLine+7);
		strcat(m_Filename,"\n\\cf0 ");
		infile.getline(szLine,_MAX_PATH);
		if(infile.eof()||infile.bad()) return FALSE;
	}

	m_Time = strdup(szLine+(bCmdLineVersion?42:44));
	szLine[36] = '\0';
	m_Date = strdup(szLine+(bCmdLineVersion?25:28));
	for(i=6;szLine[i]!=' ';++i);
	szLine[i]='\0';
	m_User = strdup(szLine+6);

	// next line is either "Checked in","Labeled","Added","Destroyed",
	// "Deleted","Purged","Shared","Recovered", or "Created"
	infile.getline(szLine,_MAX_PATH); 

	if(
		( bCmdLineVersion &&
		  (!strrncmp(szLine," added",6) || !strrncmp(szLine," destroyed",10) || !strrncmp(szLine," recovered",10)
		  || !strrncmp(szLine," deleted",8) || !strrncmp(szLine," purged",7) || !strrncmp(szLine," shared",7) || !strncmp(szLine,"Created",7))
		)
		||
		( !bCmdLineVersion &&
		  (!strncmp(szLine,"Added ",6) || !strncmp(szLine,"Destroyed ",10) || !strncmp(szLine,"Recovered ",10)
		  || !strncmp(szLine,"Deleted ",8) || !strncmp(szLine,"Purged ",7) || !strncmp(szLine,"Shared ",7) || !strncmp(szLine,"Created ",8))
		)
	  )
	{
		if(bCmdLineVersion)
		{
			for(i=strlen(szLine)-1;i>0&&szLine[i]!=' ';--i);
			if(i)
			{
				delete m_Filename;
				szLine[i]='\0';
				m_Filename = strdup(szLine);
				m_Comment = strdup(szLine+i+1); // short record
			}
			else
				m_Comment = strdup("");
		}
		else
		{
			delete m_Filename;
			for(i=0;szLine[i]!=' '&&szLine[i];++i);
			szLine[i]='\0';
			m_Filename = strdup(szLine+i+1);
			m_Comment = strdup(szLine); // short record
		}
	}
	else
	{
		infile.getline(szLine,_MAX_PATH); // "Comment:" or "Label comment:"
		if(!strncmp(szLine,"Comment:",8)||!strncmp(szLine,"Label comment:",14))
		{
			// get multiline comment
			if(bCmdLineVersion)
			{
				if(szLine[0]=='C') // Comment:
					m_Comment = strdup(szLine + 9);
				else // Label comment;
					m_Comment = strdup(szLine + 15);
			}
			else
			{
				infile.getline(szLine,_MAX_PATH);
				if(infile.eof()||infile.bad()) return FALSE;
				m_Comment = strdup(szLine+2);
			}

			for(;;)
			{
				infile.getline(szLine,_MAX_PATH);
				if(infile.eof()||infile.bad()) return FALSE;
				if(!strncmp(szLine,"*****",5)) break;

				char *oldcomment = m_Comment;
				m_Comment = new char[strlen(oldcomment)+strlen(szLine)+8];
				strcpy(m_Comment,oldcomment);
				strcat(m_Comment,"\n\\line ");
				strcat(m_Comment,szLine);
				delete oldcomment;
			}

			// put back line that begins next entry
			infile.seekg((-2) - strlen(szLine),ios::cur);
		}
		else m_Comment = strdup("");
	}
	return TRUE;
}//TableEntry::Input

void TableEntry::AppendFilename(const char *fn)
{
	char *szString = new char[strlen(fn)+strlen(m_Filename)+8];
	strcpy(szString,m_Filename);
	strcat(szString,"\n\\line ");
	strcat(szString,fn);
	delete m_Filename;
	m_Filename = szString;
}//TableEntry::AppendFilename

void TableEntry::Clear()
{
	if(m_Filename)delete m_Filename; m_Filename=NULL;
	if(m_User)    delete m_User;     m_User=NULL;
	if(m_Date)    delete m_Date;     m_Date=NULL;
	if(m_Time)    delete m_Time;     m_Time=NULL;
	if(m_Comment) delete m_Comment;  m_Comment=NULL;
}//TableEntry::Clear

void TableEntry::Output(ofstream &outfile)
{	outfile << m_Filename << CELL_TRAILER
		<< m_User << CELL_TRAILER << m_Date << CELL_TRAILER
		<< m_Time << CELL_TRAILER << m_Comment << CELL_TRAILER;
	Clear();
}//TableEntry::Output

BOOL TableEntry::operator==(TableEntry &te)
{
	if(strcmp(te.m_Comment,m_Comment)) return FALSE;
	if(strcmp(te.m_Date,m_Date)) return FALSE;
	if(strcmp(te.m_Time,m_Time)) return FALSE;
	if(strcmp(te.m_User,m_User)) return FALSE;
	return TRUE;
}//TableEntry::operator==

TableEntry &TableEntry::operator<<(TableEntry &te)
{
	m_Comment  = te.m_Comment;   te.m_Comment = NULL;
	m_Date     = te.m_Date;      te.m_Date = NULL;
	m_Filename = te.m_Filename;  te.m_Filename = NULL;
	m_Time     = te.m_Time;      te.m_Time = NULL;
	m_User     = te.m_User;      te.m_User = NULL;

	return *this;
}//TableEntry::operator=

void TableOfEntries::Process(const char *infilename,const char *outfilename)
{
	bCmdLineVersion = FALSE;

	ifstream infile(infilename,ios::nocreate|ios::in);
	ofstream outfile(outfilename,ios::trunc|ios::out);

	char szLine[_MAX_PATH];
	infile.getline(szLine,_MAX_PATH);
	if(!strncmp(szLine,"Building list for $",19))
	{
		bCmdLineVersion = TRUE;
		for(register i=strlen(szLine)-1;szLine[i]=='.';--i);
		szLine[i+1]='\0';
	}
	else if(szLine[0]!='$')
		return; // This is not a SourceSafe History file

	outfile << RTF_HEADER;
	outfile << szLine + (bCmdLineVersion?18:0);
	outfile << TABLE_HEADER;

	// Load first entry
	if(m_Entry[0].Input(infile,bCmdLineVersion))
	{
		while(m_Entry[1].Input(infile,bCmdLineVersion)) // Load next entry
		{
			// If the same comment, etc. as previous, append filename
			if(m_Entry[0]==m_Entry[1])
			{
				m_Entry[0].AppendFilename(m_Entry[1].m_Filename);
				m_Entry[1].Clear();
			}

			// Otherwise output first entry and move this one up to first place
			else
			{
				m_Entry[0].Output(outfile);
				outfile << ROW_SEPARATOR;
				outfile.flush();
				m_Entry[0] << m_Entry[1];
			}
		}
	}

	// Output last entry
	m_Entry[0].Output(outfile);
	outfile << FINAL_ROW_TRAILER;

	outfile.flush();
	outfile.close();
	infile.close();
}//TableOfEntries::Input

//eof
