// Task data
// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#include "XTaskWrapper.h"
#include "Core/Log.h"
#include "Utils/Utils.h"
#include "Utils/Random.h"
#include "Hash/sha256_mod.h"

#define bytereverse(x) ( ((x) << 24) | (((x) << 8) & 0x00ff0000) | (((x) >> 8) & 0x0000ff00) | ((x) >> 24) )

XTaskWrapper::XTaskWrapper()
    :_taskIndex(0),
    _isShareFound(false)
{
}

XTaskWrapper::~XTaskWrapper()
{
}

void XTaskWrapper::FillAndPrecalc(xdag_field* data, xdag_hash_t addressHash)
{
    _task.main_time = GetMainTime();

    XHash::SetHashState(&_task.ctx, data[0].data, sizeof(struct xdag_block) - 2 * sizeof(struct xdag_field));

    XHash::HashUpdate(&_task.ctx, data[1].data, sizeof(struct xdag_field));
    XHash::HashUpdate(&_task.ctx, addressHash, sizeof(xdag_hashlow_t));
    CRandom::FillRandomArray((uint8_t*)&_task.nonce.amount, sizeof(uint64_t));
    memcpy(_task.nonce.data, addressHash, sizeof(xdag_hashlow_t));
    memcpy(_task.lastfield.data, _task.nonce.data, sizeof(xdag_hash_t));

    //we manually set the initial target difficulty of shares
    memset(_task.minhash.data, 0xff, 24);
    _task.minhash.data[3] = 0x000008ffffffffff;

    //some precalculations on task data for GPU mining
    shamod::PrecalcState(_task.ctx.state, _task.ctx.data, _preCalcState);
    for(uint32_t i = 0; i < 14; ++i)
    {
        _reversedData[i] = bytereverse(((uint32_t*)_task.ctx.data)[i]);
    }
}

void XTaskWrapper::SetShare(xdag_hash_t last, xdag_hash_t hash)
{
    if(XHash::CompareHashes(hash, _task.minhash.data) < 0)
    {
        _shareMutex.lock();
        if(XHash::CompareHashes(hash, _task.minhash.data) < 0)
        {
            memcpy(_task.minhash.data, hash, sizeof(xdag_hash_t));
            memcpy(_task.lastfield.data, last, sizeof(xdag_hash_t));
            _isShareFound = true;
        }
        _shareMutex.unlock();
    }
}

void XTaskWrapper::DumpTask()
{
    clog(XDag::DebugChannel) << "MinHash " << HashToHexString(_task.minhash.data);
}
