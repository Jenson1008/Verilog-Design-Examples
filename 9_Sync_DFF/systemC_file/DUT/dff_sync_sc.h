#ifndef DFF_SYNC_SC_H
#define DFF_SYNC_SC_H

#include <systemc>

using sc_dt::SC_LOGIC_0;
using sc_dt::SC_LOGIC_1;
using sc_dt::SC_LOGIC_X;

SC_MODULE(dff_sync) {
    sc_core::sc_in<sc_dt::sc_logic> i_d{"i_d"}, clk{"clk"}, reset{"reset"};
    sc_core::sc_out<sc_dt::sc_logic> o_q{"o_q"}, o_qb{"o_qb"};
    sc_core::sc_signal<sc_dt::sc_logic> state{"state"};

    void capture() {
        state.write(reset.read() == SC_LOGIC_1 ? SC_LOGIC_0 : i_d.read());
    }
    void drive_outputs() {
        o_q.write(state.read());
        o_qb.write(~state.read());
    }
    SC_CTOR(dff_sync) {
        SC_METHOD(capture);
        sensitive << clk.pos();
        SC_METHOD(drive_outputs);
        sensitive << state;
    }
};

#endif
