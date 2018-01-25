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
#include "XDagCore\XCpuMiner.h"
#include "XDagCore\XTaskProcessor.h"
#include "XDagCore\XPool.h"
#include "Utils\CpuInfo.h"
#include "Utils\Random.h"

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
    else if(arg == "--opencl-platform" && i + 1 < argc)
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
    else if(arg == "--opencl-devices" || arg == "--opencl-device")
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
    else if((arg == "--cl-global-work") && i + 1 < argc)
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
    else if((arg == "--cl-local-work") && i + 1 < argc)
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
    else if(arg == "--list-devices")
    {
        _shouldListDevices = true;
    }
    else if(arg == "--benchmark-warmup" && i + 1 < argc)
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
    else if(arg == "--benchmark-trial" && i + 1 < argc)
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
    else if(arg == "--benchmark-trials" && i + 1 < argc)
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
    else if(arg == "-G" || arg == "--opencl")
    {
        _minerType = MinerType::CL;
    }
    else if(arg == "-M" || arg == "--benchmark")
    {
        _mode = OperationMode::Benchmark;
    }
    else if((arg == "-t" || arg == "--mining-threads") && i + 1 < argc)
    {
        try
        {
            _miningThreads = stol(argv[++i]);
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
        _minerType = MinerType::CPU;
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
        if(_minerType == MinerType::CL)
        {
            CLMiner::ListDevices();
        }
        if(_minerType == MinerType::CPU)
        {
            XCpuMiner::ListDevices();
        }
        return;
    }

    if(_minerType == MinerType::CL)
    {
        ConfigureGpu();
    }
    else if(_minerType == MinerType::CPU)
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
        << "    -G,--opencl  When mining use the GPU via OpenCL." << endl
        << "    -cpu  When mining use the CPU." << endl
        << endl
        << "Benchmarking mode:" << endl
        << "    -M ,--benchmark  Benchmark for mining and exit." << endl
        << "    --benchmark-warmup <seconds>  Set the duration of warmup for the benchmark tests (default: 3)." << endl
        << "    --benchmark-trial <seconds>  Set the duration for each trial for the benchmark tests (default: 3)." << endl
        << "    --benchmark-trials <n>  Set the number of benchmark trials to run (default: 5)." << endl
        << endl
        << "Mining configuration:" << endl
        << "    -p <url> Connect to a pool at URL" << endl
        << "    -a Your account address" << endl
        << "    --opencl-platform <n>  When mining using -G/--opencl use OpenCL platform n (default: 0)." << endl
        << "    --opencl-device <n>  When mining using -G/--opencl use OpenCL device n (default: 0)." << endl
        << "    --opencl-devices <0 1 ..n> Select which OpenCL devices to mine on. Default is to use all" << endl
        << "    -t, --mining-threads <n> Limit number of CPU/GPU miners to n (default: use everything available on selected platform)" << endl
        << "    --list-devices List the detected devices and exit. Should be combined with -G or -cpu flag" << endl
        << endl
        << " OpenCL configuration:" << endl
        << "    --cl-local-work Set the OpenCL local work size. Default is " << CLMiner::_defaultLocalWorkSize << endl
        << "    --cl-global-work Set the OpenCL global work size as a multiple of the local work size. Default is " << CLMiner::_defaultGlobalWorkSizeMultiplier << " * " << CLMiner::_defaultLocalWorkSize << endl
        << endl
        ;
}

