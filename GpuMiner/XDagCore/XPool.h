#pragma once

#include "XPoolConnection.h"
#include "XBlock.h"
#include "XTaskProcessor.h"
#include "XFee.h"

class XPool
{
private:
    char _poolAddress[256];
    XPoolConnection _connection;
    XPoolConnection* _currentConnection;
    XTaskProcessor *_taskProcessor;
    cheatcoin_hash_t _lastHash;
    time_t _taskTime;
    time_t _lastShareTime;
    XFee* _fee;   

    bool CheckNewTasks();
    bool SendTaskResult();
    void OnNewTask(cheatcoin_field* data);    
    bool HasNewShare();
public:
    XPool(std::string& accountAddress, std::string& poolAddress, XTaskProcessor *taskProcessor);
    virtual ~XPool();

    void SetFee(XFee* fee) { _fee = fee; }
    bool Connect();
    void Disconnect();
    bool Interract();
};
