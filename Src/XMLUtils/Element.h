#ifndef ELEMENT_H
#define ELEMENT_H

#include "XMLDefs.h"
#include <Windows.h>
#include "Attribute.h"
#include <vector>
#include <map>

using std::map;
using std::vector;

namespace XML {

typedef vector<Attribute> AttributeList;
typedef vector<class Element*> ElementList;

class Element {
public:
	Element();
	Element( LPCTSTR name);
	Element( LPCTSTR localname, LPCTSTR qname, LPCTSTR uri);
	Element( LPCTSTR name, LPCTSTR value);
	~Element();

	// attribute operations
	// find the required attribute by name
	Attribute remove( LPCTSTR name);
	bool has( LPCTSTR name);
	void add( Attribute & attribute);
	void setAttribute( LPCTSTR name, LPCTSTR value);
	size_t attributeCount();

	// element operations
	Element * get( size_t idx);
	void addElement( Element * element);
	vector<nodeid> findElements( LPCTSTR name);
	Element * popElement( nodeid id);
	void appendChild( Element * element);
	void removeElement(nodeid id);
	size_t elementCount();
	Element * last();

	wstring uri;
	wstring localname;
	wstring qname;
	wstring value;
	nodeid id;

	Element * owner;

	ElementList elements;
	AttributeList attributes;
};

}

#endif
