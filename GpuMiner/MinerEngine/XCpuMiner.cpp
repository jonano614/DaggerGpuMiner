// Implementation of CPU-mining
// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#include "XCpuMiner.h"
#include <iostream>
#include "Core/Log.h"
#include "Utils/CpuInfo.h"

unsigned XCpuMiner::_numInstances = 0;

XCpuMiner::XCpuMiner(unsigned index, XTaskProcessor* taskProcessor)
    :Miner("cpu-", index, taskProcessor)
{
}

XCpuMiner::~XCpuMiner()
{
}

void XCpuMiner::WorkLoop()
{
    xdag_hash_t hash;
    xdag_field last;
    uint64_t prevTaskIndex = 0;
    uint64_t nonce;
    int iterations = 256;

    while(true)
    {
        XTaskWrapper* taskWrapper = GetTask();
        //TODO: move this check higher (before threads creation) in order to remove spam on startup
        if(taskWrapper == NULL)
        {
            clog(LogChannel) << "No work. Pause for 3 s.";
            std::this_thread::sleep_for(std::chrono::seconds(3));
            continue;
        }

        if(taskWrapper->GetIndex() != prevTaskIndex)
        {
            prevTaskIndex = taskWrapper->GetIndex();
            memcpy(last.data, taskWrapper->GetTask()->nonce.data, sizeof(xdag_hash_t));
            nonce = last.amount + _index;
        }

        last.amount = XHash::SearchMinNonce(&taskWrapper->GetTask()->ctx, nonce, iterations, _numInstances, hash);
        taskWrapper->SetShare(last.data, hash);

        AddHashCount(iterations);

        // Check if we should stop.
        if(ShouldStop())
        {
            break;
        }
    }
}

HwMonitor XCpuMiner::Hwmon()
{
    HwMonitor hw;
    unsigned int tempC = 0, fanpcnt = 0;

    //TODO??
    hw.tempC = tempC;
    hw.fanP = fanpcnt;
    return hw;
}

void XCpuMiner::ListDevices()
{
    //TODO: multi cpu system?

    std::cout << "Cpu " << CpuInfo::GetNumberOfCpuCores() << " cores";
}
