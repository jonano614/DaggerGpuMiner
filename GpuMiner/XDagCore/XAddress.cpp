// Implementation of conversion between hash and XDAG address
// Author: Evgeniy Sukhomlinov
// 2018

// Licensed under GNU General Public License, Version 3. See the LICENSE file.

#include "XAddress.h"
#include <string.h>

static const uint8_t* bits2mime = (uint8_t*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
uint8_t XAddress::_mime2bits[256];

void XAddress::Init()
{
    int i;
    memset(_mime2bits, 0xFF, 256);
    for(i = 0; i < 64; ++i)
    {
        _mime2bits[bits2mime[i]] = i;
    }
}

bool XAddress::AddressToHash(const char *address, xdag_hash_t hash)
{
    uint8_t *fld = (uint8_t *)hash;
    int i, c, d, e, n;
    for(e = n = i = 0; i < 32; ++i)
    {
        do
        {
            if(!(c = (uint8_t)*address++))
            {
                return false;
            }
            d = _mime2bits[c];
        }
        while(d & 0xC0);
        e <<= 6;
        e |= d;
        n += 6;
        if(n >= 8)
        {
            n -= 8;
            *fld++ = e >> n;
        }
    }
    for(i = 0; i < 8; ++i)
    {
        *fld++ = 0;
    }
    return true;
}

const char* XAddress::HashToAddress(const xdag_hash_t hash)
{
    static char bufs[4][33];
    static int k = 0;
    char *buf = &bufs[k++ & 3][0], *ptr = buf;
    int i, c, d;
    const uint8_t *fld = (const uint8_t *)hash;
    for(i = c = d = 0; i < 32; ++i)
    {
        if(d < 6)
        {
            d += 8;
            c <<= 8;
            c |= *fld++;
        }
        d -= 6;
        *ptr++ = bits2mime[c >> d & 0x3F];
    }
    *ptr = 0;
    return buf;
}
