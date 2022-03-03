#pragma once
#include <thread>
#include <mutex>
#include <memory>
#include <random>
#include <future>

using namespace std;
using thread_handle = long;

class CThreadManager {

    struct SThread {
        long id;
        future<UINT> future;
        shared_ptr<LPVOID> params;
    };

private:
    map<thread_handle,shared_ptr<SThread>> threads;
    mutex mutex;
    long seed = 0;

public:
    bool isComplete(thread_handle threadId);
    void stop(thread_handle threadId);
    thread_handle start(UINT (funcptr)(LPVOID), LPVOID params);
};

