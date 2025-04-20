#include <cstddef>
#include <gtest/gtest.h>
#include "fixitures.h"

class FeedForwardTest10x3 : public FeedForwardTest<std::array<int, 2>{10, 3}> {};
TEST_F(FeedForwardTest10x3, basicTest)
{
    this->initInputsAndParams(1234, 0.0f, 1.0f);

    auto outputs_dp1 = this->FeedForward_dp1();
    auto outputs_tc1 = this->FeedForward_tc1();
    auto outputs_dp2 = this->FeedForward_dp2();
    auto outputs_cur = this->FeedForward_cur();

    for (size_t i = 0; i < outputs_dp1.size(); i++)
    {
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_tc1[i]);
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_dp2[i]);
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_cur[i]);
    }
}


class FeedForwardTest10x3x4x6x30x20x4x50 : public FeedForwardTest<std::array<int, 8>{10, 3, 4, 6, 30, 20, 4, 50}> {};
TEST_F(FeedForwardTest10x3x4x6x30x20x4x50, basicTest)
{
    this->initInputsAndParams(5678, -1.0f, 0.0f);

    auto outputs_dp1 = this->FeedForward_dp1();
    auto outputs_tc1 = this->FeedForward_tc1();
    auto outputs_dp2 = this->FeedForward_dp2();
    auto outputs_cur = this->FeedForward_cur();

    for (size_t i = 0; i < outputs_dp1.size(); i++)
    {
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_tc1[i]);
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_dp2[i]);
        EXPECT_FLOAT_EQ(outputs_dp1[i], outputs_cur[i]);
    }
}
