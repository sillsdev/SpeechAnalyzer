//
// Registry.h
//
// sdfWriteRegistryString(), sdfGetRegistryString()
//

// This function works like the Windows API function WriteProfileString
// except that in the 32-bit DLL it works with the registry instead of WIN.INI
BOOL sdfWriteRegistryString(const char* Section,const char* Key,const char* String);

// This function works like the Windows API function GetProfileString
// except that in the 32-bit DLL it works with the registry instead of WIN.INI
DWORD sdfGetRegistryString(const char* Section, const char* Key, const char* Default,
								char* ReturnBuffer, DWORD ReturnSize);
int sdfGetRegistryInt(const char* Section, const char* Key,int Default);

//end of file

