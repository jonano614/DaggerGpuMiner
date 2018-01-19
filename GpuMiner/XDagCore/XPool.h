#pragma once

#include "XNetwork.h"
#include "XBlock.h"
#include "XTaskProcessor.h"
#include "dfstools/dfslib_crypt.h"

#define CHEATCOIN_POOL_N_CONFIRMATIONS	16
#define N_MINERS		4096
#define START_N_MINERS	256
#define START_N_MINERS_IP 8
#define N_CONFIRMATIONS	CHEATCOIN_POOL_N_CONFIRMATIONS
#define MINERS_PWD		"minersgonnamine"
#define SECTOR0_BASE	0x1947f3acu
#define SECTOR0_OFFSET	0x82e9d1b5u
#define HEADER_WORD		0x3fca9e2bu
#define DATA_SIZE		(sizeof(struct cheatcoin_field) / sizeof(uint32_t))
#define SEND_PERIOD		10      //result send period

struct miner
{
    double maxdiff[N_CONFIRMATIONS];
    cheatcoin_field id;
    uint32_t data[DATA_SIZE];
    double prev_diff;
    cheatcoin_time_t main_time;
    uint64_t nfield_in;
    uint64_t nfield_out;
    uint64_t ntask;
    cheatcoin_block *block;
    uint32_t ip;
    uint16_t port;
    uint16_t state;
    uint8_t data_size;
    uint8_t block_size;
};
struct cheatcoin_block;

class XPool
{
private:
    cheatcoin_hash_t _addressHash;
    char _poolAddress[256];
    XNetwork _network;
    miner _localMiner;
    XTaskProcessor *_taskProcessor;
    //TODO: move out of here
    dfslib_crypt *_crypt;
    cheatcoin_hash_t _lastHash;
    time_t _taskTime;
    time_t _lastShareTime;
    //TODO: the purpose of these properties is unclear for me now, just copy-paste...
    int _ndata, _maxndata;
    cheatcoin_block _firstBlock;

    bool SendToPool(cheatcoin_field *fields, int fieldCount);
    bool InitCrypto();

    void OnNewTask(cheatcoin_field* data);
    bool SendTaskResult();
    bool HasNewShare();
public:
    XPool(std::string& accountAddress, std::string& poolAddress, XTaskProcessor *taskProcessor);
    virtual ~XPool();

    bool Initialize();
    bool Connect();
    void Disconnect();
    bool Interract();
};

