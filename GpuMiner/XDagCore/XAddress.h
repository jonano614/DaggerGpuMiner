#pragma once

#include "XHash.h"

class XAddress
{
private:
    uint8_t _mime2bits[256];

    void Init();
public:
    XAddress();
    ~XAddress();

    bool AddressToHash(const char* address, cheatcoin_hash_t hash);
    const char* HashToAddress(const cheatcoin_hash_t hash);
};

