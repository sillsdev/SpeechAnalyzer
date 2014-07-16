#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "XMLDefs.h"
#include <Windows.h>
#include <string>

using std::wstring;

namespace XML {

/*
* generic document nodes
*/

class Attribute {
public:
	Attribute( LPCTSTR _name, LPCTSTR _value);
	Attribute( LPCTSTR _localname, LPCTSTR _qname, LPCTSTR _type, LPCTSTR _value, LPCTSTR _uri);

	wstring localname;
	wstring qname;
	wstring type;
	wstring value;
	wstring uri;
};

}

#endif
