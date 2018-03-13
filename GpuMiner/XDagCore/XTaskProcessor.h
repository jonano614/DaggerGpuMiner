#pragma once

#include "XTaskWrapper.h"
#include "Core/Guards.h"

class XTaskProcessor
{
private:
    XTaskWrapper _tasks[2];
    uint64_t _taskCount;
    mutable Mutex _lock;
    
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
