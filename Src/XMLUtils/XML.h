#ifndef XML_H
#define XML_H

#include "XMLDefs.h"
#include <Windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include "Attribute.h"
#include "Element.h"
#include "Document.h"
#include "Handler.h"

using std::logic_error;
using std::vector;
using std::string;
using std::wstring;
using std::stringstream;

namespace XML {

class ScopedXMLUtils {
public:
    ScopedXMLUtils();
    virtual ~ScopedXMLUtils();
};

}

// helper methods
extern void unexpected(LPCTSTR name, XML::Element & right);
extern void unexpected(LPCTSTR name, XML::Attribute & right);
extern wstring utf16(LPCSTR in);
extern string utf8(LPCTSTR in);
extern wstring utf16(string & in);
extern string utf8(const wstring & in);
extern void expect(LPCTSTR name, XML::Element & element);
extern void restrict_attributes(LPCTSTR name, XML::Element & element);
extern void restrict_elements(LPCTSTR name, XML::Element & element);

#endif
