#ifndef UTILS_H
#define UTILS_H

#include <cassert>
#include <cstdint>

inline uint64_t FastRandom(uint64_t& state)
{
    // Must not be zero
    state = state ? state : 0xDEADBEEFDEADBEEF;

    // xorshift64* algorithm
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;
    return state * UINT64_C(2685821657736338717);
}

inline float FastRandomFloat(uint64_t& state)
{
    return (FastRandom(state) >> 11) * (1.0f / 9007199254740992.0f);
}

inline float FastRandomRange(uint64_t& state, float min, float max)
{
    return min + (max - min) * FastRandomFloat(state);
}

#endif