#include "Document.h"
#include "XML.h"

using namespace xercesc;
#include <vector>

using std::vector;

namespace XML {

Document::Document() :
    element(NULL) {
}

Document::~Document() {
    if (element!=NULL) {
        delete element;
        element = NULL;
    }
}

}
