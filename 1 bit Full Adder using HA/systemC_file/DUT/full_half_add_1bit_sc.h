#pragma once

#include <systemc.h>

SC_MODULE(half_adder) {
    sc_in<bool> h_a;
    sc_in<bool> h_b;
    sc_out<bool> h_sum;
    sc_out<bool> h_carry;

    void assign_outputs() {
        h_sum.write(h_a.read() ^ h_b.read());
        h_carry.write(h_a.read() & h_b.read());
    }

    SC_CTOR(half_adder) {
        SC_METHOD(assign_outputs);
        sensitive << h_a << h_b;
    }
};

SC_MODULE(full_half_add_1bit) {
    sc_in<bool> i_a;
    sc_in<bool> i_b;
    sc_in<bool> i_cin;
    sc_out<bool> o_sum;
    sc_out<bool> o_carry;

    sc_signal<bool> w_sum1;
    sc_signal<bool> w_carry1;
    sc_signal<bool> w_carry2;

    half_adder h1;
    half_adder h2;

    void assign_carry() {
        o_carry.write(w_carry1.read() | w_carry2.read());
    }

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

        SC_METHOD(assign_carry);
        sensitive << w_carry1 << w_carry2;
    }
};
