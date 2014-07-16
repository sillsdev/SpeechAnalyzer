#include "Handler.h"
#include <iostream>
#include <stdexcept>
#include <string>

using std::string;
using std::wstring;
using std::logic_error;
using std::cout;
using std::cerr;
using std::endl;

extern string utf8( const wstring & in);

namespace XML {

Handler::Handler( ) :
current(NULL) {
	cout << "constructor" << endl;
}
Handler::~Handler() {
	cout << "destructor" << endl;
}

void Handler::startDocument() {
	cout << "startDocument" << endl;
}
void Handler::endDocument() {
	cout << "endDocument" << endl;
}

void Handler::startElement( const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attributes) {
	//cout << "startElement" << endl;
	Element * element = new Element( localname, qname, uri);
	element->owner = current;

	for (size_t i=0;i<attributes.getLength();i++) {
		element->add(Attribute(attributes.getLocalName(i),attributes.getQName(i),attributes.getType(i),attributes.getValue(i),attributes.getURI(i)));
	}

	if (current == NULL) {
		document.element = element;
		current = document.element;
	} else {
		current->addElement(element);
		current = current->last();
	}
}

void Handler::endElement( const XMLCh* const uri,const XMLCh* const localname,const XMLCh* const qname) {
	//cout << "endElement" << endl;
	current = current->owner;
}

void Handler::characters( const XMLCh* const chars, const XMLSize_t length) {
	//cout << "characters" << endl;
	if (current==NULL) {
		throw logic_error("unable to add characters to document node");
	}

	current->value.append(chars);
}

void Handler::ignorableWhitespace( const XMLCh* const chars, const XMLSize_t length) {
	//cout << "ignorableWhitespace" << endl;
}

void Handler::processingInstruction(const XMLCh* const target, const XMLCh* const data) {
	//cout << "processingInstruction" << endl;
}

void Handler::warning(const SAXParseException& exc) {
	wstring what = exc.getMessage();
	cout << "warning:" << utf8(what).c_str() << endl;
}

void Handler::error(const SAXParseException& exc) {
	wstring what = exc.getMessage();
	cout << "error:" << utf8(what).c_str() << endl;
}

void Handler::fatalError(const SAXParseException& exc) {
	cout << "fatalError" << endl;
}

void Handler::notationDecl(const XMLCh* const name, const XMLCh* const publicId, const XMLCh* const systemId) {
	cout << "notationDecl" << endl;
}

void Handler::unparsedEntityDecl(const XMLCh* const name, const XMLCh* const publicId, const XMLCh* const systemId, const XMLCh* const notationName) {
	cout << "unparsedEntityDecl" << endl;
}

}
