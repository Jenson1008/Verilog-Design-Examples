#ifndef FULL_ADD_4BIT_SC_H
#define FULL_ADD_4BIT_SC_H

#include <systemc>

using sc_dt::SC_LOGIC_0;
using sc_dt::SC_LOGIC_1;

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

SC_MODULE(full_add_4bit) {
    sc_core::sc_in<sc_dt::sc_lv<4>> i_a{"i_a"};
    sc_core::sc_in<sc_dt::sc_lv<4>> i_b{"i_b"};
    sc_core::sc_in<sc_dt::sc_logic> i_cin{"i_cin"};
    sc_core::sc_out<sc_dt::sc_lv<4>> o_sum{"o_sum"};
    sc_core::sc_out<sc_dt::sc_logic> o_carry{"o_carry"};

    sc_core::sc_signal<sc_dt::sc_logic> a0{"a0"},a1{"a1"},a2{"a2"},a3{"a3"};
    sc_core::sc_signal<sc_dt::sc_logic> b0{"b0"},b1{"b1"},b2{"b2"},b3{"b3"};
    sc_core::sc_signal<sc_dt::sc_logic> s0{"s0"},s1{"s1"},s2{"s2"},s3{"s3"};
    sc_core::sc_signal<sc_dt::sc_logic> c1{"c1"},c2{"c2"},c3{"c3"},c4{"c4"};
    full_half_add_1bit fa0{"fa0"},fa1{"fa1"},fa2{"fa2"},fa3{"fa3"};

    void split_inputs() {
        const auto a=i_a.read(), b=i_b.read();
        a0.write(a[0]); a1.write(a[1]); a2.write(a[2]); a3.write(a[3]);
        b0.write(b[0]); b1.write(b[1]); b2.write(b[2]); b3.write(b[3]);
    }
    void pack_outputs() {
        sc_dt::sc_lv<4> sum;
        sum[0]=s0.read(); sum[1]=s1.read(); sum[2]=s2.read(); sum[3]=s3.read();
        o_sum.write(sum); o_carry.write(c4.read());
    }

    SC_CTOR(full_add_4bit) {
        fa0.i_a(a0); fa0.i_b(b0); fa0.i_cin(i_cin); fa0.o_sum(s0); fa0.o_carry(c1);
        fa1.i_a(a1); fa1.i_b(b1); fa1.i_cin(c1); fa1.o_sum(s1); fa1.o_carry(c2);
        fa2.i_a(a2); fa2.i_b(b2); fa2.i_cin(c2); fa2.o_sum(s2); fa2.o_carry(c3);
        fa3.i_a(a3); fa3.i_b(b3); fa3.i_cin(c3); fa3.o_sum(s3); fa3.o_carry(c4);
        SC_METHOD(split_inputs);
        sensitive << i_a << i_b;
        SC_METHOD(pack_outputs);
        sensitive << s0 << s1 << s2 << s3 << c4;
    }
};

#endif
