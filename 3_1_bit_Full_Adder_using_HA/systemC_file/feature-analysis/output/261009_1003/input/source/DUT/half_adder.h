#ifndef HALF_ADDER_H
#define HALF_ADDER_H

#include <systemc>

SC_MODULE(half_adder) {
    sc_core::sc_in<sc_dt::sc_logic> h_a;
    sc_core::sc_in<sc_dt::sc_logic> h_b;
    sc_core::sc_out<sc_dt::sc_logic> h_sum;
    sc_core::sc_out<sc_dt::sc_logic> h_carry;

    SC_CTOR(half_adder) {
        SC_METHOD(combinational);
        sensitive << h_a << h_b;
    }

    void combinational() {
        h_sum.write(h_a.read() ^ h_b.read());
        h_carry.write(h_a.read() & h_b.read());
    }
};

#endif
