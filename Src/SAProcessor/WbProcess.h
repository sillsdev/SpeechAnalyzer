#pragma once
#ifndef WBPROCESS_H
#define WBPROCESS_H

#include "Process.h"
#include "Context.h"

class CWbProcess : public CProcess {
public:
    CWbProcess(Context * pContext, WbDialogType _type) : CProcess(pContext) {
        type = _type;
    }
    WbDialogType GetDialogType() {
        return type;
    }
protected:
    WbDialogType type;
};

#endif WBPROCESS_H
