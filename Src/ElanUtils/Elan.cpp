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

using namespace xercesc;
using namespace Elan;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CAnnotationDocument Elan::LoadDocument(LPCTSTR filename) {

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

        size_t errorCount = 0;
        Handler handler;
        parser->setContentHandler(&handler);
        parser->setErrorHandler(&handler);
        parser->parse(filename);
        errorCount = parser->getErrorCount();
        document.load(*handler.document.element);

    } catch (const OutOfMemoryException &) {
        throw logic_error("out of memory");
    } catch (const XMLException &) {
        throw logic_error("xml exception occurred");
    } catch (logic_error & e) {
        throw e;
    }

    return document;
}

void CAnnotationDocument::load(Element & in) {
    expect(ANNOTATION_DOCUMENT,in);

    vector<Element *>::iterator it = in.elements.begin();
    while (it!=in.elements.end()) {
        Element & element = **it;
        if (element.localname.compare(HEADER)==0) {
            header = CHeader(element);
        } else if (element.localname.compare(TIME_ORDER)==0) {
            timeOrder = CTimeOrder(element);
        } else if (element.localname.compare(TIER)==0) {
            tiers.push_back(CTier(element));
        } else if (element.localname.compare(LINGUISTIC_TYPE)==0) {
            linguisticTypes.push_back(CLinguisticType(element));
        } else if (element.localname.compare(LOCALE)==0) {
            locales.push_back(CLocale(element));
        } else if (element.localname.compare(CONSTRAINT)==0) {
            constraints.push_back(CConstraint(element));
        } else if (element.localname.compare(CONTROLLED_VOCABULARY)==0) {
            controlledVocabularies.push_back(CControlledVocabulary(element));
        } else if (element.localname.compare(LEXICON_REF)==0) {
            lexiconRefs.push_back(CLexiconRef(element));
        } else if (element.localname.compare(EXTERNAL_REF)==0) {
            externalRefs.push_back(CExternalRef(element));
        } else {
            unexpected(ANNOTATION_DOCUMENT,element);
        }
        it++;
    }

    vector<Attribute>::iterator it2 = in.attributes.begin();
    while (it2!=in.attributes.end()) {
        Attribute & attribute = *it2;
        if (attribute.localname.compare(DATE)==0) {
            date.load(attribute);
        } else if (attribute.localname.compare(AUTHOR)==0) {
            author = attribute.value;
        } else if (attribute.localname.compare(VERSION)==0) {
            version = attribute.value;
        } else if (attribute.localname.compare(FORMAT)==0) {
            format = attribute.value;
        } else if (attribute.localname.compare(L"xsi")==0) {
            // skip
        } else if (attribute.localname.compare(L"noNamespaceSchemaLocation")==0) {
            // skip
        } else {
            unexpected(ANNOTATION_DOCUMENT,attribute);
        }
        it2++;
    }
};

unsigned int CAnnotationDocument::getTime(LPCTSTR timeSlotID) {
    for (size_t i=0; i<timeOrder.timeSlots.size(); i++) {
        CTimeSlot & timeSlot = timeOrder.timeSlots[i];
        if (_wcsicmp(timeSlot.timeSlotID.c_str(),timeSlotID)==0) {
            return timeSlot.timeValue;
        }
    }
    return 0;
}

unsigned int CAnnotationDocument::getTime(CRefAnnotation & annot, bool start) {
    for (size_t i=0; i<tiers.size(); i++) {
        Elan::CTier & tier = tiers[i];
        for (size_t j=0; j<tier.annotations.size(); j++) {
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

