// Common types of block parts
// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#pragma once

#include "XHash.h"
#include "XTime.h"

#define XDAG_BLOCK_FIELDS 16

enum class xdag_field_type
{
    XDAG_FIELD_NONCE,
    XDAG_FIELD_HEAD,
    XDAG_FIELD_IN,
    XDAG_FIELD_OUT,
    XDAG_FIELD_SIGN_IN,
    XDAG_FIELD_SIGN_OUT,
    XDAG_FIELD_PUBLIC_KEY_0,
    XDAG_FIELD_PUBLIC_KEY_1,
};

typedef uint64_t xdag_amount_t;

struct xdag_field
{
    union
    {
        struct
        {
            union
            {
                struct
                {
                    uint64_t transport_header;
                    uint64_t type;
                    xdag_time_t time;
                };
                xdag_hashlow_t hash;
            };
            union
            {
                xdag_amount_t amount;
                xdag_time_t end_time;
            };
        };
        xdag_hash_t data;
    };
};

struct xdag_block
{
    struct xdag_field field[XDAG_BLOCK_FIELDS];
};

class XBlock
{
public:
    static void GenerateFakeBlock(xdag_block *block);
};
