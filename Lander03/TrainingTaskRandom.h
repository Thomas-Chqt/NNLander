#ifndef TRAININGTASKRANDOM_H
#define TRAININGTASKRANDOM_H

#include <random>
#include <algorithm>
#include <limits> // Needed for numeric_limits
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
    SimpleNeuralNet    mBestNetwork; // Store the best network object

public:
    TrainingTaskRandom(
        const SimParams& sp,
        const std::vector<int>& architecture,
        size_t maxEpochs)
        : mSimParams(sp)
        , mNetworkArchitecture(architecture)
        , mMaxEpochs(maxEpochs)
        , mBestNetwork(architecture) // Initialize best network with architecture
    {}

    //==================================================================
    // Run a single training iteration
    // Each iteration will generate a new network with random parameters
    //  and test it on the simulation.
    // If the network is better than our current best, we save it.
    void RunIteration()
    {
        // Create a network for this iteration
        SimpleNeuralNet net(mNetworkArchitecture);
        // A different seed for each epoch, to generate different random parameters
        const uint32_t networkSeed = (uint32_t)(mCurrentEpoch + 1111);
        // Initialize the network with random parameters
        net.InitializeRandomParameters(networkSeed);

        // Random seed for the simulation
        const uint32_t simulationSeed = 1134;
        // Test the network on the simulation to get a score
        const auto currentScore = TestNetworkOnSimulation(simulationSeed, net);
        // If this network is better than our current best, save it
        if (currentScore > mBestScore)
        {
            mBestScore = currentScore;
            mBestNetwork = net; // Save the whole network object
        }

        // Increment the training epoch counter
        mCurrentEpoch += 1;
    }

    //==================================================================
    // Test a network on a simulation
    // - "seed" gives the simulation variant to test
    // - "net" is the network to test
    // Returns the score of the simulation with the given network
    //==================================================================
    double TestNetworkOnSimulation(
        uint32_t simulationSeed,
        const SimpleNeuralNet& net) const
    {
        // Create a simulation with the given seed
        Simulation sim(mSimParams, simulationSeed);

        // Run the simulation until it ends, or 30 (virtual) seconds have passed
        while (!sim.IsSimulationComplete() && sim.GetElapsedTimeS() < 30.0)
        {
            // Step the simulation forward...
            sim.AnimateSim([&](const float* states, float* actions)
            {
                // states -> net -> actions
                net.FeedForward(states, actions);
            });
        }
        // Return the score of the simulation
        return sim.CalculateScore();
    }

    // Get the best network object found so far
    const SimpleNeuralNet& GetBestNetwork() const { return mBestNetwork; }

    // Getters for training status
    size_t GetCurrentEpoch() const { return mCurrentEpoch; }
    size_t GetMaxEpochs() const { return mMaxEpochs; }
    double GetBestScore() const { return mBestScore; }
    bool IsTrainingComplete() const { return mCurrentEpoch >= mMaxEpochs; }
};

#endif
