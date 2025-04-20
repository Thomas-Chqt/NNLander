#include <cstddef>
#include <gtest/gtest.h>
#include "fixitures.h"

class FeedForwardTest10x3 : public FeedForwardTest<std::array<int, 2>{10, 3}> {};
TEST_F(FeedForwardTest10x3, basicTest)
{
    std::mt19937 mRng(1234);
    std::normal_distribution<float> dist(0.0f, 1.0f);

    FeedForwardTest10x3::InputArray inputs;
    for (auto& input : inputs)
        input = dist(mRng);
    this->setInputs(inputs);

    FeedForwardTest10x3::ParamArray params;
    for (auto& param : params)
        param = dist(mRng);
    this->setParams(params);

    FeedForwardTest10x3::OutputArray outputs_dp1 = this->FeedForward_dp1();
    FeedForwardTest10x3::OutputArray outputs_tc1 = this->FeedForward_tc1();
    FeedForwardTest10x3::OutputArray outputs_dp2 = this->FeedForward_dp2();
    FeedForwardTest10x3::OutputArray outputs_cur = this->FeedForward_cur();

    for (size_t i = 0; i < FeedForwardTest10x3::OutputArray().size(); i++)
    {
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_tc1[i]);
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_dp2[i]);
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_cur[i]);
    }
}


class FeedForwardTest10x3x4x6x30x20x4x50 : public FeedForwardTest<std::array<int, 8>{10, 3, 4, 6, 30, 20, 4, 50}> {};
TEST_F(FeedForwardTest10x3x4x6x30x20x4x50, basicTest)
{
    std::mt19937 mRng(1234);
    std::normal_distribution<float> dist(0.0f, 1.0f);

    FeedForwardTest10x3x4x6x30x20x4x50::InputArray inputs;
    for (auto& input : inputs)
        input = dist(mRng);
    this->setInputs(inputs);

    FeedForwardTest10x3x4x6x30x20x4x50::ParamArray params;
    for (auto& param : params)
        param = dist(mRng);
    this->setParams(params);

    FeedForwardTest10x3x4x6x30x20x4x50::OutputArray outputs_dp1 = this->FeedForward_dp1();
    FeedForwardTest10x3x4x6x30x20x4x50::OutputArray outputs_tc1 = this->FeedForward_tc1();
    FeedForwardTest10x3x4x6x30x20x4x50::OutputArray outputs_dp2 = this->FeedForward_dp2();
    FeedForwardTest10x3x4x6x30x20x4x50::OutputArray outputs_cur = this->FeedForward_cur();

    for (size_t i = 0; i < FeedForwardTest10x3x4x6x30x20x4x50::OutputArray().size(); i++)
    {
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_tc1[i]);
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_dp2[i]);
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_cur[i]);
    }
}
