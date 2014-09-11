#include "Lift13.h"
#include <Windows.h>
#include <iostream>
#include <memory>
#include <iostream>
#include <algorithm>
#include <map>
#include <FileUtils.h>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>

#include <uriparser/uri.h>

using std::auto_ptr;
using std::map;
using std::cout;
using std::cerr;
using std::endl;
using std::basic_string;

using namespace XML;
using namespace xercesc_3_1;
using namespace Lift13;

void toDOM(xercesc_3_1::DOMDocument * pDoc, DOMElement * pElement, Element * element) {

    AttributeList::iterator it = element->attributes.begin();
    while (it!=element->attributes.end()) {
        Attribute & attribute = *it;
        pElement->setAttribute(attribute.localname.c_str(),attribute.value.c_str());
        it++;
    }

    DOMText * pText = pDoc->createTextNode(element->value.c_str());
    pElement->appendChild(pText);

    ElementList::iterator it2 = element->elements.begin();
    while (it2!=element->elements.end()) {
        Element * child = *it2;
        DOMElement * pChild = pDoc->createElement(child->localname.c_str());
        toDOM(pDoc, pChild, child);
        pElement->appendChild(pChild);
        it2++;
    }
}

xercesc_3_1::DOMDocument * toDOM(Document & document) {

    DOMImplementation * impl = DOMImplementationRegistry::getDOMImplementation(L"Core");
    if (impl == NULL) {
        throw logic_error("Unable to load xerces library");
    }
    if (document.element==NULL) {
        throw logic_error("document element is empty");
    }
    xercesc_3_1::DOMDocument * pDoc = impl->createDocument(0, document.element->localname.c_str(), 0);
    DOMElement * pElement = pDoc->getDocumentElement();
    toDOM(pDoc, pElement, document.element);
    return pDoc;
}

void Lift13::write_document(Document & doc, LPCTSTR filename) {

    // convert to xerces DOM
    xercesc_3_1::DOMDocument * pDoc = toDOM(doc);

    // checking file existence
    FileUtils::RemoveFile(filename);

    printf("writing output\n");
    // write the document to disk
    DOMImplementation * pImplement = DOMImplementationRegistry::getDOMImplementation(L"LS");
    DOMLSSerializer * pSerializer = ((DOMImplementationLS *)pImplement)->createLSSerializer();
    DOMConfiguration * pDomConfiguration = pSerializer->getDomConfig();
    pDomConfiguration->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
    LocalFileFormatTarget target(filename);
    DOMLSOutput * pDomLsOutput = ((DOMImplementationLS *)pImplement)->createLSOutput();
    pDomLsOutput->setByteStream(&target);
    pSerializer->write(pDoc, pDomLsOutput);
    pSerializer->release();
    pDoc->release();
}

/**
* store the lift structure to filename
*/
void Lift13::store(lift & root, LPCTSTR filename) {

    try {
        ScopedXMLUtils utils;
        {
            wstring fn = filename;
            fn.append(L"-ranges");
            lift_ranges ranges = root.export_lift_ranges(fn.c_str());
            printf("generating tree\n");
            Document doc;
            doc.element = ranges.store();
            write_document(doc,fn.c_str());
        }

        {
            printf("generating tree\n");
            Document doc;
            doc.element = root.store();
            write_document(doc,filename);
        }
    } catch (exception & e) {
        printf("exception:%s\n",e.what());
        throw e;
    } catch (...) {
        printf("unexpected exception\n");
        throw;
    }

}

wstring parse_uri(LPCTSTR in) {

    UriParserStateW state;
    UriUriW uri;
    state.uri = &uri;
    if (uriParseUriW( &state, in) != URI_SUCCESS) {
        uriFreeUriMembersW(&uri);
		stringstream msg;
		msg << "unable to parse URI. position="<<state.errorPos<<", error="<<state.errorCode;
		throw logic_error(msg.str().c_str());
    }

	wstring result = uri.hostText.first;
    uriFreeUriMembersW(&uri);
	return result;
}

/**
* Read in filename and create a lift document
* NOTE: throws exception if error is detected
*/
lift Lift13::load(LPCTSTR filename) {

    lift document;
    try {
		if (!FileUtils::FileExists(filename)) {
			throw exception("file not found");
		}

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
        auto_ptr<SAX2XMLReader> parser(XMLReaderFactory::createXMLReader());
        //parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
        //parser->setFeature(XMLUni::fgXercesDynamic, false);
        //parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, doNamespaces);
        //parser->setFeature(XMLUni::fgXercesSchema, doSchema);
        //parser->setFeature(XMLUni::fgXercesHandleMultipleImports, true);
        //parser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);
        //parser->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, namespacePrefixes);

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
		if (errorCount!=0) {
			throw exception("document contained errors");
		}

		document.load(handler.document.element);

        // get the list of external files
        map<wstring,lift_ranges> hrefs = document.get_external_range_refs();

        // process the list
        printf("there are %d entries\n",hrefs.size());
        lift_range_map::iterator it = hrefs.begin();
        while (it!=hrefs.end()) {
            wstring href = (*it).first;
            wstring filename = parse_uri(href.c_str());
            wprintf(L"loading %s\n",filename.c_str());
            (*it).second = load_ranges(filename.c_str());
            printf("loaded\n");
            it++;
        }

        printf("applying %d ranges\n",hrefs.size());
        document.apply_lift_ranges(hrefs);

    } catch (const OutOfMemoryException &) {
        throw logic_error("out of memory");
    } catch (const XMLException &) {
        throw logic_error("xml exception occurred");
    } catch (logic_error & e) {
        printf("exception:%s\n",e.what());
        throw e;
    } catch (exception & e) {
        printf("exception:%s\n",e.what());
        throw e;
    } catch (...) {
        printf("unexpected exception\n");
        throw;
    }
    return document;
};

/**
* read in filename and create a lift document
*/
lift_ranges Lift13::load_ranges(LPCTSTR filename) {

    lift_ranges document;
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
        //parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
        //parser->setFeature(XMLUni::fgXercesDynamic, false);
        //parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, doNamespaces);
        //parser->setFeature(XMLUni::fgXercesSchema, doSchema);
        //parser->setFeature(XMLUni::fgXercesHandleMultipleImports, true);
        //parser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);
        //parser->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, namespacePrefixes);

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

    } catch (const OutOfMemoryException &) {
        throw logic_error("out of memory");
    } catch (const XMLException &) {
        throw logic_error("xml exception occurred");
    } catch (logic_error & e) {
        printf("exception:%s\n",e.what());
        throw e;
    } catch (exception & e) {
        printf("exception:%s\n",e.what());
        throw e;
    } catch (...) {
        printf("unexpected exception\n");
        throw;
    }
    return document;
};

