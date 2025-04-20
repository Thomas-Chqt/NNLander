#ifndef SIMPLE_NEURAL_NET_H_2
#define SIMPLE_NEURAL_NET_H_2

#include <vector>
#include <algorithm>
#include <cmath>
#include <random> // Needed for InitializeRandomParameters
#include <stdexcept> // Needed for runtime_error
#include <numeric>   // Needed for accumulate in GetFlatParameters
#include <cassert>   // For assert

#define SNN_INIT_RANDOM_UNIFORM 0
#define SNN_INIT_HE_NORMAL 0
#define SNN_INIT_XAVIER_UNIFORM 1

// commit : 0e46ddd (dpasca/master)
namespace dp2
{

//==================================================================
// Structure to hold parameters for a single layer transition
struct LayerParameters
{
    std::vector<float> weights; // Matrix: rows=currentLayerSize, cols=prevLayerSize
    std::vector<float> biases;  // Vector: size=currentLayerSize
};

//==================================================================
class SimpleNeuralNet
{
    const std::vector<int> mArchitecture;     // Network architecture (nodes per layer)
    std::vector<LayerParameters> mLayerParams; // Parameters per layer transition
    size_t mMaxLayerSize = 0;                 // Maximum number of neurons in any layer

public:
/*
The constructor takes the network architecture.
network architecture.
The network architecture tells how many layers there are and how many
neurons per layer.

NOTICE: the actual network that we use has more neurons and more layers.
The example below is just for illustration.

           O O O      | architecture[0] = 3 neurons (INPUT layer, the simulation state)
          /|/|\|\     |
         O O O O O    | architecture[1] = 5 neurons (HIDDEN layer, the thinking layer)
         X X X X X    |
         O O O O O    | architecture[2] = 5 neurons (HIDDEN layer, the thinking layer)
          \|/|\|/     |
           O O O      | architecture[3] = 3 neurons (OUTPUT layer, the actions to take)

  architecture = [3, 5, 5, 3]

  - Total Neurons:    16 -> (3   +   5   +   5   +   3)
    (Sum of all neurons in Input, Hidden, and Output layers)

  - Connections:      55 -> (   3*5  +  5*5  +  5*3   )
    (Weights linking each neuron in one layer to neurons in the next)

  - Biases:           13 -> (0   +   5   +   5   +   3)
    (One bias for each neuron, except for the input layer !)

  - Total Parameters: 68 -> (Connections + Biases)
*/
    SimpleNeuralNet(const std::vector<int>& architecture)
        : mArchitecture(architecture)
    {
        if (architecture.size() < 2) {
            throw std::runtime_error("Network architecture must have at least 2 layers (input and output).");
        }

        // Resize the layer parameters vector (one less than num layers)
        mLayerParams.resize(mArchitecture.size() - 1);

        // Initialize weights and biases vectors for each layer transition
        for (size_t i = 0; i < mLayerParams.size(); ++i) {
            int prevLayerSize = mArchitecture[i];
            int currentLayerSize = mArchitecture[i + 1];
            mLayerParams[i].weights.resize(prevLayerSize * currentLayerSize);
            mLayerParams[i].biases.resize(currentLayerSize);
        }

        // Find the maximum number of neurons in any layer
        mMaxLayerSize = *std::max_element(mArchitecture.begin(), mArchitecture.end());
    }

    // Copy constructor
    SimpleNeuralNet(const SimpleNeuralNet& other)
        : mArchitecture(other.mArchitecture),     // Copy const architecture
          mLayerParams(other.mLayerParams),       // Copy layer parameters vector (deep copy)
          mMaxLayerSize(other.mMaxLayerSize)
    {}

    // Copy assignment operator
    SimpleNeuralNet& operator=(const SimpleNeuralNet& other)
    {
        if (this == &other) // Handle self-assignment
            return *this;

        // Ensure architectures are compatible before assigning parameters
        assert(mArchitecture == other.mArchitecture);
        mLayerParams = other.mLayerParams; // Copy layer parameters (deep copy)
        // mArchitecture, mMaxLayerSize are const or derived, no need to copy
        return *this;
    }

    // Given the architecture, calculate the total number of parameters
    static size_t CalcTotalParameters(const std::vector<int>& architecture)
    {
        size_t n = 0;
        for (size_t i=1; i < architecture.size(); ++i)
            n += architecture[i-1] * architecture[i] + architecture[i];
        return n;
    }

