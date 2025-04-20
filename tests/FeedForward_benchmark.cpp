#define BENCHMARK_FIXITURE
#include "fixitures.h"

BENCHMARK_TEMPLATE_F(FeedForwardBenchmarck, 10x3dp1, std::array<int, 2>{10, 3})(benchmark::State& st) {
    this->initInputsAndParams(1234, 0.0f, 1.0f);
    for (auto _ : st) {
        this->FeedForward_dp1();
    }
}

BENCHMARK_TEMPLATE_F(FeedForwardBenchmarck, 10x3tc1, std::array<int, 2>{10, 3})(benchmark::State& st) {
    this->initInputsAndParams(1234, 0.0f, 1.0f);
    for (auto _ : st) {
        this->FeedForward_tc1();
    }
}

BENCHMARK_TEMPLATE_F(FeedForwardBenchmarck, 10x3dp2, std::array<int, 2>{10, 3})(benchmark::State& st) {
    this->initInputsAndParams(1234, 0.0f, 1.0f);
    for (auto _ : st) {
        this->FeedForward_dp2();
    }
}

BENCHMARK_TEMPLATE_F(FeedForwardBenchmarck, 10x3cur, std::array<int, 2>{10, 3})(benchmark::State& st) {
    this->initInputsAndParams(1234, 0.0f, 1.0f);
    for (auto _ : st) {
        this->FeedForward_cur();
    }
}
////////////////////////////


BENCHMARK_TEMPLATE_F(FeedForwardBenchmarck, 10x20x30x40x30x20x10dp1, std::array<int, 7>{10,20,30,40,30,20,10})(benchmark::State& st) {
    this->initInputsAndParams(1234, 0.0f, 1.0f);
    for (auto _ : st) {
        this->FeedForward_dp1();
    }
}

BENCHMARK_TEMPLATE_F(FeedForwardBenchmarck, 10x20x30x40x30x20x10tc1, std::array<int, 7>{10,20,30,40,30,20,10})(benchmark::State& st) {
    this->initInputsAndParams(1234, 0.0f, 1.0f);
    for (auto _ : st) {
        this->FeedForward_tc1();
    }
}

BENCHMARK_TEMPLATE_F(FeedForwardBenchmarck, 10x20x30x40x30x20x10dp2, std::array<int, 7>{10,20,30,40,30,20,10})(benchmark::State& st) {
    this->initInputsAndParams(1234, 0.0f, 1.0f);
    for (auto _ : st) {
        this->FeedForward_dp2();
    }
}

BENCHMARK_TEMPLATE_F(FeedForwardBenchmarck, 10x20x30x40x30x20x10cur, std::array<int, 7>{10,20,30,40,30,20,10})(benchmark::State& st) {
    this->initInputsAndParams(1234, 0.0f, 1.0f);
    for (auto _ : st) {
        this->FeedForward_cur();
    }
}
