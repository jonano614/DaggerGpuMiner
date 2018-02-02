/*
   This file is taken from ethminer project.
*/
/*
 * Evgeniy Sukhomlinov
 * 2018
 */

#pragma once

#include "Farm.h"

using namespace XDag;

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
    _isMining = true;

    if(!_hashrateTimer)
    {
        _hashrateTimer = new boost::asio::deadline_timer(_io_service, boost::posix_time::milliseconds(1000));
        _hashrateTimer->async_wait(boost::bind(&Farm::ProcessHashRate, this, boost::asio::placeholders::error));
        if(_serviceThread.joinable())
        {
            _io_service.reset();
        }
        else
        {
            _serviceThread = std::thread { boost::bind(&boost::asio::io_service::run, &_io_service) };
        }
    }

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
        _isMining = false;
    }

    _io_service.stop();
    if(_serviceThread.joinable())
    {
        _serviceThread.join();
    }

    if(_hashrateTimer)
    {
        _hashrateTimer->cancel();
        _hashrateTimer = nullptr;
    }
}

void Farm::CollectHashRate()
{
    WorkingProgress p;
    Guard l2(_minerWorkLock);
    p.ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _lastStart).count();
    //Collect
    for(auto const& i : _miners)
    {
        uint64_t minerHashCount = i->HashCount();
        p.hashes += minerHashCount;
        p.minersHashes.push_back(minerHashCount);
    }

    //Reset
    for(auto const& i : _miners)
    {
        i->ResetHashCount();
    }
    _lastStart = std::chrono::steady_clock::now();

    if(p.hashes > 0)
    {
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
        CollectHashRate();
    }

    // Restart timer 	
    _hashrateTimer->expires_at(_hashrateTimer->expires_at() + boost::posix_time::milliseconds(1000));
    _hashrateTimer->async_wait(boost::bind(&Farm::ProcessHashRate, this, boost::asio::placeholders::error));
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
            if(hwmon)
            {
                p.minerMonitors.push_back(i->Hwmon());
            }
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
