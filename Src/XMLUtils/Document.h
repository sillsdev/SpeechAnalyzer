#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "Element.h"

namespace XML {

class Document {
public:
	Document();
	~Document();
	Element * element;
};

}

#endif
