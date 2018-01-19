#pragma once

#include "XBlock.h"
#include <mutex>

struct cheatcoin_pool_task
{
    cheatcoin_field task[2], lastfield, minhash, nonce;
    cheatcoin_time_t main_time;
    SHA256_CTX ctx;
};

class XTaskWrapper
{
private:
    cheatcoin_pool_task _task;
    std::mutex _shareMutex;
public:
    XTaskWrapper();
    ~XTaskWrapper();

    cheatcoin_pool_task* GetTask() { return &_task; }
    void SetShare(cheatcoin_hash_t last, cheatcoin_hash_t hash);
    void DumpTask();
};

