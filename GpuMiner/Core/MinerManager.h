#pragma once

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
/** @file MinerAux.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * CLI module for mining.
 */

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
#include "HashCL\CLMiner.h"

using namespace XDag;

class BadArgument : public Exception {};
struct MiningChannel : public LogChannel
{
    static const char* name() { return EthGreen "  m"; }
    static const int verbosity = 2;
    static const bool debug = false;
};
#define minelog clog(MiningChannel)

inline std::string ToJS(unsigned long _n)
{
    std::string h = ToHex(ToCompactBigEndian(_n, 1));
    // remove first 0, if it is necessary;
    std::string res = h[0] != '0' ? h : h.substr(1);
    return "0x" + res;
}

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
    unsigned _miningThreads = 0;
    bool _shouldListDevices = false;
    unsigned _openclSelectedKernel = 0;  ///< A numeric value for the selected OpenCL kernel
    unsigned _openclDeviceCount = 0;
    unsigned _openclDevices[16];
    unsigned _openclThreadsPerHash = 8;
    unsigned _globalWorkSizeMultiplier = CLMiner::_defaultGlobalWorkSizeMultiplier;
    unsigned _localWorkSize = CLMiner::_defaultLocalWorkSize;

    //unsigned m_dagLoadMode = 0; // parallel
    //unsigned m_dagCreateDevice = 0;
    /// Benchmarking params
    unsigned _benchmarkWarmup = 15;
    unsigned _parallelHash = 4;
    unsigned _benchmarkTrial = 3;
    unsigned _benchmarkTrials = 5;
    unsigned _benchmarkBlock = 0;

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
