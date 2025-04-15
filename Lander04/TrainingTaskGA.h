#ifndef TRAININGTASKGA_H
#define TRAININGTASKGA_H

#include <random>
#include <algorithm>
#include <numeric>
#include <limits> // Needed for numeric_limits
#include "Utils.h"
#include "SimpleNeuralNet.h"
#include "Simulation.h"

#define USE_MUTATION_STDDEV 0

//==================================================================
// TrainingTaskGA class - handles neural network training using genetic algorithms
//==================================================================
template<std::floating_point T, NetArch auto netArch>
class TrainingTaskGA
{
public:
    using NeuralNet = SimpleNeuralNet<T, netArch>;

private:
    // Individual struct - represents a single member of the population
    struct Individual
    {
        NeuralNet network; // Neural network object
        double fitness = -std::numeric_limits<double>::max();  // Fitness score (-infinity by default)
        
        Individual() = default;

        // Constructor with existing network and optional fitness
        Individual(const  NeuralNet& net, double fitnessScore = -std::numeric_limits<double>::max())
            : network(net), fitness(fitnessScore)
        {
        }

        // Comparison operator for sorting by fitness (descending order)
        inline bool operator<(const Individual& other) const { return fitness > other.fitness; }
    };

private:
    SimParams mSimParams;

    // Training parameters
    size_t  mMaxGenerations = 0;     // Maximum number of generations
    size_t  mPopulationSize = 50;    // Size of population
    size_t  mCurrentGeneration = 0;  // Current generation
    double  mMutationRate = 0.1;     // Probability of mutation
    double  mMutationStrength = 0.3; // Scale of mutation
    double  mElitePercentage = 0.1;  // Percentage of top individuals to keep unchanged
    // Number of simulations to run for each individual
    // More variants -> more accurate evaluation (helps prevent overfitting)
    static constexpr size_t SIM_VARIANTS_N = 30;

    // Population
    std::vector<Individual> mPopulation;
    Individual mBestIndividual;

    // Random number generator
    std::mt19937 mRng;

public:
    TrainingTaskGA(
        const SimParams& sp,
        size_t maxGenerations,
        size_t populationSize,
        double mutationRate,
        double mutationStrength,
        uint32_t seed = 1234)
        : mSimParams(sp)
        , mMaxGenerations(maxGenerations)
        , mPopulationSize(populationSize)
        , mMutationRate(mutationRate)
        , mMutationStrength(mutationStrength)
        , mBestIndividual() // Initialize before mRng to match declaration order
        , mRng(seed)
    {
        // Here we create the initial population, with random networks

        // Generate random networks for each individual
        for (size_t i=0; i < mPopulationSize; ++i)
        {
            // Create a network
            NeuralNet net;
            // Initialize its parameters randomly (using a different seed for each)
            // Note: Xavier/He init could be added to InitializeRandomParameters if needed
            net.InitializeRandomParameters(mRng()); // Use the member RNG

            // Create an individual with the generated network
            mPopulation.emplace_back(net);
        }
    }

    //==================================================================
    // Run a single training iteration (one generation)
    void RunIteration(bool useThread = true)
    {
        // Create the next generation (if this is not the first generation)
        if (mCurrentGeneration != 0)
            evolve();

        // Evaluate the fitness of the population
        evaluatePopulation(useThread);

        // Sort the population by fitness (descending)
        std::sort(mPopulation.begin(), mPopulation.end());

        // Update best individual if necessary
        if (mPopulation[0].fitness > mBestIndividual.fitness)
        {
            mBestIndividual = mPopulation[0];
        }

        // Increment the generation counter
        mCurrentGeneration += 1;
    }

    //==================================================================
    // Evaluate fitness for all individuals in the population
    void evaluatePopulation(bool useThread = true)
    {
        const uint32_t simStartSeed = 1134;

        ParallelTasks pt; // Parallelization system

        // Evaluate each individual's fitness in parallel
        for (auto& individual : mPopulation)
        {
            auto task = [&]() {
                double sum = 0.0;
                for (size_t i = 0; i < SIM_VARIANTS_N; ++i)
                {
                    const auto variantSeed = simStartSeed + (uint32_t)i;
                    // Pass the individual's network directly
                    sum += TestNetworkOnSimulation(variantSeed, individual.network);
                }

                individual.fitness = sum / (double)SIM_VARIANTS_N;
            };
            if (useThread)
                pt.AddTask(task);
            else
                task();
        }
    }

