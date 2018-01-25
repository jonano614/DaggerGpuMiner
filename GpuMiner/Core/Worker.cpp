/*
   This file is taken from ethminer project.
*/
/*
 * Evgeniy Sukhomlinov
 * 2018
 */

#include "Worker.h"

#include <chrono>
#include <thread>
#include "Log.h"

using namespace XDag;

void Worker::StartWorking()
{
    Guard l(_workLock);
    if(_workerThread)
    {
        WorkerState ex = WorkerState::Stopped;
        _state.compare_exchange_strong(ex, WorkerState::Starting);
    }
    else
    {
        _state = WorkerState::Starting;
        _workerThread.reset(new std::thread([&]()
        {
            SetThreadName(_name.c_str());
            while(_state != WorkerState::Killing)
            {
                WorkerState ex = WorkerState::Starting;
                bool ok = _state.compare_exchange_strong(ex, WorkerState::Started);
                (void)ok;

                try
                {
                    WorkLoop();
                }
                catch(std::exception const& _e)
                {
                    clog(WarnChannel) << "Exception thrown in Worker thread: " << _e.what();
                }

                ex = _state.exchange(WorkerState::Stopped);
                if(ex == WorkerState::Killing || ex == WorkerState::Starting)
                {
                    _state.exchange(ex);
                }

                while(_state == WorkerState::Stopped)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
            }
        }));
    }
    while(_state == WorkerState::Starting)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(20));
    }
}

void Worker::StopWorking()
{
    DEV_GUARDED(_workLock)
        if(_workerThread)
        {
            WorkerState ex = WorkerState::Started;
            _state.compare_exchange_strong(ex, WorkerState::Stopping);

            while(_state != WorkerState::Stopped)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(20));
            }
        }
}

Worker::~Worker()
{
    DEV_GUARDED(_workLock)
        if(_workerThread)
        {
            _state.exchange(WorkerState::Killing);
            _workerThread->join();
            _workerThread.reset();
        }
}
