
#include shader/basic/sobol_matrix.glsl

const float inv32 = 1.0 / float(0xFFFFFFFFU);

float sobol(uint d, uint i) {
    uint r = 0;
    uint offset = d * 32;
    for(uint j = 0; i != 0; i >>= 1, j++) {
        if((i & 1) != 0) r ^= V[offset+j];
    }
    return float(r) * inv32;
//    return r * 1.0 / (1u << 32);
}


uint grayCode(uint i) {
    return i ^ (i>>1);
}

uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}


// consider each stochastic process(raytracing) as a sample, assign a seed for it.
// the seed is actually index of sobol sequence.
uint sobolseed = 0u;
uint sobolcurdim = 0u;

// returns random value in [0, 1]
float rand() {
//    return float(wang_hash(sobolseed)) / 4294967296.0;
    return sobol(sobolcurdim++, sobolseed);
}

vec2 rand2D() {
//    return vec2(rand(), rand());
    sobolcurdim += 2;
    return vec2(sobol(sobolcurdim - 2, sobolseed), sobol(sobolcurdim - 1, sobolseed));
}