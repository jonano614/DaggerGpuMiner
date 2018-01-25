/*
   This file is taken from ethminer project.
*/
/*
 * Evgeniy Sukhomlinov
 * 2018
 */

#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <cassert>
#include "Guards.h"

namespace XDag
{
    enum class WorkerState
    {
        Starting,
        Started,
        Stopping,
        Stopped,
        Killing
    };

    class Worker
    {
    public:
        Worker(std::string const& _name) : _name(_name) {}

        Worker(Worker const&) = delete;
        Worker& operator=(Worker const&) = delete;

        virtual ~Worker();

        /// Starts worker thread; causes startedWorking() to be called.
        void StartWorking();

        /// Stop worker thread; causes call to stopWorking().
        void StopWorking();

        bool ShouldStop() const { return _state != WorkerState::Started; }

    private:
        virtual void WorkLoop() = 0;

        std::string _name;

        mutable Mutex _workLock;						///< Lock for the network existance.
        std::unique_ptr<std::thread> _workerThread;		///< The worker thread.
        std::atomic<WorkerState> _state = { WorkerState::Starting };
    };
}
