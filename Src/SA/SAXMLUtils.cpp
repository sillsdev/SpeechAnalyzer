#include "StdAfx.h"
#include <Windows.h>
#include "SAXMLUtils.h"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include "DlgImportElanSheet.h"
#include "Elan.h"

using namespace xercesc;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/**
* Returns the SAXML node name
*/
LPCTSTR GetAnnotationName(EAnnotation ea) {
    switch (ea) {
    case PHONETIC:
        return L"Phonetic";
    case PHONEMIC:
        return L"Phonemic";
    case GLOSS:
        return L"Gloss";
    case GLOSS_NAT:
        return L"GlossNat";
    case ORTHO:
        return L"Orthographic";
    case TONE:
        return L"Tone";
    case REFERENCE:
        return L"Reference";
    case MUSIC_PL1:
        return L"Annotation";
    case MUSIC_PL2:
        return L"Annotation";
    case MUSIC_PL3:
        return L"Annotation";
    case MUSIC_PL4:
        return L"Annotation";
    }
    throw logic_error("unsupported annotation type");
}

LPCTSTR GetPhraseLevel(EAnnotation ea) {
    switch (ea) {
    case MUSIC_PL1:
        return L"1";
    case MUSIC_PL2:
        return L"2";
    case MUSIC_PL3:
        return L"3";
    case MUSIC_PL4:
        return L"4";
    }
    throw logic_error("unsupported phrase annotation type");
}

//
//    <SegmentData IsBookmark="false" Offset="228.733625" Duration="4.5113854166666663" MarkDuration="4.5113854166666663">
//      <Phonetic></Phonetic>
//      <Gloss>widow</Gloss>
//      <Reference>178</Reference>
//    </SegmentData>
//
//   <MusicSegmentData PhraseLevel="1" Offset="5.8691041666666663" Duration="4.703135416666667">
//      <Annotation></Annotation>
//   </MusicSegmentData>

DOMElement * CreateElement(xercesc::DOMDocument * doc, EAnnotation ea, LPCTSTR value) {
    DOMElement * pElement = doc->createElement(GetAnnotationName(ea));
    pElement->appendChild(doc->createTextNode(value));
    return pElement;
}

bool IsGloss(EAnnotation ea) {
    if ((ea==GLOSS)||(ea==GLOSS_NAT)) {
        return true;
    }
    return false;
}

bool IsPhrase(EAnnotation ea) {
    if ((ea==MUSIC_PL1) || (ea==MUSIC_PL2) || (ea==MUSIC_PL3) || (ea==MUSIC_PL4)) {
        return true;
    }
    return false;
}

wstring GetFloat(double val) {
    char buffer[256];
    wchar_t wbuffer[256];
    memset(buffer,0,_countof(buffer));
    wmemset(wbuffer,0,_countof(wbuffer));
    sprintf_s(buffer,_countof(buffer),"%f",val);
    for (int i=0; i<_countof(buffer); i++) {
        wbuffer[i] = buffer[i];
    }
    wstring result = wbuffer;
    return result;
}

