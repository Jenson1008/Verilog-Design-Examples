#ifndef CUSTDESIGN_SC_STIMULUS_H
#define CUSTDESIGN_SC_STIMULUS_H

#include <systemc>
#include <string>

struct CustdesignReference {
    sc_dt::sc_logic a = sc_dt::SC_LOGIC_0, abar = sc_dt::SC_LOGIC_1;
    sc_dt::sc_logic b = sc_dt::SC_LOGIC_0, bbar = sc_dt::SC_LOGIC_1;

    sc_dt::sc_logic output(sc_dt::sc_logic x) const { return (a ^ x) ^ b; }
    void tick(sc_dt::sc_logic x, bool reset) {
        const auto old_a = a, old_b = b, old_bbar = bbar;
        a = reset ? sc_dt::SC_LOGIC_0 : (old_b | ((~old_bbar) & old_a));
        abar = ~old_a;
        const auto r = old_a ^ x;
        b = reset ? sc_dt::SC_LOGIC_0 : ((~r) | ((~r) & old_b));
        bbar = ~old_b;
    }
};

template<class Cycle>
inline void run_custdesign_stimulus(Cycle cycle) {
    cycle(true, false, "reset-warmup-1");
    cycle(true, true, "reset-warmup-2");
    for (unsigned i = 0; i < 24; ++i)
        cycle(false, (i * 13u + 5u) & 1u, "state-transition-" + std::to_string(i));
    cycle(true, true, "reset-reassert");
    cycle(false, false, "post-reset");
}

#endif
