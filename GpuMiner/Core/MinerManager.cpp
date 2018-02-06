/*
   This file is taken from ethminer project.
*/
/*
 * Evgeniy Sukhomlinov
 * 2018
 */

#pragma once

#include <stdint.h>
#include "MinerManager.h"
#include "Farm.h"
#include "MinerEngine/XCpuMiner.h"
#include "XDagCore/XTaskProcessor.h"
#include "XDagCore/XPool.h"
#include "Utils/CpuInfo.h"
#include "Utils/Random.h"

using namespace std;
using namespace XDag;

bool MinerManager::InterpretOption(int& i, int argc, char** argv)
{
    string arg = argv[i];
    if((arg == "-p") && i + 1 < argc)
    {
        _mode = OperationMode::Mining;
        _poolUrl = argv[++i];
    }
    else if(arg == "-opencl-platform" && i + 1 < argc)
    {
        try
        {
            _openclPlatform = stol(argv[++i]);
        }
        catch(...)
        {
            cerr << "Bad " << arg << " option: " << argv[i] << endl;
            BOOST_THROW_EXCEPTION(BadArgument());
        }
    }
    else if(arg == "-opencl-devices")
    {
        while(_openclDeviceCount < 16 && i + 1 < argc)
        {
            try
            {
                _openclDevices[_openclDeviceCount] = stol(argv[++i]);
                ++_openclDeviceCount;
            }
            catch(...)
            {
                i--;
                break;
            }
        }
    }
    else if((arg == "-cl-global-work") && i + 1 < argc)
    {
        try
        {
            _globalWorkSizeMultiplier = stol(argv[++i]);
        }
        catch(...)
        {
            cerr << "Bad " << arg << " option: " << argv[i] << endl;
            BOOST_THROW_EXCEPTION(BadArgument());
        }
    }
    else if((arg == "-cl-local-work") && i + 1 < argc)
    {
        try
        {
            _localWorkSize = stol(argv[++i]);
        }
        catch(...)
        {
            cerr << "Bad " << arg << " option: " << argv[i] << endl;
            BOOST_THROW_EXCEPTION(BadArgument());
        }
    }
    else if(arg == "-list-devices")
    {
        _shouldListDevices = true;
    }
    else if(arg == "-benchmark-warmup" && i + 1 < argc)
    {
        try
        {
            _benchmarkWarmup = stol(argv[++i]);
        }
        catch(...)
        {
            cerr << "Bad " << arg << " option: " << argv[i] << endl;
            BOOST_THROW_EXCEPTION(BadArgument());
        }
    }
    else if(arg == "-benchmark-trial" && i + 1 < argc)
    {
        try
        {
            _benchmarkTrial = stol(argv[++i]);
        }
        catch(...)
        {
            cerr << "Bad " << arg << " option: " << argv[i] << endl;
            BOOST_THROW_EXCEPTION(BadArgument());
        }
    }
    else if(arg == "-benchmark-trials" && i + 1 < argc)
    {
        try
        {
            _benchmarkTrials = stol(argv[++i]);
        }
        catch(...)
        {
            cerr << "Bad " << arg << " option: " << argv[i] << endl;
            BOOST_THROW_EXCEPTION(BadArgument());
        }
    }
    else if(arg == "-G" || arg == "-opencl")
    {
        _minerType |= MinerType::CL;
    }
    else if(arg == "-M" || arg == "-benchmark")
    {
        _mode = OperationMode::Benchmark;
    }
    else if((arg == "-t") && i + 1 < argc)
    {
        try
        {
            _cpuMiningThreads = stol(argv[++i]);
        }
        catch(...)
        {
            cerr << "Bad " << arg << " option: " << argv[i] << endl;
            BOOST_THROW_EXCEPTION(BadArgument());
        }
    }
    else if((arg == "-d") && i + 1 < argc)
    {
        try
        {
            _openclMiningDevices = stol(argv[++i]);
        }
        catch(...)
        {
            cerr << "Bad " << arg << " option: " << argv[i] << endl;
            BOOST_THROW_EXCEPTION(BadArgument());
        }
    }
    else if(arg == "-a"  && i + 1 < argc)
    {
        _accountAddress = argv[++i];
    }
    else if(arg == "-cpu")
    {
        _minerType |= MinerType::CPU;
    }
    else if(arg == "-opencl-cpu")
    {
        _useOpenCpu = true;
    }
    else
    {
        return false;
    }
    return true;
}

