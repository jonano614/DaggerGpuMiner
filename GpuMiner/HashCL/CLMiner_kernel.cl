#define OPENCL_PLATFORM_UNKNOWN 0
#define OPENCL_PLATFORM_NVIDIA  1
#define OPENCL_PLATFORM_AMD     2
#define OPENCL_PLATFORM_CLOVER  3

#ifndef GROUP_SIZE
#define GROUP_SIZE 128
#endif

#ifndef PLATFORM
#define PLATFORM OPENCL_PLATFORM_AMD
#endif

#ifdef cl_clang_storage_class_specifiers
#pragma OPENCL EXTENSION cl_clang_storage_class_specifiers : enable
#endif

inline uint bswap_32(uint x)
{
    return (((x & 0xff000000U) >> 24) | ((x & 0x00ff0000U) >> 8) |
        ((x & 0x0000ff00U) << 8) | ((x & 0x000000ffU) << 24));
}

uint ReadBE32(uchar* ptr)
{
    return bswap_32(*(uint*)ptr);
}

void WriteBE32(uint* ptr, uint x)
{
    *ptr = bswap_32(x);
}

uint Ch(uint x, uint y, uint z) { return z ^ (x & (y ^ z)); }
uint Maj(uint x, uint y, uint z) { return (x & y) | (z & (x | y)); }
uint Sigma0(uint x) { return (x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10); }
uint Sigma1(uint x) { return (x >> 6 | x << 26) ^ (x >> 11 | x << 21) ^ (x >> 25 | x << 7); }
uint sigma0(uint x) { return (x >> 7 | x << 25) ^ (x >> 18 | x << 14) ^ (x >> 3); }
uint sigma1(uint x) { return (x >> 17 | x << 15) ^ (x >> 19 | x << 13) ^ (x >> 10); }

#define Round(a, b, c, d, e, f, g, h, k, w)\
{\
    t1 = h + Sigma1(e) + Ch(e, f, g) + k + w;\
    t2 = Sigma0(a) + Maj(a, b, c);\
    d += t1;\
    h = t1 + t2;\
}

