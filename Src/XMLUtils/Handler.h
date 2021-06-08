#ifndef HANDLER_H
#define HANDLER_H

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include "Element.h"
#include "Document.h"

using namespace xercesc;

namespace XML {

class Handler : public DefaultHandler {
public:
    Handler();
    ~Handler();

    void startDocument();
    void endDocument();
    void startElement(const XMLCh * const uri, const XMLCh * const localname, const XMLCh * const qname, const Attributes & attributes);
    void endElement(const XMLCh * const uri,const XMLCh * const localname,const XMLCh * const qname);
    void characters(const XMLCh * const chars, const XMLSize_t length);
    void ignorableWhitespace(const XMLCh * const chars, const XMLSize_t length);
    void processingInstruction(const XMLCh * const target, const XMLCh * const data);
    void warning(const SAXParseException & exc);
    void error(const SAXParseException & exc);
    void fatalError(const SAXParseException & exc);
    void notationDecl(const XMLCh * const name, const XMLCh * const publicId, const XMLCh * const systemId);
    void unparsedEntityDecl(const XMLCh * const name, const XMLCh * const publicId, const XMLCh * const systemId, const XMLCh * const notationName);

    Element * current;
    Document document;
};

}
#endif
