#ifndef SINGLEPORT_RAM_SC_STIMULUS_H
#define SINGLEPORT_RAM_SC_STIMULUS_H

#include <systemc>
#include <string>

inline sc_dt::sc_lv<128> ram_test_pattern(unsigned slot, unsigned salt) {
    sc_dt::sc_lv<128> v;
    for (unsigned bit = 0; bit < 128; ++bit)
        v[bit] = (((bit * 17u + slot * 29u + salt * 11u) ^ (bit >> 1)) & 1u)
                     ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0;
    return v;
}

template<class Transaction>
inline void run_singleport_ram_stimulus(Transaction transact) {
    for (unsigned a = 0; a < 8; ++a)
        transact(a, true, ram_test_pattern(a, 1), "write-slot-" + std::to_string(a));
    for (unsigned a = 0; a < 8; ++a)
        transact(a, false, sc_dt::sc_lv<128>(0), "read-slot-" + std::to_string(a));
    transact(3, true, ram_test_pattern(3, 7), "overwrite-slot-3");
    transact(3, false, sc_dt::sc_lv<128>(0), "read-overwritten-slot-3");
}

#endif
