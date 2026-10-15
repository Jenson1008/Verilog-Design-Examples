#ifndef SEQMULT_SC_STIMULUS_H
#define SEQMULT_SC_STIMULUS_H

#include <systemc>

inline unsigned seqmult_reference(unsigned a, unsigned b) { return (a * b) & 0xffffu; }

template<class Cycle>
inline void run_seqmult_stimulus(Cycle cycle) {
    cycle(3, 7, true, "first-load-old-partials-unknown");
    cycle(9, 11, true, "second-load-checks-prior-product");
    cycle(255, 2, false, "load-low-holds-output");
    cycle(128, 255, true, "third-load-checks-prior-product");
    cycle(0, 0, true, "zero-multiplicand");
}

#endif
