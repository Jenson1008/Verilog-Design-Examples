#ifndef NO_1S_SC_H
#define NO_1S_SC_H

#include <systemc>

SC_MODULE(no_1s) {
    sc_core::sc_in<sc_dt::sc_lv<16>> i_a{"i_a"};
    sc_core::sc_in<bool> clk{"clk"}, reset{"reset"};
    sc_core::sc_out<sc_dt::sc_lv<4>> no_ones{"no_ones"};
    sc_core::sc_signal<int> i{"i"}; // RTL integer is initialized to zero and is not reset.

    void count_bit() {
        const int index = i.read();
        if (reset.read()) {
            no_ones.write(sc_dt::sc_lv<4>("0000"));
        } else if (index < 16) {
            const sc_dt::sc_logic bit = i_a.read()[index];
            const sc_dt::sc_lv<4> prior = no_ones.read();
            if (bit == sc_dt::SC_LOGIC_0 || bit == sc_dt::SC_LOGIC_1) {
                if (prior.is_01()) {
                    const unsigned sum = prior.to_uint() + (bit == sc_dt::SC_LOGIC_1 ? 1U : 0U);
                    no_ones.write(sc_dt::sc_lv<4>(sc_dt::sc_uint<4>(sum)));
                } else no_ones.write(sc_dt::sc_lv<4>("XXXX"));
            } else no_ones.write(sc_dt::sc_lv<4>("XXXX"));
        }
        i.write(index + 1); // Deliberately increments on every rising edge, including reset.
    }
    SC_CTOR(no_1s) {
        SC_METHOD(count_bit);
        sensitive << clk.pos();
        dont_initialize();
    }
};

#endif
