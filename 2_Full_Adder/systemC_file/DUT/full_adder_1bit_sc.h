#ifndef FULL_ADDER_1BIT_SC_H
#define FULL_ADDER_1BIT_SC_H

#include <systemc>

using sc_dt::SC_LOGIC_0;
using sc_dt::SC_LOGIC_1;

SC_MODULE(full_adder_1bit) {
    sc_core::sc_in<sc_dt::sc_logic> i_a{"i_a"};
    sc_core::sc_in<sc_dt::sc_logic> i_b{"i_b"};
    sc_core::sc_in<sc_dt::sc_logic> i_cin{"i_cin"};
    sc_core::sc_out<sc_dt::sc_logic> o_sum{"o_sum"};
    sc_core::sc_out<sc_dt::sc_logic> o_carry{"o_carry"};

    void evaluate() {
        o_sum.write(i_a.read() ^ i_b.read() ^ i_cin.read());
        o_carry.write((i_a.read() & i_b.read()) |
                      (i_b.read() & i_cin.read()) |
                      (i_cin.read() & i_a.read()));
    }

    SC_CTOR(full_adder_1bit) {
        SC_METHOD(evaluate);
        sensitive << i_a << i_b << i_cin;
    }
};

#endif