    //==================================================================
    // Create a new generation through selection, crossover and mutation
    void evolve()
    {
        // Keep track of the original population
        std::vector<Individual> oldPopulation = mPopulation;

        // Clear the population for the new generation
        mPopulation.clear();

        // Calculate number of elite individuals to keep unchanged
        const size_t eliteCount = static_cast<size_t>(mPopulationSize * mElitePercentage);

        // Keep elite individuals
        for (size_t i = 0; i < eliteCount && i < oldPopulation.size(); ++i)
        {
            mPopulation.push_back(oldPopulation[i]);
        }

        // Fill the rest of the population with offspring from crossover
        while (mPopulation.size() < mPopulationSize)
        {
            // Select two parents
            const auto& parent1 = SelectParent(oldPopulation);
            const auto& parent2 = SelectParent(oldPopulation);

            // Perform crossover
            Individual child = Crossover(parent1, parent2);

            // Perform mutation
            mutate(child);

            // Add the child to the new population
            mPopulation.push_back(child);
        }
    }

    //==================================================================
    // Select a parent using tournament selection
    const Individual& SelectParent(const std::vector<Individual>& population)
    {
        // Number of individuals to consider in the tournament
        const size_t tournamentSize = 3;

        // Select random individuals for the tournament
        std::vector<size_t> indices(population.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), mRng);

        // Find the best individual among those selected
        size_t bestIdx = indices[0];
        for (size_t i = 1; i < tournamentSize && i < indices.size(); ++i)
        {
            if (population[indices[i]].fitness > population[bestIdx].fitness)
                bestIdx = indices[i];
        }

        return population[bestIdx];
    }

    //==================================================================
    // Crossover two parents to create a child
    Individual Crossover(const Individual& parent1, const Individual& parent2)
    {
        const NeuralNet& networkP1 = parent1.network;
        const NeuralNet& networkP2 = parent2.network;

        NeuralNet childNet;

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        childNet.foreachParameters([&](int layer, int row, int col, T& param)
        {
            if (dist(mRng) >= 0.5f)
                param = networkP1.GetParameter(layer, row, col); // Take from parent1 with 50% chance
            else
                param = networkP2.GetParameter(layer, row, col); // Otherwise, take from parent2
        });

        return Individual(childNet);
    }

    //==================================================================
    static std::array<float, 2> calcMeanAndStdDev(const std::vector<float>& params)
    {
        auto mean = std::accumulate(params.begin(), params.end(), 0.0f) / params.size();
        auto stdDev = std::sqrt(
                        std::accumulate(params.begin(), params.end(), 0.0f,
                            [mean](float sum, float x) {
                                return sum + (x - mean) * (x - mean);
                            }) / params.size());
        return { mean, stdDev };
    }

    //==================================================================
    // Mutate an individual
    void mutate(Individual& individual)
    {
        std::uniform_real_distribution<float> shouldMutateDist(0.0f, 1.0f);
        // Note: USE_MUTATION_STDDEV is not easily adaptable here without recalculating stddev per layer/parameter type
        // Sticking to the simpler mutation strength for now.
        std::normal_distribution<float> mutationValueDist(0.0f, (float)mMutationStrength);

        // Iterate through layers, weights, and biases
        individual.network.foreachParameters([&](int, int, int, T& param) {
            if (shouldMutateDist(mRng) < mMutationRate) {
                param += mutationValueDist(mRng);
                param = std::clamp(param, -1.0f, 1.0f); // Clamp
            }
        });
    }

    //==================================================================
    // Test a network on a simulation
    // - "seed" gives the simulation variant to test
    // - "net" is the network to test
    // Returns the score of the simulation with the given network
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
    size_t GetMaxGenerations() const { return mMaxGenerations; }
    double GetBestScore() const { return mBestIndividual.fitness; }
    size_t GetPopulationSize() const { return mPopulationSize; }
    bool IsTrainingComplete() const { return mCurrentGeneration >= mMaxGenerations; }
    // Get the best network object found so far
    const NeuralNet& GetBestIndividualNetwork() const { return mBestIndividual.network; }
    const std::vector<Individual>& GetPopulation() const { return mPopulation; }
};

#endif
