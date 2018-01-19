#pragma once

#include "XTaskWrapper.h"
#include <mutex>

class XTaskProcessor
{
private:
    XTaskWrapper _tasks[2];
    //int _taskIndex;
    //mutable std::mutex _taskMutex;
    uint64_t _taskCount;
public:
    XTaskProcessor();
    ~XTaskProcessor();

    //TODO: think of better process
    XTaskWrapper* GetNextTask();
    XTaskWrapper* GetCurrentTask();
    void SwitchTask();
    uint64_t GetCount() { return _taskCount; }

    void DumpTasks();
};
