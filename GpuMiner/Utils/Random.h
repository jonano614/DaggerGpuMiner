// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#pragma once

#include <stdint.h>

class CRandom
{
public:
    static void FillRandomArray(uint8_t *buf, uint32_t length);
};

