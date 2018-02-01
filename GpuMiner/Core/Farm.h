/*
  This file is taken from ethminer project.
*/
/*
* Evgeniy Sukhomlinov
* 2018
*/

#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>
#include <list>
#include <atomic>
#include "Common.h"
#include "Worker.h"
#include "Miner.h"
#include "XDagCore/XTaskProcessor.h"

namespace XDag
{
    /**
     * @brief A collective of Miners.
     * Miners ask for work, then submit proofs
     * @threadsafe
     */
    class Farm
    {
    public:
        struct SeekerDescriptor
        {
            std::function<unsigned()> Instances;
            std::function<Miner*(unsigned, XTaskProcessor*)> Create;
        };

        Farm(XTaskProcessor* taskProcessor) { _taskProcessor = taskProcessor; }
        ~Farm()
        {
            Stop();
        }

        void AddSeeker(SeekerDescriptor const& seeker) { _seekers.push_back(seeker); }

        /**
         * @brief Starts mining.
         */
        bool Start();

        /**
         * @brief Stop all mining activities.
         */
        void Stop();

        void CollectHashRate();

        void ProcessHashRate(const boost::system::error_code& ec);

        /**
         * @brief Stop all mining activities and Starts them again
         */
        void Restart();

        bool IsMining() const
        {
            return _isMining;
        }

        /**
         * @brief Get information on the progress of mining this work package.
         * @return The progress with mining so far.
         */
        WorkingProgress const& MiningProgress(bool hwmon = false) const;

        using MinerRestart = std::function<void()>;

        /**
         * @brief Provides a valid header based upon that received previously with setWork().
         * @param _bi The now-valid header.
         * @return true if the header was good and that the Farm should pause until more work is submitted.
         */
        void OnMinerRestart(MinerRestart const& handler) { _onMinerRestart = handler; }

        std::chrono::steady_clock::time_point FarmLaunched()
        {
            return _farm_launched;
        }

        std::string FarmLaunchedFormatted();

    private:
        std::vector<std::shared_ptr<Miner>> _miners;
        XTaskProcessor* _taskProcessor;

        std::atomic<bool> _isMining = { false };

        mutable Mutex _minerWorkLock;
        mutable Mutex _progressLock;
        mutable WorkingProgress _progress;

        mutable Mutex _hwmonsLock;

        MinerRestart _onMinerRestart;

        std::vector<SeekerDescriptor> _seekers;

        std::chrono::steady_clock::time_point _lastStart;
        uint32_t _hashrateSmoothInterval = 10000;
        std::thread _serviceThread;  ///< The IO service thread.
        boost::asio::io_service _io_service;
        boost::asio::deadline_timer *_hashrateTimer = nullptr;
        std::vector<WorkingProgress> _lastProgresses;

        std::chrono::steady_clock::time_point _farm_launched = std::chrono::steady_clock::now();
    };
}
