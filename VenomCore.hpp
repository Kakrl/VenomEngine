#include <cstdint>
#include <vector>
#include <iostream>

// Ensure the struct fits exactly or is padded to a 64-byte cache line
struct alignas(64) Order {
    uint64_t id;         // 8 bytes
    int32_t price;       // 4 bytes
    uint32_t qty;        // 4 bytes
    uint64_t timestamp;  // 8 bytes
    
    // Intrusive "Pointers" (Indices into the pool)
    int32_t next_idx = -1; // 4 bytes
    int32_t prev_idx = -1; // 4 bytes
    
    bool is_buy;         // 1 byte
    // Remaining bytes are padding to hit 64 bytes total
    char padding[31]; 
};

class OrderPool {
private:
    std::vector<Order> pool;
    std::vector<int32_t> free_indices;
    
public:
    OrderPool(size_t max_orders) {
        pool.resize(max_orders);
        free_indices.reserve(max_orders);
        // Fill free list backwards so we pop from the front of the array first
        for (int32_t i = max_orders - 1; i >= 0; --i) {
            free_indices.push_back(i);
        }
    }

    // O(1) Acquisition
    int32_t acquire() {
        if (free_indices.empty()) return -1;
        int32_t idx = free_indices.back();
        free_indices.pop_back();
        return idx;
    }

    // O(1) Release
    void release(int32_t idx) {
        pool[idx].next_idx = -1;
        pool[idx].prev_idx = -1;
        free_indices.push_back(idx);
    }

    // Direct access via index
    Order& operator[](int32_t idx) { return pool[idx]; }
};