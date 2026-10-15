#ifndef CLKDIVIDER_SC_H
#define CLKDIVIDER_SC_H

#include <systemc>

SC_MODULE(clkdivider) {
    sc_core::sc_in<bool> reset{"reset"}, clk{"clk"};
    sc_core::sc_out<sc_dt::sc_logic> out_clk{"out_clk"};
    sc_core::sc_signal<sc_dt::sc_lv<2>> clk_count{"clk_count"};
    sc_dt::sc_logic output_reg = sc_dt::SC_LOGIC_X;
    void divide() {
        if (reset.read()) {
            clk_count.write("00");
            output_reg = sc_dt::SC_LOGIC_0;
        } else {
            const sc_dt::sc_lv<2> old_count = clk_count.read();
            output_reg = old_count[1];
            if (old_count.is_01()) {
                const unsigned next = (old_count.to_uint() + 1U) & 3U;
                clk_count.write(sc_dt::sc_lv<2>(sc_dt::sc_uint<2>(next)));
            } else clk_count.write("XX");
        }
        out_clk.write(output_reg);
    }
    SC_CTOR(clkdivider) {
        SC_METHOD(divide);
        sensitive << clk.pos();
        dont_initialize();
    }
};

#endif
