#ifndef MUX_CONDT_SC_H
#define MUX_CONDT_SC_H

#include <systemc>

using sc_dt::SC_LOGIC_0;
using sc_dt::SC_LOGIC_1;
using sc_dt::SC_LOGIC_X;

SC_MODULE(mux_condt) {
    sc_core::sc_in<sc_dt::sc_lv<2>> in0{"in0"}, in1{"in1"}, in2{"in2"}, in3{"in3"};
    sc_core::sc_in<sc_dt::sc_lv<2>> select{"select"};
    sc_core::sc_out<sc_dt::sc_lv<2>> out{"out"};

    static sc_dt::sc_logic merge_bit(sc_dt::sc_logic cond,
                                     sc_dt::sc_logic when_true,
                                     sc_dt::sc_logic when_false) {
        if (cond == SC_LOGIC_1) return when_true;
        if (cond == SC_LOGIC_0) return when_false;
        return when_true == when_false &&
               (when_true == SC_LOGIC_0 || when_true == SC_LOGIC_1)
                   ? when_true : SC_LOGIC_X;
    }
    static sc_dt::sc_lv<2> conditional(sc_dt::sc_logic cond,
                                       const sc_dt::sc_lv<2>& yes,
                                       const sc_dt::sc_lv<2>& no) {
        sc_dt::sc_lv<2> result;
        for (int bit=0; bit<2; ++bit) result[bit]=merge_bit(cond,yes[bit],no[bit]);
        return result;
    }
    void evaluate() {
        const auto s=select.read();
        const auto low=conditional(s[0],in1.read(),in0.read());
        const auto high=conditional(s[0],in3.read(),in2.read());
        out.write(conditional(s[1],high,low));
    }
    SC_CTOR(mux_condt) {
        SC_METHOD(evaluate);
        sensitive << in0 << in1 << in2 << in3 << select;
    }
};

#endif
