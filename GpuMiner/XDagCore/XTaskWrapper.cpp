#include "XTaskWrapper.h"
#include "Core/Log.h"
#include "Utils/Utils.h"

XTaskWrapper::XTaskWrapper()
    :_taskIndex(0)
{
}

XTaskWrapper::~XTaskWrapper()
{
}

void XTaskWrapper::SetShare(cheatcoin_hash_t last, cheatcoin_hash_t hash)
{
    if(XHash::CompareHashes(hash, _task.minhash.data) < 0)
    {
        _shareMutex.lock();
        if(XHash::CompareHashes(hash, _task.minhash.data) < 0)
        {
            memcpy(_task.minhash.data, hash, sizeof(cheatcoin_hash_t));
            memcpy(_task.lastfield.data, last, sizeof(cheatcoin_hash_t));
        }
        _shareMutex.unlock();
    }
}

void XTaskWrapper::DumpTask()
{
    clog(XDag::DebugChannel) << "MinHash " << HashToHexString(_task.minhash.data);
}
