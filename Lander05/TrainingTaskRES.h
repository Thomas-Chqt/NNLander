#ifndef TRAININGTASKRES_H
#define TRAININGTASKRES_H

#include <cstddef>
#include <random>
#include <vector>
#include <limits> // Needed for numeric_limits
#include <cmath>  // For std::sqrt, std::exp
#include <cassert> // For assert
#include <algorithm> // For std::transform, std::clamp
#include <cstdio> // For printf debugging
#include "Utils.h"
#include "SimpleNeuralNet.h"
#include "Simulation.h"

//==================================================================
// TrainingTaskRES class - handles neural network training using REINFORCE-ES
//==================================================================
template<std::floating_point T, NetArch auto netArch>
class TrainingTaskRES
{
public:
    using NeuralNet = SimpleNeuralNet<T, netArch>;

public:
    struct Params
    {
        size_t maxGenerations = 0;     // Maximum number of generations/updates
        double sigma = 0.1;            // Standard deviation for noise perturbation
        double alpha = 0.01;           // Learning rate
        size_t numPerturbations = 50;  // Number of perturbation pairs (N/2 in some literature)
        uint32_t seed = 1234;          // Seed for random number generator
    };
private:
    const Params mPar;

    SimParams          mSimParams;

    // Number of simulations to run for each perturbed network evaluation
    // More variants -> more accurate evaluation (helps prevent overfitting)
    static constexpr size_t SIM_VARIANTS_N = 30;

    // Central network being trained
    NeuralNet mCentralNetwork;
    double mBestScore = -std::numeric_limits<double>::max(); // Track the best score achieved by the central network

    // Random number generator
    std::mt19937 mRng;

    // Parameter vector size
    size_t mTotalParams = 0;
    // Scaled hyperparameters
    double mAdaptedSigma {};
    double mAdaptedAlpha {};

    size_t mCurrentGeneration = 0;

    // Parallelization system
    ParallelTasks mPllTasks;

public:
    TrainingTaskRES(const Params& par, const SimParams& sp)
        : mPar(par)
        , mSimParams(sp)
        , mRng(par.seed)
    {
        // Initialize central network with random parameters
        mCentralNetwork.InitializeRandomParameters(mRng());
        mTotalParams = mCentralNetwork.GetTotalParameterCount();

        // Scale sigma and alpha by the number of parameters of the network
        // This helps keeping constant the effective learning when the
        // network size changes
        mAdaptedSigma = mPar.sigma / std::sqrt((double)mTotalParams);
        mAdaptedAlpha = mPar.alpha / (double)mTotalParams;

        // Initial evaluation of the central network
        mBestScore = evaluateNetwork(mCentralNetwork);
        printf("[DEBUG] Initial Central Network Score: %.4f\n", mBestScore); // Log initial score
    }

    //==================================================================
    // Evaluate fitness for a given network over multiple simulation variants
    //==================================================================
    double evaluateNetwork(const NeuralNet& net) const
    {
        const uint32_t simStartSeed = 1134; // Consistent starting seed for evaluation runs
        double totalScore = 0.0;

        for (size_t i = 0; i < SIM_VARIANTS_N; ++i)
        {
            const auto variantSeed = simStartSeed + (uint32_t)i;
            totalScore += TestNetworkOnSimulation(variantSeed, net);
        }
        return totalScore / (double)SIM_VARIANTS_N;
    }

