#include "XTaskProcessor.h"
#include "Core\Log.h"

XTaskProcessor::XTaskProcessor()
{
    //_taskIndex = -1;
    _taskCount = 0;
}


XTaskProcessor::~XTaskProcessor()
{
}

XTaskWrapper* XTaskProcessor::GetNextTask()
{
    //int index = _taskIndex > 1 ? 0 : _taskIndex + 1;
    //return &_tasks[index];
    return &_tasks[(_taskCount + 1) % 2];
}

XTaskWrapper* XTaskProcessor::GetCurrentTask()
{
    if(_taskCount <= 0)
    {
        return NULL;
    }

    //_taskMutex.lock();
    //XTaskWrapper* task = &_tasks[_taskIndex];
    XTaskWrapper* task = &_tasks[_taskCount % 2];
    //_taskMutex.unlock();
    return task;
}

void XTaskProcessor::SwitchTask()
{
    //TODO: resolve runtime error
    //_taskMutex.lock();
    //if(++_taskIndex > 2)
    //{
    //    _taskIndex = 0;
    //}
    ++_taskCount;
    //_taskMutex.unlock();
}

void XTaskProcessor::DumpTasks()
{
    clog(XDag::DebugChannel) << "Count of tasks: " << _taskCount << "task index: " << _taskCount % 2;
    clog(XDag::DebugChannel) << "Task 0";
    _tasks[0].DumpTask();
    clog(XDag::DebugChannel) << "Task 1";
    _tasks[1].DumpTask();
}