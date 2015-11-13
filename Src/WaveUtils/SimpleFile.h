#ifndef SIMPLE_FILE_H
#define SIMPLE_FILE_H

#include <Windows.h>

/**
* creates a nameless temporary file used for large memory operations
* file is deleted when object is destroyed.
*/
template<class T> class CSimpleFile {
public:
	CSimpleFile(void) :
	hFile(NULL) {

		TCHAR tempPath[MAX_PATH];
		DWORD result = FileUtils::GetTempDir(MAX_PATH,tempPath);
		if (result==0) {
			throw exception("Unable to retrieve temporary path");
		}

		memset(filename,0,MAX_PATH*sizeof(TCHAR));
		result = GetTempFileName( tempPath, L"SAT", 0, filename);
		if (result==0) {
			throw exception("Unable to retrieve temporary filename");
		}

		hFile = CreateFile( filename, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
		if (hFile==NULL) {
			throw exception("Unable to create temporary file");
		}
	}
	
	~CSimpleFile(void) {
		if (hFile!=NULL) {
			CloseHandle(hFile);
			hFile = NULL;
			DeleteFile(filename);
		}
	}
	
	/**
	* set position from front of file
	*/
	void SetPosition( LONG pos) {
		DWORD dwResult = SetFilePointer( hFile,pos,NULL,FILE_BEGIN);
		if (dwResult==INVALID_SET_FILE_POINTER) {
			throw exception("Unable to index file");
		}
	}

	/**
	* read from current position
	*/
	T Read() {
		T buffer = 0;
		DWORD bytesRead = 0;
		BOOL bResult = ReadFile( hFile, &buffer, sizeof(T), &bytesRead, NULL);
		if (!bResult) {
			throw exception("Read failed");
		}
		if (bytesRead!=sizeof(T)) {
			throw exception("Read failed");
		}
		return buffer;
	}

	/**
	* read from specified position
	*/
	T Read( LONG index) {
		DWORD dwResult = SetFilePointer( hFile,index,NULL,FILE_BEGIN);
		if (dwResult==INVALID_SET_FILE_POINTER) {
			throw exception("Unable to index file");
		}
		T buffer = 0;
		DWORD bytesRead = 0;
		BOOL bResult = ReadFile( hFile, &buffer, sizeof(T), &bytesRead, NULL);
		if (!bResult) {
			throw exception("Read failed");
		}
		if (bytesRead!=sizeof(T)) {
			throw exception("Read failed");
		}
		return buffer;
	}

	/**
	* write at current position
	*/
	void Write(T value) {
		DWORD bytesWritten = 0;
		BOOL bResult = WriteFile( hFile, &value, sizeof(T), &bytesWritten, NULL);
		if (!bResult) {
			throw exception("Write failed");
		}
		if (bytesWritten!=sizeof(T)) {
			throw exception("Write failed");
		}
	}

	/**
	* write at specified position
	*/
	void Write( size_t index, T value) {
		DWORD dwResult = SetFilePointer( hFile,index,NULL,FILE_BEGIN);
		if (dwResult==INVALID_SET_FILE_POINTER) {
			throw exception("Unable to index file");
		}
		DWORD bytesWritten = 0;
		BOOL bResult = WriteFile( hFile, &value, sizeof(T), &bytesWritten, NULL);
		if (!bResult) {
			throw exception("Write failed");
		}
		if (bytesWritten!=sizeof(T)) {
			throw exception("Write failed");
		}
	}

	/**
	* get file length
	*/
	LONG GetLength() {
		DWORD dwResult = GetFileSize( hFile, NULL);
		if (dwResult==INVALID_FILE_SIZE) {
			throw exception("GetLength failed");
		}
		return dwResult;
	}

private:
	HANDLE hFile;
	TCHAR filename[MAX_PATH];
};

#endif
