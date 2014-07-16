#include "Document.h"
#include "XML.h"

using namespace xercesc_3_1;
#include <vector>

using std::vector;

namespace XML {

Document::Document() {
}

Document::~Document() {
	if (element!=NULL) {
		delete element;
		element = NULL;
	}
}

}
