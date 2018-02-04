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

#define bytereverse(x) ( ((x) << 24) | (((x) << 8) & 0x00ff0000) | (((x) >> 8) & 0x0000ff00) | ((x) >> 24) )
#define R(x) (work[x] = (rot(work[x - 2], 15) ^ rot(work[x - 2], 13) ^ ((work[x - 2] & 0xffffffff) >> 10U)) + work[x - 7] + (rot(work[x - 15], 25) ^ rot(work[x - 15], 14) ^ ((work[x - 15] & 0xffffffff) >> 3U)) + work[x - 16])
#define sharound(a, b, c, d, e, f, g, h, x, K) { t1 = h + (rot(e, 26) ^ rot(e, 21) ^ rot(e, 7)) + (g ^ (e&(f^g))) + K + x; t2 = (rot(a, 30) ^ rot(a, 19) ^ rot(a, 10)) + ((a&b) | (c&(a | b))); d += t1; h = t1 + t2; }

__kernel void search_nonce(ulong startNonce,
    __constant uint* state,
    __constant uint* preCalcState,
    __constant uint* data,    
    const uint target0, const uint target1,
    __global volatile ulong* restrict output)
{
    ul nonce;
#ifdef VECTORS	
    nonce = (startNonce + get_global_id(0) << 1) + (uint2)(0, 1)
#else
    nonce = startNonce + get_global_id(0);
#endif

    u work[64];
    u A, B, C, D, E, F, G, H;
    u A2, B2, C2, D2, E2, F2, G2, H2;
    u t1, t2;

    A = preCalcState[0];
    B = preCalcState[1];
    C = preCalcState[2];
    D = preCalcState[3];
    E = preCalcState[4];
    F = preCalcState[5];
    G = preCalcState[6];
    H = preCalcState[7];

    work[0] = data[0];
    work[1] = data[1];
    work[2] = data[2];
    work[3] = data[3];
    work[4] = data[4];
    work[5] = data[5];
    work[6] = data[6];
    work[7] = data[7];
    work[8] = data[8];
    work[9] = data[9];
    work[10] = data[10];
    work[11] = data[11];
    work[12] = data[12];
    work[13] = data[13];
#ifdef VECTORS
    work[14] = bytereverse((uint2)((uint)nonce.x, (uint)nonce.y));
    work[15] = bytereverse((uint2)((uint)(nonce.x >> 32), (uint)(nonce.y >> 32)));
#else
    work[14] = bytereverse((uint)nonce);
    work[15] = bytereverse((uint)(nonce >> 32));
#endif

    //first 14 rounds are precalculated
    //sharound(A, B, C, D, E, F, G, H, work[0], 0x428A2F98U);
    //sharound(H, A, B, C, D, E, F, G, work[1], 0x71374491U);
    //sharound(G, H, A, B, C, D, E, F, work[2], 0xB5C0FBCFU);
    //sharound(F, G, H, A, B, C, D, E, work[3], 0xE9B5DBA5U);
    //sharound(E, F, G, H, A, B, C, D, work[4], 0x3956C25BU);
    //sharound(D, E, F, G, H, A, B, C, work[5], 0x59F111F1U);
    //sharound(C, D, E, F, G, H, A, B, work[6], 0x923F82A4U);
    //sharound(B, C, D, E, F, G, H, A, work[7], 0xAB1C5ED5U);
    //sharound(A, B, C, D, E, F, G, H, work[8], 0xD807AA98U);
    //sharound(H, A, B, C, D, E, F, G, work[9], 0x12835B01U);
    //sharound(G, H, A, B, C, D, E, F, work[10], 0x243185BEU);
    //sharound(F, G, H, A, B, C, D, E, work[11], 0x550C7DC3U);
    //sharound(E, F, G, H, A, B, C, D, work[12], 0x72BE5D74U);
    //sharound(D, E, F, G, H, A, B, C, work[13], 0x80DEB1FEU);
    sharound(C, D, E, F, G, H, A, B, work[14], 0x9BDC06A7U);
    sharound(B, C, D, E, F, G, H, A, work[15], 0xC19BF174U);
    sharound(A, B, C, D, E, F, G, H, R(16), 0xE49B69C1U);
    sharound(H, A, B, C, D, E, F, G, R(17), 0xEFBE4786U);
    sharound(G, H, A, B, C, D, E, F, R(18), 0x0FC19DC6U);
    sharound(F, G, H, A, B, C, D, E, R(19), 0x240CA1CCU);
    sharound(E, F, G, H, A, B, C, D, R(20), 0x2DE92C6FU);
    sharound(D, E, F, G, H, A, B, C, R(21), 0x4A7484AAU);
    sharound(C, D, E, F, G, H, A, B, R(22), 0x5CB0A9DCU);
    sharound(B, C, D, E, F, G, H, A, R(23), 0x76F988DAU);
    sharound(A, B, C, D, E, F, G, H, R(24), 0x983E5152U);
    sharound(H, A, B, C, D, E, F, G, R(25), 0xA831C66DU);
    sharound(G, H, A, B, C, D, E, F, R(26), 0xB00327C8U);
    sharound(F, G, H, A, B, C, D, E, R(27), 0xBF597FC7U);
    sharound(E, F, G, H, A, B, C, D, R(28), 0xC6E00BF3U);
    sharound(D, E, F, G, H, A, B, C, R(29), 0xD5A79147U);
    sharound(C, D, E, F, G, H, A, B, R(30), 0x06CA6351U);
    sharound(B, C, D, E, F, G, H, A, R(31), 0x14292967U);
    sharound(A, B, C, D, E, F, G, H, R(32), 0x27B70A85U);
    sharound(H, A, B, C, D, E, F, G, R(33), 0x2E1B2138U);
    sharound(G, H, A, B, C, D, E, F, R(34), 0x4D2C6DFCU);
    sharound(F, G, H, A, B, C, D, E, R(35), 0x53380D13U);
    sharound(E, F, G, H, A, B, C, D, R(36), 0x650A7354U);
    sharound(D, E, F, G, H, A, B, C, R(37), 0x766A0ABBU);
    sharound(C, D, E, F, G, H, A, B, R(38), 0x81C2C92EU);
    sharound(B, C, D, E, F, G, H, A, R(39), 0x92722C85U);
    sharound(A, B, C, D, E, F, G, H, R(40), 0xA2BFE8A1U);
    sharound(H, A, B, C, D, E, F, G, R(41), 0xA81A664BU);
    sharound(G, H, A, B, C, D, E, F, R(42), 0xC24B8B70U);
    sharound(F, G, H, A, B, C, D, E, R(43), 0xC76C51A3U);
    sharound(E, F, G, H, A, B, C, D, R(44), 0xD192E819U);
    sharound(D, E, F, G, H, A, B, C, R(45), 0xD6990624U);
    sharound(C, D, E, F, G, H, A, B, R(46), 0xF40E3585U);
    sharound(B, C, D, E, F, G, H, A, R(47), 0x106AA070U);
    sharound(A, B, C, D, E, F, G, H, R(48), 0x19A4C116U);
    sharound(H, A, B, C, D, E, F, G, R(49), 0x1E376C08U);
    sharound(G, H, A, B, C, D, E, F, R(50), 0x2748774CU);
    sharound(F, G, H, A, B, C, D, E, R(51), 0x34B0BCB5U);
    sharound(E, F, G, H, A, B, C, D, R(52), 0x391C0CB3U);
    sharound(D, E, F, G, H, A, B, C, R(53), 0x4ED8AA4AU);
    sharound(C, D, E, F, G, H, A, B, R(54), 0x5B9CCA4FU);
    sharound(B, C, D, E, F, G, H, A, R(55), 0x682E6FF3U);
    sharound(A, B, C, D, E, F, G, H, R(56), 0x748F82EEU);
    sharound(H, A, B, C, D, E, F, G, R(57), 0x78A5636FU);
    sharound(G, H, A, B, C, D, E, F, R(58), 0x84C87814U);
    sharound(F, G, H, A, B, C, D, E, R(59), 0x8CC70208U);
    sharound(E, F, G, H, A, B, C, D, R(60), 0x90BEFFFAU);
    sharound(D, E, F, G, H, A, B, C, R(61), 0xA4506CEBU);
    sharound(C, D, E, F, G, H, A, B, R(62), 0xBEF9A3F7U);
    sharound(B, C, D, E, F, G, H, A, R(63), 0xC67178F2U);

    A2 = A = A + state[0];
    B2 = B = B + state[1];
    C2 = C = C + state[2];
    D2 = D = D + state[3];
    E2 = E = E + state[4];
    F2 = F = F + state[5];
    G2 = G = G + state[6];
    H2 = H = H + state[7];

    work[0] = 0x80000000U;
    work[1] = 0;
    work[2] = 0;
    work[3] = 0;
    work[4] = 0;
    work[5] = 0;
    work[6] = 0;
    work[7] = 0;
    work[8] = 0;
    work[9] = 0;
    work[10] = 0;
    work[11] = 0;
    work[12] = 0;
    work[13] = 0;
    work[14] = 0;
    work[15] = 0x00001000U;

    sharound(A, B, C, D, E, F, G, H, work[0], 0x428A2F98U);
    sharound(H, A, B, C, D, E, F, G, work[1], 0x71374491U);
    sharound(G, H, A, B, C, D, E, F, work[2], 0xB5C0FBCFU);
    sharound(F, G, H, A, B, C, D, E, work[3], 0xE9B5DBA5U);
    sharound(E, F, G, H, A, B, C, D, work[4], 0x3956C25BU);
    sharound(D, E, F, G, H, A, B, C, work[5], 0x59F111F1U);
    sharound(C, D, E, F, G, H, A, B, work[6], 0x923F82A4U);
    sharound(B, C, D, E, F, G, H, A, work[7], 0xAB1C5ED5U);
    sharound(A, B, C, D, E, F, G, H, work[8], 0xD807AA98U);
    sharound(H, A, B, C, D, E, F, G, work[9], 0x12835B01U);
    sharound(G, H, A, B, C, D, E, F, work[10], 0x243185BEU);
    sharound(F, G, H, A, B, C, D, E, work[11], 0x550C7DC3U);
    sharound(E, F, G, H, A, B, C, D, work[12], 0x72BE5D74U);
    sharound(D, E, F, G, H, A, B, C, work[13], 0x80DEB1FEU);
    sharound(C, D, E, F, G, H, A, B, work[14], 0x9BDC06A7U);
    sharound(B, C, D, E, F, G, H, A, work[15], 0xC19BF174U);
    sharound(A, B, C, D, E, F, G, H, R(16), 0xE49B69C1U);
    sharound(H, A, B, C, D, E, F, G, R(17), 0xEFBE4786U);
    sharound(G, H, A, B, C, D, E, F, R(18), 0x0FC19DC6U);
    sharound(F, G, H, A, B, C, D, E, R(19), 0x240CA1CCU);
    sharound(E, F, G, H, A, B, C, D, R(20), 0x2DE92C6FU);
    sharound(D, E, F, G, H, A, B, C, R(21), 0x4A7484AAU);
    sharound(C, D, E, F, G, H, A, B, R(22), 0x5CB0A9DCU);
    sharound(B, C, D, E, F, G, H, A, R(23), 0x76F988DAU);
    sharound(A, B, C, D, E, F, G, H, R(24), 0x983E5152U);
    sharound(H, A, B, C, D, E, F, G, R(25), 0xA831C66DU);
    sharound(G, H, A, B, C, D, E, F, R(26), 0xB00327C8U);
    sharound(F, G, H, A, B, C, D, E, R(27), 0xBF597FC7U);
    sharound(E, F, G, H, A, B, C, D, R(28), 0xC6E00BF3U);
    sharound(D, E, F, G, H, A, B, C, R(29), 0xD5A79147U);
    sharound(C, D, E, F, G, H, A, B, R(30), 0x06CA6351U);
    sharound(B, C, D, E, F, G, H, A, R(31), 0x14292967U);
    sharound(A, B, C, D, E, F, G, H, R(32), 0x27B70A85U);
    sharound(H, A, B, C, D, E, F, G, R(33), 0x2E1B2138U);
    sharound(G, H, A, B, C, D, E, F, R(34), 0x4D2C6DFCU);
    sharound(F, G, H, A, B, C, D, E, R(35), 0x53380D13U);
    sharound(E, F, G, H, A, B, C, D, R(36), 0x650A7354U);
    sharound(D, E, F, G, H, A, B, C, R(37), 0x766A0ABBU);
    sharound(C, D, E, F, G, H, A, B, R(38), 0x81C2C92EU);
    sharound(B, C, D, E, F, G, H, A, R(39), 0x92722C85U);
    sharound(A, B, C, D, E, F, G, H, R(40), 0xA2BFE8A1U);
    sharound(H, A, B, C, D, E, F, G, R(41), 0xA81A664BU);
    sharound(G, H, A, B, C, D, E, F, R(42), 0xC24B8B70U);
    sharound(F, G, H, A, B, C, D, E, R(43), 0xC76C51A3U);
    sharound(E, F, G, H, A, B, C, D, R(44), 0xD192E819U);
    sharound(D, E, F, G, H, A, B, C, R(45), 0xD6990624U);
    sharound(C, D, E, F, G, H, A, B, R(46), 0xF40E3585U);
    sharound(B, C, D, E, F, G, H, A, R(47), 0x106AA070U);
    sharound(A, B, C, D, E, F, G, H, R(48), 0x19A4C116U);
    sharound(H, A, B, C, D, E, F, G, R(49), 0x1E376C08U);
    sharound(G, H, A, B, C, D, E, F, R(50), 0x2748774CU);
    sharound(F, G, H, A, B, C, D, E, R(51), 0x34B0BCB5U);
    sharound(E, F, G, H, A, B, C, D, R(52), 0x391C0CB3U);
    sharound(D, E, F, G, H, A, B, C, R(53), 0x4ED8AA4AU);
    sharound(C, D, E, F, G, H, A, B, R(54), 0x5B9CCA4FU);
    sharound(B, C, D, E, F, G, H, A, R(55), 0x682E6FF3U);
    sharound(A, B, C, D, E, F, G, H, R(56), 0x748F82EEU);
    sharound(H, A, B, C, D, E, F, G, R(57), 0x78A5636FU);
    sharound(G, H, A, B, C, D, E, F, R(58), 0x84C87814U);
    sharound(F, G, H, A, B, C, D, E, R(59), 0x8CC70208U);
    sharound(E, F, G, H, A, B, C, D, R(60), 0x90BEFFFAU);
    sharound(D, E, F, G, H, A, B, C, R(61), 0xA4506CEBU);
    sharound(C, D, E, F, G, H, A, B, R(62), 0xBEF9A3F7U);
    sharound(B, C, D, E, F, G, H, A, R(63), 0xC67178F2U);

    // hash the hash now

    work[0] = A2 + A;
    work[1] = B2 + B;
    work[2] = C2 + C;
    work[3] = D2 + D;
    work[4] = E2 + E;
    work[5] = F2 + F;
    work[6] = G2 + G;
    work[7] = H2 + H;
    work[8] = 0x80000000;
    work[9] = 0;
    work[10] = 0;
    work[11] = 0;
    work[12] = 0;
    work[13] = 0;
    work[14] = 0;
    work[15] = 0x00000100;

    A = 0x6a09e667U;
    B = 0xbb67ae85U;
    C = 0x3c6ef372U;
    D = 0xa54ff53aU;
    E = 0x510e527fU;
    F = 0x9b05688cU;
    G = 0x1f83d9abU;
    H = 0x5be0cd19U;

    sharound(A, B, C, D, E, F, G, H, work[0], 0x428A2F98U);
    sharound(H, A, B, C, D, E, F, G, work[1], 0x71374491U);
    sharound(G, H, A, B, C, D, E, F, work[2], 0xB5C0FBCFU);
    sharound(F, G, H, A, B, C, D, E, work[3], 0xE9B5DBA5U);
    sharound(E, F, G, H, A, B, C, D, work[4], 0x3956C25BU);
    sharound(D, E, F, G, H, A, B, C, work[5], 0x59F111F1U);
    sharound(C, D, E, F, G, H, A, B, work[6], 0x923F82A4U);
    sharound(B, C, D, E, F, G, H, A, work[7], 0xAB1C5ED5U);
    sharound(A, B, C, D, E, F, G, H, work[8], 0xD807AA98U);
    sharound(H, A, B, C, D, E, F, G, work[9], 0x12835B01U);
    sharound(G, H, A, B, C, D, E, F, work[10], 0x243185BEU);
    sharound(F, G, H, A, B, C, D, E, work[11], 0x550C7DC3U);
    sharound(E, F, G, H, A, B, C, D, work[12], 0x72BE5D74U);
    sharound(D, E, F, G, H, A, B, C, work[13], 0x80DEB1FEU);
    sharound(C, D, E, F, G, H, A, B, work[14], 0x9BDC06A7U);
    sharound(B, C, D, E, F, G, H, A, work[15], 0xC19BF174U);
    sharound(A, B, C, D, E, F, G, H, R(16), 0xE49B69C1U);
    sharound(H, A, B, C, D, E, F, G, R(17), 0xEFBE4786U);
    sharound(G, H, A, B, C, D, E, F, R(18), 0x0FC19DC6U);
    sharound(F, G, H, A, B, C, D, E, R(19), 0x240CA1CCU);
    sharound(E, F, G, H, A, B, C, D, R(20), 0x2DE92C6FU);
    sharound(D, E, F, G, H, A, B, C, R(21), 0x4A7484AAU);
    sharound(C, D, E, F, G, H, A, B, R(22), 0x5CB0A9DCU);
    sharound(B, C, D, E, F, G, H, A, R(23), 0x76F988DAU);
    sharound(A, B, C, D, E, F, G, H, R(24), 0x983E5152U);
    sharound(H, A, B, C, D, E, F, G, R(25), 0xA831C66DU);
    sharound(G, H, A, B, C, D, E, F, R(26), 0xB00327C8U);
    sharound(F, G, H, A, B, C, D, E, R(27), 0xBF597FC7U);
    sharound(E, F, G, H, A, B, C, D, R(28), 0xC6E00BF3U);
    sharound(D, E, F, G, H, A, B, C, R(29), 0xD5A79147U);
    sharound(C, D, E, F, G, H, A, B, R(30), 0x06CA6351U);
    sharound(B, C, D, E, F, G, H, A, R(31), 0x14292967U);
    sharound(A, B, C, D, E, F, G, H, R(32), 0x27B70A85U);
    sharound(H, A, B, C, D, E, F, G, R(33), 0x2E1B2138U);
    sharound(G, H, A, B, C, D, E, F, R(34), 0x4D2C6DFCU);
    sharound(F, G, H, A, B, C, D, E, R(35), 0x53380D13U);
    sharound(E, F, G, H, A, B, C, D, R(36), 0x650A7354U);
    sharound(D, E, F, G, H, A, B, C, R(37), 0x766A0ABBU);
    sharound(C, D, E, F, G, H, A, B, R(38), 0x81C2C92EU);
    sharound(B, C, D, E, F, G, H, A, R(39), 0x92722C85U);
    sharound(A, B, C, D, E, F, G, H, R(40), 0xA2BFE8A1U);
    sharound(H, A, B, C, D, E, F, G, R(41), 0xA81A664BU);
    sharound(G, H, A, B, C, D, E, F, R(42), 0xC24B8B70U);
    sharound(F, G, H, A, B, C, D, E, R(43), 0xC76C51A3U);
    sharound(E, F, G, H, A, B, C, D, R(44), 0xD192E819U);
    sharound(D, E, F, G, H, A, B, C, R(45), 0xD6990624U);
    sharound(C, D, E, F, G, H, A, B, R(46), 0xF40E3585U);
    sharound(B, C, D, E, F, G, H, A, R(47), 0x106AA070U);
    sharound(A, B, C, D, E, F, G, H, R(48), 0x19A4C116U);
    sharound(H, A, B, C, D, E, F, G, R(49), 0x1E376C08U);
    sharound(G, H, A, B, C, D, E, F, R(50), 0x2748774CU);
    sharound(F, G, H, A, B, C, D, E, R(51), 0x34B0BCB5U);
    sharound(E, F, G, H, A, B, C, D, R(52), 0x391C0CB3U);
    sharound(D, E, F, G, H, A, B, C, R(53), 0x4ED8AA4AU);
    sharound(C, D, E, F, G, H, A, B, R(54), 0x5B9CCA4FU);
    sharound(B, C, D, E, F, G, H, A, R(55), 0x682E6FF3U);
    sharound(A, B, C, D, E, F, G, H, R(56), 0x748F82EEU);
    sharound(H, A, B, C, D, E, F, G, R(57), 0x78A5636FU);
    sharound(G, H, A, B, C, D, E, F, R(58), 0x84C87814U);
    sharound(F, G, H, A, B, C, D, E, R(59), 0x8CC70208U);
    sharound(E, F, G, H, A, B, C, D, R(60), 0x90BEFFFAU);
    sharound(D, E, F, G, H, A, B, C, R(61), 0xA4506CEBU);
    //we don't need to do these last 2 rounds as they update F, B, E and A, but we only care about G and H
    //sharound(C, D, E, F, G, H, A, B, R(62), 0xBEF9A3F7U);
    //sharound(B, C, D, E, F, G, H, A, R(63), 0xC67178F2U);

    G = bytereverse(G + 0x1f83d9abU);
    H = bytereverse(H + 0x5be0cd19U);

#ifdef VECTORS
    if(H.x < target0 || H.x == target0 && G.x <= target1)
    {
        uint slot = min(OUTPUT_SIZE, atomic_inc((__global volatile uint*)output) + 1);
        output[slot] = nonce.x;
    }
    if(H.y < target0 || H.y == target0 && G.y <= target1)
    {
        uint slot = min(OUTPUT_SIZE, atomic_inc((__global volatile uint*)output) + 1);
        output[slot] = nonce.y;
    }
#else
    if(H < target0 || H == target0 && G <= target1)
    {
        uint slot = min(OUTPUT_SIZE, atomic_inc((__global volatile uint*)output) + 1);
        output[slot] = nonce;
    }
#endif
}
