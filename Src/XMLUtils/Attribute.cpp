#include "Attribute.h"

namespace XML {

/*
* generic document nodes
*/

Attribute::Attribute(LPCTSTR _name, LPCTSTR _value) :
    localname(_name),
    qname(_name),
    type(),
    value(_value),
    uri() {
    //wprintf(L"creating attribute %s %d\n",_name,id);
};

Attribute::Attribute(LPCTSTR _localname, LPCTSTR _qname, LPCTSTR _type, LPCTSTR _value, LPCTSTR _uri) :
    localname(_localname),
    qname(_qname),
    type(_type),
    value(_value),
    uri(_uri)  {
    //wprintf(L"creating attribute %s %d\n",_localname,id);
};

}