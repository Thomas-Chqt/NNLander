#ifndef UTILS_H
#define UTILS_H

//==================================================================
#include <cassert>
#include <cstdint>
#include <array>

//==================================================================
// Improved random number generator class - uses xoshiro256++ algorithm
// References:
// - https://prng.di.unimi.it/
// - https://en.wikipedia.org/wiki/Xorshift
//==================================================================
class RandomGenerator
{
    std::array<uint64_t, 4> s; // State

    // Helper function for xoshiro256++
    static inline uint64_t rotl(const uint64_t x, int k)
    {
        return (x << k) | (x >> (64 - k));
    }
public:
    // Initialize with a seed
    explicit RandomGenerator(uint64_t seed = 0xDEADBEEFDEADBEEF)
    {
        SeedXoshiro256(seed);
    }

    // Properly seed the generator using splitmix64 to initialize all state
    void SeedXoshiro256(uint64_t seed)
    {
        // Use splitmix64 to initialize all 4 states
        for (int i = 0; i < 4; ++i)
        {
            seed += 0x9e3779b97f4a7c15;
            uint64_t z = seed;
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
            z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
            s[i] = z ^ (z >> 31);
        }
    }

    // Generate a random uint64_t - xoshiro256++ algorithm
    uint64_t NextU64()
    {
        const uint64_t result = rotl(s[0] + s[3], 23) + s[0];
        const uint64_t t = s[1] << 17;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;
        s[3] = rotl(s[3], 45);

        return result;
    }

    // Generate a random float between 0 and 1
    float NextFloat() {
        // Use top 53 bits for maximum precision in double conversion
        return (NextU64() >> 11) * (1.0f / 9007199254740992.0f);
    }

    // Generate a random float within a range
    float RandRange(float min, float max) { return min + (max - min) * NextFloat(); }

    // Generate a random integer within a range [min, max] (inclusive)
    int RandRangeInt(int min, int max) { return min + (NextU64() % (max - min + 1)); }
};

//==================================================================
// ParallelTasks class - handles parallel execution of tasks
//==================================================================
#include <future>
#include <thread>

class ParallelTasks
{
    std::vector<std::future<void>> mFutures;
    unsigned int mThreadsN {};
public:
    ParallelTasks() : mThreadsN(std::thread::hardware_concurrency()) {}

    void AddTask(std::function<void()> task)
    {
        if (mFutures.size() >= mThreadsN)
        {
            mFutures.front().wait();
            mFutures.erase(mFutures.begin());
        }
        mFutures.push_back(std::async(std::launch::async, task));
    }

    // Wait for all pending tasks to complete
    void WaitAll()
    {
        for (auto& future : mFutures)
        {
            if (future.valid())
                future.wait();
        }
        mFutures.clear();
    }
};

#endif