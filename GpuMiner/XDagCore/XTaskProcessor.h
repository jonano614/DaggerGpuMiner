#pragma once

#include "XTaskWrapper.h"
#include <mutex>

class XTaskProcessor
{
private:
    XTaskWrapper _tasks[2];
    uint64_t _taskCount;
    bool _taskIsActive;
public:
    XTaskProcessor();
    ~XTaskProcessor();

    //TODO: think of better process
    XTaskWrapper* GetNextTask();
    XTaskWrapper* GetCurrentTask();
    void SwitchTask();
    uint64_t GetCount() { return _taskCount; }
    void ResetTasks() { _taskIsActive = false; }

    void DumpTasks();
};
