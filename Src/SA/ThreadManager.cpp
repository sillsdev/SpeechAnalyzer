#include "stdafx.h"
#include "ThreadManager.h"
#include <memory>
#include <random>
#include <future>

void monitor(int (funcptr)(LPVOID), LPVOID params) {
    funcptr(params);
}

void CThreadManager::stop(thread_handle threadId) {
    std::lock_guard<std::mutex> lck(mutex);
}

bool CThreadManager::isComplete(thread_handle threadId) {
    std::lock_guard<std::mutex> lck(mutex);
    return false;
}

thread_handle CThreadManager::start(UINT(funcptr)(LPVOID), LPVOID params) {
    std::lock_guard<std::mutex> lck(mutex);
    shared_ptr<SThread> entry = make_shared<SThread>();
    entry->params = make_shared<LPVOID>(params);
    entry->future = async(funcptr,params);
    thread_handle id = seed++;
    threads[id] = entry;
    return id;
}

