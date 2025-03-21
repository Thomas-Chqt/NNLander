#ifndef TRAININGTASKGA_H
#define TRAININGTASKGA_H

#include <random>
#include <algorithm>
#include <numeric>
#include "SimpleNeuralNet.h"
#include "Simulation.h"

//==================================================================
// Individual class - represents a single member of the population
//==================================================================
class Individual
{
public:
    std::vector<float> parameters;  // Neural network parameters
    double fitness = -std::numeric_limits<double>::max();  // Fitness score (-infinity by default)

    Individual() = default;

    Individual(const std::vector<float>& params, double fitnessScore = -std::numeric_limits<double>::max())
        : parameters(params), fitness(fitnessScore) {}

    // Comparison operator for sorting by fitness (descending order)
    bool operator<(const Individual& other) const {
        return fitness > other.fitness;
    }
};

//==================================================================
// TrainingTaskGA class - handles neural network training using genetic algorithms
//==================================================================
class TrainingTaskGA
{
private:
    SimParams          mSimParams;
    std::vector<int>   mNetworkArchitecture;

    // Training parameters
    size_t             mMaxGenerations = 0;              // Maximum number of generations
    size_t             mPopulationSize = 50;             // Size of population
    size_t             mCurrentGeneration = 0;           // Current generation
    double             mMutationRate = 0.1;              // Probability of mutation
    double             mMutationStrength = 0.3;          // Scale of mutation
    double             mElitePercentage = 0.1;           // Percentage of top individuals to keep unchanged

    // Population
    std::vector<Individual> mPopulation;
    Individual mBestIndividual;

    // Random number generator
    std::mt19937 mRng;

public:
    TrainingTaskGA(
        const SimParams& sp,
        const std::vector<int>& architecture,
        size_t maxGenerations,
        size_t populationSize,
        double mutationRate,
        double mutationStrength,
        uint32_t seed = 1234)
        : mSimParams(sp)
        , mNetworkArchitecture(architecture)
        , mMaxGenerations(maxGenerations)
        , mPopulationSize(populationSize)
        , mMutationRate(mutationRate)
        , mMutationStrength(mutationStrength)
        , mRng(seed)
    {
        // Initialize the first generation with random individuals
        InitializePopulation();
    }

    //==================================================================
    // Initialize population with random individuals
    //==================================================================
    void InitializePopulation()
    {
        // Create the neural network structure (for reference - not used directly)
        SimpleNeuralNet net(mNetworkArchitecture);
        const size_t paramCount = net.GetTotalParameters();

        // Clear and resize the population
        mPopulation.clear();
        mPopulation.resize(mPopulationSize);

        // Generate random parameters for each individual
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        for (size_t i = 0; i < mPopulationSize; ++i)
        {
            std::vector<float> params(paramCount);
            for (size_t j = 0; j < paramCount; ++j)
                params[j] = dist(mRng);

            mPopulation[i] = Individual(params);
        }
    }

    //==================================================================
    // Run a single training iteration (one generation)
    //==================================================================
    void RunIteration()
    {
        // If this is the first generation, evaluate fitness
        if (mCurrentGeneration == 0)
        {
            EvaluatePopulation();
        }
        else
        {
            // Create the next generation using genetic operators
            Evolve();
            // Evaluate the fitness of the new population
            EvaluatePopulation();
        }

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
    //==================================================================
    void EvaluatePopulation()
    {
        SimpleNeuralNet net(mNetworkArchitecture);

        const uint32_t simStartSeed = 1134;
        static size_t simVariants = 10;

        // Evaluate each individual's fitness
        for (auto& individual : mPopulation)
        {
            double sum = 0.0;
            for (size_t i = 0; i < simVariants; ++i)
                sum += TestNetworkOnSimulation(simStartSeed + i, net, individual.parameters);

            individual.fitness = sum / (double)simVariants;
        }
    }

    //==================================================================
    // Create a new generation through selection, crossover and mutation
    //==================================================================
    void Evolve()
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
            Individual parent1 = SelectParent(oldPopulation);
            Individual parent2 = SelectParent(oldPopulation);

            // Perform crossover
            Individual child = Crossover(parent1, parent2);

            // Perform mutation
            Mutate(child);

            // Add the child to the new population
            mPopulation.push_back(child);
        }
    }

    //==================================================================
    // Select a parent using tournament selection
    //==================================================================
    Individual SelectParent(const std::vector<Individual>& population)
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
            {
                bestIdx = indices[i];
            }
        }

        return population[bestIdx];
    }

    //==================================================================
    // Crossover two parents to create a child
    //==================================================================
    Individual Crossover(const Individual& parent1, const Individual& parent2)
    {
        // Uniform crossover: each parameter has a 50% chance of coming from each parent
        std::vector<float> childParams(parent1.parameters.size());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        for (size_t i = 0; i < childParams.size(); ++i)
        {
            if (dist(mRng) < 0.5f)
                childParams[i] = parent1.parameters[i];
            else
                childParams[i] = parent2.parameters[i];
        }

        return Individual(childParams);
    }

    //==================================================================
    // Mutate an individual
    //==================================================================
    void Mutate(Individual& individual)
    {
        std::uniform_real_distribution<float> shouldMutate(0.0f, 1.0f);
        std::normal_distribution<float> mutation(0.0f, mMutationStrength);

        for (float& param : individual.parameters)
        {
            // Each parameter has a chance to mutate
            if (shouldMutate(mRng) < mMutationRate)
            {
                // Add a normally distributed random value
                param += mutation(mRng);
                // Clamp to [-1, 1] range
                param = std::clamp(param, -1.0f, 1.0f);
            }
        }
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

    // Getters for training status
    size_t GetCurrentGeneration() const { return mCurrentGeneration; }
    size_t GetMaxGenerations() const { return mMaxGenerations; }
    double GetBestScore() const { return mBestIndividual.fitness; }
    size_t GetPopulationSize() const { return mPopulationSize; }
    bool IsTrainingComplete() const { return mCurrentGeneration >= mMaxGenerations; }
    const std::vector<float>& GetBestNetworkParameters() const { return mBestIndividual.parameters; }
    const std::vector<Individual>& GetPopulation() const { return mPopulation; }
};

#endif