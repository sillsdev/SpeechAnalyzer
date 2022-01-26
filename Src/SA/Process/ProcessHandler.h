#pragma once

UINT Task(LPVOID pParam);

class ProcessHandler {
private:
    enum State {IDLE,RUNNING,COMPLETE,STOPPED};
    CWinThread* pThread = nullptr;
    State state = IDLE;

public:
    void checkState(bool active, bool cancelled);
    void markComplete();
};

