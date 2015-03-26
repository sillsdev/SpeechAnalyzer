#pragma once

#include "AppDefs.h"
#include "ElanUtils.h"
#include "DlgImportElanSheet.h"
#include <map>
#include <xercesc/dom/DOM.hpp>

using std::map;

class CSAXMLUtils {
public:
    static void WriteSAXML(LPCTSTR filename, Elan::CAnnotationDocument & document, ElanMap & assignments);
	static bool ProcessTier( EAnnotation type, list<EAnnotation> stack, xercesc_3_1::DOMDocument * doc, xercesc_3_1::DOMElement & element, ElanMap & assignments, Elan::CAnnotationDocument & document, wstring parentID);
	static void AddPhraseSegments( EAnnotation atype, xercesc_3_1::DOMDocument * doc, xercesc_3_1::DOMElement & element, Elan::CAnnotationDocument & document,Elan::CTier & tier);
	static Elan::CTier * FindTier( Elan::CAnnotationDocument & document, wstring tierid);
};

