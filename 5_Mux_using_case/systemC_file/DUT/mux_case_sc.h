#ifndef MUX_CASE_SC_H
#define MUX_CASE_SC_H

#include <systemc>

using sc_dt::SC_LOGIC_0;
using sc_dt::SC_LOGIC_1;

SC_MODULE(mux_case) {
    sc_core::sc_in<sc_dt::sc_lv<2>> in0{"in0"}, in1{"in1"}, in2{"in2"}, in3{"in3"};
    sc_core::sc_in<sc_dt::sc_lv<2>> select{"select"};
    sc_core::sc_out<sc_dt::sc_lv<2>> out{"out"};

    void evaluate() {
        const auto s = select.read();
        if (s[1] == SC_LOGIC_0 && s[0] == SC_LOGIC_0) out.write(in0.read());
        else if (s[1] == SC_LOGIC_0 && s[0] == SC_LOGIC_1) out.write(in1.read());
        else if (s[1] == SC_LOGIC_1 && s[0] == SC_LOGIC_0) out.write(in2.read());
        else if (s[1] == SC_LOGIC_1 && s[0] == SC_LOGIC_1) out.write(in3.read());
        // Verilog case has no default: an X/Z select leaves the previous value.
    }

    SC_CTOR(mux_case) {
        SC_METHOD(evaluate);
        // Intentionally mirrors RTL "always @(select)": data-only changes do not update out.
        sensitive << select;
    }
};

#endif