/**
* Writes a SAXML document
* Throws an int string resource ID on failure, or logic error
*/
void CSAXMLUtils::WriteSAXML(LPCTSTR filename, Elan::CAnnotationDocument & document, ElanMap & assignments) {
    ScopedXMLUtils utils;

    DOMImplementation * impl = DOMImplementationRegistry::getDOMImplementation(L"Core");
    if (impl == NULL) {
        throw logic_error("Unable to load xerces library");
    }

    try {
        xercesc::DOMDocument * doc = impl->createDocument(0, L"SaAudioDocument", 0);
        DOMElement * rootElem = doc->getDocumentElement();
        rootElem->setAttribute(L"DocVersion",L"2");
        DOMElement * pElement = doc->createElement(L"SpeakerGender");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"SAFlags");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"RecordFileFormat");
        pElement->appendChild(doc->createTextNode(L"-1"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"RecordTimeStamp");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"RecordBandWidth");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"RecordSampleSize");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"SignalMax");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"SignalMin");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"SignalBandWidth");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"SignalEffSampSize");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"CalcFreqLow");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"CalcFreqHigh");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"CalcVoicingThd");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"CalcPercntChng");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"CalcGrpSize");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);
        pElement = doc->createElement(L"CalcIntrpGap");
        pElement->appendChild(doc->createTextNode(L"0"));
        rootElem->appendChild(pElement);

        list<EAnnotation> stack;
        stack.push_back(PHONETIC);
        stack.push_back(TONE);
        stack.push_back(PHONEMIC);
        stack.push_back(ORTHO);
        stack.push_back(GLOSS);
        stack.push_back(GLOSS_NAT);
        stack.push_back(REFERENCE);

        pElement = doc->createElement(L"Segments");
        while (stack.size()>0) {
            EAnnotation type = stack.front();
            stack.pop_front();
            if (ProcessTier(type,stack,doc,*pElement,assignments,document,L"")) {
                break;
            }
        }
        rootElem->appendChild(pElement);

        // process independent phrase segments
        pElement = doc->createElement(L"MusicSegments");
        ElanMap::iterator it = assignments.begin();
        while (it!=assignments.end()) {
            if (IsPhrase(it->first)) {
                Elan::CTier * tier = FindTier(document,it->second);
                if (tier!=NULL) {
                    AddPhraseSegments(it->first,doc,*pElement,document,*tier);
                }
            }
            it++;
        }
        rootElem->appendChild(pElement);

        // write the document to disk
        DOMImplementation * pImplement = DOMImplementationRegistry::getDOMImplementation(L"LS");
        DOMLSSerializer * pSerializer = ((DOMImplementationLS *)pImplement)->createLSSerializer();
        DOMConfiguration * pDomConfiguration = pSerializer->getDomConfig();
        pDomConfiguration->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
        LocalFileFormatTarget target(filename);
        DOMLSOutput * pDomLsOutput = ((DOMImplementationLS *)pImplement)->createLSOutput();
        pDomLsOutput->setByteStream(&target);
        pSerializer->write(doc, pDomLsOutput);
        pSerializer->release();
        doc->release();
    } catch (const OutOfMemoryException &) {
        throw logic_error("out of memory");
    } catch (const DOMException & ) {
        throw logic_error("dom exception");
    } catch (...) {
        throw logic_error("unexpected exception");
    }
}

