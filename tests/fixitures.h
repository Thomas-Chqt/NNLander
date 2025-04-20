#ifndef FIXITURES_H
#define FIXITURES_H

#include "SimpleNeuralNet.h"
#include "dp1/SimpleNeuralNet.h"
#include "dp2/SimpleNeuralNet.h"
#include "tc1/TemplateFeedForward.hpp"
#include <cstddef>
#include <iostream>
#include <vector>

#ifdef BENCHMARK_FIXITURE
    #include <benchmark/benchmark.h>
#else
    #include <gtest/gtest.h>
#endif

constexpr size_t CalcTotalParameters(auto architecture)
{
    size_t n = 0;
    for (size_t i = 1; i < architecture.size(); ++i)
        n += architecture[i - 1] * architecture[i] + architecture[i];
    return n;
}

template<auto netArch>
#ifdef BENCHMARK_FIXITURE
class FeedForwardBenchmarck : public benchmark::Fixture
#else
class FeedForwardTest : public testing::Test
#endif
{
public:
    using InputArray = std::array<float, netArch.front()>;
    using OutputArray = std::array<float, netArch.back()>;
    using ParamArray = std::array<float, CalcTotalParameters(netArch)>;

protected:
    void initInputsAndParams(int seed, float low, float high)
    {
        std::mt19937 mRng(seed);
        std::normal_distribution<float> dist(low, high);

        InputArray inputs;
        for (auto& input : inputs)
            input = dist(mRng);
        this->setInputs(inputs);

        ParamArray params;
        for (auto& param : params)
            param = dist(mRng);
        this->setParams(params);
    }


    void setInputs(const InputArray& inputs)
    {
        setInputs_dp1(inputs);
        setInputs_tc1(inputs);
        setInputs_dp2(inputs);
        setInputs_cur(inputs);
    }

    void setParams(const ParamArray& params)
    {
        setParams_dp1(params);
        setParams_tc1(params);
        setParams_dp2(params);
        setParams_cur(params);
    }

    OutputArray FeedForward_dp1()
    {
        OutputArray output;
        m_dp1Net.FeedForward(m_dp1Params.data(), m_dp1Inputs.data(), output.data());
        return output;
    }

    Eigen::Vector<float, netArch.back()> FeedForward_tc1()
    {
        Eigen::Vector<float, netArch.back()> outputs;
        std::apply([&](const auto&... params) { tc1::FeedForward(m_tc1Inputs, outputs, params...); }, m_tc1Params);
        return outputs;
    }

    OutputArray FeedForward_dp2()
    {
        OutputArray output;
        m_dp2Net.FeedForward(m_dp2Inputs.data(), output.data());
        return output;
    }

    ::SimpleNeuralNet<float, netArch>::Outputs FeedForward_cur()
    {
        typename ::SimpleNeuralNet<float, netArch>::Outputs outputs;
        m_curNet.FeedForward(m_curInputs, outputs);
        return outputs;
    }

private:
    void setInputs_dp1(const InputArray& inputs)
    {
        m_dp1Inputs = inputs;
    }

    void setInputs_tc1(const InputArray& inputs)
    {
        for (size_t i = 0; i < inputs.size(); i++)
            m_tc1Inputs[i] = inputs[i];
    }

    void setInputs_dp2(const InputArray& inputs)
    {
        m_dp2Inputs = inputs;
    }

    void setInputs_cur(const InputArray& inputs)
    {
        for (size_t i = 0; i < inputs.size(); i++)
            m_curInputs[i] = inputs[i];
    }

    void setParams_dp1(const ParamArray& params)
    {
        m_dp1Params = params;
    }

    void setParams_tc1(const ParamArray& params)
    {
        size_t paramIdx = 0;
        tc1::fillNetParams<float, netArch>([&](int, int, int) { return params[paramIdx++]; }, m_tc1Params);
    }

    void setParams_dp2(const ParamArray& params)
    {
        std::vector<dp2::LayerParameters>& layers = m_dp2Net.GetLayerParameters();
        size_t paramIdx = 0;
        size_t sz = netArch.size();
        for (size_t l = 0; l < sz-1; l++)
        {
            dp2::LayerParameters& layer = layers[l];
            size_t biasIdx = 0;
            for (size_t r = 0; r < netArch[l+1]; r++)
            {
                for (size_t c = 0; c < netArch[l]; c++)
                    layer.weights[r * netArch[l] + c] = params[paramIdx++];
                layer.biases[biasIdx++] = params[paramIdx++];
            }
        }
    }

    void setParams_cur(const ParamArray& params)
    {
        size_t paramIdx = 0;
        m_curNet.foreachParameters([&](int, int, int, float& param) { param = params[paramIdx++]; });
    }

private:
    dp1::SimpleNeuralNet m_dp1Net = std::vector<int>(netArch.begin(), netArch.end());
    InputArray m_dp1Inputs;
    ParamArray m_dp1Params;

    Eigen::Vector<float, netArch.front()> m_tc1Inputs;
    tc1::NetParam<float, netArch> m_tc1Params;

    dp2::SimpleNeuralNet m_dp2Net = std::vector<int>(netArch.begin(), netArch.end());
    InputArray m_dp2Inputs;

    ::SimpleNeuralNet<float, netArch> m_curNet;
    ::SimpleNeuralNet<float, netArch>::Inputs m_curInputs;
};

#endif // FIXITURES_H
