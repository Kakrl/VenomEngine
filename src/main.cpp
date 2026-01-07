#include <iostream>
#include <thread>
#include <atomic>
#include "OrderBook.hpp"
#include "RingBuffer.hpp"
#include "RiskEngine.hpp"
#include "LatencyTracker.hpp"
#include "Common.hpp"

std::atomic<bool> keep_running{true};

void run_mock_market_feed(OrderBook& book, OrderPool& pool) {
    // Pin to Core 3 to keep it away from the Matching and Risk engines
    pin_thread_to_core(3);

    int32_t current_price = 5000; // Starting price

    while (keep_running) {
        int32_t order_idx = pool.acquire();
        if (order_idx == -1) continue;

        Order& o = pool[order_idx];
        
        // Randomly move price +/- 1 tick
        current_price += (rand() % 3 - 1); 
        
        o.price = current_price;
        o.qty = (rand() % 100) + 1;
        o.is_buy = (rand() % 2 == 0);

        book.limit_order(order_idx);

        // Slow down slightly so we don't overwhelm the console immediately
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void run_matching_engine(OrderBook& book, SPSCRingBuffer<1024>& queue, OrderPool& pool) {
    // Pin to Core 1
    pin_thread_to_core(1); 

    std::cout << "[Matching Engine] Running on Core 1" << std::endl;

    // 2. Initialize the tracker inside the thread (to avoid cross-thread noise)
    LatencyTracker tracker(10000); 
    int32_t sample_count = 0;

    std::cout << "[Matching Engine] Running on Core 1" << std::endl;

    while (keep_running) {
        int32_t order_idx = pool.acquire();
        if (order_idx == -1) continue;

        Order& o = pool[order_idx];
        o.price = 5000;
        o.qty = 10;
        o.is_buy = true;

        // Timing logic...
        auto start = std::chrono::high_resolution_clock::now();
        book.limit_order(order_idx);
        auto end = std::chrono::high_resolution_clock::now();
        tracker.record(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start));

        // Corrected Event Initialization
        TradeEvent event;
        event.qty = o.qty;
        event.price = o.price;
        event.is_buy_side = o.is_buy;

        // Push to the Lock-Free Queue
        while (!queue.push(event)) {
            // Spin if queue is full
        }
    }
}

// The Risk Engine "Math Path"
void run_risk_engine(SPSCRingBuffer<1024>& queue) {
    // Pin to Core 2
    pin_thread_to_core(2); 

    RiskEngine risk_calc;
    std::cout << "[Risk Engine] Running on Core 2" << std::endl;

    while (keep_running) {
        TradeEvent event;
        if (queue.pop(event)) {
            // Update buckets and potentially trigger AVX-512 VPIN calculation
            risk_calc.on_trade(event);
        }
        // No 'sleep' here—we want to poll the ring buffer as fast as possible
    }
}

int main() {
    OrderPool pool(1000000); // 1 Million orders pre-allocated
    OrderBook book(pool);
    SPSCRingBuffer<4096> trade_queue;

    // Launch the 3-headed beast
    std::thread matching_thread(run_matching_engine, std::ref(book), std::ref(trade_queue), std::ref(pool));
    std::thread risk_thread(run_risk_engine, std::ref(trade_queue));
    std::thread market_thread(run_mock_market_feed, std::ref(book), std::ref(pool));

    std::cout << "VenomEngine Testing Live..." << std::endl;
    std::cin.get(); // Wait for user to press Enter

    keep_running = false;
    matching_thread.join();
    risk_thread.join();
    market_thread.join();

    return 0;
}