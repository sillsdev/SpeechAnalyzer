#include "StdAfx.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <memory>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax2/Attributes.hpp>

#include "Elan.h"

using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;
using std::wstringstream;
using std::logic_error;
using std::wstring;
using std::auto_ptr;

using namespace xercesc_3_1;
using namespace Elan;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

Attribute Factory( int index, const Attributes & attributes) {

	wstring localname = attributes.getLocalName(index);
	wstring qname = attributes.getLocalName(index);
	wstring type = attributes.getType(index);
	wstring value(attributes.getValue(index));
	wstring uri(attributes.getURI(index));
	Attribute attribute( localname.c_str(),qname.c_str(),type.c_str(),value.c_str(),uri.c_str());
	return attribute;
}

class Handler : public DefaultHandler
{
public:
    Handler ( ) :
	current(NULL) {
		cout << "constructor" << endl;
	}
    ~Handler() {
		cout << "destructor" << endl;
	}

    void startDocument() {
		cout << "startDocument" << endl;
	}
    void endDocument() {
		cout << "endDocument" << endl;
	}

    void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attributes) {
		//cout << "startElement" << endl;
		Element element( localname, qname, uri);
		element.owner = current;

		for (int i=0;i<attributes.getLength();i++) {
			Attribute attribute(attributes.getLocalName(i),attributes.getQName(i),attributes.getType(i),attributes.getValue(i),attributes.getURI(i));
			element.attributes.push_back(attribute);
		}

		if (current == NULL) {
			document.element = element;
			current = &document.element;
		} else {
			current->elements.push_back(element);
			current = &current->elements.back();
		}
	}

    void endElement( const XMLCh* const uri,const XMLCh* const localname,const XMLCh* const qname) {
		//cout << "endElement" << endl;
		current = current->owner;
	}

    void characters( const XMLCh* const chars, const XMLSize_t length) {
		//cout << "characters" << endl;
		if (current==NULL) {
			throw logic_error("unable to add characters to document node");
		}

		current->value.append(chars);
	}

    void ignorableWhitespace( const XMLCh* const chars, const XMLSize_t length) {
		//cout << "ignorableWhitespace" << endl;
	}

    void processingInstruction(const XMLCh* const target, const XMLCh* const data) {
		//cout << "processingInstruction" << endl;
	}

    void warning(const SAXParseException& exc) {
		cout << "warning" << endl;
	}

    void error(const SAXParseException& exc) {
		cout << "error" << endl;
	}

    void fatalError(const SAXParseException& exc) {
		cout << "fatalError" << endl;
	}

    void notationDecl(const XMLCh* const name, const XMLCh* const publicId, const XMLCh* const systemId) {
		cout << "notationDecl" << endl;
	}

    void unparsedEntityDecl(const XMLCh* const name, const XMLCh* const publicId, const XMLCh* const systemId, const XMLCh* const notationName) {
		cout << "unparsedEntityDecl" << endl;
	}

	Element * current;
	Document document;
};

class ScopedXMLUtils {
public:
	ScopedXMLUtils() {
         XMLPlatformUtils::Initialize();
	}
	virtual ~ScopedXMLUtils() {
	    XMLPlatformUtils::Terminate();
	}
};

CAnnotationDocument Elan::LoadDocument( LPCTSTR filename) {

	CAnnotationDocument document;
    try {
		// Initialize the XML4C2 system
		ScopedXMLUtils xmlUtils;

		bool doNamespaces = true;
		bool doSchema = true;
		bool schemaFullChecking = true;
		bool namespacePrefixes = true;
		LPCSTR encodingName = "";
		XMLFormatter::UnRepFlags unRepFlags = XMLFormatter::UnRep_Fail;
		bool expandNamespaces = true;

		//
		//  Create a SAX parser object. Then, according to what we were told on
		//  the command line, set it to validate or not.
		//
		auto_ptr<SAX2XMLReader> parser(XMLReaderFactory::createXMLReader());
		parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
		parser->setFeature(XMLUni::fgXercesDynamic, false);
		parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, doNamespaces);
		parser->setFeature(XMLUni::fgXercesSchema, doSchema);
		parser->setFeature(XMLUni::fgXercesHandleMultipleImports, true);
		parser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);
		parser->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, namespacePrefixes);

		//
		//  Create the handler object and install it as the document and error
		//  handler for the parser. Then parse the file and catch any exceptions
		//  that propogate out
		//

		int errorCount = 0;
        Handler handler;
        parser->setContentHandler(&handler);
        parser->setErrorHandler(&handler);
        parser->parse(filename);
        errorCount = parser->getErrorCount();
		document.load(handler.document.element);

    } catch (const OutOfMemoryException & e) {
		throw logic_error("out of memory");
    } catch (const XMLException & e) {
		throw logic_error("xml exception occurred");
    } catch (logic_error & e) {
		throw e;
	}

	return document;
}

string utf8( LPCTSTR in) {

    std::string out;
    unsigned int codepoint = 0;
    for ( in;  *in != 0;  ++in)
    {
        if (*in >= 0xd800 && *in <= 0xdbff)
            codepoint = ((*in - 0xd800) << 10) + 0x10000;
        else
        {
            if (*in >= 0xdc00 && *in <= 0xdfff)
                codepoint |= *in - 0xdc00;
            else
                codepoint = *in;

            if (codepoint <= 0x7f)
                out.append(1, static_cast<char>(codepoint));
            else if (codepoint <= 0x7ff)
            {
                out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
                out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
            }
            else if (codepoint <= 0xffff)
            {
                out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
                out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
                out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
            }
            else
            {
                out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
                out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
                out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
                out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
            }
            codepoint = 0;
        }
    }
    return out;
}

