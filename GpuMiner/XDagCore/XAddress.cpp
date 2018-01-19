#include "XAddress.h"
#include <string.h>

static const uint8_t* bits2mime = (uint8_t*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

XAddress::XAddress()
{
    Init();
}

XAddress::~XAddress()
{
}

void XAddress::Init()
{
    int i;
    memset(_mime2bits, 0xFF, 256);
    for (i = 0; i < 64; ++i)
    {
        _mime2bits[bits2mime[i]] = i;
    }
}

bool XAddress::AddressToHash(const char *address, cheatcoin_hash_t hash)
{
    uint8_t *fld = (uint8_t *)hash;
    int i, c, d, e, n;
    for (e = n = i = 0; i < 32; ++i)
    {
        do
        {
            if (!(c = (uint8_t)*address++))
            {
                return false;
            }
            d = _mime2bits[c];
        } 
        while (d & 0xC0);
        e <<= 6, e |= d, n += 6;
        if (n >= 8)
        {
            *fld++ = e >> (n -= 8);
        }
    }
    for (i = 0; i < 8; ++i)
    {
        *fld++ = 0;
    }
    return true;
}

const char* XAddress::HashToAddress(const cheatcoin_hash_t hash)
{
    static char bufs[4][33];
    static int k = 0;
    char *buf = &bufs[k++ & 3][0], *ptr = buf;
    int i, c, d;
    const uint8_t *fld = (const uint8_t *)hash;
    for (i = c = d = 0; i < 32; ++i)
    {
        if (d < 6)
        {
            d += 8, c <<= 8, c |= *fld++;
        }
        *ptr++ = bits2mime[c >> (d -= 6) & 0x3F];
    }
    *ptr = 0;
    return buf;
}