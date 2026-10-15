#ifndef FULL_HALF_ADD_HALF_ADDER_SC_H
#define FULL_HALF_ADD_HALF_ADDER_SC_H

#include <systemc>

SC_MODULE(half_adder) {
    sc_core::sc_in<sc_dt::sc_logic> h_a{"h_a"}, h_b{"h_b"};
    sc_core::sc_out<sc_dt::sc_logic> h_sum{"h_sum"}, h_carry{"h_carry"};

    void evaluate() {
        h_sum.write(h_a.read() ^ h_b.read());
        h_carry.write(h_a.read() & h_b.read());
    }
    SC_CTOR(half_adder) {
        SC_METHOD(evaluate);
        sensitive << h_a << h_b;
    }
};

#endif
