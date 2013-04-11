#include "Stdafx.h"
#include "AutoSave.h"
#include "resource.h"
#include "Sa_Doc.h"
#include "FileUtils.h"
#include "sa.h"
#include "Shlobj.h"
#include <Windows.h>

typedef HMODULE(__stdcall * SHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPTSTR);

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData)
{

    // If the BFFM_INITIALIZED message is received
    // set the path to the start path.
    switch (uMsg)
    {
    case BFFM_INITIALIZED:
    {
        if (NULL != lpData)
        {
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        }
    }
    }
    return 0;
}

CAutoSave::CAutoSave()
{
	saving = false;
    error = false;
	updating = false;
}

/**
* check the autosave directory to see if any files should be restored
*/
void CAutoSave::Check( CSaApp * pApp)
{
    wstring autosavedir = GetDirectory();
    CFileFind finder;
    wstring search(autosavedir);
    search.append(L"*.*");

    int count = 0;
    if (finder.FindFile(search.c_str(),0)!=TRUE) return;

	// determine if there are any files to load
    BOOL more = TRUE;
    do
    {
        more = finder.FindNextFile();
        if (finder.IsDirectory()) continue;
        if (finder.IsDots()) continue;
        wstring path = finder.GetFilePath();
        if (::EndsWith(path.c_str(),L".info"))
        {
            count++;
        }
    }
    while (more);

	// if there aren't any files, we are done.
    if (count==0) return;

	// if so, prompt the user and start reloading the files.
    int result = AfxMessageBox(IDS_AUTOSAVE_MSG, MB_YESNO | MB_ICONQUESTION);
    if (result!=IDYES)
	{
		CleanAll();
		return;
	}

    if (finder.FindFile(search.c_str(),0)!=TRUE) return;
    
	more = TRUE;
    do
    {
        more = finder.FindNextFile();
        if (finder.IsDirectory()) continue;
        if (finder.IsDots()) continue;

        wstring path = finder.GetFilePath();
        if (!::EndsWith(path.c_str(),L".info")) continue;

		// read the info file
		bool isTempFile;
		wstring aswave;
		wstring asxml;
		wstring restorewave;
		wstring root;
		wstring folder;

		ReadInfo( path.c_str(), isTempFile, aswave, asxml, restorewave, root, folder);

		// we are done with the info file.
		::DeleteFile(path.c_str());

		CString time;
		CFileStatus fs;
		if (CFile::GetStatus( aswave.c_str(), fs))
		{
			CTime t = fs.m_mtime;
			time = t.Format("%#c");
		}
		else
		{
			time = "UNKNOWN";
		}

		if (isTempFile)
		{
			CString msg;
			msg.FormatMessage(IDS_AUTOSAVE_RESTORE_TEMP, restorewave.c_str(), (LPCTSTR)time);
			if (AfxMessageBox(msg, MB_YESNO|MB_ICONQUESTION, 0) != IDYES) 
			{
				::DeleteFile( aswave.c_str());
				::DeleteFile( asxml.c_str());
				continue;
			}

			// select a directory to restore the file to.
			TCHAR documentPath[_MAX_PATH];
			wmemset( documentPath, 0, _countof(documentPath));

			HMODULE hModule = LoadLibrary(_T("SHFOLDER.DLL"));
			if (hModule != NULL)
			{
				SHGETFOLDERPATH fnShGetFolderPath = (SHGETFOLDERPATH)GetProcAddress(hModule, "SHGetFolderPathW");
				if (fnShGetFolderPath != NULL)
				{
					fnShGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, documentPath);
				}
				FreeLibrary(hModule);
			}

			{
				// szCurrent is an optional start folder. Can be NULL.
				// szPath receives the selected path on success. Must be MAX_PATH characters in length.

				CoInitialize(NULL);

				TCHAR szDisplay[MAX_PATH];
				memset(szDisplay,0,sizeof(szDisplay));

				TCHAR szPath[MAX_PATH];
				memset(szPath,0,sizeof(szPath));

				BROWSEINFO bi = { 0 };
				bi.hwndOwner = NULL;
				bi.pszDisplayName = szDisplay;
				CString msg;
				msg.LoadStringW(IDS_CHOOSE_FOLDER);
				bi.lpszTitle = msg.GetBuffer(msg.GetLength());
				bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
				bi.lpfn = BrowseCallbackProc;
				bi.lParam = (LPARAM)(LPCTSTR)documentPath;
				LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
				if (pidl==NULL)
				{
					// they cancelled...
					msg.ReleaseBuffer();
					CoUninitialize();
					return;
				}
				msg.ReleaseBuffer();

				BOOL retval = SHGetPathFromIDList( pidl, szPath);
				CoTaskMemFree(pidl);
				if (!retval)
				{
					szPath[0] = TEXT('\0');
				}

				folder = szPath;
				CoUninitialize();
			}

			AppendDirSep(folder);

			restorewave.clear();
			restorewave.append(folder);
			restorewave.append(root);
			restorewave.append(L".wav");

			wstring restorexml;
			restorexml.append(folder);
			restorexml.append(root);
			restorexml.append(L".saxml");

			// restore the files
			RestoreFile( aswave.c_str(), restorewave.c_str());
			RestoreFile( asxml.c_str(), restorexml.c_str());

			pApp->OpenDocumentFile( restorewave.c_str());
		}
		else
		{
			CString msg;
			msg.FormatMessage(IDS_AUTOSAVE_RESTORE, restorewave.c_str(), (LPCTSTR)time);
			if (AfxMessageBox(msg, MB_YESNO|MB_ICONQUESTION, 0) != IDYES) 
			{
				::DeleteFile( aswave.c_str());
				::DeleteFile( asxml.c_str());
				continue;
			}

			// restore the wave file
			RestoreFile( aswave.c_str(), restorewave.c_str());

			wstring restorexml;
			restorexml.append(folder);
			AppendDirSep(folder);
			restorexml.append(root);
			restorexml.append(L".saxml");

			RestoreFile( asxml.c_str(), restorexml.c_str());

	        pApp->OpenDocumentFile( restorewave.c_str());
		}
    } while (more);
}

