//
// Registry.cpp
//
// sdfWriteRegistryString(), sdfGetRegistryString()
//

#include <windows.h>
#include <stdlib.h> // for atoi()
#include "registry.h"

// This function works like the Windows API function WriteProfileString
// except that in the 32-bit DLL it works with the registry instead of WIN.INI
BOOL sdfWriteRegistryString(const char* Section,const char* Key,const char* String)
{
	HKEY hSIL;
	HKEY hSection;
	BOOL RetValue;

	if(RegCreateKey(HKEY_CURRENT_USER,"Software\\SIL",&hSIL)==ERROR_SUCCESS)
	{
		if(RegCreateKey(hSIL,Section,&hSection)==ERROR_SUCCESS)
		{
			if(!String) RetValue = RegDeleteValue(hSection,Key);
			else
				RetValue =
				RegSetValueEx(hSection,Key,NULL,REG_SZ,(CONST BYTE *)String,strlen(String)+1)
				==ERROR_SUCCESS ? TRUE:FALSE;
			RegCloseKey(hSection);
		}
		else RetValue = FALSE;
		RegCloseKey(hSIL);
		return RetValue;
	}
	else return FALSE;
}//sdfWriteRegistryString

// This function works like the Windows API function GetProfileString
// except that in the 32-bit DLL it works with the registry instead of WIN.INI
DWORD sdfGetRegistryString(const char* Section, const char* Key, const char* Default,
								char* ReturnBuffer, DWORD ReturnSize)
{
	HKEY hSIL;
	HKEY hSection;
	BOOL RetValue;

	if(RegOpenKey(HKEY_CURRENT_USER,"Software\\SIL",&hSIL)==ERROR_SUCCESS)
	{
		if(RegOpenKey(hSIL,Section,&hSection)==ERROR_SUCCESS)
		{
			DWORD Type = REG_SZ;

			RetValue =
				RegQueryValueEx(hSection,Key,NULL,&Type,(LPBYTE)ReturnBuffer,&ReturnSize)
				==ERROR_SUCCESS ? TRUE:FALSE;
			if(RetValue==TRUE) ReturnBuffer[ReturnSize-1]='\0';
			else strcpy(ReturnBuffer,Default);
			RegCloseKey(hSection);
		}
		else
		{
			RetValue = FALSE;
			strcpy(ReturnBuffer,Default);
		}
		RegCloseKey(hSIL);
		return RetValue;
	}
	strcpy(ReturnBuffer,Default);
	return FALSE;
}//sdfGetRegistryString

int sdfGetRegistryInt(const char* Section, const char* Key,int Default)
{
	char aNumber[256] = "";
	sdfGetRegistryString(Section,Key,"X",aNumber,256);
	if(*aNumber=='X') return Default;
	return atoi(aNumber);
}//sdfGetRegistryInt