    //==================================================================
    // Run a single training iteration (one ES update step)
    //==================================================================
    void RunIteration(bool useThread = true)
    {
        if (IsTrainingComplete()) return;

        std::vector<float> gradientEstimate(mTotalParams, 0.0f);

        // Store results from perturbations
        struct PerturbationResult
        {
            double fitness_plus {};
            double fitness_minus {};
            std::vector<float> epsilon;
        };
        std::vector<PerturbationResult> results(mPar.numPerturbations);

        std::normal_distribution<float> noiseDist{0.0f, 1.0f}; // Standard normal distribution

        // --- Generate and Evaluate Perturbations ---
        for (size_t i = 0; i < mPar.numPerturbations; ++i)
        {
            // Generate noise vector epsilon
            std::vector<float> epsilon(mTotalParams);
            for(size_t j = 0; j < mTotalParams; ++j)
                epsilon[j] = noiseDist(mRng);

            // Store epsilon for later gradient calculation
            results[i].epsilon = epsilon; // Copy epsilon

            // Create perturbed parameters theta_plus and theta_minus
            NeuralNet net_plus;
            NeuralNet net_minus;
            size_t j = 0;
            mCentralNetwork.foreachParameters([&](int layer, int row, int col, T& central_param){
                const auto perturbation = (float)mAdaptedSigma * epsilon[j];
                net_plus.GetParameter(layer, row, col) = central_param + perturbation;
                net_minus.GetParameter(layer, row, col) = central_param - perturbation;
                j++;
            });
#if 0
            for(size_t j = 0; j < mTotalParams; ++j)
            {
                // Optional: Clamp parameters if needed, though often omitted in ES
                params_plus[j] = std::clamp(params_plus[j], -1.0f, 1.0f);
                params_minus[j] = std::clamp(params_minus[j], -1.0f, 1.0f);
            }
#endif

            // --- Evaluate theta_plus ---
            auto thetaPlusTask = [this, net_plus = std::move(net_plus), i, &results]()
                {
                    results[i].fitness_plus = evaluateNetwork(net_plus);
                };
            if (useThread)
                mPllTasks.AddTask(thetaPlusTask);
            else
                thetaPlusTask();

            // --- Evaluate theta_minus ---
            auto thetaMinusTask = [this, net_minus = std::move(net_minus), i, &results]()
                {
                    results[i].fitness_minus = evaluateNetwork(net_minus);
                };
            if (useThread)
                mPllTasks.AddTask(thetaMinusTask);
            else
                thetaMinusTask();
        }

        // Wait for all evaluations to complete
        mPllTasks.WaitAll();

#if 0
        if (!(mCurrentGeneration % 100)) // Log every 10 generations to avoid spam
        {
             printf("[DEBUG Gen %zu] Perturbation Scores:\n", mCurrentGeneration);
             for (size_t i = 0; i < std::min((size_t)3, mNumPerturbations); ++i) {
                 printf("  [%zu] F+: %.4f, F-: %.4f, Diff: %.4f\n",
                        i, results[i].fitness_plus, results[i].fitness_minus,
                        results[i].fitness_plus - results[i].fitness_minus);
             }
        }
#endif

        // --- Calculate Gradient Estimate ---
        for (size_t i = 0; i < mPar.numPerturbations; ++i)
        {
            double fitness_diff = results[i].fitness_plus - results[i].fitness_minus;
            const auto& epsilon = results[i].epsilon;
            for (size_t j = 0; j < mTotalParams; ++j)
            {
                gradientEstimate[j] += (float)fitness_diff * epsilon[j];
            }
        }

        // --- Update Central Parameters ---
        const auto scaleFactor = mAdaptedAlpha / (2.0 * mPar.numPerturbations * mAdaptedSigma);
        size_t j = 0;
        mCentralNetwork.foreachParameters([&](int, int, int, T& central_param){
            central_param += (float)(scaleFactor * gradientEstimate[j]);
            // Optional: Clamp parameters
            // centralParams[j] = std::clamp(centralParams[j], -1.0f, 1.0f);
            j++;

        });

        // Evaluate the updated central network and update best score if improved
        double currentCentralScore = evaluateNetwork(mCentralNetwork);
        if (currentCentralScore > mBestScore) {
            mBestScore = currentCentralScore;
            // Could potentially save the best network parameters here if needed
        }

        // Increment the generation counter
        mCurrentGeneration += 1;
    }

    //==================================================================
    // Test a network on a simulation
    // - "seed" gives the simulation variant to test
    // - "net" is the network to test
    // Returns the score of the simulation with the given network
    // (Identical to the one in TrainingTaskGA)
    //==================================================================
    double TestNetworkOnSimulation(
        uint32_t simulationSeed,
        const NeuralNet& net) const
    {
        // Create a simulation with the given seed
        Simulation sim(mSimParams, simulationSeed);

        // Run the simulation until it ends, or 30 (virtual) seconds have passed
        while (!sim.IsSimulationComplete() && sim.GetElapsedTimeS() < Simulation::MAX_TIME_S)
        {
            // Step the simulation forward...
            sim.AnimateSim([&](const NeuralNet::Inputs& states, NeuralNet::Outputs& actions)
            {
                // states -> net -> actions
                net.FeedForward(states, actions);
            });
        }
        // Return the score of the simulation
        return sim.CalculateScore();
    }

    // Getters for training status
    size_t GetCurrentGeneration() const { return mCurrentGeneration; }
    size_t GetMaxGenerations() const { return mPar.maxGenerations; }
    double GetBestScore() const { return mBestScore; } // Returns the best score seen for the central network
    double GetSigma() const { return mAdaptedSigma; }
    double GetAlpha() const { return mAdaptedAlpha; }
    size_t GetNumPerturbations() const { return mPar.numPerturbations; }
    bool IsTrainingComplete() const { return mCurrentGeneration >= mPar.maxGenerations; }
    // Get the central network object
    const NeuralNet& GetCentralNetwork() const { return mCentralNetwork; }
};

#endif // TRAININGTASKRES_H