void MinerManager::Execute()
{
    if(_shouldListDevices)
    {
        if((_minerType & MinerType::CL) == MinerType::CL)
        {
            CLMiner::ListDevices(_useOpenCpu);
        }
        if((_minerType & MinerType::CPU) == MinerType::CPU)
        {
            XCpuMiner::ListDevices();
        }
        return;
    }

    if((_minerType & MinerType::CL) == MinerType::CL)
    {
        ConfigureGpu();
    }
    if((_minerType & MinerType::CPU) == MinerType::CPU)
    {
        ConfigureCpu();
    }
    if(_mode == OperationMode::Benchmark)
    {
        DoBenchmark(_minerType, _benchmarkWarmup, _benchmarkTrial, _benchmarkTrials);
    }
    else if(_mode == OperationMode::Mining)
    {
        DoMining(_minerType, _poolUrl, _poolRecheckPeriod);
    }
}

void MinerManager::StreamHelp(ostream& _out)
{
    _out
        << "Mining mode:" << endl
        << "    -G, -opencl  When mining use the GPU via OpenCL." << endl
        << "    -cpu  When mining use the CPU." << endl
        << endl
        << "Benchmarking mode:" << endl
        << "    -M, -benchmark  Benchmark for mining and exit." << endl
        << "    -benchmark-warmup <seconds>  Set the duration of warmup for the benchmark tests (default: 3)." << endl
        << "    -benchmark-trial <seconds>  Set the duration for each trial for the benchmark tests (default: 3)." << endl
        << "    -benchmark-trials <n>  Set the number of benchmark trials to run (default: 5)." << endl
        << endl
        << "Mining configuration:" << endl
        << "    -p <url> Connect to a pool at URL" << endl
        << "    -a Your account address" << endl
        << "    -opencl-platform <n>  When mining using -G/-opencl use OpenCL platform n (default: 0)." << endl
        << "    -opencl-devices <0 1 ..n> Select which OpenCL devices to mine on (default: use everything available on selected platform)." << endl
        << "    -t <n> Set number of CPU threads to n (default: the number of threads is equal to number of cores)." << endl
        << "    -d <n> Limit number of used GPU devices to n (default: use everything available on selected platform)." << endl
        << "    -list-devices List the detected devices and exit. Should be combined with -G or -cpu flag." << endl
        << endl
        << " OpenCL configuration:" << endl
        << "    -cl-local-work Set the OpenCL local work size. Default is " << CLMiner::_defaultLocalWorkSize << endl
        << "    -cl-global-work Set the OpenCL global work size as a multiple of the local work size. Default is " << CLMiner::_defaultGlobalWorkSizeMultiplier << " * " << CLMiner::_defaultLocalWorkSize << endl
        << endl
        << "For test purposes: " << endl
        << "    -opencl-cpu Use CPU as OpenCL device." << endl
        << endl
        ;
}

void MinerManager::DoBenchmark(MinerType type, unsigned warmupDuration, unsigned trialDuration, unsigned trials)
{
    if(type != MinerType::CL)
    {
        cout << "Benchmark is available only for OpenCL" << endl;
    }
    XTaskProcessor taskProcessor;
    FillRandomTask(taskProcessor.GetNextTask());
    taskProcessor.SwitchTask();

    Farm farm(&taskProcessor);
    farm.AddSeeker(Farm::SeekerDescriptor { &CLMiner::Instances, [](unsigned index, XTaskProcessor* taskProcessor) { return new CLMiner(index, taskProcessor); } });

    string platformInfo = "CL";
    cout << "Benchmarking on platform: " << platformInfo << endl;

    farm.Start();

    map<uint64_t, WorkingProgress> results;
    uint64_t mean = 0;
    uint64_t innerMean = 0;
    for(unsigned i = 0; i <= trials; ++i)
    {
        if(!i)
        {
            cout << "Warming up..." << endl;
        }
        else
        {
            cout << "Trial " << i << "... " << flush;
        }
        this_thread::sleep_for(chrono::seconds(i ? trialDuration : warmupDuration));

        auto mp = farm.MiningProgress();
        if(!i)
        {
            continue;
        }
        auto rate = mp.Rate();

        cout << rate << endl;
        results[rate] = mp;
        mean += rate;
    }
    farm.Stop();
    int j = -1;
    for(auto const& r : results)
    {
        if(++j > 0 && j < (int)trials - 1)
        {
            innerMean += r.second.Rate();
        }
    }
    innerMean /= (trials - 2);
    cout << "min/mean/max: " << results.begin()->second.Rate() << "/" << (mean / trials) << "/" << results.rbegin()->second.Rate() << " H/s" << endl;
    cout << "inner mean: " << innerMean << " H/s" << endl;
}

