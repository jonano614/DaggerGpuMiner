// Hashing operations
// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#ifndef XDAG_HASH_H
#define XDAG_HASH_H

#include <stdint.h>
#include <string>
#include "Hash/sha256.h"

typedef uint64_t xdag_hash_t[4];
typedef uint64_t xdag_hashlow_t[3];

class XHash
{
public:
    static void GetHash(void *data, size_t size, xdag_hash_t hash);
    static inline int CompareHashes(xdag_hash_t l, xdag_hash_t r);
    static int GetHashCtxSize() { return sizeof(SHA256_CTX); }
    static inline void HashInit(SHA256_CTX *ctx);
    static inline void HashUpdate(SHA256_CTX *ctx, void *data, size_t size);

    static uint64_t SearchMinNonce(SHA256_CTX *ctx, uint64_t &nonce, int iterations, int step, xdag_hash_t hash);
    static inline void GetHashState(SHA256_CTX *ctxv, xdag_hash_t state);
    static inline void SetHashState(SHA256_CTX *ctxv, xdag_hash_t state, size_t size);
};

#include "XHash.hpp"

#endif
