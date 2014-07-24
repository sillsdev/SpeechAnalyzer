#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <memory>
#include "XML.h"
#include <xercesc\util\PlatformUtils.hpp>
#include <xercesc\util\TransService.hpp>
#include <xercesc\util\OutOfMemoryException.hpp>
#include <xercesc\util\XMLUniDefs.hpp>
#include <xercesc\sax2\Attributes.hpp>
#include <xercesc\sax2\SAX2XMLReader.hpp>
#include <xercesc\sax2\XMLReaderFactory.hpp>
#include <xercesc\sax2\DefaultHandler.hpp>
#include <xercesc\framework\XMLFormatter.hpp>
#include <xercesc\dom\DOM.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;
using std::wstringstream;
using std::logic_error;
using std::wstring;
using std::auto_ptr;

using namespace xercesc_3_1;
using namespace XML;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace XML {

ScopedXMLUtils::ScopedXMLUtils() {
    XMLPlatformUtils::Initialize();
}

ScopedXMLUtils::~ScopedXMLUtils() {
    XMLPlatformUtils::Terminate();
}

}

Attribute factory(int index, const Attributes & attributes) {
    wstring localname = attributes.getLocalName(index);
    wstring qname = attributes.getLocalName(index);
    wstring type = attributes.getType(index);
    wstring value(attributes.getValue(index));
    wstring uri(attributes.getURI(index));
    return Attribute(localname.c_str(),qname.c_str(),type.c_str(),value.c_str(),uri.c_str());
}

string utf8(const wstring & in) {
    return utf8(in.c_str());
}

string utf8(LPCTSTR in) {
    if (in==NULL) {
        return "";
    }
    // remove the BOM
    int length2 = WideCharToMultiByte(CP_UTF8,0,in,wcslen(in),NULL,0,NULL,NULL);
    if (length2==0) {
        return "";
    }
    string result;
    result.resize(length2);
    length2 = WideCharToMultiByte(CP_UTF8,0,in, wcslen(in),&result[0],result.length(),NULL,NULL);
    if (length2==0) {
        return "";
    }
    return result;
}

wstring utf16(string & in) {
    return utf16(in.c_str());
}

wstring utf16(LPCSTR in) {
    int length2 = MultiByteToWideChar(CP_ACP,0,in,strlen(in),NULL,0);
    if (length2==0) {
        return false;
    }
    wstring result;
    result.resize(length2);
    length2 = MultiByteToWideChar(CP_ACP,0,in,strlen(in),&result[0],result.length());
    return result;
}

void restrict_elements(LPCTSTR name, Element & right) {
    if (right.elementCount()==0) {
        return;
    }
    stringstream msg;
    msg << "unexpected elements for "<<utf8(name);
    throw logic_error(msg.str().c_str());
}

void restrict_attributes(LPCTSTR name, Element & right) {
    if (right.attributeCount()==0) {
        return;
    }
    stringstream msg;
    msg << "unexpected attributes for "<<utf8(name);
    throw logic_error(msg.str().c_str());
}

void expect(LPCTSTR name, Element & element) {
    if (element.localname.compare(name)==0) {
        return;
    }
    stringstream msg;
    msg << "expected node : " << utf8(name).c_str() << " saw: " << utf8(element.localname.c_str()).c_str();
    throw logic_error(msg.str().c_str());
}

void unexpected(LPCTSTR name, Element & right) {
    stringstream msg;
    msg << "unexpected element in "<<utf8(name)<<" : "<<utf8(right.localname.c_str())<<" : "<<utf8(right.value.c_str());
    throw logic_error(msg.str().c_str());
}

void unexpected(LPCTSTR name, Attribute & right) {
    stringstream msg;
    msg << "unexpected attribute in "<<utf8(name)<<" : "<<utf8(right.localname.c_str())<<" : "<<utf8(right.value.c_str());
    throw logic_error(msg.str().c_str());
}

