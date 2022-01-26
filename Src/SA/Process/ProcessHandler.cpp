#include "stdafx.h"
#include "processhandler.h"

/**
 * @brief Drive the state of the thread to the desired state specified by 'active'
 * @param active if true, the thread should be run and completed. if false, terminate a running thread
*/
void ProcessHandler::checkState( bool active, bool cancelled) {
    TRACE("checkState state=%d active=%d\n", state, active);
    switch (state) {
    case IDLE:
        if (active) {
            if (pThread == nullptr) {
                pThread = ::AfxBeginThread(Task, this);
                state = RUNNING;
            } else {
                // this is bad. it should be running
                state = RUNNING;
            }
        }
        break;
    case RUNNING:
        if (active) {
            // do nothing. it will transition when complete
        } else {
            if (pThread != nullptr) {
                BOOL bResult = ::GetExitCodeThread(pThread->m_hThread, 0);
                TRACE("thread returned %d\n", bResult);
                pThread = nullptr;
                state = COMPLETE;
            } else {
                // this is bad it should be running
                state = COMPLETE;
            }
        }
        break;
    case STOPPED: 
        if (active) {
            pThread = ::AfxBeginThread(Task, this);
            state = RUNNING;
        } else {
            // do nothing until the user decides to resume.
        }
        break;
    case COMPLETE:
        // the thread has completed. do nothing
        break;
    }
    TRACE("newState=%d\n", state);
}

void ProcessHandler::markComplete() {
    state = COMPLETE;
}

UINT Task(LPVOID pParam) {
    TRACE("do something %lp\n");
    ProcessHandler* instance = (ProcessHandler*)pParam;
    instance->markComplete();
    return 0;
}

