#ifndef CLIPBOARDHPP
#define CLIPBOARDHPP

#include <windows.h>
#include <string.h>

extern "C" {

	class Clipboard {

		HWND hWindow;
		int clipboard_open;
		void close();
		void reportError();
		char * string;
		unsigned int RTFFormat;

	public:
		Clipboard(HWND);

		Clipboard & operator <<(LPCSTR);
		Clipboard & operator >>(char *&);
		int hasText();

		void SetTextRTF(LPCSTR RTFstring, LPCSTR TEXTstring);

		~Clipboard() {
			close();
		}
	};

}

extern char * RTFprolog;
extern char * RTFepilog;

#endif