/**
* If this is the root node - (no parent ID) - then we will control the segment loop
* If not, then we only add one entry for the specified parent ID
*/
bool CSAXMLUtils::ProcessTier(EAnnotation type, list<EAnnotation> stack, xercesc::DOMDocument * doc, xercesc::DOMElement & parent, ElanMap & assignments, Elan::CAnnotationDocument & document, wstring parentID) {

    ElanMap::iterator it = assignments.find(type);
    if (it==assignments.end()) {
        return false;
    }

    Elan::CTier * tier = FindTier(document,it->second);
    if (tier==NULL) {
        return false;
    }

    if (type==PHONETIC) {
        if (parentID.size()!=0) {
            throw IDS_ERROR_ELAN1;
        }
        for (size_t j=0; j<tier->annotations.size(); j++) {
            DOMElement * pElement = doc->createElement(L"SegmentData");
            pElement->setAttribute(L"IsBookmark",L"false");
            Elan::CAnnotation & annotation = tier->annotations[j];
            wstring annotationID;
            if (annotation.alignableAnnotation.annotationID.size()>0) {
                pElement->appendChild(CreateElement(doc,type,annotation.alignableAnnotation.annotationValue.c_str()));
                double start = document.getTime(annotation.alignableAnnotation.timeSlotRef1.c_str());
                double end = document.getTime(annotation.alignableAnnotation.timeSlotRef2.c_str());
                start /= 1000.0;
                end /= 1000.0;
                wstring buffer = GetFloat(start);
                pElement->setAttribute(L"Offset",buffer.c_str());
                buffer = GetFloat(end-start);
                pElement->setAttribute(L"Duration",buffer.c_str());
                pElement->setAttribute(L"MarkDuration",L"0");
                annotationID = annotation.alignableAnnotation.annotationID;
            } else if (annotation.refAnnotation.annotationID.size()>0) {
                pElement->appendChild(CreateElement(doc,type,annotation.refAnnotation.annotationValue.c_str()));
                double start = document.getTime(annotation.refAnnotation,true);
                double end = document.getTime(annotation.refAnnotation,false);
                start /= 1000.0;
                end /= 1000.0;
                wstring buffer = GetFloat(start);
                pElement->setAttribute(L"Offset",buffer.c_str());
                buffer = GetFloat(end-start);
                pElement->setAttribute(L"Duration",buffer.c_str());
                pElement->setAttribute(L"MarkDuration",L"0");
                annotationID = annotation.refAnnotation.annotationID;
            }
            if (annotationID.size()>0) {
				for (EAnnotation ctype : stack) {
                    ProcessTier(ctype, stack, doc, *pElement, assignments, document, annotationID);
                }
            }
            parent.appendChild(pElement);
        }
    } else if (type==GLOSS) {
        if (parentID.size()==0) {
            for (size_t j=0; j<tier->annotations.size(); j++) {
                DOMElement * pElement = doc->createElement(L"SegmentData");
                pElement->setAttribute(L"IsBookmark",L"false");
                Elan::CAnnotation & annotation = tier->annotations[j];
                wstring annotationID;
                if (annotation.alignableAnnotation.annotationID.size()>0) {
                    pElement->appendChild(CreateElement(doc,type,annotation.alignableAnnotation.annotationValue.c_str()));
                    double start = document.getTime(annotation.alignableAnnotation.timeSlotRef1.c_str());
                    double end = document.getTime(annotation.alignableAnnotation.timeSlotRef2.c_str());
                    start /= 1000.0;
                    end /= 1000.0;
                    wstring buffer = GetFloat(start);
                    pElement->setAttribute(L"Offset",buffer.c_str());
                    buffer = GetFloat(end-start);
                    pElement->setAttribute(L"Duration",buffer.c_str());
                    pElement->setAttribute(L"MarkDuration",buffer.c_str());
                    pElement->appendChild(CreateElement(doc,PHONETIC,L"\xFFFD"));
                    annotationID = annotation.alignableAnnotation.annotationID;
                } else if (annotation.refAnnotation.annotationID.size()>0) {
                    pElement->appendChild(CreateElement(doc,type,annotation.refAnnotation.annotationValue.c_str()));
                    double start = document.getTime(annotation.refAnnotation,true);
                    double end = document.getTime(annotation.refAnnotation,false);
                    start /= 1000.0;
                    end /= 1000.0;
                    wstring buffer = GetFloat(start);
                    pElement->setAttribute(L"Offset",buffer.c_str());
                    buffer = GetFloat(end-start);
                    pElement->setAttribute(L"Duration",buffer.c_str());
                    pElement->setAttribute(L"MarkDuration",buffer.c_str());
                    pElement->appendChild(CreateElement(doc,PHONETIC,L"\xFFFD"));
                    annotationID = annotation.refAnnotation.annotationID;
                }
                if (annotationID.size()>0) {
					for (EAnnotation ctype : stack) {
						ProcessTier(ctype, stack, doc, *pElement, assignments, document, annotationID);
					}
                }
                parent.appendChild(pElement);
            }
        } else {
            // there is already a segment data section
            // we are nested within an existing segment
            for (size_t j=0; j<tier->annotations.size(); j++) {
                Elan::CAnnotation & annotation = tier->annotations[j];
                if (annotation.refAnnotation.annotationID.size()>0) {
                    if (_wcsicmp(annotation.refAnnotation.annotationRef.c_str(),parentID.c_str())==0) {
						double start = document.getTime(annotation.refAnnotation,true);
						double end = document.getTime(annotation.refAnnotation,false);
						start /= 1000.0;
						end /= 1000.0;
						wstring buffer = GetFloat(end-start);
						parent.setAttribute(L"MarkDuration",buffer.c_str());
						parent.appendChild(CreateElement(doc,type,tier->annotations[j].refAnnotation.annotationValue.c_str()));
						break;
                    }
                }
            }
        }
    } else {
        // if we are a dependent segment, and we have no parents, then we need to create a segment element
        if (parentID.size()==0) {
            for (size_t j=0; j<tier->annotations.size(); j++) {
                DOMElement * pElement = doc->createElement(L"SegmentData");
                pElement->setAttribute(L"IsBookmark",L"false");
                Elan::CAnnotation & annotation = tier->annotations[j];
                wstring annotationID;
                if (annotation.alignableAnnotation.annotationID.size()>0) {
                    pElement->appendChild(CreateElement(doc,type,annotation.alignableAnnotation.annotationValue.c_str()));
                    double start = document.getTime(annotation.alignableAnnotation.timeSlotRef1.c_str());
                    double end = document.getTime(annotation.alignableAnnotation.timeSlotRef2.c_str());
                    start /= 1000.0;
                    end /= 1000.0;
                    wstring buffer = GetFloat(start);
                    pElement->setAttribute(L"Offset",buffer.c_str());
                    buffer = GetFloat(end-start);
                    pElement->setAttribute(L"Duration",buffer.c_str());
                    pElement->setAttribute(L"MarkDuration",buffer.c_str());
                    pElement->appendChild(CreateElement(doc,type,annotation.alignableAnnotation.annotationValue.c_str()));
                    annotationID = annotation.alignableAnnotation.annotationID;
                } else if (annotation.refAnnotation.annotationID.size()>0) {
                    pElement->appendChild(CreateElement(doc,type,annotation.refAnnotation.annotationValue.c_str()));
                    double start = document.getTime(annotation.refAnnotation,true);
                    double end = document.getTime(annotation.refAnnotation,false);
                    start /= 1000.0;
                    end /= 1000.0;
                    wstring buffer = GetFloat(start);
                    pElement->setAttribute(L"Offset",buffer.c_str());
                    buffer = GetFloat(end-start);
                    pElement->setAttribute(L"Duration",buffer.c_str());
                    pElement->setAttribute(L"MarkDuration",buffer.c_str());
                    pElement->appendChild(CreateElement(doc,type,annotation.refAnnotation.annotationValue.c_str()));
                    annotationID = annotation.refAnnotation.annotationID;
                }
                pElement->appendChild(CreateElement(doc,PHONETIC,L"\xFFFD"));
                if (annotationID.size()>0) {
					for (EAnnotation ctype : stack) {
						ProcessTier(ctype, stack, doc, *pElement, assignments, document, annotationID);
					}
                }
                parent.appendChild(pElement);
            }
        } else {
            // we are nested within an existing segment
            // for all other segment types
            for (size_t j=0; j<tier->annotations.size(); j++) {
                Elan::CAnnotation & annotation = tier->annotations[j];
                if (annotation.refAnnotation.annotationID.size()>0) {
                    if (_wcsicmp(annotation.refAnnotation.annotationRef.c_str(),parentID.c_str())==0) {
                        parent.appendChild(CreateElement(doc,type,annotation.refAnnotation.annotationValue.c_str()));
						break;
                    }
                }
            }
        }
    }
    return true;
}

