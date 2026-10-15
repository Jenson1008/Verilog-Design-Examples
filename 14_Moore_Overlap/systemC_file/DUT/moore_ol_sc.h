#ifndef MOORE_OL_SC_H
#define MOORE_OL_SC_H

#include <systemc>

SC_MODULE(moore_ol) {
    sc_core::sc_in<sc_dt::sc_logic> in{"in"};
    sc_core::sc_in<bool> clk{"clk"}, reset{"reset"};
    sc_core::sc_out<sc_dt::sc_logic> out{"out"};
    sc_core::sc_signal<sc_dt::sc_lv<3>> state{"state"};
    sc_dt::sc_lv<3> next_state;
    void update_state() { if (reset.read()) state.write("000"); else state.write(next_state); }
    void decode() {
        const sc_dt::sc_lv<3> current = state.read();
        if (!current.is_01()) { next_state = "000"; return; } // Default branch leaves output unchanged.
        const unsigned s = current.to_uint();
        const bool bit = in.read() == sc_dt::SC_LOGIC_1; // Verilog procedural if treats X/Z as false.
        switch (s) {
        case 0: next_state = bit ? "001" : "000"; out.write(sc_dt::SC_LOGIC_0); break;
        case 1: next_state = bit ? "010" : "000"; out.write(sc_dt::SC_LOGIC_0); break;
        case 2: next_state = bit ? "010" : "011"; out.write(sc_dt::SC_LOGIC_0); break;
        case 3: next_state = bit ? "100" : "000"; out.write(sc_dt::SC_LOGIC_0); break;
        case 4: next_state = bit ? "010" : "000"; out.write(sc_dt::SC_LOGIC_1); break;
        default: next_state = "000"; break;
        }
    }
    SC_CTOR(moore_ol) {
        SC_METHOD(update_state); sensitive << clk.pos() << reset.pos(); dont_initialize();
        SC_METHOD(decode); sensitive << state << in;
    }
};

#endif
