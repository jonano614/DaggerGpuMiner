#include "XPoolConnection.h"
#include "XAddress.h"
#include "XBlock.h"
#include "Core/Log.h"
#include "dfstools/dfslib_crypt.h"
#include "dar/crc.h"

#define SECTOR0_BASE	0x1947f3acu
#define SECTOR0_OFFSET	0x82e9d1b5u
#define HEADER_WORD		0x3fca9e2bu
#define MINERS_PWD		"minersgonnamine"
#define DATA_SIZE		(sizeof(struct xdag_field) / sizeof(uint32_t))

XPoolConnection::XPoolConnection()
{
    _localMiner.nfield_in = 0;
    _localMiner.nfield_out = 0;
}

XPoolConnection::XPoolConnection(std::string& accountAddress)
{
    SetAddress(accountAddress);
}

XPoolConnection::~XPoolConnection()
{
    if(!_crypt)
    {
        free(_crypt);
        _crypt = NULL;
    }
}

void XPoolConnection::SetAddress(std::string& accountAddress)
{
    XAddress::AddressToHash(accountAddress.c_str(), _addressHash);
}

bool XPoolConnection::InitCrypto()
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

bool XPoolConnection::Initialize()
{    
    if(!InitCrypto())
    {
        clog(XDag::LogChannel) << "Failed to initialize cryptography system";
        return false;
    }
    return XConnection::Initialize();
}

bool XPoolConnection::Connect(const char *address)
{
    _localMiner.nfield_in = 0;
    _localMiner.nfield_out = 0;
    _readDataSize = 0;
    _readDataLimit = sizeof(struct xdag_field);
    if(!XConnection::Connect(address))
    {
        return false;
    }
    xdag_block block;
    XBlock::GenerateFakeBlock(&block);
    if(!SendToPool(block.field, XDAG_BLOCK_FIELDS))
    {
        return false;
    }
    return true;
}

bool XPoolConnection::SendToPool(xdag_field *fields, int fieldCount)
{
    xdag_field fieldsCopy[XDAG_BLOCK_FIELDS];
    xdag_hash_t hash;
    int todo = fieldCount * sizeof(struct xdag_field), done = 0;
    if(!IsConnected())
    {
        return false;
    }
    memcpy(fieldsCopy, fields, todo);
    if(fieldCount == XDAG_BLOCK_FIELDS)
    {
        uint32_t crc;
        fieldsCopy[0].transport_header = 0;
        XHash::GetHash(fieldsCopy, sizeof(struct xdag_block), hash);
        fieldsCopy[0].transport_header = HEADER_WORD;
        crc = crc_of_array((uint8_t *)fieldsCopy, sizeof(struct xdag_block));
        fieldsCopy[0].transport_header |= (uint64_t)crc << 32;
    }
    for(int i = 0; i < fieldCount; ++i)
    {
        dfslib_encrypt_array(_crypt, (uint32_t *)(fieldsCopy + i), DATA_SIZE, _localMiner.nfield_out++);
    }
    while(todo)
    {
        bool success;
        bool isReady = IsReady(NetworkAction::Write, 1000, success);
        if(!success)
        {
            return false;
        }
        if(!isReady)
        {
            continue;
        }
        int res = Write((char*)fieldsCopy + done, todo);
        if(res <= 0)
        {
            return false;
        }
        done += res, todo -= res;
    }
    return true;
}

bool XPoolConnection::ReadTaskData(std::function<void(xdag_field*)> onNewTask)
{
    xdag_field recievedTaskData[2];
    bool taskIsRecieved = false;
    do
    {
        bool success;
        // TODO: think about exceptions instead of failure flag
        bool isReady = IsReady(NetworkAction::Read, 0, success);
        if(!success)
        {
            return false;
        }
        if(!isReady)
        {
            break;
        }
        int res = Read((char*)_dataBuffer + _readDataSize, _readDataLimit - _readDataSize);
        if(res <= 0)
        {
            clog(XDag::LogChannel) << "Failed to read data from pool";
            return false;
        }
        _readDataSize += res;
        if(_readDataSize == _readDataLimit)
        {
            xdag_field *last = _dataBuffer + (_readDataSize / sizeof(struct xdag_field) - 1);
            dfslib_uncrypt_array(_crypt, (uint32_t *)last->data, DATA_SIZE, _localMiner.nfield_in++);
            if(!memcmp(last->data, _addressHash, sizeof(xdag_hashlow_t)))
            {
                // if returned data contains hash of account address - pool sent information about incoming transfer
                // we just ignore it
                _readDataSize = 0;
                _readDataLimit = sizeof(struct xdag_field);
            }
            else if(_readDataLimit == 2 * sizeof(struct xdag_field))
            {
                memcpy(recievedTaskData, _dataBuffer, _readDataLimit);
                taskIsRecieved = true;
                _readDataSize = 0;
                _readDataLimit = sizeof(struct xdag_field);
            }
            else
            {
                _readDataLimit = 2 * sizeof(struct xdag_field);
            }
        }
    }
    while(true);

    if(taskIsRecieved)
    {
        onNewTask(recievedTaskData);
    }

    return true;
}

bool XPoolConnection::WriteTaskData(std::function<bool()> onSendTask)
{
    bool success;
    bool isReady = IsReady(NetworkAction::Write, 0, success);
    if(!success)
    {
        return false;
    }
    if(!isReady)
    {
        return true;
    }

    return onSendTask();
}
