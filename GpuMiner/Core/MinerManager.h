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

#pragma once

#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <random>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/optional.hpp>
#include <boost/asio.hpp>

#include "Exceptions.h"
#include "MinerEngine/CLMiner.h"

using namespace XDag;

class BadArgument : public Exception {};
struct MiningChannel : public LogChannel
{
    static const char* name() { return EthGreen "  m"; }
    static const int verbosity = 2;
    static const bool debug = false;
};
#define minelog clog(MiningChannel)

class MinerManager
{
public:
    enum class OperationMode
    {
        None,
        Benchmark,
        Mining
    };

    MinerManager(OperationMode mode = OperationMode::None);
    ~MinerManager();

    bool InterpretOption(int& i, int argc, char** argv);
    bool CheckMandatoryParams();
    void Execute();
    static void StreamHelp(std::ostream& _out);

private:
    void DoBenchmark(MinerType type, unsigned warmupDuration = 15, unsigned trialDuration = 3, unsigned trials = 5);
    void DoMining(MinerType type, std::string& remote, unsigned recheckPeriod);
    void ConfigureGpu();
    void ConfigureCpu();
    void FillRandomTask(XTaskWrapper *taskWrapper);
    void ValidateWorkerName();

    void IOWorkTimerHandler(const boost::system::error_code& ec);
    void StopIOService();

    /// Operating mode.
    OperationMode _mode;

    /// Global boost's io_service
    std::thread _io_thread;									// The IO service thread
    boost::asio::io_service _io_service;					// The IO service itself
    boost::asio::io_service::work _io_work;					// The IO work which prevents io_service.run() to return on no work thus terminating thread
    boost::asio::deadline_timer _io_work_timer;				// A dummy timer to keep io_service with something to do and prevent io shutdown
    boost::asio::io_service::strand _io_strand;				// A strand to serialize posts in multithreaded environment

    /// Mining options
    bool _running = true;
    MinerType _minerType = MinerType::NotSet;
    unsigned _openclPlatform = 0;
    unsigned _cpuMiningThreads = 0;
    unsigned _openclMiningDevices = MAX_CL_DEVICES;
    bool _shouldListDevices = false;
    unsigned _openclSelectedKernel = 0;  ///< A numeric value for the selected OpenCL kernel
    unsigned _openclDeviceCount = 0;
    unsigned _openclDevices[MAX_CL_DEVICES];
    bool _useOpenClCpu = false;
    unsigned _globalWorkSizeMultiplier = CLMiner::_defaultGlobalWorkSizeMultiplier;
    unsigned _localWorkSize = CLMiner::_defaultLocalWorkSize;
    bool _useNvidiaFix = false;
    bool _disableFee = false;
    uint32_t _nvidiaSpinDamp = CLMiner::_defaultNvidiaSpinDamp;
    bool _useVectors = false;

    /// Benchmarking params
    unsigned _benchmarkWarmup = 15;
    unsigned _benchmarkTrial = 3;
    unsigned _benchmarkTrials = 5;

    // Pool params
    std::string _poolUrl = "http://127.0.0.1:8545";
    unsigned _poolRetries = 0;
    unsigned _maxPoolRetries = 3;
    unsigned _poolRecheckPeriod = 2000;
    bool _poolRecheckSet = false;
    std::string _accountAddress;
    std::string _workerName;

    int _worktimeout = 180;
    bool _show_hwmonitors = false;

    std::string _fport = "";
};
