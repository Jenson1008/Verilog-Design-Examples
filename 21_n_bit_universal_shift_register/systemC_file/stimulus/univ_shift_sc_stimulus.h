#ifndef UNIV_SHIFT_SC_STIMULUS_H
#define UNIV_SHIFT_SC_STIMULUS_H

#include <systemc>

inline sc_dt::sc_lv<16> shift_pattern(unsigned value) {
    return sc_dt::sc_lv<16>(value & 0xffffu);
}

template<class Check>
inline void run_univ_shift_vectors(Check check) {
    const auto raw = shift_pattern(0xA53C);
    check("control-0-hold", raw, 0, sc_dt::sc_lv<16>(0));
    check("control-1-left", raw, 1, raw << 1);
    check("control-2-right", raw, 2, raw >> 1);
    check("control-3-load", raw, 3, raw);
    check("left-msb-boundary", shift_pattern(0x8001), 1, shift_pattern(0x0002));
    check("right-lsb-boundary", shift_pattern(0x8001), 2, shift_pattern(0x4000));
    check("load-before-hold", shift_pattern(0x8001), 3, shift_pattern(0x8001));
    check("hold-after-load", shift_pattern(0xFFFF), 0, shift_pattern(0x8001));
}

#endif
