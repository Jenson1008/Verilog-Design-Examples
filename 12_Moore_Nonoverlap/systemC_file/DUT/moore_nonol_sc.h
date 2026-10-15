#ifndef MOORE_NONOL_SC_H
#define MOORE_NONOL_SC_H

#include <systemc>

SC_MODULE(moore_nonol) {
    sc_core::sc_in<sc_dt::sc_logic> in{"in"};
    sc_core::sc_in<sc_dt::sc_logic> clk{"clk"}, reset{"reset"};
    sc_core::sc_out<sc_dt::sc_logic> out{"out"};

    sc_core::sc_signal<sc_dt::sc_lv<3>> present_state{"present_state"};
    sc_core::sc_signal<sc_dt::sc_lv<3>> next_state{"next_state"};
    sc_dt::sc_logic previous_clk = sc_dt::SC_LOGIC_X;
    sc_dt::sc_logic previous_reset = sc_dt::SC_LOGIC_X;

    static sc_dt::sc_lv<3> encode(unsigned value) {
        return sc_dt::sc_lv<3>(sc_dt::sc_uint<3>(value));
    }

    static sc_dt::sc_lv<3> verilog_conditional(unsigned when_true,
                                                unsigned when_false,
                                                sc_dt::sc_logic condition) {
        const sc_dt::sc_lv<3> yes = encode(when_true);
        const sc_dt::sc_lv<3> no = encode(when_false);
        if (condition == sc_dt::SC_LOGIC_1) return yes;
        if (condition == sc_dt::SC_LOGIC_0) return no;

        // Verilog ?: merges each bit when the condition is X/Z.
        sc_dt::sc_lv<3> merged;
        for (int bit = 0; bit < 3; ++bit)
            merged[bit] = yes[bit] == no[bit] ? yes[bit] : sc_dt::SC_LOGIC_X;
        return merged;
    }

    void sequence_register() {
        const sc_dt::sc_logic current_clk = clk.read();
        const sc_dt::sc_logic current_reset = reset.read();
        const bool clk_posedge = verilog_posedge(previous_clk, current_clk);
        const bool reset_posedge = verilog_posedge(previous_reset, current_reset);
        previous_clk = current_clk;
        previous_reset = current_reset;
        if (!clk_posedge && !reset_posedge) return;

        if (reset.read() == sc_dt::SC_LOGIC_1)
            present_state.write(encode(0));  // S0
        else
            present_state.write(next_state.read());
    }

    static bool verilog_posedge(sc_dt::sc_logic previous, sc_dt::sc_logic current) {
        // Verilog posedge includes 0->X/Z and X/Z->1 in addition to 0->1.
        return (previous == sc_dt::SC_LOGIC_0 && current != sc_dt::SC_LOGIC_0) ||
               ((previous == sc_dt::SC_LOGIC_X || previous == sc_dt::SC_LOGIC_Z) &&
                current == sc_dt::SC_LOGIC_1);
    }

    void decode_state() {
        const sc_dt::sc_lv<3> current = present_state.read();
        if (!current.is_01()) return;  // No matching case item: retain the RTL latch values.

        switch (current.to_uint()) {
        case 0: next_state.write(verilog_conditional(1, 0, in.read())); out.write(sc_dt::SC_LOGIC_0); break;
        case 1: next_state.write(verilog_conditional(2, 0, in.read())); out.write(sc_dt::SC_LOGIC_0); break;
        case 2: next_state.write(verilog_conditional(2, 3, in.read())); out.write(sc_dt::SC_LOGIC_0); break;
        case 3: next_state.write(verilog_conditional(4, 0, in.read())); out.write(sc_dt::SC_LOGIC_0); break;
        case 4: next_state.write(verilog_conditional(1, 0, in.read())); out.write(sc_dt::SC_LOGIC_1); break;
        // The RTL case has no default arm; preserve next_state/out for unused encodings 5..7.
        default: break;
        }
    }

    SC_CTOR(moore_nonol) {
        SC_METHOD(sequence_register);
        sensitive << clk << reset;
        dont_initialize();

        SC_METHOD(decode_state);
        sensitive << present_state << in;
    }
};

#endif
