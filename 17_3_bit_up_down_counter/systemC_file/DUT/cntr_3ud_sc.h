#ifndef CNTR_3UD_SC_H
#define CNTR_3UD_SC_H

#include <systemc>

SC_MODULE(cntr_3ud) {
    sc_core::sc_in<sc_dt::sc_logic> up_down{"up_down"};
    sc_core::sc_in<bool> clk{"clk"}, reset{"reset"};
    sc_core::sc_out<sc_dt::sc_lv<3>> bin_count{"bin_count"};
    sc_core::sc_signal<sc_dt::sc_lv<3>> count_state{"count_state"};

    void advance() {
        const sc_dt::sc_lv<3> current = count_state.read();
        sc_dt::sc_lv<3> next;
        if (reset.read()) {
            next = "000";
        } else if (!current.is_01()) {
            next = "XXX";
        } else {
            const unsigned value = current.to_uint();
            // In a Verilog procedural if, X/Z is not true and takes the up-count else branch.
            const unsigned result = up_down.read() == sc_dt::SC_LOGIC_1
                ? ((value + 7U) & 7U) : ((value + 1U) & 7U);
            next = sc_dt::sc_lv<3>(sc_dt::sc_uint<3>(result));
        }
        count_state.write(next);
        bin_count.write(next);
    }

    SC_CTOR(cntr_3ud) {
        SC_METHOD(advance);
        sensitive << clk.pos();
        dont_initialize();
    }
};

#endif
