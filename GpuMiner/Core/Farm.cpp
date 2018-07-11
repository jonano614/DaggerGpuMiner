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

// Modified by Evgeniy Sukhomlinov 2018

#include "Farm.h"

using namespace XDag;

Farm::Farm(XTaskProcessor* taskProcessor, boost::asio::io_service &io_service) :
    _io_strand(io_service),
    _hashrateTimer(io_service)
{ 
    _taskProcessor = taskProcessor;
}

/**
 * @brief Start a number of miners.
 */
bool Farm::Start()
{
    Guard l(_minerWorkLock);
    if(!_miners.empty())
    {
        return true;
    }
    if(!_seekers.size())
    {
        return false;
    }

    uint32_t ins = 0;
    for(auto const& s : _seekers)
    {
        ins += s.Instances();
    }
    _miners.reserve(ins);
    bool success = true;
    for(auto const& s : _seekers)
    {
        ins = s.Instances();
        clog(LogChannel) << "Starting " << ins << " miner instances.";
        for(uint32_t i = 0; i < ins; ++i)
        {
            _miners.push_back(std::shared_ptr<Miner>(s.Create(i, _taskProcessor)));

            if(!_miners.back()->Initialize())
            {
                clog(LogChannel) << "Failed to initialize mining";
                success = false;
                break;
            }
        }
    }
    if(!success)
    {
        for(auto const& m : _miners)
        {
            m->StopWorking();
        }
        _miners.clear();
        return false;
    }

    for(auto const& m : _miners)
    {
        // Start miners' threads. They should pause waiting for new work
        // package.
        m->StartWorking();
    }
    _isMining.store(true, std::memory_order_relaxed);
    _progressJustStarted = true;

    // Start hashrate collector
    _hashrateTimer.expires_from_now(boost::posix_time::milliseconds(1000));
    _hashrateTimer.async_wait(_io_strand.wrap(boost::bind(&Farm::ProcessHashRate, this, boost::asio::placeholders::error)));

    return true;
}

/**
 * @brief Stop all mining activities.
 */
void Farm::Stop()
{
    {
        Guard l(_minerWorkLock);
        for(auto const& m : _miners)
        {
            m->StopWorking();
        }
        _miners.clear();
        _isMining.store(false, std::memory_order_relaxed);
    }

    _hashrateTimer.cancel();
    _lastProgresses.clear();
}

void Farm::CollectHashRate()
{
    auto now = std::chrono::steady_clock::now();

    Guard l(_minerWorkLock);
    WorkingProgress p;
    p.ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastStart).count();
    _lastStart = now;

    // Collect & Reset
    for(auto const& m : _miners)
    {
        uint64_t minerHashCount = m->HashCount();
        m->ResetHashCount();
        p.hashes += minerHashCount;
        p.minersHashes.push_back(minerHashCount);
    }

    if((p.hashes > 0 || !_progressJustStarted) && IsMining())
    {
        _progressJustStarted = false;
        _lastProgresses.push_back(p);
    }

    // We smooth the hashrate over the last x seconds
    uint32_t allMs = 0;
    for(auto const& cp : _lastProgresses)
    {
        allMs += (uint32_t)cp.ms;
    }
    if(allMs > _hashrateSmoothInterval)
    {
        _lastProgresses.erase(_lastProgresses.begin());
    }
}

void Farm::ProcessHashRate(const boost::system::error_code& ec)
{
    if(!ec)
    {
        if(!IsMining())
        {
            return;
        }

        CollectHashRate();

        // Resubmit timer only if actually mining
        _hashrateTimer.expires_from_now(boost::posix_time::milliseconds(1000));
        _hashrateTimer.async_wait(_io_strand.wrap(boost::bind(&Farm::ProcessHashRate, this, boost::asio::placeholders::error)));
    }    
}

/**
 * @brief Stop all mining activities and Starts them again
 */
void Farm::Restart()
{
    Stop();
    Start();

    if(_onMinerRestart)
    {
        _onMinerRestart();
    }
}

/**
 * @brief Get information on the progress of mining this work package.
 * @return The progress with mining so far.
 */
WorkingProgress const& Farm::MiningProgress(bool hwmon) const
{
    WorkingProgress p;
    p.ms = 0;
    p.hashes = 0;
    {
        Guard l2(_minerWorkLock);
        for(auto const& i : _miners)
        {
            p.minersHashes.push_back(0);
            //if(hwmon)
            //{
            //    p.minerMonitors.push_back(i->Hwmon());
            //}
        }

        for(auto const& cp : _lastProgresses)
        {
            p.ms += cp.ms;
            p.hashes += cp.hashes;
            for(unsigned int i = 0; i < cp.minersHashes.size(); i++)
            {
                p.minersHashes.at(i) += cp.minersHashes.at(i);
            }
        }
    }

    Guard l(_progressLock);
    _progress = p;
    return _progress;
}

std::string Farm::FarmLaunchedFormatted()
{
    auto d = std::chrono::steady_clock::now() - _farm_launched;
    int hsize = 3;
    auto hhh = std::chrono::duration_cast<std::chrono::hours>(d);
    if(hhh.count() < 100)
    {
        hsize = 2;
    }
    d -= hhh;
    auto mm = std::chrono::duration_cast<std::chrono::minutes>(d);
    std::ostringstream stream;
    stream << "Time: " << std::setfill('0') << std::setw(hsize) << hhh.count() << ':' << std::setfill('0') << std::setw(2) << mm.count();
    return stream.str();
}
