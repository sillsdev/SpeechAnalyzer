#pragma once

#include "AppDefs.h"
#include "ElanUtils.h"
#include "DlgImportElanSheet.h"
#include <map>

using std::map;

class CSAXMLUtils {
public:
    static void WriteSAXML(LPCTSTR filename, Elan::CAnnotationDocument & document, ElanMap & assignments);
};

