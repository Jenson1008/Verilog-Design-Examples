#ifndef MEALY_NONOL_SC_H
#define MEALY_NONOL_SC_H

#include <systemc>

SC_MODULE(mealy_nonol) {
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
        const sc_dt::sc_logic bit = in.read();
        if (!current.is_01()) { next_state = "00"; return; }
        const unsigned s = current.to_uint();
        switch (s) {
        case 0:
            next_state = bit == sc_dt::SC_LOGIC_1 ? "01" : bit == sc_dt::SC_LOGIC_0 ? "00" : "0X";
            out.write(sc_dt::SC_LOGIC_0); break;
        case 1:
            next_state = bit == sc_dt::SC_LOGIC_1 ? "10" : bit == sc_dt::SC_LOGIC_0 ? "00" : "X0";
            out.write(sc_dt::SC_LOGIC_0); break;
        case 2:
            next_state = bit == sc_dt::SC_LOGIC_1 ? "10" : bit == sc_dt::SC_LOGIC_0 ? "11" : "1X";
            out.write(sc_dt::SC_LOGIC_0); break;
        case 3:
            next_state = "00";
            out.write(bit == sc_dt::SC_LOGIC_1 ? sc_dt::SC_LOGIC_1 : bit == sc_dt::SC_LOGIC_0 ? sc_dt::SC_LOGIC_0 : sc_dt::SC_LOGIC_X);
            break;
        default: next_state = "00"; break;
        }
    }
    SC_CTOR(mealy_nonol) {
        SC_METHOD(update_state);
        sensitive << clk.pos() << reset.pos();
        dont_initialize();
        SC_METHOD(decode);
        sensitive << state << in;
    }
};

#endif
