#ifndef BIN_GRY_SC_H
#define BIN_GRY_SC_H

#include <systemc>

template <int N = 8>
struct bin_gry : sc_core::sc_module {
    sc_core::sc_in<sc_dt::sc_lv<N>> binary{"binary"};
    sc_core::sc_out<sc_dt::sc_lv<N>> gray{"gray"};
    void convert() {
        const sc_dt::sc_lv<N> b = binary.read();
        sc_dt::sc_lv<N> g;
        g[N - 1] = b[N - 1];
        for (int bit = N - 2; bit >= 0; --bit) g[bit] = b[bit + 1] ^ b[bit];
        gray.write(g);
    }
    SC_HAS_PROCESS(bin_gry);
    explicit bin_gry(sc_core::sc_module_name name) : sc_core::sc_module(name) {
        static_assert(N >= 2, "RTL slice [N-2:0] requires N >= 2");
        SC_METHOD(convert); sensitive << binary;
    }
};

#endif
