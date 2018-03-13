#include "XTaskProcessor.h"
#include "Core/Log.h"

XTaskProcessor::XTaskProcessor()
{
    _taskCount = 0;
}

XTaskProcessor::~XTaskProcessor()
{
}

XTaskWrapper* XTaskProcessor::GetNextTask()
{
    Guard l(_lock);
    return &_tasks[(_taskCount + 1) & 1];
}

XTaskWrapper* XTaskProcessor::GetCurrentTask()
{
    Guard l(_lock);
    if(_taskCount <= 0)
    {
        return NULL;
    }

    return &_tasks[_taskCount & 1];
}

void XTaskProcessor::SwitchTask()
{
    //TODO: do I need thread sync?
    Guard l(_lock);
    ++_taskCount;
    _tasks[_taskCount & 1]._taskIndex = _taskCount;
    _tasks[_taskCount & 1]._isShareFound = false;    
}

void XTaskProcessor::DumpTasks()
{
    Guard l(_lock);
    clog(XDag::DebugChannel) << "Count of tasks: " << _taskCount << "task index: " << (_taskCount & 1);
    clog(XDag::DebugChannel) << "Task 0";
    _tasks[0].DumpTask();
    clog(XDag::DebugChannel) << "Task 1";
    _tasks[1].DumpTask();
}
