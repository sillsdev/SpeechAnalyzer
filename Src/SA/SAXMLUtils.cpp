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

using namespace xercesc_3_1;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

LPCTSTR GetAnnotationName( EAnnotation ea) {
	switch (ea) {
		case PHONETIC:	return L"Phonetic";
		case PHONEMIC:	return L"Phonemic";
		case GLOSS:		return L"Gloss";
		case GLOSS_NAT:	return L"Gloss Nat.";
		case ORTHO:		return L"Ortho";
		case TONE:		return L"Tone";
		case REFERENCE: return L"Reference";
		case MUSIC_PL1: return L"Annotation";
		case MUSIC_PL2: return L"Annotation";
		case MUSIC_PL3: return L"Annotation";
		case MUSIC_PL4: return L"Annotation";
	}
	throw logic_error("unsupported annotation type");
}

LPCTSTR GetPhraseLevel( EAnnotation ea) {
	switch (ea) {
	case MUSIC_PL1: return L"1";
	case MUSIC_PL2: return L"2";
	case MUSIC_PL3: return L"3";
	case MUSIC_PL4: return L"4";
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
 
DOMElement * CreateElement( xercesc_3_1::DOMDocument * doc, EAnnotation ea, LPCTSTR value) {
	DOMElement * pElement = doc->createElement( GetAnnotationName(ea));
	pElement->appendChild(doc->createTextNode(value));
	return pElement;
}

bool IsPhrase(EAnnotation ea) {
	if ((ea==MUSIC_PL1) || (ea==MUSIC_PL2) || (ea==MUSIC_PL3) || (ea==MUSIC_PL4)) return true;
	return false;
}

wstring GetFloat( double val) {
	char buffer[256];
	wchar_t wbuffer[256];
	memset(buffer,0,_countof(buffer));
	wmemset(wbuffer,0,_countof(wbuffer));
	sprintf_s( buffer,_countof(buffer),"%f",val);
	for (int i=0;i<_countof(buffer);i++) {
		wbuffer[i] = buffer[i];
	}
	wstring result = wbuffer;
	return result;
}

/**
* Writes a SAXML document
*/
void CSAXMLUtils::WriteSAXML( LPCTSTR filename, Elan::CAnnotationDocument & document, ElanMap & assignments) 
{
	ScopedXMLUtils utils;

    DOMImplementation * impl = DOMImplementationRegistry::getDOMImplementation(L"Core");
    if (impl == NULL) throw logic_error("Unable to load xerces library");

	try 
	{
		xercesc_3_1::DOMDocument * doc = impl->createDocument( 0, L"SaAudioDocument", 0);
		DOMElement* rootElem = doc->getDocumentElement();
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

		pElement = doc->createElement(L"Segments");
		ElanMap::iterator it = assignments.begin();
		while (it!=assignments.end()) {
			if (IsPhrase(it->first)) {
				it++;
				continue;
			}
			for (int i=0;i<document.tiers.size();i++) {
				Elan::CTier & tier = document.tiers[i];
				if (_wcsicmp(tier.tierID.c_str(),it->second.c_str())==0) {
					for (int j=0;j<tier.annotations.size();j++) {
						DOMElement * pSegData = doc->createElement(L"SegmentData");
						pSegData->setAttribute(L"IsBookmark",L"false");
						Elan::CAnnotation & annotation = tier.annotations[j];
						if (annotation.alignableAnnotation.annotationID.size()>0) {
							pSegData->appendChild(CreateElement(doc,it->first,annotation.alignableAnnotation.annotationValue.c_str()));
							double start = document.getTime(annotation.alignableAnnotation.timeSlotRef1.c_str());
							double end = document.getTime(annotation.alignableAnnotation.timeSlotRef2.c_str());
							start /= 1000.0;
							end /= 1000.0;
							wstring buffer = GetFloat(start);
							pSegData->setAttribute(L"Offset",buffer.c_str());
							buffer = GetFloat(end-start);
							pSegData->setAttribute(L"Duration",buffer.c_str());
							pSegData->setAttribute(L"MarkDuration",GetFloat(start).c_str());
						} else if (annotation.refAnnotation.annotationID.size()>0) {
							pSegData->appendChild(CreateElement(doc,it->first,annotation.refAnnotation.annotationValue.c_str()));
							double start = document.getTime(annotation.refAnnotation,true);
							double end = document.getTime(annotation.refAnnotation,false);
							start /= 1000.0;
							end /= 1000.0;
							wstring buffer = GetFloat(start);
							pSegData->setAttribute(L"Offset",buffer.c_str());
							buffer = GetFloat(end-start);
							pSegData->setAttribute(L"Duration",buffer.c_str());
							pSegData->setAttribute(L"MarkDuration",GetFloat(start).c_str());
						}
						pElement->appendChild(pSegData);
					}
				}
			}
			it++;
		}
		rootElem->appendChild(pElement);
		
		pElement = doc->createElement(L"MusicSegments");
		it = assignments.begin();
		while (it!=assignments.end()) {
			if (!IsPhrase(it->first)) {
				it++;
				continue;
			}
			for (int i=0;i<document.tiers.size();i++) {
				Elan::CTier & tier = document.tiers[i];
				if (_wcsicmp(tier.tierID.c_str(),it->second.c_str())==0) {
					for (int j=0;j<tier.annotations.size();j++) {
						Elan::CAnnotation & annotation = tier.annotations[j];
						DOMElement * pSegData = doc->createElement(L"MusicSegmentData");
						pSegData->setAttribute(L"PhraseLevel",GetPhraseLevel(it->first));
						if (annotation.alignableAnnotation.annotationID.size()>0) {
							pSegData->appendChild(CreateElement(doc,it->first,annotation.alignableAnnotation.annotationValue.c_str()));
							double start = document.getTime(annotation.alignableAnnotation.timeSlotRef1.c_str());
							double end = document.getTime(annotation.alignableAnnotation.timeSlotRef2.c_str());
							start /= 1000.0;
							end /= 1000.0;
							wstring buffer = GetFloat(start);
							pSegData->setAttribute(L"Offset",buffer.c_str());
							buffer = GetFloat(end-start);
							pSegData->setAttribute(L"Duration",buffer.c_str());
						} else if (annotation.refAnnotation.annotationID.size()>0) {
							pSegData->appendChild(CreateElement(doc,it->first,annotation.refAnnotation.annotationValue.c_str()));
							double start = document.getTime(annotation.refAnnotation,true);
							double end = document.getTime(annotation.refAnnotation,false);
							start /= 1000.0;
							end /= 1000.0;
							wstring buffer = GetFloat(start);
							pSegData->setAttribute(L"Offset",buffer.c_str());
							buffer = GetFloat(end-start);
							pSegData->setAttribute(L"Duration",buffer.c_str());
						}
						pElement->appendChild(pSegData);
					}
				}
			}
			it++;
		}
		rootElem->appendChild(pElement);

		// write the document to disk
		DOMImplementation * pImplement = DOMImplementationRegistry::getDOMImplementation(L"LS");
		DOMLSSerializer * pSerializer = ((DOMImplementationLS*)pImplement)->createLSSerializer();
		DOMConfiguration* pDomConfiguration = pSerializer->getDomConfig();
		pDomConfiguration->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
		LocalFileFormatTarget target(filename);
		DOMLSOutput* pDomLsOutput = ((DOMImplementationLS*)pImplement)->createLSOutput();
		pDomLsOutput->setByteStream(&target);
		pSerializer->write(doc, pDomLsOutput);
		pSerializer->release();
		doc->release();
	}
	catch (const OutOfMemoryException&)
	{
		throw logic_error("out of memory");
	}
	catch (const DOMException& e)
	{
		throw logic_error("dom exception");
	}
	catch (...)
	{
		throw logic_error("unexpected exception");
	}
}