void restrict_elements( LPCTSTR name, Element & right) {
	if (right.elements.size()==0) return;
	stringstream msg;
	msg << "unexpected elements for "<<utf8(name);
	throw logic_error(msg.str().c_str());
}

void restrict_attributes( LPCTSTR name, Element & right) {
	if (right.attributes.size()==0) return;
	stringstream msg;
	msg << "unexpected attributes for "<<utf8(name);
	throw logic_error(msg.str().c_str());
}

void expect( LPCTSTR name, Elan::Element & element) {
	if (element.localname.compare(name)==0) return;
	stringstream msg;
	msg << "expected node : " << utf8(name).c_str() << " saw: " << utf8( element.localname.c_str()).c_str();
	throw logic_error(msg.str().c_str());
}

void unexpected( LPCTSTR name, Element & right) {
	stringstream msg;
	msg << "unexpected element in "<<utf8(name)<<" : "<<utf8(right.localname.c_str())<<" : "<<utf8(right.value.c_str());
	throw logic_error(msg.str().c_str());
}

void unexpected( LPCTSTR name, Attribute & right) {
	stringstream msg;
	msg << "unexpected attribute in "<<utf8(name)<<" : "<<utf8(right.localname.c_str())<<" : "<<utf8(right.value.c_str());
	throw logic_error(msg.str().c_str());
}

Element::Element() :
owner(NULL) {
};

Element::Element( LPCTSTR _localname, LPCTSTR _qname, LPCTSTR _uri) :
localname(_localname),
qname(_qname),
uri(_uri),
owner(NULL) {
}

void CAnnotationDocument::load(Element & element) {
	expect(ANNOTATION_DOCUMENT,element);

	vector<Element>::iterator it = element.elements.begin();
	while (it!=element.elements.end()) {
		if (it->localname.compare(HEADER)==0) {
			header = CHeader(*it);
		} else if (it->localname.compare(TIME_ORDER)==0) {
			timeOrder = CTimeOrder(*it);
		} else if (it->localname.compare(TIER)==0) {
			tiers.push_back(CTier(*it));
		} else if (it->localname.compare(LINGUISTIC_TYPE)==0) {
			linguisticTypes.push_back(CLinguisticType(*it));
		} else if (it->localname.compare(LOCALE)==0) {
			locales.push_back(CLocale(*it));
		} else if (it->localname.compare(CONSTRAINT)==0) {
			constraints.push_back(CConstraint(*it));
		} else if (it->localname.compare(CONTROLLED_VOCABULARY)==0) {
			controlledVocabularies.push_back(CControlledVocabulary(*it));
		} else if (it->localname.compare(LEXICON_REF)==0) {
			lexiconRefs.push_back(CLexiconRef(*it));
		} else if (it->localname.compare(EXTERNAL_REF)==0) {
			externalRefs.push_back(CExternalRef(*it));
		} else {
			unexpected(ANNOTATION_DOCUMENT,*it);
		}
		it++;
	}

	vector<Attribute>::iterator it2 = element.attributes.begin();
	while (it2!=element.attributes.end()) {
		if (it2->localname.compare(DATE)==0) {
			date.load(*it2);
		} else if (it2->localname.compare(AUTHOR)==0) {
			author = it2->value;
		} else if (it2->localname.compare(VERSION)==0) {
			version = it2->value;
		} else if (it2->localname.compare(FORMAT)==0) {
			format = it2->value;
		} else if (it2->localname.compare(L"xsi")==0) {
			// skip
		} else if (it2->localname.compare(L"noNamespaceSchemaLocation")==0) {
			// skip
		} else {
			unexpected(ANNOTATION_DOCUMENT,*it2);
		}
		it2++;
	}
};

unsigned int CAnnotationDocument::getTime( LPCTSTR timeSlotID) {
	for (int i=0;i<timeOrder.timeSlots.size();i++) {
		CTimeSlot & timeSlot = timeOrder.timeSlots[i];
		if (_wcsicmp(timeSlot.timeSlotID.c_str(),timeSlotID)==0) {
			return timeSlot.timeValue;
		}
	}
	return 0;
}

unsigned int CAnnotationDocument::getTime(CRefAnnotation & annot, bool start) {
	for (int i=0;i<tiers.size();i++) {
		Elan::CTier & tier = tiers[i];
		for (int j=0;j<tier.annotations.size();j++) {
			Elan::CAnnotation & annotation = tier.annotations[j];
			if (annotation.alignableAnnotation.annotationID.length()==0) {
				continue;
			}
			if (_wcsicmp(annotation.alignableAnnotation.annotationID.c_str(),annot.annotationRef.c_str())!=0) {
				continue;
			}
			if (start) {
				return getTime(annotation.alignableAnnotation.timeSlotRef1.c_str());
			} else {
				return getTime(annotation.alignableAnnotation.timeSlotRef2.c_str());
			}
		}
	}
	return 0;
}

