#pragma once
#ifndef WBPROCESS_H
#define WBPROCESS_H

#include "sa_process.h"
#include "Context.h"

class CWbProcess : public CProcess {
public:
    CWbProcess(Context& context, WbDialogType _type) : CProcess(context) {
        type = _type;
    }
    CWbProcess() = delete;

    WbDialogType GetDialogType() {
        return type;
    }
protected:
    WbDialogType type;
};

#endif WBPROCESS_H
