#ifndef TRAININGTASKRANDOM_H
#define TRAININGTASKRANDOM_H

#include <random>
#include <algorithm>
#include "SimpleNeuralNet.h"
#include "Simulation.h"

//==================================================================
// TrainingTaskRandom class - handles neural network training
//==================================================================
class TrainingTaskRandom
{
private:
    SimParams          mSimParams;
    std::vector<int>   mNetworkArchitecture;

    // Training parameters
    size_t             mMaxEpochs = 0 ;
    size_t             mCurrentEpoch = 0;
    double             mBestScore = -std::numeric_limits<double>::max();
    std::vector<float> mBestNetworkParameters;

public:
    TrainingTaskRandom(
        const SimParams& sp,
        const std::vector<int>& architecture,
        size_t maxEpochs)
        : mSimParams(sp)
        , mNetworkArchitecture(architecture)
        , mMaxEpochs(maxEpochs)
    {}

    //==================================================================
    // Run a single training iteration
    // Each iteration will generate a new network with random parameters
    //  and test it on the simulation.
    // If the network is better than our current best, we save it.
    void RunIteration()
    {
        // Create the network structure
        SimpleNeuralNet net(mNetworkArchitecture);
        // A different seed for each epoch, to generate different random parameters
        const uint32_t networkSeed = mCurrentEpoch + 1111;
        // Generate random parameters for this new network
        const auto params = generateRandomParameters(net, networkSeed);

        // Random seed for the simulation
        const uint32_t simulationSeed = 1134;
        // Test the network on the simulation to get a score
        const auto currentScore = TestNetworkOnSimulation(simulationSeed, net, params);
        // If this network is better than our current best, save it
        if (currentScore > mBestScore)
        {
            mBestScore = currentScore;
            mBestNetworkParameters = params;
        }

        // Increment the training epoch counter
        mCurrentEpoch += 1;
    }

    //==================================================================
    // Test a network on a simulation
    // - "seed" gives the simulation variant to test
    // - "params" are the weights and biases of the network to test
    // Returns the score of the simulation with the given parameters
    //==================================================================
    double TestNetworkOnSimulation(
        uint32_t simulationSeed,
        const SimpleNeuralNet& net,
        const std::vector<float>& params) const
    {
        // Create a simulation with the given seed
        Simulation sim(mSimParams, simulationSeed);

        // Run the simulation until it ends, or 30 (virtual) seconds have passed
        while (!sim.IsSimulationComplete() && sim.GetElapsedTimeS() < 30.0)
        {
            // Step the simulation forward...
            sim.AnimateSim([&](const float* states, float* actions)
            {
                // states -> net(params) -> actions
                net.FeedForward(params.data(), states, actions);
            });
        }
        // Return the score of the simulation
        return sim.CalculateScore();
    }

    // Generate random parameters for neural network
    static std::vector<float> generateRandomParameters(const SimpleNeuralNet& net, uint32_t seed)
    {
        // Use a random number generator between -1.0 and 1.0
        std::mt19937 rng(seed);
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        // Generate random parameters
        std::vector<float> params(net.GetTotalParameters());
        for (size_t i = 0; i < params.size(); ++i)
            params[i] = dist(rng);

        return params;
    }

    const auto& GetBestNetworkParameters() const { return mBestNetworkParameters; }

    // Getters for training status
    size_t GetCurrentEpoch() const { return mCurrentEpoch; }
    size_t GetMaxEpochs() const { return mMaxEpochs; }
    double GetBestScore() const { return mBestScore; }
    bool IsTrainingComplete() const { return mCurrentEpoch >= mMaxEpochs; }
};

#endif
