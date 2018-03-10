#pragma once

#include "XHash.h"

class XAddress
{
private:
    static uint8_t _mime2bits[256];
public:
    static void Init();
    static bool AddressToHash(const char* address, cheatcoin_hash_t hash);
    static const char* HashToAddress(const cheatcoin_hash_t hash);
};

