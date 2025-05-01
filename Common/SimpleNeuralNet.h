#ifndef SIMPLE_NEURAL_NET_H
#define SIMPLE_NEURAL_NET_H

#include <array>
#include <concepts>
#include <tuple>
#include <cmath>
#include <random> // Needed for InitializeRandomParameters
#include <cassert>   // For assert
#include <Eigen/Dense>

#define SNN_INIT_RANDOM_UNIFORM 0
#define SNN_INIT_HE_NORMAL 0
#define SNN_INIT_XAVIER_UNIFORM 1

template<typename T, typename Scalar>
concept EigenMatrix = requires
{
    requires std::is_base_of_v<Eigen::MatrixBase<std::decay_t<T>>, std::decay_t<T>>;
    requires std::is_same_v<typename T::Scalar, Scalar>;
};

template<typename T, typename Scalar, int Cols>
concept EigenMatrixC = requires
{
    requires EigenMatrix<T, Scalar>;
    requires std::decay_t<T>::ColsAtCompileTime == Cols;
};

template<typename T>
concept NetArch = requires(T t)
{
    { t.size()  } -> std::convertible_to<size_t>;
    { t.front() } -> std::convertible_to<int>;
    { t.back()  } -> std::convertible_to<int>;
    { t[0]      } -> std::convertible_to<int>;
};

template<typename T, typename Y>
concept OnParamFunc = requires(T t, int l, int r, int c, Y& param)
{
    { t(l, r, c, param) };
};

template<typename T, typename Y>
concept OnConstParamFunc = requires(T t, int l, int r, int c, const Y& param)
{
    { t(l, r, c, param) };
};

template<std::floating_point T, NetArch auto netArch>
class SimpleNeuralNet
{
public:
    using Parameters = decltype([]<size_t... Is>(std::index_sequence<Is...>) {
        return std::tuple<Eigen::Matrix<T, netArch[Is+1], netArch[Is]+1>...>{};
    }(std::make_index_sequence<netArch.size()-1>{}));

    using Inputs = Eigen::Vector<T, netArch.front()>;
    using Outputs = Eigen::Vector<T, netArch.back()>;

private:
    Parameters mParams;

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
    SimpleNeuralNet()
    {
        assert(netArch.size() >= 2); // TODO : check at compile time
    }

    // Copy constructor
    SimpleNeuralNet(const SimpleNeuralNet& other) : mParams(other.mParams)
    {
    }

    // Copy assignment operator
    SimpleNeuralNet& operator=(const SimpleNeuralNet& other)
    {
        if (this != &other) // Handle self-assignment
        {
            mParams = other.mParams; // Copy parameters (deep copy)
        }
        return *this;
    }

    // Calculate the total number of parameters
    constexpr static size_t CalcTotalParameters()
    {
        size_t n = 0;
        for (size_t i = 1; i < netArch.size(); ++i)
            n += netArch[i-1] * netArch[i] + netArch[i];
        return n;
    }

    //==================================================================
    // Feed forward function
    // This function builds a net with the given Parameters and then
    // applies the Inputs to the net to get the Outputs.
    // inputs -> net -> outputs
    //==================================================================

    void FeedForward(const Inputs& pInputs, Outputs& pOutputs) const
    {
        std::apply([&](const auto&... params) { this->FeedForward(pInputs, pOutputs, params...); }, mParams);
    }
    
    // Get the total number of parameters (weights + biases) in the network
    constexpr size_t GetTotalParameterCount() const { return CalcTotalParameters(); }

    // Set the network parameters from layer structure
    inline void SetParameters(const Parameters& pParams) { mParams = pParams; }

    // Get the network parameters as layer structure
    const Parameters& GetParameters() const { return mParams; }

     // Get mutable access for mutation (use with caution)
    Parameters& GetParameters() { return mParams; }

    T& GetParameter(int layer, int row, int col)
    {
        // each element of the array return the r, c of one layer, this allow to get a layer with a
        // non constexpr layer index
        static constexpr auto getParamsFuncArray = []<size_t... Idxs>(std::index_sequence<Idxs...>) {
            return std::array<T&(*)(Parameters&, int, int), std::tuple_size_v<Parameters>>{ [](Parameters& p, int r, int c) -> T& { return std::get<Idxs>(p)(r, c); }... };
        }(std::make_index_sequence<std::tuple_size_v<Parameters>>{});

        return getParamsFuncArray[layer](mParams, row, col);
    }

    const T& GetParameter(int layer, int row, int col) const
    {
        return const_cast<SimpleNeuralNet*>(this)->GetParameter(layer, row, col);
    }

    void foreachParameters(const OnParamFunc<T> auto& func)
    {
        auto fillLayer = [&]<size_t Idx>() {
            EigenMatrix<T> auto& layer = std::get<Idx>(mParams);
            for (int r = 0; r < std::remove_cvref_t<decltype(layer)>::RowsAtCompileTime; r++) {
                for (int c = 0; c < std::remove_cvref_t<decltype(layer)>::ColsAtCompileTime; c++) {
                    func(Idx, r, c, layer(r, c));
                }
            }
        };

        [&]<size_t... Idxs>(std::index_sequence<Idxs...>) {
            (fillLayer.template operator()<Idxs>(), ...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(mParams)>>>{});
    }

    void foreachParameters(const OnConstParamFunc<T> auto& func) const
    {
        const_cast<SimpleNeuralNet*>(this)->foreachParameters([&](int l, int r, int c, T& param){ func(l, r, c, param); });
    }

    // Initialize parameters with random values
    void InitializeRandomParameters(uint32_t seed)
    {
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
        foreachParameters([&](int layerIdx, int row, int col, float& param){
            (void)row;
            // --- Initialize Weights (Xavier Uniform) ---
            auto fan_in = netArch[layerIdx];
            auto fan_out = netArch[layerIdx + 1];
            // Calculate the range limit for Xavier uniform initialization
            float limit = (fan_in + fan_out > 0) ? std::sqrt(6.0f / (float)(fan_in + fan_out)) : 0.0f;
            std::uniform_real_distribution<float> weight_dist(-limit, limit);

            if (col < netArch[layerIdx])
                param = weight_dist(rng);

            // --- Initialize Biases (Zero) ---
            else
                param = 0;
        });
#endif
    }

private:
    float Activate(float x) const { return x > 0.0f ? x : 0.0f; } // ReLU
    //float Activate(float x) const { return x > 0.0f ? x : 0.01f * x; } // Leaky ReLU
    
    template<int I, int O>
    void FeedForward(const Eigen::Vector<T, I>& pInputs, Eigen::Vector<T, O>& pOutputs, const Eigen::Matrix<T, O, I+1>& pParams) const
    {
        pOutputs = (pParams * pInputs.homogeneous()).unaryExpr([&](T x) { return Activate(x); });
    }

    template<int I, int O>
    void FeedForward(const Eigen::Vector<T, I>& pInputs, Eigen::Vector<T, O>& pOutputs, const EigenMatrixC<T, I+1> auto& pParams,
                     const EigenMatrix<T> auto&  pRemaingParams, const EigenMatrix<T> auto& ... pRemaingParamsPack) const
    {
        Eigen::Vector<T, std::remove_cvref_t<decltype(pParams)>::RowsAtCompileTime> outputs;
        FeedForward(pInputs, outputs, pParams);
        FeedForward(outputs, pOutputs, pRemaingParams, pRemaingParamsPack...);
    }
};

#endif