    //==================================================================
    // Feed forward function
    // This function builds a net with the given Parameters and then
    // applies the Inputs to the net to get the Outputs.
    // inputs -> net -> outputs
    //==================================================================
    void FeedForward(const float* pInputs, float* pOutputs) const
    {
        // Basic check if layer params structure seems initialized
        assert(mLayerParams.size() == (mArchitecture.size() - 1));

        // Allocate buffers on the stack to avoid touching the heap
        float* lay0_outs = (float*)alloca(mMaxLayerSize * sizeof(float));
        float* lay1_outs = (float*)alloca(mMaxLayerSize * sizeof(float));

        // Copy inputs (simulation states) to first layer outputs
        for (int i=0; i < mArchitecture[0]; ++i)
            lay0_outs[i] = pInputs[i];

        // Process each layer transition
        for (size_t i = 0; i < mLayerParams.size(); ++i)
        {
            const auto& currentLayerParams = mLayerParams[i];
            const int prevLayerSize = mArchitecture[i];
            const int currentLayerSize = mArchitecture[i + 1];

            // Check if the specific layer parameters seem valid
            assert(currentLayerParams.weights.size() == (size_t)(prevLayerSize * currentLayerSize));
            assert(currentLayerParams.biases.size() == (size_t)currentLayerSize);


            const float* weights = currentLayerParams.weights.data();
            const float* biases = currentLayerParams.biases.data();
            // int weightIdx = 0; // Unused variable

            // For each neuron in the current layer
            for (int n1 = 0; n1 < currentLayerSize; ++n1)
            {
                // Sum weighted inputs
                auto sum = 0.0f;
                for (int n0 = 0; n0 < prevLayerSize; ++n0)
                {
                    // Access weights matrix (row-major: n1 * prevLayerSize + n0 or
                    //                        col-major: n0 * currentLayerSize + n1)
                    // Assuming weights stored row-major (neuron in current layer is row index)
                    sum += lay0_outs[n0] * weights[n1 * prevLayerSize + n0];
                    // If col-major: sum += lay0_outs[n0] * weights[n0 * currentLayerSize + n1];
                }
                sum += biases[n1]; // Add bias for this neuron

                lay1_outs[n1] = sum; // Store intermediate result
            }

            // Apply activation function to all neurons in the current layer
            for (int n1 = 0; n1 < currentLayerSize; ++n1)
                lay1_outs[n1] = Activate(lay1_outs[n1]);

            // Swap buffers, next-layer output becomes current-layer output
            std::swap(lay0_outs, lay1_outs);
        }

        // Copy final layer outputs to outputs array
        for (int i = 0; i < mArchitecture.back(); ++i)
            pOutputs[i] = lay0_outs[i];
    }

    // Get the architecture of the network
    const auto& GetArchitecture() const { return mArchitecture; }

    // Get the total number of parameters (weights + biases) in the network
    size_t GetTotalParameterCount() const {
        return CalcTotalParameters(mArchitecture);
    }

    // Set the network parameters from layer structure
    void SetLayerParameters(const std::vector<LayerParameters>& layerParams)
    {
        assert(layerParams.size() == mLayerParams.size());
        // Could add more detailed size checks per layer here if needed
        mLayerParams = layerParams;
    }

    // Get the network parameters as layer structure
    const std::vector<LayerParameters>& GetLayerParameters() const { return mLayerParams; }
     // Get mutable access for mutation (use with caution)
    std::vector<LayerParameters>& GetLayerParameters() { return mLayerParams; }

    // Initialize parameters with random values
    void InitializeRandomParameters(uint32_t seed, float minVal = -1.0f, float maxVal = 1.0f)
    {
        (void)minVal; (void)maxVal;
#if SNN_INIT_RANDOM_UNIFORM
        std::mt19937 rng(seed);
        std::uniform_real_distribution<float> dist(minVal, maxVal);

        for (auto& layer : mLayerParams)
        {
            for (float& weight : layer.weights)
                weight = dist(rng);
            for (float& bias : layer.biases)
                bias = dist(rng);
        }
#elif SNN_INIT_HE_NORMAL
        std::mt19937 rng(seed);
        // Initialize weights and biases layer by layer
        for (size_t layerIdx = 0; layerIdx < mLayerParams.size(); ++layerIdx)
        {
            // --- Initialize Weights (He Normal) ---
            auto fan_in = mArchitecture[layerIdx];
            // Avoid division by zero if a layer has 0 inputs (shouldn't happen in valid architecture)
            float stddev = (fan_in > 0) ? std::sqrt(2.0f / (float)fan_in) : 0.0f;
            std::normal_distribution<float> weight_dist(0.0f, stddev);

            auto& layer_weights = mLayerParams[layerIdx].weights;
            for (float& w : layer_weights)
                w = weight_dist(rng);

            // --- Initialize Biases (Zero) ---
            for (float& b : mLayerParams[layerIdx].biases)
                b = 0.0f;
        }
#elif SNN_INIT_XAVIER_UNIFORM
        std::mt19937 rng(seed);
        // Initialize weights and biases layer by layer
        for (size_t layerIdx = 0; layerIdx < mLayerParams.size(); ++layerIdx)
        {
            // --- Initialize Weights (Xavier Uniform) ---
            auto fan_in = mArchitecture[layerIdx];
            auto fan_out = mArchitecture[layerIdx + 1];
            // Calculate the range limit for Xavier uniform initialization
            float limit = (fan_in + fan_out > 0) ? std::sqrt(6.0f / (float)(fan_in + fan_out)) : 0.0f;
            std::uniform_real_distribution<float> weight_dist(-limit, limit);

            auto& layer_weights = mLayerParams[layerIdx].weights;
            for (float& w : layer_weights)
                w = weight_dist(rng);

            // --- Initialize Biases (Zero) ---
            for (float& b : mLayerParams[layerIdx].biases)
                b = 0.0f;
        }
#endif
    }

private:
    float Activate(float x) const { return x > 0.0f ? x : 0.0f; } // ReLU
    //float Activate(float x) const { return x > 0.0f ? x : 0.01f * x; } // Leaky ReLU
};

}

#endif
