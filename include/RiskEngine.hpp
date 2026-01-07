#pragma once
#include "RingBuffer.hpp" // For TradeEvent definition
#include <vector>

class RiskEngine {
private:
    float current_buy_vol = 0.0f;
    float current_sell_vol = 0.0f;
    const float bucket_volume_threshold = 1000.0f;

    std::vector<float> buy_buckets;
    std::vector<float> sell_buckets;

public:
    RiskEngine() {
        buy_buckets.reserve(16);
        sell_buckets.reserve(16);
    }

    // This is called by the Risk Thread in main.cpp
    void on_trade(const TradeEvent& event);

    // The AVX-512 Math Kernel (implemented in .cpp)
    float run_vpin_calculation();
};