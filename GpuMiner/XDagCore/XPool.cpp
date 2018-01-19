#include "XPool.h"
#include <stdlib.h>
#include "dfstools\dfslib_string.h"
#include "dar\crc.h"
#include "XAddress.h"
#include "Core\Log.h"
#include "Utils\Random.h"
#include "Utils\StringFormat.h"

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
}

XPool::~XPool()
{
    if (!_crypt)
    {
        free(_crypt);
        _crypt = NULL;
    }
}

bool XPool::SendToPool(cheatcoin_field *fields, int fieldCount)
{
    cheatcoin_field fieldsCopy[CHEATCOIN_BLOCK_FIELDS];
    cheatcoin_hash_t hash;
    int todo = fieldCount * sizeof(struct cheatcoin_field), done = 0;
    if (!_network.IsConnected())
    {
        return false;
    }
    memcpy(fieldsCopy, fields, todo);
    if (fieldCount == CHEATCOIN_BLOCK_FIELDS)
    {
        uint32_t crc;
        fieldsCopy[0].transport_header = 0;
        XHash::GetHash(fieldsCopy, sizeof(struct cheatcoin_block), hash);
        fieldsCopy[0].transport_header = HEADER_WORD;
        crc = crc_of_array((uint8_t *)fieldsCopy, sizeof(struct cheatcoin_block));
        fieldsCopy[0].transport_header |= (uint64_t)crc << 32;
    }
    for (int i = 0; i < fieldCount; ++i)
    {
        dfslib_encrypt_array(_crypt, (uint32_t *)(fieldsCopy + i), DATA_SIZE, _localMiner.nfield_out++);
    }
    while (todo)
    {
        pollfd p;
        p.fd = _network.GetSocket();
        p.events = POLLOUT;
        if (!_network.Poll(&p, 1, 1000))
        {
            continue;
        }
        if (p.revents & (POLLHUP | POLLERR))
        {
            return false;
        }
        if (!(p.revents & POLLOUT))
        {
            continue;
        }
        int res = _network.Write((char*)fieldsCopy + done, todo);
        if (res <= 0)
        {
            return false;
        }
        done += res, todo -= res;
    }

    if (fieldCount == CHEATCOIN_BLOCK_FIELDS)
    {
        clog(XDag::LogChannel) << string_format("Sent block info t=%llx res=OK\n%016llx%016llx%016llx%016llx",
            fields[0].time, hash[3], hash[2], hash[1], hash[0]);
    }
    return true;
}

bool XPool::Initialize()
{
    if (!_network.Initialize() || !InitCrypto())
    {
        return false;
    }

    if (!XBlock::GetFirstBlock(&_firstBlock))
    {
        return false;
    }

    crc_init();
    return true;
}

bool XPool::InitCrypto()
{
    dfslib_string str;
    uint32_t sector[128];
    int i;
    _crypt = (dfslib_crypt*)malloc(sizeof(struct dfslib_crypt));
    if (!_crypt)
    {
        return false;
    }
    dfslib_crypt_set_password(_crypt, dfslib_utf8_string(&str, MINERS_PWD, strlen(MINERS_PWD)));
    for (i = 0; i < 128; ++i)
    {
        sector[i] = SECTOR0_BASE + i * SECTOR0_OFFSET;
    }
    for (i = 0; i < 128; ++i)
    {
        dfslib_crypt_set_sector0(_crypt, sector);
        dfslib_encrypt_sector(_crypt, sector, SECTOR0_BASE + i * SECTOR0_OFFSET);
    }
    return true;
}

bool XPool::Connect()
{
    if (!_network.Connect(_poolAddress))
    {
        return false;
    }
    //as far as I understand it is necessary for miner identification
    if (!SendToPool(_firstBlock.field, CHEATCOIN_BLOCK_FIELDS))
    {
        return false;
    }
    return true;
}

void XPool::Disconnect()
{
    _network.Close();
}

