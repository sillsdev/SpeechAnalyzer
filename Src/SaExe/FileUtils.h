#define DIRECTORY_SEPARATOR '\\'

extern bool FileExists( LPCTSTR path);
extern bool FolderExists( LPCTSTR path);
extern bool CreateFolder( LPCTSTR path);
extern void AppendDirSep( LPTSTR path, size_t size);
