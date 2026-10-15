#ifndef HALF_ADDER_SC_H
#define HALF_ADDER_SC_H

#include <systemc>

SC_MODULE(half_adder) {
    sc_core::sc_in<sc_dt::sc_logic> i_a{"i_a"}, i_b{"i_b"};
    sc_core::sc_out<sc_dt::sc_logic> o_sum{"o_sum"}, o_carry{"o_carry"};

    void evaluate() {
        o_sum.write(i_a.read() ^ i_b.read());
        o_carry.write(i_a.read() & i_b.read());
    }
    SC_CTOR(half_adder) {
        SC_METHOD(evaluate);
        sensitive << i_a << i_b;
    }
};

#endif
