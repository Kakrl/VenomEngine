#include <chrono>
#include <vector>
#include <algorithm>
#include <iostream>
#include <numeric>

class LatencyTracker {
private:
    std::vector<double> samples;
    const size_t max_samples;

public:
    LatencyTracker(size_t capacity) : max_samples(capacity) {
        samples.reserve(capacity);
    }

    // Record a measurement in nanoseconds
    void record(std::chrono::nanoseconds duration) {
        if (samples.size() < max_samples) {
            samples.push_back(static_cast<double>(duration.count()));
        }
    }

    void report() {
        if (samples.empty()) return;

        std::sort(samples.begin(), samples.end());
        
        double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
        double avg = sum / samples.size();
        double p50 = samples[samples.size() * 0.50];
        double p90 = samples[samples.size() * 0.90];
        double p99 = samples[samples.size() * 0.99];
        double max_lat = samples.back();

        std::cout << "\n--- Latency Report (Nanoseconds) ---" << std::endl;
        std::cout << "Samples: " << samples.size() << std::endl;
        std::cout << "Average: " << avg << " ns" << std::endl;
        std::cout << "P50    : " << p50 << " ns" << std::endl;
        std::cout << "P99    : " << p99 << " ns" << std::endl;
        std::cout << "Max    : " << max_lat << " ns" << std::endl;
        std::cout << "------------------------------------\n" << std::endl;
    }
};