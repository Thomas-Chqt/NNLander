#ifndef SIMPLE_NEURAL_NET_H
#define SIMPLE_NEURAL_NET_H

#include <vector>
#include <algorithm>
#include <cmath>

//==================================================================
class SimpleNeuralNet
{
    const std::vector<int> mArchitecture; // Network architecture (nodes per layer)
    size_t mTotalParameters = 0;          // Total number of parameters in the network
    size_t mMaxLayerSize = 0;             // Maximum number of neurons in any layer
public:
/*
The constructor takes pointers to weights (weights and biases) and the
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
        // Calculate total number of parameters needed
        mTotalParameters = CalcTotalParameters(mArchitecture);

        // Find the maximum number of neurons in any layer
        mMaxLayerSize = *std::max_element(mArchitecture.begin(), mArchitecture.end());
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
    // inputs -> net(parameters) -> outputs
    //==================================================================
    void FeedForward(const float* pParameters, const float* pInputs, float* pOutputs) const
    {
        // Allocate buffers on the stack to avoid touching the heap
        float* lay0_outs = (float*)alloca(mMaxLayerSize * sizeof(float));
        float* lay1_outs = (float*)alloca(mMaxLayerSize * sizeof(float));

        // Copy inputs (simulation states) to first layer outputs
        for (int i=0; i < mArchitecture[0]; ++i)
            lay0_outs[i] = pInputs[i];

        // Parameter index tracker
        int paramIdx = 0;

        // Process each layer
        for (size_t lay_1=1; lay_1 < mArchitecture.size(); ++lay_1)
        {
            const auto lay1_n = mArchitecture[lay_1];
            const auto lay0_n = mArchitecture[lay_1-1];

            // For each neuron in the current layer
            for (int n1=0; n1 < lay1_n; ++n1)
            {
                // Sum weighted inputs
                auto sum = 0.0f;
                for (int n0=0; n0 < lay0_n; ++n0)
                    sum += lay0_outs[n0] * pParameters[paramIdx++];
                sum += pParameters[paramIdx++]; // Add bias

                lay1_outs[n1] = sum; // Store
            }
            // Apply activation function to each neuron (faster to do here than in the loop)
            for (int n1=0; n1 < lay1_n; ++n1)
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

    // Get total number of parameters (weights + biases)
    size_t GetTotalParameters() const { return mTotalParameters; }

private:
    float Activate(float x) const { return x > 0.0f ? x : 0.0f; } // ReLU
    //float Activate(float x) const { return x > 0.0f ? x : 0.01f * x; } // Leaky ReLU
};

#endif 