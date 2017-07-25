#include "Element.h"

#include <sstream>
#include <stdexcept>
#include "XML.h"

using std::stringstream;
using std::logic_error;

unsigned int seed = 0;

nodeid generateSeed() {
    return seed++;
}

namespace XML {

Element::Element() :
    owner(),
    id(generateSeed()) {
    //wprintf(L"creating element NULL %d\n",id);
};

Element::Element(LPCTSTR _localname, LPCTSTR _qname, LPCTSTR _uri) :
    localname(_localname),
    qname(_qname),
    uri(_uri),
    owner(),
    id(generateSeed())  {
    //wprintf(L"creating element %s %d\n",_localname,id);
}

Element::Element(LPCTSTR _name) :
    localname(_name),
    qname(_name),
    uri(),
    owner(),
    id(generateSeed())  {
    //wprintf(L"creating element %s %d\n",_name,id);
}

Element::Element(LPCTSTR _name, LPCTSTR _value) :
    localname(_name),
    qname(_name),
    uri(),
    value(_value),
    owner(),
    id(generateSeed())  {
    //wprintf(L"creating element %s %d\n",_name,id);
}

Element::~Element() {
    while (elements.size()>0) {
        Element * pThis = elements.back();
        elements.pop_back();
        delete pThis;
    }
}

void Element::addElement(Element * element) {
    elements.push_back(element);
}

// find a required attribute or element
bool Element::has(LPCTSTR name) {
    for (size_t i=0; i<attributes.size(); i++) {
        if (attributes[i].localname.compare(name)==0) {
            return true;
        }
    }
    return false;
};

void Element::removeElement(nodeid id) {
    printf("removing (%d) from %s\n",id, utf8(localname).c_str());
    vector<Element *>::iterator it = elements.begin();
    while (it!=elements.end()) {
        if ((*it)->id==id) {
            elements.erase(it);
            return;
        }
        it++;
    }
}

void Element::setAttribute(LPCTSTR name, LPCTSTR value) {
    attributes.push_back(Attribute(name,value));
}

void Element::appendChild(Element * element) {
	elements.push_back(element);
}

vector<nodeid> Element::findElements(LPCTSTR name) {
    vector<nodeid> result;
    for (size_t i=0; i<elements.size(); i++) {
        if (elements[i]->localname.compare(name)==0) {
            result.push_back(elements[i]->id);
        }
    }
    return result;
}

Element * Element::popElement(nodeid id) {
    vector<Element *>::iterator it = elements.begin();
    while (it!=elements.end()) {
        if ((*it)->id==id) {
            Element * result = *it;
            elements.erase(it);
            return result;
        }
        it++;
    }
    throw logic_error("expected node is missing");
}

Attribute Element::remove(LPCTSTR name) {
    AttributeList::iterator it = attributes.begin();
    while (it!=attributes.end()) {
        Attribute attribute = *it;
        if (attribute.localname.compare(name)==0) {
            attributes.erase(it);
            return attribute;
        }
        it++;
    }
    stringstream msg;
    msg << "cant find "<<utf8(name);
    throw logic_error(msg.str().c_str());
};

void Element::add(Attribute & attribute) {
    attributes.push_back(attribute);
}

size_t Element::elementCount() {
    return elements.size();
}

size_t Element::attributeCount() {
    return attributes.size();
}

Element * Element::last() {
    return elements.back();
}

Element * Element::get(size_t idx) {
    return elements[idx];
}

}
