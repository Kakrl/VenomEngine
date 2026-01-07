#include <atomic>
#include <array>

struct TradeEvent {
    uint32_t qty;
    int32_t price;
    bool is_buy_side;
};

template<size_t Capacity>
class SPSCRingBuffer {
private:
    // Align to 64 bytes to prevent "False Sharing" between Producer and Consumer
    alignas(64) std::atomic<size_t> head{0}; 
    alignas(64) std::atomic<size_t> tail{0};
    std::array<TradeEvent, Capacity> buffer;

public:
    bool push(const TradeEvent& event) {
        size_t t = tail.load(std::memory_order_relaxed);
        size_t next_t = (t + 1) % Capacity;
        if (next_t == head.load(std::memory_order_acquire)) return false; // Full

        buffer[t] = event;
        tail.store(next_t, std::memory_order_release);
        return true;
    }

    bool pop(TradeEvent& event) {
        size_t h = head.load(std::memory_order_relaxed);
        if (h == tail.load(std::memory_order_acquire)) return false; // Empty

        event = buffer[h];
        head.store((h + 1) % Capacity, std::memory_order_release);
        return true;
    }
};