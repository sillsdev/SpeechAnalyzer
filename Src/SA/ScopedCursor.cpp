#include "Stdafx.h"
#include "ScopedCursor.h"

CScopedCursor::CScopedCursor(CCmdTarget * aParent) :
    parent(aParent) {
    parent->BeginWaitCursor();
}

CScopedCursor::~CScopedCursor() {
    parent->EndWaitCursor();
}
