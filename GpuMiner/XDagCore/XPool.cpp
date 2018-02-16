#include "XPool.h"
#include <stdlib.h>
#include "XTime.h"
#include "Core/Log.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "XAddress.h"

#define FIRST_SHARE_SEND_TIMEOUT 10
#define BLOCK_TIME 64

XPool::XPool(std::string& accountAddress, std::string& poolAddress, XTaskProcessor *taskProcessor)
{
    strcpy(_poolAddress, poolAddress.c_str());
    _taskProcessor = taskProcessor;
    _taskTime = 0;
    _lastShareTime = 0;
    _currentConnection = NULL;
    memset(_lastHash, 0, sizeof(cheatcoin_hash_t));
    _connection.SetAddress(accountAddress);
}

XPool::~XPool()
{
    Disconnect();
}

bool XPool::Connect()
{
    if(!_connection.Initialize())
    {
        clog(XDag::LogChannel) << "Failed to initialize network connection";
        return false;
    }
    if(!_connection.Connect(_poolAddress))
    {
        return false;
    }
    _currentConnection = &_connection;
    return true;
}

void XPool::Disconnect()
{
    _connection.Close();
}

//requests new tasks from pool and sends shares if ready
bool XPool::Interract()
{
    if(!_currentConnection->IsConnected())
    {
        clog(XDag::LogChannel) << "Connection closed";
        return false;
    }

    bool success = CheckNewTasks();
    success = success && SendTaskResult();
    return success;
}

bool XPool::CheckNewTasks()
{
    return _currentConnection->ReadTaskData([this](cheatcoin_field* data) { OnNewTask(data); });
}

void XPool::OnNewTask(cheatcoin_field* data)
{
    //if fee connection is activated - we should recieve a new task from the fee connection
    if(_fee != NULL && _fee->ShouldSwitchConnection(&_currentConnection, &_connection))
    {
#ifdef _DEBUG
        std::cout << "Connection changed" << std::endl;
#endif // _DEBUG
        return;
    }

    XTaskWrapper* task = _taskProcessor->GetNextTask();
    task->FillAndPrecalc(data, _currentConnection->GetAddressHash());

    _taskProcessor->SwitchTask();
    _lastShareTime = _taskTime = time(0);

    clog(XDag::LogChannel) << string_format("New task: t=%llx N=%llu", task->GetTask()->main_time << 16 | 0xffff, _taskProcessor->GetCount());
#ifdef _TEST_TASKS
    _taskProcessor->DumpTasks();
#endif // _DEBUG
#ifdef _DEBUG
    std::cout << "State:" << std::endl;
    DumpHex((uint8_t*)task->GetTask()->ctx.state, 32);
    std::cout << "Data:" << std::endl;
    DumpHex(task->GetTask()->ctx.data, 56);
    std::cout << "Start nonce: " << task->GetTask()->lastfield.amount << std::endl;
    std::cout << "Start minhash:" << std::endl;
    std::cout << HashToHexString(task->GetTask()->minhash.data) << std::endl;
#endif // _DEBUG
}

bool XPool::SendTaskResult()
{
    if(!HasNewShare())
    {
        return true;
    }
    return _currentConnection->WriteTaskData([this]()
    {
        cheatcoin_pool_task *task = _taskProcessor->GetCurrentTask()->GetTask();
        uint64_t *hash = task->minhash.data;
        _lastShareTime = time(0);
        memcpy(_lastHash, hash, sizeof(cheatcoin_hash_t));
        bool res = _currentConnection->SendToPool(&task->lastfield, 1);
        clog(XDag::LogChannel) << string_format("Share t=%llx res=%s\n%016llx%016llx%016llx%016llx",
            task->main_time << 16 | 0xffff, res ? "OK" : "Fail", hash[3], hash[2], hash[1], hash[0]);
#ifdef _DEBUG
        std::cout << "Address: " << XAddress::HashToAddress(_currentConnection->GetAddressHash()) << std::endl;
#endif // _DEBUG
        if(!res)
        {
            clog(XDag::LogChannel) << "Failed to send task result";
            return false;
        }
        return true;
    });    
}

bool XPool::HasNewShare()
{
    if(_taskProcessor->GetCurrentTask() == NULL || !_taskProcessor->GetCurrentTask()->IsShareFound())
    {
        return false;
    }
    time_t currentTime = time(0);
    if(currentTime - _taskTime < FIRST_SHARE_SEND_TIMEOUT)
    {
        return false;
    }
    //There is no sense to send the same results
    return memcmp(_lastHash, _taskProcessor->GetCurrentTask()->GetTask()->minhash.data, sizeof(cheatcoin_hash_t)) != 0;
}