void sha256_transform(uint* s, uchar* chunk)
{
    uint a = s[0], b = s[1], c = s[2], d = s[3], e = s[4], f = s[5], g = s[6], h = s[7];
    uint w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15;
    uint t1, t2;

    w0 = ReadBE32(chunk + 0);
    Round(a, b, c, d, e, f, g, h, 0x428a2f98, w0);

    w1 = ReadBE32(chunk + 4);
    Round(h, a, b, c, d, e, f, g, 0x71374491, w1);

    w2 = ReadBE32(chunk + 8);
    Round(g, h, a, b, c, d, e, f, 0xb5c0fbcf, w2);

    w3 = ReadBE32(chunk + 12);
    Round(f, g, h, a, b, c, d, e, 0xe9b5dba5, w3);

    w4 = ReadBE32(chunk + 16);
    Round(e, f, g, h, a, b, c, d, 0x3956c25b, w4);

    w5 = ReadBE32(chunk + 20);
    Round(d, e, f, g, h, a, b, c, 0x59f111f1, w5);

    w6 = ReadBE32(chunk + 24);
    Round(c, d, e, f, g, h, a, b, 0x923f82a4, w6);

    w7 = ReadBE32(chunk + 28);
    Round(b, c, d, e, f, g, h, a, 0xab1c5ed5, w7);

    w8 = ReadBE32(chunk + 32);
    Round(a, b, c, d, e, f, g, h, 0xd807aa98, w8);

    w9 = ReadBE32(chunk + 36);
    Round(h, a, b, c, d, e, f, g, 0x12835b01, w9);

    w10 = ReadBE32(chunk + 40);
    Round(g, h, a, b, c, d, e, f, 0x243185be, w10);

    w11 = ReadBE32(chunk + 44);
    Round(f, g, h, a, b, c, d, e, 0x550c7dc3, w11);

    w12 = ReadBE32(chunk + 48);
    Round(e, f, g, h, a, b, c, d, 0x72be5d74, w12);

    w13 = ReadBE32(chunk + 52);
    Round(d, e, f, g, h, a, b, c, 0x80deb1fe, w13);

    w14 = ReadBE32(chunk + 56);
    Round(c, d, e, f, g, h, a, b, 0x9bdc06a7, w14);

    w15 = ReadBE32(chunk + 60);
    Round(b, c, d, e, f, g, h, a, 0xc19bf174, w15);

    w0 += sigma1(w14) + w9 + sigma0(w1);
    Round(a, b, c, d, e, f, g, h, 0xe49b69c1, w0);

    w1 += sigma1(w15) + w10 + sigma0(w2);
    Round(h, a, b, c, d, e, f, g, 0xefbe4786, w1);

    w2 += sigma1(w0) + w11 + sigma0(w3);
    Round(g, h, a, b, c, d, e, f, 0x0fc19dc6, w2);

    w3 += sigma1(w1) + w12 + sigma0(w4);
    Round(f, g, h, a, b, c, d, e, 0x240ca1cc, w3);

    w4 += sigma1(w2) + w13 + sigma0(w5);
    Round(e, f, g, h, a, b, c, d, 0x2de92c6f, w4);

    w5 += sigma1(w3) + w14 + sigma0(w6);
    Round(d, e, f, g, h, a, b, c, 0x4a7484aa, w5);

    w6 += sigma1(w4) + w15 + sigma0(w7);
    Round(c, d, e, f, g, h, a, b, 0x5cb0a9dc, w6);

    w7 += sigma1(w5) + w0 + sigma0(w8);
    Round(b, c, d, e, f, g, h, a, 0x76f988da, w7);

    w8 += sigma1(w6) + w1 + sigma0(w9);
    Round(a, b, c, d, e, f, g, h, 0x983e5152, w8);

    w9 += sigma1(w7) + w2 + sigma0(w10);
    Round(h, a, b, c, d, e, f, g, 0xa831c66d, w9);

    w10 += sigma1(w8) + w3 + sigma0(w11);
    Round(g, h, a, b, c, d, e, f, 0xb00327c8, w10);

    w11 += sigma1(w9) + w4 + sigma0(w12);
    Round(f, g, h, a, b, c, d, e, 0xbf597fc7, w11);

    w12 += sigma1(w10) + w5 + sigma0(w13);
    Round(e, f, g, h, a, b, c, d, 0xc6e00bf3, w12);

    w13 += sigma1(w11) + w6 + sigma0(w14);
    Round(d, e, f, g, h, a, b, c, 0xd5a79147, w13);

    w14 += sigma1(w12) + w7 + sigma0(w15);
    Round(c, d, e, f, g, h, a, b, 0x06ca6351, w14);

    w15 += sigma1(w13) + w8 + sigma0(w0);
    Round(b, c, d, e, f, g, h, a, 0x14292967, w15);

    w0 += sigma1(w14) + w9 + sigma0(w1);
    Round(a, b, c, d, e, f, g, h, 0x27b70a85, w0);

    w1 += sigma1(w15) + w10 + sigma0(w2);
    Round(h, a, b, c, d, e, f, g, 0x2e1b2138, w1);

    w2 += sigma1(w0) + w11 + sigma0(w3);
    Round(g, h, a, b, c, d, e, f, 0x4d2c6dfc, w2);

    w3 += sigma1(w1) + w12 + sigma0(w4);
    Round(f, g, h, a, b, c, d, e, 0x53380d13, w3);

    w4 += sigma1(w2) + w13 + sigma0(w5);
    Round(e, f, g, h, a, b, c, d, 0x650a7354, w4);

    w5 += sigma1(w3) + w14 + sigma0(w6);
    Round(d, e, f, g, h, a, b, c, 0x766a0abb, w5);

    w6 += sigma1(w4) + w15 + sigma0(w7);
    Round(c, d, e, f, g, h, a, b, 0x81c2c92e, w6);

    w7 += sigma1(w5) + w0 + sigma0(w8);
    Round(b, c, d, e, f, g, h, a, 0x92722c85, w7);

    w8 += sigma1(w6) + w1 + sigma0(w9);
    Round(a, b, c, d, e, f, g, h, 0xa2bfe8a1, w8);

    w9 += sigma1(w7) + w2 + sigma0(w10);
    Round(h, a, b, c, d, e, f, g, 0xa81a664b, w9);

    w10 += sigma1(w8) + w3 + sigma0(w11);
    Round(g, h, a, b, c, d, e, f, 0xc24b8b70, w10);

    w11 += sigma1(w9) + w4 + sigma0(w12);
    Round(f, g, h, a, b, c, d, e, 0xc76c51a3, w11);

    w12 += sigma1(w10) + w5 + sigma0(w13);
    Round(e, f, g, h, a, b, c, d, 0xd192e819, w12);

    w13 += sigma1(w11) + w6 + sigma0(w14);
    Round(d, e, f, g, h, a, b, c, 0xd6990624, w13);

    w14 += sigma1(w12) + w7 + sigma0(w15);
    Round(c, d, e, f, g, h, a, b, 0xf40e3585, w14);

    w15 += sigma1(w13) + w8 + sigma0(w0);
    Round(b, c, d, e, f, g, h, a, 0x106aa070, w15);

    w0 += sigma1(w14) + w9 + sigma0(w1);
    Round(a, b, c, d, e, f, g, h, 0x19a4c116, w0);

    w1 += sigma1(w15) + w10 + sigma0(w2);
    Round(h, a, b, c, d, e, f, g, 0x1e376c08, w1);

    w2 += sigma1(w0) + w11 + sigma0(w3);
    Round(g, h, a, b, c, d, e, f, 0x2748774c, w2);

    w3 += sigma1(w1) + w12 + sigma0(w4);
    Round(f, g, h, a, b, c, d, e, 0x34b0bcb5, w3);

    w4 += sigma1(w2) + w13 + sigma0(w5);
    Round(e, f, g, h, a, b, c, d, 0x391c0cb3, w4);

    w5 += sigma1(w3) + w14 + sigma0(w6);
    Round(d, e, f, g, h, a, b, c, 0x4ed8aa4a, w5);

    w6 += sigma1(w4) + w15 + sigma0(w7);
    Round(c, d, e, f, g, h, a, b, 0x5b9cca4f, w6);

    w7 += sigma1(w5) + w0 + sigma0(w8);
    Round(b, c, d, e, f, g, h, a, 0x682e6ff3, w7);

    w8 += sigma1(w6) + w1 + sigma0(w9);
    Round(a, b, c, d, e, f, g, h, 0x748f82ee, w8);

    w9 += sigma1(w7) + w2 + sigma0(w10);
    Round(h, a, b, c, d, e, f, g, 0x78a5636f, w9);

    w10 += sigma1(w8) + w3 + sigma0(w11);
    Round(g, h, a, b, c, d, e, f, 0x84c87814, w10);

    w11 += sigma1(w9) + w4 + sigma0(w12);
    Round(f, g, h, a, b, c, d, e, 0x8cc70208, w11);

    w12 += sigma1(w10) + w5 + sigma0(w13);
    Round(e, f, g, h, a, b, c, d, 0x90befffa, w12);

    w13 += sigma1(w11) + w6 + sigma0(w14);
    Round(d, e, f, g, h, a, b, c, 0xa4506ceb, w13);

    uint w14_2 = w14 + sigma1(w12) + w7 + sigma0(w15);
    Round(c, d, e, f, g, h, a, b, 0xbef9a3f7, w14_2);

    uint w15_2 = w15 + sigma1(w13) + w8 + sigma0(w0);
    Round(b, c, d, e, f, g, h, a, 0xc67178f2, w15_2);

    s[0] += a;
    s[1] += b;
    s[2] += c;
    s[3] += d;
    s[4] += e;
    s[5] += f;
    s[6] += g;
    s[7] += h;
    chunk += 64;
}

