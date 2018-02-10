/* хеш-функция, T13.654-T13.775 $DVS:time$ */

#ifndef XDAG_HASH_H
#define XDAG_HASH_H

#include <stdint.h>
#include <string>
#include "Hash/sha256.h"

typedef uint64_t cheatcoin_hash_t[4];
typedef uint64_t cheatcoin_hashlow_t[3];

class XHash
{
public:
    static void GetHash(void *data, size_t size, cheatcoin_hash_t hash);
    static inline int CompareHashes(cheatcoin_hash_t l, cheatcoin_hash_t r);
    static int GetHashCtxSize() { return sizeof(SHA256_CTX); }
    static inline void HashInit(SHA256_CTX *ctx);
    static inline void HashUpdate(SHA256_CTX *ctx, void *data, size_t size);

    static uint64_t SearchMinNonce(SHA256_CTX *ctx, uint64_t &nonce, int iterations, int step, cheatcoin_hash_t hash);
    static inline void GetHashState(SHA256_CTX *ctxv, cheatcoin_hash_t state);
    static inline void SetHashState(SHA256_CTX *ctxv, cheatcoin_hash_t state, size_t size);
};

#include "XHash.hpp"

#endif
