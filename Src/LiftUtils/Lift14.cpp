#include "Lift14.h"
#include <Windows.h>
#include <iostream>

using std::auto_ptr;
using std::cout;
using std::cerr;
using std::endl;
using namespace XML;
using namespace xercesc;
using namespace Lift14;

/**
* store the lift structure to filename
*/
void Lift14::store(lift & root, LPCTSTR filename) {

}

/**
* read in filename and create a lift document
*/
lift Lift14::load(LPCTSTR filename) {

    lift document;
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

    } catch (const OutOfMemoryException &) {
        throw logic_error("out of memory");
    } catch (const XMLException &) {
        throw logic_error("xml exception occurred");
    } catch (logic_error & e) {
        throw e;
    } catch (...) {
        throw;
    }
    return document;
};

/**
* read in filename and create a lift document
*/
lift_ranges Lift14::load_ranges(LPCTSTR filename) {

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

    } catch (const OutOfMemoryException &) {
        throw logic_error("out of memory");
    } catch (const XMLException &) {
        throw logic_error("xml exception occurred");
    } catch (logic_error & e) {
        throw e;
    } catch (...) {
        throw;
    }
    return document;
};

