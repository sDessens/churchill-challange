#ifndef TIMER_H
#define TIMER_H

#include <iostream>

#include <atomic>

struct rdtsc_timer {
    rdtsc_timer()
        :   first(__rdtsc())
    {
        std::atomic_thread_fence(std::memory_order_seq_cst);
    }

    double elapsed()
    {
        std::atomic_thread_fence(std::memory_order_seq_cst);

        unsigned long long last = __rdtsc();
        auto diff = last - first;
        return diff / 2300000000.0; // insert your cpu's stock speed here.
    }

    unsigned long long first;
};

#endif // TIMER_H
