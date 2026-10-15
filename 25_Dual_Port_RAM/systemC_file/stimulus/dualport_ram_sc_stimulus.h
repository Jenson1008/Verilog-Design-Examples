#ifndef DUALPORT_RAM_SC_STIMULUS_H
#define DUALPORT_RAM_SC_STIMULUS_H

#include <systemc>
#include <string>

struct DualPortRamTransaction {
    const char* name;
    bool write_a;
    unsigned address_a;
    sc_dt::sc_lv<128> input_a;
    bool write_b;
    unsigned address_b;
    sc_dt::sc_lv<128> input_b;
};

inline sc_dt::sc_lv<128> dualport_ram_pattern(unsigned seed) {
    sc_dt::sc_lv<128> value;
    for (unsigned bit = 0; bit < 128; ++bit) {
        const unsigned mixed = (bit * 29u + seed * 43u + (bit >> 2) * seed) ^ (seed >> (bit % 5));
        value[bit] = (mixed & 1u) ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0;
    }
    return value;
}

template<class Transaction>
inline void run_dualport_ram_stimulus(Transaction transaction) {
    const auto d0 = dualport_ram_pattern(1), d1 = dualport_ram_pattern(2);
    const auto d2 = dualport_ram_pattern(3), d3 = dualport_ram_pattern(4);
    const auto d4 = dualport_ram_pattern(5), d5 = dualport_ram_pattern(6);
    transaction({"write-a-addr0", true, 0, d0, false, 0, d1});
    transaction({"independent-writes-addr2-5", true, 2, d2, true, 5, d3});
    transaction({"read-addr0-and-5", false, 0, d4, false, 5, d5});
    transaction({"read-addr2-and-5", false, 2, d4, false, 5, d5});
    transaction({"write-a-read-b-same-address", true, 3, d4, false, 3, d5});
    transaction({"read-updated-a-and-addr2", false, 3, d5, false, 2, d4});
    transaction({"read-a-addr5-write-b-addr6", false, 5, d0, true, 6, d5});
    transaction({"read-addr0-and-6", false, 0, d1, false, 6, d2});
    transaction({"independent-writes-addr1-7", true, 1, d1, true, 7, d2});
    transaction({"read-back-addr1-and-7", false, 1, d0, false, 7, d3});
}

#endif
