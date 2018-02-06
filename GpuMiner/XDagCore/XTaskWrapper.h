#pragma once

#include "XTime.h"
#include "XHash.h"
#include "XBlock.h"
#include <mutex>

//TODO: refactor
struct cheatcoin_pool_task
{
    cheatcoin_field lastfield, minhash, nonce;
    cheatcoin_time_t main_time;
    SHA256_CTX ctx;
};

class XTaskProcessor;

class XTaskWrapper
{
    friend class XTaskProcessor;
private:
    cheatcoin_pool_task _task;
    std::mutex _shareMutex;
    uint64_t _taskIndex;
    bool _isShareFound;

    uint32_t _preCalcState[8];
    uint32_t _reversedData[14];
public:
    XTaskWrapper();
    ~XTaskWrapper();

    void FillAndPrecalc(cheatcoin_field* data, cheatcoin_hash_t addressHash);
    cheatcoin_pool_task* GetTask() { return &_task; }
    void SetShare(cheatcoin_hash_t last, cheatcoin_hash_t hash);
    uint64_t GetIndex() { return _taskIndex; }
    bool IsShareFound() { return _isShareFound; }

    uint32_t* GetPrecalcState() { return _preCalcState; }
    uint32_t* GetReversedData() { return _reversedData; }

    void DumpTask();
};