wstring CAutoSave::GetDirectory()
{
    // find or create the autosave directory
    TCHAR buffer[_MAX_PATH];
    ::GetEnvironmentVariable(L"TEMP",buffer,_countof(buffer));
    wstring autosavedir(buffer);
    AppendDirSep(autosavedir);
    autosavedir.append(L"SpeechAnalyzer");
    AppendDirSep(autosavedir);
    return autosavedir;
}

void CAutoSave::CleanAll()
{
    wstring autosavedir = GetDirectory();
    wstring search(autosavedir);
    search.append(L"*.*");

    CFileFind finder;
    if (finder.FindFile(search.c_str(),0))
    {
        BOOL more = TRUE;
        do
        {
            more = finder.FindNextFileW();
            if (finder.IsDirectory()) continue;
            if (finder.IsDots()) continue;
            wstring path = finder.GetFilePath();
            ::RemoveFile(path.c_str());
        }
        while (more);
    }
}

void CAutoSave::StoreAutoRecoveryInformation( CSaDoc * pDoc)
{

    //TRACE(L"auto saving...\n");

	if (updating)
	{
		//TRACE("save in process. ignoring request\n");
		return;
	}

	updating = true;

    if (!pDoc->IsModified())
    {
        //TRACE("No changes pending. nothing to save\n");
		updating = false;
        return;
    }

    CSaApp * pSaApp = (CSaApp *)AfxGetApp();

    // find or create the autosave directory
    TCHAR buffer[_MAX_PATH];
    ::GetEnvironmentVariable(L"TEMP",buffer,_countof(buffer));
    wstring temp(buffer);
    AppendDirSep(temp);

    wstring autosavedir = GetDirectory();

    if (!::FolderExists(autosavedir.c_str()))
    {
        //create directory ?
        if (CreateDirectory(autosavedir.c_str(), NULL) ==0)
        {
            //an error has occured, process the error here
            TRACE("Unable to create autosave directory!\n");
            if (!error)
            {
                pSaApp->ErrorMessage(IDS_ERROR_AUTOSAVE_FAIL,autosavedir.c_str());
                error = true;
            }
			updating = false;
            return;
        }
    }

	bool isTempFile = false;
    wstring filename;
	wstring root;
	wstring restorewave;
	wstring folder;
	wstring currentwave;
	wstring currentxml;

    // what is our scenario?
	if (pDoc->IsUsingTempFile())
    {
        // a recorded file
		wstring original = pDoc->GetTempFilename();
        // extract the filename
        filename.append(original);
        filename = filename.substr(temp.length(),filename.length()-temp.length());
		isTempFile = true;
		root = filename.substr( 0, filename.length()-4);
		restorewave = original;
		folder = L"";

		currentwave.append(autosavedir);
		currentwave.append(root);
		currentwave.append(L".tmp.autosave");

		currentxml.append(autosavedir);
		currentxml.append(root);
		currentxml.append(L".saxml.autosave");
	}
    else
    {
        // a prerecorded file
        wstring original = pDoc->GetPathName();
        filename = original;
        int pos = filename.rfind('\\');
        if (pos!=wstring::npos)
        {
            filename = filename.substr(pos+1,filename.length()-pos-1);
        }
		root = filename.substr( 0, filename.length()-4);
		restorewave = original;
		folder = original.substr(0,pos);
		::AppendDirSep(folder);

		currentwave.append(autosavedir);
		currentwave.append(root);
		currentwave.append(L".wav.autosave");

		currentxml.append(autosavedir);
		currentxml.append(root);
		currentxml.append(L".saxml.autosave");
	}

	// compare sizes.  if they are the same, don't do anything.
	ULONGLONG srcsize = GetFileSize(restorewave.c_str());
	ULONGLONG destsize = GetFileSize(currentwave.c_str());
	//TRACE(L"wave size size: %lu dest size: %lu\n",(unsigned long)srcsize,(unsigned long)destsize);

	if (srcsize!=destsize)
	{
	    //TRACE(L"saving audio for %s\n",restorewave.c_str());
		// if the current autosave wave file exists, delete it
		if (currentwave.length()>0)
		{
			if (::FileExists(currentwave.c_str()))
			{
				::RemoveFile(currentwave.c_str());
			}
		}

		if (!::CopyFile(restorewave.c_str(), currentwave.c_str(), TRUE))
		{
			TRACE(L"Unable to save wave file\n");
		}
	}
	else
	{
		//TRACE("skipping .wave save because files are same size\n");
	}

	if (pDoc->IsTransModified())
    {
        //TRACE(L"saving transcription for %s\n",(LPCTSTR)restorewave.c_str());

		if (currentxml.length()>0)
		{
			if (::FileExists(currentxml.c_str()))
			{
				::RemoveFile(currentxml.c_str());
			}
		}

        // copy any transcription to the autosave directory
        saving = true;
        pDoc->WriteDataFiles( currentwave.c_str());
        saving = false;

		// rename it to the appropriate name
		wstring oldname;
		oldname.append(currentwave);
		int pos = oldname.rfind('.');
		oldname = oldname.substr(0,pos);
		oldname.append(L".saxml");
		::RenameFile( oldname.c_str(), currentxml.c_str());
    }

	// write the info file
	wstring info;
	info.append(autosavedir);
	info.append(root);
	info.append(L".info");

	WriteInfo( info.c_str(), isTempFile, currentwave.c_str(), currentxml.c_str(), restorewave.c_str(), root.c_str(), folder.c_str());

    //TRACE(L"saved\n");
	updating = false;
}