void MinerManager::DoBenchmark(MinerType type, unsigned warmupDuration, unsigned trialDuration, unsigned trials)
{
    XTaskProcessor taskProcessor;
    FillRandomTask(taskProcessor.GetNextTask());
    taskProcessor.SwitchTask();

    Farm farm(&taskProcessor);
    map<string, Farm::SealerDescriptor> sealers;
    sealers["opencl"] = Farm::SealerDescriptor{ &CLMiner::Instances, [](unsigned index, XTaskProcessor* taskProcessor) { return new CLMiner(index, taskProcessor); } };
    farm.SetSealers(sealers);

    string platformInfo = "CL";
    cout << "Benchmarking on platform: " << platformInfo << endl;

    if(type == MinerType::CL)
    {
        farm.Start("opencl", false);
    }

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
    if(!pool.Connect() || !pool.Interract())
    {
        cerr << "Cannot connect to pool" << endl;
        exit(-1);
    }

    Farm farm(&taskProcessor);

    map<string, Farm::SealerDescriptor> sealers;
    sealers["opencl"] = Farm::SealerDescriptor{ &CLMiner::Instances, [](unsigned index, XTaskProcessor* taskProcessor) { return new CLMiner(index, taskProcessor); } };
    sealers["cpu"] = Farm::SealerDescriptor{ &XCpuMiner::Instances, [](unsigned index, XTaskProcessor* taskProcessor) { return new XCpuMiner(index, taskProcessor); } };

    farm.SetSealers(sealers);

    if(type == MinerType::CL)
    {
        farm.Start("opencl", false);
    }
    else if(type == MinerType::CPU)
    {
        farm.Start("cpu", false);
    }

    uint32_t iteration = 0;
    bool isConnected = true;
    while(_running)
    {
        if(!isConnected)
        {
            isConnected = pool.Connect();
        }
        if(!isConnected)
        {
            cerr << "Cannot connect to pool. Reconnection..." << endl;
            this_thread::sleep_for(chrono::milliseconds(5000));
            continue;
        }
        if(!pool.Interract())
        {
            pool.Disconnect();
            isConnected = false;
            cerr << "Failed to get data from pool. Reconnection..." << endl;
            this_thread::sleep_for(chrono::milliseconds(5000));
            continue;
        }

        auto mp = farm.MiningProgress();
        if(!iteration++)
        {
            continue;
        }
        minelog << mp;

        this_thread::sleep_for(chrono::milliseconds(_poolRecheckPeriod));
    }
    farm.Stop();
}

void MinerManager::ConfigureGpu()
{
    if(_openclDeviceCount > 0)
    {
        CLMiner::SetDevices(_openclDevices, _openclDeviceCount);
        _miningThreads = _openclDeviceCount;
    }

    if(!CLMiner::ConfigureGPU(
        _localWorkSize,
        _globalWorkSizeMultiplier,
        _openclPlatform))
    {
        exit(1);
    }
    CLMiner::SetNumInstances(_miningThreads);
}

void MinerManager::ConfigureCpu()
{
    if(_miningThreads == UINT_MAX)
    {
        _miningThreads = CpuInfo::GetNumberOfCpuCores();
    }
    XCpuMiner::SetNumInstances(_miningThreads);
}

bool MinerManager::CheckMandatoryParams()
{
    return (_shouldListDevices && (_minerType == MinerType::CL || _minerType == MinerType::CPU))
        || _mode == OperationMode::Benchmark && _minerType == MinerType::CL
        || ((_minerType == MinerType::CL || _minerType == MinerType::CPU) && !_accountAddress.empty() && !_poolUrl.empty());
}

void MinerManager::FillRandomTask(XTaskWrapper *taskWrapper)
{
    cheatcoin_pool_task *task = taskWrapper->GetTask();
    task->main_time = XBlock::GetMainTime();

    cheatcoin_hash_t data0;
    cheatcoin_hash_t data1;
    cheatcoin_hash_t addressHash;
    CRandom::FillRandomArray((uint8_t*)data0, sizeof(cheatcoin_hash_t));
    CRandom::FillRandomArray((uint8_t*)data1, sizeof(cheatcoin_hash_t));
    CRandom::FillRandomArray((uint8_t*)addressHash, sizeof(cheatcoin_hash_t));

    XHash::SetHashState(&task->ctx, data0, sizeof(struct cheatcoin_block) - 2 * sizeof(struct cheatcoin_field));

    XHash::HashUpdate(&task->ctx, data1, sizeof(struct cheatcoin_field));
    XHash::HashUpdate(&task->ctx, addressHash, sizeof(cheatcoin_hashlow_t));
    CRandom::FillRandomArray((uint8_t*)task->nonce.data, sizeof(cheatcoin_hash_t));
    memcpy(task->nonce.data, addressHash, sizeof(cheatcoin_hashlow_t));
    memcpy(task->lastfield.data, task->nonce.data, sizeof(cheatcoin_hash_t));
    XHash::HashFinal(&task->ctx, &task->nonce.amount, sizeof(uint64_t), task->minhash.data);
}