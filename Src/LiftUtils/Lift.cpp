#include "Lift.h"

#ifdef _DEBUG
#pragma comment( lib, "uriparserd")
#else
#pragma comment( lib, "uriparser")
#endif

void Lift::dumpChildren( Element & element) {
	if (element.elementCount()==0) return;
	//printf("dumpChildren children=%d\n",element.elementCount());
	for (size_t i=0;i<element.elementCount();i++) {
		printf("losing %s\n",utf8(element.get(i)->localname.c_str()).c_str());
	}
	if (element.elementCount()>0) {
		::DebugBreak();
	}
}

void Lift::removeElement( Element & parent, Element & element) {
	try {
		printf("removing %s (%lp) from %s (%lp)\n",utf8(element.localname).c_str(), element, utf8(parent.localname).c_str(), parent);
		dumpChildren(element);
		parent.removeElement(element.id);
	} catch(logic_error & e) {
		printf("failure:%s\n",e.what());
		throw e;
	} catch(...) {
		int i=0;
		throw;
	}
}