bool CAutoSave::IsSaving()
{
	return saving;
}

void CAutoSave::WriteInfo( LPCTSTR path, bool isTempFile, LPCTSTR aswave, LPCTSTR asxml, LPCTSTR restoreWave, LPCTSTR root, LPCTSTR folder)
{
	wofstream ofs(path);
	ofs << isTempFile << "\n";
	ofs << aswave << "\n";
	ofs << asxml << "\n";
	ofs << restoreWave << "\n";
	ofs << root << "\n";
	ofs << folder << "\n";
	ofs.close();
}

void CAutoSave::ReadInfo( LPCTSTR path, bool & isTempFile, wstring & aswave, wstring & asxml, wstring & restoreWave, wstring & root, wstring & folder)
{
	wifstream ifs(path);
	wstring val;
	getline(ifs, val);
	isTempFile = (val.compare(L"0")!=0)?true:false;
	getline(ifs, aswave);
	getline(ifs, asxml);
	getline(ifs, restoreWave);
	getline(ifs, root);
	getline(ifs, folder);
	ifs.close();
}

void CAutoSave::RestoreFile( LPCTSTR from, LPCTSTR to)
{
	if (::FileExists( to))
	{
		wstring backup;
		backup.append(to);
		backup.append(L".bak");
		if (::FileExists( backup.c_str()))
		{
			::DeleteFile( backup.c_str());
		}
		::RenameFile( to, backup.c_str());
	}
	::CopyFile( from, to, FALSE);
	::DeleteFile( from);
}

ULONGLONG CAutoSave::GetFileSize( LPCTSTR filename)
{
	if (filename==NULL) return 0;
	if (wcslen(filename)==0) return 0;
	CFileStatus fs;
	if (!CFile::GetStatus( filename,fs)) return 0;
	return fs.m_size;
}