void shasha(uint* state, uint* data, ulong nonce, uchar *hash)
{
    uint stateBuffer[8];
    uint dataBuffer[16];

#pragma unroll
    for(int i = 0; i < 8; ++i)
    {
        stateBuffer[i] = state[i];
    }
#pragma unroll
    for(int i = 0; i < 14; ++i)
    {
        dataBuffer[i] = data[i];
    }

    ((ulong*)dataBuffer)[7] = nonce;
    sha256_transform(stateBuffer, (uchar*)dataBuffer);

    dataBuffer[0] = 0x80;
#pragma unroll
    for (int i = 1; i < 14; ++i)
    {
        dataBuffer[i] = 0;
    }
    dataBuffer[15] = 0x00100000;
    dataBuffer[14] = 0;
    sha256_transform(stateBuffer, (uchar*)dataBuffer);

    WriteBE32(dataBuffer, stateBuffer[0]);
    WriteBE32(dataBuffer + 1, stateBuffer[1]);
    WriteBE32(dataBuffer + 2, stateBuffer[2]);
    WriteBE32(dataBuffer + 3, stateBuffer[3]);
    WriteBE32(dataBuffer + 4, stateBuffer[4]);
    WriteBE32(dataBuffer + 5, stateBuffer[5]);
    WriteBE32(dataBuffer + 6, stateBuffer[6]);
    WriteBE32(dataBuffer + 7, stateBuffer[7]);

    stateBuffer[0] = 0x6a09e667;
    stateBuffer[1] = 0xbb67ae85;
    stateBuffer[2] = 0x3c6ef372;
    stateBuffer[3] = 0xa54ff53a;
    stateBuffer[4] = 0x510e527f;
    stateBuffer[5] = 0x9b05688c;
    stateBuffer[6] = 0x1f83d9ab;
    stateBuffer[7] = 0x5be0cd19;

    dataBuffer[8] = 0x80;
#pragma unroll
    for (int i = 9; i < 14; ++i)
    {
        dataBuffer[i] = 0;
    }
    dataBuffer[15] = 0x10000;
    dataBuffer[14] = 0;
    sha256_transform(stateBuffer, (uchar*)dataBuffer);

    WriteBE32((uint*)hash, stateBuffer[0]);
    WriteBE32((uint*)(hash + 4), stateBuffer[1]);
    WriteBE32((uint*)(hash + 8), stateBuffer[2]);
    WriteBE32((uint*)(hash + 12), stateBuffer[3]);
    WriteBE32((uint*)(hash + 16), stateBuffer[4]);
    WriteBE32((uint*)(hash + 20), stateBuffer[5]);
    WriteBE32((uint*)(hash + 24), stateBuffer[6]);
    WriteBE32((uint*)(hash + 28), stateBuffer[7]);
}

