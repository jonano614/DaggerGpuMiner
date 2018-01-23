/* хеш-функция, T13.654-T13.775 $DVS:time$ */

#include <string.h>
#include "XHash.h"
#include "Utils\StringFormat.h"
#include "Hash\sha256_mod.h"

void XHash::GetHash(void *data, size_t size, cheatcoin_hash_t hash)
{
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (uint8_t*)data, size);
    sha256_final(&ctx, (uint8_t *)hash);
    sha256_init(&ctx);
    sha256_update(&ctx, (uint8_t *)hash, sizeof(cheatcoin_hash_t));
    sha256_final(&ctx, (uint8_t *)hash);
}

uint64_t XHash::SearchMinNonce(SHA256_CTX *ctx, uint64_t &nonce, int iterations, int step, cheatcoin_hash_t hash)
{
    cheatcoin_hash_t currentHash;
    uint64_t minNonce = 0;
    for(int i = 0; i < iterations; ++i)
    {
        shamod::shasha(ctx->state, nonce, (uint8_t*)hash);
        if(!i || CompareHashes(currentHash, hash) < 0)
        {
            memcpy(hash, currentHash, sizeof(cheatcoin_hash_t));
            minNonce = nonce;
        }
        nonce += step;
    }
    return minNonce;
}

std::string HashToHex(const cheatcoin_hash_t& hash)
{
    return string_format("%016llx%016llx%016llx%016llx", hash[3], hash[2], hash[1], hash[0]);
}