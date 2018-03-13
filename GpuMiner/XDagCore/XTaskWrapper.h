#pragma once

#include "XTime.h"
#include "XHash.h"
#include "XBlock.h"
#include <mutex>

//TODO: refactor
struct xdag_pool_task
{
    xdag_field lastfield, minhash, nonce;
    xdag_time_t main_time;
    SHA256_CTX ctx;
};

class XTaskProcessor;

class XTaskWrapper
{
    friend class XTaskProcessor;
private:
    xdag_pool_task _task;
    std::mutex _shareMutex;
    uint64_t _taskIndex;
    bool _isShareFound;

    uint32_t _preCalcState[8];
    uint32_t _reversedData[14];
public:
    XTaskWrapper();
    ~XTaskWrapper();

    void FillAndPrecalc(xdag_field* data, xdag_hash_t addressHash);
    xdag_pool_task* GetTask() { return &_task; }
    void SetShare(xdag_hash_t last, xdag_hash_t hash);
    uint64_t GetIndex() { return _taskIndex; }
    bool IsShareFound() { return _isShareFound; }

    uint32_t* GetPrecalcState() { return _preCalcState; }
    uint32_t* GetReversedData() { return _reversedData; }

    void DumpTask();
};
