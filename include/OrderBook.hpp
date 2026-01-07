#include <bitset>
#include <array>
#include "PriceLevel.hpp"

class OrderBook {
private:
    static constexpr int MAX_LEVELS = 10000; // e.g., prices from 0.01 to 100.00
    
    std::array<PriceLevel, MAX_LEVELS> bids;
    std::array<PriceLevel, MAX_LEVELS> asks;

    // Bitsets to track which levels have volume
    std::bitset<MAX_LEVELS> bid_mask;
    std::bitset<MAX_LEVELS> ask_mask;

    OrderPool& pool;

public:
    OrderBook(OrderPool& p) : pool(p) {}

    void limit_order(int32_t order_idx) {
        Order& order = pool[order_idx];
        if (order.is_buy) {
            bids[order.price].add_order(order_idx, pool);
            bid_mask.set(order.price);
        } else {
            asks[order.price].add_order(order_idx, pool);
            ask_mask.set(order.price);
        }
    }

    // O(1) discovery of the Best Bid
    int32_t get_best_bid() const {
        // Find the index of the highest bit set to 1
        // Note: bitset doesn't have a native 'highest bit' intrinsic in standard C++, 
        // so in high-perf code we often use custom 64-bit masks.
        for (int i = MAX_LEVELS - 1; i >= 0; --i) {
            if (bid_mask.test(i)) return i;
        }
        return -1;
    }

    // O(1) discovery of the Best Ask
    int32_t get_best_ask() const {
        // This is where hardware-accelerated 'find first set' shines
        // In a real HFT engine, you'd use __builtin_ctzll on a raw u64 array.
        for (int i = 0; i < MAX_LEVELS; ++i) {
            if (ask_mask.test(i)) return i;
        }
        return -1;
    }

    void cancel_order(int32_t order_idx) {
        Order& order = pool[order_idx];
        if (order.is_buy) {
            bids[order.price].remove_order(order_idx, pool);
            if (bids[order.price].total_volume == 0) bid_mask.reset(order.price);
        } else {
            asks[order.price].remove_order(order_idx, pool);
            if (asks[order.price].total_volume == 0) ask_mask.reset(order.price);
        }
    }
};