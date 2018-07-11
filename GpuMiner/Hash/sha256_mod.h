// Implementation of SHA256 optimized for XDAG
// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

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
