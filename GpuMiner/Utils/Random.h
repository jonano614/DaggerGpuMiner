#pragma once

#include <stdint.h>

class CRandom
{
public:
    static void FillRandomArray(uint8_t *buf, uint32_t length);
};

