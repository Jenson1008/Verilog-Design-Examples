#ifndef FULL_HALF_ADD_1BIT_SC_H
#define FULL_HALF_ADD_1BIT_SC_H

#include <systemc>
#include "half_adder_sc.h"

SC_MODULE(full_half_add_1bit) {
    sc_core::sc_in<sc_dt::sc_logic> i_a{"i_a"}, i_b{"i_b"}, i_cin{"i_cin"};
    sc_core::sc_out<sc_dt::sc_logic> o_sum{"o_sum"}, o_carry{"o_carry"};
    sc_core::sc_signal<sc_dt::sc_logic> w_sum1{"w_sum1"}, w_carry1{"w_carry1"}, w_carry2{"w_carry2"};
    half_adder h1{"h1"}, h2{"h2"};

    void combine_carry() { o_carry.write(w_carry1.read() | w_carry2.read()); }
    SC_CTOR(full_half_add_1bit) {
        h1.h_a(i_a); h1.h_b(i_b); h1.h_sum(w_sum1); h1.h_carry(w_carry1);
        h2.h_a(w_sum1); h2.h_b(i_cin); h2.h_sum(o_sum); h2.h_carry(w_carry2);
        SC_METHOD(combine_carry);
        sensitive << w_carry1 << w_carry2;
    }
};

#endif
