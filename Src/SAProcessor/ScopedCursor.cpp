#include "pch.h"
#include "ScopedCursor.h"

CScopedCursor::CScopedCursor(CmdTarget & cmdTarget) : cmdTarget(cmdTarget) {
    cmdTarget.BeginWaitCursor();
}

CScopedCursor::~CScopedCursor() {
    cmdTarget.EndWaitCursor();
}
