#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#endif
#include <iostream>

inline void pin_thread_to_core(int core_id) {
#ifdef _WIN32
    // Windows: SetThreadAffinityMask uses a bitmask (1 << core_id)
    HANDLE thread = GetCurrentThread();
    DWORD_PTR mask = (1ULL << core_id);
    if (SetThreadAffinityMask(thread, mask) == 0) {
        std::cerr << "Failed to set Windows thread affinity." << std::endl;
    }
#else
    // Linux: pthread_setaffinity_np uses cpu_set_t
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        std::cerr << "Failed to set Linux thread affinity: " << rc << std::endl;
    }
#endif
}