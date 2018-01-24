#pragma once

#include <stdint.h>

#define SHA256_BLOCK_SIZE 32     

namespace shamod
{
    void shasha(uint32_t* state, const uint8_t* data, uint64_t nonce, uint8_t *hash);
}
