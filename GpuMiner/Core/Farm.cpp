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

#pragma once

#include "Farm.h"

using namespace XDag;

/**
 * @brief Start a number of miners.
 */
bool Farm::Start(std::string const& _sealer, bool mixed)
{
    Guard l(_minerWorkLock);
    if(!_miners.empty() && _lastSealer == _sealer)
    {
        return true;
    }
    if(!_sealers.count(_sealer))
    {
        return false;
    }

    if(!mixed)
    {
        _miners.clear();
    }
    auto ins = _sealers[_sealer].Instances();
    unsigned start = 0;
    if(!mixed)
    {
        _miners.reserve(ins);
    }
    else
    {
        start = _miners.size();
        ins += start;
        _miners.reserve(ins);
    }
    for(unsigned i = start; i < ins; ++i)
    {
        // TODO: Improve miners creation, use unique_ptr.
        _miners.push_back(std::shared_ptr<Miner>(_sealers[_sealer].Create(i, _taskProcessor)));

        // Start miners' threads. They should pause waiting for new work
        // package.
        _miners.back()->StartWorking();
    }
    _isMining = true;
    _lastSealer = _sealer;
    _lastMixed = mixed;

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
            _serviceThread = std::thread{ boost::bind(&boost::asio::io_service::run, &_io_service) };
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
        //TODO: temporary
        for(auto const& i : _miners)
        {
            i->StopWorking();
        }
        _miners.clear();
        _isMining = false;
    }

    _io_service.stop();
    _serviceThread.join();

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
    int allMs = 0;
    for(auto const& cp : _lastProgresses)
    {
        allMs += cp.ms;
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
    Start(_lastSealer, _lastMixed);

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