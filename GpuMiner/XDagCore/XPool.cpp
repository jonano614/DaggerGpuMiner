#include "XPool.h"
#include <stdlib.h>
#include "dfstools/dfslib_string.h"
#include "dar/crc.h"
#include "XAddress.h"
#include "XTime.h"
#include "Core/Log.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"

#define FIRST_SHARE_SEND_TIMEOUT 10
#define BLOCK_TIME 64

XPool::XPool(std::string& accountAddress, std::string& poolAddress, XTaskProcessor *taskProcessor)
{
    strcpy(_poolAddress, poolAddress.c_str());
    _taskProcessor = taskProcessor;
    _ndata = 0;
    _maxndata = sizeof(struct cheatcoin_field);
    _localMiner.nfield_in = 0;
    _localMiner.nfield_out = 0;
    _taskTime = 0;
    _lastShareTime = 0;
    memset(_lastHash, 0, sizeof(cheatcoin_hash_t));

    XAddress address;
    address.AddressToHash(accountAddress.c_str(), _addressHash);
    memcpy(_localMiner.id.data, _addressHash, sizeof(cheatcoin_hash_t));
}

XPool::~XPool()
{
    if(!_crypt)
    {
        free(_crypt);
        _crypt = NULL;
    }
}

bool XPool::Initialize()
{
    if(!_connection.Initialize())
    {
        clog(XDag::LogChannel) << "Failed to initialize network connection";
    }
    if(!InitCrypto())
    {
        clog(XDag::LogChannel) << "Failed to initialize cryptography system";
        return false;
    }

    XBlock::GenerateFakeBlock(&_firstBlock);
    crc_init();
    return true;
}

bool XPool::InitCrypto()
{
    dfslib_string str;
    uint32_t sector[128];
    _crypt = (dfslib_crypt*)malloc(sizeof(struct dfslib_crypt));
    if(!_crypt)
    {
        return false;
    }
    dfslib_crypt_set_password(_crypt, dfslib_utf8_string(&str, MINERS_PWD, (uint32_t)strlen(MINERS_PWD)));
    for(int i = 0; i < 128; ++i)
    {
        sector[i] = SECTOR0_BASE + i * SECTOR0_OFFSET;
    }
    for(int i = 0; i < 128; ++i)
    {
        dfslib_crypt_set_sector0(_crypt, sector);
        dfslib_encrypt_sector(_crypt, sector, SECTOR0_BASE + i * SECTOR0_OFFSET);
    }
    return true;
}

bool XPool::Connect()
{
    _localMiner.nfield_in = 0;
    _localMiner.nfield_out = 0;
    _ndata = 0;
    _maxndata = sizeof(struct cheatcoin_field);

    if(!_connection.Connect(_poolAddress))
    {
        return false;
    }
    //as far as I understand it is necessary for miner identification
    if(!SendToPool(_firstBlock.field, CHEATCOIN_BLOCK_FIELDS))
    {
        return false;
    }
    return true;
}

void XPool::Disconnect()
{
    _connection.Close();
}

//requests new tasks from pool and sends shares if ready
bool XPool::Interract()
{
    if(!_connection.IsConnected())
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
    cheatcoin_field data[2];
    for(;;)
    {
        bool success;
        // TODO: think about exceptions instead of failure flag
        bool isReady = _connection.IsReady(NetworkAction::Read, 0, success);
        if(!success)
        {
            return false;
        }
        if(!isReady)
        {
            break;
        }
        int res = _connection.Read((char*)data + _ndata, _maxndata - _ndata);
        if(res < 0)
        {
            clog(XDag::LogChannel) << "Failed to read data from pool";
            return false;
        }
        _ndata += res;
        if(_ndata == _maxndata)
        {
            cheatcoin_field *last = data + (_ndata / sizeof(struct cheatcoin_field) - 1);
            dfslib_uncrypt_array(_crypt, (uint32_t *)last->data, DATA_SIZE, _localMiner.nfield_in++);
            if(!memcmp(last->data, _addressHash, sizeof(cheatcoin_hashlow_t)))
            {
                // if returned data contains hash of account address - pool sent information about incoming transfer
                // we just ignore it
                _ndata = 0;
                _maxndata = sizeof(struct cheatcoin_field);
            }
            else if(_maxndata == 2 * sizeof(struct cheatcoin_field))
            {
                OnNewTask(data);
            }
            else
            {
                _maxndata = 2 * sizeof(struct cheatcoin_field);
            }
        }
    }
    return true;
}

