#include "Utils.h"
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "StringFormat.h"

void DumpHex(const uint8_t* byteArray, int length, bool oneLine)
{
    int width = 0;
    for(const unsigned char* p = byteArray; length > 0; ++p)
    {
        if(!oneLine && width >= 16)
        {
            putchar('\n');
            width = 0;
        }
        printf("%02x", *p);
        if(!oneLine)
        {
            putchar(' ');
        }
        --length;
        ++width;
    }
    putchar('\n');
}

std::string ToHexString(const unsigned char *byteArray, int length)
{
    std::stringstream ss;
    ss << std::hex;
    for(int i(0); i < length; ++i)
    {
        ss << (int)byteArray[i];
    }
    return ss.str();
}

std::string HashToHexString(const uint64_t* hash)
{
    std::string hex = string_format("%016llx%016llx%016llx%016llx", hash[3], hash[2], hash[1], hash[0]);
    return hex;
}