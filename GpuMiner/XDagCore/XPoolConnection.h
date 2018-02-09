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
    cheatcoin_hash_t _addressHash;
    miner _localMiner;
    dfslib_crypt *_crypt;
    int _ndata, _maxndata;
    cheatcoin_block _firstBlock;

    bool InitCrypto();    
public:
    XPoolConnection();
    XPoolConnection(std::string& accountAddress);
    virtual ~XPoolConnection();

    void SetAddress(std::string& accountAddress);
    virtual bool Initialize();
    virtual bool Connect(const char *address);
    uint64_t* GetAddressHash() { return _addressHash; }

    bool ReadTaskData(std::function<void(cheatcoin_field*)> onNewTask, bool& noData);
    bool WriteTaskData(std::function<bool()> onSendTask);
    bool SendToPool(cheatcoin_field *fields, int fieldCount);
};

