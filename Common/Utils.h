#ifndef UTILS_H
#define UTILS_H

//==================================================================
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <array>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>
#include <thread>
#include <atomic>

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

class ParallelTasks
{
    std::vector<std::thread> mThreads;
    std::queue<std::function<void()>> mTasks;

    std::mutex mMutex;
    std::condition_variable mCondVar;
    bool mTerminate = false;
    std::atomic<int> mRunningTaskCount = 0;

public:
    ParallelTasks()
    {
        mThreads.resize(std::thread::hardware_concurrency());
        for (auto& el : mThreads)
        {
            el = std::thread([&]() {
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mMutex);
                        mCondVar.wait(lock, [this]() { return mTerminate || mTasks.empty() == false; });
                        if (mTerminate)
                            break;
                        task = std::move(mTasks.front());
                        mTasks.pop();
                    }
                    mRunningTaskCount++;
                    task();
                    mRunningTaskCount--;
                    mRunningTaskCount.notify_all();
                }
            });
        }
    }

    void AddTask(const std::function<void()>& task)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mTasks.push(task);
        }
        mCondVar.notify_one();
    }

    // Wait for all pending tasks to complete
    void WaitAll()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mMutex);
                if (mTasks.empty())
                    break;
                task = std::move(mTasks.front());
                mTasks.pop();
            }
            task();
        }
        int old;
        while ((old = mRunningTaskCount) > 0)
            mRunningTaskCount.wait(old);
    }

    ~ParallelTasks()
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mTerminate = true;
        }
        mCondVar.notify_all();
        for (auto& t : mThreads)
            t.join();
    }
};

#endif
