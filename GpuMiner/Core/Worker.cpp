/*
    This file is part of cpp-ethereum.

    cpp-ethereum is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp-ethereum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Worker.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
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
