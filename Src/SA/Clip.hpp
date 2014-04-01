#ifndef CLIPBOARDHPP
#define CLIPBOARDHPP

#include <windows.h>
#include <string.h>

extern "C" {

	class CClipboard {

		HWND hWindow;
		int clipboard_open;
		void close();
		void reportError();
		char * string;
		unsigned int RTFFormat;

	public:
		CClipboard(HWND);
		~CClipboard();

		CClipboard & operator <<(LPCSTR);
		CClipboard & operator >>(char *&);
		int hasText();
		void SetTextRTF(LPCSTR RTFstring, LPCSTR TEXTstring);
	};

}

extern char * RTFprolog;
extern char * RTFepilog;

#endif
