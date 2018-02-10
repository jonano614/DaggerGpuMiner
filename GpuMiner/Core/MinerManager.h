/*
   This file is taken from ethminer project.
*/
/*
 * Evgeniy Sukhomlinov
 * 2018
 */

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

    MinerManager(OperationMode mode = OperationMode::None) : _mode(_mode) {}

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

    /// Operating mode.
    OperationMode _mode;

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
    bool _useOpenCpu = false;
    unsigned _globalWorkSizeMultiplier = CLMiner::_defaultGlobalWorkSizeMultiplier;
    unsigned _localWorkSize = CLMiner::_defaultLocalWorkSize;

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

    int _worktimeout = 180;
    bool _show_hwmonitors = false;

    std::string _fport = "";
};
