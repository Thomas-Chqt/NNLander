#ifndef UTILS_H
#define UTILS_H

//==================================================================
#include <cassert>
#include <cstdint>

inline uint64_t FastRandom(uint64_t& state)
{
    // Loop over for a random number of times, for added entropy
    const auto n = (int)(2 + ((state>>3) & 3));
    for (int i = 0; i < n; ++i)
    {
        // Must not be zero
        state = state ? state : 0xDEADBEEFDEADBEEF;

        // xorshift64* algorithm
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        state *= UINT64_C(2685821657736338717);
    }
    return state;
}

inline float FastRandomFloat(uint64_t& state)
{
    return (FastRandom(state) >> 11) * (1.0f / 9007199254740992.0f);
}

inline float FastRandomRange(uint64_t& state, float min, float max)
{
    return min + (max - min) * FastRandomFloat(state);
}

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