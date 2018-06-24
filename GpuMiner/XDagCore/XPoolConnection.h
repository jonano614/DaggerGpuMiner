#pragma once

#include "XConnection.h"
#include "XBlock.h"
#include <functional>

struct miner
{
    uint64_t nfield_in;
    uint64_t nfield_out;
};

struct dfslib_crypt;

class XPoolConnection : public XConnection
{
private:
    xdag_hash_t _addressHash;
    miner _localMiner;
    dfslib_crypt *_crypt;

    int _readDataSize, _readDataLimit;
    xdag_field _dataBuffer[2];

    bool InitCrypto();    
public:
    XPoolConnection();
    XPoolConnection(std::string& accountAddress);
    virtual ~XPoolConnection();

    void SetAddress(std::string& accountAddress);
    virtual bool Initialize();
    virtual bool Connect(const char *address);
    uint64_t* GetAddressHash() { return _addressHash; }
    void SendWorkerName(const char* workerName);

    bool ReadTaskData(std::function<void(xdag_field*)> onNewTask);
    bool WriteTaskData(std::function<bool()> onSendTask);
    bool SendToPool(xdag_field *fields, int fieldCount);
};

