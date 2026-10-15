#ifndef LFSR_SC_STIMULUS_H
#define LFSR_SC_STIMULUS_H

#include <systemc>
#include <string>

inline unsigned lfsr_oracle_next(unsigned state, bool reset) {
    if (reset || state == 0) return 1;
    return (((state >> 1) ^ state) & 1u) << 3 | (state >> 1);
}

template<class Cycle>
inline void run_lfsr_stimulus(Cycle cycle) {
    cycle(true, "synchronous-reset-seed");
    cycle(false, "first-feedback");
    for (unsigned i = 0; i < 18; ++i) cycle(false, "lfsr-transition-" + std::to_string(i));
    cycle(true, "reset-reseed");
    cycle(false, "post-reset-transition");
}

#endif
