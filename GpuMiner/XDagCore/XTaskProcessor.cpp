#include "XTaskProcessor.h"
#include "Core/Log.h"

XTaskProcessor::XTaskProcessor()
{
    _taskCount = 0;
    _taskIsActive = false;
}

XTaskProcessor::~XTaskProcessor()
{
}

XTaskWrapper* XTaskProcessor::GetNextTask()
{
    return &_tasks[(_taskCount + 1) & 1];
}

XTaskWrapper* XTaskProcessor::GetCurrentTask()
{
    if(!_taskIsActive)
    {
        return NULL;
    }

    return &_tasks[_taskCount & 1];
}

void XTaskProcessor::SwitchTask()
{
    ++_taskCount;
    _tasks[_taskCount & 1]._taskIndex = _taskCount;
    _tasks[_taskCount & 1]._isShareFound = false;   
    _taskIsActive = true;
}

void XTaskProcessor::DumpTasks()
{
    clog(XDag::DebugChannel) << "Count of tasks: " << _taskCount << "task index: " << (_taskCount & 1);
    clog(XDag::DebugChannel) << "Task 0";
    _tasks[0].DumpTask();
    clog(XDag::DebugChannel) << "Task 1";
    _tasks[1].DumpTask();
}
