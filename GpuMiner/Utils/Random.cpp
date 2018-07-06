// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#include "Random.h"
#include <random>

void CRandom::FillRandomArray(uint8_t *buf, uint32_t length)
{
    std::random_device rd;
    std::mt19937 generator(rd());

    for(uint32_t i = 0; i < length; ++i)
    {
        buf[i] = generator() % 256;
    }
}
