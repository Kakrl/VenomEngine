#include "VenomCore.hpp"

struct PriceLevel {
    int32_t head_idx = -1;
    int32_t tail_idx = -1;
    uint32_t total_volume = 0;

    // Adds an order to the end of the queue (Time Priority)
    void add_order(int32_t order_idx, OrderPool& pool) {
        Order& order = pool[order_idx];
        total_volume += order.qty;

        if (tail_idx == -1) {
            // Level is empty
            head_idx = tail_idx = order_idx;
            order.next_idx = -1;
            order.prev_idx = -1;
        } else {
            // Link the current tail to the new order
            pool[tail_idx].next_idx = order_idx;
            order.prev_idx = tail_idx;
            order.next_idx = -1;
            tail_idx = order_idx;
        }
    }

    // Removes an order from the middle or ends (Cancellations)
    void remove_order(int32_t order_idx, OrderPool& pool) {
        Order& order = pool[order_idx];
        total_volume -= order.qty;

        if (order.prev_idx != -1) {
            pool[order.prev_idx].next_idx = order.next_idx;
        } else {
            head_idx = order.next_idx; // It was the head
        }

        if (order.next_idx != -1) {
            pool[order.next_idx].prev_idx = order.prev_idx;
        } else {
            tail_idx = order.prev_idx; // It was the tail
        }
        
        // Reset the order's own links for safety
        order.next_idx = -1;
        order.prev_idx = -1;
    }
};