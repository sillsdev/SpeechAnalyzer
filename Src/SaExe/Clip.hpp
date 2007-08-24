#ifndef CLIPBOARDHPP
#define CLIPBOARDHPP

#include <windows.h>
#include <string.h>

extern "C"
{

  class Clipboard {

    HWND hWindow;
    int clipboard_open;
    void close();
    void reportError();
    char* string;
    unsigned int RTFFormat;

  public:

    Clipboard(HWND);

    Clipboard& operator <<(const char*);
    Clipboard& operator >>(char*&);
    int hasText();

    void SetTextRTF(const char* RTFstring, const char* TEXTstring);

    ~Clipboard() {close();}
  };

}

extern char *RTFprolog;
extern char *RTFepilog;

#endif
