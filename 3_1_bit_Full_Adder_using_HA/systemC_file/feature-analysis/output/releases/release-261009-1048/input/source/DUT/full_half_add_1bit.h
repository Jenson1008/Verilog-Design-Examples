#ifndef FULL_HALF_ADD_1BIT_H
#define FULL_HALF_ADD_1BIT_H

#include <systemc>

#include "half_adder.h"

SC_MODULE(full_half_add_1bit) {
    sc_core::sc_in<sc_dt::sc_logic> i_a;
    sc_core::sc_in<sc_dt::sc_logic> i_b;
    sc_core::sc_in<sc_dt::sc_logic> i_cin;
    sc_core::sc_out<sc_dt::sc_logic> o_sum;
    sc_core::sc_out<sc_dt::sc_logic> o_carry;

    sc_core::sc_signal<sc_dt::sc_logic> w_sum1;
    sc_core::sc_signal<sc_dt::sc_logic> w_carry1;
    sc_core::sc_signal<sc_dt::sc_logic> w_carry2;

    half_adder h1;
    half_adder h2;

    SC_CTOR(full_half_add_1bit)
        : h1("h1"), h2("h2") {
        h1.h_a(i_a);
        h1.h_b(i_b);
        h1.h_sum(w_sum1);
        h1.h_carry(w_carry1);

        h2.h_a(w_sum1);
        h2.h_b(i_cin);
        h2.h_sum(o_sum);
        h2.h_carry(w_carry2);

        SC_METHOD(combine_carries);
        sensitive << w_carry1 << w_carry2;
    }

    void combine_carries() {
        o_carry.write(w_carry1.read() | w_carry2.read());
    }
};

#endif
