// Operates with current task data
// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#pragma once

#include "XTaskWrapper.h"
#include "Core/Guards.h"

class XTaskProcessor
{
private:
    XTaskWrapper _tasks[2];
    uint64_t _taskCount;
    bool _taskIsActive;
    mutable Mutex _lock;
    
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
