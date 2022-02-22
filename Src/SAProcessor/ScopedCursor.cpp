#include "pch.h"
#include "ScopedCursor.h"

CScopedCursor::CScopedCursor(View & view) : view(view) {
    view.BeginWaitCursor();
}

CScopedCursor::~CScopedCursor() {
    view.EndWaitCursor();
}
