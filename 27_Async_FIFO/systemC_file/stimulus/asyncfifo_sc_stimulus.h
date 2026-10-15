#ifndef ASYNCFIFO_SC_STIMULUS_H
#define ASYNCFIFO_SC_STIMULUS_H

#include <systemc>
#include <deque>
#include <string>

struct AsyncFifoReference {
    std::deque<unsigned> queue;
    unsigned last_out = 0;
    void write(unsigned data) { if (queue.size() < 8) queue.push_back(data); }
    void read() { if (!queue.empty()) { last_out = queue.front(); queue.pop_front(); } }
};

template<class Write, class CheckFull, class Read, class CheckEmpty>
inline void run_asyncfifo_stimulus(Write write, CheckFull check_full,
                                   Read read, CheckEmpty check_empty) {
    for (unsigned i = 0; i < 8; ++i) write(i, (i * 43u + 9u) & 0xffu);
    check_full();
    for (unsigned i = 0; i < 8; ++i) read(i);
    check_empty();
}

#endif
