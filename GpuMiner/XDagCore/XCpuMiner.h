#pragma once

#include "Core\Miner.h"

using namespace XDag;

//Mainly for test purposes
class XCpuMiner : public Miner
{
private:
    static unsigned _numInstances;

    void WorkLoop() override;
public:
    XCpuMiner(unsigned index, XTaskProcessor* taskProcessor);
    virtual ~XCpuMiner();

    static unsigned Instances() { return _numInstances > 0 ? _numInstances : 1; }
    static void SetNumInstances(unsigned instances) { _numInstances = instances; }
    static void ListDevices();

    HwMonitor Hwmon() override;
protected:
    void KickOff() override;
    void Pause() override;
};