void XPool::OnNewTask(cheatcoin_field* data)
{
    XTaskWrapper* task = _taskProcessor->GetNextTask();
    task->FillAndPrecalc(data, _addressHash);

    _taskProcessor->SwitchTask();
    _lastShareTime = _taskTime = time(0);

    clog(XDag::LogChannel) << string_format("New task: t=%llx N=%llu", task->GetTask()->main_time << 16 | 0xffff, _taskProcessor->GetCount());
#if _TEST_TASKS
    _taskProcessor->DumpTasks();
#endif
    _ndata = 0;
    _maxndata = sizeof(struct cheatcoin_field);

#if _DEBUG
    std::cout << "State:" << std::endl;
    DumpHex((uint8_t*)task->GetTask()->ctx.state, 32);
    std::cout << "Data:" << std::endl;
    DumpHex(task->GetTask()->ctx.data, 56);
    std::cout << "Start nonce: " << task->GetTask()->lastfield.amount << std::endl;
    std::cout << "Start minhash:" << std::endl;
    DumpHex((uint8_t*)task->GetTask()->minhash.data, 32);
    std::cout << HashToHexString(task->GetTask()->minhash.data) << std::endl;
#endif
}

bool XPool::SendTaskResult()
{
    if(!HasNewShare())
    {
        return true;
    }
    bool success;
    bool isReady = _connection.IsReady(NetworkAction::Write, 0, success);
    if(!success)
    {
        return false;
    }
    if(!isReady)
    {
        return true;
    }

    cheatcoin_pool_task *task = _taskProcessor->GetCurrentTask()->GetTask();
    uint64_t *hash = task->minhash.data;
    _lastShareTime = time(0);
    memcpy(_lastHash, hash, sizeof(cheatcoin_hash_t));
    bool res = SendToPool(&task->lastfield, 1);
    clog(XDag::LogChannel) << string_format("Share t=%llx res=%s\n%016llx%016llx%016llx%016llx",
        task->main_time << 16 | 0xffff, res ? "OK" : "Fail", hash[3], hash[2], hash[1], hash[0]);
    if(!res)
    {
        clog(XDag::LogChannel) << "Failed to send task result";
        return false;
    }
    return true;
}

bool XPool::SendToPool(cheatcoin_field *fields, int fieldCount)
{
    cheatcoin_field fieldsCopy[CHEATCOIN_BLOCK_FIELDS];
    cheatcoin_hash_t hash;
    int todo = fieldCount * sizeof(struct cheatcoin_field), done = 0;
    if(!_connection.IsConnected())
    {
        return false;
    }
    memcpy(fieldsCopy, fields, todo);
    if(fieldCount == CHEATCOIN_BLOCK_FIELDS)
    {
        uint32_t crc;
        fieldsCopy[0].transport_header = 0;
        XHash::GetHash(fieldsCopy, sizeof(struct cheatcoin_block), hash);
        fieldsCopy[0].transport_header = HEADER_WORD;
        crc = crc_of_array((uint8_t *)fieldsCopy, sizeof(struct cheatcoin_block));
        fieldsCopy[0].transport_header |= (uint64_t)crc << 32;
    }
    for(int i = 0; i < fieldCount; ++i)
    {
        dfslib_encrypt_array(_crypt, (uint32_t *)(fieldsCopy + i), DATA_SIZE, _localMiner.nfield_out++);
    }
    while(todo)
    {
        bool success;
        bool isReady = _connection.IsReady(NetworkAction::Write, 1000, success);
        if(!success)
        {
            return false;
        }
        if(!isReady)
        {
            continue;
        }
        int res = _connection.Write((char*)fieldsCopy + done, todo);
        if(res <= 0)
        {
            return false;
        }
        done += res, todo -= res;
    }

    if(fieldCount == CHEATCOIN_BLOCK_FIELDS)
    {
        clog(XDag::LogChannel) << string_format("Sent block info t=%llx res=OK\n%016llx%016llx%016llx%016llx",
            fields[0].time, hash[3], hash[2], hash[1], hash[0]);
    }
    return true;
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
