// Implementation of conversion between hash and XDAG address
// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#pragma once

#include "XHash.h"

class XAddress
{
private:
    static uint8_t _mime2bits[256];
public:
    static void Init();
    static bool AddressToHash(const char* address, xdag_hash_t hash);
    static const char* HashToAddress(const xdag_hash_t hash);
};

