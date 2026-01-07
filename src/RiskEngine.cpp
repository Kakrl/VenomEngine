#include "RiskEngine.hpp"
#include <immintrin.h>
#include <iostream>

void RiskEngine::on_trade(const TradeEvent& event) {
    if (event.is_buy_side) current_buy_vol += event.qty;
    else current_sell_vol += event.qty;

    // Once we hit the volume threshold, "close" the bucket
    if ((current_buy_vol + current_sell_vol) >= bucket_volume_threshold) {
        buy_buckets.push_back(current_buy_vol);
        sell_buckets.push_back(current_sell_vol);

        current_buy_vol = 0;
        current_sell_vol = 0;

        // If we have 16 buckets, run the high-performance math
        if (buy_buckets.size() >= 16) {
            float vpin = run_vpin_calculation();
            std::cout << "[Risk Engine] Current VPIN: " << vpin << std::endl;
            buy_buckets.clear();
            sell_buckets.clear();
        }
    }
}

float RiskEngine::run_vpin_calculation() {
    // This is the AVX-512 code we wrote earlier
    // For now, we'll use the buy_buckets.data() and sell_buckets.data()
    __m512 b = _mm512_loadu_ps(buy_buckets.data());
    __m512 s = _mm512_loadu_ps(sell_buckets.data());
    __m512 diff = _mm512_sub_ps(b, s);
    __m512 abs_diff = _mm512_andnot_ps(_mm512_set1_ps(-0.0f), diff);
    
    float sum = _mm512_reduce_add_ps(abs_diff);
    return sum / (16.0f * bucket_volume_threshold);
}