Elan::CTier * CSAXMLUtils::FindTier(Elan::CAnnotationDocument & document, wstring tierid) {
    for (size_t i=0; i<document.tiers.size(); i++) {
        if (_wcsicmp(document.tiers[i].tierID.c_str(),tierid.c_str())==0) {
            return &document.tiers[i];
        }
    }
    return NULL;
}

void CSAXMLUtils::AddPhraseSegments(EAnnotation atype,
                                    xercesc::DOMDocument * doc,
                                    DOMElement & element,
                                    Elan::CAnnotationDocument & document,
                                    Elan::CTier & tier) {

    for (size_t j=0; j<tier.annotations.size(); j++) {
        Elan::CAnnotation & annotation = tier.annotations[j];
        DOMElement * pSegData = doc->createElement(L"MusicSegmentData");
        pSegData->setAttribute(L"PhraseLevel",GetPhraseLevel(atype));
        if (annotation.alignableAnnotation.annotationID.size()>0) {
            pSegData->appendChild(CreateElement(doc,atype,annotation.alignableAnnotation.annotationValue.c_str()));
            double start = document.getTime(annotation.alignableAnnotation.timeSlotRef1.c_str());
            double end = document.getTime(annotation.alignableAnnotation.timeSlotRef2.c_str());
            start /= 1000.0;
            end /= 1000.0;
            wstring buffer = GetFloat(start);
            pSegData->setAttribute(L"Offset",buffer.c_str());
            buffer = GetFloat(end-start);
            pSegData->setAttribute(L"Duration",buffer.c_str());
        } else if (annotation.refAnnotation.annotationID.size()>0) {
            pSegData->appendChild(CreateElement(doc,atype,annotation.refAnnotation.annotationValue.c_str()));
            double start = document.getTime(annotation.refAnnotation,true);
            double end = document.getTime(annotation.refAnnotation,false);
            start /= 1000.0;
            end /= 1000.0;
            wstring buffer = GetFloat(start);
            pSegData->setAttribute(L"Offset",buffer.c_str());
            buffer = GetFloat(end-start);
            pSegData->setAttribute(L"Duration",buffer.c_str());
        }
        element.appendChild(pSegData);
    }
}