bool XPool::Interract()
{
    cheatcoin_field data[2];
    for (;;)
    {
        pollfd p;
        if (!_network.IsConnected())
        {
            //mess = "socket is closed";
            return false;
        }
        p.fd = _network.GetSocket();
        p.events = POLLIN | (HasNewShare() ? POLLOUT : 0);
        if (!_network.Poll(&p, 1, 0))
        {
            break;
        }
        if (p.revents & POLLHUP)
        {
            //mess = "socket hangup";
            return false;
        }
        if (p.revents & POLLERR)
        {
            //mess = "socket error";
            return false;
        }
        if (p.revents & POLLIN)
        {
            int res = _network.Read((char*)data + _ndata, _maxndata - _ndata);
            if (res < 0)
            {
                //mess = "read error on socket";
                return false;
            }
            _ndata += res;
            if (_ndata == _maxndata)
            {
                cheatcoin_field *last = data + (_ndata / sizeof(struct cheatcoin_field) - 1);
                dfslib_uncrypt_array(_crypt, (uint32_t *)last->data, DATA_SIZE, _localMiner.nfield_in++);
                if (_maxndata == 2 * sizeof(struct cheatcoin_field))
                {
                    OnNewTask(data);
                }
                else
                {
                    _maxndata = 2 * sizeof(struct cheatcoin_field);
                }
            }
        }
        if (p.revents & POLLOUT)
        {
            if (!SendTaskResult())
            {
                return false;
            }
        }
    }
    return true;
}

void XPool::OnNewTask(cheatcoin_field* data)
{
    cheatcoin_pool_task *task = _taskProcessor->GetNextTask()->GetTask();
    task->main_time = XBlock::GetMainTime();

    XHash::SetHashState(&task->ctx, data[0].data, sizeof(struct cheatcoin_block) - 2 * sizeof(struct cheatcoin_field));

    XHash::HashUpdate(&task->ctx, data[1].data, sizeof(struct cheatcoin_field));
    XHash::HashUpdate(&task->ctx, _addressHash, sizeof(cheatcoin_hashlow_t));
    CRandom::FillRandomArray((uint8_t*)task->nonce.data, sizeof(cheatcoin_hash_t));
    memcpy(task->nonce.data, _addressHash, sizeof(cheatcoin_hashlow_t));
    memcpy(task->lastfield.data, task->nonce.data, sizeof(cheatcoin_hash_t));
    XHash::HashFinal(&task->ctx, &task->nonce.amount, sizeof(uint64_t), task->minhash.data);

    _taskProcessor->SwitchTask();
    _taskTime = time(0);

    clog(XDag::LogChannel) << string_format("New task: t=%llx N=%llu", task->main_time << 16 | 0xffff, _taskProcessor->GetCount());
#if _TEST_TASKS
    _taskProcessor->DumpTasks();
#endif
    _ndata = 0;
    _maxndata = sizeof(struct cheatcoin_field);
}

bool XPool::SendTaskResult()
{
    cheatcoin_pool_task *task = _taskProcessor->GetCurrentTask()->GetTask();
    uint64_t *hash = task->minhash.data;
    _lastShareTime = time(0);
    memcpy(_lastHash, hash, sizeof(cheatcoin_hash_t));
    bool res = SendToPool(&task->lastfield, 1);
    clog(XDag::LogChannel) << string_format("Share t=%llx res=%s\n%016llx%016llx%016llx%016llx",
       task->main_time << 16 | 0xffff, res ? "OK" : "Fail", hash[3], hash[2], hash[1], hash[0]);
    if (!res)
    {
        //mess = "write error on socket";
        return false;
    }
    return true;
}

bool XPool::HasNewShare()
{
    if (_taskProcessor->GetCurrentTask() == NULL)
    {
        return false;
    }
    time_t currentTime = time(0);
    if (currentTime - _lastShareTime < SEND_PERIOD || currentTime - _taskTime > 64)
    {
        return false;
    }
    //There is no sense to send the same results
    return XHash::CompareHashes(_lastHash, _taskProcessor->GetCurrentTask()->GetTask()->minhash.data) != 0;
}