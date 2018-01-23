#include "Random.h"
#include <random>

void CRandom::FillRandomArray(uint8_t *buf, uint32_t length)
{
    std::random_device rd;
    std::mt19937 generator(rd());

    for(int i = 0; i < length; ++i)
    {
        buf[i] = generator() % 256;
    }
}
