#ifndef MUX_ALWAYS_SC_H
#define MUX_ALWAYS_SC_H

#include <systemc>

using sc_dt::SC_LOGIC_0;
using sc_dt::SC_LOGIC_1;

SC_MODULE(mux_always) {
    sc_core::sc_in<sc_dt::sc_logic> in0{"in0"}, in1{"in1"}, select{"select"};
    sc_core::sc_out<sc_dt::sc_logic> out{"out"};
    void evaluate() {
        out.write((~select.read() & in0.read()) | (select.read() & in1.read()));
    }
    SC_CTOR(mux_always) {
        SC_METHOD(evaluate);
        sensitive << in0 << in1 << select;
    }
};

#endif
