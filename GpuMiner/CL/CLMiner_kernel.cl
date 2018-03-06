#ifdef VECTORS
    typedef uint2 u;
    typedef ulong2 ul;
#else
    typedef uint u;
    typedef ulong ul;
#endif

#ifdef BITALIGN
    #pragma OPENCL EXTENSION cl_amd_media_ops : enable
    #define rot(x, y) amd_bitalign(x, x, (u)(32 - y))
#else
    #define rot(x, y) rotate(x, (u)y)
#endif

#ifdef BFI_INT
    #define Ch(x, y, z) amd_bytealign(x, y, z)
#else
    #define Ch(x, y, z) bitselect((u)z, (u)y, (u)x)
#endif

#define Ma(x, y, z) Ch((z ^ x), y, x)

#define bytereverse(x) ( ((x) << 24U) | (((x) << 8U) & 0x00ff0000U) | (((x) >> 8U) & 0x0000ff00U) | ((x) >> 24U) )
#define Sigma0(x) (rot(x, 30U) ^ rot(x, 19U) ^ rot(x, 10U))
#define Sigma1(x) (rot(x, 26U) ^ rot(x, 21U) ^ rot(x, 7U))
#define sigma0(x) (rot(x, 25U) ^ rot(x, 14U) ^ (x >> 3U))
#define sigma1(x) (rot(x, 15U) ^ rot(x, 13U) ^ (x >> 10U))
#define Round(a, b, c, d, e, f, g, h, k, w)\
{\
    t1 = h + Sigma1(e) + Ch(e, f, g) + k + w;\
    t2 = Sigma0(a) + Ma(a, b, c);\
    d += t1;\
    h = t1 + t2;\
}

