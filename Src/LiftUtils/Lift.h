#ifndef LIFT_H
#define LIFT_H

#include <string>
#include <list>
#include <XMLUtils.h>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <assert.h>
#include "XMLUtils.h"

using std::stringstream;
using std::vector;
using std::list;
using std::wstring;
using std::exception;

using namespace XML;

namespace Lift {

typedef wstring url;
typedef wstring datetime;
typedef wstring key;
typedef wstring lang;
typedef wstring refid;

class multitext;

extern void removeElement( Element & parent, Element & element);
extern void dumpChildren( Element & element);

// node names
#define DATE_CREATED	L"dateCreated"
#define DATE_MODIFIED	L"dateModified"
#define SOURCE			L"source"
#define SPAN			L"span"
#define ANNOTATION		L"annotation"

template<class T> class required {
public:
	required( LPCTSTR _name) :
	data(_name),
	name(_name) {
	};
	T & get() {
		return data;
	}
	void set( T value) {
		data = value;
	}
	void load(Element * element) {
		data.load(element);
	}
	Element * store() {
		return data.store();
	}
	bool operator!=(const required<T> & right) const {
		if (_wcsicmp(name,right.name)==0) return false;
		if (data==right.data) return false;
		return true;
	}
	bool operator==(const required<T> & right) const {
		if (_wcsicmp(name,right.name)!=0) return false;
		if (data!=right.data) return false;
		return true;
	}

	LPCTSTR name;
private:
	T data;
};

template<class T> class optional {
public:
	optional( LPCTSTR _name) :
	name(_name) {
	};
	bool has_entry() {
		return (value.size()>0);
	}
	T & get() {
		return value[0];
	}
	void add( const optional<T> & right) {
		if (value.size()==0) {
			value = right.value;
			return;
		}
		// we already have something, do nothing
	}
	bool operator!=( const optional<T> & right) const {
		if (name==right.name) return false;
		if (value==right.value) return false;
		return true;
	}
	void remove() {
		value.clear();
	}
	void operator=( const T & right) {
		value.clear();
		value.push_back(right);
	}

	LPCTSTR name;
	vector<T> value;
};

template<class T> class zero_more {
public:
	zero_more( LPCTSTR _name) :
	name(_name) {
	};
	bool has_entries() {
		return (inner.size()>0);
	}
	T & get(int idx) {
		return inner[idx];
	}
	size_t size() const {
		return inner.size();
	}
	void append( T & right) {
		inner.push_back(right);
	}
	void append( vector<T> & right) {
		for (size_t i=0;i<right.size();i++) {
			inner.push_back(right[i]);
		}
	}
	void add( const zero_more<T> & right) {
		
		if (inner.size()==0) {
			inner = right.inner;
			return;
		}
		// we already have something?
		for (size_t j=0;j<right.inner.size();j++) {
			for (size_t i=0;i<inner.size();i++) {
				bool found = false;
				if (inner[i] == right.inner[j]) {
					found=true;
					break;
				}
				// if it wasn't in the list then just add it
				if (!found) {
					inner.push_back(right.inner[j]);
				}
			}
		}
	}

	bool operator!=( const zero_more<T> & right) const {
		if (name!=right.name) return true;
		if (inner!=right.inner) return true;
		return false;
	}
	void remove() {
		inner.clear();
	}

	LPCTSTR name;
	vector<T> inner;
};

template<class T> class one_more {
public:
	one_more( LPCTSTR _name) :
	name(_name) {
	};
	bool has_entries() {
		return (inner.size()>0);
	}
	LPCTSTR name;
	vector<T> inner;
};

class lift_base {
public:
	lift_base( LPCTSTR _name) :
	name(_name) {
	}

	virtual Element * store() = 0;
	virtual void load( Element * in) = 0;

	void expect( Element * in, LPCTSTR name) {
		if (in->localname.compare(name)!=0) {
			stringstream msg;
			msg << "expected "<<utf8(name).c_str()<<", saw "<< in->localname.c_str();
			throw logic_error(msg.str().c_str());
		}
	};
	

	void load_value( wstring & target, Element * parent) {
		target = parent->value;
	};
	

	//zero_more
	template<class T> void load_element( zero_more<T> & target, Element * parent) {
		vector<nodeid> children = parent->findElements( target.name);
		//printf("children=%d for %s\n",children.size(),utf8(parent->localname).c_str());
		for (size_t i=0;i<children.size();i++) {
			T t(target.name);
			t.load(parent->popElement(children[i]));
			target.inner.push_back(t);
		}
	};

	//one_more
	template<class T> void load_element( one_more<T> & target, Element * parent) {
		vector<nodeid> children = parent->findElements( target.name.c_str());
		//printf("children=%d for %s\n",children.size(),utf8(parent->localname).c_str());
		if (children.size()==0) {
			throw logic_error("required-multiple element is missing");
		}
		for (size_t i=0;i<children.size();i++) {
			T t(target.name.c_str());
			t.load(children[i]);
			target.inner.push_back(t);
			parent->removeElement(children[i]);
		}
	};


	// optional
	void load_attribute( optional<wstring> & target, Element * parent) {
		if (!parent->has(target.name)) {
			return;
		}
		assert(target.value.size()==0);
		Attribute attribute = parent->remove(target.name);
		target.value.push_back(attribute.value);
	};

	void load_attribute( optional<int> & target, Element * parent) {
		if (!parent->has(target.name)) {
			return;
		}
		assert(target.value.size()==0);
		Attribute attribute = parent->remove( target.name);
		target.value.push_back(_wtoi(attribute.value.c_str()));
	};

