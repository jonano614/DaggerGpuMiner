#pragma once

#include <stdint.h>

#define SHA256_BLOCK_SIZE 32     

namespace shamod
{
    //modified version of standard SHA256D, optimized special for mining
    void shasha(const uint32_t* state, const uint8_t* data, uint64_t nonce, uint8_t *hash);

    //makes first 14 rounds on input data
    void PrecalcState(const uint32_t* state, const uint8_t* data, uint32_t* precalcedState);
}
