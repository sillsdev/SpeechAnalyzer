#ifndef SCOPEDFILEPROTECTION_H
#define SCOPEDFILEPROTECTION_H

class CScopedFileProtection {
public:
	CScopedFileProtection(LPCTSTR filename);
	~CScopedFileProtection(void);
private:
	LPCTSTR filename;
	CFileStatus save;
	BOOL success;
};

#endif
