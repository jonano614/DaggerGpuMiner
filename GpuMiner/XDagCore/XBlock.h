#pragma once

#include "XHash.h"
#include "XTime.h"

#define CHEATCOIN_BLOCK_FIELDS 16

enum class cheatcoin_field_type
{
    CHEATCOIN_FIELD_NONCE,
    CHEATCOIN_FIELD_HEAD,
    CHEATCOIN_FIELD_IN,
    CHEATCOIN_FIELD_OUT,
    CHEATCOIN_FIELD_SIGN_IN,
    CHEATCOIN_FIELD_SIGN_OUT,
    CHEATCOIN_FIELD_PUBLIC_KEY_0,
    CHEATCOIN_FIELD_PUBLIC_KEY_1,
};

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

class XBlock
{
public:
    static void GenerateFakeBlock(cheatcoin_block *block);
};
