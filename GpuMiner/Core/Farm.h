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
/** @file Farm.h
* @author Gav Wood <i@gavwood.com>
* @date 2015
*/

// Modified by Evgeniy Sukhomlinov 2018

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

        Farm(XTaskProcessor* taskProcessor, boost::asio::io_service & io_service);
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
            return _isMining.load(std::memory_order_relaxed);
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

        boost::asio::io_service::strand _io_strand;
        boost::asio::deadline_timer _hashrateTimer;
        std::vector<WorkingProgress> _lastProgresses;
        bool _progressJustStarted;

        std::chrono::steady_clock::time_point _farm_launched = std::chrono::steady_clock::now();
    };
}