__kernel void search_nonce(ulong startNonce,
    __constant uint* state,
    __constant uint* preCalcState,
    __constant uint* data,    
    const uint targetH, const uint targetG,
    __global volatile ulong* restrict output)
{
#ifdef VECTORS	
    ul nonce = (startNonce + (get_global_id(0) << 1) * ITERATIONS_COUNT) + (ulong2)(0, 1);
#else
    ul nonce = startNonce + get_global_id(0) * ITERATIONS_COUNT;
#endif

    u t1, t2;
    ulong minNonce;
    uint minH, minG;

    for(uint i = 0; i < ITERATIONS_COUNT; ++i)
    {
        u a = preCalcState[0];
        u b = preCalcState[1];
        u c = preCalcState[2];
        u d = preCalcState[3];
        u e = preCalcState[4];
        u f = preCalcState[5];
        u g = preCalcState[6];
        u h = preCalcState[7];

        u w0 = data[0];
        u w1 = data[1];
        u w2 = data[2];
        u w3 = data[3];
        u w4 = data[4];
        u w5 = data[5];
        u w6 = data[6];
        u w7 = data[7];
        u w8 = data[8];
        u w9 = data[9];
        u w10 = data[10];
        u w11 = data[11];
        u w12 = data[12];
        u w13 = data[13];
    #ifdef VECTORS
        u w14 = bytereverse((uint2)((uint)nonce.x, (uint)nonce.y));
        u w15 = bytereverse((uint2)((uint)(nonce.x >> 32), (uint)(nonce.y >> 32)));
    #else
        u w14 = bytereverse((uint)nonce);
        u w15 = bytereverse((uint)(nonce >> 32));
    #endif

        //first 14 rounds are precalculated
        //Round(a, b, c, d, e, f, g, h, 0x428a2f98U, w0);
        //Round(h, a, b, c, d, e, f, g, 0x71374491U, w1);
        //Round(g, h, a, b, c, d, e, f, 0xb5c0fbcfU, w2);
        //Round(f, g, h, a, b, c, d, e, 0xe9b5dba5U, w3);
        //Round(e, f, g, h, a, b, c, d, 0x3956c25bU, w4);
        //Round(d, e, f, g, h, a, b, c, 0x59f111f1U, w5);
        //Round(c, d, e, f, g, h, a, b, 0x923f82a4U, w6);
        //Round(b, c, d, e, f, g, h, a, 0xab1c5ed5U, w7);
        //Round(a, b, c, d, e, f, g, h, 0xd807aa98U, w8);
        //Round(h, a, b, c, d, e, f, g, 0x12835b01U, w9);
        //Round(g, h, a, b, c, d, e, f, 0x243185beU, w10);
        //Round(f, g, h, a, b, c, d, e, 0x550c7dc3U, w11);
        //Round(e, f, g, h, a, b, c, d, 0x72be5d74U, w12);
        //Round(d, e, f, g, h, a, b, c, 0x80deb1feU, w13);
        Round(c, d, e, f, g, h, a, b, 0x9bdc06a7U, w14);
        Round(b, c, d, e, f, g, h, a, 0xc19bf174U, w15);
        w0 += sigma1(w14) + w9 + sigma0(w1);
        Round(a, b, c, d, e, f, g, h, 0xe49b69c1U, w0);
        w1 += sigma1(w15) + w10 + sigma0(w2);
        Round(h, a, b, c, d, e, f, g, 0xefbe4786U, w1);
        w2 += sigma1(w0) + w11 + sigma0(w3);
        Round(g, h, a, b, c, d, e, f, 0x0fc19dc6U, w2);
        w3 += sigma1(w1) + w12 + sigma0(w4);
        Round(f, g, h, a, b, c, d, e, 0x240ca1ccU, w3);
        w4 += sigma1(w2) + w13 + sigma0(w5);
        Round(e, f, g, h, a, b, c, d, 0x2de92c6fU, w4);
        w5 += sigma1(w3) + w14 + sigma0(w6);
        Round(d, e, f, g, h, a, b, c, 0x4a7484aaU, w5);
        w6 += sigma1(w4) + w15 + sigma0(w7);
        Round(c, d, e, f, g, h, a, b, 0x5cb0a9dcU, w6);
        w7 += sigma1(w5) + w0 + sigma0(w8);
        Round(b, c, d, e, f, g, h, a, 0x76f988daU, w7);
        w8 += sigma1(w6) + w1 + sigma0(w9);
        Round(a, b, c, d, e, f, g, h, 0x983e5152U, w8);
        w9 += sigma1(w7) + w2 + sigma0(w10);
        Round(h, a, b, c, d, e, f, g, 0xa831c66dU, w9);
        w10 += sigma1(w8) + w3 + sigma0(w11);
        Round(g, h, a, b, c, d, e, f, 0xb00327c8U, w10);
        w11 += sigma1(w9) + w4 + sigma0(w12);
        Round(f, g, h, a, b, c, d, e, 0xbf597fc7U, w11);
        w12 += sigma1(w10) + w5 + sigma0(w13);
        Round(e, f, g, h, a, b, c, d, 0xc6e00bf3U, w12);
        w13 += sigma1(w11) + w6 + sigma0(w14);
        Round(d, e, f, g, h, a, b, c, 0xd5a79147U, w13);
        w14 += sigma1(w12) + w7 + sigma0(w15);
        Round(c, d, e, f, g, h, a, b, 0x06ca6351U, w14);
        w15 += sigma1(w13) + w8 + sigma0(w0);
        Round(b, c, d, e, f, g, h, a, 0x14292967U, w15);
        w0 += sigma1(w14) + w9 + sigma0(w1);
        Round(a, b, c, d, e, f, g, h, 0x27b70a85U, w0);
        w1 += sigma1(w15) + w10 + sigma0(w2);
        Round(h, a, b, c, d, e, f, g, 0x2e1b2138U, w1);
        w2 += sigma1(w0) + w11 + sigma0(w3);
        Round(g, h, a, b, c, d, e, f, 0x4d2c6dfcU, w2);
        w3 += sigma1(w1) + w12 + sigma0(w4);
        Round(f, g, h, a, b, c, d, e, 0x53380d13U, w3);
        w4 += sigma1(w2) + w13 + sigma0(w5);
        Round(e, f, g, h, a, b, c, d, 0x650a7354U, w4);
        w5 += sigma1(w3) + w14 + sigma0(w6);
        Round(d, e, f, g, h, a, b, c, 0x766a0abbU, w5);
        w6 += sigma1(w4) + w15 + sigma0(w7);
        Round(c, d, e, f, g, h, a, b, 0x81c2c92eU, w6);
        w7 += sigma1(w5) + w0 + sigma0(w8);
        Round(b, c, d, e, f, g, h, a, 0x92722c85U, w7);
        w8 += sigma1(w6) + w1 + sigma0(w9);
        Round(a, b, c, d, e, f, g, h, 0xa2bfe8a1U, w8);
        w9 += sigma1(w7) + w2 + sigma0(w10);
        Round(h, a, b, c, d, e, f, g, 0xa81a664bU, w9);
        w10 += sigma1(w8) + w3 + sigma0(w11);
        Round(g, h, a, b, c, d, e, f, 0xc24b8b70U, w10);
        w11 += sigma1(w9) + w4 + sigma0(w12);
        Round(f, g, h, a, b, c, d, e, 0xc76c51a3U, w11);
        w12 += sigma1(w10) + w5 + sigma0(w13);
        Round(e, f, g, h, a, b, c, d, 0xd192e819U, w12);
        w13 += sigma1(w11) + w6 + sigma0(w14);
        Round(d, e, f, g, h, a, b, c, 0xd6990624U, w13);
        w14 += sigma1(w12) + w7 + sigma0(w15);
        Round(c, d, e, f, g, h, a, b, 0xf40e3585U, w14);
        w15 += sigma1(w13) + w8 + sigma0(w0);
        Round(b, c, d, e, f, g, h, a, 0x106aa070U, w15);
        w0 += sigma1(w14) + w9 + sigma0(w1);
        Round(a, b, c, d, e, f, g, h, 0x19a4c116U, w0);
        w1 += sigma1(w15) + w10 + sigma0(w2);
        Round(h, a, b, c, d, e, f, g, 0x1e376c08U, w1);
        w2 += sigma1(w0) + w11 + sigma0(w3);
        Round(g, h, a, b, c, d, e, f, 0x2748774cU, w2);
        w3 += sigma1(w1) + w12 + sigma0(w4);
        Round(f, g, h, a, b, c, d, e, 0x34b0bcb5U, w3);
        w4 += sigma1(w2) + w13 + sigma0(w5);
        Round(e, f, g, h, a, b, c, d, 0x391c0cb3U, w4);
        w5 += sigma1(w3) + w14 + sigma0(w6);
        Round(d, e, f, g, h, a, b, c, 0x4ed8aa4aU, w5);
        w6 += sigma1(w4) + w15 + sigma0(w7);
        Round(c, d, e, f, g, h, a, b, 0x5b9cca4fU, w6);
        w7 += sigma1(w5) + w0 + sigma0(w8);
        Round(b, c, d, e, f, g, h, a, 0x682e6ff3U, w7);
        w8 += sigma1(w6) + w1 + sigma0(w9);
        Round(a, b, c, d, e, f, g, h, 0x748f82eeU, w8);
        w9 += sigma1(w7) + w2 + sigma0(w10);
        Round(h, a, b, c, d, e, f, g, 0x78a5636fU, w9);
        w10 += sigma1(w8) + w3 + sigma0(w11);
        Round(g, h, a, b, c, d, e, f, 0x84c87814U, w10);
        w11 += sigma1(w9) + w4 + sigma0(w12);
        Round(f, g, h, a, b, c, d, e, 0x8cc70208U, w11);
        w12 += sigma1(w10) + w5 + sigma0(w13);
        Round(e, f, g, h, a, b, c, d, 0x90befffaU, w12);
        w13 += sigma1(w11) + w6 + sigma0(w14);
        Round(d, e, f, g, h, a, b, c, 0xa4506cebU, w13);
        w14 += sigma1(w12) + w7 + sigma0(w15);
        Round(c, d, e, f, g, h, a, b, 0xbef9a3f7U, w14);
        w15 += sigma1(w13) + w8 + sigma0(w0);
        Round(b, c, d, e, f, g, h, a, 0xc67178f2U, w15);

        u a2 = a = a + state[0];
        u b2 = b = b + state[1];
        u c2 = c = c + state[2];
        u d2 = d = d + state[3];
        u e2 = e = e + state[4];
        u f2 = f = f + state[5];
        u g2 = g = g + state[6];
        u h2 = h = h + state[7];

        w0 = 0x80000000U;
        w1 = 0;
        w2 = 0;
        w3 = 0;
        w4 = 0;
        w5 = 0;
        w6 = 0;
        w7 = 0;
        w8 = 0;
        w9 = 0;
        w10 = 0;
        w11 = 0;
        w12 = 0;
        w13 = 0;
        w14 = 0;
        w15 = 0x00001000U;

        Round(a, b, c, d, e, f, g, h, 0x428a2f98U, w0);
        Round(h, a, b, c, d, e, f, g, 0x71374491U, w1);
        Round(g, h, a, b, c, d, e, f, 0xb5c0fbcfU, w2);
        Round(f, g, h, a, b, c, d, e, 0xe9b5dba5U, w3);
        Round(e, f, g, h, a, b, c, d, 0x3956c25bU, w4);
        Round(d, e, f, g, h, a, b, c, 0x59f111f1U, w5);
        Round(c, d, e, f, g, h, a, b, 0x923f82a4U, w6);
        Round(b, c, d, e, f, g, h, a, 0xab1c5ed5U, w7);
        Round(a, b, c, d, e, f, g, h, 0xd807aa98U, w8);
        Round(h, a, b, c, d, e, f, g, 0x12835b01U, w9);
        Round(g, h, a, b, c, d, e, f, 0x243185beU, w10);
        Round(f, g, h, a, b, c, d, e, 0x550c7dc3U, w11);
        Round(e, f, g, h, a, b, c, d, 0x72be5d74U, w12);
        Round(d, e, f, g, h, a, b, c, 0x80deb1feU, w13);
        Round(c, d, e, f, g, h, a, b, 0x9bdc06a7U, w14);
        Round(b, c, d, e, f, g, h, a, 0xc19bf174U, w15);
        w0 += sigma1(w14) + w9 + sigma0(w1);
        Round(a, b, c, d, e, f, g, h, 0xe49b69c1U, w0);
        w1 += sigma1(w15) + w10 + sigma0(w2);
        Round(h, a, b, c, d, e, f, g, 0xefbe4786U, w1);
        w2 += sigma1(w0) + w11 + sigma0(w3);
        Round(g, h, a, b, c, d, e, f, 0x0fc19dc6U, w2);
        w3 += sigma1(w1) + w12 + sigma0(w4);
        Round(f, g, h, a, b, c, d, e, 0x240ca1ccU, w3);
        w4 += sigma1(w2) + w13 + sigma0(w5);
        Round(e, f, g, h, a, b, c, d, 0x2de92c6fU, w4);
        w5 += sigma1(w3) + w14 + sigma0(w6);
        Round(d, e, f, g, h, a, b, c, 0x4a7484aaU, w5);
        w6 += sigma1(w4) + w15 + sigma0(w7);
        Round(c, d, e, f, g, h, a, b, 0x5cb0a9dcU, w6);
        w7 += sigma1(w5) + w0 + sigma0(w8);
        Round(b, c, d, e, f, g, h, a, 0x76f988daU, w7);
        w8 += sigma1(w6) + w1 + sigma0(w9);
        Round(a, b, c, d, e, f, g, h, 0x983e5152U, w8);
        w9 += sigma1(w7) + w2 + sigma0(w10);
        Round(h, a, b, c, d, e, f, g, 0xa831c66dU, w9);
        w10 += sigma1(w8) + w3 + sigma0(w11);
        Round(g, h, a, b, c, d, e, f, 0xb00327c8U, w10);
        w11 += sigma1(w9) + w4 + sigma0(w12);
        Round(f, g, h, a, b, c, d, e, 0xbf597fc7U, w11);
        w12 += sigma1(w10) + w5 + sigma0(w13);
        Round(e, f, g, h, a, b, c, d, 0xc6e00bf3U, w12);
        w13 += sigma1(w11) + w6 + sigma0(w14);
        Round(d, e, f, g, h, a, b, c, 0xd5a79147U, w13);
        w14 += sigma1(w12) + w7 + sigma0(w15);
        Round(c, d, e, f, g, h, a, b, 0x06ca6351U, w14);
        w15 += sigma1(w13) + w8 + sigma0(w0);
        Round(b, c, d, e, f, g, h, a, 0x14292967U, w15);
        w0 += sigma1(w14) + w9 + sigma0(w1);
        Round(a, b, c, d, e, f, g, h, 0x27b70a85U, w0);
        w1 += sigma1(w15) + w10 + sigma0(w2);
        Round(h, a, b, c, d, e, f, g, 0x2e1b2138U, w1);
        w2 += sigma1(w0) + w11 + sigma0(w3);
        Round(g, h, a, b, c, d, e, f, 0x4d2c6dfcU, w2);
        w3 += sigma1(w1) + w12 + sigma0(w4);
        Round(f, g, h, a, b, c, d, e, 0x53380d13U, w3);
        w4 += sigma1(w2) + w13 + sigma0(w5);
        Round(e, f, g, h, a, b, c, d, 0x650a7354U, w4);
        w5 += sigma1(w3) + w14 + sigma0(w6);
        Round(d, e, f, g, h, a, b, c, 0x766a0abbU, w5);
        w6 += sigma1(w4) + w15 + sigma0(w7);
        Round(c, d, e, f, g, h, a, b, 0x81c2c92eU, w6);
        w7 += sigma1(w5) + w0 + sigma0(w8);
        Round(b, c, d, e, f, g, h, a, 0x92722c85U, w7);
        w8 += sigma1(w6) + w1 + sigma0(w9);
        Round(a, b, c, d, e, f, g, h, 0xa2bfe8a1U, w8);
        w9 += sigma1(w7) + w2 + sigma0(w10);
        Round(h, a, b, c, d, e, f, g, 0xa81a664bU, w9);
        w10 += sigma1(w8) + w3 + sigma0(w11);
        Round(g, h, a, b, c, d, e, f, 0xc24b8b70U, w10);
        w11 += sigma1(w9) + w4 + sigma0(w12);
        Round(f, g, h, a, b, c, d, e, 0xc76c51a3U, w11);
        w12 += sigma1(w10) + w5 + sigma0(w13);
        Round(e, f, g, h, a, b, c, d, 0xd192e819U, w12);
        w13 += sigma1(w11) + w6 + sigma0(w14);
        Round(d, e, f, g, h, a, b, c, 0xd6990624U, w13);
        w14 += sigma1(w12) + w7 + sigma0(w15);
        Round(c, d, e, f, g, h, a, b, 0xf40e3585U, w14);
        w15 += sigma1(w13) + w8 + sigma0(w0);
        Round(b, c, d, e, f, g, h, a, 0x106aa070U, w15);
        w0 += sigma1(w14) + w9 + sigma0(w1);
        Round(a, b, c, d, e, f, g, h, 0x19a4c116U, w0);
        w1 += sigma1(w15) + w10 + sigma0(w2);
        Round(h, a, b, c, d, e, f, g, 0x1e376c08U, w1);
        w2 += sigma1(w0) + w11 + sigma0(w3);
        Round(g, h, a, b, c, d, e, f, 0x2748774cU, w2);
        w3 += sigma1(w1) + w12 + sigma0(w4);
        Round(f, g, h, a, b, c, d, e, 0x34b0bcb5U, w3);
        w4 += sigma1(w2) + w13 + sigma0(w5);
        Round(e, f, g, h, a, b, c, d, 0x391c0cb3U, w4);
        w5 += sigma1(w3) + w14 + sigma0(w6);
        Round(d, e, f, g, h, a, b, c, 0x4ed8aa4aU, w5);
        w6 += sigma1(w4) + w15 + sigma0(w7);
        Round(c, d, e, f, g, h, a, b, 0x5b9cca4fU, w6);
        w7 += sigma1(w5) + w0 + sigma0(w8);
        Round(b, c, d, e, f, g, h, a, 0x682e6ff3U, w7);
        w8 += sigma1(w6) + w1 + sigma0(w9);
        Round(a, b, c, d, e, f, g, h, 0x748f82eeU, w8);
        w9 += sigma1(w7) + w2 + sigma0(w10);
        Round(h, a, b, c, d, e, f, g, 0x78a5636fU, w9);
        w10 += sigma1(w8) + w3 + sigma0(w11);
        Round(g, h, a, b, c, d, e, f, 0x84c87814U, w10);
        w11 += sigma1(w9) + w4 + sigma0(w12);
        Round(f, g, h, a, b, c, d, e, 0x8cc70208U, w11);
        w12 += sigma1(w10) + w5 + sigma0(w13);
        Round(e, f, g, h, a, b, c, d, 0x90befffaU, w12);
        w13 += sigma1(w11) + w6 + sigma0(w14);
        Round(d, e, f, g, h, a, b, c, 0xa4506cebU, w13);
        w14 += sigma1(w12) + w7 + sigma0(w15);
        Round(c, d, e, f, g, h, a, b, 0xbef9a3f7U, w14);
        w15 += sigma1(w13) + w8 + sigma0(w0);
        Round(b, c, d, e, f, g, h, a, 0xc67178f2U, w15);

        // hash the hash now

        w0 = a2 + a;
        w1 = b2 + b;
        w2 = c2 + c;
        w3 = d2 + d;
        w4 = e2 + e;
        w5 = f2 + f;
        w6 = g2 + g;
        w7 = h2 + h;
        w8 = 0x80000000U;
        w9 = 0;
        w10 = 0;
        w11 = 0;
        w12 = 0;
        w13 = 0;
        w14 = 0;
        w15 = 0x00000100U;

        a = 0x6a09e667U;
        b = 0xbb67ae85U;
        c = 0x3c6ef372U;
        d = 0xa54ff53aU;
        e = 0x510e527fU;
        f = 0x9b05688cU;
        g = 0x1f83d9abU;
        h = 0x5be0cd19U;

        Round(a, b, c, d, e, f, g, h, 0x428a2f98U, w0);
        Round(h, a, b, c, d, e, f, g, 0x71374491U, w1);
        Round(g, h, a, b, c, d, e, f, 0xb5c0fbcfU, w2);
        Round(f, g, h, a, b, c, d, e, 0xe9b5dba5U, w3);
        Round(e, f, g, h, a, b, c, d, 0x3956c25bU, w4);
        Round(d, e, f, g, h, a, b, c, 0x59f111f1U, w5);
        Round(c, d, e, f, g, h, a, b, 0x923f82a4U, w6);
        Round(b, c, d, e, f, g, h, a, 0xab1c5ed5U, w7);
        Round(a, b, c, d, e, f, g, h, 0xd807aa98U, w8);
        Round(h, a, b, c, d, e, f, g, 0x12835b01U, w9);
        Round(g, h, a, b, c, d, e, f, 0x243185beU, w10);
        Round(f, g, h, a, b, c, d, e, 0x550c7dc3U, w11);
        Round(e, f, g, h, a, b, c, d, 0x72be5d74U, w12);
        Round(d, e, f, g, h, a, b, c, 0x80deb1feU, w13);
        Round(c, d, e, f, g, h, a, b, 0x9bdc06a7U, w14);
        Round(b, c, d, e, f, g, h, a, 0xc19bf174U, w15);
        w0 += sigma1(w14) + w9 + sigma0(w1);
        Round(a, b, c, d, e, f, g, h, 0xe49b69c1U, w0);
        w1 += sigma1(w15) + w10 + sigma0(w2);
        Round(h, a, b, c, d, e, f, g, 0xefbe4786U, w1);
        w2 += sigma1(w0) + w11 + sigma0(w3);
        Round(g, h, a, b, c, d, e, f, 0x0fc19dc6U, w2);
        w3 += sigma1(w1) + w12 + sigma0(w4);
        Round(f, g, h, a, b, c, d, e, 0x240ca1ccU, w3);
        w4 += sigma1(w2) + w13 + sigma0(w5);
        Round(e, f, g, h, a, b, c, d, 0x2de92c6fU, w4);
        w5 += sigma1(w3) + w14 + sigma0(w6);
        Round(d, e, f, g, h, a, b, c, 0x4a7484aaU, w5);
        w6 += sigma1(w4) + w15 + sigma0(w7);
        Round(c, d, e, f, g, h, a, b, 0x5cb0a9dcU, w6);
        w7 += sigma1(w5) + w0 + sigma0(w8);
        Round(b, c, d, e, f, g, h, a, 0x76f988daU, w7);
        w8 += sigma1(w6) + w1 + sigma0(w9);
        Round(a, b, c, d, e, f, g, h, 0x983e5152U, w8);
        w9 += sigma1(w7) + w2 + sigma0(w10);
        Round(h, a, b, c, d, e, f, g, 0xa831c66dU, w9);
        w10 += sigma1(w8) + w3 + sigma0(w11);
        Round(g, h, a, b, c, d, e, f, 0xb00327c8U, w10);
        w11 += sigma1(w9) + w4 + sigma0(w12);
        Round(f, g, h, a, b, c, d, e, 0xbf597fc7U, w11);
        w12 += sigma1(w10) + w5 + sigma0(w13);
        Round(e, f, g, h, a, b, c, d, 0xc6e00bf3U, w12);
        w13 += sigma1(w11) + w6 + sigma0(w14);
        Round(d, e, f, g, h, a, b, c, 0xd5a79147U, w13);
        w14 += sigma1(w12) + w7 + sigma0(w15);
        Round(c, d, e, f, g, h, a, b, 0x06ca6351U, w14);
        w15 += sigma1(w13) + w8 + sigma0(w0);
        Round(b, c, d, e, f, g, h, a, 0x14292967U, w15);
        w0 += sigma1(w14) + w9 + sigma0(w1);
        Round(a, b, c, d, e, f, g, h, 0x27b70a85U, w0);
        w1 += sigma1(w15) + w10 + sigma0(w2);
        Round(h, a, b, c, d, e, f, g, 0x2e1b2138U, w1);
        w2 += sigma1(w0) + w11 + sigma0(w3);
        Round(g, h, a, b, c, d, e, f, 0x4d2c6dfcU, w2);
        w3 += sigma1(w1) + w12 + sigma0(w4);
        Round(f, g, h, a, b, c, d, e, 0x53380d13U, w3);
        w4 += sigma1(w2) + w13 + sigma0(w5);
        Round(e, f, g, h, a, b, c, d, 0x650a7354U, w4);
        w5 += sigma1(w3) + w14 + sigma0(w6);
        Round(d, e, f, g, h, a, b, c, 0x766a0abbU, w5);
        w6 += sigma1(w4) + w15 + sigma0(w7);
        Round(c, d, e, f, g, h, a, b, 0x81c2c92eU, w6);
        w7 += sigma1(w5) + w0 + sigma0(w8);
        Round(b, c, d, e, f, g, h, a, 0x92722c85U, w7);
        w8 += sigma1(w6) + w1 + sigma0(w9);
        Round(a, b, c, d, e, f, g, h, 0xa2bfe8a1U, w8);
        w9 += sigma1(w7) + w2 + sigma0(w10);
        Round(h, a, b, c, d, e, f, g, 0xa81a664bU, w9);
        w10 += sigma1(w8) + w3 + sigma0(w11);
        Round(g, h, a, b, c, d, e, f, 0xc24b8b70U, w10);
        w11 += sigma1(w9) + w4 + sigma0(w12);
        Round(f, g, h, a, b, c, d, e, 0xc76c51a3U, w11);
        w12 += sigma1(w10) + w5 + sigma0(w13);
        Round(e, f, g, h, a, b, c, d, 0xd192e819U, w12);
        w13 += sigma1(w11) + w6 + sigma0(w14);
        Round(d, e, f, g, h, a, b, c, 0xd6990624U, w13);
        w14 += sigma1(w12) + w7 + sigma0(w15);
        Round(c, d, e, f, g, h, a, b, 0xf40e3585U, w14);
        w15 += sigma1(w13) + w8 + sigma0(w0);
        Round(b, c, d, e, f, g, h, a, 0x106aa070U, w15);
        w0 += sigma1(w14) + w9 + sigma0(w1);
        Round(a, b, c, d, e, f, g, h, 0x19a4c116U, w0);
        w1 += sigma1(w15) + w10 + sigma0(w2);
        Round(h, a, b, c, d, e, f, g, 0x1e376c08U, w1);
        w2 += sigma1(w0) + w11 + sigma0(w3);
        Round(g, h, a, b, c, d, e, f, 0x2748774cU, w2);
        w3 += sigma1(w1) + w12 + sigma0(w4);
        Round(f, g, h, a, b, c, d, e, 0x34b0bcb5U, w3);
        w4 += sigma1(w2) + w13 + sigma0(w5);
        Round(e, f, g, h, a, b, c, d, 0x391c0cb3U, w4);
        w5 += sigma1(w3) + w14 + sigma0(w6);
        Round(d, e, f, g, h, a, b, c, 0x4ed8aa4aU, w5);
        w6 += sigma1(w4) + w15 + sigma0(w7);
        Round(c, d, e, f, g, h, a, b, 0x5b9cca4fU, w6);
        w7 += sigma1(w5) + w0 + sigma0(w8);
        Round(b, c, d, e, f, g, h, a, 0x682e6ff3U, w7);
        w8 += sigma1(w6) + w1 + sigma0(w9);
        Round(a, b, c, d, e, f, g, h, 0x748f82eeU, w8);
        w9 += sigma1(w7) + w2 + sigma0(w10);
        Round(h, a, b, c, d, e, f, g, 0x78a5636fU, w9);
        w10 += sigma1(w8) + w3 + sigma0(w11);
        Round(g, h, a, b, c, d, e, f, 0x84c87814U, w10);
        w11 += sigma1(w9) + w4 + sigma0(w12);
        Round(f, g, h, a, b, c, d, e, 0x8cc70208U, w11);
        w12 += sigma1(w10) + w5 + sigma0(w13);
        Round(e, f, g, h, a, b, c, d, 0x90befffaU, w12);
        w13 += sigma1(w11) + w6 + sigma0(w14);
        Round(d, e, f, g, h, a, b, c, 0xa4506cebU, w13);
        ///////////////////
        //we don't need to do these last 2 rounds as they update F, B, E and A, but we only care about G and H
        //w14 += sigma1(w12) + w7 + sigma0(w15);
        //Round(c, d, e, f, g, h, a, b, 0xbef9a3f7U, w14);
        //w15 += sigma1(w13) + w8 + sigma0(w0);
        //Round(b, c, d, e, f, g, h, a, 0xc67178f2U, w15);

        g = bytereverse(g + 0x1f83d9abU);
        h = bytereverse(h + 0x5be0cd19U);

#ifdef VECTORS
        if(!i || h.x < minH || (h.x == minH && g.x <= minG))
        {
            minNonce = nonce.x;
            minH = h.x;
            minG = g.x;
        }
        if(h.y < minH || (h.y == minH && g.y <= minG))
        {
            minNonce = nonce.y;
            minH = h.y;
            minG = g.y;
        }
        nonce += 2;
#else
        if(!i || h < minH || (h == minH && g <= minG))
        {
            minNonce = nonce;
            minH = h;
            minG = g;
        }
        ++nonce;
#endif
        
    }

    if(minH < targetH || (minH == targetH && minG <= targetG))
    {
        uint slot = min(OUTPUT_SIZE, atomic_inc((__global volatile uint*)output) + 1);
        output[slot] = minNonce;
    }
}