void MinerManager::DoMining(MinerType type, string& remote, unsigned recheckPeriod)
{
    XTaskProcessor taskProcessor;

    XPool pool(_accountAddress, remote, &taskProcessor);
    if(!pool.Initialize())
    {
        cerr << "Pool initialization error" << endl;
        exit(-1);
    }
    if(!pool.Connect())
    {
        cerr << "Cannot connect to pool" << endl;
        exit(-1);
    }
    //wait a bit
    this_thread::sleep_for(chrono::milliseconds(200));

    Farm farm(&taskProcessor);

    if((type & MinerType::CL) == MinerType::CL)
    {
        farm.AddSeeker(Farm::SeekerDescriptor { &CLMiner::Instances, [](unsigned index, XTaskProcessor* taskProcessor) { return new CLMiner(index, taskProcessor); } });
    }
    if((type & MinerType::CPU) == MinerType::CPU)
    {
        farm.AddSeeker(Farm::SeekerDescriptor { &XCpuMiner::Instances, [](unsigned index, XTaskProcessor* taskProcessor) { return new XCpuMiner(index, taskProcessor); } });
    }

    if(!farm.Start())
    {
        cerr << "Failed to start mining";
        exit(-1);
    }

    uint32_t iteration = 0;
    bool isConnected = true;
    while(_running)
    {
        if(!isConnected)
        {
            isConnected = pool.Connect();
            if(isConnected)
            {
                if(!farm.Start())
                {
                    cerr << "Failed to restart mining";
                    exit(-1);
                }
            }
            else
            {
                cerr << "Cannot connect to pool. Reconnection..." << endl;
                this_thread::sleep_for(chrono::milliseconds(5000));
                continue;
            }
        }

        if(!pool.Interract())
        {
            pool.Disconnect();
            farm.Stop();
            isConnected = false;
            cerr << "Failed to get data from pool. Reconnection..." << endl;
            this_thread::sleep_for(chrono::milliseconds(5000));
            continue;
        }

        if(iteration > 0 && (iteration & 1) == 0)
        {
            auto mp = farm.MiningProgress();
            minelog << mp;
        }

        this_thread::sleep_for(chrono::milliseconds(_poolRecheckPeriod));
        ++iteration;
    }
    farm.Stop();
}

void MinerManager::ConfigureGpu()
{
    if(_openclDeviceCount > 0)
    {
        CLMiner::SetDevices(_openclDevices, _openclDeviceCount);
        _openclMiningDevices = _openclDeviceCount;
    }

    if(!CLMiner::ConfigureGPU(
        _localWorkSize,
        _globalWorkSizeMultiplier,
        _openclPlatform,
        _useOpenCpu))
    {
        exit(1);
    }

    CLMiner::SetNumInstances(_openclMiningDevices);
}

void MinerManager::ConfigureCpu()
{
    if(_cpuMiningThreads == 0)
    {
        _cpuMiningThreads = CpuInfo::GetNumberOfCpuCores();
    }
    XCpuMiner::SetNumInstances(_cpuMiningThreads);
}

bool MinerManager::CheckMandatoryParams()
{
    return (_shouldListDevices && _minerType != MinerType::NotSet)
        || _mode == OperationMode::Benchmark && _minerType == MinerType::CL
        || ((_minerType == MinerType::CPU || _minerType == MinerType::CL) && !_accountAddress.empty() && !_poolUrl.empty());
}

void MinerManager::FillRandomTask(XTaskWrapper *taskWrapper)
{
    cheatcoin_field data[2];
    cheatcoin_hash_t addressHash;
    CRandom::FillRandomArray((uint8_t*)(data[0].data), sizeof(cheatcoin_hash_t));
    CRandom::FillRandomArray((uint8_t*)(data[1].data), sizeof(cheatcoin_hash_t));
    CRandom::FillRandomArray((uint8_t*)addressHash, sizeof(cheatcoin_hash_t));

    taskWrapper->FillAndPrecalc(data, addressHash);
}
