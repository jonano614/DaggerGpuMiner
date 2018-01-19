#include "XTaskWrapper.h"
#include "Core\Log.h"

XTaskWrapper::XTaskWrapper()
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
    clog(XDag::DebugChannel) << "MinHash " << HashToHex(_task.minhash.data);
}