	template<class T> void load_element( optional<T> & target, Element * parent) {
		vector<nodeid> children = parent->findElements( target.name);
		if (children.size()==0) {
			return;
		}
		if (children.size()>1) {
			throw logic_error("contained multiple elements. expected one");
		}
		//printf("children=%d for %s\n",children.size(),utf8(parent->localname).c_str());
		assert(target.value.size()==0);
		T t(target.name);
		t.load(parent->popElement(children[0]));
		target.value.push_back(t);
	};

	template<> void load_element<wstring>( optional<wstring> & target, Element * parent) {
		vector<nodeid> children = parent->findElements( target.name);
		if (children.size()==0) {
			return;
		}
		if (children.size()>1) {
			throw logic_error("document has multiple entries. expected one");
		}
		assert( target.value.size()==0);
		Element * element = parent->popElement(children[0]);
		target.value.push_back(element->value);
	};


	// required
	void load_attribute( required<wstring> & target, Element * parent) {
		if (!parent->has(target.name)) {
			throw logic_error("required attribute is missing");
		}
		Attribute attribute = parent->remove( target.name);
		target.set(attribute.value);
	};

	template<class T> void load_element( required<T> & target, Element * parent) {
		if (parent->elementCount()==0) {
			throw logic_error("required element is missing");
		}
		target.load( parent->get(0));
	};


	// required
	template<class T> void store_attribute( required<T> & target, Element * parent) {
		parent->getAttributes().push_back( target.inner.store());
	}
	template<> void store_attribute<wstring>( required<wstring> & target, Element * parent) {
		parent->setAttribute( target.name, target.get().c_str());
	}

	template<class T> void store_element( required<T> & target, Element * parent) {
		parent->appendChild( target.store());
	}


	// optional
	template<class T> void store_attribute( optional<T> & target, Element * parent) {
		if (target.inner.size()==0) return;
		parent->getAttributes().push_back( target.inner[0].store());
	}
	template<> void store_attribute<wstring>( optional<wstring> & target, Element * parent) {
		if (target.value.size()==0) return;
		parent->setAttribute( target.name, target.value[0].c_str());
	}
	template<> void store_attribute<int>( optional<int> & target, Element * parent) {
		if (target.value.size()==0) return;
		wchar_t buffer[128];
		wmemset(buffer,0,_countof(buffer));
		_itow_s(target.value[0],buffer,_countof(buffer),10);
		parent->setAttribute( target.name, buffer);
	}
	template<class T> void store_element( optional<T> & target, Element * parent) {
		if (target.value.size()==0) return;
		Element * e = target.value[0].store();
		parent->appendChild( e);
	}
	template<> void store_element<wstring>( optional<wstring> & target, Element * parent) {
		if (target.value.size()==0) return;
		Element * element = new Element(target.name);
		element->value = target.value[0].c_str();
		parent->appendChild( element);
	}


	// zero_more
	template<class T> void store_attribute( zero_more<T> & target, Element * parent) {
		if (target.inner.size()==0) return;
		for (int i=0;i<target.inner.size();i++) {
			parent->getAttributes().push_back(target.inner[i].store());
		}
	}
	template<> void store_attribute<wstring>( zero_more<wstring> & target, Element * parent) {
		if (target.inner.size()==0) return;
		for (size_t i=0;i<target.inner.size();i++) {
			parent->setAttribute(target.name,target.inner[i].c_str());
		}
	}
	template<class T> void store_element( zero_more<T> & target, Element * parent) {
		if (target.inner.size()==0) return;
		for (size_t i=0;i<target.inner.size();i++) {
			Element * element = target.inner[i].store();
			parent->appendChild(element);
		}
	}
	template<> void store_element<wstring>( zero_more<wstring> & target, Element * parent) {
		if (target.inner.size()==0) return;
		for (size_t i=0;i<target.inner.size();i++) {
			Element * element = new Element(target.name);
			element->value = target.inner[i].c_str();
			parent->appendChild(element);
		}
	}


	// zero_more
	template<class T> void store_attribute( one_more<T> & target, Element * parent) {
		if (target.inner.size()==0) throw logic_error("required-multiple element is empty");
		for (int i=0;i<target.inner.size();i++) {
			parent->getAttributes().push_back(target.inner[i].store());
		}
	}
	template<> void store_attribute<wstring>( one_more<wstring> & target, Element * parent) {
		if (target.inner.size()==0) throw logic_error("required-multiple element is empty");
		for (size_t i=0;i<target.inner.size();i++) {
			parent->setAttribute(target.name,target.inner[i].c_str());
		}
	}
	template<class T> void store_element( one_more<T> & target, Element * parent) {
		if (target.inner.size()==0) throw logic_error("required-multiple element is empty");
		for (size_t i=0;i<target.inner.size();i++) {
			Element * element = target.inner[i].store(doc);
			parent->appendChild(element);
		}
	}
	template<> void store_element<wstring>( one_more<wstring> & target, Element * parent) {
		if (target.inner.size()==0) throw logic_error("required-multiple element is empty");
		for (size_t i=0;i<target.inner.size();i++) {
			Element * element = new Element(target.name);
			element->value = target.inner[i].c_str();
			parent->appendChild(element);
		}
	}



	//value
	void store_value( wstring & target, Element * parent) {
		parent->value = target.c_str();
	};

	LPCTSTR name;
};

}
#endif
