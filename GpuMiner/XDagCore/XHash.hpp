#include <string.h>

int XHash::CompareHashes(cheatcoin_hash_t l, cheatcoin_hash_t r)
{
    for(int i = 3; i >= 0; --i)
    {
        if(l[i] != r[i])
        {
            return (l[i] < r[i] ? -1 : 1);
        }
    }
    return 0;
}

void XHash::HashInit(SHA256_CTX *ctx)
{
    sha256_init(ctx);
}

void XHash::HashUpdate(SHA256_CTX *ctx, void *data, size_t size)
{
    sha256_update(ctx, (uint8_t*)data, size);
}

inline void XHash::GetHashState(SHA256_CTX *ctx, cheatcoin_hash_t state)
{
    memcpy(state, ctx->state, sizeof(cheatcoin_hash_t));
}

inline void XHash::SetHashState(SHA256_CTX *ctx, cheatcoin_hash_t state, size_t size)
{
    memcpy(ctx->state, state, sizeof(cheatcoin_hash_t));
    ctx->datalen = 0;
    ctx->bitlen = size << 3;
}