int cmphash(uint *l, uint *r) 
{
#pragma unroll
    for (int i = 7; i >= 0; --i)
    {
        if (l[i] != r[i])
        {
            return (l[i] < r[i] ? -1 : 1);
        }
    }
    return 0;
}

__kernel void search_nonce(__constant uint const* hashState,
                            __constant uint const* data,
                            ulong startNonce, 
                            uint iterations,
                            __constant uint const* targetHash,
                            __global ulong *output)
{
    uint hash[8];
    uint minHash[8];
    uint localHashState[8];
    uint localData[16];
    ulong min_nonce = 0;
    uint id = get_global_id(0);
    ulong nonce = startNonce + id * iterations;

#pragma unroll
    for(uint i = 0; i < 8; ++i)
    {
        minHash[i] = targetHash[i];
    }
#pragma unroll
    for(uint i = 0; i < 8; ++i)
    {
        localHashState[i] = hashState[i];
    }
#pragma unroll
    for(uint i = 0; i < 14; ++i)
    {
        localData[i] = data[i];
    }
    for(uint i = 0; i < iterations; ++i)
    {
        shasha(localHashState, localData, nonce, (uchar*)hash);

        if(cmphash(hash, minHash) < 0)
        {
#pragma unroll
            for(uint i = 0; i < 8; ++i)
            {
                minHash[i] = hash[i];
            }
            min_nonce = nonce;
        }
        ++nonce;
    }
    if (min_nonce > 0)
    {
        output[OUTPUT_SIZE] = output[min_nonce & OUTPUT_MASK] = min_nonce;
    }
}
