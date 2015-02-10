#ifndef AUTOSAVE_H
#define AUTOSAVE_H

class CSaApp;
class CSaDoc;

#ifdef DEBUG
#define AUTOSAVE_TIMER      10000	// fires every ten seconds
#else
#define AUTOSAVE_TIMER      60000   // fires every minute
#endif

class CAutoSave
{
public:
	// per-document methods
	CAutoSave();
	void StoreAutoRecoveryInformation( CSaDoc * pDoc);
	bool IsSaving();
	bool IsUpdating();

	// global methods
	static void Check( CSaApp * pApp);
    static void CleanAll();
    static wstring GetDirectory();
    
private:
	void WriteInfo( LPCTSTR path, bool isTempFile, LPCTSTR aswave, LPCTSTR asxml, LPCTSTR restorewave, LPCTSTR root, LPCTSTR folder);
	ULONGLONG GetFileSize( LPCTSTR filename);

	static void ReadInfo( LPCTSTR path, bool & isTempFile, wstring & aswave, wstring & asxml, wstring & restorewave, wstring & root, wstring & folder);
	static void RestoreFile( LPCTSTR from, LPCTSTR to);

    bool error;
    bool saving;
	bool updating;
};

#endif
