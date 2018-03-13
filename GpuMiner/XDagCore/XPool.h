#pragma once

#include "XPoolConnection.h"
#include "XBlock.h"
#include "XTaskProcessor.h"

class XPool
{
private:
    char _poolAddress[256];
    XPoolConnection _connection;
    XPoolConnection* _currentConnection;
    XTaskProcessor *_taskProcessor;
    xdag_hash_t _lastHash;
    time_t _taskTime;
    time_t _lastShareTime;

    bool CheckNewTasks();
    bool SendTaskResult();
    void OnNewTask(xdag_field* data);    
    bool HasNewShare();
public:
    XPool(std::string& accountAddress, std::string& poolAddress, XTaskProcessor *taskProcessor);
    virtual ~XPool();

    bool Connect();
    void Disconnect();
    bool Interract();
};
