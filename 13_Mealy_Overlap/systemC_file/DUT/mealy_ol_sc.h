#ifndef MEALY_OL_SC_H
#define MEALY_OL_SC_H

#include <systemc>

SC_MODULE(mealy_ol) {
    sc_core::sc_in<sc_dt::sc_logic> in{"in"};
    sc_core::sc_in<bool> clk{"clk"}, reset{"reset"};
    sc_core::sc_out<sc_dt::sc_logic> out{"out"};
    sc_core::sc_signal<sc_dt::sc_lv<2>> state{"state"};
    sc_dt::sc_lv<2> next_state;

    void update_state() {
        if (reset.read()) state.write("00");
        else state.write(next_state);
    }
    void decode() {
        const sc_dt::sc_lv<2> current = state.read();
        if (!current.is_01()) return; // RTL case has no default assignment here.
        const unsigned s = current.to_uint();
        const bool bit = in.read() == sc_dt::SC_LOGIC_1; // Procedural if treats X/Z as not true.
        switch (s) {
        case 0: next_state = bit ? "01" : "00"; out.write(sc_dt::SC_LOGIC_0); break;
        case 1: next_state = bit ? "10" : "00"; out.write(sc_dt::SC_LOGIC_0); break;
        case 2: next_state = bit ? "10" : "11"; out.write(sc_dt::SC_LOGIC_0); break;
        case 3: next_state = bit ? "01" : "00"; out.write(bit ? sc_dt::SC_LOGIC_1 : sc_dt::SC_LOGIC_0); break;
        default: break;
        }
    }
    SC_CTOR(mealy_ol) {
        SC_METHOD(update_state);
        sensitive << clk.pos() << reset.pos();
        dont_initialize();
        SC_METHOD(decode);
        sensitive << state << in;
    }
};

#endif
