#ifndef SYNCFIFO_SC_STIMULUS_H
#define SYNCFIFO_SC_STIMULUS_H

#include <systemc>
#include <deque>
#include <string>

struct SyncFifoReference {
    std::deque<unsigned> queue;
    unsigned last_out = 0;
    bool out_known = false;
    void step(bool write, bool read, unsigned data) {
        const bool accept_read = read && !queue.empty();
        const bool accept_write = write && queue.size() < 8;
        const unsigned popped = accept_read ? queue.front() : last_out;
        if (accept_read) queue.pop_front();
        if (accept_write) queue.push_back(data);
        if (accept_read) { last_out = popped; out_known = true; }
    }
};

template<class Cycle>
inline void run_syncfifo_stimulus(Cycle cycle) {
    for (unsigned i = 0; i < 8; ++i)
        cycle(true, false, (i * 37u + 11u) & 0xffu, "fill-" + std::to_string(i));
    cycle(false, true, 0, "read-first");
    cycle(true, true, 0xA7, "simultaneous-interior-read-write");
    for (unsigned i = 0; i < 7; ++i) cycle(false, true, 0, "drain-" + std::to_string(i));
    cycle(false, false, 0, "idle-empty");
}

#endif
