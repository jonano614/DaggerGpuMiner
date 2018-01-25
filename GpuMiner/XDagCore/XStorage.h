#pragma once

#include <stdint.h>
#include "XHash.h"

#define CHEATCOIN_BLOCK_FIELDS 16

typedef uint64_t cheatcoin_time_t;
typedef uint64_t cheatcoin_amount_t;

struct cheatcoin_field
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
                    cheatcoin_time_t time;
                };
                cheatcoin_hashlow_t hash;
            };
            union
            {
                cheatcoin_amount_t amount;
                cheatcoin_time_t end_time;
            };
        };
        cheatcoin_hash_t data;
    };
};

struct cheatcoin_block
{
    struct cheatcoin_field field[CHEATCOIN_BLOCK_FIELDS];
};

class XStorage
{
public:
    static cheatcoin_time_t GetMainTime();
    static bool GetFirstBlock(cheatcoin_block *firstBlock);
    static bool CheckStorageFolder();
};

